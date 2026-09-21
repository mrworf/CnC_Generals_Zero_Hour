bool TextureLoadTaskClass::Begin_Load(void)
{
	WWASSERT(TextureLoader::Is_DX8_Thread());

	bool loaded = false;

	// if allowed, begin a compressed load
	if (Texture->Is_Compression_Allowed()) {
		loaded = Begin_Compressed_Load();
	}

	// otherwise, begin an uncompressed load
	if (!loaded) {
		loaded = Begin_Uncompressed_Load();
	}

	// if not loaded, abort.
	if (!loaded) {
		return false;
	}

	// lock surfaces in preparation for copy
	Lock_Surfaces();

	State = STATE_LOAD_BEGUN;

	return true;
}


// ----------------------------------------------------------------------------
//
// Load mipmap levels to a pre-generated and locked texture object based on
// information in load task object. Try loading from a DDS file first and if
// that fails try a TGA.
//
// ----------------------------------------------------------------------------
bool TextureLoadTaskClass::Load(void)
{
	WWMEMLOG(MEM_TEXTURE);
#if defined(ZH_WW3D_CPU_ONLY)
	WWASSERT(CpuTextureHandle);
#else
	WWASSERT(Peek_D3D_Texture());
#endif

	bool loaded = false;

	// if allowed, try to load compressed mipmaps
	if (Texture->Is_Compression_Allowed()) {
		loaded = Load_Compressed_Mipmap();
	}

	// otherwise, load uncompressed mipmaps
	if (!loaded) {
		loaded = Load_Uncompressed_Mipmap();
	}

	State = STATE_LOAD_MIPMAP;

#if defined(ZH_WW3D_CPU_ONLY)
	if (!loaded) throw std::runtime_error("original texture mip source is missing or malformed");
#endif

	return loaded;
}


void TextureLoadTaskClass::End_Load(void)
{
	WWASSERT(TextureLoader::Is_DX8_Thread());

	Unlock_Surfaces();
	Apply(true);

	State = STATE_LOAD_COMPLETE;
}


void TextureLoadTaskClass::Finish_Load(void)
{
	switch (State) {
		// NOTE: fall-through below is intentional.

		case STATE_NONE:
			if (!Begin_Load()) {
				Apply_Missing_Texture();
				break;
			}

		case STATE_LOAD_BEGUN:
			Load();

		case STATE_LOAD_MIPMAP:
			End_Load();

		default:
			break;
	}
}


void TextureLoadTaskClass::Apply_Missing_Texture(void)
{
	WWASSERT(TextureLoader::Is_DX8_Thread());
	WWASSERT(!D3DTexture);

#if defined(ZH_WW3D_CPU_ONLY)
	auto& edge=zh::original_runtime::OriginalGpuEdge::required();
	if (!Texture->As_TextureClass())
		throw std::runtime_error("original missing texture subclass is unsupported");
	const auto fallback=edge.missing_texture();
	edge.publish_texture(Texture,fallback,true);
	Texture->As_TextureClass()->Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,128,128);
#else
	D3DTexture = MissingTexture::_Get_Missing_Texture();
	Apply(true);
#endif
}


void TextureLoadTaskClass::Apply(bool initialize)
{
#if defined(ZH_WW3D_CPU_ONLY)
	WWASSERT(CpuTextureHandle);
#else
	WWASSERT(D3DTexture);
#endif

	// Verify that none of the mip levels are locked
	for (unsigned i=0;i<MipLevelCount;++i) {
		WWASSERT(LockedSurfacePtr[i]==NULL);
	}

#if defined(ZH_WW3D_CPU_ONLY)
	if (!initialize || !Texture->As_TextureClass())
		throw std::runtime_error("original texture task publication is unsupported");
	const unsigned width=std::max(1U,Width>>Reduction);
	const unsigned height=std::max(1U,Height>>Reduction);
	zh::original_runtime::OriginalGpuEdge::required().publish_texture(Texture,CpuTextureHandle);
	Texture->As_TextureClass()->Apply_Gpu_Texture(Format,width,height);
	CpuTextureHandle={};
#else
	Texture->Apply_New_Surface(D3DTexture, initialize);
	D3DTexture->Release();
	D3DTexture = NULL;
#endif
}
