#include "Common/GlobalData.h"
#include "WW3D2/dx8wrapper.h"
#include "original_gpu_edge.h"

#include <stdexcept>

namespace {
bool shader_manager_initialized = false;

void require_shader_manager()
{
	if (!shader_manager_initialized)
		throw std::runtime_error("original terrain shader manager is not initialized");
}

void clear_terrain_pass(bool invalidate_shader)
{
	auto *edge = zh::original_runtime::OriginalGpuEdge::active();
	if (!edge) return;
	edge->select_texture(0, nullptr);
	edge->select_texture(1, nullptr);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_TEXCOORDINDEX, 1);
	if (invalidate_shader) ShaderClass::Invalidate();
}

void select_filter(zh::original_runtime::OriginalGpuEdge &edge, unsigned stage)
{
	constexpr unsigned point = 1;
	constexpr unsigned linear_filter = 2;
	constexpr unsigned clamp = 1;
	const bool linear = TheGlobalData &&
		(TheGlobalData->m_bilinearTerrainTex || TheGlobalData->m_trilinearTerrainTex);
	const bool trilinear = TheGlobalData && TheGlobalData->m_trilinearTerrainTex;
	using State = zh::original_runtime::OriginalGpuEdge::FilterStageState;
	edge.set_filter_stage_state(stage, State::min_filter, linear ? linear_filter : point);
	edge.set_filter_stage_state(stage, State::mag_filter, linear ? linear_filter : point);
	edge.set_filter_stage_state(stage, State::mip_filter,
		trilinear || stage == 1 ? linear_filter : point);
	edge.set_filter_stage_state(stage, State::address_u, clamp);
	edge.set_filter_stage_state(stage, State::address_v, clamp);
}
}

TextureClass *W3DShaderManager::m_Textures[8]{};
W3DShaderManager::ShaderTypes W3DShaderManager::m_currentShader = W3DShaderManager::ST_INVALID;
Int W3DShaderManager::m_currentShaderPass = -1;

void W3DShaderManager::init()
{
	if (shader_manager_initialized)
		throw std::runtime_error("original terrain shader manager duplicate init");
	if (!zh::original_runtime::OriginalGpuEdge::active())
		throw std::runtime_error("original terrain shader manager requires a GPU translation session");
	for (auto &texture : m_Textures) texture = nullptr;
	m_currentShader = ST_INVALID;
	m_currentShaderPass = -1;
	shader_manager_initialized = true;
}

void W3DShaderManager::shutdown()
{
	if (!shader_manager_initialized) return;
	if (m_currentShader != ST_INVALID)
		throw std::runtime_error("original terrain shader manager shutdown requires an idle pass");
	clear_terrain_pass(true);
	for (auto &texture : m_Textures) texture = nullptr;
	m_currentShaderPass = -1;
	shader_manager_initialized = false;
}

Int W3DShaderManager::getShaderPasses(ShaderTypes shader)
{
	require_shader_manager();
	if (shader != ST_TERRAIN_BASE)
		throw std::runtime_error("original terrain shader kind is unsupported by Linux minimum profile");
	return 2;
}

Int W3DShaderManager::setShader(ShaderTypes shader, Int pass)
{
	require_shader_manager();
	if (shader != ST_TERRAIN_BASE)
		throw std::runtime_error("original terrain shader kind is unsupported by Linux minimum profile");
	if (pass < 0 || pass >= 2)
		throw std::runtime_error("original terrain shader pass is outside the two-pass base route");
	if (shader == m_currentShader && pass == m_currentShaderPass) return TRUE;
	TextureClass *selected = m_Textures[pass];
	if (!m_Textures[0] || !m_Textures[1] || !selected)
		throw std::runtime_error("original terrain shader atlas pair is incomplete");
	auto &edge = zh::original_runtime::OriginalGpuEdge::required();
	// Validate both published owners before changing delayed source state.
	(void)edge.texture_handle(m_Textures[0]);
	(void)edge.texture_handle(m_Textures[1]);
	DX8Wrapper::Apply_Render_State_Changes();
	try {
		edge.select_texture(0, selected);
		edge.select_texture(1, nullptr);
		select_filter(edge, 0);
		select_filter(edge, 1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP,
			pass == 0 ? D3DTOP_DISABLE : D3DTOP_MODULATE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, pass);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, pass != 0);
		if (pass != 0) {
			DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
	} catch (...) {
		clear_terrain_pass(false);
		m_currentShader = ST_INVALID;
		m_currentShaderPass = -1;
		throw;
	}
	m_currentShader = shader;
	m_currentShaderPass = pass;
	return TRUE;
}

void W3DShaderManager::resetShader(ShaderTypes shader)
{
	require_shader_manager();
	if (shader != ST_TERRAIN_BASE)
		throw std::runtime_error("original terrain shader reset kind is unsupported");
	if (m_currentShader == ST_INVALID) return;
	if (m_currentShader != shader)
		throw std::runtime_error("original terrain shader reset does not match active pass");
	clear_terrain_pass(true);
	m_currentShader = ST_INVALID;
	m_currentShaderPass = -1;
}
