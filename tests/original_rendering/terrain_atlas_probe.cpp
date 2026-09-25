#include "PreRTS.h"

#include "Common/MapReaderWriterInfo.h"
#include "Common/GlobalData.h"
#include "w3d_shader_manager_cpu_types.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/ww3d.h"
#include "original_gpu_edge.h"
#include "zh/original_process.h"
#include "zh/renderer/recording_device.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
using zh::renderer::RecordingGpuDevice;

void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

class TestVisualMap final : public WorldHeightMap
{
public:
	explicit TestVisualMap(ChunkInputStream *input) : WorldHeightMap(input, FALSE) {}
	ICoord2D source_position(Int index) const { return m_sourceTiles[index]->m_tileLocationInTexture; }
	ICoord2D edge_position(Int index) const { return m_edgeTiles[index]->m_tileLocationInTexture; }
	Region2D edge_range(Int index) { Region2D range; getUVForBlend(index, &range); return range; }
	bool rejects_missing_source_owner()
	{
		TileData *owner = m_sourceTiles[0];
		m_sourceTiles[0] = nullptr;
		bool rejected = false;
		try { Int edge_height = 0; (void)updateTileTexturePositions(&edge_height); }
		catch (const std::runtime_error &) { rejected = true; }
		m_sourceTiles[0] = owner;
		return rejected;
	}
	bool rejects_capacity_overflow()
	{
		constexpr Int class_count = 5;
		constexpr Int tile_count = class_count * 100;
		TXTextureClass saved_classes[class_count];
		TileData *saved_tiles[tile_count];
		for (Int index = 0; index < class_count; ++index)
			saved_classes[index] = m_textureClasses[index];
		for (Int index = 0; index < tile_count; ++index)
			saved_tiles[index] = m_sourceTiles[index];
		const Int saved_class_count = m_numTextureClasses;
		const Int saved_tile_count = m_numBitmapTiles;
		TileData *owner = m_sourceTiles[0];
		const ICoord2D saved_position = owner->m_tileLocationInTexture;
		m_numTextureClasses = class_count;
		m_numBitmapTiles = tile_count;
		for (Int index = 0; index < class_count; ++index) {
			m_textureClasses[index].width = 10;
			m_textureClasses[index].firstTile = index * 100;
		}
		for (Int index = 0; index < tile_count; ++index) m_sourceTiles[index] = owner;
		bool rejected = false;
		try { Int edge_height = 0; (void)updateTileTexturePositions(&edge_height); }
		catch (const std::runtime_error &) { rejected = true; }
		m_numTextureClasses = saved_class_count;
		m_numBitmapTiles = saved_tile_count;
		for (Int index = 0; index < class_count; ++index)
			m_textureClasses[index] = saved_classes[index];
		for (Int index = 0; index < tile_count; ++index)
			m_sourceTiles[index] = saved_tiles[index];
		owner->m_tileLocationInTexture = saved_position;
		return rejected;
	}
};

TestVisualMap *open_map(const char *path)
{
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original terrain atlas map unreadable");
	auto *map = NEW_REF(TestVisualMap, (&input));
	input.close();
	return map;
}

template <typename Operation>
void require_rejected(Operation operation, const char *message)
{
	bool rejected = false;
	try { operation(); } catch (const std::runtime_error &) { rejected = true; }
	require(rejected, message);
}

void verify_terrain_shader(RecordingGpuDevice &device,
	zh::original_runtime::OriginalGpuEdge &edge, TextureClass *base, TextureClass *alpha)
{
	using Edge = zh::original_runtime::OriginalGpuEdge;
	using Op = Edge::CombinerOp;
	using Arg = Edge::CombinerArg;
	require_rejected([] { (void)W3DShaderManager::getShaderPasses(W3DShaderManager::ST_TERRAIN_BASE); },
		"original terrain shader query before init was accepted");
	// This probe deliberately has no W3DDisplay, so it owns the otherwise
	// display-owned shader-manager fixture explicitly.
	W3DShaderManager::init();
	require_rejected([] { W3DShaderManager::init(); },
		"original terrain shader duplicate init was accepted");
	require(W3DShaderManager::getShaderPasses(W3DShaderManager::ST_TERRAIN_BASE) == 2,
		"original minimum terrain shader pass count changed");
	require(W3DShaderManager::getShaderPasses(W3DShaderManager::ST_ROAD_BASE) == 1,
		"original minimum road shader pass count changed");
	require_rejected([] {
		(void)W3DShaderManager::getShaderPasses(W3DShaderManager::ST_TERRAIN_BASE_NOISE1);
	}, "original optional terrain noise shader was exposed");
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);
	}, "original optional effect shader was exposed");
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 2);
	}, "original terrain shader accepted an invalid pass");
	require_rejected([] {
		(void)W3DShaderManager::getShaderPasses(W3DShaderManager::ST_ROAD_BASE_NOISE1);
	}, "original optional road noise shader was exposed");
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_ROAD_BASE, 1);
	}, "original road shader accepted an invalid pass");
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 0);
	}, "original terrain shader accepted an absent atlas pair");

	ShaderClass baseline;
	baseline.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
	DX8Wrapper::Set_Shader(baseline);
	DX8Wrapper::Set_Material(nullptr);
	DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix4x4(true));
	DX8Wrapper::Set_Transform(D3DTS_VIEW, Matrix4x4(true));
	DX8Wrapper::Set_Transform(D3DTS_PROJECTION, Matrix4x4(true));
	DX8Wrapper::Apply_Render_State_Changes();
	W3DShaderManager::setTexture(0, base);
	W3DShaderManager::setTexture(1, alpha);
	TextureClass unpublished("unpublished", "unpublished.tga", MIP_LEVELS_ALL,
		WW3D_FORMAT_UNKNOWN, true, true);
	W3DShaderManager::setTexture(1, &unpublished);
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 0);
	}, "original terrain shader accepted an unpublished alias owner");
	W3DShaderManager::setTexture(1, alpha);
	device.fail_next_sampler_create();
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 0);
	}, "original terrain shader sampler failure was ignored");
	require(!edge.pending_stage(0).source && !edge.pending_stage(1).source &&
		device.resource_counts().samplers == 0 &&
		W3DShaderManager::getCurrentShader() == W3DShaderManager::ST_INVALID,
		"original terrain shader failure retained delayed state");

	require(W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 0),
		"original terrain base pass zero failed after rollback");
	const auto pass0_texture = edge.pending_stage(0);
	const auto pass0_sampler = device.sampler_descriptor(pass0_texture.sampler);
	const auto pass0 = Edge::map_applied_state(DX8_FVF_XYZNUV2);
	require(pass0_texture.source == base && pass0_texture.texture == edge.texture_handle(base) &&
		pass0_sampler.min_filter == zh::renderer::Filter::linear &&
		pass0_sampler.mag_filter == zh::renderer::Filter::linear &&
		pass0_sampler.mip_filter == zh::renderer::Filter::nearest &&
		pass0_sampler.address_u == zh::renderer::AddressMode::clamp_edge &&
		pass0_sampler.address_v == zh::renderer::AddressMode::clamp_edge,
		"original terrain base pass zero selected the wrong atlas/filter");
	require(pass0.stages[0].uv_source == 0 && pass0.stages[0].color.op == Op::modulate &&
		pass0.stages[0].color.first == Arg::texture &&
		pass0.stages[0].color.second == Arg::diffuse &&
		pass0.stages[0].alpha.op == Op::disable &&
		pass0.stages[1].color.op == Op::disable && !pass0.pipeline.blend.enabled,
		"original terrain base pass zero state changed");
	const auto active_sampler = pass0_texture.sampler;
	require(W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 0) &&
		edge.pending_stage(0).sampler == active_sampler && device.resource_counts().samplers == 2,
		"original terrain shader idempotence cleared an active stage");
	require_rejected([] { W3DShaderManager::shutdown(); },
		"original terrain shader shutdown accepted an active pass");
	W3DShaderManager::resetShader(W3DShaderManager::ST_TERRAIN_BASE);
	if (edge.pending_stage(0).source || edge.pending_stage(1).source ||
		device.resource_counts().samplers != 0)
		throw std::runtime_error("original terrain pass zero reset retained texture state: samplers=" +
			std::to_string(device.resource_counts().samplers));

	DX8Wrapper::Set_Shader(baseline);
	DX8Wrapper::Apply_Render_State_Changes();
	require(W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 1),
		"original terrain alpha pass failed");
	const auto pass1_texture = edge.pending_stage(0);
	const auto pass1 = Edge::map_applied_state(DX8_FVF_XYZNUV2);
	require(pass1_texture.source == alpha &&
		pass1_texture.texture == edge.texture_handle(base) &&
		pass1_texture.texture == edge.texture_handle(alpha),
		"original terrain alpha pass did not select the shared alias handle");
	require(pass1.stages[0].uv_source == 1 && pass1.stages[0].color.op == Op::modulate &&
		pass1.stages[0].alpha.op == Op::modulate && pass1.pipeline.blend.enabled &&
		pass1.pipeline.blend.source_color == zh::renderer::BlendFactor::src_alpha &&
		pass1.pipeline.blend.destination_color == zh::renderer::BlendFactor::inv_src_alpha &&
		pass1.stages[1].color.op == Op::disable,
		"original terrain alpha pass state changed");
	W3DShaderManager::resetShader(W3DShaderManager::ST_TERRAIN_BASE);
	W3DShaderManager::resetShader(W3DShaderManager::ST_TERRAIN_BASE);
	DX8Wrapper::Set_Shader(baseline);
	DX8Wrapper::Apply_Render_State_Changes();
	require(!Edge::map_applied_state(DX8_FVF_XYZNUV2).pipeline.blend.enabled,
		"original terrain shader reset did not restore delayed shader selection");

	W3DShaderManager::setTexture(0, nullptr);
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_ROAD_BASE, 0);
	}, "original road shader accepted a missing alpha atlas");
	W3DShaderManager::setTexture(0, &unpublished);
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_ROAD_BASE, 0);
	}, "original road shader accepted an unpublished alpha atlas");
	W3DShaderManager::setTexture(0, alpha);
	device.fail_next_sampler_create();
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_ROAD_BASE, 0);
	}, "original road shader sampler failure was ignored");
	require(!edge.pending_stage(0).source && !edge.pending_stage(1).source &&
		device.resource_counts().samplers == 0 &&
		W3DShaderManager::getCurrentShader() == W3DShaderManager::ST_INVALID,
		"original road shader failure retained delayed state");
	require(W3DShaderManager::setShader(W3DShaderManager::ST_ROAD_BASE, 0),
		"original road alpha material failed after rollback");
	const auto road_texture = edge.pending_stage(0);
	const auto road_state = Edge::map_applied_state(DX8_FVF_XYZNUV2);
	require(road_texture.source == alpha && road_texture.texture == edge.texture_handle(alpha) &&
		road_state.stages[0].uv_source == 0 &&
		road_state.stages[0].color.op == Op::modulate &&
		road_state.stages[0].color.first == Arg::texture &&
		road_state.stages[0].color.second == Arg::diffuse &&
		road_state.stages[0].alpha.op == Op::modulate &&
		road_state.pipeline.blend.enabled &&
		road_state.pipeline.blend.source_color == zh::renderer::BlendFactor::src_alpha &&
		road_state.pipeline.blend.destination_color == zh::renderer::BlendFactor::inv_src_alpha &&
		!road_state.pipeline.depth_stencil.depth_write &&
		road_state.pipeline.depth_stencil.depth_compare == zh::renderer::CompareOp::less_equal &&
		!road_state.lighting && road_state.stages[1].color.op == Op::disable,
		"original road alpha material state changed");
	require_rejected([] { W3DShaderManager::resetShader(W3DShaderManager::ST_TERRAIN_BASE); },
		"original road shader accepted a mismatched reset");
	W3DShaderManager::resetShader(W3DShaderManager::ST_ROAD_BASE);
	require(!edge.pending_stage(0).source && !edge.pending_stage(1).source &&
		device.resource_counts().samplers == 0 &&
		W3DShaderManager::getCurrentShader() == W3DShaderManager::ST_INVALID,
		"original road shader reset retained state");
	W3DShaderManager::shutdown();
	require_rejected([] {
		(void)W3DShaderManager::setShader(W3DShaderManager::ST_TERRAIN_BASE, 0);
	}, "original terrain shader accepted a pass after shutdown");
}

void expect_rollback(const char *path, unsigned create_after, unsigned upload_after)
{
	RecordingGpuDevice device;
	zh::original_runtime::OriginalGpuEdge edge(device);
	if (create_after != ~0U) device.fail_texture_create_after(create_after);
	if (upload_after != ~0U) device.fail_texture_upload_after(upload_after);
	auto *map = open_map(path);
	try {
		bool rejected = false;
		try { (void)map->getTerrainTexture(); } catch (const std::runtime_error &) { rejected = true; }
		require(rejected, "original terrain atlas injected failure was ignored");
		require(device.resource_counts().total() == 0,
			"original terrain atlas partial failure retained a resource");
		TextureClass *retry = map->getTerrainTexture();
		require(retry && device.resource_counts().textures == 2,
			"original terrain atlas retry failed after rollback");
		map->Release_Ref(); map = nullptr;
	} catch (...) {
		if (map) map->Release_Ref();
		throw;
	}
	require(device.resource_counts().total() == 0,
		"original terrain atlas retry teardown retained a resource");
}

void successful_generation(const char *path)
{
	RecordingGpuDevice device;
	zh::original_runtime::OriginalGpuEdge edge(device);
	auto *map = open_map(path);
	try {
	TextureClass *base = map->getTerrainTexture();
	TextureClass *alpha = map->getAlphaTerrainTexture();
	TextureClass *edge_texture = map->getEdgeTerrainTexture();
	require(base == map->getTerrainTexture() && alpha == map->getAlphaTerrainTexture() &&
		edge_texture == map->getEdgeTerrainTexture(),
		"original terrain atlas getter was not idempotent");
	const auto base_handle = edge.texture_handle(base);
	const auto alpha_handle = edge.texture_handle(alpha);
	const auto edge_handle = edge.texture_handle(edge_texture);
	require(base_handle == alpha_handle && base_handle != edge_handle &&
		device.resource_counts().textures == 2,
		"original terrain base/alpha alias ownership changed");
	const auto base_desc = device.texture_descriptor(base_handle);
	const auto edge_desc = device.texture_descriptor(edge_handle);
	if (!(base_desc.width == 2048 && base_desc.height == 128 && base_desc.mip_levels == 3 &&
		base_desc.format == zh::renderer::TextureFormat::bgr5a1))
		throw std::runtime_error("original terrain base atlas descriptor changed: " +
			std::to_string(base_desc.width) + "x" + std::to_string(base_desc.height) + " mips=" +
			std::to_string(base_desc.mip_levels) + " format=" +
			std::to_string(static_cast<unsigned>(base_desc.format)));
	require(edge_desc.width == 2048 && edge_desc.height == 1 && edge_desc.mip_levels == 3 &&
		edge_desc.format == zh::renderer::TextureFormat::bgra8,
		"original terrain edge atlas descriptor changed");
	const auto base0 = device.texture_bytes(base_handle, 0);
	const std::size_t clear = (100U * 2048U + 100U) * 2U;
	require(base0.size() == 2048U * 128U * 2U && base0[clear] == 0 && base0[clear + 1] == 0,
		"original terrain base atlas clear region changed");
	const std::size_t inset = (4U * 2048U + 4U) * 2U;
	require(base0[inset] == 0 && base0[inset + 1] == 0x80 &&
		device.texture_bytes(base_handle, 1).size() == 1024U * 64U * 2U &&
		device.texture_bytes(base_handle, 2).size() == 512U * 32U * 2U,
		"original terrain base atlas packing or mip chain changed");
	const auto edge0 = device.texture_bytes(edge_handle, 0);
	require(edge0.size() == 2048U * 4U && edge0[0] == 0 && edge0[1] == 0 &&
		edge0[2] == 255 && edge0[3] == 128 &&
		device.texture_bytes(edge_handle, 1).size() == 1024U * 4U &&
		device.texture_bytes(edge_handle, 2).size() == 512U * 4U,
		"original terrain edge atlas gradient or mip chain changed");
	require(!map->getFlipState(0, 0), "original flat terrain flip table changed");
	require(TheGlobalData, "original terrain shader global settings are absent");
	const Bool old_bilinear = TheGlobalData->m_bilinearTerrainTex;
	const Bool old_trilinear = TheGlobalData->m_trilinearTerrainTex;
	TheWritableGlobalData->m_bilinearTerrainTex = true;
	TheWritableGlobalData->m_trilinearTerrainTex = false;
	verify_terrain_shader(device, edge, base, alpha);
	TheWritableGlobalData->m_bilinearTerrainTex = old_bilinear;
	TheWritableGlobalData->m_trilinearTerrainTex = old_trilinear;
	WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_BILINEAR);
	DX8Wrapper::Set_Texture(0, base);
	device.fail_next_sampler_create();
	bool sampler_failure = false;
	try { DX8Wrapper::Apply_Render_State_Changes(); }
	catch (const std::runtime_error &error) {
		sampler_failure = std::string(error.what()).find("sampler creation failed") != std::string::npos;
	}
	require(sampler_failure && device.resource_counts().samplers == 0,
		"original terrain base material sampler failure did not roll back");
	DX8Wrapper::Apply_Render_State_Changes();
	const auto selected = edge.pending_stage(0);
	const auto sampler = device.sampler_descriptor(selected.sampler);
	require(selected.source == base && selected.texture == base_handle &&
		sampler.min_filter == zh::renderer::Filter::linear &&
		sampler.mag_filter == zh::renderer::Filter::linear &&
		sampler.mip_filter == zh::renderer::Filter::nearest &&
		sampler.address_u == zh::renderer::AddressMode::repeat &&
		sampler.address_v == zh::renderer::AddressMode::repeat,
		"original terrain base material selection changed");
	DX8Wrapper::Apply_Render_State_Changes();
	require(device.resource_counts().samplers == 1,
		"original terrain base material idempotence changed");
	DX8Wrapper::Set_Texture(0, nullptr);
	DX8Wrapper::Apply_Render_State_Changes();
	WW3D::Enable_Texturing(false);
	DX8Wrapper::Set_Texture(0, base);
	DX8Wrapper::Apply_Render_State_Changes();
	require(!edge.pending_stage(0).texture,
		"original terrain base ignored disabled texturing");
	WW3D::Enable_Texturing(true);
	DX8Wrapper::Set_Texture(0, nullptr);
	DX8Wrapper::Apply_Render_State_Changes();
	DX8Wrapper::Set_Texture(0, base);
	DX8Wrapper::Apply_Render_State_Changes();
	DX8Wrapper::Release_Render_State();
	DX8Wrapper::Apply_Render_State_Changes();
	require(device.resource_counts().samplers == 0,
		"original terrain base delayed state retained a sampler");
	require(device.snapshot().find("draw ") == std::string::npos,
		"original terrain atlas ownership emitted a draw");
	map->Release_Ref(); map = nullptr;
	require(device.resource_counts().total() == 0,
		"original terrain atlas teardown retained a resource");
	} catch (...) {
		if (map) map->Release_Ref();
		throw;
	}
}

unsigned packed_color(const UnsignedByte color[4])
{
	return 0x8000U | ((color[2] >> 3) << 10) | ((color[1] >> 3) << 5) | (color[0] >> 3);
}

bool close_to(float left, float right)
{
	return std::fabs(left - right) < 0.00001f;
}

void successful_query_generation(const char *path)
{
	RecordingGpuDevice device;
	zh::original_runtime::OriginalGpuEdge edge(device);
	require(TheWritableGlobalData, "original terrain query global settings are absent");
	const Bool saved_adjust_cliffs = TheWritableGlobalData->m_adjustCliffTextures;
	TheWritableGlobalData->m_adjustCliffTextures = TRUE;
	auto *map = open_map(path);
	try {
		require_rejected([&] { (void)map->edge_range(0); },
			"original terrain edge query accepted an unpublished atlas");
		(void)map->getTerrainTexture();
		const Int blend_cells[9] = {0, 1, 2, 3, 4, 5, 6, 8, 9};
		const UnsignedByte expected_alpha[9][4] = {
			{0, 255, 255, 0}, {255, 0, 0, 255}, {0, 0, 255, 255},
			{255, 255, 0, 0}, {0, 0, 255, 0}, {255, 255, 255, 0},
			{255, 0, 255, 255}, {255, 255, 0, 255}, {0, 0, 0, 0}};
		const Bool expected_flip[9] = {FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
			FALSE, TRUE, FALSE};
		for (Int record = 0; record < 9; ++record) {
			float u[4]{}, v[4]{}; UnsignedByte alpha[4]{}; Bool flip = FALSE;
			const Int x = blend_cells[record] % 8;
			const Int y = blend_cells[record] / 8;
			map->getAlphaUVData(x, y, u, v, alpha, &flip, FALSE);
			for (Int corner = 0; corner < 4; ++corner)
				require(alpha[corner] == expected_alpha[record][corner],
					"original authored terrain blend alpha changed");
			require(flip == expected_flip[record],
				"original authored terrain blend flip changed");
		}
		const Int extra_cells[8] = {16, 17, 18, 19, 20, 21, 22, 24};
		for (Int record = 0; record < 8; ++record) {
			float u[4]{}, v[4]{}; UnsignedByte alpha[4]{};
			Bool flip = FALSE, cliff = FALSE;
			const Int x = extra_cells[record] % 8;
			const Int y = extra_cells[record] / 8;
			require(map->getExtraAlphaUVData(x, y, u, v, alpha, &flip, &cliff) && !cliff,
				"original authored extra terrain blend was not selected");
			for (Int corner = 0; corner < 4; ++corner)
				require(alpha[corner] == expected_alpha[record][corner],
					"original authored extra terrain blend alpha changed");
			require(flip == expected_flip[record],
				"original authored extra terrain blend flip changed");
		}
		float u[4]{}, v[4]{};
		for (Int quadrant = 0; quadrant < 4; ++quadrant) {
			const Bool stretched = map->getUVData(quadrant, 5, u, v, FALSE);
			require(!stretched && close_to(u[0], float(quadrant & 1 ? 36 : 4) / 2048) &&
				close_to(u[1], float(quadrant & 1 ? 68 : 36) / 2048) &&
				close_to(v[0], float(quadrant & 2 ? 100 : 132) / 256) &&
				close_to(v[2], float(quadrant & 2 ? 68 : 100) / 256),
				"original authored terrain base quadrant UV changed");
		}
		require(!map->getUVData(0, 5, u, v, TRUE) && close_to(u[0], 4.0f / 2048) &&
			close_to(u[1], 68.0f / 2048) && close_to(v[0], 132.0f / 256) &&
			close_to(v[2], 68.0f / 256),
			"original authored terrain full-tile UV changed");
		require(map->getUVData(0, 4, u, v, FALSE) &&
			close_to(u[0], 148.0f / 2048) && close_to(u[1], 148.0f / 2048) &&
			close_to(u[2], 1.0f + 148.0f / 2048) &&
			close_to(v[0], 68.0f / 256) && close_to(v[1], 8.0f + 68.0f / 256) &&
			map->getFlipState(0, 4) && map->getCliffState(0, 4) &&
			map->isCliffMappedTexture(0, 4),
			"original authored terrain cliff UV/flip changed");
		const Region2D range = map->edge_range(0);
		require(close_to(range.lo.x, 4.0f / 2048) && close_to(range.lo.y, 4.0f / 128) &&
			close_to(range.hi.x, 68.0f / 2048) && close_to(range.hi.y, 68.0f / 128),
			"original authored terrain custom edge range changed");
		const TXTextureClass texture_class = map->getTextureFromIndex(1);
		require(texture_class.firstTile == 4 && map->getTextureClassNoBlend(0, 4) == 1,
			"original authored terrain texture-class lookup changed");
		require_rejected([&] { (void)map->edge_range(1); },
			"original terrain edge query accepted an invalid class");
		require_rejected([&] { (void)map->getTextureFromIndex(2); },
			"original terrain texture query accepted an invalid class");
		UnsignedByte alpha[4]{}; Bool flip = FALSE;
		require_rejected([&] { map->getAlphaUVData(7, 7, u, v, alpha, &flip, FALSE); },
			"original terrain alpha query accepted an invalid cell");
		Bool cliff = FALSE;
		require_rejected([&] { (void)map->getExtraAlphaUVData(-1, 0, u, v, alpha, &flip, &cliff); },
			"original terrain extra blend query accepted an invalid cell");
		map->Release_Ref(); map = nullptr;
		TheWritableGlobalData->m_adjustCliffTextures = saved_adjust_cliffs;
		require(device.resource_counts().total() == 0,
			"original authored terrain query teardown retained a resource");
	} catch (...) {
		if (map) map->Release_Ref();
		TheWritableGlobalData->m_adjustCliffTextures = saved_adjust_cliffs;
		throw;
	}
}

void successful_authored_generation(const char *path)
{
	RecordingGpuDevice device;
	zh::original_runtime::OriginalGpuEdge edge(device);
	auto *map = open_map(path);
	try {
		require(map->rejects_missing_source_owner(),
			"original authored terrain atlas accepted a missing source owner");
		require(map->rejects_capacity_overflow(),
			"original authored terrain atlas accepted capacity overflow");
		TextureClass *base = map->getTerrainTexture();
		TextureClass *alpha = map->getAlphaTerrainTexture();
		TextureClass *edge_texture = map->getEdgeTerrainTexture();
		const auto base_handle = edge.texture_handle(base);
		const auto edge_handle = edge.texture_handle(edge_texture);
		const auto base_desc = device.texture_descriptor(base_handle);
		const auto edge_desc = device.texture_descriptor(edge_handle);
		require(base_handle == edge.texture_handle(alpha) && base_handle != edge_handle &&
			base_desc.width == 2048 && base_desc.height == 256 && base_desc.mip_levels == 3 &&
			edge_desc.width == 2048 && edge_desc.height == 128 && edge_desc.mip_levels == 3,
			"original authored terrain atlas descriptors changed");
		const ICoord2D expected_positions[5] = {{4, 68}, {68, 68}, {4, 4}, {68, 4}, {148, 4}};
		const UnsignedByte expected_colors[5][4] = {
			{3, 2, 1, 4}, {7, 6, 5, 8}, {11, 10, 9, 12}, {15, 14, 13, 16},
			{23, 22, 21, 24}};
		const auto base_bytes = device.texture_bytes(base_handle, 0);
		for (Int tile = 0; tile < 5; ++tile) {
			const ICoord2D position = map->source_position(tile);
			require(position.x == expected_positions[tile].x &&
				position.y == expected_positions[tile].y,
				"original authored terrain tile placement changed");
			const std::size_t offset = (static_cast<std::size_t>(position.y) * 2048 + position.x) * 2;
			const unsigned packed = base_bytes[offset] | (unsigned(base_bytes[offset + 1]) << 8);
			require(packed == packed_color(expected_colors[tile]),
				"original authored terrain atlas tile bytes changed");
		}
		const ICoord2D edge_position = map->edge_position(0);
		const auto edge_bytes = device.texture_bytes(edge_handle, 0);
		const std::size_t edge_offset =
			(static_cast<std::size_t>(edge_position.y) * 2048 + edge_position.x) * 4;
		require(edge_position.x == 4 && edge_position.y == 4 &&
			edge_bytes[edge_offset] == 33 && edge_bytes[edge_offset + 1] == 32 &&
			edge_bytes[edge_offset + 2] == 31 && edge_bytes[edge_offset + 3] == 255,
			"original authored edge atlas placement changed");
		map->Release_Ref(); map = nullptr;
		require(device.resource_counts().total() == 0,
			"original authored terrain atlas teardown retained a resource");
	} catch (...) {
		if (map) map->Release_Ref();
		throw;
	}
}
}

extern "C" void zh_probe_terrain_atlas()
{
	const char *path = std::getenv("ZH_M22_TERRAIN_ATLAS_MAP");
	require(path, "original terrain atlas map path missing");
	// Compare source ownership against the same process-scoped DX8/edge bootstrap
	// that every atlas generation requires; it is not atlas-owned state.
	{
		RecordingGpuDevice device;
		zh::original_runtime::OriginalGpuEdge edge(device);
	}
	const std::size_t baseline = zh::original_process::live_pool_allocations();
	expect_rollback(path, 0, ~0U); // Base atlas create.
	expect_rollback(path, ~0U, 0); // Base atlas first upload.
	expect_rollback(path, 1, ~0U); // Edge atlas create after base and alias.
	expect_rollback(path, ~0U, 3); // Edge atlas upload after three base mips.
	for (Int generation = 0; generation != 2; ++generation) successful_generation(path);
	if (const char *authored_path = std::getenv("ZH_M22_TERRAIN_ATLAS_AUTHORED_MAP")) {
		expect_rollback(authored_path, 0, ~0U);
		expect_rollback(authored_path, ~0U, 3);
		for (Int generation = 0; generation != 2; ++generation)
			successful_authored_generation(authored_path);
		std::puts("original authored terrain atlas: base=2048x256 edge=2048x128 classes=3 resources=0");
	}
	if (const char *query_path = std::getenv("ZH_M22_TERRAIN_QUERY_MAP")) {
		for (Int generation = 0; generation != 2; ++generation)
			successful_query_generation(query_path);
		std::puts("original authored terrain queries: blends=9 extras=8 cliff=1 resources=0");
	}
	const auto final_allocations = zh::original_process::live_pool_allocations();
	if (final_allocations != baseline)
		throw std::runtime_error("original terrain atlas generations retained pool allocations: " +
			std::to_string(baseline) + "->" + std::to_string(final_allocations));
	std::puts("original terrain atlas: base=2048x128 alias=shared edge=2048x1 mips=3 resources=0 draws=0");
}
