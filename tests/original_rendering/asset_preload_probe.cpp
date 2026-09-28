#include "PreRTS.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/RandomValue.h"
#include "GameClient/ClientRandomValue.h"
#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "asset_import.h"
#include "assetmgr.h"
#include "proto.h"
#include "texture.h"
#include "ww3d.h"
#include "dx8wrapper.h"
#include "mesh.h"
#include "meshmdl.h"
#include "vertmaterial.h"
#include "camera.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "zh/platform/bgfx_device.h"
#include <SDL3/SDL.h>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <cstdlib>
#include <memory>

extern unsigned _MinTextureFilters[8][TextureFilterClass::FILTER_TYPE_COUNT];
extern unsigned _MagTextureFilters[8][TextureFilterClass::FILTER_TYPE_COUNT];
extern unsigned _MipMapFilters[8][TextureFilterClass::FILTER_TYPE_COUNT];

struct W3DAssetImportProbeAccess {
    static void fault(int ordinal) { ww3d_import::Attempt::fault_ordinal=ordinal;ww3d_import::Attempt::fault_count=0; }
    static unsigned faults() { return ww3d_import::Attempt::fault_count; }
    static std::vector<std::string> prototypes(WW3DAssetManager &assets) {
        std::vector<std::string> result;
        for (int i=0;i<assets.Prototypes.Count();++i) result.emplace_back(assets.Prototypes[i]->Get_Name());
        return result;
    }
};
struct W3DAssetPreloadProbeAccess {
    static std::size_t slots(GameClient &client) { return client.m_drawableVector.size(); }
    static std::size_t capacity(GameClient &client) { return client.m_drawableVector.capacity(); }
};
namespace {
void require(bool value,const char *category) { if (!value) throw std::runtime_error(category); }
template<class F> bool rejected(F action) { try { action(); } catch (...) { return true; } return false; }
struct TraceDisplay final : W3DDisplay {
    std::vector<std::string> requests;
    bool throwModel=false;
    TraceDisplay() { requests.reserve(1024); }
    void preloadModelAssets(AsciiString model) override {
        requests.emplace_back(std::string("model:")+model.str());
        if (throwModel) { throwModel=false;throw std::runtime_error("generated preload callback fault"); }
        W3DDisplay::preloadModelAssets(model);
    }
    void preloadTextureAssets(AsciiString texture) override {
        requests.emplace_back(std::string("texture:")+texture.str());
        W3DDisplay::preloadTextureAssets(texture);
    }
};
void throwingAdmission(void*) { throw std::runtime_error("generated preload admission fault"); }
template<class T> struct BorrowedReplacement {
    T *&slot;T *saved;
    BorrowedReplacement(T *&target,T *replacement):slot(target),saved(target) { slot=replacement; }
    ~BorrowedReplacement() { slot=saved; }
};
struct Live {
    std::vector<Drawable*> draws;
    std::vector<DrawModule*> modules;
    explicit Live(GameClient &client) {
        for (auto *draw=client.firstDrawable();draw;draw=draw->getNextDrawable()) {
            draws.push_back(draw);
            for (auto **module=draw->getDrawModulesNonDirty();*module;++module) modules.push_back(*module);
            require(client.findDrawableByID(draw->getID())==draw,"preload live lookup identity differs");
        }
    }
    bool same(const Live &other) const { return draws==other.draws && modules==other.modules; }
};
struct RandomImage {
    UnsignedInt client[6]{},logic[6]{},seed=0;
    RandomImage() { CopyGameClientRandomState(client);CopyGameLogicRandomState(&seed,logic); }
    bool same(const RandomImage &other) const { return seed==other.seed &&
        !std::memcmp(client,other.client,sizeof(client)) && !std::memcmp(logic,other.logic,sizeof(logic)); }
};
struct FilterImage {
    int profile=WW3D::Get_Texture_Filter();
    std::array<unsigned,3*8*TextureFilterClass::FILTER_TYPE_COUNT> tables{};
    FilterImage() {
        unsigned *next=tables.data();
        for (auto table:{_MinTextureFilters,_MagTextureFilters,_MipMapFilters}) {
            std::memcpy(next,table,sizeof(_MinTextureFilters));
            next+=8*TextureFilterClass::FILTER_TYPE_COUNT;
        }
    }
    bool same(const FilterImage &other) const { return profile==other.profile && tables==other.tables; }
};
std::vector<unsigned char> bytes(const TextureClass &texture) {
    const auto *first=reinterpret_cast<const unsigned char*>(&texture);
    return {first,first+sizeof(texture)};
}
void lazy(WW3DAssetManager &assets) {
    // Free hash slots retain unspecified old Value storage; only the native
    // bucket iterator represents live cache ownership after a reset/purge.
    HashTemplateIterator<StringClass,TextureClass*> entries(assets.Texture_Hash());
    for (entries.First();!entries.Is_Done();entries.Next()) {
        auto *texture=entries.Peek_Value();
        require(!texture->Is_Initialized() && !texture->Is_Procedural() &&
            texture->Get_Width()==0 && texture->Get_Height()==0 && texture->Num_Refs()==1,
            "preload descriptor performed eager work or retained caller ref");
    }
}
void importFaults(TraceDisplay &display,WW3DAssetManager &assets)
{
    // Every ordinal starts from an exact accepted manager. Earlier successful
    // requests remain published; only this independent request is withdrawn.
    unsigned boundaries=0;
    for (unsigned ordinal=0;ordinal<256;++ordinal) {
        const auto names=W3DAssetImportProbeAccess::prototypes(assets);
        std::array<unsigned char,sizeof(W3DAssetManager)> image{};
        std::memcpy(image.data(),&assets,image.size());
        const std::string file="PreloadFault"+std::to_string(ordinal);
        const std::string prototype="PRELOAD.FAULT"+std::to_string(ordinal);
        const RandomImage random;
        W3DAssetImportProbeAccess::fault(static_cast<int>(ordinal));
        const bool failed=rejected([&]{display.preloadModelAssets(AsciiString(file.c_str()));});
        const unsigned visited=W3DAssetImportProbeAccess::faults();
        W3DAssetImportProbeAccess::fault(-1);
        if (!failed) {
            require(assets.Find_Prototype(prototype.c_str()) && ordinal==visited,
                "preload import sweep did not reach its exact success boundary");
            boundaries=ordinal;break;
        }
        require(visited==ordinal+1 && !assets.Find_Prototype(prototype.c_str()) &&
            W3DAssetImportProbeAccess::prototypes(assets)==names &&
            std::memcmp(image.data(),&assets,image.size())==0 && random.same(RandomImage()),
            "preload import fault changed accepted cache/prototype identity");
        display.preloadModelAssets(AsciiString(file.c_str()));
        auto *accepted=assets.Find_Prototype(prototype.c_str());
        require(accepted,"preload import fault retry did not publish source");
        // Retire only the accepted test request so later fault ordinals retain
        // equivalent graph size instead of growing their own fault boundary.
        assets.Remove_Prototype(accepted);accepted->DeleteSelf();
        require(W3DAssetImportProbeAccess::prototypes(assets)==names,
            "preload request retirement changed earlier accepted siblings");
    }
    require(boundaries>0 && boundaries<256,"preload import sweep exhausted declared bound");
}
void temporaryImportFaults(Display *savedDisplay)
{
    // A fresh equivalent display generation per ordinal keeps the native
    // per-request graph/workload invariant. Successful earlier requests may
    // remain accepted within one GameClient preload; it is not a batch import.
    unsigned boundaries=0;
    for (unsigned ordinal=0;ordinal<4096;++ordinal) {
        zh::renderer::RecordingGpuDevice device;
        bool failed=false;unsigned visited=0;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            TraceDisplay display;display.init();TheDisplay=&display;
            const Live live(*TheGameClient);const auto id=TheGameClient->getDrawableIDCounter();
            const auto frame=device.snapshot();const auto native=device.resource_counts();
            const RandomImage random;
            W3DAssetImportProbeAccess::fault(static_cast<int>(ordinal));
            failed=rejected([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
            visited=W3DAssetImportProbeAccess::faults();W3DAssetImportProbeAccess::fault(-1);
            require(live.same(Live(*TheGameClient)) &&
                TheGameClient->getDrawableIDCounter()==DrawableID(unsigned(id)+1) &&
                !TheGameClient->findDrawableByID(id) && frame==device.snapshot() &&
                native==device.resource_counts() && random.same(RandomImage()),
                "temporary import ordinal changed live scene/registry/ref/RNG/native ownership");
            lazy(*W3DDisplay::m_assetManager);
            if (failed) {
                require(visited==ordinal+1,"temporary import fault ordinal/count differed");
                TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);
                require(live.same(Live(*TheGameClient)) &&
                    TheGameClient->getDrawableIDCounter()==DrawableID(unsigned(id)+2) &&
                    !TheGameClient->findDrawableByID(DrawableID(unsigned(id)+1)) &&
                    frame==device.snapshot() && native==device.resource_counts(),
                    "temporary import ordinal retry changed ID/sibling/native identity");
                lazy(*W3DDisplay::m_assetManager);
            }
            TheDisplay=savedDisplay;
        }
        require(!device.resource_counts().total(),"temporary import ordinal generation retained resources");
        if (!failed) { require(visited==ordinal,"temporary import success boundary count differed");boundaries=ordinal;break; }
    }
    require(boundaries>0 && boundaries<4096,"temporary import sweep exhausted bounded workload");
    std::printf("original asset preload boundaries: rejected=%u retry=1\n",boundaries);
}
void physicalPreload(Display *savedDisplay)
{
    // The established full-probe process owns shipping allocator/filesystem
    // services before bgfx starts, and retains them until its worker stops.
    require(SDL_Init(SDL_INIT_VIDEO),"preload physical video service rejected");
    auto *window=SDL_CreateWindow("generated native preload mesh",32,24,SDL_WINDOW_HIDDEN);
    require(window,"preload physical window provider rejected");
    const int savedFilter=WW3D::Get_Texture_Filter();
    try {
        for (unsigned generation=0;generation<2;++generation) {
            zh::renderer::BgfxOptions options;options.shader_root=ZH_BGFX_SHADER_DIR;
            zh::renderer::BgfxGpuDevice device(options);
            require(device.claim_window(window),"preload physical device/window rejected");
            {
                zh::original_runtime::OriginalGpuEdge edge(device);
                W3DDisplay display;display.init();TheDisplay=&display;
                // Shipping device setup initializes these public profile tables;
                // CPU Init alone deliberately does not. Preload must not do it.
                WW3D::Set_Texture_Filter(TextureFilterClass::TEXTURE_FILTER_BILINEAR);
                const FilterImage filter;
                auto &assets=*W3DDisplay::m_assetManager;
                const auto native=device.live_resource_count(),frames=device.native_frame_advance_count();
                display.preloadTextureAssets(AsciiString("MYTEX.TGA"));
                auto *descriptor=assets.Texture_Hash().Get("mytex.tga");
                require(descriptor && !descriptor->Is_Initialized() && !edge.resident_texture(descriptor),
                    "physical regular preload did eager texture work");
                W3DAssetImportProbeAccess::fault(0);
                const bool failed=rejected([&]{display.preloadModelAssets(AsciiString("PreloadPhysical"));});
                W3DAssetImportProbeAccess::fault(-1);
                require(failed && !assets.Find_Prototype("TEST.LITONE01") &&
                    assets.Texture_Hash().Get("mytex.tga")==descriptor && !descriptor->Is_Initialized() &&
                    device.live_resource_count()==native && device.native_frame_advance_count()==frames &&
                    filter.same(FilterImage()),
                    "physical preload rejection touched accepted native identity/frame");
                display.preloadModelAssets(AsciiString("PreloadPhysical"));
                auto *prototype=assets.Find_Prototype("TEST.LITONE01");
                require(prototype && assets.Texture_Hash().Get("mytex.tga")==descriptor &&
                    !descriptor->Is_Initialized() && descriptor->Get_Mip_Level_Count()==MIP_LEVELS_ALL &&
                    descriptor->Get_Filter().Get_Mip_Mapping()!=TextureFilterClass::FILTER_TYPE_NONE &&
                    device.live_resource_count()==native &&
                    device.native_frame_advance_count()==frames && filter.same(FilterImage()),
                    "physical model preload published GPU work or replaced lazy descriptor");
                auto *mesh=static_cast<MeshClass*>(assets.Create_Render_Obj("TEST.LITONE01"));
                const auto retireMesh=[](MeshClass *value) {
                    if (value) { if (value->Peek_Scene()) value->Remove();value->Release_Ref(); }
                };
                std::unique_ptr<MeshClass,decltype(retireMesh)> meshOwner(mesh,retireMesh);
                require(mesh && mesh->Class_ID()==RenderObjClass::CLASSID_MESH,
                    "preloaded physical source mesh consumer rejected");
                mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
                zh::renderer::TextureDesc target;target.width=32;target.height=24;target.render_target=true;
                target.format=zh::renderer::TextureFormat::rgba8;const auto color=device.create_texture(target,"preload mesh color");
                target.format=zh::renderer::TextureFormat::depth24_stencil8;const auto depth=device.create_texture(target,"preload mesh depth");
                require(color && depth,"preload physical frame targets rejected");
                edge.bind_frame_targets(color,depth,32,24);
                CameraClass camera;camera.Set_Position(Vector3(0,0,1));
                camera.Set_View_Plane(Vector2(-1,-0.75f),Vector2(1,0.75f));camera.Set_Clip_Planes(0.995f,2);
                const auto frame=[&] {
                    try {
                    require(WW3D::Begin_Render(true,true,Vector3(0.2f,0.4f,0.6f),1)==WW3D_ERROR_OK,
                        "preload physical source frame begin rejected");
                    DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix3D(true));
                    W3DDisplay::m_3DScene->doRender(&camera);
                    require(WW3D::End_Render(false)==WW3D_ERROR_OK && device.present(color),
                        "preload physical source frame completion rejected");
                    return device.readback_rgba(color);
                    } catch (...) { edge.abort_source_frame();throw; }
                };
                const auto empty=frame();W3DDisplay::m_3DScene->Add_Render_Object(mesh);
                const auto pixels=frame();
                require(pixels.size()==32*24*4 && pixels!=empty && assets.Find_Prototype("TEST.LITONE01")==prototype &&
                    assets.Texture_Hash().Get("mytex.tga")==descriptor && descriptor->Is_Initialized() &&
                    edge.resident_texture(descriptor) && !edge.is_missing_texture(descriptor),
                    "preloaded exact source identities did not produce physical texture/mesh pixels");
                require(frame()==pixels,"preloaded physical equivalent source retry changed pixels");
                meshOwner.reset();
                edge.release_source_buffers();device.destroy(depth);device.destroy(color);
                TheDisplay=savedDisplay;
            }
            device.release_window();
            require(device.wait_idle() && device.live_resource_count()==0,"preload physical generation retained resources");
        }
    } catch (...) {
        W3DAssetImportProbeAccess::fault(-1);TheDisplay=savedDisplay;
        WW3D::Set_Texture_Filter(savedFilter);
        SDL_DestroyWindow(window);SDL_Quit();throw;
    }
    WW3D::Set_Texture_Filter(savedFilter);
    require(WW3D::Get_Texture_Filter()==savedFilter,"physical fixture did not restore declared filter profile");
    SDL_DestroyWindow(window);SDL_Quit();
    std::puts("original asset preload physical: lazy=1 identity=1 pixels=1 retry=1 generations=2 resources=0");
}
}

extern "C" void zh_probe_asset_preload()
{
    const auto *temporary=TheThingFactory->findTemplate(AsciiString("PreloadTemporaryFixture"),FALSE);
    require(temporary,"preload generated temporary template missing");
    const Bool oldEverything=TheWritableGlobalData->m_preloadEverything;
    TheWritableGlobalData->m_preloadEverything=FALSE;
    auto *savedDisplay=TheDisplay;
    DX8Wrapper::Reset_Source_State();
    // Minimal/headless absence is a genuine no-op. A foreign callback cannot
    // replace an installed owner and its throw runs before any temporary ID.
    const Live absentBaseline(*TheGameClient);const auto absentNext=TheGameClient->getDrawableIDCounter();
    preflightClientPreloadAdmission();
    int admissionOwner=0,foreign=0;
    const auto admission=installClientPreloadAdmission(&admissionOwner,throwingAdmission);
    require(admission && !installClientPreloadAdmission(&foreign,throwingAdmission) &&
        !removeClientPreloadAdmission(&foreign,admission) && !removeClientPreloadAdmission(&admissionOwner,admission+1),
        "preload optional admission owner/token validation failed");
    require(rejected([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);}) &&
        absentBaseline.same(Live(*TheGameClient)) && TheGameClient->getDrawableIDCounter()==absentNext,
        "preload callback rejection created partial temporary state");
    {
        zh::renderer::RecordingGpuDevice device;
        zh::original_runtime::OriginalGpuEdge edge(device);
        auto *factory=_TheFileFactory;
        W3DDisplay display;
        require(rejected([&]{display.init();}) && !W3DDisplay::m_assetManager &&
            !W3DDisplay::m_3DScene && !TheW3DFileSystem && _TheFileFactory==factory &&
            !device.resource_counts().total() && rejected([&]{preflightClientPreloadAdmission();}) &&
            !removeClientPreloadAdmission(&foreign,admission),
            "display bootstrap conflict changed the original preload admission/provider owner");
    }
    require(removeClientPreloadAdmission(&admissionOwner,admission) &&
        !removeClientPreloadAdmission(&admissionOwner,admission),"preload callback retired more than once");
    try {
        for (unsigned generation=0;generation<2;++generation) {
            zh::renderer::RecordingGpuDevice device;
            {
                auto edge=std::make_unique<zh::original_runtime::OriginalGpuEdge>(device);
                TraceDisplay display;display.init();TheDisplay=&display;
                auto &assets=*W3DDisplay::m_assetManager;
                const auto frame=device.snapshot();const auto resources=device.resource_counts();
                display.preloadModelAssets(AsciiString("PreloadMissing"));
                require(!assets.Find_Prototype("PRELOAD.MISSING") &&
                    W3DAssetImportProbeAccess::prototypes(assets).empty(),"optional missing preload published state");
                display.preloadModelAssets(AsciiString("PreloadDirect"));
                require(assets.Find_Prototype("PRELOAD.DIRECT"),"native model preload did not import generated source");
                display.preloadModelAssets(AsciiString("PreloadQualified.w3d"));
                require(assets.Find_Prototype("PRELOAD.QUALIFIED"),"literal qualified suffix preload changed source naming");
                display.preloadTextureAssets(AsciiString("PreloadLazy.tga"));
                auto *descriptor=assets.Texture_Hash().Get("preloadlazy.tga");
                require(descriptor,"regular lazy preload descriptor absent");
                const auto image=bytes(*descriptor);
                display.preloadTextureAssets(AsciiString("PreloadLazy.tga"));
                require(assets.Texture_Hash().Get("preloadlazy.tga")==descriptor && bytes(*descriptor)==image,
                    "preload cache hit changed descriptor identity/default/access/ref bytes");
                auto *factory=_TheFileFactory;_TheFileFactory=nullptr;
                const bool removed=rejected([&]{display.preloadModelAssets(AsciiString("PreloadDirect"));});
                _TheFileFactory=factory;
                require(removed,"preload missing file provider was admitted");
                auto *filesystem=TheFileSystem;TheFileSystem=nullptr;
                const bool missingFs=rejected([&]{display.preloadTextureAssets(AsciiString("PreloadLazy.tga"));});
                TheFileSystem=filesystem;
                require(missingFs && bytes(*descriptor)==image,"preload removed VFS changed descriptor");
                require(rejected([&]{display.preloadModelAssets(AsciiString("PreloadUnsupported"));}) ||
                    !assets.Find_Prototype("PRELOAD.UNSUPPORTED"),"unsupported preload published prototype");
                importFaults(display,assets);
                const Live baseline(*TheGameClient);
                const auto next=TheGameClient->getDrawableIDCounter();
                const auto slots=W3DAssetPreloadProbeAccess::slots(*TheGameClient);
                const auto capacity=W3DAssetPreloadProbeAccess::capacity(*TheGameClient);
                const auto noCreation=[&](auto action) {
                    const auto id=TheGameClient->getDrawableIDCounter();
                    const Live live(*TheGameClient);
                    const auto trace=device.snapshot();const auto native=device.resource_counts();
                    const RandomImage random;
                    require(rejected(action) && TheGameClient->getDrawableIDCounter()==id &&
                        live.same(Live(*TheGameClient)) && W3DAssetPreloadProbeAccess::slots(*TheGameClient)==slots &&
                        W3DAssetPreloadProbeAccess::capacity(*TheGameClient)==capacity &&
                        device.snapshot()==trace && device.resource_counts()==native && random.same(RandomImage()),
                        "preload admission rejection changed temporary/counter/native baseline");
                };
                { BorrowedReplacement<Display> removed(TheDisplay,nullptr);
                  noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);}); }
                { BorrowedReplacement<FileFactoryClass> foreign(_TheFileFactory,reinterpret_cast<FileFactoryClass*>(1));
                  noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);}); }
                { BorrowedReplacement<W3DAssetManager> foreign(W3DDisplay::m_assetManager,reinterpret_cast<W3DAssetManager*>(1));
                  noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);}); }
                for (const char *stage:{"drawable-registry","draw-modules","client-modules",
                        "object-created-before:0","object-created-after:0","drawable-resolution"}) {
                    setenv("ZH_M22_CONSTRUCTION_FAIL_STAGE",stage,1);
                    const bool failed=rejected([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
                    unsetenv("ZH_M22_CONSTRUCTION_FAIL_STAGE");
                    require(failed && baseline.same(Live(*TheGameClient)) &&
                        TheGameClient->getDrawableIDCounter()==next && !TheGameClient->findDrawableByID(next),
                        "temporary constructor callback fault changed live owner/ID baseline");
                }
                W3DAssetImportProbeAccess::fault(0);
                const bool failed=rejected([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
                W3DAssetImportProbeAccess::fault(-1);
                require(failed && baseline.same(Live(*TheGameClient)) &&
                    TheGameClient->getDrawableIDCounter()==DrawableID(unsigned(next)+1) &&
                    !TheGameClient->findDrawableByID(next) &&
                    W3DAssetPreloadProbeAccess::slots(*TheGameClient)==slots &&
                    W3DAssetPreloadProbeAccess::capacity(*TheGameClient)==capacity,
                    "temporary preload fault did not restore exact live owner with one consumed ID");
                TheGameClient->preloadAssets(TIME_OF_DAY_NIGHT);
                require(baseline.same(Live(*TheGameClient)) &&
                    TheGameClient->getDrawableIDCounter()==DrawableID(unsigned(next)+2) &&
                    !TheGameClient->findDrawableByID(DrawableID(unsigned(next)+1)),
                    "temporary preload clean retry changed siblings or next ID");
                require(assets.Find_Prototype("PRELOAD.TEMPORARY"),"temporary module preload skipped native model request");
                const auto callbackNext=TheGameClient->getDrawableIDCounter();
                display.throwModel=true;
                bool exactFailure=false;
                try { TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON); }
                catch (const std::runtime_error &failure) { exactFailure=std::strcmp(failure.what(),"generated preload callback fault")==0; }
                require(exactFailure && baseline.same(Live(*TheGameClient)) &&
                    TheGameClient->getDrawableIDCounter()==DrawableID(unsigned(callbackNext)+1) &&
                    !TheGameClient->findDrawableByID(callbackNext),"temporary cleanup masked callback failure or leaked ownership");
                // Record actual public GameClient request order with a live
                // sibling, the selected temporary, debris, particle, defaults.
                const auto *loadedTemplate=TheThingFactory->findTemplate(AsciiString("PreloadLoadedFixture"),FALSE);
                require(loadedTemplate,"preload generated loaded-state template missing");
                auto *loaded=TheThingFactory->newDrawable(loadedTemplate);
                const Live withLoaded(*TheGameClient);
                extern std::vector<AsciiString> debrisModelNamesGlobalHack;
                const auto debris=debrisModelNamesGlobalHack;
                debrisModelNamesGlobalHack.push_back(AsciiString("PreloadDebris"));
                display.requests.clear();
                TheGameClient->preloadAssets(TIME_OF_DAY_MORNING);
                require(withLoaded.same(Live(*TheGameClient)) && debrisModelNamesGlobalHack.empty(),
                    "native preload traversal changed sibling or debris completion");
                const auto &trace=display.requests;
                require(trace.size()==42 && trace[0]=="model:preloadloaded" && trace[1]=="model:preloadtemporary" &&
                    trace[2]=="model:PreloadDebris" && trace[3]=="texture:PreloadParticle.tga" &&
                    trace[4]=="texture:ptspruce01.tga" && trace.back()=="texture:pmcrates.tga",
                    "native loaded/temporary/debris/particle/default source request order changed");
                const auto successful=trace;
                display.requests.clear();TheGameClient->preloadAssets(TIME_OF_DAY_NIGHT);
                auto retry=successful;retry.erase(retry.begin()+2);
                require(display.requests==retry,"time-selected native preload retry changed condition/default order");
                debrisModelNamesGlobalHack=debris;
                TheGameClient->destroyDrawable(loaded);
                require(baseline.same(Live(*TheGameClient)),"loaded-state preload sibling teardown retained owner");
                unsigned allTemplates=0;
                for (auto *entry=TheThingFactory->firstTemplate();entry;entry=entry->friend_getNextTemplate()) ++allTemplates;
                const auto allNext=TheGameClient->getDrawableIDCounter();
                TheWritableGlobalData->m_preloadEverything=TRUE;
                TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);
                TheWritableGlobalData->m_preloadEverything=FALSE;
                require(allTemplates>1 && baseline.same(Live(*TheGameClient)) &&
                    TheGameClient->getDrawableIDCounter()==DrawableID(unsigned(allNext)+allTemplates),
                    "preloadEverything changed native selection/live cleanup or ID multiplicity");
                display.requests.clear();display.reset();
                require(baseline.same(Live(*TheGameClient)),"display reset changed native preload live siblings");
                lazy(assets);
                require(device.snapshot()==frame && device.resource_counts()==resources,
                    "descriptor preload emitted GPU/stage/frame work");
                zh::renderer::TextureDesc target;target.width=4;target.height=4;target.render_target=true;
                target.format=zh::renderer::TextureFormat::rgba8;const auto color=device.create_texture(target,"preload phase color");
                target.format=zh::renderer::TextureFormat::depth24_stencil8;const auto depth=device.create_texture(target,"preload phase depth");
                require(color && depth,"preload phase target fixture rejected");edge->bind_frame_targets(color,depth,4,4);
                require(edge->begin_tree_source_frame(),"preload pending frame fixture rejected");
                noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
                require(edge->abort_tree_source_frame(),"preload pending frame abort failed");
                edge->begin_source_frame(true,true,0,0,0,1);
                noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
                edge->end_source_frame(false);device.destroy(depth);device.destroy(color);
                edge.reset();
                noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
                edge=std::make_unique<zh::original_runtime::OriginalGpuEdge>(device);
                noCreation([&]{TheGameClient->preloadAssets(TIME_OF_DAY_AFTERNOON);});
                TheDisplay=savedDisplay;
            }
            require(!device.resource_counts().total(),"preload generated generation retained native resources");
        }
    } catch (...) {
        W3DAssetImportProbeAccess::fault(-1);
        unsetenv("ZH_M22_CONSTRUCTION_FAIL_STAGE");
        TheDisplay=savedDisplay;TheWritableGlobalData->m_preloadEverything=oldEverything;
        throw;
    }
    temporaryImportFaults(savedDisplay);
    TheDisplay=savedDisplay;TheWritableGlobalData->m_preloadEverything=oldEverything;
    if (std::getenv("ZH_M22_ASSET_PRELOAD_PHYSICAL")) physicalPreload(savedDisplay);
    std::puts("original asset preload: source=1 lazy=1 temporary=1 retry=1 generations=2 resources=0");
}
