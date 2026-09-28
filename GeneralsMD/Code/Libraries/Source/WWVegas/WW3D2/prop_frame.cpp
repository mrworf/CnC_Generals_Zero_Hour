#include "prop_frame.h"
#if defined(ZH_WW3D_CPU_ONLY)
#include "assetmgr.h"
#include "prop_graph.h"
#include "animobj.h"
#include "hlod.h"
#include "collect.h"
#include "mesh.h"
#include "meshmdl.h"
#include "nullrobj.h"
#include "boxrobj.h"
#include "light.h"
#include "mapper.h"
#include "clone_graph.h"
#include "hanimmgr.h"
#include "hrawanim.h"
#include "hcanim.h"
#include "pivot.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <typeinfo>

namespace ww3d_prop {
namespace {
constexpr std::size_t max_nodes=4096,max_bytes=64U*1024U*1024U;
void reject() { throw std::runtime_error("original prop frame graph is not admitted"); }
bool finite(const Matrix3D& matrix) noexcept
{
    for (int r=0;r<3;++r) for (int c=0;c<4;++c)
        if (!std::isfinite(matrix[r][c])) return false;
    return true;
}
}
struct FrameGraph::State {
    struct Object {
        RenderObjClass *identity;
        unsigned long bits;
        Matrix3D transform;
        SphereClass sphere;
        AABoxClass box;
        float size,scale;
        bool transform_identity;
        RenderObjClass *container;
    };
    struct Composite {
        CompositeRenderObjClass *identity;
        SphereClass sphere;
        AABoxClass box;
    };
    struct Mesh {
        MeshClass *identity;
        MeshModelClass *model;
        LightEnvironmentClass *environment;
        std::array<unsigned char,sizeof(LightEnvironmentClass)> local;
        float alpha,pass_alpha,emissive;
        int offset;
        MeshClass *next;
    };
    struct Pivot { Matrix3D transform;bool visible; };
    struct Animation {
        Animatable3DObjClass *identity;
        HTreeClass *tree;
        bool valid;
        int mode;
        std::vector<Pivot> pivots;
        HAnimClass *motion0=nullptr,*motion1=nullptr;
        float frame0=0,frame1=0,previous0=0,previous1=0,percentage=0;
        int anim_mode=0,sync=0;
        float direction=0,multiplier=0;
    };
    struct Hierarchy { HLodClass *identity;int lod; };
    struct Collection { CollectionClass *identity;SphereClass sphere;AABoxClass box; };
    struct Light { LightClass *identity;Vector3 ambient,diffuse,specular; };
    struct Material { VertexMaterialClass *identity;std::shared_ptr<VertexMaterialClass::SourceFrameState> state; };
    struct Mapper { TextureMapperClass *identity;std::shared_ptr<TextureMapperClass::SourceFrameState> state; };
    WW3DAssetManager *assets;
    std::vector<Object> objects;
    std::vector<Composite> composites;
    std::vector<Mesh> mesh_states;
    std::vector<MeshClass*> mesh_identities;
    std::vector<Animation> animations;
    std::vector<Hierarchy> hierarchies;
    std::vector<Collection> collections;
    std::vector<Light> lights;
    std::vector<Material> materials;
    std::vector<Mapper> mappers;
    Random4Class random=ww3d_clone::capture_mapper_random();
    bool random_used=false;
    std::array<RenderObjClass*,256> path{};
    std::vector<HAnimClass*> motions;
    std::size_t bytes=0;
    bool pinned=false;
    explicit State(WW3DAssetManager *p):assets(p) {}
    ~State() { if (pinned) for (const auto& object:objects) object.identity->Release_Ref(); }
    void extent(std::size_t count,std::size_t element)
    {
        if (count>max_nodes || element>max_bytes || count>(max_bytes-bytes)/element) reject();
        bytes+=count*element;
    }
    bool registered_motion(HAnimClass *motion)
    {
        // Compare identities before calling through the provider. This does
        // not load, resolve a name, increment refs, or synthesize a resource.
        HAnimManagerIterator it(assets->HAnimManager);
        unsigned count=0;
        for (it.First();!it.Is_Done();it.Next()) {
            if (count++==max_nodes) reject();
            if (it.Get_Current_Anim()==motion) return true;
        }
        return false;
    }
    void motion(HAnimClass *p,int pivots)
    {
        if (!p || !registered_motion(p)) reject();
        // Morph animation has a separate vertex mutation path. Unknown
        // virtual providers cannot prove a sound-free immutable visual graph.
        if ((typeid(*p)!=typeid(HRawAnimClass) && typeid(*p)!=typeid(HCompressedAnimClass))
            || p->Has_Embedded_Sounds() || p->Get_Num_Pivots()!=pivots
            || p->Get_Num_Frames()<=0 || !std::isfinite(p->Get_Frame_Rate())
            || p->Get_Frame_Rate()<=0 || p->Num_Refs()<=0) reject();
        if (std::find(motions.begin(),motions.end(),p)==motions.end()) motions.push_back(p);
    }
    void animation(Animatable3DObjClass *p)
    {
        Animation a{p,p->HTree,p->IsTreeValid,p->CurMotionMode,{}};
        if (a.tree) {
            Audit audit(*assets);audit.hierarchy(a.tree);
            extent(static_cast<std::size_t>(a.tree->NumPivots),sizeof(Pivot));
            a.pivots.reserve(a.tree->NumPivots);
            for (int i=0;i<a.tree->NumPivots;++i) {
                if (!finite(a.tree->Pivot[i].Transform)) reject();
                a.pivots.push_back({a.tree->Pivot[i].Transform,a.tree->Pivot[i].IsVisible});
            }
        }
        switch (a.mode) {
        case Animatable3DObjClass::NONE:
        case Animatable3DObjClass::BASE_POSE: break;
        case Animatable3DObjClass::SINGLE_ANIM:
            if (!a.tree) reject();motion(p->ModeAnim.Motion,a.tree->NumPivots);
            a.motion0=p->ModeAnim.Motion;a.frame0=p->ModeAnim.Frame;a.previous0=p->ModeAnim.PrevFrame;
            a.anim_mode=p->ModeAnim.AnimMode;a.sync=p->ModeAnim.LastSyncTime;
            a.direction=p->ModeAnim.animDirection;a.multiplier=p->ModeAnim.frameRateMultiplier;
            if (!std::isfinite(a.frame0) || !std::isfinite(a.previous0) || !std::isfinite(a.direction)
                || !std::isfinite(a.multiplier) || a.anim_mode<RenderObjClass::ANIM_MODE_MANUAL
                || a.anim_mode>RenderObjClass::ANIM_MODE_ONCE_BACKWARDS) reject();
            break;
        case Animatable3DObjClass::DOUBLE_ANIM:
            if (!a.tree) reject();
            // Native transform update asks both providers about embedded
            // sounds unconditionally. A null half is not safely renderable.
            motion(p->ModeInterp.Motion0,a.tree->NumPivots);
            motion(p->ModeInterp.Motion1,a.tree->NumPivots);
            a.motion0=p->ModeInterp.Motion0;a.motion1=p->ModeInterp.Motion1;
            a.frame0=p->ModeInterp.Frame0;a.frame1=p->ModeInterp.Frame1;
            a.previous0=p->ModeInterp.PrevFrame0;a.previous1=p->ModeInterp.PrevFrame1;
            a.percentage=p->ModeInterp.Percentage;
            if (!std::isfinite(a.frame0) || !std::isfinite(a.frame1) || !std::isfinite(a.previous0)
                || !std::isfinite(a.previous1) || !std::isfinite(a.percentage)) reject();
            break;
        case Animatable3DObjClass::MULTIPLE_ANIM:
            // The native combo is externally owned and has no store/provider
            // identity. It is indeterminate at this bounded frame boundary;
            // reject before dereferencing its pointer or triggering any sound.
            reject();break;
        default: reject();
        }
        animations.push_back(std::move(a));
    }
    void material(VertexMaterialClass *p)
    {
        if (!p || std::find_if(materials.begin(),materials.end(),[p](const auto& m){return m.identity==p;})!=materials.end()) return;
        if (materials.size()==max_nodes) reject();
        auto snapshot=p->captureSourceFrame();
        materials.push_back({p,std::move(snapshot)});
        for (unsigned stage=0;stage<MeshBuilderClass::MAX_STAGES;++stage) {
            auto *mapper=p->Peek_Mapper(stage);
            if (!mapper || std::find_if(mappers.begin(),mappers.end(),[mapper](const auto& m){return m.identity==mapper;})!=mappers.end()) continue;
            if (mappers.size()==max_nodes) reject();
            auto snapshot=mapper->captureSourceFrame();
            random_used=random_used || typeid(*mapper)==typeid(RandomTextureMapperClass);
            mappers.push_back({mapper,std::move(snapshot)});
        }
    }
    void visit(RenderObjClass *p,unsigned depth)
    {
        if (!p || depth==path.size() || objects.size()==max_nodes) reject();
        for (unsigned i=0;i<depth;++i) if (path[i]==p) reject();
        if (std::find_if(objects.begin(),objects.end(),[p](const Object& o){return o.identity==p;})!=objects.end()) return;
        if (p->Num_Refs()<=0 || p->Num_Refs()>std::numeric_limits<int>::max()-32
            || p->Get_Render_Hook() || !finite(p->Transform) || !std::isfinite(p->ObjectScale)) reject();
        extent(1,sizeof(Object));path[depth]=p;
        objects.push_back({p,p->Bits,p->Transform,p->CachedBoundingSphere,p->CachedBoundingBox,
            p->NativeScreenSize,p->ObjectScale,p->IsTransformIdentity,p->Container});
        if (auto *composite=dynamic_cast<CompositeRenderObjClass*>(p))
            composites.push_back({composite,composite->ObjSphere,composite->ObjBox});
        if (typeid(*p)==typeid(HLodClass)) {
            auto *h=static_cast<HLodClass*>(p);
            if (h->LodCount<=0 || h->LodCount>1024 || !h->Lod || h->CurLod<0 || h->CurLod>=h->LodCount) reject();
            animation(h);hierarchies.push_back({h,h->CurLod});
            for (int l=0;l<h->LodCount;++l) {
                if (h->Lod[l].Count()<0 || h->Lod[l].Count()>static_cast<int>(max_nodes)) reject();
                for (int i=0;i<h->Lod[l].Count();++i) visit(h->Lod[l][i].Model,depth+1);
            }
            if (h->AdditionalModels.Count()<0 || h->AdditionalModels.Count()>static_cast<int>(max_nodes)) reject();
            for (int i=0;i<h->AdditionalModels.Count();++i) visit(h->AdditionalModels[i].Model,depth+1);
        } else if (typeid(*p)==typeid(CollectionClass)) {
            auto *c=static_cast<CollectionClass*>(p);
            if (c->SubObjects.Count()<0 || c->SubObjects.Count()>static_cast<int>(max_nodes)) reject();
            collections.push_back({c,c->BoundSphere,c->BoundBox});
            for (int i=0;i<c->SubObjects.Count();++i) visit(c->SubObjects[i],depth+1);
        } else if (typeid(*p)==typeid(MeshClass)) {
            auto *m=static_cast<MeshClass*>(p);
            if (!m->Model || m->DecalMesh) reject();
            Mesh snapshot{m,m->Model,m->LightEnvironment,{},
                m->m_alphaOverride,m->m_materialPassAlphaOverride,m->m_materialPassEmissiveOverride,
                m->BaseVertexOffset,m->NextVisibleSkin};
            // Native default environments leave inactive light slots
            // unspecified; preserve bytes rather than evaluating those slots.
            std::memcpy(snapshot.local.data(),&m->m_localLightEnv,sizeof(m->m_localLightEnv));
            mesh_states.push_back(snapshot);
            mesh_identities.push_back(m);
            if (m->Model->Get_Pass_Count()<1 || m->Model->Get_Pass_Count()>4 ||
                m->Model->Get_Vertex_Count()<1 || m->Model->Get_Vertex_Count()>65535) reject();
            for (unsigned pass=0;pass<m->Model->Get_Pass_Count();++pass) {
                if (auto *single=m->Model->Peek_Single_Material(pass)) material(single);
                else for (int vertex=0;vertex<m->Model->Get_Vertex_Count();++vertex)
                    material(m->Model->Peek_Material(vertex,pass));
            }
        } else if (typeid(*p)==typeid(LightClass)) {
            auto *light=static_cast<LightClass*>(p);Light snapshot{light,{},{},{}};
            light->Get_Ambient(&snapshot.ambient);light->Get_Diffuse(&snapshot.diffuse);light->Get_Specular(&snapshot.specular);
            lights.push_back(snapshot);
        } else if (typeid(*p)!=typeid(Null3DObjClass) && typeid(*p)!=typeid(AABoxRenderObjClass)
            && typeid(*p)!=typeid(OBBoxRenderObjClass)) reject();
    }
    void restore() noexcept
    {
        for (const auto& material:materials) material.identity->restoreSourceFrame(*material.state);
        for (const auto& mapper:mappers) mapper.identity->restoreSourceFrame(*mapper.state);
        if (random_used) ww3d_clone::restore_mapper_random(random);
        for (const auto& a:animations) {
            auto *p=a.identity;
            if (p->HTree!=a.tree || p->CurMotionMode!=a.mode) std::terminate();
            p->IsTreeValid=a.valid;
            if (a.tree) {
                if (a.tree->NumPivots!=static_cast<int>(a.pivots.size())) std::terminate();
                for (int i=0;i<a.tree->NumPivots;++i) {
                    a.tree->Pivot[i].Transform=a.pivots[i].transform;
                    a.tree->Pivot[i].IsVisible=a.pivots[i].visible;
                }
            }
            if (a.mode==Animatable3DObjClass::SINGLE_ANIM) {
                if (p->ModeAnim.Motion!=a.motion0) std::terminate();
                p->ModeAnim.Frame=a.frame0;p->ModeAnim.PrevFrame=a.previous0;
                p->ModeAnim.AnimMode=a.anim_mode;p->ModeAnim.LastSyncTime=a.sync;
                p->ModeAnim.animDirection=a.direction;p->ModeAnim.frameRateMultiplier=a.multiplier;
            } else if (a.mode==Animatable3DObjClass::DOUBLE_ANIM) {
                if (p->ModeInterp.Motion0!=a.motion0 || p->ModeInterp.Motion1!=a.motion1) std::terminate();
                p->ModeInterp.Frame0=a.frame0;p->ModeInterp.Frame1=a.frame1;
                p->ModeInterp.PrevFrame0=a.previous0;p->ModeInterp.PrevFrame1=a.previous1;
                p->ModeInterp.Percentage=a.percentage;
            }
        }
        for (const auto& h:hierarchies) h.identity->CurLod=h.lod;
        for (const auto& c:composites) { c.identity->ObjSphere=c.sphere;c.identity->ObjBox=c.box; }
        for (const auto& c:collections) { c.identity->BoundSphere=c.sphere;c.identity->BoundBox=c.box; }
        for (const auto& m:mesh_states) {
            if (m.identity->Model!=m.model) std::terminate();
            m.identity->LightEnvironment=m.environment;
            std::memcpy(&m.identity->m_localLightEnv,m.local.data(),sizeof(m.identity->m_localLightEnv));
            m.identity->m_alphaOverride=m.alpha;m.identity->m_materialPassAlphaOverride=m.pass_alpha;
            m.identity->m_materialPassEmissiveOverride=m.emissive;
            m.identity->BaseVertexOffset=m.offset;m.identity->NextVisibleSkin=m.next;
        }
        for (const auto& light:lights) {
            light.identity->Set_Ambient(light.ambient);light.identity->Set_Diffuse(light.diffuse);light.identity->Set_Specular(light.specular);
        }
        for (const auto& o:objects) {
            if (o.identity->Container!=o.container) std::terminate();
            o.identity->Bits=o.bits;o.identity->Transform=o.transform;
            o.identity->CachedBoundingSphere=o.sphere;o.identity->CachedBoundingBox=o.box;
            o.identity->NativeScreenSize=o.size;o.identity->ObjectScale=o.scale;
            o.identity->IsTransformIdentity=o.transform_identity;
        }
    }
};
FrameGraph::FrameGraph(std::unique_ptr<State> p):state(std::move(p)) {}
FrameGraph::~FrameGraph()=default;
std::shared_ptr<FrameGraph> FrameGraph::capture(const std::vector<RenderObjClass*>& roots)
{
    auto *assets=WW3DAssetManager::Get_Instance();
    if (!assets || roots.size()>max_nodes) reject();
    auto candidate=std::unique_ptr<State>(new State(assets));
    for (auto *root:roots) candidate->visit(root,0);
    // No source refs or state have changed before complete graph admission.
    for (const auto& object:candidate->objects) object.identity->Add_Ref();
    candidate->pinned=true;
    return std::shared_ptr<FrameGraph>(new FrameGraph(std::move(candidate)));
}
void FrameGraph::restore() noexcept { state->restore(); }
const std::vector<MeshClass*>& FrameGraph::meshes() const noexcept { return state->mesh_identities; }
}
#endif
