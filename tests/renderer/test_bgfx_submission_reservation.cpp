#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include <bx/hash.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>

#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace {
using Command = bgfx::BoundedSubmissionCommand;
using Receipt = bgfx::BoundedSubmissionReceipt;
using Desc = bgfx::BoundedSubmissionDesc;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
Desc budget(std::uint64_t generation = 1)
{ return {generation,4096,4096,256,4096,64u<<20}; }
bool same(const Receipt& a, const Receipt& b)
{
    return a.context==b.context && a.sequence==b.sequence && a.generation==b.generation &&
        a.frameBefore==b.frameBefore && a.frameAfter==b.frameAfter && a.draws==b.draws && a.views==b.views
        && a.bindingsBefore==b.bindingsBefore && a.bindingsAfter==b.bindingsAfter;
}
std::vector<std::pair<std::uint64_t,std::uint32_t>> invalid_states()
{
    using Limits=bgfx::BoundedSubmissionLimits;
    std::vector<std::pair<std::uint64_t,std::uint32_t>> values;
    values.emplace_back(std::uint64_t(9)<<BGFX_STATE_DEPTH_TEST_SHIFT,0);
    values.emplace_back(std::uint64_t(3)<<BGFX_STATE_CULL_SHIFT,0);
    values.emplace_back(std::uint64_t(5)<<BGFX_STATE_PT_SHIFT,0);
    for(unsigned shift=0;shift<16;shift+=4)
        values.emplace_back(std::uint64_t(14)<<(BGFX_STATE_BLEND_SHIFT+shift),0);
    for(unsigned shift=0;shift<6;shift+=3)
        values.emplace_back(std::uint64_t(5)<<(BGFX_STATE_BLEND_EQUATION_SHIFT+shift),0);
    for(unsigned target=0;target<3;++target) {
        values.emplace_back(BGFX_STATE_BLEND_INDEPENDENT,std::uint32_t(14)<<(target*11));
        values.emplace_back(BGFX_STATE_BLEND_INDEPENDENT,std::uint32_t(14)<<(target*11+4));
        // The third target has only two equation bits in the 32-bit encoding;
        // its entire representable range (0..3) is valid.
        if(target<2) values.emplace_back(BGFX_STATE_BLEND_INDEPENDENT,std::uint32_t(5)<<(target*11+8));
    }
    for(unsigned bit=0;bit<64;++bit)
        if(!Limits::acceptsState(std::uint64_t(1)<<bit,0))
            values.emplace_back(std::uint64_t(1)<<bit,0);
    return values;
}
std::vector<std::uint32_t> invalid_stencils()
{
    return {std::uint32_t(9)<<BGFX_STENCIL_TEST_SHIFT,
        std::uint32_t(8)<<BGFX_STENCIL_OP_FAIL_S_SHIFT,
        std::uint32_t(8)<<BGFX_STENCIL_OP_FAIL_Z_SHIFT,
        std::uint32_t(8)<<BGFX_STENCIL_OP_PASS_Z_SHIFT};
}
std::vector<std::uint32_t> invalid_samplers()
{
    using Limits=bgfx::BoundedSubmissionLimits;
    std::vector<std::uint32_t> values{std::uint32_t(3)<<BGFX_SAMPLER_MIN_SHIFT,
        std::uint32_t(3)<<BGFX_SAMPLER_MAG_SHIFT,std::uint32_t(9)<<BGFX_SAMPLER_COMPARE_SHIFT};
    for(unsigned bit=0;bit<32;++bit)
        if(!Limits::acceptsSamplerFlags(std::uint32_t(1)<<bit)) values.push_back(std::uint32_t(1)<<bit);
    return values;
}
void encoded_cpu()
{
    using Limits=bgfx::BoundedSubmissionLimits;
    const std::uint64_t state=BGFX_STATE_WRITE_MASK|BGFX_STATE_DEPTH_TEST_ALWAYS
        |(std::uint64_t(0xdddd)<<BGFX_STATE_BLEND_SHIFT)
        |(std::uint64_t(4|(4<<3))<<BGFX_STATE_BLEND_EQUATION_SHIFT)
        |BGFX_STATE_CULL_CCW|BGFX_STATE_PT_POINTS|BGFX_STATE_ALPHA_REF(255)
        |BGFX_STATE_POINT_SIZE(15)|BGFX_STATE_MSAA|BGFX_STATE_LINEAA
        |BGFX_STATE_CONSERVATIVE_RASTER|BGFX_STATE_FRONT_CCW
        |BGFX_STATE_BLEND_INDEPENDENT|BGFX_STATE_BLEND_ALPHA_TO_COVERAGE;
    const std::uint32_t independent=std::uint32_t(13|(13<<4)|(4<<8))
        |(std::uint32_t(13|(13<<4)|(4<<8))<<11)|(std::uint32_t(13|(13<<4)|(3<<8))<<22);
    check(Limits::acceptsState(state,independent),"exact encoded state maxima rejected");
    for(const auto [flags,rgba]:invalid_states())
        check(!Limits::acceptsState(flags|BGFX_STATE_WRITE_RGB,rgba),"encoded state bound+1/gap admitted");
    const std::uint32_t stencil=BGFX_STENCIL_TEST_ALWAYS|BGFX_STENCIL_OP_FAIL_S_INVERT
        |BGFX_STENCIL_OP_FAIL_Z_INVERT|BGFX_STENCIL_OP_PASS_Z_INVERT
        |BGFX_STENCIL_FUNC_REF(255)|BGFX_STENCIL_FUNC_RMASK(255);
    check(Limits::acceptsStencil(stencil),"exact stencil maxima rejected");
    for(const auto flags:invalid_stencils())
        check(!Limits::acceptsStencil(flags|BGFX_STENCIL_FUNC_REF(255)),"stencil bound+1 admitted");
    const std::uint32_t sampler=BGFX_SAMPLER_UVW_BORDER|BGFX_SAMPLER_MIN_ANISOTROPIC
        |BGFX_SAMPLER_MAG_ANISOTROPIC|BGFX_SAMPLER_MIP_POINT|BGFX_SAMPLER_COMPARE_ALWAYS
        |BGFX_SAMPLER_BORDER_COLOR(15)|BGFX_SAMPLER_SAMPLE_STENCIL|BGFX_SAMPLER_SRGB;
    check(Limits::acceptsSamplerFlags(sampler),"exact sampler maxima rejected");
    for(const auto flags:invalid_samplers())
        check(!Limits::acceptsSamplerFlags(flags|BGFX_SAMPLER_U_CLAMP),"sampler bound+1/gap admitted");
    const std::uint32_t swap=BGFX_SWAP_CHAIN_MSAA_X16|BGFX_SWAP_CHAIN_FULLSCREEN
        |BGFX_SWAP_CHAIN_SRGB_BACKBUFFER|BGFX_SWAP_CHAIN_HDR10|BGFX_SWAP_CHAIN_HIDPI
        |BGFX_SWAP_CHAIN_TRANSPARENT_BACKBUFFER;
    check(Limits::acceptsSwapChainFlags(swap),"exact swapchain flags maxima rejected");
    bgfx::SwapChain descriptor; descriptor.flags=swap;
    descriptor.formatColor=bgfx::TextureFormat::Count;
    descriptor.formatDepthStencil=bgfx::TextureFormat::Count;
    check(Limits::acceptsSwapChainEncoding(descriptor),"exact swapchain enum sentinels rejected");
    descriptor.formatColor=static_cast<bgfx::TextureFormat::Enum>(bgfx::TextureFormat::Count+1);
    check(!Limits::acceptsSwapChainEncoding(descriptor),"swapchain color enum bound+1 admitted");
    descriptor.formatColor=bgfx::TextureFormat::Count;
    descriptor.formatDepthStencil=static_cast<bgfx::TextureFormat::Enum>(bgfx::TextureFormat::Count+1);
    check(!Limits::acceptsSwapChainEncoding(descriptor),"swapchain depth enum bound+1 admitted");
    check(!Limits::acceptsSwapChainFlags(std::uint32_t(5)<<BGFX_SWAP_CHAIN_MSAA_SHIFT),"swapchain MSAA bound+1 admitted");
    for(unsigned bit=0;bit<32;++bit)
        if((swap& (std::uint32_t(1)<<bit))==0
           &&(BGFX_SWAP_CHAIN_MSAA_MASK & (std::uint32_t(1)<<bit))==0)
            check(!Limits::acceptsSwapChainFlags(swap|(std::uint32_t(1)<<bit)),"swapchain unknown bit admitted");
}
void cpu()
{
    encoded_cpu();
    for (unsigned generation=1; generation<=2; ++generation) {
        auto good=budget(generation);
        check(bgfx::BoundedSubmissionLimits::accepts(good,4096), "exact CPU limits rejected");
        for (unsigned category=0; category<10; ++category) {
            auto bad=good; unsigned count=1;
            switch(category) {
            case 0: bad.generation=0; break;
            case 1: bad.commands=0; break;
            case 2: ++bad.commands; break;
            case 3: ++bad.draws; break;
            case 4: ++bad.views; break;
            case 5: ++bad.resources; break;
            case 6: bad.bytes=0; break;
            case 7: ++bad.bytes; break;
            case 8: count=0; break;
            case 9: count=4097; break;
            }
            check(!bgfx::BoundedSubmissionLimits::accepts(bad,count), "CPU malformed/bound+1 admitted");
        }
        Command command; command.kind=Command::CompleteFrame;
        const Receipt baseline{91,92,93,94,95,96,97};
        auto receipt=baseline;
        check(!bgfx::submitBounded(good,&command,1,receipt) && same(receipt,baseline),
              "absent runtime consumed receipt");
        check(bgfx::boundedSubmissionPhase()==bgfx::BoundedSubmissionPhase::Idle, "absent runtime retained phase");
    }
}
struct Allocator final : bx::AllocatorI {
    bx::DefaultAllocator inner;
    std::atomic<std::uint64_t> live{0}, replay_allocations{0};
    std::uint64_t preparation_allocations=0, fail_at=0;
    bool reenter=false, reentry_rejected=false, mutate_after_copy=false, changed_input=false;
    float* external_payload=nullptr;
    Command* external_commands=nullptr;
    unsigned external_count=0;
    Desc external_desc{};
    Receipt recursive_receipt{81,82,83,84,85,86,87};
    void* realloc(void* ptr, size_t size, size_t align, const char* file, std::uint32_t line) override
    {
        const auto phase=bgfx::boundedSubmissionPhase();
        if (size && phase==bgfx::BoundedSubmissionPhase::Replaying)
            ++replay_allocations;
        if (size && phase==bgfx::BoundedSubmissionPhase::Preparing) {
            ++preparation_allocations;
            if(mutate_after_copy && preparation_allocations==3) {
                // The complete immutable slab already exists before model
                // allocation. Its parameters and payload must not be reread.
                mutate_after_copy=false; changed_input=true;
                external_commands[1].vertices=0;
                external_commands[1].uniformCount=0;
                external_payload[0]=0;
            }
            if (reenter) {
                reenter=false;
                const auto baseline=recursive_receipt;
                reentry_rejected=!bgfx::submitBounded(external_desc,external_commands,external_count,recursive_receipt)
                    && same(baseline,recursive_receipt);
            }
            if (fail_at && preparation_allocations==fail_at)
                return nullptr;
        }
        auto* result=inner.realloc(ptr,size,align,file,line);
        if (!ptr && result) ++live;
        if (ptr && !size) --live;
        return result;
    }
    void fault(std::uint64_t at) { preparation_allocations=0; fail_at=at; }
};
std::vector<char> file(const std::filesystem::path& path)
{
    std::ifstream in(path,std::ios::binary);
    check(bool(in),"generated shader unavailable");
    return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
}
bgfx::ShaderHandle shader(const char* family)
{
    auto data=file(std::filesystem::path(ZH_BGFX_SHADER_DIR)/(std::string(family)+".bin"));
    check(data.size()>=22,"generated shader header incomplete");
    const auto count=std::uint8_t(data[20])|(unsigned(std::uint8_t(data[21]))<<8);
    std::size_t offset=22;
    // The accepted device normalization changes container CPU identifiers only,
    // not SPIR-V, reflection offsets or sampling bindings.
    for(unsigned i=0; i<count; ++i) {
        check(offset<data.size(),"generated identifier count invalid");
        const auto length=std::uint8_t(data[offset++]);
        check(length && offset+length+10<=data.size(),"generated identifier extent invalid");
        for(unsigned j=0; j<length; ++j) if(data[offset+j]=='.') data[offset+j]='_';
        offset+=length+10;
    }
    return bgfx::createShader(bgfx::copy(data.data(),std::uint32_t(data.size())));
}
struct Scene {
    Allocator& allocator;
    bgfx::ShaderHandle vs=BGFX_INVALID_HANDLE, fs=BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle program=BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vertices=BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle indices=BGFX_INVALID_HANDLE;
    bgfx::UniformHandle viewport=BGFX_INVALID_HANDLE, sampler=BGFX_INVALID_HANDLE, array=BGFX_INVALID_HANDLE;
    bgfx::TextureHandle red=BGFX_INVALID_HANDLE, green=BGFX_INVALID_HANDLE;
    bgfx::TextureHandle color=BGFX_INVALID_HANDLE, capture=BGFX_INVALID_HANDLE;
    bgfx::FrameBufferHandle target=BGFX_INVALID_HANDLE;
    std::array<float,4> viewport_bytes{64,64,0,0};
    std::array<float,4*255> array_bytes{};
    std::array<bgfx::BoundedSubmissionUniform,2> uniforms{};
    bgfx::BoundedSubmissionTexture texture{};
    std::array<Command,3> commands{};
    explicit Scene(Allocator& owner):allocator(owner)
    {
        vs=shader("renderer/video.vert"); fs=shader("renderer/video.frag");
        check(bgfx::isValid(vs)&&bgfx::isValid(fs),"generated native shaders rejected");
        program=bgfx::createProgram(vs,fs,false);
        const auto find=[&](bgfx::ShaderHandle handle,const char* suffix) {
            std::array<bgfx::UniformHandle,64> handles{};
            const auto count=bgfx::getShaderUniforms(handle,handles.data(),handles.size());
            for(unsigned i=0;i<count;++i) {
                bgfx::UniformInfo info{}; bgfx::getUniformInfo(handles[i],info);
                if(std::string(info.name).ends_with(suffix)) return handles[i];
            }
            return bgfx::UniformHandle{std::numeric_limits<std::uint16_t>::max()};
        };
        viewport=find(vs,"viewport"); sampler=find(fs,"movie_texture_image");
        check(bgfx::isValid(program)&&bgfx::isValid(viewport)&&bgfx::isValid(sampler),"generated native uniform unavailable");
        array=bgfx::createUniform("bounded_unused_array",bgfx::UniformType::Vec4,255);
        struct Vertex {float x,y; std::uint32_t color; float u,v;};
        const std::array<Vertex,3> triangle{{{0,0,0xffffffff,0,0},{128,0,0xffffffff,2,0},{0,128,0xffffffff,0,2}}};
        const std::array<std::uint16_t,3> ib{0,1,2};
        bgfx::VertexLayout layout;
        layout.begin().add(bgfx::Attrib::Position,2,bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,4,bgfx::AttribType::Uint8,true)
            .add(bgfx::Attrib::TexCoord0,2,bgfx::AttribType::Float).end();
        vertices=bgfx::createVertexBuffer(bgfx::copy(triangle.data(),sizeof(triangle)),layout);
        indices=bgfx::createIndexBuffer(bgfx::copy(ib.data(),sizeof(ib)));
        const std::array<std::uint8_t,4> red_pixel{255,0,0,255}, green_pixel{0,255,0,255};
        red=bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::RGBA8,0,bgfx::copy(red_pixel.data(),4));
        green=bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::RGBA8,0,bgfx::copy(green_pixel.data(),4));
        color=bgfx::createTexture2D(64,64,false,1,bgfx::TextureFormat::BGRA8,BGFX_TEXTURE_RT);
        capture=bgfx::createTexture2D(64,64,false,1,bgfx::TextureFormat::BGRA8,BGFX_TEXTURE_BLIT_DST|BGFX_TEXTURE_READ_BACK);
        target=bgfx::createFrameBuffer(1,&color,false);
        check(bgfx::isValid(vertices)&&bgfx::isValid(indices)&&bgfx::isValid(array)&&bgfx::isValid(red)
            &&bgfx::isValid(green)&&bgfx::isValid(color)&&bgfx::isValid(capture)&&bgfx::isValid(target),"generated native resource unavailable");
        uniforms[0]={viewport,1,sizeof(viewport_bytes),viewport_bytes.data()};
        uniforms[1]={array,255,sizeof(array_bytes),array_bytes.data()};
        texture={sampler,red,BGFX_SAMPLER_MIN_POINT|BGFX_SAMPLER_MAG_POINT|BGFX_SAMPLER_MIP_POINT,8};
        commands[0].kind=Command::View; commands[0].view=1; commands[0].framebuffer=target;
        commands[0].width=commands[0].height=64; commands[0].clearFlags=BGFX_CLEAR_COLOR;
        commands[0].clearRgba=0x000000ff;
        auto& draw=commands[1]; draw.kind=Command::Draw; draw.view=1; draw.program=program;
        draw.vertex=vertices; draw.index=indices; draw.vertices=draw.indices=3;
        draw.uniforms=uniforms.data(); draw.uniformCount=uniforms.size();
        draw.textures=&texture; draw.textureCount=1; draw.state=BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A;
        commands[2].kind=Command::CompleteFrame;
        bgfx::frame(); bgfx::frame();
    }
    std::vector<std::uint8_t> pixels(bgfx::TextureHandle override_source=BGFX_INVALID_HANDLE)
    {
        std::vector<std::uint8_t> pixels(64*64*4);
        bgfx::setViewRect(255,0,0,64,64);
        bgfx::TextureRegion destination{}, source{};
        destination.handle=capture; source.handle=bgfx::isValid(override_source)?override_source:color;
        bgfx::blit(255,destination,source);
        bgfx::frame();
        const auto ready=bgfx::read(destination,pixels.data());
        unsigned frames=0;
        while(bgfx::frame()<ready) check(++frames<16,"native readback frame bound exceeded");
        bgfx::frame(); // Wait for the retained CPU readback pointer before inspection/destruction.
        return pixels;
    }
    Receipt submit(Desc desc)
    {
        Receipt receipt;
        check(bgfx::submitBounded(desc,commands.data(),commands.size(),receipt),"native admitted manifest rejected");
        check(allocator.replay_allocations==0,"native API replay allocated");
        return receipt;
    }
    void close()
    {
        bgfx::destroy(target); bgfx::destroy(capture); bgfx::destroy(color);
        bgfx::destroy(red); bgfx::destroy(green); bgfx::destroy(vertices); bgfx::destroy(indices);
        bgfx::destroy(program); bgfx::destroy(vs); bgfx::destroy(fs); bgfx::destroy(array);
        bgfx::frame(); bgfx::frame();
    }
};

std::pair<std::uint32_t,std::uint32_t> collision(std::uint16_t red,std::uint16_t green)
{
    // Generated witness of the reviewed pinned binding-key encoding, not a
    // private-header/symbol consumer. The native bounded path must compare
    // full values even when the ordinary 32-bit key is identical.
    struct PackedBinding {
        std::uint32_t flags=0,offset=0,size=UINT32_MAX;
        std::uint16_t first_layer=0,layers=UINT16_MAX,index=0;
        std::uint8_t type=3,format=0,access=0,mip=0,mips=UINT8_MAX,pad=0;
    };
    static_assert(sizeof(PackedBinding)==24&&std::has_unique_object_representations_v<PackedBinding>);
    std::unordered_map<std::uint32_t,std::uint32_t> seen;
    const std::uint32_t point=BGFX_SAMPLER_MIN_POINT|BGFX_SAMPLER_MAG_POINT|BGFX_SAMPLER_MIP_POINT;
    for(std::uint32_t ordinal=1;ordinal<=1048576;++ordinal) {
        const auto pattern=ordinal*2654435761U;
        bx::HashMurmur3 hash; hash.begin(); hash.add(std::uint32_t(0xffff));
        for(unsigned stage=0;stage<16;++stage) {
            PackedBinding binding;
            binding.index=(pattern&(1u<<(stage*2)))?green:red;
            binding.flags=(pattern&(1u<<(stage*2+1)))?point:0;
            hash.add(binding);
        }
        const auto key=hash.end();
        const auto [position,inserted]=seen.emplace(key,pattern);
        if(!inserted&&((position->second^pattern)&(1u<<16))) {
            auto first=position->second,second=pattern;
            if(first&(1u<<16)) std::swap(first,second);
            return {first,second};
        }
    }
    throw std::runtime_error("bounded generated collision search exhausted");
}

struct VideoWindow {
    SDL_Window* window=nullptr;
    VideoWindow()
    {
        check(SDL_Init(SDL_INIT_VIDEO),"physical window video initialization unavailable");
        window=SDL_CreateWindow("generated native reservation",64,64,SDL_WINDOW_VULKAN|SDL_WINDOW_HIDDEN);
        if(!window) { SDL_Quit(); throw std::runtime_error("physical generated window unavailable"); }
    }
    ~VideoWindow() { SDL_DestroyWindow(window); SDL_Quit(); }
    bgfx::SwapChain descriptor(unsigned width,unsigned height)
    {
        bgfx::SwapChain desc; desc.width=width; desc.height=height;
        desc.formatColor=bgfx::TextureFormat::BGRA8;
        desc.formatDepthStencil=bgfx::TextureFormat::Count;
        const auto properties=SDL_GetWindowProperties(window);
        const std::string driver=SDL_GetCurrentVideoDriver();
        if(driver=="x11") {
            desc.nwh=reinterpret_cast<void*>(std::uintptr_t(SDL_GetNumberProperty(properties,SDL_PROP_WINDOW_X11_WINDOW_NUMBER,0)));
            desc.ndt=SDL_GetPointerProperty(properties,SDL_PROP_WINDOW_X11_DISPLAY_POINTER,nullptr);
        } else if(driver=="wayland") {
            desc.nwh=SDL_GetPointerProperty(properties,SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER,nullptr);
            desc.ndt=SDL_GetPointerProperty(properties,SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER,nullptr);
        } else throw std::runtime_error("physical window provider category unsupported");
        check(desc.nwh&&desc.ndt,"physical generated window identity unavailable");
        return desc;
    }
};
void physical()
{
    VideoWindow window;
    std::uint64_t prior_context=0;
    for(unsigned generation=1;generation<=2;++generation) {
        check(SDL_SetWindowSize(window.window,64,64),"physical generation window reset rejected");
        SDL_PumpEvents();
        Allocator allocator;
        bgfx::Init init; init.type=bgfx::RendererType::Vulkan; init.fallback=false;
        init.swapChain.width=init.swapChain.height=0; init.allocator=&allocator;
        if(std::string(SDL_GetCurrentVideoDriver())=="wayland")
            init.platformData.type=bgfx::NativeWindowHandleType::Wayland;
        check(bgfx::init(init),"physical native initialization rejected");
        try {
            Scene scene(allocator);
            auto good=budget(generation);
            const Receipt baseline{91,92,93,94,95,96,97};
            const auto reject=[&](const Desc& desc,const Command* commands,unsigned count) {
                auto receipt=baseline;
                const auto draws=bgfx::getStats()->numDraw;
                check(!bgfx::submitBounded(desc,commands,count,receipt)&&same(receipt,baseline),"negative manifest mutated receipt");
                check(bgfx::boundedSubmissionPhase()==bgfx::BoundedSubmissionPhase::Idle
                    && bgfx::getStats()->numDraw==draws,"negative manifest emitted work");
            };
            for(unsigned category=0;category<22;++category) {
                auto desc=good; auto commands=scene.commands; auto uniforms=scene.uniforms;
                std::array<bgfx::BoundedSubmissionTexture,2> textures{scene.texture,scene.texture};
                switch(category) {
                case 0: desc.generation=0; break;
                case 1: ++desc.commands; break;
                case 2: desc.draws=0; break;
                case 3: desc.views=0; break;
                case 4: desc.bytes=1; break;
                case 5: commands[2].kind=Command::Count; break;
                case 6: commands[1].program=BGFX_INVALID_HANDLE; break;
                case 7: commands[1].firstIndex=UINT32_MAX; break;
                case 8: commands[1].firstVertex=UINT32_MAX; break;
                case 9: commands[0].view=UINT16_MAX; break;
                case 10: commands[0].minDepth=std::numeric_limits<float>::quiet_NaN(); break;
                case 11: commands[0].clearFlags=UINT16_MAX; break;
                case 12: commands[1].uniforms=nullptr; break;
                case 13: uniforms[1]=uniforms[0]; commands[1].uniforms=uniforms.data(); break;
                case 14: uniforms[0].bytes=1; commands[1].uniforms=uniforms.data(); break;
                case 15: commands[1].textures=textures.data(); commands[1].textureCount=2; break;
                case 16: commands[1].vertices=0; break;
                case 17: commands[1].state=UINT64_MAX; break;
                case 18: commands[0].kind=Command::CompleteFrame; break;
                case 19: commands[2].kind=Command::Retire; commands[2].resource=Command::ResourceCount; break;
                case 20: uniforms[0].count=0; commands[1].uniforms=uniforms.data(); break;
                case 21: commands[1].view=2; break;
                }
                reject(desc,commands.data(),commands.size());
            }
            reject(good,nullptr,1); reject(good,scene.commands.data(),0);
            std::atomic<bool> wrong_thread{false};
            std::thread outsider([&]{
                auto receipt=baseline;
                wrong_thread=!bgfx::submitBounded(good,scene.commands.data(),scene.commands.size(),receipt)&&same(receipt,baseline);
            }); outsider.join();
            check(wrong_thread,"foreign API thread acquired manifest");
            auto* extra=bgfx::begin(true);
            check(extra!=nullptr,"extra encoder fixture unavailable");
            reject(good,scene.commands.data(),scene.commands.size());
            bgfx::end(extra); bgfx::frame();

            // Private copies and capacity replacements all precede publication.
            // Fail each checked allocation at the same untouched native baseline.
            std::uint64_t faults=0;
            for(std::uint64_t boundary=1;boundary<128;++boundary) {
                allocator.fault(boundary);
                auto receipt=baseline;
                const auto resources=allocator.live.load();
                const bool accepted=bgfx::submitBounded(good,scene.commands.data(),scene.commands.size(),receipt);
                if(accepted) {
                    check(boundary>1 && allocator.preparation_allocations<boundary,"fault injection missed its boundary");
                    check(receipt.context>prior_context&&receipt.sequence==1&&receipt.generation==generation
                        &&receipt.frameAfter==receipt.frameBefore+1,"success receipt identity/order differs");
                    prior_context=receipt.context;
                    faults=boundary-1;
                    break;
                }
                check(same(receipt,baseline)&&allocator.live==resources
                    &&bgfx::boundedSubmissionPhase()==bgfx::BoundedSubmissionPhase::Idle,"failed allocation published storage or retained owner");
            }
            check(faults>=4&&allocator.replay_allocations==0,"native allocation witness incomplete");
            allocator.fault(0);
            const auto red_pixels=scene.pixels();
            const auto center=(32*64+32)*4;
            check(red_pixels[center+2]==255&&red_pixels[center+1]==0,"physical admitted sampled bytes differ");
            scene.texture.texture=scene.green;
            allocator.external_commands=scene.commands.data(); allocator.external_count=scene.commands.size();
            allocator.external_desc=good; allocator.reenter=true;
            const auto retry=scene.submit(good);
            check(allocator.reentry_rejected && retry.sequence==2&&retry.context==prior_context,"reentry consumed owner or sequence");
            const auto green_pixels=scene.pixels();
            check(green_pixels[center+1]==255&&green_pixels[center+2]==0,"physical clean retry sampled bytes differ");

            std::uint64_t sequence=retry.sequence;
            const auto accept=[&](const Command* commands,unsigned count,Desc desc) {
                Receipt receipt;
                check(bgfx::submitBounded(desc,commands,count,receipt),"expanded admitted manifest rejected");
                check(receipt.context==prior_context && receipt.sequence==++sequence
                    && receipt.generation==generation && allocator.replay_allocations==0,
                    "expanded receipt sequence or API allocator differs");
                return receipt;
            };

            // A fully admitted snapshot does not reread caller commands or
            // payloads after the native allocator's model-allocation boundary.
            allocator.fault(0); allocator.mutate_after_copy=true;
            allocator.external_payload=scene.viewport_bytes.data();
            const auto snapshot=accept(scene.commands.data(),scene.commands.size(),good);
            check(allocator.changed_input&&snapshot.draws==1,"immutable-copy mutation control was not reached");
            scene.commands[1].vertices=3; scene.commands[1].uniformCount=scene.uniforms.size();
            scene.viewport_bytes={64,64,0,0};
            check(scene.pixels()==green_pixels,"immutable snapshot reread caller payload");

            // A valid red clear/draw before a late invalid encoded value must
            // never reach either reservation allocation or the accepted target.
            const auto reject_encoded=[&](const Command& invalid) {
                auto red_binding=scene.texture; red_binding.texture=scene.red;
                std::array<Command,4> mixed{};
                mixed[0]=scene.commands[0]; mixed[0].clearRgba=0xff0000ff;
                mixed[1]=scene.commands[1]; mixed[1].textures=&red_binding;
                mixed[2]=invalid; mixed[3].kind=Command::CompleteFrame;
                allocator.fault(0); const auto live=allocator.live.load();
                reject(good,mixed.data(),mixed.size());
                check(allocator.preparation_allocations==0&&allocator.replay_allocations==0
                    &&allocator.live==live,"encoded rejection allocated or published work");
            };
            for(const auto [state,rgba]:invalid_states()) {
                auto invalid=scene.commands[1];
                invalid.state=state|BGFX_STATE_WRITE_RGB; invalid.rgba=rgba;
                reject_encoded(invalid);
            }
            for(const auto stencil:invalid_stencils())
                for(unsigned face=0;face<2;++face) {
                    auto invalid=scene.commands[1];
                    if(face==0) invalid.stencilFront=stencil;
                    else invalid.stencilBack=stencil;
                    reject_encoded(invalid);
                }
            for(const auto flags:invalid_samplers()) {
                auto invalid=scene.commands[1]; auto texture=scene.texture;
                texture.flags=flags|BGFX_SAMPLER_U_CLAMP; invalid.textures=&texture;
                reject_encoded(invalid);
            }
            {
                auto invalid=scene.commands[1]; auto texture=scene.texture;
                texture.stage=static_cast<std::uint8_t>(bgfx::getCaps()->limits.maxTextureSamplers);
                invalid.textures=&texture; reject_encoded(invalid);
            }
            for(unsigned category=0;category<5;++category) {
                Command invalid;
                if(category==0) {
                    invalid=scene.commands[0]; invalid.clearFlags=BGFX_CLEAR_COLOR|0x8000;
                } else {
                    invalid.kind=Command::ResizeSwapChain;
                    invalid.framebuffer=scene.target;
                    if(category==1) invalid.swapChain.flags=std::uint32_t(5)<<BGFX_SWAP_CHAIN_MSAA_SHIFT;
                    if(category==2) invalid.swapChain.flags=BGFX_SWAP_CHAIN_MSAA_X16|0x80000000;
                    if(category==3) invalid.swapChain.formatColor=static_cast<bgfx::TextureFormat::Enum>(bgfx::TextureFormat::Count+1);
                    if(category==4) invalid.swapChain.formatDepthStencil=static_cast<bgfx::TextureFormat::Enum>(bgfx::TextureFormat::Count+1);
                }
                reject_encoded(invalid);
            }
            check(scene.pixels()==green_pixels,"encoded rejection changed accepted pixels");

            // Late recognized rejection must not queue even a partial clear or
            // draw against the previously accepted target; an ordinary read
            // afterward would flush any such leaked native prefix.
            auto red_binding=scene.texture; red_binding.texture=scene.red;
            std::array<Command,4> late{};
            late[0]=scene.commands[0]; late[0].clearRgba=0xff0000ff;
            late[1]=scene.commands[1]; late[1].textures=&red_binding;
            late[2].kind=Command::Retire; late[2].resource=Command::Texture;
            late[2].resourceIndex=UINT16_MAX; late[3].kind=Command::CompleteFrame;
            reject(good,late.data(),late.size());
            check(scene.pixels()==green_pixels,"late rejection emitted a partial accepted-target clear/draw");
            // The real presentation owner also admits a non-indexed triangle.
            auto nonindexed=scene.commands;
            nonindexed[1].index=BGFX_INVALID_HANDLE; nonindexed[1].indices=0;
            const auto nonindexed_receipt=accept(nonindexed.data(),nonindexed.size(),good);
            check(nonindexed_receipt.draws==1,"non-indexed native draw omitted");
            check(scene.pixels()==green_pixels,"non-indexed physical bytes differ");

            // Preserve a queued ordinary view/clear and its cache prefix.
            auto prefix_color=bgfx::createTexture2D(64,64,false,1,bgfx::TextureFormat::BGRA8,BGFX_TEXTURE_RT);
            auto prefix_target=bgfx::createFrameBuffer(1,&prefix_color,false);
            check(bgfx::isValid(prefix_color)&&bgfx::isValid(prefix_target),"ordinary prefix target unavailable");
            bgfx::setViewFrameBuffer(0,prefix_target); bgfx::setViewRect(0,0,0,64,64);
            bgfx::setViewClear(0,BGFX_CLEAR_COLOR,0x123456ff); bgfx::touch(0);
            const auto prefix=accept(scene.commands.data(),scene.commands.size(),good);
            check(prefix.bindingsBefore>=1&&prefix.bindingsAfter==prefix.bindingsBefore+1,
                  "ordinary binding prefix was discarded or duplicated");
            const auto prefix_pixels=scene.pixels(prefix_color);
            check(prefix_pixels[center]==0x56&&prefix_pixels[center+1]==0x34&&prefix_pixels[center+2]==0x12,
                  "ordinary queued clear was removed");
            bgfx::destroy(prefix_target); bgfx::destroy(prefix_color);
            bgfx::frame(); bgfx::frame();

            // Native duplicate indices still represent independently acquired refs.
            std::array<bgfx::ShaderHandle,2> alias_vs{},alias_fs{};
            std::array<bgfx::ProgramHandle,2> alias_program{};
            std::vector<Command> retirements;
            const auto retirement=[&](Command::Resource type,std::uint16_t handle) {
                Command command; command.kind=Command::Retire; command.resource=type; command.resourceIndex=handle;
                retirements.push_back(command);
            };
            for(unsigned i=0;i<2;++i) {
                alias_vs[i]=shader("renderer/video.vert"); alias_fs[i]=shader("renderer/video.frag");
                alias_program[i]=bgfx::createProgram(alias_vs[i],alias_fs[i],false);
                check(alias_vs[i].idx==scene.vs.idx&&alias_fs[i].idx==scene.fs.idx
                    &&alias_program[i].idx==scene.program.idx,"native dedup fixture did not acquire shared refs");
                retirement(Command::Program,alias_program[i].idx);
                retirement(Command::Shader,alias_vs[i].idx); retirement(Command::Shader,alias_fs[i].idx);
            }
            Command finish; finish.kind=Command::CompleteFrame;
            retirements.push_back(finish);
            auto small=good; small.resources=5;
            reject(small,retirements.data(),retirements.size());
            small.resources=6;
            accept(retirements.data(),retirements.size(),small);
            accept(scene.commands.data(),scene.commands.size(),good);
            check(scene.pixels()==green_pixels,"shared retirement destroyed accepted native ownership");

            auto disposable=bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::RGBA8);
            std::array<Command,3> duplicate_retire{};
            duplicate_retire[0].kind=Command::Retire; duplicate_retire[0].resource=Command::Texture;
            duplicate_retire[0].resourceIndex=disposable.idx; duplicate_retire[1]=duplicate_retire[0];
            duplicate_retire[2]=finish;
            reject(good,duplicate_retire.data(),duplicate_retire.size());
            duplicate_retire[1]=finish;
            accept(duplicate_retire.data(),2,good);
            reject(good,duplicate_retire.data(),2);
            accept(scene.commands.data(),scene.commands.size(),good);
            check(scene.pixels()==green_pixels,"stale/double retirement changed surviving bytes");

            // Every view is admitted once; touch contributes no depth-control entry.
            std::vector<Command> all_views(257);
            for(unsigned i=0;i<256;++i) {
                all_views[i]=scene.commands[0]; all_views[i].view=std::uint16_t(i);
                all_views[i].touch=true; all_views[i].clearRgba=0x345678ff;
            }
            all_views.back()=finish;
            small=good; small.views=255; small.draws=0;
            reject(small,all_views.data(),all_views.size());
            small.views=256;
            const auto views=accept(all_views.data(),all_views.size(),small);
            check(views.views==256&&views.draws==0&&views.bindingsAfter==1,
                  "exact view/touch limit or identical-binding reuse differs");
            scene.pixels();

            // A frozen public draw manifest reaches the exact command limit.
            // One immutable slab permits every checked allocation failure to be
            // swept, including each render/bind/depth block and uniform growth.
            std::vector<Command> maximum(4096,scene.commands[1]);
            maximum.front()=scene.commands[0]; maximum.back()=finish;
            small=good; small.draws=4093;
            reject(small,maximum.data(),maximum.size());
            small=good;
            auto extra_commands=maximum; extra_commands.push_back(finish);
            reject(small,extra_commands.data(),extra_commands.size());
            maximum.back().kind=Command::Count;
            reject(small,maximum.data(),maximum.size()); maximum.back()=finish;
            std::uint64_t maximal_faults=0;
            bool maximal_accepted=false;
            for(std::uint64_t boundary=1;boundary<=512;++boundary) {
                allocator.fault(boundary);
                auto receipt=baseline; const auto resources=allocator.live.load();
                if(bgfx::submitBounded(good,maximum.data(),maximum.size(),receipt)) {
                    check(allocator.preparation_allocations<boundary&&receipt.sequence==++sequence
                        &&receipt.context==prior_context&&receipt.draws==4094&&receipt.views==1
                        &&receipt.bindingsAfter==2,"maximal admitted extent/receipt/cache differs");
                    maximal_faults=boundary-1; maximal_accepted=true; break;
                }
                check(same(receipt,baseline)&&allocator.live==resources
                    &&bgfx::boundedSubmissionPhase()==bgfx::BoundedSubmissionPhase::Idle,
                    "maximal fault published capacity or retained synchronization");
            }
            check(maximal_accepted&&maximal_faults>=32&&allocator.replay_allocations==0,
                  "maximal storage failure sweep incomplete");
            allocator.fault(0);
            check(scene.pixels()==green_pixels,"maximal replay changed immutable sampled bytes");

            // A source-suspended presentation advances once with no view/draw
            // or resize command; it does not synthesize a zero-size swap chain.
            const auto suspended=accept(&finish,1,good);
            check(suspended.draws==0&&suspended.views==0&&suspended.frameAfter==suspended.frameBefore+1,
                  "suspended native frame consumed draw state");

            check(bgfx::getCaps()->limits.maxTextureSamplers>=16,"required generated binding capacity unavailable");
            std::array<bgfx::UniformHandle,16> collision_samplers{};
            for(unsigned stage=0;stage<16;++stage) {
                collision_samplers[stage]=stage==8?scene.sampler:
                    bgfx::createUniform(("bounded_stage_"+std::to_string(stage)).c_str(),bgfx::UniformType::Sampler);
                check(bgfx::isValid(collision_samplers[stage]),"collision sampler provider unavailable");
            }
            const auto [first_pattern,second_pattern]=collision(scene.red.idx,scene.green.idx);
            std::array<std::array<bgfx::BoundedSubmissionTexture,16>,2> collision_bindings{};
            for(unsigned variant=0;variant<2;++variant) {
                const auto pattern=variant?second_pattern:first_pattern;
                for(unsigned stage=0;stage<16;++stage)
                    collision_bindings[variant][stage]={collision_samplers[stage],
                        (pattern&(1u<<(stage*2)))?scene.green:scene.red,
                        (pattern&(1u<<(stage*2+1)))?std::uint32_t(BGFX_SAMPLER_MIN_POINT|BGFX_SAMPLER_MAG_POINT|BGFX_SAMPLER_MIP_POINT):0u,
                        std::uint8_t(stage)};
            }
            std::array<Command,6> collision_commands{};
            collision_commands.front()=scene.commands[0]; collision_commands.back()=finish;
            for(unsigned i=1;i<=4;++i) {
                collision_commands[i]=scene.commands[1];
                collision_commands[i].textures=collision_bindings[(i-1)%2].data();
                collision_commands[i].textureCount=16;
            }
            const auto colliding=accept(collision_commands.data(),collision_commands.size(),good);
            check(colliding.draws==4&&colliding.bindingsAfter==3,
                "distinct colliding bindings aliased or repeated equal bindings duplicated");
            check(scene.pixels()==green_pixels,"colliding texture key changed physical sampling");
            for(unsigned stage=0;stage<16;++stage)
                if(stage!=8) bgfx::destroy(collision_samplers[stage]);
            bgfx::frame(); bgfx::frame();

            auto window_desc=window.descriptor(64,64);
            auto window_target=bgfx::createFrameBuffer(window_desc);
            check(bgfx::isValid(window_target),"physical window framebuffer rejected");
            bgfx::frame(); bgfx::frame();
            std::array<Command,4> window_commands{};
            window_commands[0].kind=Command::ResizeSwapChain;
            window_commands[0].framebuffer=window_target; window_commands[0].swapChain=window_desc;
            window_commands[1]=scene.commands[0]; window_commands[1].view=3;
            window_commands[1].framebuffer=window_target;
            window_commands[2]=nonindexed[1]; window_commands[2].view=3;
            window_commands[3]=finish;
            for(unsigned category=0;category<6;++category) {
                auto bad=window_commands;
                if(category==0) bad.back().kind=Command::Count;
                if(category==1) bad.front().swapChain.width=0;
                if(category==2) bad.front().swapChain.flags=UINT32_MAX;
                if(category==3) bad.front().swapChain.nwh=nullptr;
                if(category==4) bad.front().swapChain.formatColor=bgfx::TextureFormat::RGBA8;
                if(category==5) bad.front().framebuffer=scene.target;
                reject(good,bad.data(),bad.size());
            }
            small=good; small.resources=0;
            reject(small,window_commands.data(),window_commands.size());
            small.resources=1;
            const auto window_receipt=accept(window_commands.data(),window_commands.size(),small);
            check(window_receipt.draws==1&&window_receipt.views==1,"physical window frame omitted replay");
            accept(&finish,1,good); // Source suspension emits no swap-chain mutation.
            bgfx::frame(); bgfx::frame();
            check(SDL_SetWindowSize(window.window,32,32),"physical generated resize rejected");
            SDL_PumpEvents();
            window_commands[0].swapChain=window.descriptor(32,32);
            window_commands[1].width=window_commands[1].height=32;
            scene.viewport_bytes={32,32,0,0};
            accept(window_commands.data(),window_commands.size(),small);
            bgfx::frame(); bgfx::frame();
            std::array<Command,2> window_retirement{};
            window_retirement[0].kind=Command::Retire; window_retirement[0].resource=Command::FrameBuffer;
            window_retirement[0].resourceIndex=window_target.idx; window_retirement[1]=finish;
            accept(window_retirement.data(),window_retirement.size(),small);
            reject(good,window_retirement.data(),window_retirement.size());
            scene.viewport_bytes={64,64,0,0};
            accept(scene.commands.data(),scene.commands.size(),good);
            check(scene.pixels()==green_pixels,"window retry/retirement changed surviving target");
            scene.close();
        } catch(...) { bgfx::shutdown(); throw; }
        bgfx::shutdown();
        check(allocator.live==0&&allocator.replay_allocations==0,"native allocator residual or replay allocation");
    }
}
}
int main(int argc,char** argv)
{
    try {
        cpu();
        if(argc==2&&std::string(argv[1])=="--gpu") physical();
        std::cout<<"bounded native reservation controls passed\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"bounded native reservation failure: "<<error.what()<<'\n';
        return 1;
    }
}
