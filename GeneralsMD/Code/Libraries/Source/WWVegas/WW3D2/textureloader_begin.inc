bool TextureLoadTaskClass::Begin_Compressed_Load(void)
{
	unsigned orig_w,orig_h,orig_d,orig_mip_count,reduction;
	WW3DFormat orig_format;
	if (!Get_Texture_Information
		  (
				Texture->Get_Full_Path(),
				reduction,
				orig_w,
				orig_h,
				orig_d,
				orig_format,
				orig_mip_count,
				true
			)
		)
	{
		return false;
	}

	// Destination size will be the next power of two square from the larger width and height...
	unsigned int width	= orig_w;
	unsigned int height	= orig_h;
	TextureLoader::Validate_Texture_Size(width, height,orig_d);

	// If the size doesn't match, try and see if texture reduction would help... (mainly for
	// cases where loaded texture is larger than hardware limit)
	if (width != orig_w || height != orig_h)
	{
		for (unsigned int i = 1; i < orig_mip_count; ++i)
		{
			unsigned w=orig_w>>i;
			if (w<4) w=4;
			unsigned h=orig_h>>i;
			if (h<4) h=4;
			unsigned tmp_w=w;
			unsigned tmp_h=h;

			TextureLoader::Validate_Texture_Size(w,h,orig_d);

			if (w == tmp_w && h == tmp_h)
			{
				Reduction	+= i;
				width			=	w;
				height		=	h;
				break;
			}
		}
	}

	Width		= width;
	Height	= height;
	Format	= Get_Valid_Texture_Format(orig_format, Texture->Is_Compression_Allowed());
	Reduction = reduction;


	if (!Texture->Is_Reducible() || Texture->MipLevelCount == MIP_LEVELS_1)
		Reduction = 0;	//app doesn't want this texture to ever be reduced.
	else
	//Make sure we don't reduce below the level requested by the app
	if (Texture->MipLevelCount != MIP_LEVELS_ALL && (Texture->MipLevelCount - Reduction) < 1)
		Reduction = Texture->MipLevelCount - 1;

	//Another sanity check
	if (Reduction >= orig_mip_count)
		Reduction = 0;	//should not be possible to get here, but check just in case.

	unsigned int mip_level_count = Get_Mip_Level_Count();
	int reducedWidth=Width;
	int reducedHeight=Height;

	// If texture wants all mip levels, take as many as the file contains (not necessarily all)
	// Otherwise take as many mip levels as the texture wants, not to exceed the count in file...
	if (!mip_level_count)
	{
		reducedWidth >>= Reduction;
		reducedHeight >>= Reduction;
		mip_level_count = orig_mip_count-Reduction;//dds_file.Get_Mip_Level_Count();
		if (mip_level_count < 1)
			mip_level_count = 1;	//sanity check to make sure something gets loaded.
	}
	else
	{
		if (mip_level_count > orig_mip_count)
		{	//dds_file.Get_Mip_Level_Count()) {
			mip_level_count = orig_mip_count;//dds_file.Get_Mip_Level_Count();
		}

		if (Reduction)
		{	reducedWidth >>= Reduction;
			reducedHeight >>= Reduction;
			mip_level_count -= Reduction;	//reduced requested number by those removed.
		}
	}

	// Once more, verify that the mip level count is correct (in case it was changed here it might not
	// match the size...well actually it doesn't have to match but it can't be bigger than the size)
	unsigned int max_mip_level_count = 1;
	unsigned int w = 4;
	unsigned int h = 4;

	while (w < Width && h < Height)
	{
		w += w;
		h += h;
		max_mip_level_count++;
	}

	if (mip_level_count > max_mip_level_count)
	{
		mip_level_count = max_mip_level_count;
	}

#if defined(ZH_WW3D_CPU_ONLY)
	zh::original_runtime::OriginalGpuEdge::required().texture_creation_unavailable(
		Format, reducedWidth, reducedHeight, mip_level_count, Reduction);
#else
	D3DTexture	= DX8Wrapper::_Create_DX8_Texture
	(
		reducedWidth,
		reducedHeight,
		Format,
		(MipCountType)mip_level_count,
#ifdef USE_MANAGED_TEXTURES
		D3DPOOL_MANAGED
#else
		D3DPOOL_SYSTEMMEM
#endif
	);
#endif

	MipLevelCount = mip_level_count;

	return true;
}

bool TextureLoadTaskClass::Begin_Uncompressed_Load(void)
{
	unsigned width,height,depth,orig_mip_count,reduction;
	WW3DFormat orig_format;
	if (!Get_Texture_Information
		  (
				Texture->Get_Full_Path(),
				reduction,
				width,
				height,
				depth,
				orig_format,
				orig_mip_count,
				false
			)
		)
	{
		return false;
	}

	WW3DFormat src_format=orig_format;
	WW3DFormat dest_format=src_format;
	dest_format=Get_Valid_Texture_Format(dest_format,false);	// No compressed destination format if reading from targa...

   if (	src_format != WW3D_FORMAT_A8R8G8B8
		&&	src_format != WW3D_FORMAT_R8G8B8
		&&	src_format != WW3D_FORMAT_X8R8G8B8 )
	{
		WWDEBUG_SAY(("Invalid TGA format used in %s - only 24 and 32 bit formats should be used!\n", Texture->Get_Full_Path()));
	}

	// Destination size will be the next power of two square from the larger width and height...
	unsigned ow = width;
	unsigned oh = height;
	TextureLoader::Validate_Texture_Size(width, height,depth);
	if (width != ow || height != oh)
	{
		WWDEBUG_SAY(("Invalid texture size, scaling required. Texture: %s, size: %d x %d -> %d x %d\n", Texture->Get_Full_Path(), ow, oh, width, height));
	}

	Width		= width;
	Height	= height;
	Reduction = reduction;

	if (!Texture->Is_Reducible() || Texture->MipLevelCount == MIP_LEVELS_1)
		Reduction = 0;	//app doesn't want this texture to ever be reduced.
	else
	//Make sure we don't reduce below the level requested by the app
	if (Texture->MipLevelCount != MIP_LEVELS_ALL && (Texture->MipLevelCount - Reduction) < 1)
		Reduction = Texture->MipLevelCount - 1;

	//Another sanity check
	if (Reduction >= orig_mip_count)
		Reduction = 0;	//should not be possible to get here, but check just in case.

	if (Format == WW3D_FORMAT_UNKNOWN)
	{
		Format=dest_format;
	//	Format = Get_Valid_Texture_Format(dest_format, false); validated above
	}
	else
	{
		Format = Get_Valid_Texture_Format(Format, false);
	}

	int reducedWidth=Width;
	int reducedHeight=Height;
	int reducedMipCount=Texture->MipLevelCount;

	if (Reduction)
	{	//we don't care about specific levels so reduce them if needed.
		reducedWidth >>= Reduction;
		reducedHeight >>= Reduction;
		if (reducedMipCount != MIP_LEVELS_ALL)
			reducedMipCount -= Reduction;
	}

#if defined(ZH_WW3D_CPU_ONLY)
	zh::original_runtime::OriginalGpuEdge::required().texture_creation_unavailable(
		Format, reducedWidth, reducedHeight, reducedMipCount, Reduction);
#else
	D3DTexture = DX8Wrapper::_Create_DX8_Texture
	(
		reducedWidth,
		reducedHeight,
		Format,
		(MipCountType)reducedMipCount,
#ifdef USE_MANAGED_TEXTURES
		D3DPOOL_MANAGED
#else
		D3DPOOL_SYSTEMMEM
#endif
	);
#endif

	return true;
}
