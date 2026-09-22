#include "original_gpu_edge.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {
using zh::renderer::TextureHandle;

[[noreturn]] void terrain_material_pending()
{
	throw std::runtime_error("original terrain material application is pending");
}

unsigned requested_mips(MipCountType count)
{
	return count == MIP_LEVELS_ALL ? 0U : static_cast<unsigned>(count);
}

void allocate_terrain_texture(TextureClass *source, WW3DFormat format,
	unsigned width, unsigned height, MipCountType count)
{
	auto &edge = zh::original_runtime::OriginalGpuEdge::required();
	unsigned mips = requested_mips(count);
	TextureHandle handle = edge.create_texture(format, width, height, mips);
	try {
		edge.publish_texture(source, handle);
		source->Apply_Gpu_Texture(format, width, height);
	} catch (...) {
		edge.discard_texture(handle);
		throw;
	}
}

std::vector<UnsignedByte> downsample_packed(const std::vector<UnsignedByte> &source,
	unsigned width, unsigned height)
{
	const unsigned next_width = std::max(1U, width / 2);
	const unsigned next_height = std::max(1U, height / 2);
	std::vector<UnsignedByte> result(next_width * next_height * 2);
	for (unsigned y = 0; y < next_height; ++y) {
		for (unsigned x = 0; x < next_width; ++x) {
			unsigned sums[4]{};
			for (unsigned dy = 0; dy < (height > 1 ? 2U : 1U); ++dy) {
				for (unsigned dx = 0; dx < (width > 1 ? 2U : 1U); ++dx) {
					const unsigned sx = std::min(width - 1, x * 2 + dx);
					const unsigned sy = std::min(height - 1, y * 2 + dy);
					const std::size_t offset = (sy * width + sx) * 2;
					const unsigned value = source[offset] | (unsigned(source[offset + 1]) << 8);
					sums[0] += value & 31U; sums[1] += (value >> 5) & 31U;
					sums[2] += (value >> 10) & 31U; sums[3] += (value >> 15) & 1U;
				}
			}
			const unsigned samples = (width > 1 ? 2U : 1U) * (height > 1 ? 2U : 1U);
			const unsigned value = ((sums[0] + samples / 2) / samples)
				| (((sums[1] + samples / 2) / samples) << 5)
				| (((sums[2] + samples / 2) / samples) << 10)
				| ((sums[3] * 2 >= samples ? 1U : 0U) << 15);
			const std::size_t offset = (y * next_width + x) * 2;
			result[offset] = static_cast<UnsignedByte>(value);
			result[offset + 1] = static_cast<UnsignedByte>(value >> 8);
		}
	}
	return result;
}

std::vector<UnsignedByte> downsample_bgra(const std::vector<UnsignedByte> &source,
	unsigned width, unsigned height)
{
	const unsigned next_width = std::max(1U, width / 2);
	const unsigned next_height = std::max(1U, height / 2);
	std::vector<UnsignedByte> result(next_width * next_height * 4);
	for (unsigned y = 0; y < next_height; ++y) {
		for (unsigned x = 0; x < next_width; ++x) {
			unsigned sums[4]{};
			unsigned samples = 0;
			for (unsigned dy = 0; dy < (height > 1 ? 2U : 1U); ++dy) {
				for (unsigned dx = 0; dx < (width > 1 ? 2U : 1U); ++dx) {
					const unsigned sx = std::min(width - 1, x * 2 + dx);
					const unsigned sy = std::min(height - 1, y * 2 + dy);
					for (unsigned channel = 0; channel < 4; ++channel)
						sums[channel] += source[(sy * width + sx) * 4 + channel];
					++samples;
				}
			}
			for (unsigned channel = 0; channel < 4; ++channel)
				result[(y * next_width + x) * 4 + channel] =
					static_cast<UnsignedByte>((sums[channel] + samples / 2) / samples);
		}
	}
	return result;
}

template <typename Downsample>
void upload_mips(TextureClass *source, std::vector<UnsignedByte> bytes,
	unsigned width, unsigned height, unsigned bytes_per_pixel, Downsample downsample)
{
	auto &edge = zh::original_runtime::OriginalGpuEdge::required();
	const TextureHandle handle = edge.texture_handle(source);
	unsigned levels = source->Get_Mip_Level_Count() == MIP_LEVELS_ALL ? 1U :
		static_cast<unsigned>(source->Get_Mip_Level_Count());
	if (source->Get_Mip_Level_Count() == MIP_LEVELS_ALL) {
		unsigned w = width, h = height;
		while (w > 1 || h > 1) { ++levels; w = std::max(1U, w / 2); h = std::max(1U, h / 2); }
	}
	for (unsigned level = 0; level < levels; ++level) {
		edge.upload_texture(handle, level, width, height, width * bytes_per_pixel,
			bytes.data(), bytes.size());
		if (level + 1 < levels) {
			bytes = downsample(bytes, width, height);
			width = std::max(1U, width / 2); height = std::max(1U, height / 2);
		}
	}
}
}

TerrainTextureClass::TerrainTextureClass(int height) :
	TextureClass(TEXTURE_WIDTH, height, WW3D_FORMAT_A1R5G5B5, MIP_LEVELS_3)
{
	if (height <= 0) throw std::runtime_error("original terrain atlas height is invalid");
	allocate_terrain_texture(this, WW3D_FORMAT_A1R5G5B5, TEXTURE_WIDTH, height, MIP_LEVELS_3);
}

TerrainTextureClass::TerrainTextureClass(int height, int width) :
	TextureClass(width, height, WW3D_FORMAT_A1R5G5B5, MIP_LEVELS_ALL)
{
	if (width <= 0 || height <= 0) throw std::runtime_error("original flat terrain atlas extent is invalid");
	allocate_terrain_texture(this, WW3D_FORMAT_A1R5G5B5, width, height, MIP_LEVELS_ALL);
}

int TerrainTextureClass::update(WorldHeightMap *map)
{
	if (!map || Get_Width() != TEXTURE_WIDTH || Get_Height() <= 0)
		throw std::runtime_error("original terrain atlas update input is invalid");
	const unsigned width = static_cast<unsigned>(Get_Width());
	const unsigned height = static_cast<unsigned>(Get_Height());
	std::vector<UnsignedByte> bytes(width * height * 2, 0);
	for (Int tile_index = 0; tile_index < map->m_numBitmapTiles; ++tile_index) {
		TileData *tile = map->getSourceTile(tile_index);
		if (!tile || tile->m_tileLocationInTexture.x <= 0) continue;
		const ICoord2D position = tile->m_tileLocationInTexture;
		if (position.x + TILE_PIXEL_EXTENT > Int(width) || position.y + TILE_PIXEL_EXTENT > Int(height))
			throw std::runtime_error("original terrain tile exceeds atlas");
		for (Int row = 0; row < TILE_PIXEL_EXTENT; ++row) {
			UnsignedByte *source = tile->getRGBDataForWidth(TILE_PIXEL_EXTENT)
				+ (TILE_PIXEL_EXTENT - 1 - row) * TILE_BYTES_PER_PIXEL * TILE_PIXEL_EXTENT;
			for (Int column = 0; column < TILE_PIXEL_EXTENT; ++column) {
				const unsigned packed = 0x8000U | ((source[2] >> 3) << 10)
					| ((source[1] >> 3) << 5) | (source[0] >> 3);
				const std::size_t offset = ((position.y + row) * width + position.x + column) * 2;
				bytes[offset] = static_cast<UnsignedByte>(packed);
				bytes[offset + 1] = static_cast<UnsignedByte>(packed >> 8);
				source += TILE_BYTES_PER_PIXEL;
			}
		}
	}
	for (Int texture_class = 0; texture_class < map->m_numTextureClasses; ++texture_class) {
		const Int span = map->m_textureClasses[texture_class].width * TILE_PIXEL_EXTENT;
		const ICoord2D origin = map->m_textureClasses[texture_class].positionInTexture;
		if (origin.x <= 0) continue;
		if (origin.x < 4 || origin.y < 4 || origin.x + span + 4 > Int(width) || origin.y + span + 4 > Int(height))
			throw std::runtime_error("original terrain class border exceeds atlas");
		for (Int row = 0; row < span; ++row) {
			auto *line = bytes.data() + ((origin.y + row) * width + origin.x) * 2;
			std::memcpy(line - 8, line + (span - 4) * 2, 8);
			std::memcpy(line + span * 2, line, 8);
		}
		for (Int border = 0; border < 4; ++border) {
			const std::size_t row_bytes = (span + 8) * 2;
			auto *first = bytes.data() + ((origin.y - border - 1) * width + origin.x - 4) * 2;
			auto *last = bytes.data() + ((origin.y + span + border) * width + origin.x - 4) * 2;
			std::memcpy(first, bytes.data() + ((origin.y + span - border - 1) * width + origin.x - 4) * 2, row_bytes);
			std::memcpy(last, bytes.data() + ((origin.y + border) * width + origin.x - 4) * 2, row_bytes);
		}
	}
	upload_mips(this, std::move(bytes), width, height, 2, downsample_packed);
	return static_cast<int>(height);
}

Bool TerrainTextureClass::updateFlat(WorldHeightMap *, Int, Int, Int, Int)
{
	throw std::runtime_error("original flat terrain atlas update is pending");
}
void TerrainTextureClass::setLOD(Int) {}
void TerrainTextureClass::Apply(unsigned int stage) { TextureClass::Apply(stage); }

AlphaTerrainTextureClass::AlphaTerrainTextureClass(TextureClass *base) :
	TextureClass(8, 8, WW3D_FORMAT_A1R5G5B5, MIP_LEVELS_1)
{
	if (!base) throw std::runtime_error("original alpha terrain alias requires base atlas");
	auto &edge = zh::original_runtime::OriginalGpuEdge::required();
	edge.publish_texture_alias(this, base);
	Apply_Gpu_Texture(base->Get_Texture_Format(), base->Get_Width(), base->Get_Height());
}
void AlphaTerrainTextureClass::Apply(unsigned int) { terrain_material_pending(); }

AlphaEdgeTextureClass::AlphaEdgeTextureClass(int height, MipCountType count) :
	TextureClass(TEXTURE_WIDTH, height, WW3D_FORMAT_A8R8G8B8, count)
{
	if (height <= 0) throw std::runtime_error("original alpha edge atlas height is invalid");
	allocate_terrain_texture(this, WW3D_FORMAT_A8R8G8B8, TEXTURE_WIDTH, height, count);
}
int AlphaEdgeTextureClass::update256(WorldHeightMap *) { return 1; }
int AlphaEdgeTextureClass::update(WorldHeightMap *map)
{
	if (!map) throw std::runtime_error("original alpha edge atlas update requires map");
	const unsigned width = static_cast<unsigned>(Get_Width());
	const unsigned height = static_cast<unsigned>(Get_Height());
	std::vector<UnsignedByte> bytes(width * height * 4, 0);
	for (unsigned y = 0; y < height; ++y) for (unsigned x = 0; x < width; ++x) {
		auto *pixel = bytes.data() + (y * width + x) * 4;
		pixel[0] = static_cast<UnsignedByte>(x / 2); pixel[2] = static_cast<UnsignedByte>(255 - y / 2);
		pixel[3] = 128;
	}
	for (Int tile_index = 0; tile_index < map->m_numEdgeTiles; ++tile_index) {
		TileData *tile = map->getEdgeTile(tile_index);
		if (!tile || tile->m_tileLocationInTexture.x <= 0) continue;
		const ICoord2D position = tile->m_tileLocationInTexture;
		if (position.x + TILE_PIXEL_EXTENT > Int(width) || position.y + TILE_PIXEL_EXTENT > Int(height))
			throw std::runtime_error("original alpha edge tile exceeds atlas");
		for (Int row = 0; row < TILE_PIXEL_EXTENT; ++row) {
			UnsignedByte *source = tile->getRGBDataForWidth(TILE_PIXEL_EXTENT)
				+ (TILE_PIXEL_EXTENT - 1 - row) * TILE_BYTES_PER_PIXEL * TILE_PIXEL_EXTENT;
			for (Int column = 0; column < TILE_PIXEL_EXTENT; ++column) {
				auto *pixel = bytes.data() + ((position.y + row) * width + position.x + column) * 4;
				pixel[0] = source[0]; pixel[1] = source[1]; pixel[2] = source[2];
				pixel[3] = source[0] == 0 && source[1] == 0 && source[2] == 0 ? 0x80 :
					(source[0] == 0xff && source[1] == 0xff && source[2] == 0xff ? 0 : 0xff);
				source += TILE_BYTES_PER_PIXEL;
			}
		}
	}
	upload_mips(this, std::move(bytes), width, height, 4, downsample_bgra);
	return static_cast<int>(height);
}
void AlphaEdgeTextureClass::Apply(unsigned int) { terrain_material_pending(); }

LightMapTerrainTextureClass::LightMapTerrainTextureClass(AsciiString name, MipCountType count) :
	TextureClass(name.isEmpty() ? "TSNoiseUrb.tga" : name.str(),
		name.isEmpty() ? "TSNoiseUrb.tga" : name.str(), count) {}
void LightMapTerrainTextureClass::Apply(unsigned int) { terrain_material_pending(); }
ScorchTextureClass::ScorchTextureClass(MipCountType count) :
	TextureClass("EXScorch01.tga", "EXScorch01.tga", count) {}
void ScorchTextureClass::Apply(unsigned int) { terrain_material_pending(); }
CloudMapTerrainTextureClass::CloudMapTerrainTextureClass(MipCountType count) :
	TextureClass("TSCloudMed.tga", "TSCloudMed.tga", count), m_xSlidePerSecond(-0.02f),
	m_ySlidePerSecond(-0.03f), m_curTick(0), m_xOffset(0), m_yOffset(0) {}
void CloudMapTerrainTextureClass::Apply(unsigned int) { terrain_material_pending(); }
void CloudMapTerrainTextureClass::restore() { terrain_material_pending(); }
