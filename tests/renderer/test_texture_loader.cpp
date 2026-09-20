#include "zh/renderer/texture_loader.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace zh::renderer;

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void put16(std::vector<UInt8>& bytes, std::size_t offset, UInt16 value) { bytes[offset] = value & 0xffU; bytes[offset + 1] = value >> 8U; }
void put32(std::vector<UInt8>& bytes, std::size_t offset, UInt32 value)
{
    for (unsigned i = 0; i < 4; ++i) bytes[offset + i] = static_cast<UInt8>(value >> (8U * i));
}

std::vector<UInt8> dds(UInt32 fourcc, std::size_t block_size)
{
    std::vector<UInt8> bytes(128 + block_size, 0);
    put32(bytes, 0, 0x20534444U); put32(bytes, 4, 124); put32(bytes, 12, 4); put32(bytes, 16, 4);
    put32(bytes, 28, 1); put32(bytes, 76, 32); put32(bytes, 84, fourcc);
    return bytes;
}

void test_tga_channel_and_origin_conversion()
{
    std::vector<UInt8> bytes(18 + 12, 0);
    bytes[2] = 2; put16(bytes, 12, 2); put16(bytes, 14, 2); bytes[16] = 24; // bottom-left
    // Bottom row: red, green. Top row: blue, white (BGR source).
    const UInt8 pixels[] = {0,0,255, 0,255,0, 255,0,0, 255,255,255};
    for (std::size_t i = 0; i < sizeof(pixels); ++i) bytes[18 + i] = pixels[i];
    auto result = parse_texture(bytes, false);
    check(static_cast<bool>(result), "valid TGA rejected");
    check(result.texture.bytes[0] == 0 && result.texture.bytes[2] == 255, "TGA origin was not normalized to top-left");
    check(result.texture.bytes[8] == 255 && result.texture.bytes[9] == 0 && result.texture.bytes[10] == 0,
        "TGA BGR was not converted to RGBA");

    bytes[2] = 10;
    check(!parse_texture(bytes, false), "RLE TGA accepted");
    bytes.resize(19);
    check(!parse_texture(bytes, false), "truncated TGA accepted");
}

void test_bc1_retention_and_fallback()
{
    auto bytes = dds(0x31545844U, 8);
    // red endpoint, green endpoint, all pixels select red
    bytes[128] = 0x00; bytes[129] = 0xf8; bytes[130] = 0xe0; bytes[131] = 0x07;
    auto retained = parse_texture(bytes, true);
    check(static_cast<bool>(retained) && retained.texture.descriptor.format == TextureFormat::bc1, "supported BC1 was not retained");
    check(retained.texture.bytes.size() == 8 && !retained.texture.decoded_bc_fallback, "retained BC1 payload changed");
    auto decoded = parse_texture(bytes, false);
    check(static_cast<bool>(decoded) && decoded.texture.descriptor.format == TextureFormat::rgba8, "BC1 fallback did not produce RGBA8");
    check(decoded.texture.bytes.size() == 64 && decoded.texture.bytes[0] == 255 && decoded.texture.bytes[1] == 0
        && decoded.texture.bytes[2] == 0 && decoded.texture.bytes[3] == 255, "BC1 fallback pixels are wrong");
}

void test_dxt2_dxt4_semantics_and_alpha()
{
    auto dxt2 = dds(0x32545844U, 16);
    dxt2[128] = 0x0f; // alpha nibbles: pixel 0 = 15, pixel 1 = 0
    dxt2[136] = 0xff; dxt2[137] = 0xff;
    auto two = parse_texture(dxt2, false);
    check(static_cast<bool>(two) && two.texture.premultiplied_alpha && two.texture.bytes[3] == 255
        && two.texture.bytes[7] == 0, "DXT2 premultiplied alpha semantics were lost");

    auto dxt4 = dds(0x34545844U, 16);
    dxt4[128] = 200; dxt4[129] = 10; // all alpha indices select 200
    dxt4[136] = 0xff; dxt4[137] = 0xff;
    auto four = parse_texture(dxt4, false);
    check(static_cast<bool>(four) && four.texture.premultiplied_alpha && four.texture.bytes[3] == 200,
        "DXT4 premultiplied alpha semantics were lost");

    auto dxt3 = dds(0x33545844U, 16);
    check(static_cast<bool>(parse_texture(dxt3, true)) && !parse_texture(dxt3, true).texture.premultiplied_alpha,
        "DXT3 incorrectly marked premultiplied");
}

void test_dds_rejections()
{
    auto bad = dds(0x31545844U, 8);
    put32(bad, 4, 120);
    check(!parse_texture(bad, false), "bad DDS header size accepted");
    bad = dds(0x30315844U, 8);
    check(!parse_texture(bad, false), "unsupported DDS format accepted");
    bad = dds(0x31545844U, 8); bad.resize(132);
    check(!parse_texture(bad, false), "truncated DDS blocks accepted");
    bad = dds(0x31545844U, 8); put32(bad, 16, 20000);
    check(!parse_texture(bad, false), "excessive DDS dimension accepted");
}
} // namespace

int main()
{
    try {
        test_tga_channel_and_origin_conversion();
        test_bc1_retention_and_fallback();
        test_dxt2_dxt4_semantics_and_alpha();
        test_dds_rejections();
        std::cout << "texture loader tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
