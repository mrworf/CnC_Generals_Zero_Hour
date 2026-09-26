#include "Common/GlobalData.h"
#include "WW3D2/dx8wrapper.h"
#include "original_gpu_edge.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "tree_shroud_projection_cpu.h"

#include <stdexcept>
#include <cmath>
#include <limits>
#include <exception>

namespace {
bool shader_manager_initialized = false;

Matrix4x4 shroudTransform(W3DShroud& shroud)
{
    Matrix4x4 view;DX8Wrapper::Get_Transform(D3DTS_VIEW,view);
    return zh::original_runtime::detail::tree_shroud_projection(view,shroud.getCellWidth(),shroud.getCellHeight(),
        shroud.getTextureWidth(),shroud.getTextureHeight(),shroud.getDrawOriginX(),shroud.getDrawOriginY());
}

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
	if (shader == ST_TERRAIN_BASE) return 2;
	if (shader == ST_ROAD_BASE) return 1;
	else
		throw std::runtime_error("original terrain shader kind is unsupported by Linux minimum profile");
}

Int W3DShaderManager::setShroudTex(Int stage)
{
    auto& edge=zh::original_runtime::OriginalGpuEdge::required();
    edge.guard_nonstage_mutation();
    require_shader_manager();
    if (stage!=1 || !edge.idle_preparation_ready())
        throw std::runtime_error("original tree shroud binding phase or stage unavailable");
    auto* shroud=W3DTerrainVisual::peekPublishedShroud();
    if (!shroud) throw std::runtime_error("original tree shroud current resident content unavailable");
    auto* texture=shroud->getShroudTexture();
    const auto matrix=shroudTransform(*shroud);
    zh::original_runtime::OriginalGpuEdge::SourceStageSelection selection{1,texture};
    zh::original_runtime::OriginalGpuEdge::SourceStageDesc desc;
    desc.generation=edge.generation();desc.selections=&selection;desc.count=1;
    zh::original_runtime::OriginalGpuEdge::SourceStageToken token;
    if (!edge.begin_source_stages(desc,token))
        throw std::runtime_error("original tree shroud binding admission rejected");
    try {
        DX8Wrapper::Set_Texture(1,texture);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_ALPHAARG2,D3DTA_CURRENT);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_COLOROP,D3DTOP_MODULATE);
        DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2);
        DX8Wrapper::Set_Transform(D3DTS_TEXTURE0+1,matrix);
        edge.apply_source_stages(token);
        if (!edge.commit_source_stages(token)) throw std::runtime_error("original tree shroud binding commit rejected");
    } catch (...) {
        if (!edge.abort_source_stages(token)) std::terminate();
        if (!edge.drain_source_references(edge.generation()))
            throw std::runtime_error("original tree shroud failed-binding pin drain pending");
        throw;
    }
    return TRUE; // Ordinary caller boundary owns accepted-pin drain.
}

Int W3DShaderManager::setShader(ShaderTypes shader, Int pass)
{
	require_shader_manager();
	if (shader != ST_TERRAIN_BASE && shader != ST_ROAD_BASE)
		throw std::runtime_error("original terrain shader kind is unsupported by Linux minimum profile");
	const Int pass_count = shader == ST_TERRAIN_BASE ? 2 : 1;
	if (pass < 0 || pass >= pass_count)
		throw std::runtime_error("original terrain shader pass is outside the minimum route");
	if (shader == m_currentShader && pass == m_currentShaderPass) return TRUE;
	TextureClass *selected = shader == ST_TERRAIN_BASE ? m_Textures[pass] : m_Textures[0];
	if (!selected || (shader == ST_TERRAIN_BASE && (!m_Textures[0] || !m_Textures[1])))
		throw std::runtime_error("original terrain shader atlas pair is incomplete");
	auto &edge = zh::original_runtime::OriginalGpuEdge::required();
	// Validate every required published owner before changing delayed source state.
	(void)edge.texture_handle(selected);
	if (shader == ST_TERRAIN_BASE) {
		(void)edge.texture_handle(m_Textures[0]);
		(void)edge.texture_handle(m_Textures[1]);
	}
	DX8Wrapper::Apply_Render_State_Changes();
	try {
		edge.select_texture(0, selected);
		edge.select_texture(1, nullptr);
		select_filter(edge, 0);
		select_filter(edge, 1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ALPHAOP,
			shader == ST_ROAD_BASE || pass != 0 ? D3DTOP_MODULATE : D3DTOP_DISABLE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX,
			shader == ST_TERRAIN_BASE ? pass : 0);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		const Bool alpha_blend = shader == ST_ROAD_BASE || pass != 0;
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, alpha_blend);
		if (alpha_blend) {
			DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		if (shader == ST_ROAD_BASE) {
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
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
	if (shader != ST_TERRAIN_BASE && shader != ST_ROAD_BASE)
		throw std::runtime_error("original terrain shader reset kind is unsupported");
	if (m_currentShader == ST_INVALID) return;
	if (m_currentShader != shader)
		throw std::runtime_error("original terrain shader reset does not match active pass");
	clear_terrain_pass(true);
	m_currentShader = ST_INVALID;
	m_currentShaderPass = -1;
}
