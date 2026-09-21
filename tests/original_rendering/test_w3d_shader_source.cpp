#include "shader.h"
#include "dx8wrapper.h"
#include "vertmaterial.h"
#include "lightenvironment.h"
#include "matrix3d.h"
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
#include <cstring>

#undef assert
#define assert(condition) do { if (!(condition)) { std::fprintf(stderr,"original shader source invariant %s:%d: %s\n",__FILE__,__LINE__,#condition); std::abort(); } } while (false)

int main()
{
	zh::renderer::RecordingGpuDevice device;
	zh::original_runtime::OriginalGpuEdge::PhysicalState retired;
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
		ShaderClass untextured=shader;
		untextured.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
		DX8Wrapper::Set_Shader(untextured);
		DX8Wrapper::Apply_Render_State_Changes();
		bool missing_world=false;
		try { (void)edge.prepare_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) {
			missing_world=std::string(error.what()).find("world/view/projection")!=std::string::npos;
		}
		assert(missing_world && device.resource_counts().total()==0);
		DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
		DX8Wrapper::Apply_Render_State_Changes();
		const auto base_pipeline=Edge::map_applied_state(DX8_FVF_XYZNUV1).pipeline;
		const auto physical=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		assert(device.pipeline_descriptor(physical.pipeline).raster.depth_bias==0.0f);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,8);
		const auto biased=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		assert(device.pipeline_descriptor(biased.pipeline).raster.depth_bias==-8.0f &&
			DX8Wrapper::Snapshot_Source_State().render.at(D3DRS_ZBIAS)==8);
		bool unsupported_bias=false;
		try { DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,7); }
		catch (const std::runtime_error&) { unsupported_bias=true; }
		assert(unsupported_bias && DX8Wrapper::Snapshot_Source_State().render.at(D3DRS_ZBIAS)==8);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,0);
		const auto unbiased=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		assert(device.pipeline_descriptor(unbiased.pipeline).raster.depth_bias==0.0f &&
			zh::renderer::PipelineKey(
				zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1).pipeline)==
			zh::renderer::PipelineKey(base_pipeline));
		retired=physical;
		bool no_variant=false;
		try { (void)edge.prepare_applied_state(DX8_FVF_XYZNDUV1); }
		catch (const std::runtime_error& error) {
			no_variant=std::string(error.what()).find("exact shader input variant")!=std::string::npos;
		}
		assert(no_variant && device.resource_counts().total()==0);
		const auto variant_retry=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		retired=variant_retry;
		assert(variant_retry.texture_mask==0 && variant_retry.fragment_bindings.texture_count==0 &&
			variant_retry.vertex_bindings.uniform_count==1 && variant_retry.fragment_bindings.uniform_count==1);
		edge.validate_prepared_state(variant_retry);
		auto forged_binding=variant_retry;
		forged_binding.vertex_bindings.uniforms[0].size=4;
		bool rejected_binding=false;
		try { edge.validate_prepared_state(forged_binding); }
		catch (const std::runtime_error&) { rejected_binding=true; }
		assert(rejected_binding);
		const auto pipeline=device.pipeline_descriptor(variant_retry.pipeline);
		assert(pipeline.vertex_layout==zh::renderer::VertexLayout::original_fvf &&
			pipeline.original_fvf.stride==32 && !pipeline.blend.enabled &&
			pipeline.depth_stencil.depth_write);
		const auto vertex_bytes=device.buffer_bytes(variant_retry.vertex_bindings.uniforms[0].buffer);
		const auto fragment_bytes=device.buffer_bytes(variant_retry.fragment_bindings.uniforms[0].buffer);
		assert(vertex_bytes.size()==sizeof(Edge::VertexUniform) &&
			fragment_bytes.size()==sizeof(Edge::FragmentUniform));
		Edge::VertexUniform vertex_uniform{};
		Edge::FragmentUniform fragment_uniform{};
		std::memcpy(&vertex_uniform,vertex_bytes.data(),vertex_bytes.size());
		std::memcpy(&fragment_uniform,fragment_bytes.data(),fragment_bytes.size());
		assert(vertex_uniform.world[0]==1.0f && vertex_uniform.view[5]==1.0f &&
			vertex_uniform.projection[10]==1.0f && fragment_uniform.diffuse[0]==1.0f &&
			fragment_uniform.stage_ops[0][0]==static_cast<int>(Edge::CombinerOp::select_second));
		assert(device.snapshot().find("begin_pass")==std::string::npos &&
			device.snapshot().find("draw ")==std::string::npos);
		for (unsigned failure=0;failure<4;++failure) {
			switch (failure) {
			case 0: device.fail_next_shader_create(); break;
			case 1: device.fail_next_pipeline_create(); break;
			case 2: device.fail_next_buffer_create(); break;
			case 3: device.fail_next_buffer_upload(); break;
			}
			bool rejected=false;
			try { (void)edge.prepare_applied_state(DX8_FVF_XYZNUV1); }
			catch (const std::runtime_error&) { rejected=true; }
			assert(rejected && device.resource_counts().total()==0);
		}
		const auto replay=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		edge.validate_prepared_state(replay);
		bool superseded=false;
		try { edge.validate_prepared_state(variant_retry); }
		catch (const std::runtime_error&) { superseded=true; }
		assert(superseded);
		DX8Wrapper::Set_Shader(shader);
		bool invalid_physical=false;
		try { edge.validate_prepared_state(replay); }
		catch (const std::runtime_error&) { invalid_physical=true; }
		assert(invalid_physical);
		DX8Wrapper::Apply_Render_State_Changes();
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
		bool lit_physical=false;
		try { (void)edge.prepare_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) {
			lit_physical=std::string(error.what()).find("selected source light environment")!=std::string::npos;
		}
		assert(lit_physical && device.resource_counts().total()>0);
		DX8Wrapper::Set_Shader(untextured);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING,1);
		LightEnvironmentClass lit_environment;
		lit_environment.Reset(Vector3(0,0,0),Vector3(0.1f,0.2f,0.3f));
		lit_environment.Pre_Render_Update(Matrix3D(true));
		DX8Wrapper::Set_Light_Environment(&lit_environment);
		device.fail_next_shader_create();
		bool lit_create_failure=false;
		try { (void)edge.prepare_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) {
			lit_create_failure=std::string(error.what()).find("shader creation failed")!=std::string::npos;
		}
		assert(lit_create_failure && device.resource_counts().total()==0 &&
			DX8Wrapper::Snapshot_Source_State().light_environment_selected);
		const auto lit_retry=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		edge.validate_prepared_state(lit_retry);
		assert(device.resource_counts().total()>0);
		const auto skin_layout=Edge::layout_for_fvf(DX8_FVF_XYZNDUV2);
		const FVFInfoClass source_skin_layout(DX8_FVF_XYZNDUV2);
		assert(skin_layout.stride==sizeof(VertexFormatXYZNDUV2) &&
			skin_layout.attribute_count==5 &&
			skin_layout.attributes[1].offset==source_skin_layout.Get_Normal_Offset() &&
			skin_layout.attributes[2].offset==source_skin_layout.Get_Diffuse_Offset() &&
			skin_layout.attributes[3].offset==source_skin_layout.Get_Tex_Offset(0) &&
			skin_layout.attributes[4].offset==source_skin_layout.Get_Tex_Offset(1));
		auto* skin_material=NEW_REF(VertexMaterialClass,());
		skin_material->Set_Lighting(true);
		skin_material->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
		skin_material->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
		skin_material->Set_Emissive_Color_Source(VertexMaterialClass::COLOR2);
		DX8Wrapper::Set_Material(skin_material);
		skin_material->Release_Ref();
		DX8Wrapper::Apply_Render_State_Changes();
		const auto skin_state=edge.prepare_applied_state(DX8_FVF_XYZNDUV2);
		edge.validate_prepared_state(skin_state);
		const auto skin_pipeline=device.pipeline_descriptor(skin_state.pipeline);
		assert(skin_pipeline.original_fvf.stride==skin_layout.stride &&
			skin_pipeline.original_fvf.attribute_count==5);
		const auto skin_uniform_bytes=device.buffer_bytes(skin_state.vertex_bindings.uniforms[0].buffer);
		Edge::VertexUniform skin_uniform{};
		std::memcpy(&skin_uniform,skin_uniform_bytes.data(),skin_uniform_bytes.size());
		assert((skin_uniform.lit_material_sources==std::array<std::int32_t,4>{1,1,0,2}) &&
			skin_uniform.lit_switches[3]==1);
		const auto skin_source=DX8Wrapper::Snapshot_Source_State();
		assert(skin_source.render.at(D3DRS_AMBIENTMATERIALSOURCE)==D3DMCS_COLOR1 &&
			skin_source.render.at(D3DRS_DIFFUSEMATERIALSOURCE)==D3DMCS_COLOR1);
		bool invalid_skin_source=false;
		try { DX8Wrapper::Set_DX8_Render_State(D3DRS_DIFFUSEMATERIALSOURCE,99); }
		catch (const std::runtime_error& error) {
			invalid_skin_source=std::string(error.what()).find("unsupported")!=std::string::npos;
		}
		assert(invalid_skin_source);
		const auto skin_retry=edge.prepare_applied_state(DX8_FVF_XYZNDUV2);
		edge.validate_prepared_state(skin_retry);
		bool unsupported_lit_fvf=false;
		try { (void)edge.prepare_applied_state(DX8_FVF_XYZNDUV1); }
		catch (const std::runtime_error& error) {
			unsupported_lit_fvf=std::string(error.what()).find("exact shader input variant")!=std::string::npos;
		}
		assert(unsupported_lit_fvf && device.resource_counts().total()>0);
		edge.validate_prepared_state(skin_retry);
		device.fail_next_buffer_upload();
		bool lit_upload_failure=false;
		try { (void)edge.prepare_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error& error) {
			lit_upload_failure=std::string(error.what()).find("uniform upload failed")!=std::string::npos;
		}
		assert(lit_upload_failure && device.resource_counts().total()==0);
		const auto lit_upload_retry=edge.prepare_applied_state(DX8_FVF_XYZNUV1);
		edge.validate_prepared_state(lit_upload_retry);
		DX8Wrapper::Set_Material(nullptr);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_Light_Environment(nullptr);
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
		DX8Wrapper::Set_Shader(shader);
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
	{
		zh::original_runtime::OriginalGpuEdge recreated(device);
		bool wrong_generation=false;
		try { recreated.validate_prepared_state(retired); }
		catch (const std::runtime_error&) { wrong_generation=true; }
		assert(wrong_generation && device.resource_counts().total()==0);
	}
	std::puts("original ShaderClass software-profile source decisions and negative/retry: ok");
}
