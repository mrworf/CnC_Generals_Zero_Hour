#include "shader.h"
#include "dx8wrapper.h"
#include "vertmaterial.h"
#include "dx8fvf.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>
#include <limits>

#undef assert
#define assert(condition) do { if (!(condition)) std::abort(); } while (false)

int main()
{
	zh::renderer::RecordingGpuDevice device;
	{
		zh::original_runtime::OriginalGpuEdge edge(device);
		using Edge=zh::original_runtime::OriginalGpuEdge;
		assert(DX8Wrapper::Get_Texture_Op_Caps()==
			(D3DTEXOPCAPS_SELECTARG1|D3DTEXOPCAPS_MODULATE|D3DTEXOPCAPS_ADD));
		ShaderClass shader;
		shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
		DX8Wrapper::Set_Shader(shader);
		bool pending_snapshot=false;
		try { (void)DX8Wrapper::Snapshot_Source_State(); }
		catch (const std::runtime_error&) { pending_snapshot=true; }
		assert(pending_snapshot);
		DX8Wrapper::Apply_Render_State_Changes();
		const auto canonical=DX8Wrapper::Snapshot_Source_State();
		assert(canonical.stages[0].at(D3DTSS_COLOROP)==D3DTOP_MODULATE);
		assert(canonical.stages[1].at(D3DTSS_COLOROP)==D3DTOP_DISABLE);
		assert(canonical.render.at(D3DRS_ZFUNC)==D3DCMP_LESSEQUAL);
		bool no_material=false;
		try { (void)zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) { no_material=std::string(error.what()).find("material")!=std::string::npos; }
		assert(no_material);
		DX8Wrapper::Set_Material(nullptr);
		DX8Wrapper::Apply_Render_State_Changes();
		const auto original=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		assert(original.pipeline.vertex_layout==zh::renderer::VertexLayout::original_fvf);
		assert(original.pipeline.original_fvf.stride==32);
		assert(original.stages[0].color.op==zh::original_runtime::OriginalGpuEdge::CombinerOp::modulate);
		assert(original.stages[0].color.first==zh::original_runtime::OriginalGpuEdge::CombinerArg::texture);
		assert(original.stages[0].texture_required &&
			original.stages[1].color.op==zh::original_runtime::OriginalGpuEdge::CombinerOp::disable);
		assert((original.diffuse==std::array<float,4>{1,1,1,1}));
		const std::array<std::pair<unsigned,Edge::CombinerOp>,5> all_ops{{
			{D3DTOP_DISABLE,Edge::CombinerOp::disable},
			{D3DTOP_SELECTARG1,Edge::CombinerOp::select_first},
			{D3DTOP_SELECTARG2,Edge::CombinerOp::select_second},
			{D3DTOP_MODULATE,Edge::CombinerOp::modulate},
			{D3DTOP_ADD,Edge::CombinerOp::add},
		}};
		for (const auto& [source_op,mapped_op]:all_ops) {
			DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,source_op);
			DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAOP,source_op);
			for (unsigned source_arg : {D3DTA_DIFFUSE,D3DTA_CURRENT,D3DTA_TEXTURE}) {
				DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLORARG1,source_arg);
				DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLORARG2,source_arg);
				DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAARG1,source_arg);
				DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAARG2,source_arg);
				const auto mapped=Edge::map_applied_state(DX8_FVF_XYZNUV1);
				assert(mapped.stages[0].color.op==mapped_op && mapped.stages[0].alpha.op==mapped_op);
				if (source_op!=D3DTOP_DISABLE) {
					const auto arg=static_cast<Edge::CombinerArg>(source_arg);
					assert(mapped.stages[0].color.first==arg && mapped.stages[0].alpha.first==arg);
					if (source_op!=D3DTOP_SELECTARG1)
						assert(mapped.stages[0].color.second==arg && mapped.stages[0].alpha.second==arg);
				}
			}
		}
		ShaderClass::Invalidate();
		DX8Wrapper::Set_Shader(shader);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC,99);
		bool bad_depth=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) { bad_depth=std::string(error.what()).find("comparison")!=std::string::npos; }
		assert(bad_depth);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE,99);
		bool bad_cull=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) { bad_cull=std::string(error.what()).find("cull")!=std::string::npos; }
		assert(bad_cull);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE,D3DCULL_CW);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING,1);
		assert(Edge::map_applied_state(DX8_FVF_XYZNUV1).lighting);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING,0);
		D3DMATERIAL8 invalid_material{};
		invalid_material.Power=std::numeric_limits<float>::quiet_NaN();
		DX8Wrapper::Set_DX8_Material(&invalid_material);
		bool bad_material=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) { bad_material=std::string(error.what()).find("material power")!=std::string::npos; }
		assert(bad_material);
		DX8Wrapper::Set_Material(nullptr);
		DX8Wrapper::Apply_Render_State_Changes();
		const std::string initial=device.snapshot();
		assert(initial.find("Set_DX8_Texture_Stage_State=0:1:4")!=std::string::npos);
		assert(initial.find("Set_DX8_Texture_Stage_State=1:1:1")!=std::string::npos);
		assert(initial.find("Set_DX8_Render_State=23:4")!=std::string::npos);
		assert(initial.find("Set_DX8_Render_State=22:2")!=std::string::npos);
		assert(!device.pass_active() && DX8Wrapper::Pending_Changes()==0);
		DX8Wrapper::Set_Shader(shader);
		assert(DX8Wrapper::Pending_Changes()==0 && device.snapshot()==initial);

		DX8Wrapper::Set_Fog(true,Vector3(0.1f,0.2f,0.3f),5.0f,80.0f);
		bool invalidated=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) { invalidated=std::string(error.what()).find("pending source application")!=std::string::npos; }
		assert(invalidated);
		shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
		shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
		shader.Set_Alpha_Test(ShaderClass::ALPHATEST_ENABLE);
		shader.Set_Fog_Func(ShaderClass::FOG_WHITE);
		shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
		shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
		shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_ADD);
		shader.Set_Post_Detail_Alpha_Func(ShaderClass::DETAILALPHA_SCALE);
		DX8Wrapper::Set_Shader(shader);
		DX8Wrapper::Apply_Render_State_Changes();
		const std::string selected=device.snapshot();
		assert(selected.find("Set_DX8_Render_State=19:5")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=20:6")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=27:1")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=24:96")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=28:1")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=34:16777215")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=14:0")!=std::string::npos);
		assert(selected.find("Set_DX8_Render_State=22:1")!=std::string::npos);
		assert(selected.find("Set_DX8_Texture_Stage_State=1:1:7")!=std::string::npos);
		assert(selected.find("Set_DX8_Texture_Stage_State=1:4:4")!=std::string::npos);
		const auto selected_state=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV2);
		assert(selected_state.pipeline.blend.enabled &&
			selected_state.pipeline.blend.source_color==zh::renderer::BlendFactor::src_alpha &&
			selected_state.pipeline.blend.destination_color==zh::renderer::BlendFactor::inv_src_alpha);
		assert(selected_state.alpha_test && selected_state.alpha_reference==96.0f/255.0f &&
			selected_state.alpha_compare==zh::renderer::CompareOp::greater_equal);
		assert(!selected_state.pipeline.depth_stencil.depth_write &&
			selected_state.pipeline.raster.cull==zh::renderer::CullMode::none);
		assert(selected_state.pipeline.fog_enabled && selected_state.fog_start==5.0f &&
			selected_state.fog_end==80.0f && selected_state.fog_color[0]==1.0f);
		assert(selected_state.stages[1].color.op==Edge::CombinerOp::add &&
			selected_state.stages[1].alpha.op==Edge::CombinerOp::modulate);
		bool missing_uv=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) { missing_uv=std::string(error.what()).find("source UV")!=std::string::npos; }
		assert(missing_uv);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,99);
		bool bad_blend=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV2); }
		catch (const std::runtime_error& error) { bad_blend=std::string(error.what()).find("blend factor")!=std::string::npos; }
		assert(bad_blend);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		DX8Wrapper::Set_Transform(D3DTS_TEXTURE0,Matrix4x4(true));
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
		const auto transformed=Edge::map_applied_state(DX8_FVF_XYZNUV2);
		assert(transformed.stages[0].transform_set &&
			transformed.stages[0].transform_flags==D3DTTFF_COUNT2 &&
			transformed.stages[0].transform[0]==1.0f && transformed.stages[0].transform[5]==1.0f);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGEND,0x7fc00000U);
		bool bad_fog=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV2); }
		catch (const std::runtime_error& error) { bad_fog=std::string(error.what()).find("fog range")!=std::string::npos; }
		assert(bad_fog);
		DX8Wrapper::Set_Fog(true,Vector3(0.1f,0.2f,0.3f),5.0f,80.0f);

		shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_ADDSIGNED);
		shader.Set_Post_Detail_Alpha_Func(ShaderClass::DETAILALPHA_DISABLE);
		DX8Wrapper::Set_Shader(shader);
		DX8Wrapper::Apply_Render_State_Changes();
		assert(device.snapshot().find("Set_DX8_Texture_Stage_State=1:1:7")!=std::string::npos);
		shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_SCALE2X);
		shader.Set_Primary_Gradient(ShaderClass::GRADIENT_ADD);
		shader.Set_Fog_Func(ShaderClass::FOG_SCALE_FRAGMENT);
		shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
		DX8Wrapper::Set_Shader(shader);
		DX8Wrapper::Apply_Render_State_Changes();
		const std::string fallback=device.snapshot();
		assert(fallback.find("Set_DX8_Texture_Stage_State=0:1:7")!=std::string::npos);
		assert(fallback.find("Set_DX8_Texture_Stage_State=1:1:4")!=std::string::npos);
		assert(fallback.find("Set_DX8_Render_State=34:0")!=std::string::npos);
		assert(fallback.find("Set_DX8_Render_State=23:8")!=std::string::npos);
		const auto fallback_state=Edge::map_applied_state(DX8_FVF_XYZNUV2);
		assert(fallback_state.stages[0].color.op==Edge::CombinerOp::add &&
			fallback_state.stages[1].color.op==Edge::CombinerOp::modulate &&
			fallback_state.pipeline.depth_stencil.depth_compare==zh::renderer::CompareOp::always &&
			fallback_state.fog_color[0]==0.0f);

		shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_SUB);
		DX8Wrapper::Set_Shader(shader);
		bool rejected=false;
		try { DX8Wrapper::Apply_Render_State_Changes(); }
		catch (const std::runtime_error& error) {
			rejected=std::string(error.what()).find("SUBTRACT unsupported")!=std::string::npos;
		}
		assert(rejected && (DX8Wrapper::Pending_Changes()&(1U<<9)));
		bool pending_reject=false;
		try { (void)Edge::map_applied_state(DX8_FVF_XYZNUV2); }
		catch (const std::runtime_error& error) {
			pending_reject=std::string(error.what()).find("pending source application")!=std::string::npos;
		}
		assert(pending_reject);
		shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_SCALE);
		DX8Wrapper::Set_Shader(shader);
		DX8Wrapper::Apply_Render_State_Changes();
		assert(DX8Wrapper::Pending_Changes()==0 && !device.pass_active());

		shader.Set_Post_Detail_Alpha_Func(ShaderClass::DETAILALPHA_INVSCALE);
		DX8Wrapper::Set_Shader(shader);
		rejected=false;
		try { DX8Wrapper::Apply_Render_State_Changes(); }
		catch (const std::runtime_error& error) {
			rejected=std::string(error.what()).find("ADDSMOOTH unsupported")!=std::string::npos;
		}
		assert(rejected && (DX8Wrapper::Pending_Changes()&(1U<<9)));
		shader.Set_Post_Detail_Alpha_Func(ShaderClass::DETAILALPHA_DISABLE);
		DX8Wrapper::Set_Shader(shader);
		DX8Wrapper::Apply_Render_State_Changes();
		assert(DX8Wrapper::Pending_Changes()==0);
		shader.Set_NPatch_Enable(ShaderClass::NPATCH_ENABLE);
		DX8Wrapper::Set_Shader(shader);
		rejected=false;
		try { DX8Wrapper::Apply_Render_State_Changes(); }
		catch (const std::runtime_error& error) {
			rejected=std::string(error.what()).find("NPATCH")!=std::string::npos;
		}
		assert(rejected && (DX8Wrapper::Pending_Changes()&(1U<<9)));
		shader.Set_NPatch_Enable(ShaderClass::NPATCH_DISABLE);
		shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE2X);
		DX8Wrapper::Set_Shader(shader);
		rejected=false;
		try { DX8Wrapper::Apply_Render_State_Changes(); }
		catch (const std::runtime_error& error) {
			rejected=std::string(error.what()).find("MODULATE2X")!=std::string::npos;
		}
		assert(rejected && (DX8Wrapper::Pending_Changes()&(1U<<9)));
	}
	assert(device.resource_counts().total()==0);
	bool stale_state=false;
	try { (void)zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1); }
	catch (const std::runtime_error& error) { stale_state=std::string(error.what()).find("session")!=std::string::npos; }
	assert(stale_state);
	std::puts("original ShaderClass software-profile source decisions and negative/retry: ok");
}
