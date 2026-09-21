#include "shader.h"
#include "dx8wrapper.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <string>

#undef assert
#define assert(condition) do { if (!(condition)) std::abort(); } while (false)

int main()
{
	zh::renderer::RecordingGpuDevice device;
	{
		zh::original_runtime::OriginalGpuEdge edge(device);
		assert(DX8Wrapper::Get_Texture_Op_Caps()==
			(D3DTEXOPCAPS_SELECTARG1|D3DTEXOPCAPS_MODULATE|D3DTEXOPCAPS_ADD));
		ShaderClass shader;
		shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
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

		shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_SUB);
		DX8Wrapper::Set_Shader(shader);
		bool rejected=false;
		try { DX8Wrapper::Apply_Render_State_Changes(); }
		catch (const std::runtime_error& error) {
			rejected=std::string(error.what()).find("SUBTRACT unsupported")!=std::string::npos;
		}
		assert(rejected && (DX8Wrapper::Pending_Changes()&(1U<<9)));
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
	std::puts("original ShaderClass software-profile source decisions and negative/retry: ok");
}
