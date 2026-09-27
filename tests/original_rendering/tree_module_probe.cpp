#include "PreRTS.h"
#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/SubsystemInterface.h"
#include "Common/MapReaderWriterInfo.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "WWLib/RAMFILE.H"
#include "assetmgr.h"
#include "camera.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include <array>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

void zh_m22_tree_factory_layout(std::size_t *);
void zh_m22_tree_constructor_layout(std::size_t *);
extern "C" void zh_probe_tree_module_physical(const char *,const char *);

struct W3DTreeModuleGeneratedProbeAccess {
    static std::vector<unsigned char> image(const Thing &owner)
    {
        std::vector<unsigned char> result;
        const auto bytes=[&](const auto &value) {
            const auto *begin=reinterpret_cast<const unsigned char *>(&value);
            result.insert(result.end(),begin,begin+sizeof(value));
        };
        bytes(owner.m_transform);bytes(owner.m_cachedPos);bytes(owner.m_cachedAngle);
        bytes(owner.m_cachedDirVector);bytes(owner.m_cachedAltitudeAboveTerrain);
        bytes(owner.m_cachedAltitudeAboveTerrainOrWater);bytes(owner.m_cacheFlags);
        return result;
    }
    static void caches(Thing &owner)
    {
        owner.m_cachedDirVector={1.25f,-2.5f,3.75f};
        owner.m_cachedAltitudeAboveTerrain=7.25f;
        owner.m_cachedAltitudeAboveTerrainOrWater=8.5f;
        owner.m_cacheFlags=7;
    }
    static std::array<std::size_t,8> layout()
    {
        return {sizeof(W3DTreeDraw),alignof(W3DTreeDraw),offsetof(W3DTreeDraw,m_treeAdded),
            offsetof(W3DTreeDraw,m_treeOwner),offsetof(W3DTreeDraw,m_treeID),
            offsetof(W3DTreeDraw,m_treeEpoch),offsetof(W3DTreeDraw,m_treeDevice),
            offsetof(W3DTreeDraw,m_treeDeviceGeneration)};
    }
    static void pending(GameLogic &owner,Object *object) { owner.m_objectsToDestroy.push_back(object); }
    static void unpending(GameLogic &owner,Object *object) { owner.m_objectsToDestroy.remove(object); }
    static void process(GameLogic &owner) { owner.processDestroyList(); }
    static void destroyAll(GameLogic &owner) { owner.destroyAllObjectsImmediate(); }
};

extern "C" void zh_probe_tree_module_destroy(Object *object)
{
    TheGameLogic->destroyObject(object);
    W3DTreeModuleGeneratedProbeAccess::process(*TheGameLogic);
}

namespace {
void require(bool value,const char *category)
{ if (!value) throw std::runtime_error(category); }
template<class F> bool rejected(F action)
{ try { action(); } catch (...) { return true; } return false; }
class CallbackThing final:public Thing {
public:
    explicit CallbackThing(const ThingTemplate *source):Thing(source) {}
    ~CallbackThing() override=default;
    bool reject=false;unsigned calls=0;
protected:
    MemoryPool *getObjectMemoryPool() override { return nullptr; }
    void reactToTransformChange(const Matrix3D *,const Coord3D *,Real) override
    { ++calls;if (reject) throw ERROR_INVALID_D3D; }
};
template<class F> void transforms(Thing &owner,const F &rejector)
{
    W3DTreeModuleGeneratedProbeAccess::caches(owner);
    const auto before=W3DTreeModuleGeneratedProbeAccess::image(owner);
    const Coord3D next{15,20,4};Matrix3D matrix(1);matrix.Set_Translation(Vector3(16,22,6));
    for (unsigned ordinal=0;ordinal<4;++ordinal) {
        require(rejected([&] {
            rejector();
            switch (ordinal) {
            case 0:owner.setPositionZ(5);break;
            case 1:owner.setPosition(&next);break;
            case 2:owner.setOrientation(0.75f);break;
            default:owner.setTransformMatrix(&matrix);break;
            }
        }),"tree module transform rejection missing");
        require(W3DTreeModuleGeneratedProbeAccess::image(owner)==before,
            "tree module transform/cache rollback differs");
    }
}
}

extern "C" void zh_probe_tree_module()
{
    const auto layout=W3DTreeModuleGeneratedProbeAccess::layout();
    std::array<std::size_t,8> factory{},constructor{};
    zh_m22_tree_factory_layout(factory.data());zh_m22_tree_constructor_layout(constructor.data());
    require(layout==factory && layout==constructor,"tree module factory/constructor layout mismatch");
    const auto *control=TheThingFactory->findTemplate(AsciiString("ModuleControlFixture"),FALSE);
    require(control,"tree module generated control template missing");
    const auto *treeTemplate=TheThingFactory->findTemplate(AsciiString("ModuleTreeFixture"),FALSE);
    require(treeTemplate,"tree module generated tree template missing");
    CallbackThing callback(control);const Coord3D initial{12,18,2};callback.setPosition(&initial);
    callback.reject=true;transforms(callback,[]{});
    callback.reject=false;const auto calls=callback.calls;callback.setPositionZ(3);
    require(callback.calls==calls+1 && callback.getPosition()->z==3,
        "tree module unrelated successful callback changed");
    const auto oldTrees=TheWritableGlobalData->m_useTrees;
    const auto oldPartition=TheWritableGlobalData->m_partitionCellSize;
    TheWritableGlobalData->m_useTrees=TRUE;TheWritableGlobalData->m_partitionCellSize=MAP_XY_FACTOR;
    const char *path=std::getenv("ZH_M22_TREE_PREPARATION_MAP");
    const char *asset=std::getenv("ZH_M22_TREE_PREPARATION_ASSET");
    require(path && asset,"tree module generated map/model missing");
    for (unsigned generation=0;generation<2;++generation) {
        zh::renderer::RecordingGpuDevice device;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            auto *savedDisplay=TheDisplay;auto *savedVisual=TheTerrainVisual;auto *savedView=TheTacticalView;
            W3DDisplay display;display.init();TheDisplay=&display;
            W3DTerrainVisual visual;TheTerrainVisual=&visual;
            try {
                display.setWidth(32);display.setHeight(24);visual.init();
                require(visual.load(AsciiString(path)),"tree module generated map admission rejected");
                std::ifstream input(asset,std::ios::binary);
                std::vector<char> bytes(std::istreambuf_iterator<char>{input},std::istreambuf_iterator<char>{});
                RAMFileClass packet(bytes.data(),int(bytes.size()));
                require(!bytes.empty() && static_cast<WW3DAssetManager *>(W3DDisplay::m_assetManager)->Load_3D_Assets(packet),
                    "tree module generated model admission rejected");
                auto *view=new W3DView;TheTacticalView=view;view->init();display.attachView(view);
                view->setWidth(32);view->setHeight(24);view->setDefaultView(0,0,1);
                auto *camera=view->get3DCamera();camera->Set_Clip_Planes(0.1f,1000);
                camera->Set_Position(Vector3(12,18,30));
                zh::renderer::TextureDesc desc;desc.width=32;desc.height=24;desc.render_target=true;
                desc.sampled=false;desc.format=zh::renderer::TextureFormat::rgba8;
                const auto color=device.create_texture(desc,"tree module color");
                desc.format=zh::renderer::TextureFormat::depth24_stencil8;
                const auto depth=device.create_texture(desc,"tree module depth");
                require(color && depth,"tree module targets missing");edge.bind_frame_targets(color,depth,32,24);
                auto *terrain=TheTerrainRenderObject;terrain->getShroud()->render(camera);
                Object *seed=TheGameLogic->getFirstObject();require(seed,"tree module generated team missing");
                auto *constructorDraw=TheThingFactory->newDrawable(control);
                const auto *typedData=treeTemplate->getDrawModuleInfo().getNthData(0);
                const auto *wrongData=control->getDrawModuleInfo().getNthData(0);
                const auto constructorResources=device.resource_counts();
                require(rejected([&]{newInstance(W3DTreeDraw)(constructorDraw,nullptr);})
                    && rejected([&]{newInstance(W3DTreeDraw)(constructorDraw,wrongData);}),
                    "tree module null/wrong data constructor admitted");
                auto *savedTerrain=TheTerrainRenderObject;TheTerrainRenderObject=nullptr;
                const bool absentOwner=rejected([&]{newInstance(W3DTreeDraw)(constructorDraw,typedData);});
                TheTerrainRenderObject=savedTerrain;
                require(absentOwner && device.resource_counts()==constructorResources && !terrain->treeInstanceCount(),
                    "tree module missing-owner constructor changed publication/resources");
                TheGameClient->destroyDrawable(constructorDraw);
                const char *constructionFaults[]={"object-id","team","behavior-modules","behavior-resolution",
                    "radar","logic","object","create","partition","drawable-registry","draw-modules",
                    "client-modules","drawable-resolution","drawable","binding","init"};
                for (const char *fault:constructionFaults) {
                    auto *objectHead=TheGameLogic->getFirstObject();auto *drawHead=TheGameClient->firstDrawable();
                    const auto baselineResources=device.resource_counts();
                    UnsignedInt before[6]{},after[6]{};CopyGameClientRandomState(before);
                    setenv("ZH_M22_CONSTRUCTION_FAIL_STAGE",fault,1);
                    const bool failed=rejected([&]{TheThingFactory->newObject(treeTemplate,seed->getTeam());});
                    unsetenv("ZH_M22_CONSTRUCTION_FAIL_STAGE");CopyGameClientRandomState(after);
                    require(failed && TheGameLogic->getFirstObject()==objectHead
                        && TheGameClient->firstDrawable()==drawHead && !terrain->treeInstanceCount()
                        && !terrain->treeTypeCount() && device.resource_counts()==baselineResources
                        && !std::memcmp(before,after,sizeof(before)),
                        "tree module constructor unwind changed immediate baseline");
                    auto *retry=TheThingFactory->newObject(treeTemplate,seed->getTeam());
                    require(retry && dynamic_cast<W3DTreeDraw *>(retry->getDrawable()->getDrawModules()[0]),
                        "tree module constructor retry did not use physical factory");
                    TheGameLogic->destroyObject(retry);W3DTreeModuleGeneratedProbeAccess::process(*TheGameLogic);
                    require(TheGameLogic->getFirstObject()==objectHead && TheGameClient->firstDrawable()==drawHead
                        && !terrain->treeInstanceCount() && device.resource_counts()==baselineResources,
                        "tree module constructor retry retained ownership");
                }
                const std::pair<const char *,const char *> admissionFaults[]={
                    {"ZH_M22_TREE_REGISTRY_FAIL_AT","type"},{"ZH_M22_TREE_REGISTRY_FAIL_AT","instance"},
                    {"ZH_M22_TREE_MODEL_FAIL_AT","root"},{"ZH_M22_TREE_ATLAS_FAIL_AT","read"},
                    {"ZH_M22_TREE_ATLAS_FAIL_AT","tile"},{"ZH_M22_TREE_ATLAS_FAIL_AT","pack"},
                    {"ZH_M22_TREE_ATLAS_FAIL_AT","pixels"},{"ZH_M22_TREE_RESOURCE_FAIL_AT","geometry"},
                    {"ZH_M22_TREE_RESOURCE_FAIL_AT","vertex"},{"ZH_M22_TREE_RESOURCE_FAIL_AT","index"},
                    {"ZH_M22_TREE_RESOURCE_FAIL_AT","texture-create"},{"ZH_M22_TREE_RESOURCE_FAIL_AT","texture-upload"},
                    {"ZH_M22_TREE_RESOURCE_FAIL_AT","texture-publish"},{"ZH_M22_TREE_RESOURCE_FAIL_AT","registry"}};
                for (const auto &fault:admissionFaults) {
                    auto *candidate=TheThingFactory->newObject(treeTemplate,seed->getTeam());
                    auto *candidateDraw=candidate->getDrawable();
                    const auto objectImage=W3DTreeModuleGeneratedProbeAccess::image(*candidate);
                    const auto drawImage=W3DTreeModuleGeneratedProbeAccess::image(*candidateDraw);
                    const auto baselineResources=device.resource_counts();
                    UnsignedInt before[6]{},after[6]{};CopyGameClientRandomState(before);
                    setenv(fault.first,fault.second,1);
                    const bool failed=rejected([&]{candidate->setPosition(&initial);});
                    unsetenv(fault.first);CopyGameClientRandomState(after);
                    require(failed && W3DTreeModuleGeneratedProbeAccess::image(*candidate)==objectImage
                        && W3DTreeModuleGeneratedProbeAccess::image(*candidateDraw)==drawImage
                        && !terrain->treeInstanceCount() && !terrain->treeTypeCount()
                        && device.resource_counts()==baselineResources && !std::memcmp(before,after,sizeof(before)),
                        "tree module first-transform fault changed immediate baseline");
                    candidate->setPosition(&initial);
                    require(terrain->treeInstanceCount()==1,"tree module first-transform retry failed");
                    TheGameLogic->destroyObject(candidate);W3DTreeModuleGeneratedProbeAccess::process(*TheGameLogic);
                    require(!terrain->treeInstanceCount() && !terrain->treeTypeCount()
                        && device.resource_counts()==baselineResources,"tree module transform retry retained resources");
                }
                Object *object=TheThingFactory->newObject(treeTemplate,seed->getTeam());
                Drawable *draw=object->getDrawable();require(draw && draw->getDrawModules()[0],"tree module control drawable missing");
                auto *module=dynamic_cast<W3DTreeDraw *>(draw->getDrawModules()[0]);
                require(module && module->getModuleNameKey()==NAMEKEY("W3DTreeDraw"),
                    "tree module full factory selected wrong create proc");
                require(terrain->treeInstanceCount()==0,"tree module zero-XY did not defer admission");
                draw->setInstanceScale(0);
                const auto zeroObject=W3DTreeModuleGeneratedProbeAccess::image(*object);
                const auto zeroDraw=W3DTreeModuleGeneratedProbeAccess::image(*draw);
                const auto zeroFrame=device.snapshot();const auto zeroResources=device.resource_counts();
                UnsignedInt zeroRng[6]{},zeroAfter[6]{};CopyGameClientRandomState(zeroRng);
                require(rejected([&]{object->setPosition(&initial);}),"tree module zero-scale admission did not reject");
                CopyGameClientRandomState(zeroAfter);
                require(W3DTreeModuleGeneratedProbeAccess::image(*object)==zeroObject
                    && W3DTreeModuleGeneratedProbeAccess::image(*draw)==zeroDraw
                    && !terrain->treeInstanceCount() && device.snapshot()==zeroFrame
                    && device.resource_counts()==zeroResources && !std::memcmp(zeroRng,zeroAfter,sizeof(zeroRng)),
                    "tree module zero-scale rejection changed source/RNG/resources");
                draw->setInstanceScale(1);
                object->setPosition(&initial);
                require(terrain->treeInstanceCount()==1,"tree module public first transform did not register");
                object->setPosition(&initial);require(terrain->treeInstanceCount()==1,"tree module duplicate transform added twice");
                const auto ownerObjectImage=W3DTreeModuleGeneratedProbeAccess::image(*object);
                const auto ownerDrawImage=W3DTreeModuleGeneratedProbeAccess::image(*draw);
                TheTerrainRenderObject=nullptr;
                const Coord3D moved{13,19,3};
                const bool removedProvider=rejected([&]{object->setPosition(&moved);});
                TheTerrainRenderObject=terrain;
                require(removedProvider && terrain->treeInstanceCount()==1
                    && W3DTreeModuleGeneratedProbeAccess::image(*object)==ownerObjectImage
                    && W3DTreeModuleGeneratedProbeAccess::image(*draw)==ownerDrawImage,
                    "tree module provider removal changed accepted transforms");
                object->setPosition(&moved);object->setPosition(&initial);
                const auto id=draw->getID();const auto objectID=object->getID();
                const auto phaseResult=terrain->prepareTreeRenderPhase(camera);
                require(phaseResult==BaseHeightMapRenderObjClass::TREE_PHASE_READY,"tree module phase preparation rejected");
                const auto phase=terrain->preparedTreePhaseIdentity();const auto resources=device.resource_counts();
                edge.begin_source_frame(true,true,0,0,0,1);
                const auto baseline=device.snapshot();UnsignedInt rng[6]{},after[6]{};CopyGameClientRandomState(rng);
                transforms(*draw,[]{});transforms(*object,[]{});
                const auto check=[&](auto action) {
                    require(rejected(action),"tree module active-frame removal admitted");
                    CopyGameClientRandomState(after);
                    require(object->getDrawable()==draw && draw->getObject()==object && !object->isDestroyed()
                        && TheGameLogic->findObjectByID(objectID)==object && TheGameClient->findDrawableByID(id)==draw
                        && draw->getDrawModules()[0]==module && terrain->treeInstanceCount()==1
                        && terrain->preparedTreePhaseIdentity()==phase && device.resource_counts()==resources
                        && device.snapshot()==baseline && !std::memcmp(rng,after,sizeof(rng)),
                        "tree module active-frame rejection mutated accepted ownership");
                };
                check([&]{TheGameClient->destroyDrawable(draw);});
                check([&]{TheGameClient->reset();});check([&]{TheGameLogic->destroyObject(object);});
                check([&]{W3DTreeModuleGeneratedProbeAccess::destroyAll(*TheGameLogic);});check([&]{TheGameLogic->reset();});
                check([&]{draw->friend_rollbackConstruction();});check([&]{object->friend_rollbackConstruction();});
                check([&]{draw->friend_deleteInstance();});check([&]{object->friend_deleteInstance();});
                check([&]{module->deleteInstance();});check([&]{TheGameEngine->reset();});
                check([&]{TheSubsystemList->resetAll();});check([&]{visual.reset();});
                check([&]{terrain->reset();});check([&]{terrain->freeMapResources();});
                check([&]{terrain->removeAllTrees();});
                check([&]{TheThingFactory->newObject(treeTemplate,seed->getTeam());});
                W3DTreeModuleGeneratedProbeAccess::pending(*TheGameLogic,object);
                check([&]{W3DTreeModuleGeneratedProbeAccess::process(*TheGameLogic);});
                W3DTreeModuleGeneratedProbeAccess::unpending(*TheGameLogic,object);
                edge.abort_source_frame();
                TheGameLogic->destroyObject(object);W3DTreeModuleGeneratedProbeAccess::process(*TheGameLogic);
                require(!terrain->treeInstanceCount() && !terrain->treeTypeCount() && !terrain->preparedTreePhaseIdentity()
                    && !TheGameLogic->findObjectByID(objectID) && !TheGameClient->findDrawableByID(id),
                    "tree module idle retry retained publication");
                const auto *otherTemplate=TheThingFactory->findTemplate(AsciiString("ModuleTreeOtherFixture"),FALSE);
                require(otherTemplate,"tree module generated second type missing");
                auto *aliasA=TheThingFactory->newObject(treeTemplate,seed->getTeam());
                auto *aliasB=TheThingFactory->newObject(treeTemplate,seed->getTeam());
                auto *other=TheThingFactory->newObject(otherTemplate,seed->getTeam());
                aliasA->setPosition(&initial);aliasB->setPosition(&initial);other->setPosition(&initial);
                const auto survivingID=other->getDrawable()->getID();
                require(terrain->treeInstanceCount()==3 && terrain->treeTypeCount()==2,
                    "tree module aliases/types did not publish exact multiplicity");
                zh_probe_tree_module_destroy(aliasA);
                require(terrain->treeInstanceCount()==2 && terrain->treeTypeCount()==2,
                    "tree module alias removal retired surviving type");
                zh_probe_tree_module_destroy(aliasB);
                require(terrain->treeInstanceCount()==1 && terrain->treeTypeCount()==1
                    && TheGameClient->findDrawableByID(survivingID)==other->getDrawable(),
                    "tree module distinct-type compaction changed survivor identity");
                other->setPosition(&initial);
                require(terrain->prepareTreeRenderPhase(camera)==BaseHeightMapRenderObjClass::TREE_PHASE_READY,
                    "tree module compacted survivor could not prepare");
                zh_probe_tree_module_destroy(other);
                require(!terrain->treeInstanceCount() && !terrain->treeTypeCount()
                    && !terrain->preparedTreePhaseIdentity(),"tree module last alias retained registry/phase");
                auto *stale=TheThingFactory->newObject(treeTemplate,seed->getTeam());stale->setPosition(&initial);
                const auto staleEpoch=terrain->treeOwnerEpoch();terrain->removeAllTrees();
                auto *replacement=TheThingFactory->newObject(treeTemplate,seed->getTeam());replacement->setPosition(&initial);
                const auto replacementID=replacement->getDrawable()->getID();
                require(terrain->treeOwnerEpoch()!=staleEpoch && rejected([&]{stale->setPosition(&moved);}),
                    "tree module stale epoch retargeted replacement registry");
                zh_probe_tree_module_destroy(stale);
                require(terrain->treeInstanceCount()==1 && TheGameClient->findDrawableByID(replacementID)==replacement->getDrawable(),
                    "tree module stale detach removed replacement identity");
                replacement->setPosition(&moved);zh_probe_tree_module_destroy(replacement);
                require(!terrain->treeInstanceCount() && !terrain->treeTypeCount(),
                    "tree module replacement retry retained registry");
                const auto *slopeTemplate=TheThingFactory->findTemplate(AsciiString("ModuleTreeSlopeFixture"),FALSE);
                require(slopeTemplate,"tree module generated slope template missing");
                auto *slope=TheThingFactory->newObject(slopeTemplate,seed->getTeam());slope->setPosition(&initial);
                edge.begin_source_frame(true,true,0,0,0,1);
                transforms(*slope->getDrawable(),[]{});transforms(*slope,[]{});
                edge.abort_source_frame();zh_probe_tree_module_destroy(slope);
                require(!terrain->treeInstanceCount() && !terrain->treeTypeCount(),
                    "tree module nested slope rollback retained ownership");
                visual.reset();edge.release_source_buffers();device.destroy(depth);device.destroy(color);
            } catch (...) {
                edge.abort_source_frame();TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;throw;
            }
            TheTacticalView=savedView;TheTerrainVisual=savedVisual;TheDisplay=savedDisplay;
        }
        require(!device.resource_counts().total(),"tree module generation retained resources");
    }
    if (std::getenv("ZH_M22_TREE_MODULE_PHYSICAL")) zh_probe_tree_module_physical(path,asset);
    TheWritableGlobalData->m_useTrees=oldTrees;TheWritableGlobalData->m_partitionCellSize=oldPartition;
    std::puts("original tree module: layout=1 transform=1 preflight=1 retry=1 generations=2 resources=0");
}
