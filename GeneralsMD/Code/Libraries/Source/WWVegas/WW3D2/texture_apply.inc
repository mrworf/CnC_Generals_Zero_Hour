void TextureClass::Apply(unsigned int stage)
{
#if defined(ZH_WW3D_CPU_ONLY)
	zh::original_runtime::OriginalGpuEdge::SourceMutationGuard mutation{zh::original_runtime::OriginalGpuEdge::active()};
	if (auto* edge=zh::original_runtime::OriginalGpuEdge::active()) edge->guard_source_stage(stage,this,true);
	if (!Filter.Can_Apply(stage))
	{
		if (auto* edge=zh::original_runtime::OriginalGpuEdge::active()) edge->poison_source_stages();
		throw std::runtime_error("original texture filter or stage is invalid");
	}
#endif
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
