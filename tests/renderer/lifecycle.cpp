#include "support.h"
#include "workload.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

using namespace qualification;
namespace {
struct UploadLedger {
    std::atomic<uint64_t> acquired{0},released{0},bytes{0};
    bool settled() const { return acquired.load()==released.load(); }
    ~UploadLedger() {
        // Keep callback user data alive during exception unwinding as well.
        // Resource owners are declared after this ledger and retire first.
        for(int n=0;n<8 && !settled();++n)bgfx::frame();
        if(!settled()) {std::fputs("FAIL: outstanding upload holder at ledger teardown\n",stderr);std::abort();}
    }
};
struct Upload {
    UploadLedger& ledger;
    std::vector<uint8_t> bytes;
    Upload(UploadLedger& owner,const void* data,uint32_t size) : ledger(owner),bytes(size) {
        std::memcpy(bytes.data(),data,size);
    }
    static void release(void*,void* user) {
        std::unique_ptr<Upload> holder(static_cast<Upload*>(user));
        holder->ledger.released.fetch_add(1);
    }
};
const bgfx::Memory* reference(UploadLedger& ledger,const void* data,uint32_t size) {
    require(size>0,"empty upload");
    auto holder=std::make_unique<Upload>(ledger,data,size);
    const auto* memory=bgfx::makeRef(holder->bytes.data(),size,Upload::release,holder.get());
    ledger.acquired.fetch_add(1);
    ledger.bytes.fetch_add(size);
    holder.release();
    return memory;
}
struct Counts {
    uint16_t textures,vertices,indices,dynamicVertices,dynamicIndices,frames,programs,shaders,uniforms,layouts;
    bool operator==(const Counts&) const=default;
};
Counts counts() {
    const auto* s=bgfx::getStats();
    return {s->numTextures,s->numVertexBuffers,s->numIndexBuffers,s->numDynamicVertexBuffers,
        s->numDynamicIndexBuffers,s->numFrameBuffers,s->numPrograms,s->numShaders,s->numUniforms,s->numVertexLayouts};
}
void drain(UploadLedger& ledger) {
    for(int n=0;n<8;++n) bgfx::frame();
    require(ledger.settled(),"upload release callbacks did not settle");
}
void baseline(UploadLedger& ledger,const Counts& expected) {
    drain(ledger);
    if(!(counts()==expected)) {
        const auto got=counts();
        std::fprintf(stderr,"FAIL: resource residual tex=%u/%u vb=%u/%u ib=%u/%u fb=%u/%u layouts=%u/%u\n",
            got.textures,expected.textures,got.vertices,expected.vertices,got.indices,expected.indices,
            got.frames,expected.frames,got.layouts,expected.layouts);
        throw std::runtime_error("settled scene resource baseline");
    }
}
Pixel identity(uint32_t id,uint8_t family=0) {
    return {uint8_t(1+id%251),uint8_t(1+(id/251)%251),uint8_t(32+family),255};
}
std::array<float,4> normalized(Pixel p) { return {p.r/255.0f,p.g/255.0f,p.b/255.0f,p.a/255.0f}; }
struct GridVertex { float x,y,z; uint32_t color; float u,v,u2,v2; };
static_assert(sizeof(GridVertex)==32);
bgfx::VertexLayout gridLayout() {
    bgfx::VertexLayout layout;
    layout.begin().add(bgfx::Attrib::Position,3,bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,4,bgfx::AttribType::Uint8,true)
        .add(bgfx::Attrib::TexCoord0,2,bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord1,2,bgfx::AttribType::Float).end();
    return layout;
}
struct GridData {
    std::vector<GridVertex> vertices;
    std::vector<uint16_t> indices;
    GridData() {
        for(uint32_t y=0;y<=workload::cells;++y) for(uint32_t x=0;x<=workload::cells;++x) {
            const float u=float(x)/workload::cells,v=float(y)/workload::cells;
            vertices.push_back({2*u-1,2*v-1,.5f,0xffffffff,u,v,u,v});
        }
        for(uint32_t y=0;y<workload::cells;++y) for(uint32_t x=0;x<workload::cells;++x) {
            const uint16_t a=uint16_t(y*(workload::cells+1)+x),b=a+1,c=a+workload::cells+1,d=c+1;
            for(uint16_t value:{a,b,c,b,d,c}) indices.push_back(value);
        }
        require(vertices.size()==workload::gridVertices && indices.size()==workload::gridIndices,"grid census");
    }
};
struct Batch {
    Owned<bgfx::VertexBufferHandle> vertices;
    Owned<bgfx::IndexBufferHandle> indices;
    Owned<bgfx::TextureHandle> texture;
    Batch(UploadLedger& ledger,const GridData& grid,uint32_t first,uint32_t count) {
        require(count>0 && count<=workload::loadingBatch,"batch layer census");
        std::vector<GridVertex> data;std::vector<uint16_t> index;
        for(uint32_t layer=0;layer<count;++layer) {
            data.insert(data.end(),grid.vertices.begin(),grid.vertices.end());
            for(uint16_t value:grid.indices)index.push_back(uint16_t(value+layer*workload::gridVertices));
        }
        vertices.h=bgfx::createVertexBuffer(reference(ledger,data.data(),uint32_t(data.size()*sizeof(GridVertex))),gridLayout());
        require(bgfx::isValid(vertices.h),"batch vertex owner");
        indices.h=bgfx::createIndexBuffer(reference(ledger,index.data(),uint32_t(index.size()*sizeof(uint16_t))));
        require(bgfx::isValid(indices.h),"batch index owner");
        // Explicit mutable storage. Initial-memory texture creation is immutable.
        // A one-layer final batch still needs an array-compatible image view.
        const uint16_t layers=uint16_t(count<2?2:count);
        texture.h=bgfx::createTexture2D(workload::baseTexture,workload::baseTexture,false,layers,bgfx::TextureFormat::RGBA8);
        require(bgfx::isValid(texture.h),"terrain texture array");
        for(uint16_t layer=0;layer<layers;++layer) {
            const std::vector<Pixel> pixels(std::size_t(workload::baseTexture)*workload::baseTexture,
                layer<count?identity(first+layer):Pixel{0,0,0,0});
            bgfx::updateTexture2D(texture.h,layer,0,0,0,workload::baseTexture,workload::baseTexture,
                reference(ledger,pixels.data(),uint32_t(pixels.size()*sizeof(Pixel))));
        }
    }
};
struct Token { uint64_t generation; uint32_t tile; };
struct Scene {
    uint64_t generation;
    uint32_t tileCount=0;
    std::vector<std::unique_ptr<Batch>> batches;
    explicit Scene(uint64_t id) : generation(id) {}
    void prepare(UploadLedger& ledger,uint32_t count,uint32_t failAt=UINT32_MAX) {
        require(count>0 && count<=workload::tiles,"invalid generated scene count");
        require(batches.empty(),"scene already prepared");
        const auto* caps=bgfx::getCaps();
        const auto live=counts();
        const uint32_t batchCount=(count+workload::loadingBatch-1)/workload::loadingBatch;
        require(batchCount<=caps->limits.maxTextures-live.textures && batchCount<=caps->limits.maxVertexBuffers-live.vertices &&
            batchCount<=caps->limits.maxIndexBuffers-live.indices && caps->limits.maxTextureLayers>=workload::loadingBatch,"scene resource headroom");
        GridData grid;
        batches.reserve(batchCount);
        for(uint32_t n=0;n<count;n+=workload::loadingBatch) {
            if(failAt>=n && failAt<n+workload::loadingBatch) {
                // Force partial acquisition before rejecting this candidate;
                // the local construction owner never publishes its batch.
                if(failAt>n) {Batch pending(ledger,grid,n,failAt-n);}
                throw std::runtime_error("injected preparation failure");
            }
            batches.push_back(std::make_unique<Batch>(ledger,grid,n,std::min(workload::loadingBatch,count-n)));
            bgfx::frame();
        }
        tileCount=count;
    }
    struct Selection {const Batch& batch;uint16_t layer;};
    Selection resolve(Token token) const {
        require(token.generation==generation && token.tile<tileCount,"stale scene token");
        return {*batches[token.tile/workload::loadingBatch],uint16_t(token.tile%workload::loadingBatch)};
    }
};
void tileDraw(const Programs& p,Scene::Selection selected,uint16_t view,std::array<float,4> color={1,1,1,1}) {
    bgfx::setVertexBuffer(0,selected.batch.vertices.h);
    bgfx::setIndexBuffer(selected.batch.indices.h,selected.layer*workload::gridIndices,workload::gridIndices);
    const std::array<float,4> uv{1,1,0,0};
    bgfx::setUniform(p.color.h,color.data());
    bgfx::setUniform(p.uv.h,uv.data());
    const std::array<float,4> layer{float(selected.layer),0,0,0};bgfx::setUniform(p.layer.h,layer.data());
    bgfx::setTexture(0,p.sampler.h,selected.batch.texture.h,BGFX_SAMPLER_MIN_POINT|BGFX_SAMPLER_MAG_POINT);
    bgfx::setState(write);
    bgfx::submit(view,p.array.h);
}
void drawWitness(const Programs& p,const Quad& quad,uint32_t id,uint16_t view,Pixel color) {
    bgfx::setScissor(uint16_t(id%1024),uint16_t(id/1024),1,1);
    draw(p,quad,view,normalized(color));
}
void checkWitnesses(const Target& target,const std::vector<Pixel>& pixels,uint32_t first,uint32_t count,uint8_t family) {
    for(uint32_t n=0;n<count;++n) {
        const uint32_t id=first+n;
        pixel(pixels,target,int(id%1024),int(id/1024),identity(id,family),"complete draw identity coverage");
    }
}
struct FrameDemand {
    uint32_t draws,touches,blits,highestView;
    void validate() const {
        const auto& limits=bgfx::getCaps()->limits;
        require(uint64_t(draws)+touches<=limits.maxDrawCalls && blits<=limits.maxBlits &&
            highestView<limits.maxViews,"frame command/view capacity");
    }
};
void demandNegatives() {
    const auto& limits=bgfx::getCaps()->limits;
    const auto before=counts();
    for(const FrameDemand invalid:std::array{FrameDemand{limits.maxDrawCalls,1,0,0},
        FrameDemand{UINT32_MAX,UINT32_MAX,0,0},FrameDemand{0,0,limits.maxBlits+1,0},FrameDemand{0,0,0,limits.maxViews}}) {
        bool rejected=false;try {invalid.validate();}catch(const std::runtime_error&){rejected=true;}
        require(rejected && counts()==before,"invalid frame census must reject before effects");
    }
    FrameDemand{limits.maxDrawCalls-1,1,limits.maxBlits,limits.maxViews-1}.validate();
}
void capacity(const Programs& p) {
    UploadLedger ledger; // outlives every upload, including canceled owners
    drain(ledger);
    const auto before=counts();
    const auto start=std::chrono::steady_clock::now();
    for(uint64_t generation=1;generation<=3;++generation) {
    {
        Scene scene(generation);
        scene.prepare(ledger,workload::tiles);
        drain(ledger);
        constexpr uint32_t batches=workload::tiles/workload::loadingBatch;
        require(counts().textures==before.textures+batches &&
            counts().vertices==before.vertices+batches && counts().indices==before.indices+batches,"loaded scene resource census");
        Quad quad;
        Target target(1024,64);
        FrameDemand{workload::mixedDraws,0,2,250}.validate();
        uint32_t id=0; uint8_t family=0;
        for(const auto group:workload::groups) {
            target.view(family,BGFX_CLEAR_NONE);
            if(family==0) bgfx::setViewClear(0,BGFX_CLEAR_COLOR,0);
            for(uint32_t n=0;n<group.draws;++n,++id) {
                const auto color=identity(id,family);
                if(family==0) {
                    bgfx::setScissor(uint16_t(id%1024),uint16_t(id/1024),1,1);
                    const auto tile=scene.resolve({generation,n%workload::tiles});
                    // Unique texture is physically sampled; modulate to the
                    // per-draw identity while preserving its source owner.
                    const auto authored=identity(n%workload::tiles);
                    tileDraw(p,tile,family,{float(color.r)/authored.r,float(color.g)/authored.g,float(color.b)/authored.b,1});
                } else drawWitness(p,quad,id,family,color);
            }
            std::printf("PASS: mixed census family=%s draws=%u\n",group.name,group.draws);
            ++family;
        }
        bgfx::blit(100,bgfx::TextureRegion{.handle=target.readback.h},bgfx::TextureRegion{.handle=target.color.h});
        bgfx::frame();
        bgfx::frame(); // wait for the diagnostic frame's render-side submission
        require(bgfx::getStats()->numDrawCallsPeak==workload::mixedDraws,"mixed frame exact requested draw census");
        const auto pixels=target.pixels();
        id=0; family=0;
        for(const auto group:workload::groups) { checkWitnesses(target,pixels,id,group.draws,family++); id+=group.draws; }
        pixel(pixels,target,1023,63,{0,0,0,0},"untouched mixed-frame tail");
        std::printf("PASS: physical mixed frame draws=%u base_upload_bytes=%llu texture_estimate=%lld\n",
            id,static_cast<unsigned long long>(ledger.bytes.load()),static_cast<long long>(bgfx::getStats()->textureMemoryUsed));
    }
    baseline(ledger,before);
    std::printf("PASS: full terrain generation=%llu settled texture_estimate=%lld gpu_estimate=%lld\n",
        static_cast<unsigned long long>(generation),static_cast<long long>(bgfx::getStats()->textureMemoryUsed),
        static_cast<long long>(bgfx::getStats()->gpuMemoryUsed));
    }
    const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    std::printf("PASS: terrain load/render/teardown seconds=%.3f uploads=%llu released=%llu\n",elapsed,
        static_cast<unsigned long long>(ledger.acquired.load()),static_cast<unsigned long long>(ledger.released.load()));
}
void drawBoundary(const Programs& p) {
    Quad quad;
    Target target(1024,64);
    const uint32_t count=bgfx::getCaps()->limits.maxDrawCalls-1;
    require(count>workload::mixedDraws && count<=65535,"physical boundary witness size");
    demandNegatives();
    FrameDemand{count,1,1,250}.validate();
    target.view(0,BGFX_CLEAR_COLOR,0);
    // Three ordered consumers, including a scoped clear and a scene copy, are
    // active at the public draw boundary. No over-limit submit is ever issued.
    for(uint32_t n=0;n<count;++n) drawWitness(p,quad,n,0,identity(n,1));
    bgfx::blit(1,bgfx::TextureRegion{.handle=target.readback.h},bgfx::TextureRegion{.handle=target.color.h});
    target.view(2,BGFX_CLEAR_COLOR,0xabcdef01);
    bgfx::setViewRect(2,1023,63,1,1); bgfx::touch(2);
    bgfx::frame(); bgfx::frame();
    // Public touch is a dummy submit and consumes one render item even though
    // it produces no triangle. Account for this clear consumer in admission.
    require(bgfx::getStats()->numDrawCallsPeak==count+1,"boundary draws plus clear-touch census");
    // Read the captured pre-clear scene to prove every boundary draw and copy.
    std::vector<Pixel> pixels(std::size_t(target.w)*target.h);
    const auto ready=bgfx::read(bgfx::TextureRegion{.handle=target.readback.h},pixels.data());
    uint32_t frame=0; for(int n=0;n<32 && frame<ready;++n) frame=bgfx::frame();
    require(frame>=ready,"boundary read completion");
    checkWitnesses(target,pixels,0,count,1);
    require(pixels.back().a==0,"copy-before-scoped-clear ordering at capacity");
    std::printf("PASS: physical public draw boundary count=%u, scoped clear/copy/tail preserved\n",count);
}
struct PayloadGeometry {
    Owned<bgfx::VertexBufferHandle> vertices;
    Owned<bgfx::IndexBufferHandle> indices;
    PayloadGeometry(UploadLedger& ledger,uint32_t vertexCount,uint32_t indexCount) {
        require(vertexCount>=4 && vertexCount<=65535 && indexCount>=6 && indexCount%6==0,"generated geometry census");
        const std::array<GridVertex,4> quad={GridVertex{-1,-1,.5f,0xffffffff,0,0,0,0},GridVertex{1,-1,.5f,0xffffffff,1,0,1,0},
            GridVertex{-1,1,.5f,0xffffffff,0,1,0,1},GridVertex{1,1,.5f,0xffffffff,1,1,1,1}};
        std::vector<GridVertex> data(vertexCount);
        for(uint32_t n=0;n<vertexCount;++n)data[n]=quad[n%4];
        std::vector<uint16_t> index(indexCount);
        const uint16_t last=uint16_t((vertexCount/4-1)*4);
        const uint16_t face[]={last,uint16_t(last+1),uint16_t(last+2),uint16_t(last+1),uint16_t(last+3),uint16_t(last+2)};
        for(uint32_t n=0;n<indexCount;++n)index[n]=face[n%6];
        vertices.h=bgfx::createVertexBuffer(reference(ledger,data.data(),uint32_t(data.size()*sizeof(GridVertex))),gridLayout());
        require(bgfx::isValid(vertices.h),"source-sized vertex backing");
        indices.h=bgfx::createIndexBuffer(reference(ledger,index.data(),uint32_t(index.size()*sizeof(uint16_t))));
        require(bgfx::isValid(indices.h),"source-sized index backing");
    }
};
void uploadPressure(const Programs& p) {
    UploadLedger ledger; drain(ledger); const auto before=counts();
    {
        std::vector<Owned<bgfx::TextureHandle>> textures;
        const auto start=std::chrono::steady_clock::now();
        for(uint32_t n=0;n<64;++n) {
            const std::vector<Pixel> data(std::size_t(workload::highTexture)*workload::highTexture,identity(n,2));
            textures.emplace_back(bgfx::createTexture2D(workload::highTexture,workload::highTexture,false,1,bgfx::TextureFormat::RGBA8,0,
                reference(ledger,data.data(),uint32_t(data.size()*sizeof(Pixel)))));
            if((n+1)%8==0)bgfx::frame();
        }
        // Full-size buffers are initialized, uploaded and actually submitted.
        const std::array sizes={std::pair{30000u,60000u},std::pair{12000u,24000u},std::pair{16384u,32766u},std::pair{32768u,49152u}};
        std::vector<std::unique_ptr<PayloadGeometry>> geometries;
        for(const auto [v,i]:sizes)geometries.push_back(std::make_unique<PayloadGeometry>(ledger,v,i));
        Target target(128,64); Quad quad;
        target.view(0,BGFX_CLEAR_COLOR,0);
        for(uint32_t n=0;n<64;++n) {
            bgfx::setScissor(uint16_t(n),0,1,64);
            draw(p,quad,0,{1,1,1,1},write,BGFX_STENCIL_NONE,BGFX_STENCIL_NONE,textures[n].h);
        }
        for(uint32_t n=0;n<geometries.size();++n) {
            bgfx::setScissor(uint16_t(64+n),0,1,64);
            bgfx::setVertexBuffer(0,geometries[n]->vertices.h);
            bgfx::setIndexBuffer(geometries[n]->indices.h);
            const auto color=normalized(identity(n,3));const std::array<float,4> uv{1,1,0,0};
            bgfx::setUniform(p.color.h,color.data());bgfx::setUniform(p.uv.h,uv.data());
            bgfx::setState(write);bgfx::submit(0,p.solid.h);
        }
        const auto pixels=target.pixels();
        for(uint32_t n=0;n<64;++n)pixel(pixels,target,int(n),32,identity(n,2),"high-LOD authored upload");
        for(uint32_t n=0;n<geometries.size();++n)pixel(pixels,target,int(64+n),32,identity(n,3),"source-sized geometry tail selection");
        drain(ledger);
        std::printf("PASS: source-sized upload bytes=%llu holders=%llu elapsed=%.3f seconds\n",
            static_cast<unsigned long long>(ledger.bytes.load()),static_cast<unsigned long long>(ledger.acquired.load()),
            std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count());
    }
    baseline(ledger,before);
}
void changedBuffers(const Programs& p,UploadLedger& ledger,Target& target) {
    Owned<bgfx::DynamicVertexBufferHandle> vertices(bgfx::createDynamicVertexBuffer(4,gridLayout()));
    Owned<bgfx::DynamicIndexBufferHandle> indices(bgfx::createDynamicIndexBuffer(6));
    for(bool updated:{false,true}) {
        const uint32_t packed=updated?0xffff0000:0xff0000ff;
        const std::array data={GridVertex{-1,-1,.5f,packed,0,0,0,0},GridVertex{1,-1,.5f,packed,1,0,1,0},
            GridVertex{-1,1,.5f,packed,0,1,0,1},GridVertex{1,1,.5f,packed,1,1,1,1}};
        const std::array<uint16_t,6> first{0,1,2,1,3,2},second{2,1,0,2,3,1};
        const auto& index=updated?second:first;
        bgfx::update(vertices.h,0,reference(ledger,data.data(),sizeof(data)));
        bgfx::update(indices.h,0,reference(ledger,index.data(),sizeof(index)));
        // The index backing changes winding with culling disabled; both
        // attributes and selected buffer owners must follow the accepted update.
        bgfx::resetView(0);bgfx::setViewMode(0,bgfx::ViewMode::Sequential);
        bgfx::setViewFrameBuffer(0,target.framebuffer.h);bgfx::setViewRect(0,0,0,target.w,target.h);
        bgfx::setViewClear(0,BGFX_CLEAR_COLOR,0);
        bgfx::setVertexBuffer(0,vertices.h);bgfx::setIndexBuffer(indices.h);
        const std::array<float,4> white{1,1,1,1},uv{1,1,0,0};
        bgfx::setUniform(p.color.h,white.data());bgfx::setUniform(p.uv.h,uv.data());
        bgfx::setState(write);bgfx::submit(0,p.solid.h);
        // Readback is a GPU completion event; an upload-release count alone
        // could not prove the updated packed attributes and indices were used.
        pixel(target.pixels(),target,32,32,updated?Pixel{0,0,255,255}:Pixel{255,0,0,255},"dynamic vertex/index update physical payload");
    }
}
void lifecycle(const Programs& p) {
    UploadLedger ledger;
    // Initialize the stock public dynamic-buffer pools before measuring scene
    // lifetimes; they may retain a reusable backing after their first use.
    {Target warm;changedBuffers(p,ledger,warm);}
    drain(ledger); const auto before=counts();
    for(uint64_t generation=1;generation<=6;++generation) {
        {
            Scene accepted(generation); accepted.prepare(ledger,32);
            drain(ledger);
            const auto acceptedCounts=counts();
            // Candidate failures happen after real acquisitions while the
            // accepted scene remains usable. Construction owners roll back.
            for(uint32_t failAt:{0u,1u,31u}) {
                bool failed=false;
                try { Scene candidate(generation+100);candidate.prepare(ledger,32,failAt); }
                catch(const std::runtime_error& e) { failed=std::string(e.what())=="injected preparation failure"; }
                require(failed,"candidate fault injection");
                baseline(ledger,acceptedCounts);
            }
            for(uint32_t invalid:{0u,workload::tiles+1,UINT32_MAX}) {
                bool rejected=false;try { Scene candidate(900);candidate.prepare(ledger,invalid); }
                catch(const std::runtime_error&) {rejected=true;}
                require(rejected,"invalid scene census rejection");baseline(ledger,acceptedCounts);
            }
            { Scene canceled(generation+200);canceled.prepare(ledger,48); } // cancel before publication
            baseline(ledger,acceptedCounts);
            bool stale=false;try { (void)accepted.resolve({generation+1,0}); } catch(const std::runtime_error&) {stale=true;}
            require(stale,"stale generation admitted");
            for(uint32_t index:{32u,UINT32_MAX}) {
                bool rejected=false;try {(void)accepted.resolve({generation,index});}catch(const std::runtime_error&){rejected=true;}
                require(rejected,"out-of-range scene token admitted");
            }
            Target target; target.view(0,BGFX_CLEAR_COLOR);
            tileDraw(p,accepted.resolve({generation,31}),0);
            pixel(target.pixels(),target,32,32,identity(31),"accepted scene after candidate failures");
            const auto changed=identity(uint32_t(generation),4);
            std::vector<Pixel> pixels(std::size_t(workload::baseTexture)*workload::baseTexture,changed);
            const auto selected=accepted.resolve({generation,31});
            bgfx::updateTexture2D(selected.batch.texture.h,selected.layer,0,0,0,workload::baseTexture,workload::baseTexture,
                reference(ledger,pixels.data(),uint32_t(pixels.size()*sizeof(Pixel))));
            target.view(0,BGFX_CLEAR_COLOR);tileDraw(p,accepted.resolve({generation,31}),0);
            pixel(target.pixels(),target,32,32,changed,"changed texture generation");
            changedBuffers(p,ledger,target);
            // Retry previously failed preparation on the same process/context.
            Scene replacement(generation+100);replacement.prepare(ledger,32);
            target.view(0,BGFX_CLEAR_COLOR);tileDraw(p,replacement.resolve({generation+100,31}),0);
            pixel(target.pixels(),target,32,32,identity(31),"corrected replacement retry");
        }
        baseline(ledger,before);
    }
    std::printf("PASS: six load/failed-candidate/cancel/update/replace/teardown lifetimes uploads=%llu releases=%llu\n",
        static_cast<unsigned long long>(ledger.acquired.load()),static_cast<unsigned long long>(ledger.released.load()));
}
struct Video {
    Video() {
        require(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        try {require(SDL_Vulkan_LoadLibrary(nullptr),SDL_GetError());}
        catch(...) {SDL_Quit();throw;}
    }
    ~Video() {SDL_Vulkan_UnloadLibrary();SDL_Quit();}
};
struct Window {
    SDL_Window* window=nullptr;
    Window() {
        window=SDL_CreateWindow("Zero Hour stock renderer qualification",160,120,SDL_WINDOW_RESIZABLE|SDL_WINDOW_VULKAN);
        if(!window) throw std::runtime_error(SDL_GetError());
    }
    ~Window() {SDL_DestroyWindow(window);}
    bgfx::SwapChain description() const {
        bgfx::SwapChain swap;
        const auto props=SDL_GetWindowProperties(window);
        const std::string driver=SDL_GetCurrentVideoDriver();
        if(driver=="x11") {
            swap.ndt=SDL_GetPointerProperty(props,SDL_PROP_WINDOW_X11_DISPLAY_POINTER,nullptr);
            swap.nwh=reinterpret_cast<void*>(uintptr_t(SDL_GetNumberProperty(props,SDL_PROP_WINDOW_X11_WINDOW_NUMBER,0)));
        } else if(driver=="wayland") {
            swap.ndt=SDL_GetPointerProperty(props,SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER,nullptr);
            swap.nwh=SDL_GetPointerProperty(props,SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER,nullptr);
        } else throw std::runtime_error("qualification needs native X11 or Wayland window");
        int w=0,h=0;require(SDL_GetWindowSizeInPixels(window,&w,&h),SDL_GetError());
        require(swap.nwh && swap.ndt && w>0 && h>0,"native SDL3 surface");
        swap.width=uint32_t(w);swap.height=uint32_t(h);
        swap.formatColor=bgfx::TextureFormat::BGRA8;
        swap.formatDepthStencil=bgfx::TextureFormat::D24S8;
        swap.numBackBuffers=3;
        swap.maxFrameLatency=2;
        return swap;
    }
};
void presentation(const std::string& shaderDir,bool initOnly=false) {
    Video video; // process service outlives all window/device generations
    for(int repeat=0;repeat<3;++repeat) {
        Window window;auto swap=window.description();
        const auto type=std::string(SDL_GetCurrentVideoDriver())=="wayland"?bgfx::NativeWindowHandleType::Wayland:bgfx::NativeWindowHandleType::Default;
        Context context(&swap,type);
        if(initOnly)continue;
        Programs p(shaderDir);Quad quad;
        for(const auto [width,height]:std::array{std::pair{160,120},std::pair{321,241},std::pair{192,128},std::pair{160,120}}) {
            require(SDL_SetWindowSize(window.window,width,height),SDL_GetError());
            require(SDL_SyncWindow(window.window),SDL_GetError());
            SDL_PumpEvents();swap=window.description();bgfx::reset(BGFX_RESET_NONE,&swap);
            bgfx::resetView(0);bgfx::setViewFrameBuffer(0,BGFX_INVALID_HANDLE);
            bgfx::setViewRect(0,0,0,uint16_t(swap.width),uint16_t(swap.height));
            bgfx::setViewClear(0,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH,0x102030ff);draw(p,quad,0,{0,1,0,1});
            bgfx::frame();bgfx::frame();
            require(bgfx::getStats()->width==swap.width && bgfx::getStats()->height==swap.height,"physical swap-chain resize dimensions");
            Target offscreen(uint16_t(swap.width),uint16_t(swap.height));offscreen.view(1,BGFX_CLEAR_COLOR);
            draw(p,quad,1,{0,0,1,1});pixel(offscreen.pixels(),offscreen,width/2,height/2,{0,0,255,255},"offscreen target after native resize");
        }
    }
    std::puts(initOnly?"PASS: three native SDL3/Vulkan initialization-only contexts":"PASS: three native SDL3/Vulkan contexts with four window/swap-chain/target sizes each");
}
} // namespace

int main(int argc,char** argv) {
    if(argc!=3) {std::fprintf(stderr,"usage: renderer_lifecycle SHADERS FAMILY\n");return 2;}
    try {
        const std::string family=argv[2];
        if(family=="presentation" || family=="presentation-init")presentation(argv[1],family=="presentation-init");
        else {
            require(family=="capacity"||family=="draw-boundary"||family=="uploads"||family=="lifetimes","unknown capacity/lifecycle family");
            Context context;Programs p(argv[1]);
            if(family=="capacity")capacity(p);
            if(family=="draw-boundary")drawBoundary(p);
            if(family=="uploads")uploadPressure(p);
            if(family=="lifetimes")lifecycle(p);
        }
        std::puts("PASS: qualification complete");return 0;
    } catch(const std::exception& error) {std::fprintf(stderr,"FAIL: %s\n",error.what());return 1;}
}
