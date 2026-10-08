#include "support.h"
using namespace qualification;
namespace {

struct SampleView {
    bgfx::TextureHandle texture;
    uint32_t available, first, count;
    std::array<float,4> uv{1,1,0,0};
    void validate() const {
        require(bgfx::isValid(texture) && available>0 && available<=UINT8_MAX &&
            count>0 && first<available && count<=available-first, "invalid sampled mip range");
        for(float value:uv) require(std::isfinite(value), "non-finite sampled coordinates");
    }
};

void admission(const Programs& p,const Quad& q) {
    const Pixel white{255,255,255,255};
    Owned<bgfx::TextureHandle> texture(bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::RGBA8,0,bgfx::copy(&white,sizeof(white))));
    const SampleView valid{texture.h,1,0,1};
    valid.validate();
    // No invalid case may reach an upstream API. Validate against a real live
    // candidate, then render it after every rejection in this same process.
    auto reject=[](const SampleView& view) {
        bool rejected=false;
        try { view.validate(); } catch(const std::runtime_error&) { rejected=true; }
        require(rejected,"malformed sample was accepted");
    };
    reject({texture.h,1,0,0});
    reject({texture.h,1,1,1});
    reject({texture.h,1,0,2});
    reject({texture.h,1,UINT32_MAX,2});
    reject({texture.h,UINT32_MAX,0,1});
    reject({BGFX_INVALID_HANDLE,1,0,1});
    reject({texture.h,1,0,1,{1,1,std::numeric_limits<float>::infinity(),0}});
    reject({texture.h,1,0,1,{1,1,0,std::numeric_limits<float>::quiet_NaN()}});
    SampleView{texture.h,UINT8_MAX,UINT8_MAX-1,1}.validate(); // arithmetic boundary, no submission
    Target target;
    target.view(0,BGFX_CLEAR_COLOR);
    valid.validate();
    bgfx::setVertexBuffer(0,q.vertices.h);
    bgfx::setIndexBuffer(q.indices.h);
    const std::array<float,4> color{0,1,0,1};
    bgfx::setUniform(p.color.h,color.data());
    bgfx::setUniform(p.uv.h,valid.uv.data());
    bgfx::setTexture(0,p.sampler.h,valid.texture,0,1,uint8_t(valid.first),uint8_t(valid.count));
    bgfx::setState(write);
    bgfx::submit(0,p.texture.h);
    const auto data=target.pixels();
    require(data[32*target.w+32].g==255 && data[32*target.w+32].r==0,"valid candidate after rejected input");
    std::puts("PASS: descriptor rejection and same-candidate rendering retry");
}


void viewport_and_depth(const Programs& p,const Quad& q) {
    Target t;
    for(bool stencil:{false,true}) {
        t.view(0,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL,0xff0000ff,.25f,7);
        bgfx::touch(0);
        t.view(1,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL,0x0000ffff,1,0);
        bgfx::setViewRect(1,16,16,32,32);
        bgfx::touch(1);
        t.view(2);
        draw(p,q,2,{0,1,0,1},write|(stencil?BGFX_STATE_DEPTH_TEST_ALWAYS:BGFX_STATE_DEPTH_TEST_LESS),
            stencil?(BGFX_STENCIL_TEST_EQUAL|BGFX_STENCIL_FUNC_REF(0)|BGFX_STENCIL_FUNC_RMASK(255)|keep):BGFX_STENCIL_NONE);
        const auto data=t.pixels();
        pixel(data,t,0,0,{255,0,0,255},"outside clear");
        pixel(data,t,15,16,{255,0,0,255},"clear boundary outside");
        pixel(data,t,16,16,{0,255,0,255},"clear boundary inside");
        pixel(data,t,47,47,{0,255,0,255},"clear far boundary inside");
        pixel(data,t,48,47,{255,0,0,255},"clear far boundary outside");
    }
    std::puts("PASS: viewport color/depth/stencil preservation");
}

void selective_stencil(const Programs& p,const Quad& q) {
    Target t;
    // Upstream PR #3821 introduced the write mask in the second public stencil
    // argument's RMASK field. No private headers/helpers or vendor changes.
    for(uint8_t mask:{uint8_t(0x80),uint8_t(0xff),uint8_t(0x00)}) {
        t.view(0,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH|BGFX_CLEAR_STENCIL,0xff0000ff,1,0x18);
        const uint32_t replace=BGFX_STENCIL_TEST_ALWAYS|BGFX_STENCIL_FUNC_REF(0x80)|BGFX_STENCIL_FUNC_RMASK(255)|
            BGFX_STENCIL_OP_FAIL_S_KEEP|BGFX_STENCIL_OP_FAIL_Z_KEEP|BGFX_STENCIL_OP_PASS_Z_REPLACE;
        draw(p,q,0,{1,1,1,1},0,replace,BGFX_STENCIL_FUNC_RMASK(mask));
        t.view(1);
        const auto expected=uint8_t((0x18 & ~mask)|(0x80 & mask));
        const uint32_t increment=BGFX_STENCIL_TEST_ALWAYS|BGFX_STENCIL_FUNC_RMASK(255)|
            BGFX_STENCIL_OP_FAIL_S_KEEP|BGFX_STENCIL_OP_FAIL_Z_KEEP|BGFX_STENCIL_OP_PASS_Z_INCR;
        draw(p,q,1,{1,1,1,1},0,increment,BGFX_STENCIL_FUNC_RMASK(7));
        t.view(2);
        draw(p,q,2,{0,1,0,1},write,BGFX_STENCIL_TEST_EQUAL|BGFX_STENCIL_FUNC_REF(expected+1)|BGFX_STENCIL_FUNC_RMASK(255)|keep);
        const auto data=t.pixels();
        pixel(data,t,32,32,{0,255,0,255},"selective stencil bits");
    }
    std::puts("PASS: selective stencil write masks preserve player bits with shadow counts");
}

void mips(const Programs& p,const Quad& q) {
    // All levels have authored distinct colors; range/filter tests must not
    // succeed by reading an uninitialized allocation tail.
    std::vector<Pixel> authored;
    const std::array<Pixel,7> colors={Pixel{255,0,0,255},Pixel{0,255,0,255},Pixel{0,0,255,255},Pixel{255,255,0,255},Pixel{255,0,255,255},Pixel{0,255,255,255},Pixel{255,255,255,255}};
    for(int level=0;level<7;++level) authored.insert(authored.end(),std::size_t(64>>level)*(64>>level),colors[level]);
    Owned<bgfx::TextureHandle> texture(bgfx::createTexture2D(64,64,true,1,bgfx::TextureFormat::RGBA8,0,
        bgfx::copy(authored.data(),static_cast<uint32_t>(authored.size()*sizeof(Pixel)))));
    Target t;
    for(uint8_t level=0;level<7;++level) {
        t.view(0,BGFX_CLEAR_COLOR);
        draw(p,q,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,texture.h,level,1,
            BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_MIP_POINT,{1024,1024,0,0});
        pixel(t.pixels(),t,32,32,colors[level],"declared mip view");
    }
    t.view(0,BGFX_CLEAR_COLOR);
    draw(p,q,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,texture.h,0,3,
        BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_MIP_POINT,{1024,1024,0,0});
    pixel(t.pixels(),t,32,32,colors[2],"partial mip range");
    t.view(0,BGFX_CLEAR_COLOR);
    draw(p,q,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,texture.h,0,7,
        BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_MIP_POINT,{1024,1024,0,0},1);
    pixel(t.pixels(),t,32,32,colors[0],"public quarter-mip LOD clamp");
    std::puts("PASS: authored mip views, partial chains, base-level LOD clamp");
}

void spatial_filters(const Programs& p,const Quad& q) {
    std::vector<Pixel> authored;
    for(int y=0;y<64;++y) for(int x=0;x<64;++x)
        authored.push_back(x%2 ? Pixel{255,255,255,255}:Pixel{0,0,0,255});
    for(int level=1;level<7;++level)
        authored.insert(authored.end(),std::size_t(64>>level)*(64>>level),Pixel{255,0,0,255});
    Owned<bgfx::TextureHandle> texture(bgfx::createTexture2D(64,64,true,1,bgfx::TextureFormat::RGBA8,0,
        bgfx::copy(authored.data(),static_cast<uint32_t>(authored.size()*sizeof(Pixel)))));
    Target t;
    constexpr uint32_t common=BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_MIP_POINT;
    for(bool minify:{false,true}) for(bool minPoint:{false,true}) {
        const uint32_t flags=common|(minPoint?BGFX_SAMPLER_MIN_POINT:BGFX_SAMPLER_MAG_POINT);
        const float scale=minify?2.0f:.25f;
        const float offset=minify?0.0f:(1.0f/64.0f-.5f/64.0f*.25f);
        t.view(0,BGFX_CLEAR_COLOR);
        draw(p,q,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,texture.h,0,7,flags,{scale,scale,offset,0},1);
        const uint8_t expected=(minify?minPoint:!minPoint)?255:128;
        pixel(t.pixels(),t,0,32,{expected,expected,expected,255},"independent min/mag base sampling");
    }
    t.view(0,BGFX_CLEAR_COLOR);
    draw(p,q,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,texture.h,0,7,common,{2,2,0,0});
    pixel(t.pixels(),t,0,32,{255,0,0,255},"unclamped mip selection control");
    std::puts("PASS: independent min/mag spatial filtering and unclamped control");
}

void ordering_and_attributes(const Programs& p) {
    Quad q;
    Target source, result;
    Owned<bgfx::TextureHandle> copied(bgfx::createTexture2D(64,64,false,1,bgfx::TextureFormat::RGBA8,BGFX_TEXTURE_BLIT_DST));
    source.view(0,BGFX_CLEAR_COLOR);
    draw(p,q,0,{1,0,0,1});
    bgfx::setScissor(32,0,32,64);
    draw(p,q,0,{0,0,1,1});
    bgfx::blit(1,bgfx::TextureRegion{.handle=copied.h},bgfx::TextureRegion{.handle=source.color.h});
    source.view(2);
    draw(p,q,2,{0,1,0,1});
    result.view(3,BGFX_CLEAR_COLOR);
    draw(p,q,3,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,copied.h,0,1,
        BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP,{1,1,.5f,0});
    const auto shifted=result.pixels();
    pixel(shifted,result,16,32,{0,0,255,255},"distorted prior-scene sample");
    pixel(source.pixels(),source,16,32,{0,255,0,255},"later scene overwrite control");
    // Normalized Uint8 ABGR transport and sequential alpha layering.
    Quad packed(.5f,0xff00ff00);
    result.view(0,BGFX_CLEAR_COLOR,0x0000ffff);
    draw(p,packed,0,{1,1,1,1});
    draw(p,q,0,{1,0,0,.5f},write|BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,BGFX_STATE_BLEND_INV_SRC_ALPHA));
    pixel(result.pixels(),result,32,32,{128,127,0,191},"packed color and ordered alpha");
    std::puts("PASS: draw-copy-draw ordering, packed color, sequential transparency");
}

void original_materials(const Programs& p,const Quad& q) {
    const Pixel first{255,128,64,255}, second{64,128,255,255};
    Owned<bgfx::TextureHandle> t0(bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::RGBA8,0,bgfx::copy(&first,sizeof(first))));
    Owned<bgfx::TextureHandle> t1(bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::RGBA8,0,bgfx::copy(&second,sizeof(second))));
    Target target;
    for(bool flat:{false,true}) {
        target.view(0,BGFX_CLEAR_COLOR);
        bgfx::setTexture(1,p.detail.h,t1.h);
        draw(p,q,0,{.5f,1,.5f,.25f},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,t0.h,0,1,0,{1,1,0,0},UINT8_MAX,flat?p.flat.h:p.terrain.h);
        pixel(target.pixels(),target,32,32,flat?Pixel{32,64,32,64}:Pixel{104,128,56,64},"original terrain material equation");
    }
    std::puts("PASS: source terrain and flat-terrain shader equations");
}

void projected_shadows(const Programs& p,const Quad& q) {
    Target shadow,receiver;
    // Model MatrixMapperClass STQ transport, not the old D3D matrix ABI.
    // Source Init_Multiplicative uses ZERO/SRC_COLOR and no depth writes.
    for(bool perspective:{false,true}) {
        shadow.view(0,BGFX_CLEAR_COLOR,0xffffffff);
        bgfx::setScissor(40,1,23,62);
        draw(p,q,0,{.25f,.25f,.25f,1});
        receiver.view(1,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH,0xcc9966ff,.5f);
        const float matrix[]={.5f,0,perspective?.5f:0,0, 0,.5f,0,0, 0,0,0,0, .5f,.5f,1,1};
        bgfx::setUniform(p.projection.h,matrix);
        bgfx::setVertexBuffer(0,q.vertices.h);
        bgfx::setIndexBuffer(q.indices.h);
        bgfx::setTexture(0,p.sampler.h,shadow.color.h,BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|BGFX_SAMPLER_MIN_POINT|BGFX_SAMPLER_MAG_POINT);
        bgfx::setState(write|BGFX_STATE_DEPTH_TEST_LEQUAL|BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ZERO,BGFX_STATE_BLEND_SRC_COLOR));
        bgfx::submit(1,p.project.h);
        const auto data=receiver.pixels();
        pixel(data,receiver,8,32,{204,153,102,255},"outside projected shadow");
        pixel(data,receiver,48,32,perspective?Pixel{204,153,102,255}:Pixel{51,38,26,255},"projected Q division control");
        pixel(data,receiver,60,32,{51,38,26,255},"projected shadow interior");
    }
    std::puts("PASS: render-to-texture projected shadows, varying Q and multiplicative blending");
}

void source_formats(const Programs& p,const Quad& q) {
    // Synthetic BC1 block with all indices selecting opaque RGB565 red.
    const uint8_t bc1[]={0x00,0xf8,0x00,0x00,0,0,0,0};
    const uint8_t bgra[]={0x40,0x80,0xff,0xff};
    require(bgfx::isTextureValid(0,false,1,bgfx::TextureFormat::BC1,0),"BC1 format unsupported");
    require(bgfx::isTextureValid(0,false,1,bgfx::TextureFormat::BGRA8,0),"BGRA8 format unsupported");
    Owned<bgfx::TextureHandle> compressed(bgfx::createTexture2D(4,4,false,1,bgfx::TextureFormat::BC1,0,bgfx::copy(bc1,sizeof(bc1))));
    Owned<bgfx::TextureHandle> packed(bgfx::createTexture2D(1,1,false,1,bgfx::TextureFormat::BGRA8,0,bgfx::copy(bgra,sizeof(bgra))));
    Target target;
    for(bool block:{false,true}) {
        target.view(0,BGFX_CLEAR_COLOR);
        draw(p,q,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,block?compressed.h:packed.h);
        pixel(target.pixels(),target,32,32,block?Pixel{255,0,0,255}:Pixel{255,128,64,255},"source texture transport");
    }
    std::puts("PASS: generated BC1 and BGRA8 source texture transport");
}
}

int main(int argc,char** argv) {
    if(argc!=3) { std::fprintf(stderr,"usage: renderer_qualification SHADER_DIRECTORY FAMILY\n"); return 2; }
    try {
        const std::string family=argv[2];
        const std::array names={"init","admission","clears","stencil","mips","filters","ordering","materials","projection","formats","all"};
        bool known=false;
        for(const auto* name:names) known|=family==name;
        require(known,"unknown qualification family");
        Context context;
        if(family=="init") {
            std::puts("PASS: initialization-only control");
            std::puts("PASS: qualification complete");
            return 0;
        }
        Programs programs(argv[1]);
        Quad quad;
        const auto selected=[&](const char* name){return family=="all" || family==name;};
        if(selected("admission")) admission(programs,quad);
        if(selected("clears")) viewport_and_depth(programs,quad);
        if(selected("stencil")) selective_stencil(programs,quad);
        if(selected("mips")) mips(programs,quad);
        if(selected("filters")) spatial_filters(programs,quad);
        if(selected("ordering")) ordering_and_attributes(programs);
        if(selected("materials")) original_materials(programs,quad);
        if(selected("projection")) projected_shadows(programs,quad);
        if(selected("formats")) source_formats(programs,quad);
        std::puts("PASS: qualification complete");
        return 0;
    } catch(const std::exception& e) {
        std::fprintf(stderr,"FAIL: %s\n",e.what());
        return 1;
    }
}
