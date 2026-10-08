#pragma once
#include <bgfx/bgfx.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

namespace qualification {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }

template<class H> struct Owned {
    H h{bgfx::kInvalidHandle};
    Owned() = default;
    explicit Owned(H value) : h(value) { require(bgfx::isValid(h), "resource creation"); }
    ~Owned() { if (bgfx::isValid(h)) bgfx::destroy(h); }
    Owned(const Owned&) = delete;
    Owned& operator=(const Owned&) = delete;
    Owned(Owned&& other) noexcept : h(std::exchange(other.h,H{bgfx::kInvalidHandle})) {}
};

struct Context {
    explicit Context(const bgfx::SwapChain* swap=nullptr, bgfx::NativeWindowHandleType::Enum type=bgfx::NativeWindowHandleType::Default) {
        bgfx::Init init;
        init.type = bgfx::RendererType::Vulkan;
        init.fallback = false;
        init.debug = true;
        init.swapChain.width = 0;
        init.swapChain.height = 0;
        if(swap) init.swapChain=*swap;
        init.platformData.type=type;
        require(bgfx::init(init), "Vulkan initialization");
        const auto* caps = bgfx::getCaps();
        if (bgfx::getRendererType() != bgfx::RendererType::Vulkan || caps->limits.maxViews <= 250 ||
            !bgfx::isTextureValid(0,false,1,bgfx::TextureFormat::RGBA8,BGFX_TEXTURE_RT) ||
            !bgfx::isTextureValid(0,false,1,bgfx::TextureFormat::D24S8,BGFX_TEXTURE_RT) ||
            !bgfx::isTextureValid(0,false,1,bgfx::TextureFormat::RGBA8,BGFX_TEXTURE_BLIT_DST|BGFX_TEXTURE_READ_BACK)) {
            bgfx::shutdown();
            throw std::runtime_error("required target/readback formats or view capacity unavailable");
        }
        std::printf("GPU: Vulkan vendor=%04x device=%04x maxViews=%u maxDraws=%u\n", caps->vendorId, caps->deviceId, caps->limits.maxViews, caps->limits.maxDrawCalls);
        std::printf("GPU: handles textures=%u vertices=%u indices=%u uniform-reservation=%u resource-reservation=%u\n",
            caps->limits.maxTextures,caps->limits.maxVertexBuffers,caps->limits.maxIndexBuffers,
            caps->limits.minUniformBufferSize,caps->limits.minResourceCbSize);
    }
    ~Context() { bgfx::shutdown(); }
};

bgfx::ShaderHandle shader(const std::string& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    require(bool(stream), "shader file open");
    const auto size = stream.tellg();
    require(size > 0 && size < (1 << 24), "shader file size");
    std::vector<char> data(static_cast<std::size_t>(size));
    stream.seekg(0);
    stream.read(data.data(), size);
    require(bool(stream), "shader file read");
    return bgfx::createShader(bgfx::copy(data.data(), static_cast<uint32_t>(data.size())));
}

struct Programs {
    Owned<bgfx::ShaderHandle> vs, fs_solid, fs_texture, fs_terrain, fs_flat, vs_project, fs_project, fs_array;
    Owned<bgfx::ProgramHandle> solid, texture, terrain, flat, project, array;
    Owned<bgfx::UniformHandle> color, uv, sampler, detail, projection, layer;
    explicit Programs(const std::string& dir)
        : vs(shader(dir + "/vs_qualify.bin")), fs_solid(shader(dir + "/fs_solid.bin")),
          fs_texture(shader(dir + "/fs_texture.bin")),
          fs_terrain(shader(dir + "/fs_terrain.bin")), fs_flat(shader(dir + "/fs_flat.bin")),
          vs_project(shader(dir + "/vs_project.bin")), fs_project(shader(dir + "/fs_project.bin")),
          fs_array(shader(dir + "/fs_array.bin")),
          solid(bgfx::createProgram(vs.h, fs_solid.h)), texture(bgfx::createProgram(vs.h, fs_texture.h)),
          terrain(bgfx::createProgram(vs.h, fs_terrain.h)), flat(bgfx::createProgram(vs.h, fs_flat.h)),
          project(bgfx::createProgram(vs_project.h, fs_project.h)),
          array(bgfx::createProgram(vs.h, fs_array.h)),
          color(bgfx::createUniform("u_color", bgfx::UniformType::Vec4)),
          uv(bgfx::createUniform("u_uv", bgfx::UniformType::Vec4)),
          sampler(bgfx::createUniform("s_color", bgfx::UniformType::Sampler)),
          detail(bgfx::createUniform("s_detail", bgfx::UniformType::Sampler)),
          projection(bgfx::createUniform("u_project", bgfx::UniformType::Mat4)),
          layer(bgfx::createUniform("u_layer", bgfx::UniformType::Vec4)) {}
};

struct Vertex { float x,y,z; uint32_t color; float u,v; };
struct Quad {
    Owned<bgfx::VertexBufferHandle> vertices;
    Owned<bgfx::IndexBufferHandle> indices;
    static bgfx::VertexBufferHandle make(float z, uint32_t packed) {
        const Vertex data[] = {{-1,-1,z,packed,0,0},{1,-1,z,packed,1,0},{-1,1,z,packed,0,1},{1,1,z,packed,1,1}};
        bgfx::VertexLayout layout;
        layout.begin().add(bgfx::Attrib::Position,3,bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,4,bgfx::AttribType::Uint8,true)
            .add(bgfx::Attrib::TexCoord0,2,bgfx::AttribType::Float).end();
        return bgfx::createVertexBuffer(bgfx::copy(data, sizeof(data)), layout);
    }
    explicit Quad(float z=.5f, uint32_t packed=0xffffffff) : vertices(make(z,packed)) {
        const uint16_t data[] = {0,1,2,1,3,2};
        indices.h=bgfx::createIndexBuffer(bgfx::copy(data,sizeof(data)));
        require(bgfx::isValid(indices.h), "index buffer");
    }
};

struct Pixel { uint8_t r,g,b,a; };
struct Target {
    uint16_t w,h;
    Owned<bgfx::TextureHandle> color,depth,readback;
    Owned<bgfx::FrameBufferHandle> framebuffer;
    Target(uint16_t width=64,uint16_t height=64) : w(width),h(height),
        color(bgfx::createTexture2D(w,h,false,1,bgfx::TextureFormat::RGBA8,BGFX_TEXTURE_RT)),
        depth(bgfx::createTexture2D(w,h,false,1,bgfx::TextureFormat::D24S8,BGFX_TEXTURE_RT)),
        readback(bgfx::createTexture2D(w,h,false,1,bgfx::TextureFormat::RGBA8,BGFX_TEXTURE_BLIT_DST|BGFX_TEXTURE_READ_BACK)) {
        bgfx::TextureHandle attachments[]={color.h,depth.h};
        framebuffer.h=bgfx::createFrameBuffer(2,attachments,false);
        require(bgfx::isValid(framebuffer.h), "framebuffer");
    }
    void view(uint16_t id,uint16_t clear=BGFX_CLEAR_NONE,uint32_t rgba=0x000000ff,float z=1,uint8_t stencil=0) {
        bgfx::resetView(id);
        bgfx::setViewMode(id,bgfx::ViewMode::Sequential);
        bgfx::setViewFrameBuffer(id,framebuffer.h);
        bgfx::setViewRect(id,0,0,w,h);
        bgfx::setViewClear(id,clear,rgba,z,stencil);
    }
    std::vector<Pixel> pixels() {
        bgfx::blit(250,bgfx::TextureRegion{.handle=readback.h},bgfx::TextureRegion{.handle=color.h});
        bgfx::frame();
        std::vector<Pixel> data(std::size_t(w)*h);
        const auto ready=bgfx::read(bgfx::TextureRegion{.handle=readback.h},data.data());
        uint32_t frame=0;
        for(int attempt=0;attempt<32; ++attempt) {
            frame=bgfx::frame();
            if(frame>=ready) break;
        }
        require(frame>=ready,"readback completion budget");
        return data;
    }
};

constexpr uint64_t write=BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A;
constexpr uint32_t keep=BGFX_STENCIL_OP_FAIL_S_KEEP|BGFX_STENCIL_OP_FAIL_Z_KEEP|BGFX_STENCIL_OP_PASS_Z_KEEP;

void draw(const Programs& p,const Quad& q,uint16_t view,std::array<float,4> color,
          uint64_t state=write,uint32_t front=BGFX_STENCIL_NONE,uint32_t back=BGFX_STENCIL_NONE,
          bgfx::TextureHandle texture=BGFX_INVALID_HANDLE,uint8_t mip=0,uint8_t count=1,
          uint32_t sampler=BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP,
          std::array<float,4> uv={1,1,0,0},uint8_t maxLod=UINT8_MAX,
          bgfx::ProgramHandle program=BGFX_INVALID_HANDLE) {
    bgfx::setVertexBuffer(0,q.vertices.h);
    bgfx::setIndexBuffer(q.indices.h);
    bgfx::setUniform(p.color.h,color.data());
    bgfx::setUniform(p.uv.h,uv.data());
    bgfx::setState(state);
    bgfx::setStencil(front,back);
    if(bgfx::isValid(texture)) bgfx::setTexture(0,p.sampler.h,texture,0,1,mip,count,sampler,0,maxLod);
    bgfx::submit(view,bgfx::isValid(program)?program:(bgfx::isValid(texture)?p.texture.h:p.solid.h));
}

bool close(Pixel a,Pixel b,int tolerance=2) {
    return std::abs(int(a.r)-b.r)<=tolerance && std::abs(int(a.g)-b.g)<=tolerance &&
        std::abs(int(a.b)-b.b)<=tolerance && std::abs(int(a.a)-b.a)<=tolerance;
}
void pixel(const std::vector<Pixel>& data,const Target& target,int x,int y,Pixel expected,const char* message) {
    const auto actual=data[std::size_t(y)*target.w+x];
    if(!close(actual,expected)) {
        std::fprintf(stderr,"FAIL: %s at %d,%d got %u,%u,%u,%u expected %u,%u,%u,%u\n",message,x,y,actual.r,actual.g,actual.b,actual.a,expected.r,expected.g,expected.b,expected.a);
        throw std::runtime_error(message);
    }
}

} // namespace qualification
