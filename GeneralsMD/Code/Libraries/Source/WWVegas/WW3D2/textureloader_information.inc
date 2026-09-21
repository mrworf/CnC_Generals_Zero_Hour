// Original TextureLoader::Get_Texture_Information decision body shared by
// the native and Linux device configurations of textureloader.cpp.
static bool Get_Texture_Information
(
	const char* filename,
	unsigned& reduction,
	unsigned& w,
	unsigned& h,
	unsigned& d,
	WW3DFormat& format,
	unsigned& mip_count,
	bool compressed
)
{
#if defined(ZH_WW3D_CPU_ONLY)
	// The production W3DDisplay explicitly disables thumbnail loading. Its
	// Win32 mix-file manager is not initialized here; fail if another caller
	// enables that as-yet-unported physical/file-system route.
	if (WW3D::Get_Thumbnail_Enabled())
		throw std::runtime_error("original thumbnail manager requires an unavailable Win32 mix-file route");
	ThumbnailClass* thumb=nullptr;
#else
	ThumbnailClass* thumb=ThumbnailManagerClass::Peek_Thumbnail_Instance_From_Any_Manager(filename);
#endif

	if (!thumb)
	{
		if (compressed)
		{
			DDSFileClass dds_file(filename, 0);
			if (!dds_file.Is_Available()) return false;

			// Destination size will be the next power of two square from the larger width and height...
			w = dds_file.Get_Width(0);
			h = dds_file.Get_Height(0);
			d = dds_file.Get_Depth(0);
			format = dds_file.Get_Format();
			mip_count = dds_file.Get_Mip_Level_Count();
			//Figure out correct reduction
			int reqReduction=WW3D::Get_Texture_Reduction();	//requested reduction

			if (reqReduction >= mip_count)
				reqReduction=mip_count-1;	//leave only the lowest level

			//Clamp reduction
			int curReduction=0;
			int curWidth=w;
			int curHeight=h;
			int minDim=WW3D::Get_Texture_Min_Dimension();

			while (curReduction < reqReduction && curWidth > minDim && curHeight > minDim)
			{	curWidth >>=1;	//keep dividing
				curHeight >>=1;
				curReduction++;
			}
			reduction=curReduction;
			return true;
		}

		Targa targa;
		if (TARGA_ERROR_HANDLER(targa.Open(filename, TGA_READMODE), filename))
		{
			return false;
		}

		unsigned int bpp;
		WW3DFormat dest_format;
		Get_WW3D_Format(dest_format,format,bpp,targa);

		mip_count = 0;

		//Figure out how many mip levels this texture will occupy
		for (int i=targa.Header.Width, j=targa.Header.Height; i > 0 && j > 0; i>>=1, j>>=1)
				mip_count++;

		//Figure out correct reduction
		int reqReduction=WW3D::Get_Texture_Reduction();	//requested reduction

		if (reqReduction >= mip_count)
			reqReduction=mip_count-1;	//leave only the lowest level

		//Clamp reduction
		int curReduction=0;
		int curWidth=targa.Header.Width;
		int curHeight=targa.Header.Height;
		int minDim=WW3D::Get_Texture_Min_Dimension();

		while (curReduction < reqReduction && curWidth > minDim && curHeight > minDim)
		{	curWidth >>=1;	//keep dividing
			curHeight >>=1;
			curReduction++;
		}
		reduction=curReduction;

		// Destination size will be the next power of two square from the larger width and height...
		w = targa.Header.Width;
		h = targa.Header.Height;
		d = 1;
		return true;
	}

#if !defined(ZH_WW3D_CPU_ONLY)
	if (compressed &&
		thumb->Get_Original_Texture_Format()!=WW3D_FORMAT_DXT1 &&
		thumb->Get_Original_Texture_Format()!=WW3D_FORMAT_DXT2 &&
		thumb->Get_Original_Texture_Format()!=WW3D_FORMAT_DXT3 &&
		thumb->Get_Original_Texture_Format()!=WW3D_FORMAT_DXT4 &&
		thumb->Get_Original_Texture_Format()!=WW3D_FORMAT_DXT5) {
		return false;
	}

	w=thumb->Get_Original_Texture_Width() >> reduction;
	h=thumb->Get_Original_Texture_Height() >> reduction;
	//d=thumb->Get_Original_Texture_Depth() >> reduction; // need to a volume texture support to thumbnails...maybe
	mip_count=thumb->Get_Original_Texture_Mip_Level_Count();
	format=thumb->Get_Original_Texture_Format();
	return true;
#else
	return false; // The production no-thumbnail route returned above.
#endif
}
