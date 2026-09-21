#include <bgfx/bgfx.h>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

struct Vertex { float x, y, z; uint32_t abgr; };

static bgfx::ShaderHandle loadShader(const char* path)
{
    std::ifstream stream(path, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(stream)), {});
    if (bytes.empty()) return BGFX_INVALID_HANDLE;
    return bgfx::createShader(bgfx::copy(bytes.data(), uint32_t(bytes.size())));
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s <vs_cubes.bin> <fs_cubes.bin>\n", argv[0]);
        return 2;
    }
    constexpr uint16_t width = 160;
    constexpr uint16_t height = 120;
    bgfx::Init init;
    init.type = bgfx::RendererType::Vulkan;
    init.fallback = false;
    init.swapChain.width = 0;
    init.swapChain.height = 0;
    if (!bgfx::init(init)) {
        std::fprintf(stderr, "bgfx Vulkan initialization failed\n");
        return 2;
    }

    auto color = bgfx::createTexture2D(width, height, false, 1,
        bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT);
    auto depth = bgfx::createTexture2D(width, height, false, 1,
        bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT);
    const bgfx::TextureHandle attachments[] = {color, depth};
    auto framebuffer = bgfx::createFrameBuffer(2, attachments, false);
    auto readback = bgfx::createTexture2D(width, height, false, 1,
        bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);
    if (!bgfx::isValid(framebuffer) || !bgfx::isValid(readback)) {
        std::fprintf(stderr, "attachment creation failed\n");
        return 3;
    }

    bgfx::VertexLayout layout;
    layout.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true).end();
    const Vertex vertices[] = {
        {-1, 1, .5f, 0xff00ff00}, {1, 1, .5f, 0xff00ff00},
        {-1, -1, .5f, 0xff00ff00}, {1, -1, .5f, 0xff00ff00}
    };
    const uint16_t indices[] = {0, 1, 2, 1, 3, 2};
    auto vertexBuffer = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), layout);
    auto indexBuffer = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
    auto vs = loadShader(argv[1]);
    auto fs = loadShader(argv[2]);
    auto program = bgfx::createProgram(vs, fs, true);
    if (!bgfx::isValid(program)) return 4;

    // Two ordered views share the same attachments and one bgfx frame.
    bgfx::setViewFrameBuffer(0, framebuffer);
    bgfx::setViewRect(0, 0, 0, width, height);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
        0xff0000ff, 0.25f, 7);
    bgfx::touch(0);
    bgfx::setViewFrameBuffer(1, framebuffer);
    bgfx::setViewRect(1, 40, 30, 80, 60);
    bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
        0x0000ffff, 1.0f, 0);
    bgfx::touch(1);
    bgfx::blit(3, {.handle = readback}, {.handle = color});
    uint32_t currentFrame = bgfx::frame();

    std::vector<uint8_t> pixels(width * height * 4);
    const uint32_t expectedFrame = bgfx::read({.handle = readback}, pixels.data());
    while (currentFrame < expectedFrame) currentFrame = bgfx::frame();

    const auto check = [&](int x, int y, bool blue) {
        const uint8_t* p = &pixels[(y * width + x) * 4];
        std::printf("pixel(%d,%d)=%02x%02x%02x%02x expected=%s\n",
            x, y, p[0], p[1], p[2], p[3], blue ? "blue" : "red");
        return blue ? p[0] == 0xff && p[1] == 0 && p[2] == 0 && p[3] == 0xff
                    : p[0] == 0 && p[1] == 0 && p[2] == 0xff && p[3] == 0xff;
    };
    bool ok = check(0, 0, false) && check(39, 30, false) &&
        check(40, 30, true) && check(119, 89, true) &&
        check(120, 89, false) && check(159, 119, false);

    const auto drawAndCheck = [&](bool stencil) {
        bgfx::touch(0);
        bgfx::touch(1);
        bgfx::setViewFrameBuffer(2, framebuffer);
        bgfx::setViewRect(2, 0, 0, width, height);
        bgfx::setViewClear(2, BGFX_CLEAR_NONE);
        bgfx::setVertexBuffer(0, vertexBuffer);
        bgfx::setIndexBuffer(indexBuffer);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
            (stencil ? BGFX_STATE_DEPTH_TEST_ALWAYS : BGFX_STATE_DEPTH_TEST_LESS));
        if (stencil) bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL |
            BGFX_STENCIL_FUNC_REF(0) | BGFX_STENCIL_FUNC_RMASK(0xff) |
            BGFX_STENCIL_OP_PASS_Z_KEEP);
        bgfx::submit(2, program);
        bgfx::blit(3, {.handle = readback}, {.handle = color});
        currentFrame = bgfx::frame();
        const uint32_t ready = bgfx::read({.handle = readback}, pixels.data());
        while (currentFrame < ready) currentFrame = bgfx::frame();
        const auto green = [&](int x, int y) {
            const uint8_t* p = &pixels[(y * width + x) * 4];
            std::printf("%s pixel(%d,%d)=%02x%02x%02x%02x\n",
                stencil ? "stencil" : "depth", x, y, p[0], p[1], p[2], p[3]);
            return p[0] == 0 && p[1] == 0xff && p[2] == 0 && p[3] == 0xff;
        };
        return check(0, 0, false) && green(40, 30) && green(119, 89) &&
            check(120, 89, false);
    };
    ok = drawAndCheck(false) && ok;
    ok = drawAndCheck(true) && ok;
    bgfx::destroy(program);
    bgfx::destroy(vertexBuffer);
    bgfx::destroy(indexBuffer);
    bgfx::destroy(framebuffer);
    bgfx::destroy(color);
    bgfx::destroy(depth);
    bgfx::destroy(readback);
    bgfx::shutdown();
    return ok ? 0 : 1;
}
