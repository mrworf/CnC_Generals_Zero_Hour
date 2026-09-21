void TextureClass::Apply(unsigned int stage)
{
	// Initialization needs to be done when texture is used if it hasn't been done before.
	if (!Initialized)
	{
		Init();
	}
	LastAccessed=WW3D::Get_Sync_Time();

#if defined(ZH_WW3D_CPU_ONLY)
	// The source still selects enabled/null and filter order; only the
	// physical DX8 texture assignment becomes pending public-GPU state.
	zh::original_runtime::OriginalGpuEdge::required().select_texture(stage,
		WW3D::Is_Texturing_Enabled() ? this : nullptr);
#else
	DX8_RECORD_TEXTURE(this);
	if (WW3D::Is_Texturing_Enabled())
	{
		DX8Wrapper::Set_DX8_Texture(stage, Peek_D3D_Base_Texture());
	}
	else
	{
		DX8Wrapper::Set_DX8_Texture(stage, NULL);
	}
#endif
	Filter.Apply(stage);
}
