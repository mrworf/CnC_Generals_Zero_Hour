#include "PreRTS.h"
#include "volume_stencil_contract.h"
#include "zh/renderer/recording_device.h"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

using namespace zh::renderer;
namespace { void require(bool v,const char *m) { if(!v) throw std::runtime_error(m); }
template <typename F> bool rejected(F f) { try { f(); } catch (const std::runtime_error &) { return true; } return false; } }
extern "C" void zh_probe_volumetric_stencil_contract()
{
	require(std::getenv("ZH_M22_VOLUME_STENCIL_PROFILE"),"volume stencil profile missing");
	for (int generation=0; generation!=2; ++generation) {
		RecordingGpuDevice device;
		auto vs=device.create_shader({ShaderStage::vertex,"generated-volume",0,0},"volume vertex");
		auto fs=device.create_shader({ShaderStage::fragment,"generated-volume",0,0},"volume fragment");
		auto protocol=zh::original_runtime::make_volume_stencil_protocol(vs,fs,TextureFormat::rgba8,0x80);
		require(rejected([&] { (void)zh::original_runtime::make_volume_stencil_protocol(vs,fs,TextureFormat::depth24_stencil8,0x80); }),
			"volume stencil accepted a depth-only composite target");
		require(protocol.increment.blend.color_write_mask==0 && protocol.increment.raster.cull==CullMode::clockwise &&
			protocol.increment.depth_stencil.depth_pass==StencilOp::increment_clamp &&
			protocol.decrement.raster.cull==CullMode::counter_clockwise && protocol.decrement.depth_stencil.depth_pass==StencilOp::decrement_clamp &&
			protocol.composite.blend.enabled && protocol.composite.blend.source_color==BlendFactor::dst_color &&
			protocol.composite.depth_stencil.stencil_compare==CompareOp::less_equal,"volume stencil state drifted");
		device.fail_next_pipeline_create(); require(!device.create_pipeline(PipelineKey(protocol.increment),"volume increment failure"),"volume pipeline failure missing");
		auto inc=device.create_pipeline(PipelineKey(protocol.increment),"volume increment");
		auto dec=device.create_pipeline(PipelineKey(protocol.decrement),"volume decrement");
		auto cmp=device.create_pipeline(PipelineKey(protocol.composite),"volume composite");
		device.fail_next_buffer_create(); require(!device.create_buffer({64,BufferUsage::vertex,true},"volume buffer failure"),"volume buffer failure missing");
		auto vb=device.create_buffer({64,BufferUsage::vertex,true},"volume vertices");
		TextureDesc td; td.width=8; td.height=8; td.render_target=true;
		auto color=device.create_texture(td,"volume color"); td.format=TextureFormat::depth24_stencil8; td.sampled=false;
		device.set_texture_format_supported(TextureFormat::depth24_stencil8,false);
		require(!device.create_texture(td,"volume unsupported depth"),"volume accepted unsupported D24S8");
		device.set_texture_format_supported(TextureFormat::depth24_stencil8,true);
		auto depth=device.create_texture(td,"volume depth");
		std::array<unsigned char,64> bytes{}; device.fail_next_buffer_upload();
		require(!device.upload({vb,64,0,64},bytes.data()),"volume upload failure missing");
		require(device.upload({vb,64,0,64},bytes.data()),"volume upload retry failed");
		RenderPassDesc pass; pass.color_targets[0]=color; pass.color_target_count=1; pass.depth_target=depth; pass.width=8; pass.height=8;
		DrawDesc draw; draw.vertex_buffer=vb; draw.vertex_or_index_count=3; draw.pipeline=inc;
		require(!device.draw(draw),"volume accepted a draw outside its pass");
		require(device.begin_pass(pass,"volume stencil pass"),"volume pass failed");
		device.fail_next_draw(); require(!device.draw(draw),"volume draw failure missing");
		require(device.draw(draw),"volume increment draw failed"); draw.pipeline=dec; require(device.draw(draw),"volume decrement draw failed"); draw.pipeline=cmp; require(device.draw(draw),"volume composite draw failed"); require(device.end_pass(),"volume pass end failed");
		require(device.operation_counts().draws==3,"volume draw ordering missing");
		const auto trace=device.snapshot();
		const auto begin=trace.find("begin_pass label=\"volume stencil pass\"");
		const auto increment=trace.find("draw pipeline=P1");
		const auto decrement=trace.find("draw pipeline=P2");
		const auto composite=trace.find("draw pipeline=P3");
		const auto end=trace.find("end_pass");
		require(begin!=std::string::npos && increment>begin && decrement>increment && composite>decrement && end>composite,
			"volume stencil Recording order drifted");
		device.destroy(depth); device.destroy(color); device.destroy(vb); device.destroy(cmp); device.destroy(dec); device.destroy(inc); device.destroy(fs); device.destroy(vs);
		require(!device.resource_counts().total(),"volume stencil resources leaked");
	}
	std::puts("original volume stencil: public-state=1 retries=1 generations=2 resources=0");
}
