#include "zh/platform/bgfx_device.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

std::vector<zh::renderer::UInt8> pixels(unsigned width, unsigned height, unsigned bytes_per_pixel, unsigned seed)
{
    std::vector<zh::renderer::UInt8> result(width * height * bytes_per_pixel);
    for (unsigned index = 0; index < result.size(); ++index)
        result[index] = static_cast<zh::renderer::UInt8>(seed + index);
    return result;
}

void test_multi_mip_lifecycle()
{
    using namespace zh::renderer;
    BgfxOptions options;
    options.shader_root = ZH_BGFX_SHADER_DIR;

    for (unsigned generation = 0; generation != 2; ++generation) {
        BgfxGpuDevice device(options);
        for (const auto format : {TextureFormat::rgba8, TextureFormat::bgr5a1}) {
            const unsigned bytes_per_pixel = format == TextureFormat::bgr5a1 ? 2U : 4U;
            TextureDesc desc;
            desc.width = 8;
            desc.height = 4;
            desc.format = format;
            desc.mip_levels = 4;
            auto texture = device.create_texture(desc, "M22 multi-mip source atlas");
            check(bool(texture), "multi-mip sampled texture allocation failed");

            for (unsigned mip = 0; mip != desc.mip_levels; ++mip) {
                const unsigned width = std::max(1U, desc.width >> mip);
                const unsigned height = std::max(1U, desc.height >> mip);
                const auto bytes = pixels(width, height, bytes_per_pixel, generation * 17U + mip);
                check(device.upload_texture({texture, width, height, width * bytes_per_pixel, bytes.size(), mip}, bytes.data()),
                    "declared source atlas mip upload failed");
            }

            const auto level_zero = pixels(8, 4, bytes_per_pixel, 91);
            check(!device.upload_texture({texture, 8, 4, 8 * bytes_per_pixel, level_zero.size(), 4}, level_zero.data()),
                "out-of-range mip upload accepted");
            check(!device.upload_texture({texture, 4, 4, 4 * bytes_per_pixel, level_zero.size(), 0}, level_zero.data()),
                "wrong base mip extent accepted");
            check(!device.upload_texture({texture, 1, 1, bytes_per_pixel - 1U, bytes_per_pixel, 3}, level_zero.data()),
                "short mip row pitch accepted");
            check(!device.upload_texture({texture, 1, 1, bytes_per_pixel, bytes_per_pixel * 2U, 3}, level_zero.data()),
                "oversized mip payload accepted");

            TextureDesc over_mipped = desc;
            over_mipped.mip_levels = 5;
            check(!device.create_texture(over_mipped, "oversized mip chain"),
                "mip chain beyond source extent accepted");
            TextureDesc render_target = desc;
            render_target.render_target = true;
            check(!device.create_texture(render_target, "mipped render target"),
                "mipped render target accepted");

            device.destroy(texture);
            check(!device.upload_texture({texture, 8, 4, 8 * bytes_per_pixel, level_zero.size(), 0}, level_zero.data()),
                "retired multi-mip texture accepted an upload");
            auto replacement = device.create_texture(desc, "recreated M22 multi-mip source atlas");
            check(replacement && replacement != texture, "multi-mip texture did not recreate with new generation");
            device.destroy(replacement);
        }
        check(device.wait_idle() && device.live_resource_count() == 0,
            "multi-mip texture lifecycle leaked resources");
    }
}

} // namespace

int main()
{
    try {
        test_multi_mip_lifecycle();
        std::cout << "bgfx multi-mip texture contract: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
