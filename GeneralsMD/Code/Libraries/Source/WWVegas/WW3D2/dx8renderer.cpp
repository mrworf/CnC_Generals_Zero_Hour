/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8renderer.cpp                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 111                                                         $*
 *                                                                                             *
 * 06/27/02 KM Changes to max texture stage caps																*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define ENABLE_CATEGORY_LOG
//#define ENABLE_STRIPING

#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "dx8polygonrenderer.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#if !defined(ZH_WW3D_CPU_ONLY)
#include "dx8caps.h"
#include "dx8rendererdebugger.h"
#endif
#include "wwdebug.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "rinfo.h"
#include "statistics.h"
#include "meshmdl.h"
#include "vp.h"
#include "decalmsh.h"
#include "matpass.h"
#include "camera.h"
#include "stripoptimizer.h"
#include "meshgeometry.h"
#include "texture.h"
#if defined(ZH_WW3D_CPU_ONLY)
#include "original_gpu_edge.h"
#include "prop_frame.h"
#include "clone_graph.h"
#include "static_sort_list.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>
#include <limits>
#endif

namespace {
#if defined(ZH_WW3D_CPU_ONLY)
constexpr unsigned fvf_xyz = 0x002, fvf_normal = 0x010;
constexpr unsigned fvf_diffuse = 0x040, fvf_specular = 0x080;
constexpr unsigned fvf_tex1 = 0x100, fvf_tex2 = 0x200;
constexpr unsigned fvf_tex3 = 0x300, fvf_tex4 = 0x400;
constexpr unsigned fvf_tex5 = 0x500, fvf_tex6 = 0x600;
constexpr unsigned fvf_tex7 = 0x700, fvf_tex8 = 0x800;
#else
constexpr unsigned fvf_xyz = D3DFVF_XYZ, fvf_normal = D3DFVF_NORMAL;
constexpr unsigned fvf_diffuse = D3DFVF_DIFFUSE, fvf_specular = D3DFVF_SPECULAR;
constexpr unsigned fvf_tex1 = D3DFVF_TEX1, fvf_tex2 = D3DFVF_TEX2;
constexpr unsigned fvf_tex3 = D3DFVF_TEX3, fvf_tex4 = D3DFVF_TEX4;
constexpr unsigned fvf_tex5 = D3DFVF_TEX5, fvf_tex6 = D3DFVF_TEX6;
constexpr unsigned fvf_tex7 = D3DFVF_TEX7, fvf_tex8 = D3DFVF_TEX8;
#endif
}

/*
** Global Instance of the DX8MeshRender
*/
DX8MeshRendererClass TheDX8MeshRenderer;
#if defined(ZH_WW3D_CPU_ONLY)
bool DX8FVFCategoryContainer::Source_Frame_Empty() noexcept
{
    if (AnythingToRender || AnyDelayedPassesToRender || visible_matpass_head || visible_matpass_tail) return false;
    for (auto& list:visible_texture_category_list) if (!list.Is_Empty()) return false;
    return true;
}
#endif
bool DX8TextureCategoryClass::m_gForceMultiply = false; // Forces opaque materials to use the multiply blend - pseudo transparent effect.  jba.
// ----------------------------------------------------------------------------

static DynamicVectorClass<Vector3>				_TempVertexBuffer;
static DynamicVectorClass<Vector3>				_TempNormalBuffer;

static MultiListClass<MeshModelClass>			_RegisteredMeshList;
static TextureCategoryList							texture_category_delete_list;
static FVFCategoryList								fvf_category_container_delete_list;

#if defined(ZH_WW3D_CPU_ONLY)
bool DX8MeshRendererClass::Source_Frame_Queues_Empty() const noexcept
{
    if (visible_decal_meshes || !texture_category_delete_list.Is_Empty()
        || !fvf_category_container_delete_list.Is_Empty()) return false;
    const auto empty=[](FVFCategoryList* list) {
        if (!list) return true;
        FVFCategoryListIterator it(list);
        for (;!it.Is_Done();it.Next()) if (!it.Peek_Obj()->Source_Frame_Empty()) return false;
        return true;
    };
    for (int i=0;i<texture_category_container_lists_rigid.Count();++i)
        if (!empty(texture_category_container_lists_rigid[i])) return false;
    return empty(texture_category_container_list_skin);
}
#endif

// helper data structure
class PolyRemover : public MultiListObjectClass
{
public:
	DX8TextureCategoryClass *	src;
	DX8TextureCategoryClass *	dest;
	DX8PolygonRendererClass *  pr;
};

typedef MultiListClass<PolyRemover>			PolyRemoverList;
typedef MultiListIterator<PolyRemover>		PolyRemoverListIterator;

#define VERTEX_BUFFER_OVERFLOW	0xffff		//'Generals' flag to signal when a mesh didn't fit in streaming vertex buffer.

/**
** PolyRenderTaskClass
** This is a record of a polyrendere that needs to be rendered
** for this frame.  Since MeshClass instances can share meshmodels
** (and therefore their dx8 polygon renderers) this record contains
** a pointer to the polygon renderer and the MeshClass instance that
** it is being rendered for. 
*/
class PolyRenderTaskClass : public AutoPoolClass<PolyRenderTaskClass, 256>
{
public:
#if defined(ZH_WW3D_CPU_ONLY)
    PolyRenderTaskClass():Renderer(NULL),Mesh(NULL),NextVisible(NULL) {}
    void Bind(DX8PolygonRendererClass *renderer,MeshClass *mesh) noexcept
    { Renderer=renderer;Mesh=mesh;NextVisible=NULL;Mesh->Add_Ref(); }
#endif
	PolyRenderTaskClass(DX8PolygonRendererClass * p_renderer,MeshClass * p_mesh) :
		Renderer(p_renderer),
		Mesh(p_mesh),
		NextVisible(NULL)
	{
		WWASSERT(Renderer != NULL);
		WWASSERT(Mesh != NULL);
		Mesh->Add_Ref();
	}

	~PolyRenderTaskClass(void)
	{
		if (Mesh) Mesh->Release_Ref();
	}

	DX8PolygonRendererClass *	Peek_Polygon_Renderer(void)							{ return Renderer; }
	MeshClass *						Peek_Mesh(void)											{ return Mesh; }

	PolyRenderTaskClass *		Get_Next_Visible(void)									{ return NextVisible; }
	void								Set_Next_Visible(PolyRenderTaskClass * prtc)		{ NextVisible = prtc; }

protected:

	DX8PolygonRendererClass *	Renderer;
	MeshClass *						Mesh;
	PolyRenderTaskClass *		NextVisible;

};

DEFINE_AUTO_POOL(PolyRenderTaskClass, 256);

/**
** MatPassTaskClass
** This is the record of a material pass that needs to be rendered on
** a particular mesh.  These are linked into the FVF container which
** contains the mesh model.  They are also pooled to remove memory 
** allocation overhead.
*/
class MatPassTaskClass : public AutoPoolClass<MatPassTaskClass, 256>
{
public:
#if defined(ZH_WW3D_CPU_ONLY)
    MatPassTaskClass():MaterialPass(NULL),Mesh(NULL),NextVisible(NULL) {}
    void Bind(MaterialPassClass *pass,MeshClass *mesh) noexcept
    { MaterialPass=pass;Mesh=mesh;NextVisible=NULL;MaterialPass->Add_Ref();Mesh->Add_Ref(); }
#endif
	MatPassTaskClass(MaterialPassClass * pass,MeshClass * mesh) :
		MaterialPass(pass),
		Mesh(mesh),
		NextVisible(NULL)
	{
		WWASSERT(MaterialPass != NULL);
		WWASSERT(Mesh != NULL);
		MaterialPass->Add_Ref();
		Mesh->Add_Ref();
	}

	~MatPassTaskClass(void)
	{
		if (MaterialPass) MaterialPass->Release_Ref();
		if (Mesh) Mesh->Release_Ref();
	}
	
	MaterialPassClass *	Peek_Material_Pass(void)							{ return MaterialPass; }
	MeshClass *				Peek_Mesh(void)										{ return Mesh; }
	
	MatPassTaskClass *	Get_Next_Visible(void)								{ return NextVisible; }
	void						Set_Next_Visible(MatPassTaskClass * mpr)		{ NextVisible = mpr; }

private:

	MaterialPassClass *	MaterialPass;
	MeshClass *				Mesh;
	MatPassTaskClass *	NextVisible;
};

DEFINE_AUTO_POOL(MatPassTaskClass, 256);

#if defined(ZH_WW3D_CPU_ONLY)
struct DX8MeshRendererClass::SourceFrameCheckpoint {
    struct List { GenericMultiListClass *identity;MultiListNodeClass *next,*previous; };
    struct Node {
        MultiListNodeClass *identity,*next,*previous,*next_list;
        MultiListObjectClass *object;
        GenericMultiListClass *list;
    };
    struct Object { MultiListObjectClass *identity;MultiListNodeClass *head; };
    struct Poly { PolyRenderTaskClass *identity,*next; };
    struct Material { MatPassTaskClass *identity,*next; };
    struct Category { DX8TextureCategoryClass *identity;PolyRenderTaskClass *head; };
    struct Vertex { VertexBufferClass *identity;std::vector<unsigned char> bytes; };
    struct Index { IndexBufferClass *identity;std::vector<unsigned short> bytes; };
    struct Scratch {
        Vector3 *original=nullptr;
        int capacity=0,count=0,growth=0;
        bool valid=false,allocated=false;
        std::unique_ptr<Vector3[]> candidate;
        std::vector<unsigned char> bytes;
        unsigned required=0;
    };
    struct Container {
        DX8FVFCategoryContainer *identity;
        MatPassTaskClass *head,*tail;
        bool anything,delayed;
        DX8RigidFVFCategoryContainer *rigid;
        MatPassTaskClass *delayed_head,*delayed_tail;
        DX8SkinFVFCategoryContainer *skin;
        MeshClass *skin_head,*skin_tail;
        unsigned skin_count;
        int used_indices;
        int used_vertices;
    };
    DX8MeshRendererClass *owner=nullptr;
    std::vector<List> lists;
    std::vector<Node> nodes;
    std::vector<Object> objects;
    std::vector<Poly> poly_tasks;
    std::vector<Material> material_tasks;
    std::vector<Category> categories;
    std::vector<Container> containers;
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    bool buffers_pinned=false;
    Scratch skin_vertices,skin_normals;
    std::vector<MultiListNodeClass*> detached;
    std::vector<MultiListNodeClass*> node_slots;
    std::vector<PolyRenderTaskClass*> poly_slots,retired_poly;
    std::vector<MatPassTaskClass*> material_slots,retired_material;
    std::vector<DX8TextureCategoryClass*> retired_categories;
    std::vector<DX8FVFCategoryContainer*> retired_containers;
    std::shared_ptr<ww3d_prop::FrameGraph> graph;
    DefaultStaticSortListClass *sorts=nullptr;
    unsigned min_sort=0,max_sort=0;
    std::vector<RenderObjClass*> candidate_render_refs,retired_baseline_render_refs;
    unsigned poly_used=0,material_used=0,node_used=0;
    bool closed=false;
    ~SourceFrameCheckpoint();
};
namespace {
DX8MeshRendererClass::SourceFrameCheckpoint *source_mesh_frame=nullptr;
DX8PolygonRendererList *source_prepared_polygons=nullptr;
MeshModelClass *source_prepared_model=nullptr;
struct SourceRegistrationBatch {
    struct Container { DX8FVFCategoryContainer *root,*latest;FVFCategoryList *list; };
    struct Group { bool skin;unsigned fvf;FVFCategoryList *list; };
    struct Category { DX8TextureCategoryClass *candidate,*root; };
    std::vector<Container> containers;
    std::vector<Group> groups;
    std::vector<Category> categories;
    std::vector<std::function<void()>> publications;
    std::size_t bytes=0;
};
SourceRegistrationBatch *source_registration_batch=nullptr;
constexpr unsigned source_node_bound=32768;
PolyRenderTaskClass *source_poly_task(DX8PolygonRendererClass *renderer,MeshClass *mesh)
{
    if (!source_mesh_frame) return new PolyRenderTaskClass(renderer,mesh);
    auto& c=*source_mesh_frame;
    if (!renderer || !mesh || c.poly_used==c.poly_slots.size())
        throw std::runtime_error("original source polygon task capacity rejected");
    auto *task=c.poly_slots[c.poly_used++];task->Bind(renderer,mesh);return task;
}
MatPassTaskClass *source_material_task(MaterialPassClass *pass,MeshClass *mesh)
{
    if (!source_mesh_frame) return new MatPassTaskClass(pass,mesh);
    auto& c=*source_mesh_frame;
    if (!pass || !mesh || c.material_used==c.material_slots.size())
        throw std::runtime_error("original source material task capacity rejected");
    auto *task=c.material_slots[c.material_used++];task->Bind(pass,mesh);return task;
}
void source_retire(PolyRenderTaskClass *task) noexcept
{
    if (!source_mesh_frame) { delete task;return; }
    auto& c=*source_mesh_frame;
    if (c.retired_poly.size()==c.retired_poly.capacity()) std::terminate();
    c.retired_poly.push_back(task);
}
void source_retire(MatPassTaskClass *task) noexcept
{
    if (!source_mesh_frame) { delete task;return; }
    auto& c=*source_mesh_frame;
    if (c.retired_material.size()==c.retired_material.capacity()) std::terminate();
    c.retired_material.push_back(task);
}
bool source_baseline_node(const DX8MeshRendererClass::SourceFrameCheckpoint& c,MultiListNodeClass *node) noexcept
{
    return std::find_if(c.nodes.begin(),c.nodes.end(),[node](const auto& n){return n.identity==node;})!=c.nodes.end();
}
bool source_candidate_node(const DX8MeshRendererClass::SourceFrameCheckpoint& c,MultiListNodeClass *node) noexcept
{
    return std::find(c.node_slots.begin(),c.node_slots.end(),node)!=c.node_slots.end();
}
void source_unlink(MultiListNodeClass *node) noexcept
{
    auto *head=node->Object->Get_List_Node();
    MultiListNodeClass *previous=nullptr;
    for (auto *p=head;p && p!=node;p=p->NextList) previous=p;
    if (previous) previous->NextList=node->NextList;
    else node->Object->Set_List_Node(node->NextList);
    node->Prev->Next=node->Next;node->Next->Prev=node->Prev;
    node->List=nullptr;node->Prev=node->Next=node->NextList=nullptr;
}
}
DX8MeshRendererClass::SourceFrameCheckpoint::~SourceFrameCheckpoint()
{
    if (!closed && source_mesh_frame==this) DX8MeshRendererClass::Restore_Source_Frame(*this);
    for (auto *task:poly_slots) delete task;
    for (auto *task:material_slots) delete task;
    for (auto *node:node_slots) { if (node->List) std::terminate();delete node; }
    for (auto *object:candidate_render_refs) object->Release_Ref();
    if (buffers_pinned) {
        for (const auto& vertex:vertices) vertex.identity->Release_Ref();
        for (const auto& index:indices) index.identity->Release_Ref();
    }
}
bool DX8MeshRendererClass::Source_Frame_Checkpoint_Active() noexcept { return source_mesh_frame!=nullptr; }
std::pair<Vector3*,unsigned> DX8MeshRendererClass::Peek_Source_Skin_Scratch(bool normals) noexcept
{
    const auto& buffer=normals?_TempNormalBuffer:_TempVertexBuffer;
    return {buffer.Vector,static_cast<unsigned>(buffer.VectorMax)};
}
void DX8MeshRendererClass::Prepare_Source_Skin_Scratch(unsigned vertices)
{
    if (!vertices || vertices>65535) throw std::runtime_error("original source skin scratch range rejected");
    if (!source_mesh_frame) {
        if (_TempVertexBuffer.Length()<static_cast<int>(vertices) && !_TempVertexBuffer.Resize(vertices)) throw std::bad_alloc();
        if (_TempNormalBuffer.Length()<static_cast<int>(vertices) && !_TempNormalBuffer.Resize(vertices)) throw std::bad_alloc();
        return;
    }
    if (vertices>static_cast<unsigned>(_TempVertexBuffer.VectorMax) || vertices>static_cast<unsigned>(_TempNormalBuffer.VectorMax))
        throw std::runtime_error("original source skin scratch capacity was not admitted");
    source_mesh_frame->skin_vertices.required=std::max(vertices,source_mesh_frame->skin_vertices.required);
    source_mesh_frame->skin_normals.required=std::max(vertices,source_mesh_frame->skin_normals.required);
}
std::shared_ptr<DX8MeshRendererClass::SourceFrameCheckpoint> DX8MeshRendererClass::Capture_Source_Frame(unsigned bound,DefaultStaticSortListClass *sorts)
{
    if (source_mesh_frame || !bound || bound>4096 || visible_decal_meshes)
        throw std::runtime_error("original source mesh frame admission rejected");
    auto c=std::make_shared<SourceFrameCheckpoint>();c->owner=this;
    std::size_t buffer_bytes=0;
    unsigned skin_vertices=0;
    const auto vertex_buffer=[&](VertexBufferClass *p) {
        if (!p || std::find_if(c->vertices.begin(),c->vertices.end(),[p](const auto& v){return v.identity==p;})!=c->vertices.end()) return;
        if (p->Num_Refs()<=0 || p->Num_Refs()>std::numeric_limits<int>::max()-32 ||
            (p->Type()!=BUFFER_TYPE_DX8 && p->Type()!=BUFFER_TYPE_SORTING))
            throw std::runtime_error("original source vertex checkpoint provider rejected");
        const std::size_t size=std::size_t(p->Get_Vertex_Count())*p->FVF_Info().Get_FVF_Size();
        if (size>64U*1024U*1024U-buffer_bytes) throw std::runtime_error("original source buffer checkpoint byte bound rejected");
        const auto *bytes=p->Type()==BUFFER_TYPE_DX8?static_cast<DX8VertexBufferClass*>(p)->Get_CPU_Vertex_Buffer():
            reinterpret_cast<const unsigned char*>(static_cast<SortingVertexBufferClass*>(p)->VertexBuffer);
        if (!bytes || !size) throw std::runtime_error("original source vertex checkpoint bytes rejected");
        c->vertices.push_back({p,std::vector<unsigned char>(bytes,bytes+size)});buffer_bytes+=size;
    };
    const auto index_buffer=[&](IndexBufferClass *p) {
        if (!p || std::find_if(c->indices.begin(),c->indices.end(),[p](const auto& v){return v.identity==p;})!=c->indices.end()) return;
        if (p->Num_Refs()<=0 || p->Num_Refs()>std::numeric_limits<int>::max()-32 ||
            (p->Type()!=BUFFER_TYPE_DX8 && p->Type()!=BUFFER_TYPE_SORTING))
            throw std::runtime_error("original source index checkpoint provider rejected");
        const std::size_t size=std::size_t(p->Get_Index_Count())*sizeof(unsigned short);
        if (size>64U*1024U*1024U-buffer_bytes) throw std::runtime_error("original source buffer checkpoint byte bound rejected");
        const auto *bytes=p->Type()==BUFFER_TYPE_DX8?static_cast<DX8IndexBufferClass*>(p)->Get_CPU_Index_Buffer():
            static_cast<SortingIndexBufferClass*>(p)->index_buffer;
        if (!bytes || !size) throw std::runtime_error("original source index checkpoint bytes rejected");
        c->indices.push_back({p,std::vector<unsigned short>(bytes,bytes+p->Get_Index_Count())});buffer_bytes+=size;
    };
    const auto list=[&](GenericMultiListClass& list) {
        if (std::find_if(c->lists.begin(),c->lists.end(),[&](const auto& p){return p.identity==&list;})!=c->lists.end()) return;
        c->lists.push_back({&list,list.Head.Next,list.Head.Prev});
        for (auto *node=list.Head.Next;node!=&list.Head;node=node->Next) {
            if (!node || !node->Object || node->List!=&list || c->nodes.size()==source_node_bound)
                throw std::runtime_error("original source mesh list provider/bound rejected");
            c->nodes.push_back({node,node->Next,node->Prev,node->NextList,node->Object,node->List});
            if (std::find_if(c->objects.begin(),c->objects.end(),[&](const auto& o){return o.identity==node->Object;})==c->objects.end())
                c->objects.push_back({node->Object,node->Object->Get_List_Node()});
        }
    };
    std::vector<RenderObjClass*> meshes;
    const auto material_tasks=[&](MatPassTaskClass *head) {
        for (auto *task=head;task;task=task->Get_Next_Visible()) {
            if (c->material_tasks.size()==bound) throw std::runtime_error("original source material baseline bound rejected");
            c->material_tasks.push_back({task,task->Get_Next_Visible()});meshes.push_back(task->Peek_Mesh());
        }
    };
    const auto container_list=[&](FVFCategoryList *fvfs) {
        if (!fvfs) return;list(*fvfs);
        FVFCategoryListIterator iterator(fvfs);
        for (;!iterator.Is_Done();iterator.Next()) {
            auto *p=iterator.Peek_Obj();
            auto *rigid=dynamic_cast<DX8RigidFVFCategoryContainer*>(p);
            auto *skin=dynamic_cast<DX8SkinFVFCategoryContainer*>(p);
            if ((!rigid && !skin) || c->containers.size()==4096) throw std::runtime_error("original source container provider rejected");
            index_buffer(p->index_buffer);if (rigid) vertex_buffer(rigid->vertex_buffer);
            c->containers.push_back({p,p->visible_matpass_head,p->visible_matpass_tail,p->AnythingToRender,p->AnyDelayedPassesToRender,
                rigid,rigid?rigid->delayed_matpass_head:nullptr,rigid?rigid->delayed_matpass_tail:nullptr,
                skin,skin?skin->VisibleSkinHead:nullptr,skin?skin->VisibleSkinTail:nullptr,skin?skin->VisibleVertexCount:0,
                p->used_indices,rigid?rigid->used_vertices:0});
            material_tasks(p->visible_matpass_head);
            if (rigid) material_tasks(rigid->delayed_matpass_head);
            for (unsigned pass=0;pass<p->MAX_PASSES;++pass) {
                list(p->texture_category_list[pass]);list(p->visible_texture_category_list[pass]);
                TextureCategoryListIterator categories(&p->texture_category_list[pass]);
                for (;!categories.Is_Done();categories.Next()) {
                    auto *category=categories.Peek_Obj();list(category->PolygonRendererList);
                    if (skin) {
                        DX8PolygonRendererListIterator polys(&category->PolygonRendererList);
                        for (;!polys.Is_Done();polys.Next()) {
                            const int vertices=polys.Peek_Obj()->Get_Mesh_Model_Class()->Get_Vertex_Count();
                            if (vertices<1 || vertices>65535) throw std::runtime_error("original source skin preparation range rejected");
                            skin_vertices=std::max(skin_vertices,static_cast<unsigned>(vertices));
                        }
                    }
                    c->categories.push_back({category,category->render_task_head});
                    for (auto *task=category->render_task_head;task;task=task->Get_Next_Visible()) {
                        if (c->poly_tasks.size()==bound) throw std::runtime_error("original source polygon baseline bound rejected");
                        c->poly_tasks.push_back({task,task->Get_Next_Visible()});meshes.push_back(task->Peek_Mesh());
                    }
                }
            }
        }
    };
    list(_RegisteredMeshList);list(texture_category_delete_list);list(fvf_category_container_delete_list);
    for (int i=0;i<texture_category_container_lists_rigid.Count();++i) container_list(texture_category_container_lists_rigid[i]);
    container_list(texture_category_container_list_skin);
    if (sorts) {
        if (sorts->MinSort<1 || sorts->MaxSort>MAX_SORT_LEVEL || sorts->MinSort>sorts->MaxSort)
            throw std::runtime_error("original source static sort range rejected");
        c->sorts=sorts;c->min_sort=sorts->MinSort;c->max_sort=sorts->MaxSort;
        for (auto& sorted:sorts->SortLists) {
            list(sorted);
            GenericMultiListIterator iterator(&sorted);
            // Iterate exact reference-list identities without a transferring
            // Get/Remove call. The graph pin is separate from list ownership.
            for (auto *node=sorted.Head.Next;node!=&sorted.Head;node=node->Next)
                meshes.push_back(static_cast<RenderObjClass*>(node->Object));
        }
    }
    c->graph=ww3d_prop::FrameGraph::capture(meshes);
    c->detached.reserve(source_node_bound);
    c->objects.reserve(source_node_bound);
    c->candidate_render_refs.reserve(bound);c->retired_baseline_render_refs.reserve(source_node_bound);
    c->retired_poly.reserve(bound+c->poly_tasks.size());c->retired_material.reserve(bound+c->material_tasks.size());
    c->retired_categories.reserve(4096);c->retired_containers.reserve(4096);
    c->poly_slots.reserve(bound);c->material_slots.reserve(bound);c->node_slots.reserve(bound);
    for (unsigned i=0;i<bound;++i) {
        std::unique_ptr<PolyRenderTaskClass> poly(new PolyRenderTaskClass);
        c->poly_slots.push_back(poly.get());poly.release();
        std::unique_ptr<MatPassTaskClass> material(new MatPassTaskClass);
        c->material_slots.push_back(material.get());material.release();
        std::unique_ptr<MultiListNodeClass> node(new MultiListNodeClass);
        c->node_slots.push_back(node.get());node.release();
    }
    const auto scratch=[&](DynamicVectorClass<Vector3>& buffer,SourceFrameCheckpoint::Scratch& saved) {
        if (!buffer.IsValid || buffer.VectorMax<0 || buffer.VectorMax>65535 || buffer.ActiveCount<0 || buffer.ActiveCount>buffer.VectorMax ||
            (buffer.VectorMax && (!buffer.Vector || !buffer.IsAllocated)))
            throw std::runtime_error("original source skin scratch ownership rejected");
        saved.original=buffer.Vector;saved.capacity=buffer.VectorMax;saved.count=buffer.ActiveCount;saved.growth=buffer.GrowthStep;
        saved.valid=buffer.IsValid;saved.allocated=buffer.IsAllocated;
        const std::size_t size=std::size_t(std::max(static_cast<unsigned>(buffer.VectorMax),skin_vertices))*sizeof(Vector3);
        if (size>64U*1024U*1024U-buffer_bytes) throw std::runtime_error("original source scratch byte bound rejected");
        buffer_bytes+=size;
        if (buffer.VectorMax) {
            const auto *begin=reinterpret_cast<const unsigned char*>(buffer.Vector);
            saved.bytes.assign(begin,begin+sizeof(Vector3)*buffer.VectorMax);
        }
        if (skin_vertices>static_cast<unsigned>(buffer.VectorMax)) {
            saved.candidate.reset(new Vector3[skin_vertices]);
            std::memset(saved.candidate.get(),0,skin_vertices*sizeof(Vector3));
            if (!saved.bytes.empty()) std::memcpy(saved.candidate.get(),saved.bytes.data(),saved.bytes.size());
        }
    };
    scratch(_TempVertexBuffer,c->skin_vertices);scratch(_TempNormalBuffer,c->skin_normals);
    // Complete admission precedes publication of the optional frame owner.
    for (const auto& vertex:c->vertices) vertex.identity->Add_Ref();
    for (const auto& index:c->indices) index.identity->Add_Ref();
    c->buffers_pinned=true;
    const auto publish_scratch=[&](DynamicVectorClass<Vector3>& buffer,SourceFrameCheckpoint::Scratch& saved) noexcept {
        if (saved.candidate) { buffer.Vector=saved.candidate.get();buffer.VectorMax=skin_vertices;buffer.IsAllocated=true;buffer.IsValid=true; }
    };
    publish_scratch(_TempVertexBuffer,c->skin_vertices);publish_scratch(_TempNormalBuffer,c->skin_normals);
    source_mesh_frame=c.get();return c;
}
bool DX8MeshRendererClass::Add_Source_Frame_List(GenericMultiListClass& list,MultiListObjectClass *object,bool tail,bool only_once)
{
    if (!object) throw std::runtime_error("original source list object rejected");
    if (!source_mesh_frame) return tail?list.Internal_Add_Tail(object,only_once):list.Internal_Add(object,only_once);
    auto& c=*source_mesh_frame;
    if (std::find_if(c.lists.begin(),c.lists.end(),[&](const auto& p){return p.identity==&list;})==c.lists.end())
        throw std::runtime_error("original source list insertion was not admitted");
    if (only_once && list.Contains(object)) return false;
    const bool known=std::find_if(c.objects.begin(),c.objects.end(),[&](const auto& p){return p.identity==object;})!=c.objects.end();
    if (c.node_used==c.node_slots.size() || (!known && c.objects.size()==c.objects.capacity()))
        throw std::runtime_error("original source list insertion capacity rejected");
    if (!known) c.objects.push_back({object,object->Get_List_Node()});
    auto *node=c.node_slots[c.node_used++];node->Object=object;node->List=&list;
    node->NextList=object->Get_List_Node();object->Set_List_Node(node);
    node->Prev=tail?list.Head.Prev:&list.Head;node->Next=tail?&list.Head:list.Head.Next;
    node->Next->Prev=node;node->Prev->Next=node;
    return true;
}
void DX8MeshRendererClass::Add_Source_Frame_Render_List(GenericMultiListClass& list,RenderObjClass *object)
{
    if (source_mesh_frame && source_mesh_frame->candidate_render_refs.size()==source_mesh_frame->candidate_render_refs.capacity())
        throw std::runtime_error("original source static sort reference capacity rejected");
    Add_Source_Frame_List(list,object,true,false);
    object->Add_Ref();
    if (source_mesh_frame) source_mesh_frame->candidate_render_refs.push_back(object);
}
RenderObjClass* DX8MeshRendererClass::Remove_Source_Frame_Render_Head(GenericMultiListClass& list)
{
    auto *node=list.Head.Next;
    if (node==&list.Head) return nullptr;
    const bool baseline=source_mesh_frame && source_baseline_node(*source_mesh_frame,node);
    if (baseline && source_mesh_frame->retired_baseline_render_refs.size()==source_mesh_frame->retired_baseline_render_refs.capacity())
        throw std::runtime_error("original source static sort retirement capacity rejected");
    auto *object=static_cast<RenderObjClass*>(Remove_Source_Frame_Head(list));
    if (baseline) source_mesh_frame->retired_baseline_render_refs.push_back(object);
    return object;
}
void DX8MeshRendererClass::Release_Source_Frame_Render_Ref(RenderObjClass *object) noexcept
{
    if (!source_mesh_frame) object->Release_Ref();
}
void DX8MeshRendererClass::Publish_Prepared_Polygon(MeshModelClass *model,DX8PolygonRendererClass *polygon)
{
    if (source_prepared_polygons) {
        if (model!=source_prepared_model) throw std::runtime_error("original prepared polygon owner rejected");
        ww3d_clone::Attempt::fault();
        source_prepared_polygons->Add_Tail(polygon);
    } else model->PolygonRendererList.Add_Tail(polygon);
}
void DX8MeshRendererClass::Withdraw_Prepared_Category(DX8TextureCategoryClass *category) noexcept
{
    while (auto *polygon=category->PolygonRendererList.Remove_Head()) {
        polygon->Set_Texture_Category(nullptr);delete polygon;
    }
}
void DX8MeshRendererClass::Destroy_Prepared_Container(DX8FVFCategoryContainer *container) noexcept
{
    for (auto& list:container->texture_category_list)
        while (auto *category=list.Remove_Head()) { Withdraw_Prepared_Category(category);delete category; }
    delete container;
}
void DX8FVFCategoryContainer::Add_Visible_Texture_Category(DX8TextureCategoryClass *category,int pass)
{
    if (pass<0 || pass>=MAX_PASSES || !category || !texture_category_list[pass].Contains(category))
        throw std::runtime_error("original source visible category rejected");
    DX8MeshRendererClass::Add_Source_Frame_List(visible_texture_category_list[pass],category);
    AnythingToRender=true;
}
MultiListObjectClass* DX8MeshRendererClass::Remove_Source_Frame_Head(GenericMultiListClass& list)
{
    auto *node=list.Head.Next;
    if (node==&list.Head) return nullptr;
    if (!source_mesh_frame) { auto *object=node->Object;list.Internal_Remove(object);return object; }
    auto& c=*source_mesh_frame;
    if (c.detached.size()==c.detached.capacity()
        || std::find_if(c.lists.begin(),c.lists.end(),[&](const auto& p){return p.identity==&list;})==c.lists.end())
        throw std::runtime_error("original source list withdrawal was not admitted");
    c.detached.push_back(node);source_unlink(node);return node->Object;
}
void DX8MeshRendererClass::Restore_Source_Frame(SourceFrameCheckpoint& c) noexcept
{
    if (source_mesh_frame!=&c || c.closed) std::terminate();
    source_mesh_frame=nullptr;
    for (const auto& list:c.lists) {
        while (list.identity->Head.Next!=&list.identity->Head) {
            auto *node=list.identity->Head.Next;source_unlink(node);
            if (!source_baseline_node(c,node) && !source_candidate_node(c,node)) delete node;
        }
    }
    for (auto *node:c.detached) if (!source_baseline_node(c,node) && !source_candidate_node(c,node)) delete node;
    for (const auto& n:c.nodes) {
        n.identity->Next=n.next;n.identity->Prev=n.previous;n.identity->NextList=n.next_list;
        n.identity->Object=n.object;n.identity->List=n.list;
    }
    for (const auto& list:c.lists) { list.identity->Head.Next=list.next;list.identity->Head.Prev=list.previous; }
    for (const auto& object:c.objects) object.identity->Set_List_Node(object.head);
    for (const auto& task:c.poly_tasks) task.identity->Set_Next_Visible(task.next);
    for (const auto& task:c.material_tasks) task.identity->Set_Next_Visible(task.next);
    for (const auto& category:c.categories) category.identity->render_task_head=category.head;
    for (const auto& p:c.containers) {
        p.identity->visible_matpass_head=p.head;p.identity->visible_matpass_tail=p.tail;
        p.identity->AnythingToRender=p.anything;p.identity->AnyDelayedPassesToRender=p.delayed;
        if (p.rigid) { p.rigid->delayed_matpass_head=p.delayed_head;p.rigid->delayed_matpass_tail=p.delayed_tail; }
        if (p.skin) { p.skin->VisibleSkinHead=p.skin_head;p.skin->VisibleSkinTail=p.skin_tail;p.skin->VisibleVertexCount=p.skin_count; }
        p.identity->used_indices=p.used_indices;if (p.rigid) p.rigid->used_vertices=p.used_vertices;
    }
    for (const auto& vertex:c.vertices) {
        auto *target=vertex.identity->Type()==BUFFER_TYPE_DX8?static_cast<DX8VertexBufferClass*>(vertex.identity)->Get_CPU_Vertex_Buffer():
            reinterpret_cast<unsigned char*>(static_cast<SortingVertexBufferClass*>(vertex.identity)->VertexBuffer);
        std::memcpy(target,vertex.bytes.data(),vertex.bytes.size());
    }
    for (const auto& index:c.indices) {
        auto *target=index.identity->Type()==BUFFER_TYPE_DX8?static_cast<DX8IndexBufferClass*>(index.identity)->Get_CPU_Index_Buffer():
            static_cast<SortingIndexBufferClass*>(index.identity)->index_buffer;
        std::memcpy(target,index.bytes.data(),index.bytes.size()*sizeof(unsigned short));
    }
    c.graph->restore();c.closed=true;
    const auto restore_scratch=[](DynamicVectorClass<Vector3>& buffer,SourceFrameCheckpoint::Scratch& saved) noexcept {
        buffer.Vector=saved.original;buffer.VectorMax=saved.capacity;buffer.ActiveCount=saved.count;buffer.GrowthStep=saved.growth;
        buffer.IsAllocated=saved.allocated;buffer.IsValid=saved.valid;
        if (!saved.bytes.empty()) std::memcpy(buffer.Vector,saved.bytes.data(),saved.bytes.size());
    };
    restore_scratch(_TempVertexBuffer,c.skin_vertices);restore_scratch(_TempNormalBuffer,c.skin_normals);
    if (c.sorts) { c.sorts->MinSort=c.min_sort;c.sorts->MaxSort=c.max_sort; }
}
bool DX8MeshRendererClass::Source_Frame_Ready_To_Commit(const SourceFrameCheckpoint& c) noexcept
{
    if (source_mesh_frame!=&c || c.closed || !c.owner->Source_Frame_Queues_Empty()) return false;
    if (c.sorts && !c.sorts->Source_Frame_Empty()) return false;
    for (auto *node:c.node_slots) if (node->List) return false;
    // Candidate tasks must have been consumed rather than silently discarded.
    for (unsigned i=0;i<c.poly_used;++i)
        if (std::find(c.retired_poly.begin(),c.retired_poly.end(),c.poly_slots[i])==c.retired_poly.end()) return false;
    for (unsigned i=0;i<c.material_used;++i)
        if (std::find(c.retired_material.begin(),c.retired_material.end(),c.material_slots[i])==c.retired_material.end()) return false;
    return true;
}
void DX8MeshRendererClass::Commit_Source_Frame(SourceFrameCheckpoint& c) noexcept
{
    if (!Source_Frame_Ready_To_Commit(c)) std::terminate();
    source_mesh_frame=nullptr;
    const auto commit_scratch=[](DynamicVectorClass<Vector3>& buffer,SourceFrameCheckpoint::Scratch& saved) noexcept {
        if (!saved.candidate) return;
        if (saved.required>static_cast<unsigned>(saved.capacity)) {
            buffer.Vector=saved.candidate.release();buffer.VectorMax=saved.required;
            if (saved.allocated) delete[] saved.original;
        } else {
            if (!saved.bytes.empty()) std::memcpy(saved.original,saved.candidate.get(),saved.bytes.size());
            buffer.Vector=saved.original;buffer.VectorMax=saved.capacity;buffer.IsAllocated=saved.allocated;buffer.IsValid=saved.valid;
        }
    };
    commit_scratch(_TempVertexBuffer,c.skin_vertices);commit_scratch(_TempNormalBuffer,c.skin_normals);
    for (auto *node:c.detached) if (!source_candidate_node(c,node)) delete node;
    const auto candidate_poly=[&](PolyRenderTaskClass *task) { return std::find(c.poly_slots.begin(),c.poly_slots.end(),task)!=c.poly_slots.end(); };
    const auto candidate_material=[&](MatPassTaskClass *task) { return std::find(c.material_slots.begin(),c.material_slots.end(),task)!=c.material_slots.end(); };
    for (auto *task:c.retired_poly) if (!candidate_poly(task)) delete task;
    for (auto *task:c.retired_material) if (!candidate_material(task)) delete task;
    for (auto *category:c.retired_categories) delete category;
    for (auto *container:c.retired_containers) delete container;
    for (auto *object:c.retired_baseline_render_refs) object->Release_Ref();
    c.closed=true;
}
#else
static PolyRenderTaskClass *source_poly_task(DX8PolygonRendererClass *renderer,MeshClass *mesh)
{ return new PolyRenderTaskClass(renderer,mesh); }
static MatPassTaskClass *source_material_task(MaterialPassClass *pass,MeshClass *mesh)
{ return new MatPassTaskClass(pass,mesh); }
static void source_retire(PolyRenderTaskClass *task) { delete task; }
static void source_retire(MatPassTaskClass *task) { delete task; }
#endif


// ----------------------------------------------------------------------------


inline static bool Equal_Material(const VertexMaterialClass* mat1,const VertexMaterialClass* mat2)
{
	int crc0 = mat1 ? mat1->Get_CRC() : 0;
	int crc1 = mat2 ? mat2->Get_CRC() : 0;
	return (crc0 == crc1);
}


DX8TextureCategoryClass::DX8TextureCategoryClass(
	DX8FVFCategoryContainer* container_,
	TextureClass** texs,
	ShaderClass shd, 
	VertexMaterialClass* mat,
	int pass_)
	:
	pass(pass_),
	shader(shd),
	render_task_head(NULL),
	material(mat),
	container(container_)
{
	WWASSERT(pass>=0);
	WWASSERT(pass<DX8FVFCategoryContainer::MAX_PASSES);

	for (int a=0;a<MeshMatDescClass::MAX_TEX_STAGES;++a) 
	{
		textures[a]=NULL;
		REF_PTR_SET(textures[a],texs[a]);
	}

	if (material) material->Add_Ref();
}

DX8TextureCategoryClass::~DX8TextureCategoryClass()
{
	#if defined(ZH_WW3D_CPU_ONLY)
	// A failed physical draw leaves the current task linked. Invalidation must
	// retire its mesh ref before unregistering the polygon renderer/model.
	while (render_task_head) {
		PolyRenderTaskClass* task=render_task_head;
		render_task_head=task->Get_Next_Visible();
		delete task;
	}
	#endif
	// Unregistering the mesh where polygon renderers are connected to kills all polygon renderers
	while (DX8PolygonRendererClass* p_renderer=PolygonRendererList.Get_Head()) {
		TheDX8MeshRenderer.Unregister_Mesh_Type(p_renderer->Get_Mesh_Model_Class());
	}
	for (int a=0;a<MeshMatDescClass::MAX_TEX_STAGES;++a) 
	{
		REF_PTR_RELEASE(textures[a]);
	}

	REF_PTR_RELEASE(material);
}

void DX8TextureCategoryClass::Add_Render_Task(DX8PolygonRendererClass * p_renderer,MeshClass * p_mesh)
{
	PolyRenderTaskClass * new_prt = source_poly_task(p_renderer,p_mesh);
	new_prt->Set_Next_Visible(render_task_head);
	render_task_head = new_prt;

	container->Add_Visible_Texture_Category(this,pass);
}

void DX8TextureCategoryClass::Add_Polygon_Renderer(DX8PolygonRendererClass* p_renderer,DX8PolygonRendererClass* add_after_this)
{
	WWASSERT(p_renderer!=NULL);
	WWASSERT(!PolygonRendererList.Contains(p_renderer));

	if (add_after_this != NULL) {
		bool res = PolygonRendererList.Add_After(p_renderer,add_after_this,false);
		WWASSERT(res != NULL);
	} else {
		PolygonRendererList.Add(p_renderer);
	}

	p_renderer->Set_Texture_Category(this);
}

void DX8TextureCategoryClass::Remove_Polygon_Renderer(DX8PolygonRendererClass* p_renderer)
{
	PolygonRendererList.Remove(p_renderer);
	p_renderer->Set_Texture_Category(NULL);
	if (PolygonRendererList.Peek_Head() == NULL) {
		container->Remove_Texture_Category(this);
		texture_category_delete_list.Add_Tail(this);
	}
}


void DX8FVFCategoryContainer::Remove_Texture_Category(DX8TextureCategoryClass* tex_category)
{
	unsigned pass;
	for (pass=0;pass<passes;++pass) {
		texture_category_list[pass].Remove(tex_category);
	}
	for (pass=0; pass<passes; pass++) {
		// If any of the texture category lists has anything in it, no need to delete this container
		if (texture_category_list[pass].Peek_Head() != NULL) return;
	}
	fvf_category_container_delete_list.Add_Tail(this);
}

void DX8FVFCategoryContainer::Add_Visible_Material_Pass(MaterialPassClass * pass,MeshClass * mesh)
{
	MatPassTaskClass * new_mpr = source_material_task(pass,mesh);

	if (visible_matpass_head == NULL) {
		WWASSERT(visible_matpass_tail == NULL);
		visible_matpass_head = new_mpr;
	} else {
		WWASSERT(visible_matpass_tail != NULL);
		visible_matpass_tail->Set_Next_Visible(new_mpr);
	}

	visible_matpass_tail = new_mpr;
	AnythingToRender=true;
}

void DX8FVFCategoryContainer::Render_Procedural_Material_Passes(void)
{
	// additional passes
	MatPassTaskClass * mpr = visible_matpass_head;
	MatPassTaskClass * last_mpr = NULL;
   	bool renderTasksRemaining=false;

	while (mpr != NULL) {
		SNAPSHOT_SAY(("Render_Procedural_Material_Pass\n"));

   		MeshClass * mesh = mpr->Peek_Mesh();
   	
   		if (mesh->Get_Base_Vertex_Offset() == VERTEX_BUFFER_OVERFLOW)	//check if this mesh is valid
   		{	//skip this mesh so it gets rendered later after vertices are filled in.
	        last_mpr = mpr;
   			mpr = mpr->Get_Next_Visible();
   			renderTasksRemaining = true;
   			continue;
   		}
	
		mpr->Peek_Mesh()->Render_Material_Pass(mpr->Peek_Material_Pass(),index_buffer);
		MatPassTaskClass * next_mpr = mpr->Get_Next_Visible();
		
		// remove from list, then delete
		if (last_mpr == NULL) {
			visible_matpass_head = next_mpr;
		} else {
	       last_mpr->Set_Next_Visible(next_mpr);
	    }

		source_retire(mpr);
		mpr = next_mpr;
	}

	visible_matpass_tail = renderTasksRemaining ? last_mpr : NULL;
}

void DX8RigidFVFCategoryContainer::Add_Delayed_Visible_Material_Pass(MaterialPassClass * pass, MeshClass * mesh)
{
	MatPassTaskClass * new_mpr = source_material_task(pass,mesh);

	if (delayed_matpass_head == NULL) {
		WWASSERT(delayed_matpass_tail == NULL);
		delayed_matpass_head = new_mpr;
	} else {
		WWASSERT(delayed_matpass_tail != NULL);
		delayed_matpass_tail->Set_Next_Visible(new_mpr);
	}

	delayed_matpass_tail = new_mpr;
	AnyDelayedPassesToRender=true;
}

void DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes(void)
{
#if defined(ZH_WW3D_CPU_ONLY)
	if (!Any_Delayed_Passes_To_Render()) return;
	DX8Wrapper::Set_Vertex_Buffer(vertex_buffer);
	DX8Wrapper::Set_Index_Buffer(index_buffer,0);
	SNAPSHOT_SAY(("DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes()\n"));
	// The source task remains owned by the original container until its draw
	// succeeds. Linux device errors can unwind; keep the failed task linked
	// so the enclosing scene can abort/reset without a dangling list head.
	while (delayed_matpass_head != NULL) {
		MatPassTaskClass * mpr=delayed_matpass_head;
		mpr->Peek_Mesh()->Render_Material_Pass(mpr->Peek_Material_Pass(),index_buffer);
		delayed_matpass_head=mpr->Get_Next_Visible();
		if (delayed_matpass_head==NULL) delayed_matpass_tail=NULL;
		source_retire(mpr);
	}
	AnyDelayedPassesToRender=false;
#else
	if (!Any_Delayed_Passes_To_Render()) return;
	AnyDelayedPassesToRender=false;

	DX8Wrapper::Set_Vertex_Buffer(vertex_buffer);
	DX8Wrapper::Set_Index_Buffer(index_buffer,0);

	SNAPSHOT_SAY(("DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes()\n"));

	// additional passes
	MatPassTaskClass * mpr = delayed_matpass_head;
	while (mpr != NULL) {
	
		mpr->Peek_Mesh()->Render_Material_Pass(mpr->Peek_Material_Pass(),index_buffer);
		MatPassTaskClass * next_mpr = mpr->Get_Next_Visible();
		
		delete mpr;
		mpr = next_mpr;
	}

	delayed_matpass_head = delayed_matpass_tail = NULL;
#endif
}


void DX8TextureCategoryClass::Log(bool only_visible)
{
#ifdef ENABLE_CATEGORY_LOG
	StringClass work(255,true);
	work.Format("	DX8TextureCategoryClass\n");
	WWDEBUG_SAY((work));

	StringClass work2(255,true);
	for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
		work2.Format("	texture[%d]: %x (%s)\n", stage, textures[stage], textures[stage] ? textures[stage]->Get_Name() : "-");
		work+=work2;
	}
	work2.Format("	material: %x (%s)\n	shader: %x\n", material, material ? material->Get_Name() : "-", shader);
	work+=work2;
	WWDEBUG_SAY((work));

	work.Format("	%8s %8s %6s %6s %6s %5s %s\n",
		"idx_cnt",
		"poly_cnt",
		"i_offs",
		"min_vi",
		"vi_rng",
		"ident",
		"name");
	WWDEBUG_SAY((work));

	DX8PolygonRendererListIterator it(&PolygonRendererList);
	while (!it.Is_Done()) {
	
		DX8PolygonRendererClass* p_renderer = it.Peek_Obj();

		PolyRenderTaskClass * prtc=render_task_head;
		while (prtc) {
			if (prtc->Peek_Polygon_Renderer()==p_renderer) break;
			prtc = prtc->Get_Next_Visible();
		}

		if (prtc != NULL) {
			WWDEBUG_SAY(("+"));
			p_renderer->Log();
		} else {
			if (!only_visible) {
				WWDEBUG_SAY(("-"));
				p_renderer->Log();
			}
		}
		it.Next();
	}
#endif
}

// ----------------------------------------------------------------------------

DX8FVFCategoryContainer::DX8FVFCategoryContainer(unsigned FVF_,bool sorting_)
	:
	FVF(FVF_),
	sorting(sorting_),
	visible_matpass_head(NULL),
	visible_matpass_tail(NULL),
	index_buffer(0),
	used_indices(0),
	passes(MAX_PASSES),
	uv_coordinate_channels(0),
	AnythingToRender(false),
	AnyDelayedPassesToRender(false)
{
	if ((FVF&fvf_tex1)==fvf_tex1) uv_coordinate_channels=1;
	if ((FVF&fvf_tex2)==fvf_tex2) uv_coordinate_channels=2;
	if ((FVF&fvf_tex3)==fvf_tex3) uv_coordinate_channels=3;
	if ((FVF&fvf_tex4)==fvf_tex4) uv_coordinate_channels=4;
	if ((FVF&fvf_tex5)==fvf_tex5) uv_coordinate_channels=5;
	if ((FVF&fvf_tex6)==fvf_tex6) uv_coordinate_channels=6;
	if ((FVF&fvf_tex7)==fvf_tex7) uv_coordinate_channels=7;
	if ((FVF&fvf_tex8)==fvf_tex8) uv_coordinate_channels=8;
}

// ----------------------------------------------------------------------------

DX8FVFCategoryContainer::~DX8FVFCategoryContainer()
{
#if defined(ZH_WW3D_CPU_ONLY)
	// An aborted source frame invalidates the container without reaching the
	// procedural-pass flush. Each abandoned task still owns its mesh and pass.
	while (visible_matpass_head != NULL) {
		MatPassTaskClass * task=visible_matpass_head;
		visible_matpass_head=task->Get_Next_Visible();
		delete task;
	}
	visible_matpass_tail=NULL;
#endif
	REF_PTR_RELEASE(index_buffer);

	for (unsigned p=0;p<passes;++p) {
		while (DX8TextureCategoryClass * tex = texture_category_list[p].Remove_Head()) {
			delete tex;
		}
	}
}

// ----------------------------------------------------------------------------

DX8TextureCategoryClass* DX8FVFCategoryContainer::Find_Matching_Texture_Category(
	TextureClass* texture,
	unsigned pass,
	unsigned stage,
	DX8TextureCategoryClass* ref_category)
{
	// Find texture category which matches ref_category's properties but has 'texture' on given pass and stage.
	DX8TextureCategoryClass* dest_tex_category=NULL;
	TextureCategoryListIterator dest_it(&texture_category_list[pass]);
	while (!dest_it.Is_Done()) {
		if (dest_it.Peek_Obj()->Peek_Texture(stage)==texture) {
			// Compare all stage's textures
			dest_tex_category=dest_it.Peek_Obj();
			bool all_textures_same = true;
			for (unsigned int s = 0; s < MeshMatDescClass::MAX_TEX_STAGES; s++) {
				if (stage!=s) {
					all_textures_same = all_textures_same && (dest_tex_category->Peek_Texture(s) == ref_category->Peek_Texture(s));
				}
			}
			if (all_textures_same &&
				Equal_Material(dest_tex_category->Peek_Material(),ref_category->Peek_Material()) &&
				dest_tex_category->Get_Shader()==ref_category->Get_Shader()) {
				return dest_tex_category;
			}
		}
		dest_it.Next();
	}
	return NULL;
}

DX8TextureCategoryClass* DX8FVFCategoryContainer::Find_Matching_Texture_Category(
		VertexMaterialClass* vmat,
		unsigned pass,		
		DX8TextureCategoryClass* ref_category)
{
	// Find texture category which matches ref_category's properties but has 'vmat' on given pass
	DX8TextureCategoryClass* dest_tex_category=NULL;
	TextureCategoryListIterator dest_it(&texture_category_list[pass]);
	while (!dest_it.Is_Done()) {
		if (Equal_Material(dest_it.Peek_Obj()->Peek_Material(),vmat)) {
			// Compare all stage's textures
			dest_tex_category=dest_it.Peek_Obj();
			bool all_textures_same = true;
			for (unsigned int s = 0; s < MeshMatDescClass::MAX_TEX_STAGES; s++)
				all_textures_same = all_textures_same && (dest_tex_category->Peek_Texture(s) == ref_category->Peek_Texture(s));			
			if (all_textures_same &&				
				dest_tex_category->Get_Shader()==ref_category->Get_Shader()) {
				return dest_tex_category;
			}
		}
		dest_it.Next();
	}
	return NULL;
}

void DX8FVFCategoryContainer::Change_Polygon_Renderer_Texture(
	DX8PolygonRendererList& polygon_renderer_list,
	TextureClass* texture,
	TextureClass* new_texture,
	unsigned pass,
	unsigned stage)
{
	WWASSERT(pass<passes);

	PolyRemoverList prl;

	bool foundtexture=false;

	if (texture==new_texture) return;

	// Find source texture category, then find all polygon renderers who belong to that category
	// and move them to destination category.
	TextureCategoryListIterator src_it(&texture_category_list[pass]);
	while (!src_it.Is_Done()) {
		DX8TextureCategoryClass* src_tex_category=src_it.Peek_Obj();		
		if (src_tex_category->Peek_Texture(stage)==texture) {
			foundtexture=true;
			DX8PolygonRendererListIterator poly_it(&polygon_renderer_list);
			while (!poly_it.Is_Done()) {
				// If source texture category contains polygon renderer, move to destination category
				DX8PolygonRendererClass* polygon_renderer=poly_it.Peek_Obj();
				DX8TextureCategoryClass *prc=polygon_renderer->Get_Texture_Category();

				if (prc==src_tex_category) {					
					DX8TextureCategoryClass* dest_tex_category=Find_Matching_Texture_Category(new_texture,pass,stage,src_tex_category);

					if (!dest_tex_category) {
						TextureClass * tmp_textures[MeshMatDescClass::MAX_TEX_STAGES];
						for (int s=0;s<MeshMatDescClass::MAX_TEX_STAGES;++s) {
							tmp_textures[s]=src_tex_category->Peek_Texture(s);
						}
						tmp_textures[stage]=new_texture;

						DX8TextureCategoryClass * new_tex_category=W3DNEW DX8TextureCategoryClass(
							this,
							tmp_textures,
							src_tex_category->Get_Shader(),
							const_cast<VertexMaterialClass*>(src_tex_category->Peek_Material()),
							pass);
		
						/*
						** Add the texture category object into the list, immediately after any existing
						** texture category object which uses the same texture.  This will result in
						** the list always having matching texture categories next to each other.
						*/
						bool found_similar_category = false;
						TextureCategoryListIterator tex_it(&texture_category_list[pass]);
						while (!tex_it.Is_Done()) {
							// Categorize according to first stage's texture for now
							if (tex_it.Peek_Obj()->Peek_Texture(0) == tmp_textures[0]) {
								texture_category_list[pass].Add_After(new_tex_category,tex_it.Peek_Obj());
								found_similar_category = true;
								break;
							}
							tex_it.Next();
						}

						if (!found_similar_category) {
							texture_category_list[pass].Add_Tail(new_tex_category);
						}
						dest_tex_category=new_tex_category;
					}
					PolyRemover *rem=W3DNEW PolyRemover;
					rem->src=src_tex_category;
					rem->dest=dest_tex_category;
					rem->pr=polygon_renderer;
					prl.Add(rem);					
				}
				poly_it.Next();
			} // while			
		} //if src_texture==texture
		else
			// quit loop if we've got a texture change
			if (foundtexture) break;
		src_it.Next();
	} // while

	PolyRemoverListIterator prli(&prl);

	while (!prli.Is_Done())
	{
		PolyRemover *rem=prli.Peek_Obj();
		rem->src->Remove_Polygon_Renderer(rem->pr);
		rem->dest->Add_Polygon_Renderer(rem->pr);		
		prli.Remove_Current_Object();
		delete rem;
	}
}

void DX8FVFCategoryContainer::Change_Polygon_Renderer_Material(
		DX8PolygonRendererList& polygon_renderer_list,
		VertexMaterialClass* vmat,
		VertexMaterialClass* new_vmat,
		unsigned pass)
{
	WWASSERT(pass<passes);

	PolyRemoverList prl;

	bool foundtexture=false;

	if (vmat==new_vmat) return;

	// Find source texture category, then find all polygon renderers who belong to that category
	// and move them to destination category.
	TextureCategoryListIterator src_it(&texture_category_list[pass]);
	while (!src_it.Is_Done()) {
		DX8TextureCategoryClass* src_tex_category=src_it.Peek_Obj();
		if (src_tex_category->Peek_Material()==vmat) {			
			DX8PolygonRendererListIterator poly_it(&polygon_renderer_list);
			while (!poly_it.Is_Done()) {
				// If source texture category contains polygon renderer, move to destination category
				DX8PolygonRendererClass* polygon_renderer=poly_it.Peek_Obj();
				DX8TextureCategoryClass *prc=polygon_renderer->Get_Texture_Category();
				if (prc==src_tex_category) {
					foundtexture=true;
					DX8TextureCategoryClass* dest_tex_category=Find_Matching_Texture_Category(new_vmat,pass,src_tex_category);

					if (!dest_tex_category) {
						TextureClass * tmp_textures[MeshMatDescClass::MAX_TEX_STAGES];
						for (int s=0;s<MeshMatDescClass::MAX_TEX_STAGES;++s) {
							tmp_textures[s]=src_tex_category->Peek_Texture(s);
						}						

						DX8TextureCategoryClass * new_tex_category=W3DNEW DX8TextureCategoryClass(
							this,
							tmp_textures,
							src_tex_category->Get_Shader(),
							const_cast<VertexMaterialClass*>(new_vmat),
							pass);
		
						/*
						** Add the texture category object into the list, immediately after any existing
						** texture category object which uses the same texture.  This will result in
						** the list always having matching texture categories next to each other.
						*/
						bool found_similar_category = false;
						TextureCategoryListIterator tex_it(&texture_category_list[pass]);
						while (!tex_it.Is_Done()) {
							// Categorize according to first stage's texture for now
							if (tex_it.Peek_Obj()->Peek_Texture(0) == tmp_textures[0]) {
								texture_category_list[pass].Add_After(new_tex_category,tex_it.Peek_Obj());
								found_similar_category = true;
								break;
							}
							tex_it.Next();
						}

						if (!found_similar_category) {
							texture_category_list[pass].Add_Tail(new_tex_category);
						}
						dest_tex_category=new_tex_category;
					}
					PolyRemover *rem=W3DNEW PolyRemover;
					rem->src=src_tex_category;
					rem->dest=dest_tex_category;
					rem->pr=polygon_renderer;
					prl.Add(rem);
				}
				poly_it.Next();
			} // while			
		} // if 
		else
			if (foundtexture) break;
		src_it.Next();
	} // while

	PolyRemoverListIterator prli(&prl);

	while (!prli.Is_Done())
	{
		PolyRemover *rem=prli.Peek_Obj();
		rem->src->Remove_Polygon_Renderer(rem->pr);
		rem->dest->Add_Polygon_Renderer(rem->pr);		
		prli.Remove_Current_Object();
		delete rem;
	}
}

// ----------------------------------------------------------------------------

unsigned DX8FVFCategoryContainer::Define_FVF(MeshModelClass* mmc,bool enable_lighting)
{
	if ((!!mmc->Get_Flag(MeshGeometryClass::SORT)) && WW3D::Is_Sorting_Enabled()) {
		return dynamic_fvf_type;
	}

	unsigned fvf=fvf_xyz;

	int tex_coord_count=mmc->Get_UV_Array_Count();
#if defined(ZH_WW3D_CPU_ONLY)
	if (tex_coord_count < 0 || tex_coord_count > 8)
		throw std::runtime_error("unsupported original mesh texture-coordinate count");
#endif

	if (mmc->Get_Color_Array(0,false)) {
		fvf|=fvf_diffuse;
	}
	if (mmc->Get_Color_Array(1,false)) {
		fvf|=fvf_specular;
	}
	
	switch (tex_coord_count) {
	default:
	case 0:
		break;
	case 1: fvf|=fvf_tex1; break;
	case 2: fvf|=fvf_tex2; break;
	case 3: fvf|=fvf_tex3; break;
	case 4: fvf|=fvf_tex4; break;
	case 5: fvf|=fvf_tex5; break;
	case 6: fvf|=fvf_tex6; break;
	case 7: fvf|=fvf_tex7; break;
	case 8: fvf|=fvf_tex8; break;
	}

	if (!mmc->Needs_Vertex_Normals()) {  //enable_lighting || mmc->Get_Flag(MeshModelClass::PRELIT_MASK)) {
		return fvf;
	}

	fvf|=fvf_normal;	// Realtime-lit
	return fvf;
}

// ----------------------------------------------------------------------------

DX8RigidFVFCategoryContainer::DX8RigidFVFCategoryContainer(unsigned FVF,bool sorting_)
	:
	DX8FVFCategoryContainer(FVF,sorting_),
	vertex_buffer(0),
	used_vertices(0),
	delayed_matpass_head(NULL),
	delayed_matpass_tail(NULL)
{
}

// ----------------------------------------------------------------------------

DX8RigidFVFCategoryContainer::~DX8RigidFVFCategoryContainer()
{
#if defined(ZH_WW3D_CPU_ONLY)
	while (delayed_matpass_head != NULL) {
		MatPassTaskClass * task=delayed_matpass_head;
		delayed_matpass_head=task->Get_Next_Visible();
		delete task;
	}
	delayed_matpass_tail=NULL;
#endif
	REF_PTR_RELEASE(vertex_buffer);
}

// ----------------------------------------------------------------------------

void DX8RigidFVFCategoryContainer::Log(bool only_visible)
{
#ifdef ENABLE_CATEGORY_LOG
	StringClass work(255,true);
	work.Format("DX8RigidFVFCategoryContainer --------------\n");
	WWDEBUG_SAY((work));
	if (vertex_buffer) {
		StringClass fvfname(255,true);
		vertex_buffer->FVF_Info().Get_FVF_Name(fvfname);
		work.Format("VB size (used/total): %d/%d FVF: %s\n",used_vertices,vertex_buffer->Get_Vertex_Count(),fvfname);
		WWDEBUG_SAY((work));
	}
	else {
		WWDEBUG_SAY(("EMPTY VB\n"));
	}
	if (index_buffer) {
		work.Format("IB size (used/total): %d/%d\n",used_indices,index_buffer->Get_Index_Count());
		WWDEBUG_SAY((work));
	}
	else {
		WWDEBUG_SAY(("EMPTY IB\n"));
	}

	for (unsigned p=0;p<passes;++p) {
		WWDEBUG_SAY(("Pass: %d\n",p));

		TextureCategoryListIterator it(&texture_category_list[p]);
		while (!it.Is_Done()) {
			it.Peek_Obj()->Log(only_visible);
			it.Next();
		}
	}
#endif
}

// ----------------------------------------------------------------------------
//
// Generic render function for rigid meshes
//
// ----------------------------------------------------------------------------

void DX8RigidFVFCategoryContainer::Render(void)
{
	if (!Anything_To_Render()) return;
	AnythingToRender=false;

	DX8Wrapper::Set_Vertex_Buffer(vertex_buffer);

	DX8Wrapper::Set_Index_Buffer(index_buffer,0);

	SNAPSHOT_SAY(("DX8RigidFVFCategoryContainer::Render()\n"));
	// The Z-biasing was causing more problems than they solved.
	// Disabling it for now HY.
	//int zbias=0;
	//DX8Wrapper::Set_DX8_ZBias(zbias);
	for (unsigned p=0;p<passes;++p) {
		SNAPSHOT_SAY(("Pass: %d\n",p));
#if defined(ZH_WW3D_CPU_ONLY)
		while (auto *tex=static_cast<DX8TextureCategoryClass*>(DX8MeshRendererClass::Remove_Source_Frame_Head(visible_texture_category_list[p]))) {
#else
		while (DX8TextureCategoryClass * tex = visible_texture_category_list[p].Remove_Head()) {
#endif
			tex->Render();
		}
		//zbias++;
		//if (zbias>15) zbias=15;
		//DX8Wrapper::Set_DX8_ZBias(zbias);
	}

	Render_Procedural_Material_Passes();

	//DX8Wrapper::Set_DX8_ZBias(0);
}

// ----------------------------------------------------------------------------

bool DX8RigidFVFCategoryContainer::Check_If_Mesh_Fits(MeshModelClass* mmc)
{
	if (!vertex_buffer) return true;	// No VB created - mesh will fit as a new vb will be created when inserting
	unsigned required_vertices=mmc->Get_Vertex_Count();
	unsigned available_vertices=vertex_buffer->Get_Vertex_Count()-used_vertices;
	unsigned required_polygons=mmc->Get_Polygon_Count();
	if (mmc->Get_Gap_Filler()) {
		required_polygons+=mmc->Get_Gap_Filler()->Get_Polygon_Count();
	} 
	unsigned required_indices=required_polygons*3*mmc->Get_Pass_Count();
	unsigned available_indices=index_buffer->Get_Index_Count()-used_indices;
	if (
		required_vertices<=available_vertices &&
		(required_indices)<=available_indices) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------

class Vertex_Split_Table
{
	MeshModelClass* mmc;
	bool npatch_enable;
	unsigned polygon_count;
	TriIndex* polygon_array;

	bool allocated_polygon_array;

public:
	Vertex_Split_Table(MeshModelClass* mmc_)
		:
		mmc(mmc_),
		npatch_enable(false),
		allocated_polygon_array(false)
	{
#if defined(ZH_WW3D_CPU_ONLY)
		if (WW3D::Get_NPatches_Level() > 1 &&
			mmc->Needs_Vertex_Normals() && mmc->Get_Flag(MeshGeometryClass::ALLOW_NPATCHES))
			throw std::runtime_error("original NPatches mesh requires unsupported tessellation");
#else
		if (DX8Wrapper::Get_Current_Caps()->Support_NPatches() && mmc->Needs_Vertex_Normals()) {
			if (mmc->Get_Flag(MeshGeometryClass::ALLOW_NPATCHES)) {
				npatch_enable=true;
			}
		}
#endif

		const GapFillerClass* gap_filler=mmc->Get_Gap_Filler();
		polygon_count=mmc->Get_Polygon_Count();
		if (gap_filler) polygon_count+=gap_filler->Get_Polygon_Count();
//		if (mmc->Get_Gap_Filler_Polygon_Count()) {
			allocated_polygon_array=true;
			polygon_array=W3DNEWARRAY TriIndex[polygon_count];
			memcpy(
				polygon_array,
				mmc->Get_Polygon_Array(),
				mmc->Get_Polygon_Count()*sizeof(TriIndex));
			if (gap_filler) {
				memcpy(
					polygon_array+mmc->Get_Polygon_Count(),
					gap_filler->Get_Polygon_Array(),
					gap_filler->Get_Polygon_Count()*sizeof(TriIndex));
			}
//		}
//		else {
//			polygon_array=const_cast<TriIndex*>(mmc->Get_Polygon_Array());
//		}

	}

	~Vertex_Split_Table()
	{
		if (allocated_polygon_array) {
			delete[] polygon_array;
		}
	}

	const Vector3* Get_Vertex_Array() const
	{
		return mmc->Get_Vertex_Array();
	}

	const Vector3* Get_Vertex_Normal_Array() const
	{
		return mmc->Get_Vertex_Normal_Array();
	}

	const unsigned* Get_Color_Array(unsigned index) const
	{
		return mmc->Get_Color_Array(index,false);
	}

	const Vector2* Get_UV_Array(unsigned uv_array_index) const
	{
		return mmc->Get_UV_Array_By_Index(uv_array_index);
	}

	unsigned Get_Vertex_Count() const
	{
		return mmc->Get_Vertex_Count();
	}

	unsigned Get_Polygon_Count() const
	{
		return polygon_count;
	}

	unsigned Get_Pass_Count() const
	{
		return mmc->Get_Pass_Count();
	}

	TextureClass* Peek_Texture(unsigned idx,unsigned pass,unsigned stage)
	{
		if (mmc->Has_Texture_Array(pass,stage)) {
			if (idx>=unsigned(mmc->Get_Polygon_Count())) {
				WWASSERT(mmc->Get_Gap_Filler());
				return mmc->Get_Gap_Filler()->Get_Texture_Array(pass,stage)[idx-mmc->Get_Polygon_Count()];
			}
			return mmc->Peek_Texture(idx,pass,stage);
		}
		return mmc->Peek_Single_Texture(pass,stage);
	}

	VertexMaterialClass* Peek_Material(unsigned idx,unsigned pass)
	{
		if (mmc->Has_Material_Array(pass)) {
			if (idx>=unsigned(mmc->Get_Polygon_Count())) {
				WWASSERT(mmc->Get_Gap_Filler());
				return mmc->Get_Gap_Filler()->Get_Material_Array(pass)[idx-mmc->Get_Polygon_Count()];
			}
			return mmc->Peek_Material(mmc->Get_Polygon_Array()[idx][0],pass);
		}
		return mmc->Peek_Single_Material(pass);
	}

	ShaderClass Peek_Shader(unsigned idx,unsigned pass)
	{
		if (mmc->Has_Shader_Array(pass)) {
			ShaderClass shader;
			
			if (idx>=unsigned(mmc->Get_Polygon_Count())) {
				WWASSERT(mmc->Get_Gap_Filler());
				shader=mmc->Get_Gap_Filler()->Get_Shader_Array(pass)[idx-mmc->Get_Polygon_Count()];
			}
			else shader=mmc->Get_Shader(idx,pass);

			if (npatch_enable) {
				shader.Set_NPatch_Enable(ShaderClass::NPATCH_ENABLE);
			}

			return shader;
		}
		if (!npatch_enable) return mmc->Get_Single_Shader(pass);
		ShaderClass shader=mmc->Get_Single_Shader(pass);
		shader.Set_NPatch_Enable(ShaderClass::NPATCH_ENABLE);
		return shader;

	}

	MeshModelClass* Get_Mesh_Model_Class()
	{
		return mmc;
	}

	unsigned short* Get_Polygon_Array(unsigned pass)
	{
		return (unsigned short*)polygon_array;
	}
};

// ----------------------------------------------------------------------------

void DX8RigidFVFCategoryContainer::Add_Mesh(MeshModelClass* mmc_)
{
	WWASSERT(Check_If_Mesh_Fits(mmc_));

	Vertex_Split_Table split_table(mmc_);
	int needed_vertices=split_table.Get_Vertex_Count();

	/*
	** This FVFCategoryContainer doesn't have a vertex buffer yet so allocate one big
	** enough to contain this mesh.
	*/
	if (!vertex_buffer) {
		int vb_size=4000;
		if (vb_size<needed_vertices) vb_size=needed_vertices;
		if (sorting) {
			vertex_buffer=NEW_REF(SortingVertexBufferClass,(vb_size));
			WWASSERT(vertex_buffer->FVF_Info().Get_FVF()==FVF);	// Only one sorting FVF type!
		}
		else {
			vertex_buffer=NEW_REF(DX8VertexBufferClass,(
				FVF,
				vb_size,
#if defined(ZH_WW3D_CPU_ONLY)
				DX8VertexBufferClass::USAGE_DEFAULT));
#else
				(DX8Wrapper::Get_Current_Caps()->Support_NPatches() && WW3D::Get_NPatches_Level()>1) ? DX8VertexBufferClass::USAGE_NPATCHES : DX8VertexBufferClass::USAGE_DEFAULT));
#endif
		}
	}

	/*
	** Append this mesh's vertices to the vertex buffer.
	*/

	VertexBufferClass::AppendLockClass l(vertex_buffer,used_vertices,split_table.Get_Vertex_Count());
	const FVFInfoClass fi=vertex_buffer->FVF_Info();
	unsigned char *vb=(unsigned char*) l.Get_Vertex_Array();
	unsigned int i;
	const Vector3 *locs=split_table.Get_Vertex_Array();
	const Vector3 *norms=split_table.Get_Vertex_Normal_Array();
	const unsigned *diffuse=split_table.Get_Color_Array(0);
	const unsigned *specular=split_table.Get_Color_Array(1);
	for (i=0; i<split_table.Get_Vertex_Count(); i++)
	{
		*(Vector3*)(vb+fi.Get_Location_Offset())=locs[i];
		
		if ((FVF&fvf_normal)==fvf_normal && norms) {
			*(Vector3*)(vb+fi.Get_Normal_Offset())=norms[i];
		}

		if ((FVF&fvf_diffuse)==fvf_diffuse) {
			if (diffuse) {
				*(unsigned int*)(vb+fi.Get_Diffuse_Offset())=diffuse[i];
			} else {
				*(unsigned int*)(vb+fi.Get_Diffuse_Offset()) = 0xFFFFFFFF;
			}
		}
		
		if ((FVF&fvf_specular)==fvf_specular) {
			if (specular) {
				*(unsigned int*)(vb+fi.Get_Specular_Offset())=specular[i];
			} else {
				*(unsigned int*)(vb+fi.Get_Specular_Offset()) = 0xFFFFFFFF;
			}
		}

		vb+=fi.Get_FVF_Size();
	}
	

	/*
	** Append the UV coordinates to the vertex buffer
	*/
	int uvcount = 0;
	if ((FVF&fvf_tex1) == fvf_tex1) {
		uvcount = 1;
	}
	if ((FVF&fvf_tex2) == fvf_tex2) {
		uvcount = 2;
	}
	if ((FVF&fvf_tex3) == fvf_tex3) {
		uvcount = 3;
	}
	if ((FVF&fvf_tex4) == fvf_tex4) {
		uvcount = 4;
	}
	if ((FVF&fvf_tex5) == fvf_tex5) {
		uvcount = 5;
	}
	if ((FVF&fvf_tex6) == fvf_tex6) {
		uvcount = 6;
	}
	if ((FVF&fvf_tex7) == fvf_tex7) {
		uvcount = 7;
	}
	if ((FVF&fvf_tex8) == fvf_tex8) {
		uvcount = 8;
	}
	
	for (int j=0; j<uvcount; j++) {
		unsigned char *vb=(unsigned char*) l.Get_Vertex_Array();
		const Vector2*uvs=split_table.Get_UV_Array(j);
		if (uvs) {
			for (i=0; i<split_table.Get_Vertex_Count(); i++)
			{
				*(Vector2*)(vb+fi.Get_Tex_Offset(j))=uvs[i];
				vb+=fi.Get_FVF_Size();
			}		
		}
	}

	Generate_Texture_Categories(split_table,used_vertices);

	used_vertices+=needed_vertices;//vertex_count;
}

void DX8FVFCategoryContainer::Insert_To_Texture_Category(
	Vertex_Split_Table& split_table,
	TextureClass** texs,
	VertexMaterialClass* mat,
	ShaderClass shader,
	int pass,
	unsigned vertex_offset)
{
	/*
	** Try to find a DX8TextureCategoryClass in this FVF container which matches the
	** given textures(one per stage), material and shader combination.
	*/
	bool fit_in_existing_category = false;
	TextureCategoryListIterator it(&texture_category_list[pass]);
	while (!it.Is_Done()) {
		DX8TextureCategoryClass * tex_category=it.Peek_Obj();
		// Compare all stage's textures
		bool all_textures_same = true;
		for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			all_textures_same = all_textures_same && (tex_category->Peek_Texture(stage) == texs[stage]);
		}
		if (all_textures_same && Equal_Material(tex_category->Peek_Material(),mat) && tex_category->Get_Shader()==shader) {
			used_indices+=tex_category->Add_Mesh(split_table,vertex_offset,used_indices,index_buffer,pass);
			fit_in_existing_category = true;
			break;
		}
		it.Next();
	}

	if (!fit_in_existing_category) {
		
		DX8TextureCategoryClass * new_tex_category=W3DNEW DX8TextureCategoryClass(this,texs,shader,mat,pass);
#if defined(ZH_WW3D_CPU_ONLY)
		const auto cleanup=[](DX8TextureCategoryClass *p) { DX8MeshRendererClass::Withdraw_Prepared_Category(p);delete p; };
		std::unique_ptr<DX8TextureCategoryClass,decltype(cleanup)> candidate(new_tex_category,cleanup);
		if (source_prepared_model) ww3d_clone::Attempt::fault();
#endif
		used_indices+=new_tex_category->Add_Mesh(split_table,vertex_offset,used_indices,index_buffer,pass);
		
		/*
		** Add the texture category object into the list, immediately after any existing
		** texture category object which uses the same texture.  This will result in
		** the list always having matching texture categories next to each other.
		*/
		bool found_similar_category = false;
		TextureCategoryListIterator it(&texture_category_list[pass]);
		while (!it.Is_Done()) {
			// Categorize according to first stage's texture for now
			if (it.Peek_Obj()->Peek_Texture(0) == texs[0]) {
				texture_category_list[pass].Add_After(new_tex_category,it.Peek_Obj());
				found_similar_category = true;
				break;
			}
			it.Next();
		}

		if (!found_similar_category) {
#if defined(ZH_WW3D_CPU_ONLY)
			if (source_prepared_model) ww3d_clone::Attempt::fault();
#endif
			texture_category_list[pass].Add_Tail(new_tex_category);
		}
#if defined(ZH_WW3D_CPU_ONLY)
		candidate.release();
#endif
	}
}

const unsigned MAX_ADDED_TYPE_COUNT=64;
struct Textures_Material_And_Shader_Booking_Struct
{
	TextureClass* added_textures[MeshMatDescClass::MAX_TEX_STAGES][MAX_ADDED_TYPE_COUNT];
	VertexMaterialClass* added_materials[MAX_ADDED_TYPE_COUNT];
	ShaderClass added_shaders[MAX_ADDED_TYPE_COUNT];
	unsigned added_type_count;

	Textures_Material_And_Shader_Booking_Struct() : added_type_count(0) {}

	bool Add_Textures_Material_And_Shader(TextureClass** texs, VertexMaterialClass* mat, ShaderClass shd)
	{
		for (unsigned a=0;a<added_type_count;++a) {
			// Compare textures
			bool all_textures_same = true;
			for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
				all_textures_same = all_textures_same && (texs[stage] == added_textures[stage][a]);
			}
			if (all_textures_same && Equal_Material(mat,added_materials[a]) && shd==added_shaders[a]) {
				return false;
			}
		}
		WWASSERT(added_type_count<MAX_ADDED_TYPE_COUNT);
#if defined(ZH_WW3D_CPU_ONLY)
		if (added_type_count==MAX_ADDED_TYPE_COUNT) throw std::runtime_error("original mesh material booking capacity rejected");
#endif
		for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			added_textures[stage][added_type_count]=texs[stage];
		}
		added_materials[added_type_count]=mat;
		added_shaders[added_type_count]=shd;
		added_type_count++;
		return true;
	}
};

void DX8FVFCategoryContainer::Generate_Texture_Categories(Vertex_Split_Table& split_table,unsigned vertex_offset)
{
	int polygon_count=split_table.Get_Polygon_Count();
	int index_count=polygon_count*3*split_table.Get_Pass_Count(); 

	/*
	** If we don't have an index buffer yet, allocate one.  Make it hold at least 12000 entries,
	** more if the mesh requires it.  
	*/
	if (!index_buffer) {
		int ib_size=12000;  
		if (ib_size<index_count) ib_size=index_count;
		if (sorting) {
			index_buffer=NEW_REF(SortingIndexBufferClass,(ib_size));
		}
		else {
			index_buffer=NEW_REF(DX8IndexBufferClass,(
				ib_size,
#if defined(ZH_WW3D_CPU_ONLY)
				DX8IndexBufferClass::USAGE_DEFAULT));
#else
				(DX8Wrapper::Get_Current_Caps()->Support_NPatches() && WW3D::Get_NPatches_Level()>1) ? DX8IndexBufferClass::USAGE_NPATCHES : DX8IndexBufferClass::USAGE_DEFAULT));
#endif
		}
	}

	for (unsigned pass=0;pass<split_table.Get_Pass_Count();++pass) {
		Textures_Material_And_Shader_Booking_Struct textures_material_and_shader_booking;

		unsigned old_used_indices=used_indices;

		for (int i=0;i<polygon_count;++i) {
			TextureClass* textures[MeshMatDescClass::MAX_TEX_STAGES];
			// disabled this assert as MAX_TEXTURE_STAGES is now 8, but legacy MeshMat::MAX_TEX_STAGES is still 2
	//		WWASSERT(MAX_TEXTURE_STAGES==MeshMatDescClass::MAX_TEX_STAGES);
			for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;stage++) 
			{
				textures[stage]=split_table.Peek_Texture(i,pass,stage);
			}
			VertexMaterialClass* mat=split_table.Peek_Material(i,pass);
			ShaderClass shader=split_table.Peek_Shader(i,pass);
			if (!textures_material_and_shader_booking.Add_Textures_Material_And_Shader(textures,mat,shader)) continue;

			Insert_To_Texture_Category(split_table,textures,mat,shader,pass,vertex_offset);
		}

		int new_inds=used_indices-old_used_indices;
		WWASSERT(new_inds<=polygon_count*3);
	}
}

// ----------------------------------------------------------------------------

DX8SkinFVFCategoryContainer::DX8SkinFVFCategoryContainer(bool sorting)
	:
	DX8FVFCategoryContainer(DX8_FVF_XYZNUV1,sorting),
	VisibleVertexCount(0),
	VisibleSkinHead(NULL),
	VisibleSkinTail(NULL)
{
}

// ----------------------------------------------------------------------------

DX8SkinFVFCategoryContainer::~DX8SkinFVFCategoryContainer()
{
}

// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Log(bool only_visible)
{
#ifdef ENABLE_CATEGORY_LOG
	StringClass work(255,true);
	work.Format("DX8SkinFVFCategoryContainer --------------\n");
	WWDEBUG_SAY((work));

	if (index_buffer) {
		work.Format("IB size (used/total): %d/%d\n",used_indices,index_buffer->Get_Index_Count());
		WWDEBUG_SAY((work));
	}
	else {
		WWDEBUG_SAY(("EMPTY IB\n"));
	}

	for (unsigned pass=0;pass<passes;++pass) {
		TextureCategoryListIterator it(&texture_category_list[pass]);
		while (!it.Is_Done()) {
			it.Peek_Obj()->Log(only_visible);
			it.Next();
		}
	}
#endif
}

// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Render(void)
{
	SNAPSHOT_SAY(("DX8SkinFVFCategoryContainer::Render()\n"));
	if (!Anything_To_Render()) {
		SNAPSHOT_SAY(("Nothing to render\n"));
		return;
	}
	AnythingToRender=false;
	// The Linux physical edge can reject a pending upload/draw. Keep the
	// original skin/category queues live until a successful flush, so the
	// owning scene can abort the pass and retry the same source submission.
#if defined(ZH_WW3D_CPU_ONLY)
	try {
#endif

	DX8Wrapper::Set_Vertex_Buffer(NULL);	// Free up the reference to the current vertex buffer
														// (in case it is the dynamic, which may have to be resized)

	//'Generals' customization to allow more than 65535 vertices
	unsigned int maxVertexCount=VisibleVertexCount;
	if (maxVertexCount > 65535)
	{	//clamp vertex count to maximum size that can be indexed by 16-bit index
		maxVertexCount = 65535;
	}

	DynamicVBAccessClass vb(
		sorting ? BUFFER_TYPE_DYNAMIC_SORTING : BUFFER_TYPE_DYNAMIC_DX8,
		dynamic_fvf_type,
		maxVertexCount);
	SNAPSHOT_SAY(("DynamicVBAccess - %s - %d vertices\n",sorting ? "sorting" : "non-sorting",VisibleVertexCount));

	unsigned int renderedVertexCount=0;

	MeshClass * mesh = VisibleSkinHead;
	MeshClass * remainingMesh = VisibleSkinHead;
	while (renderedVertexCount < VisibleVertexCount)
	{	mesh = remainingMesh;
		{	DynamicVBAccessClass::WriteLockClass l(&vb);
			VertexFormatXYZNDUV2 * dest_verts = l.Get_Formatted_Vertex_Array();
			unsigned vertex_offset=0;
			remainingMesh = NULL;

			while (mesh != NULL) {

				MeshModelClass * mmc = mesh->Peek_Model();
				int mesh_vertex_count=mmc->Get_Vertex_Count();
				//'Generals' mod to deal with cases where not all meshes fit in VB.
				if (vertex_offset+mesh_vertex_count > maxVertexCount || remainingMesh)
				{	//flag mesh so we know it didn't fit in the vertex buffer
					mesh->Set_Base_Vertex_Offset(VERTEX_BUFFER_OVERFLOW);
					if (remainingMesh == NULL)
						remainingMesh = mesh;	//start of meshes that didn't fit in buffer
					mesh = mesh->Peek_Next_Visible_Skin();	//skip rendering this mesh
					continue;
				}


		// If this assert hits, a skinned mesh has probably been added to the scene more than once.
		// Example: A skinned mesh was added to the scene then it was attached to a bone without being removed from the scene.
		WWASSERT((vertex_offset+mesh_vertex_count)<=VisibleVertexCount);
			DX8_RECORD_SKIN_RENDER(mesh->Get_Num_Polys(),mesh_vertex_count);

#if defined(ZH_WW3D_CPU_ONLY)
				DX8MeshRendererClass::Prepare_Source_Skin_Scratch(mesh_vertex_count);
#else
				if (_TempVertexBuffer.Length() < mesh_vertex_count) _TempVertexBuffer.Resize(mesh_vertex_count);
				if (_TempNormalBuffer.Length() < mesh_vertex_count) _TempNormalBuffer.Resize(mesh_vertex_count);
#endif

				Vector3* loc=&(_TempVertexBuffer[0]);
				Vector3* norm=&(_TempNormalBuffer[0]);
				const Vector2* uv0=mmc->Get_UV_Array_By_Index(0);
				const Vector2* uv1=mmc->Get_UV_Array_By_Index(1);
				const unsigned* diffuse=mmc->Get_Color_Array(0,false);

				VertexFormatXYZNDUV2* verts=dest_verts+vertex_offset;

				mesh->Get_Deformed_Vertices(loc,norm);

				for (int v=0;v<mesh_vertex_count;++v) {
					verts[v].x=(*loc)[0];
					verts[v].y=(*loc)[1];
					verts[v].z=(*loc)[2];
					verts[v].nx=(*norm)[0];
					verts[v].ny=(*norm)[1];
					verts[v].nz=(*norm)[2];
					if (diffuse) {
						verts[v].diffuse=*diffuse++;
					}
					else {
						verts[v].diffuse=0;
					}
					if (uv0) {
						verts[v].u1=(*uv0)[0];
						verts[v].v1=(*uv0)[1];
						uv0++;
					}
					else {
						verts[v].u1=0.0f;
						verts[v].v1=0.0f;
					}
					if (uv1) {
						verts[v].u2=(*uv1)[0];
						verts[v].v2=(*uv1)[1];
						uv1++;
					}
					else {
						verts[v].u2=0.0f;
						verts[v].v2=0.0f;
					}

					loc++;
					norm++;
				}

				mesh->Set_Base_Vertex_Offset(vertex_offset);
				vertex_offset+=mesh_vertex_count;
				renderedVertexCount += mesh_vertex_count;
				
				mesh = mesh->Peek_Next_Visible_Skin();
			}	//while
		}//lock

		SNAPSHOT_SAY(("Set vb: %x ib: %x\n",vb,index_buffer));

		DX8Wrapper::Set_Vertex_Buffer(vb);
		DX8Wrapper::Set_Index_Buffer(index_buffer,0);

		//Flush the meshes which fit in the vertex buffer, applying all texture variations
		for (unsigned pass=0;pass<passes;++pass) {
			SNAPSHOT_SAY(("Pass: %d\n",pass));

			TextureCategoryListIterator it(&visible_texture_category_list[pass]);
			while (!it.Is_Done()) {
				it.Peek_Obj()->Render();
				it.Next();
			}
		}

		Render_Procedural_Material_Passes();
	}//while

	//remove all the rendered data from queues
	for (unsigned pass=0;pass<passes;++pass) {
#if defined(ZH_WW3D_CPU_ONLY)
		while (auto *tex=static_cast<DX8TextureCategoryClass*>(DX8MeshRendererClass::Remove_Source_Frame_Head(visible_texture_category_list[pass]))) {
#else
		while (DX8TextureCategoryClass * tex = visible_texture_category_list[pass].Remove_Head()) {
#endif
		}
	}

	WWASSERT(renderedVertexCount==VisibleVertexCount);


	clearVisibleSkinList();
#if defined(ZH_WW3D_CPU_ONLY)
	} catch (...) {
		AnythingToRender=true;
		throw;
	}
#endif
}

bool DX8SkinFVFCategoryContainer::Check_If_Mesh_Fits(MeshModelClass* mmc)
{
	if (!index_buffer) return true;	// No IB created - mesh will fit as a new ib will be created when inserting
	int required_polygons=mmc->Get_Polygon_Count();
	if (mmc->Get_Gap_Filler()) {
		required_polygons+=mmc->Get_Gap_Filler()->Get_Polygon_Count();
	}

	if ((required_polygons*3*mmc->Get_Pass_Count())<=index_buffer->Get_Index_Count()-used_indices) {
		return true;
	}
	return false;
}

void DX8SkinFVFCategoryContainer::clearVisibleSkinList() 
{
	while (VisibleSkinHead != NULL)
	{
		MeshClass* next = VisibleSkinHead->Peek_Next_Visible_Skin();
		VisibleSkinHead->Set_Next_Visible_Skin(NULL);
		VisibleSkinHead = next;
	}
	VisibleSkinHead = NULL;
	VisibleSkinTail = NULL;
	VisibleVertexCount = 0;
}
void DX8SkinFVFCategoryContainer::Add_Visible_Skin(MeshClass * mesh) 
{
	if (mesh->Peek_Next_Visible_Skin() != NULL || mesh == VisibleSkinTail)
	{
		DEBUG_CRASH(("Mesh %s is already a visible skin, and we tried to add it again... please notify Mark W or Steven J immediately!\n",mesh->Get_Name()));
		return;
	}
	if (VisibleSkinHead == NULL)
		VisibleSkinTail = mesh;
	mesh->Set_Next_Visible_Skin(VisibleSkinHead);
	VisibleSkinHead = mesh;
	VisibleVertexCount += mesh->Peek_Model()->Get_Vertex_Count();
}


// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Reset()
{
	clearVisibleSkinList();
	
	for (unsigned pass=0;pass<passes;++pass) {
		while (DX8TextureCategoryClass* texture_category=texture_category_list[pass].Peek_Head()) {
			delete texture_category;
		}
	}

	REF_PTR_RELEASE(index_buffer);
	used_indices=0;
}

// ----------------------------------------------------------------------------

void DX8SkinFVFCategoryContainer::Add_Mesh(MeshModelClass* mmc)
{
	Vertex_Split_Table split_table(mmc);

	Generate_Texture_Categories(split_table,0);
}

// ----------------------------------------------------------------------------

unsigned DX8TextureCategoryClass::Add_Mesh(
	Vertex_Split_Table& split_table,
	unsigned vertex_offset,
	unsigned index_offset,
	IndexBufferClass* index_buffer,
	unsigned pass)
{
	int poly_count=split_table.Get_Polygon_Count();

	unsigned index_count=0;

	/*
	** Count the polygons in the given mesh in the given pass which match this texture category
	*/
	unsigned polygons=0;

	for (int i=0;i<poly_count;++i) {
		bool all_textures_same = true;
		for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			all_textures_same = all_textures_same && (split_table.Peek_Texture(i, pass, stage) == textures[stage]);
		}
		VertexMaterialClass* mat=split_table.Peek_Material(i,pass);
		ShaderClass shd=split_table.Peek_Shader(i,pass);

		if (all_textures_same && Equal_Material(mat,material) && shd==shader) {
			polygons++;
		}
	}

	/*
	** Add the indices for the polygons that match into this renderer's dx8 index table.
	*/
	if (polygons) {

		index_count=polygons*3;
#ifndef ENABLE_STRIPING
		bool stripify=false;
#else
		bool stripify=true;
		if (index_buffer->Type()==BUFFER_TYPE_SORTING || index_buffer->Type()==BUFFER_TYPE_DYNAMIC_SORTING) {
			stripify=false;
		}
#endif
		const TriIndex* src_indices=(const TriIndex*)split_table.Get_Polygon_Array(pass);//mmc->Get_Polygon_Array();

		if (stripify) {
			int* triangles=W3DNEWARRAY int[index_count];
			int triangle_index_count=0;
			for (int i=0;i<poly_count;++i) {
				bool all_textures_same = true;
				for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
					all_textures_same = all_textures_same && (split_table.Peek_Texture(i, pass, stage) == textures[stage]);
				}
				VertexMaterialClass* mat=split_table.Peek_Material(i,pass);
				ShaderClass shd=split_table.Peek_Shader(i,pass);

				if (all_textures_same && Equal_Material(mat,material) && shd==shader) {
					triangles[triangle_index_count++]=src_indices[i][0]+vertex_offset;
					triangles[triangle_index_count++]=src_indices[i][1]+vertex_offset;
					triangles[triangle_index_count++]=src_indices[i][2]+vertex_offset;
				}
			}

			int* strips=StripOptimizerClass::Stripify(triangles, triangle_index_count/3);
			delete[] triangles;
			int* strip=StripOptimizerClass::Combine_Strips(strips+1,strips[0]);
			delete[] strips;

			if (index_count<unsigned(strip[0])) {
				stripify=false;
			}
			else {
				index_count=strip[0];

				DX8PolygonRendererClass* p_renderer=W3DNEW DX8PolygonRendererClass(
					index_count,
					split_table.Get_Mesh_Model_Class(),
					this,
					vertex_offset,
					index_offset,
					true,
					pass);
				PolygonRendererList.Add_Tail(p_renderer);

				{
					IndexBufferClass::AppendLockClass l(index_buffer,index_offset,index_count);
					unsigned short* dst_indices=l.Get_Index_Array();

					unsigned short vmin=0xffff;
					unsigned short vmax=0;

					/*
					** Iterate over the polys for this pass, adding each one that matches this texture+material+shader
					*/
					for (unsigned i=0;i<index_count;++i) {
						unsigned short idx;

						idx=static_cast<unsigned short>(strip[i+1]);
						vmin=MIN(vmin,idx);
						vmax=MAX(vmax,idx);
						*dst_indices++=idx;
					}
					
					/*
					** Remember the min and max vertex indices that these polygons used (for optimization)
					*/
					p_renderer->Set_Vertex_Index_Range(vmin,vmax-vmin+1);
				}
			}
			delete[] strip;
		}

		// Need to check stripify again as it may be changed to false by the previous statement
		if (!stripify ) {
			DX8PolygonRendererClass* p_renderer=W3DNEW DX8PolygonRendererClass(
				index_count,
				split_table.Get_Mesh_Model_Class(),
				this,
				vertex_offset,
				index_offset,
				false,
				pass);
#if defined(ZH_WW3D_CPU_ONLY)
			const auto cleanup=[](DX8PolygonRendererClass *p) { p->Set_Texture_Category(nullptr);delete p; };
			std::unique_ptr<DX8PolygonRendererClass,decltype(cleanup)> candidate(p_renderer,cleanup);
			if (source_prepared_model) ww3d_clone::Attempt::fault();
#endif
			PolygonRendererList.Add_Tail(p_renderer);
#if defined(ZH_WW3D_CPU_ONLY)
			candidate.release();
#endif

			IndexBufferClass::AppendLockClass l(index_buffer,index_offset,index_count);
			unsigned short* dst_indices=l.Get_Index_Array();

			unsigned short vmin=0xffff;
			unsigned short vmax=0;

			/*
			** Iterate over the polys for this pass, adding each one that matches this texture+material+shader
			*/
			for (int i=0;i<poly_count;++i) {
				bool all_textures_same = true;
				for (unsigned int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
					all_textures_same = all_textures_same && (split_table.Peek_Texture(i, pass, stage) == textures[stage]);
				}
				VertexMaterialClass* mat=split_table.Peek_Material(i,pass);
				ShaderClass shd=split_table.Peek_Shader(i,pass);

				if (all_textures_same && Equal_Material(mat,material) && shd==shader) {
					unsigned short idx;

					idx=static_cast<unsigned short>(src_indices[i][0]+vertex_offset);
					vmin=MIN(vmin,idx);
					vmax=MAX(vmax,idx);
					*dst_indices++=idx;
//					WWDEBUG_SAY(("%d, ",idx));

					idx=static_cast<unsigned short>(src_indices[i][1]+vertex_offset);
					vmin=MIN(vmin,idx);
					vmax=MAX(vmax,idx);
					*dst_indices++=idx;
//					WWDEBUG_SAY(("%d, ",idx));

					idx=static_cast<unsigned short>(src_indices[i][2]+vertex_offset);
					vmin=MIN(vmin,idx);
					vmax=MAX(vmax,idx);
					*dst_indices++=idx;
//					WWDEBUG_SAY(("%d\n",idx));
				}
			}

			WWASSERT((vmax-vmin)<split_table.Get_Mesh_Model_Class()->Get_Vertex_Count());
			
			/*
			** Remember the min and max vertex indices that these polygons used (for optimization)
			*/
			p_renderer->Set_Vertex_Index_Range(vmin,vmax-vmin+1);
			WWASSERT(index_count<=unsigned(split_table.Get_Polygon_Count()*3));
		}
	}

	return index_count;
}

// ----------------------------------------------------------------------------

void DX8TextureCategoryClass::Render(void)
{
	#ifdef WWDEBUG
	if (!WW3D::Expose_Prelit()) {
	#endif

		for (unsigned i=0;i<MeshMatDescClass::MAX_TEX_STAGES;++i) 
		{
			SNAPSHOT_SAY(("Set_Texture(%d,%s)\n",i,Peek_Texture(i) ? Peek_Texture(i)->Get_Texture_Name() : "NULL"));
			DX8Wrapper::Set_Texture(i,Peek_Texture(i));
		}

	#ifdef WWDEBUG
	}
	#endif

	SNAPSHOT_SAY(("Set_Material(%s)\n",Peek_Material() ? Peek_Material()->Get_Name() : "NULL"));
	VertexMaterialClass *vmaterial=(VertexMaterialClass *)Peek_Material();	//ugly cast from const but we'll restore it after changes so okay. -MW
	DX8Wrapper::Set_Material(vmaterial);

	SNAPSHOT_SAY(("Set_Shader(%x)\n",Get_Shader()));
	ShaderClass theShader = Get_Shader();

	//Setup an alpha blend version of this shader just in case it's needed. -MW
	ShaderClass theAlphaShader = theShader;
	theAlphaShader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
	theAlphaShader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
	//if we want to allow other translucent polygons behind this mesh, we need to disable z-write but
	//this will cause sorting errors on this mesh.
	//theAlphaShader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);

	DX8Wrapper::Set_Shader(theShader);

	if (m_gForceMultiply && theShader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO) {
		theShader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_SRC_COLOR);
		theShader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ZERO);
		DX8Wrapper::Set_Shader(theShader);
		//VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
		//DX8Wrapper::Set_Material(material);
		//REF_PTR_RELEASE(material);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
	}


	bool renderTasksRemaining=false;

	PolyRenderTaskClass * prt = render_task_head;
	PolyRenderTaskClass * last_prt = NULL;

	while (prt) {

		/*
		** Dig out the parameters for this render task
		*/
		DX8PolygonRendererClass * renderer = prt->Peek_Polygon_Renderer();
		MeshClass * mesh = prt->Peek_Mesh();
		
		if (mesh->Get_Base_Vertex_Offset() == VERTEX_BUFFER_OVERFLOW)	//check if this mesh is valid
		{	//skip this mesh so it gets rendered later after vertices are filled in.
			last_prt = prt;
			prt = prt->Get_Next_Visible();
			renderTasksRemaining = true;
			continue;
		}

		SNAPSHOT_SAY(("mesh = %s\n",mesh->Get_Name()));

		#ifdef WWDEBUG	
		// Debug rendering: if it exists, expose prelighting on this mesh by disabling all base textures.
		if (WW3D::Expose_Prelit()) {
			switch (mesh->Peek_Model()->Get_Flag (MeshGeometryClass::PRELIT_MASK)) {

				unsigned i;

				case MeshGeometryClass::PRELIT_VERTEX:
					
					// Disable texturing on all stages and passes.
					for (i = 0; i < MeshMatDescClass::MAX_TEX_STAGES; i++) 
					{
						DX8Wrapper::Set_Texture (i, NULL);
					}
					break;

				case MeshGeometryClass::PRELIT_LIGHTMAP_MULTI_PASS:
					
					// Disable texturing on all but the last pass.
					if (pass == mesh->Peek_Model()->Get_Pass_Count() - 1) {
						for (i = 0; i < MeshMatDescClass::MAX_TEX_STAGES; i++) 
						{
							DX8Wrapper::Set_Texture (i, Peek_Texture (i));
						}
					} else {
						for (i = 0; i < MAX_TEXTURE_STAGES; i++) {
							DX8Wrapper::Set_Texture (i, NULL);
						}
					}
					break;

				case MeshGeometryClass::PRELIT_LIGHTMAP_MULTI_TEXTURE:
					
					// Disable texturing on all but the zeroth stage of each pass.
					DX8Wrapper::Set_Texture (0, Peek_Texture (0));
					for (i = 1; i < MeshMatDescClass::MAX_TEX_STAGES; i++) 
					{
						DX8Wrapper::Set_Texture (i, NULL);
					}
					break;

				default:
					for (i = 0; i < MeshMatDescClass::MAX_TEX_STAGES; i++) 
					{
						DX8Wrapper::Set_Texture (i, Peek_Texture (i));
					}
					break;
			}
		}
		#endif

		/*
		** If the user is not installing LightEnvironmentClasses, we leave the lighting render
		** states untouched.  This way they can set a couple global lights that affect the entire scene.
		*/
		LightEnvironmentClass * lenv = mesh->Get_Lighting_Environment();
		if (lenv != NULL) {
			SNAPSHOT_SAY(("LightEnvironment, lights: %d\n",lenv->Get_Light_Count()));
			DX8Wrapper::Set_Light_Environment(lenv);
		}
		else {
			SNAPSHOT_SAY(("No light environment\n"));
		}

		/*
		** Support for ALIGNED and ORIENTED camera modes
		*/
		const Matrix3D* world_transform = &mesh->Get_Transform();
		bool identity=mesh->Is_Transform_Identity();
		Matrix3D tmp_world;

		if (mesh->Peek_Model()->Get_Flag(MeshModelClass::ALIGNED)) {
			SNAPSHOT_SAY(("Camera mode ALIGNED\n"));

			Vector3 mesh_position;
			Vector3 camera_z_vector;
			
			TheDX8MeshRenderer.Peek_Camera()->Get_Transform().Get_Z_Vector(&camera_z_vector);
			mesh->Get_Transform().Get_Translation(&mesh_position);

			tmp_world.Obj_Look_At(mesh_position,mesh_position + camera_z_vector,0.0f);
			world_transform = &tmp_world;

		} else if (mesh->Peek_Model()->Get_Flag(MeshModelClass::ORIENTED)) {
			SNAPSHOT_SAY(("Camera mode ORIENTED\n"));
		
			Vector3 mesh_position;
			Vector3 camera_position;

			TheDX8MeshRenderer.Peek_Camera()->Get_Transform().Get_Translation(&camera_position);
			mesh->Get_Transform().Get_Translation(&mesh_position);

			tmp_world.Obj_Look_At(mesh_position,camera_position,0.0f);
			world_transform = &tmp_world;
		
		} else if (mesh->Peek_Model()->Get_Flag(MeshModelClass::SKIN)) {
			SNAPSHOT_SAY(("Set world identity (for skin)\n"));
			
			tmp_world.Make_Identity();
			world_transform = &tmp_world;
			identity=true;
		}


		if (identity) {
			SNAPSHOT_SAY(("Set_World_Identity\n"));
			DX8Wrapper::Set_World_Identity();
		}
		else {
			SNAPSHOT_SAY(("Set_World_Transform\n"));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,*world_transform);
		}

		
//--------------------------------------------------------------------
		if (mesh->Get_ObjectScale() != 1.0f)
			DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS, TRUE);
//--------------------------------------------------------------------
		/*
		** Render mesh using either sorting or immediate pipeline
		*/
		//(gth) this if statement's contents are not tabbed to avoid perforce merge problems...
		if (
#if !defined(ZH_WW3D_CPU_ONLY)
			!DX8RendererDebugger::Is_Enabled() ||
#endif
			!mesh->Is_Disabled_By_Debugger()) {

		if ((!!mesh->Peek_Model()->Get_Flag(MeshGeometryClass::SORT)) && WW3D::Is_Sorting_Enabled()) {
			renderer->Render_Sorted(mesh->Get_Base_Vertex_Offset(),mesh->Get_Bounding_Sphere());
		} else {
			//non-transparent mesh that will be rendered immediately.  Okay to adjust the shader/material
			//if necessary
			if (mesh->Get_Alpha_Override() != 1.0 || (mesh->Get_User_Data() && *(int *)mesh->Get_User_Data() == RenderObjClass::USER_DATA_MATERIAL_OVERRIDE))
			{	//mesh has material override of some kind
				//adjust the opacity of this model
				float oldOpacity=vmaterial->Get_Opacity();
				Vector3 oldDiffuse;
				Vector2 oldUVOffset;
				unsigned int oldUVOffsetSyncTime;
				vmaterial->Get_Diffuse(&oldDiffuse);
				LinearOffsetTextureMapperClass *oldMapper=(LinearOffsetTextureMapperClass *)vmaterial->Peek_Mapper();
				if ( mesh->Get_User_Data() && *(int *)mesh->Get_User_Data() == RenderObjClass::USER_DATA_MATERIAL_OVERRIDE && oldMapper && oldMapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET)
				{	RenderObjClass::Material_Override *matOverride=(RenderObjClass::Material_Override *)mesh->Get_User_Data();
					oldUVOffsetSyncTime = oldMapper->Get_LastUsedSyncTime();
					oldMapper->Set_LastUsedSyncTime(WW3D::Get_Sync_Time());	//make sure zero time passes for the mapper.
					oldMapper->Get_Current_UV_Offset(oldUVOffset);
					oldMapper->Set_Current_UV_Offset(matOverride->customUVOffset);
				}
				else
					oldMapper=NULL;
				if (mesh->Get_Alpha_Override() != 1.0)
				{
					if (mesh->Is_Additive())
					{	//additvie blended mesh can't switch to alpha or we will get a black outline.
						//so adjust diffuse color instead.
						vmaterial->Set_Diffuse(mesh->Get_Alpha_Override(),mesh->Get_Alpha_Override(),mesh->Get_Alpha_Override());
						theAlphaShader = theShader;	//keep using additive blending.
					}
					vmaterial->Set_Opacity(mesh->Get_Alpha_Override());
					DX8Wrapper::Set_Shader(theAlphaShader);
					DX8Wrapper::Apply_Render_State_Changes();
					DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,(int)((float)0x60*mesh->Get_Alpha_Override()));
					renderer->Render(mesh->Get_Base_Vertex_Offset());
					DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,0x60);
					vmaterial->Set_Opacity(oldOpacity);	//restore previous value
					vmaterial->Set_Diffuse(oldDiffuse.X,oldDiffuse.Y,oldDiffuse.Z);
					DX8Wrapper::Set_Shader(theShader);	//restore previous value
				}
				else
					renderer->Render(mesh->Get_Base_Vertex_Offset());

				if (oldMapper)	//did we override the uv offset?
				{	oldMapper->Set_LastUsedSyncTime(oldUVOffsetSyncTime);
					oldMapper->Set_Current_UV_Offset(oldUVOffset);
				}
				DX8Wrapper::Set_Material(NULL);	//force a reset of vertex material since we secretly changed opacity
				DX8Wrapper::Set_Material(vmaterial);	//restore previous material.
			} 
			else
				renderer->Render(mesh->Get_Base_Vertex_Offset());
		}
//--------------------------------------------------------------------
		if (mesh->Get_ObjectScale() != 1.0f)
			DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS, FALSE);
//--------------------------------------------------------------------




        } // (gth) non-tabbed to aviod per-force merge problems...

		/*
		** Move to the next render task.  Note that the delete should be fast because prt's are pooled
		*/
		PolyRenderTaskClass * next_prt = prt->Get_Next_Visible();

		// remove from list, then delete
		if (last_prt == NULL) {
		   render_task_head = next_prt;
		} else {
		  last_prt->Set_Next_Visible(next_prt);
		}

		source_retire(prt);
		prt = next_prt;
	}

	if (!renderTasksRemaining)
	{	WWASSERT(!render_task_head);
		Clear_Render_List();
	}
}


DX8MeshRendererClass::DX8MeshRendererClass()
	:
	camera(NULL),
	enable_lighting(true),
	texture_category_container_list_skin(NULL),
	visible_decal_meshes(NULL)
{
}

DX8MeshRendererClass::~DX8MeshRendererClass()
{
	Invalidate(true);
	Clear_Pending_Delete_Lists();
	if (texture_category_container_list_skin != NULL) {
		delete texture_category_container_list_skin;
	}
}

void DX8MeshRendererClass::Init(void)
{
	// DMS - Only allocate one if we havent already (leak fix)
	if(!texture_category_container_list_skin)
		texture_category_container_list_skin = W3DNEW FVFCategoryList;
}

void DX8MeshRendererClass::Shutdown(void)
{
	Invalidate(true);
	Clear_Pending_Delete_Lists();
	_TempVertexBuffer.Clear();	//free memory
	_TempNormalBuffer.Clear();
	#if defined(ZH_WW3D_CPU_ONLY)
	// Rendering has returned every per-draw task. Retain an active pool on a
	// failed/incomplete frame; otherwise retire its process-static slab before
	// the next original WW3D generation.
	const bool render_tasks_drained = PolyRenderTaskClass::Release_Empty_Blocks();
	WWASSERT(render_tasks_drained);
	const bool material_tasks_drained = MatPassTaskClass::Release_Empty_Blocks();
	WWASSERT(material_tasks_drained);
	#endif
}

// ----------------------------------------------------------------------------

void DX8MeshRendererClass::Clear_Pending_Delete_Lists()
{
#if defined(ZH_WW3D_CPU_ONLY)
	if (source_mesh_frame) {
		auto& c=*source_mesh_frame;
		if (texture_category_delete_list.Count()>static_cast<int>(c.retired_categories.capacity()-c.retired_categories.size())
			|| fvf_category_container_delete_list.Count()>static_cast<int>(c.retired_containers.capacity()-c.retired_containers.size()))
			throw std::runtime_error("original source deferred retirement capacity rejected");
		while (auto *category=static_cast<DX8TextureCategoryClass*>(Remove_Source_Frame_Head(texture_category_delete_list)))
			c.retired_categories.push_back(category);
		while (auto *container=static_cast<DX8FVFCategoryContainer*>(Remove_Source_Frame_Head(fvf_category_container_delete_list)))
			c.retired_containers.push_back(container);
		return;
	}
#endif
	while (DX8TextureCategoryClass* category=texture_category_delete_list.Remove_Head()) {
		delete category;
	}
	while (DX8FVFCategoryContainer* container=fvf_category_container_delete_list.Remove_Head()) {
		delete container;
	}
}

// ----------------------------------------------------------------------------

static void Add_Rigid_Mesh_To_Container(FVFCategoryList* container_list,unsigned fvf,MeshModelClass* mmc)
{
	WWASSERT(container_list);
	DX8FVFCategoryContainer * container = NULL;
	bool sorting=((!!mmc->Get_Flag(MeshModelClass::SORT)) && WW3D::Is_Sorting_Enabled() && (mmc->Get_Sort_Level() == SORT_LEVEL_NONE));

	FVFCategoryListIterator it(container_list);
	while (!it.Is_Done()) {
		container = it.Peek_Obj();
		if (sorting==container->Is_Sorting() && container->Check_If_Mesh_Fits(mmc)) {
			container->Add_Mesh(mmc);
			return;
		}
		it.Next();
	}

	container=W3DNEW DX8RigidFVFCategoryContainer(fvf,sorting);
	container_list->Add_Tail(container);
	container->Add_Mesh(mmc);
}

// ----------------------------------------------------------------------------

void DX8MeshRendererClass::Unregister_Mesh_Type(MeshModelClass* mmc)
{
	while (DX8PolygonRendererClass* n=mmc->PolygonRendererList.Remove_Head()) {
		delete n;
	}
	_RegisteredMeshList.Remove(mmc);

	// Also remove the gap filler!
	if (mmc->GapFiller) {
		GapFillerClass* gf=mmc->GapFiller;
		mmc->GapFiller=NULL;
		delete gf;
	}

}

#if defined(ZH_WW3D_CPU_ONLY)
void DX8MeshRendererClass::Prepare_Mesh_Type_Strong(MeshModelClass *model)
{
    auto *edge=zh::original_runtime::OriginalGpuEdge::active();
    if (!model || !edge || !edge->idle_preparation_ready() || edge->tree_source_frame_pending()
        || source_mesh_frame || source_prepared_polygons || !model->Get_Vertex_Count()
        || model->Get_Vertex_Count()>65535 || !model->Get_Polygon_Count() || model->Get_Pass_Count()>DX8FVFCategoryContainer::MAX_PASSES
        || !model->Get_Pass_Count() || model->Get_Polygon_Count()>65535U/(3*model->Get_Pass_Count())
        || model->GapFiller || WW3D::Get_NPatches_Level()>1
        || WW3D::Get_NPatches_Gap_Filling_Mode()==WW3D::NPATCHES_GAP_FILLING_FORCE)
        throw std::runtime_error("original prop mesh preparation owner/range rejected");
    if (!model->PolygonRendererList.Is_Empty()) return;
    const bool skin=model->Get_Flag(MeshModelClass::SKIN) && model->VertexBoneLink;
    const bool sorting=model->Get_Flag(MeshModelClass::SORT) && WW3D::Is_Sorting_Enabled() && model->Get_Sort_Level()==SORT_LEVEL_NONE;
    const unsigned fvf=skin?DX8_FVF_XYZNUV1:DX8FVFCategoryContainer::Define_FVF(model,enable_lighting);
    FVFCategoryList *destination=nullptr;
    DX8FVFCategoryContainer *accepted=nullptr;
    DX8FVFCategoryContainer *base=nullptr;
    if (skin) destination=texture_category_container_list_skin;
    else for (int i=0;i<texture_category_container_lists_rigid.Count();++i) {
        auto *list=texture_category_container_lists_rigid[i];
        if (!list) throw std::runtime_error("original prop FVF provider rejected");
        if (!list->Peek_Head() || list->Peek_Head()->Get_FVF()==fvf) { destination=list;break; }
    }
    if (!destination && source_registration_batch)
        for (const auto& group:source_registration_batch->groups)
            if (group.skin==skin && group.fvf==fvf) { destination=group.list;break; }
    if (destination) {
        FVFCategoryListIterator iterator(destination);
        for (;!iterator.Is_Done();iterator.Next()) {
            auto *container=iterator.Peek_Obj();
            auto *effective=container;
            if (source_registration_batch) for (const auto& prior:source_registration_batch->containers)
                if (prior.root==container) { effective=prior.latest;break; }
            if (effective->sorting==sorting && effective->Check_If_Mesh_Fits(model)) { accepted=container;base=effective;break; }
        }
        if (!accepted && source_registration_batch) for (const auto& prior:source_registration_batch->containers) {
            if (prior.list==destination && prior.latest->sorting==sorting && prior.latest->Check_If_Mesh_Fits(model))
                { accepted=prior.root;base=prior.latest;break; }
        }
    }
    if (accepted && !accepted->Source_Frame_Empty())
        throw std::runtime_error("original prop preparation overlaps queued category work");
    ww3d_clone::Attempt::fault();
    auto new_list=std::make_shared<std::unique_ptr<FVFCategoryList>>();
    ww3d_clone::Attempt::fault();
    auto replacement=std::make_shared<std::unique_ptr<FVFCategoryList*[]>>();
    int replacement_capacity=texture_category_container_lists_rigid.VectorMax;
    if (!destination) {
        ww3d_clone::Attempt::fault();new_list->reset(new FVFCategoryList);destination=new_list->get();
        if (!skin) {
            auto& lists=texture_category_container_lists_rigid;
            if (lists.ActiveCount>=4096 || lists.VectorMax<lists.ActiveCount)
                throw std::runtime_error("original prop FVF list capacity rejected");
            if (!source_registration_batch && lists.ActiveCount==lists.VectorMax) {
                replacement_capacity=std::max(lists.VectorMax+lists.VectorMax/4,lists.VectorMax+4);
                if (replacement_capacity>4096) throw std::runtime_error("original prop FVF growth capacity rejected");
                replacement->reset(new FVFCategoryList*[replacement_capacity]());
                if (lists.ActiveCount) std::copy(lists.Vector,lists.Vector+lists.ActiveCount,replacement->get());
            }
        }
    }
    ww3d_clone::Attempt::fault();auto private_model=std::make_shared<DX8PolygonRendererList>();
    struct PreparedLists {
        FVFCategoryList containers;
        MultiListClass<MeshModelClass> registered;
        ~PreparedLists() { while (containers.Remove_Head()) {} while (registered.Remove_Head()) {} }
    };
    ww3d_clone::Attempt::fault();auto private_lists=std::make_shared<PreparedLists>();
    ww3d_clone::Attempt::fault();auto transferred=std::make_shared<bool>(false);
    const auto cleanup=[private_model,private_lists,transferred](DX8FVFCategoryContainer *p) {
        if (!*transferred) DX8MeshRendererClass::Destroy_Prepared_Container(p);
    };
    ww3d_clone::Attempt::fault();std::shared_ptr<DX8FVFCategoryContainer> candidate(
        skin?static_cast<DX8FVFCategoryContainer*>(new DX8SkinFVFCategoryContainer(sorting)):
             static_cast<DX8FVFCategoryContainer*>(new DX8RigidFVFCategoryContainer(fvf,sorting)),cleanup);
    struct CategoryPair { DX8TextureCategoryClass *candidate,*accepted; };
    std::vector<CategoryPair> pairs;
    ww3d_clone::Attempt::fault();pairs.reserve(4096);
    auto *rigid=dynamic_cast<DX8RigidFVFCategoryContainer*>(candidate.get());
    auto *old_rigid=accepted?dynamic_cast<DX8RigidFVFCategoryContainer*>(accepted):nullptr;
    auto *base_rigid=base?dynamic_cast<DX8RigidFVFCategoryContainer*>(base):nullptr;
    const auto copy_vertex=[&](VertexBufferClass *buffer) {
        ww3d_clone::Attempt::fault();
        if (buffer->Engine_Refs()) throw std::runtime_error("original prop vertex append ownership rejected");
        if (buffer->Type()==BUFFER_TYPE_DX8) {
            auto *copy=new DX8VertexBufferClass(fvf,buffer->Get_Vertex_Count(),DX8VertexBufferClass::USAGE_DEFAULT);
            std::memcpy(copy->Get_CPU_Vertex_Buffer(),static_cast<DX8VertexBufferClass*>(buffer)->Get_CPU_Vertex_Buffer(),
                std::size_t(buffer->Get_Vertex_Count())*buffer->FVF_Info().Get_FVF_Size());return static_cast<VertexBufferClass*>(copy);
        }
        if (buffer->Type()!=BUFFER_TYPE_SORTING) throw std::runtime_error("original prop vertex provider rejected");
        auto *copy=new SortingVertexBufferClass(buffer->Get_Vertex_Count());
        std::memcpy(copy->VertexBuffer,static_cast<SortingVertexBufferClass*>(buffer)->VertexBuffer,
            std::size_t(buffer->Get_Vertex_Count())*sizeof(VertexFormatXYZNDUV2));return static_cast<VertexBufferClass*>(copy);
    };
    const auto copy_index=[&](IndexBufferClass *buffer) {
        ww3d_clone::Attempt::fault();
        if (buffer->Engine_Refs()) throw std::runtime_error("original prop index append ownership rejected");
        if (buffer->Type()==BUFFER_TYPE_DX8) {
            auto *copy=new DX8IndexBufferClass(buffer->Get_Index_Count(),DX8IndexBufferClass::USAGE_DEFAULT);
            std::memcpy(copy->Get_CPU_Index_Buffer(),static_cast<DX8IndexBufferClass*>(buffer)->Get_CPU_Index_Buffer(),
                std::size_t(buffer->Get_Index_Count())*sizeof(unsigned short));return static_cast<IndexBufferClass*>(copy);
        }
        if (buffer->Type()!=BUFFER_TYPE_SORTING) throw std::runtime_error("original prop index provider rejected");
        auto *copy=new SortingIndexBufferClass(buffer->Get_Index_Count());
        std::memcpy(copy->index_buffer,static_cast<SortingIndexBufferClass*>(buffer)->index_buffer,
            std::size_t(buffer->Get_Index_Count())*sizeof(unsigned short));return static_cast<IndexBufferClass*>(copy);
    };
    if (accepted) {
        if ((skin && !dynamic_cast<DX8SkinFVFCategoryContainer*>(accepted)) || (!skin && !old_rigid))
            throw std::runtime_error("original prop container kind rejected");
        candidate->used_indices=base->used_indices;
        if (base->index_buffer) candidate->index_buffer=copy_index(base->index_buffer);
        if (rigid) { rigid->used_vertices=base_rigid->used_vertices;if (base_rigid->vertex_buffer) rigid->vertex_buffer=copy_vertex(base_rigid->vertex_buffer); }
        for (unsigned pass=0;pass<DX8FVFCategoryContainer::MAX_PASSES;++pass) {
            TextureCategoryListIterator iterator(&base->texture_category_list[pass]);
            for (;!iterator.Is_Done();iterator.Next()) {
                auto *old=iterator.Peek_Obj();
                if (pairs.size()==pairs.capacity()) throw std::runtime_error("original prop category clone capacity rejected");
                ww3d_clone::Attempt::fault();
                std::unique_ptr<DX8TextureCategoryClass> next(new DX8TextureCategoryClass(candidate.get(),old->textures,old->shader,old->material,old->pass));
                next->shader=old->shader;
                ww3d_clone::Attempt::fault();
                candidate->texture_category_list[pass].Add_Tail(next.get());
                auto *root=old;
                if (source_registration_batch) for (const auto& prior:source_registration_batch->categories)
                    if (prior.candidate==old) { root=prior.root;break; }
                pairs.push_back({next.get(),root});next.release();
            }
        }
    }
    ww3d_clone::Attempt::fault();
    if (!accepted) private_lists->containers.Add_Tail(candidate.get());
    ww3d_clone::Attempt::fault();
    if (!skin) private_lists->registered.Add_Tail(model);
    struct Selector {
        Selector(MeshModelClass *model,DX8PolygonRendererList *list) { source_prepared_model=model;source_prepared_polygons=list; }
        ~Selector() { source_prepared_model=nullptr;source_prepared_polygons=nullptr; }
    } selector(model,private_model.get());
    ww3d_clone::Attempt::fault();candidate->Add_Mesh(model);
    if (private_model->Is_Empty()) throw std::runtime_error("original prop prepared polygon graph empty");
    if (source_registration_batch) {
        auto& batch=*source_registration_batch;
        const std::size_t bytes=std::size_t(candidate->index_buffer->Get_Index_Count())*sizeof(unsigned short)
            +(rigid?std::size_t(rigid->vertex_buffer->Get_Vertex_Count())*rigid->vertex_buffer->FVF_Info().Get_FVF_Size():0);
        if (bytes>64U*1024U*1024U-batch.bytes || batch.containers.size()==batch.containers.capacity()
            || pairs.size()>batch.categories.capacity()-batch.categories.size()
            || (*new_list && batch.groups.size()==batch.groups.capacity()))
            throw std::runtime_error("original prop registration batch capacity rejected");
        batch.bytes+=bytes;
        auto prior=std::find_if(batch.containers.begin(),batch.containers.end(),[&](const auto& p){return p.root==accepted;});
        if (accepted && prior!=batch.containers.end()) prior->latest=candidate.get();
        else batch.containers.push_back({accepted?accepted:candidate.get(),candidate.get(),destination});
        for (const auto& pair:pairs) batch.categories.push_back({pair.candidate,pair.accepted});
        if (*new_list) batch.groups.push_back({skin,fvf,destination});
    }
    // All storage, material refs, buffer bytes and both polygon memberships
    // now exist privately. The remaining publication consists of byte copies
    // and exact-node relinking; none allocates or calls a provider.
    const auto move_node=[](MultiListNodeClass *node,GenericMultiListClass& to,MultiListNodeClass *after) noexcept {
        node->Prev->Next=node->Next;node->Next->Prev=node->Prev;
        node->List=&to;node->Prev=after?after:to.Head.Prev;node->Next=node->Prev->Next;
        node->Next->Prev=node;node->Prev->Next=node;
    };
    auto publication=[this,model,accepted,old_rigid,rigid,candidate,transferred,private_lists,private_model,new_list,replacement,
                      replacement_capacity,pairs=std::move(pairs),destination,skin,move_node]() noexcept {
    if (accepted) {
        if (rigid) {
            if (!old_rigid->vertex_buffer) { old_rigid->vertex_buffer=rigid->vertex_buffer;rigid->vertex_buffer=nullptr; }
            else {
                auto *from=rigid->vertex_buffer;auto *to=old_rigid->vertex_buffer;
                void *target=to->Type()==BUFFER_TYPE_DX8?static_cast<void*>(static_cast<DX8VertexBufferClass*>(to)->Get_CPU_Vertex_Buffer()):static_cast<void*>(static_cast<SortingVertexBufferClass*>(to)->VertexBuffer);
                const void *bytes=from->Type()==BUFFER_TYPE_DX8?static_cast<const void*>(static_cast<DX8VertexBufferClass*>(from)->Get_CPU_Vertex_Buffer()):static_cast<const void*>(static_cast<SortingVertexBufferClass*>(from)->VertexBuffer);
                std::memcpy(target,bytes,std::size_t(to->Get_Vertex_Count())*to->FVF_Info().Get_FVF_Size());
            }
            old_rigid->used_vertices=rigid->used_vertices;
        }
        if (!accepted->index_buffer) { accepted->index_buffer=candidate->index_buffer;candidate->index_buffer=nullptr; }
        else {
            auto *from=candidate->index_buffer;auto *to=accepted->index_buffer;
            auto *target=to->Type()==BUFFER_TYPE_DX8?static_cast<DX8IndexBufferClass*>(to)->Get_CPU_Index_Buffer():static_cast<SortingIndexBufferClass*>(to)->index_buffer;
            const auto *bytes=from->Type()==BUFFER_TYPE_DX8?static_cast<DX8IndexBufferClass*>(from)->Get_CPU_Index_Buffer():static_cast<SortingIndexBufferClass*>(from)->index_buffer;
            std::memcpy(target,bytes,std::size_t(to->Get_Index_Count())*sizeof(unsigned short));
        }
        accepted->used_indices=candidate->used_indices;
        for (unsigned pass=0;pass<DX8FVFCategoryContainer::MAX_PASSES;++pass) {
            auto& source=candidate->texture_category_list[pass];auto& target=accepted->texture_category_list[pass];
            MultiListNodeClass *after=&target.Head;
            for (auto *node=source.Head.Next;node!=&source.Head;) {
                auto *next=node->Next;auto *category=static_cast<DX8TextureCategoryClass*>(node->Object);
                const auto pair=std::find_if(pairs.begin(),pairs.end(),[&](const auto& p){return p.candidate==category;});
                if (pair!=pairs.end()) {
                    auto& polygons=category->PolygonRendererList;auto& destination=pair->accepted->PolygonRendererList;
                    while (polygons.Head.Next!=&polygons.Head) {
                        auto *poly_node=polygons.Head.Next;
                        static_cast<DX8PolygonRendererClass*>(poly_node->Object)->Set_Texture_Category(pair->accepted);
                        move_node(poly_node,destination,nullptr);
                    }
                    for (after=target.Head.Next;after!=&target.Head && after->Object!=pair->accepted;after=after->Next) {}
                    if (after==&target.Head) std::terminate();
                } else {
                    category->container=accepted;move_node(node,target,after);after=node;
                }
                node=next;
            }
        }
    } else { move_node(private_lists->containers.Head.Next,*destination,nullptr);*transferred=true; }
    while (private_model->Head.Next!=&private_model->Head) move_node(private_model->Head.Next,model->PolygonRendererList,nullptr);
    if (!skin) move_node(private_lists->registered.Head.Next,_RegisteredMeshList,nullptr);
    if (*new_list) {
        if (skin) texture_category_container_list_skin=new_list->release();
        else {
            auto& lists=texture_category_container_lists_rigid;
            if (*replacement) { auto *old=lists.Vector;lists.Vector=replacement->release();lists.VectorMax=replacement_capacity;delete[] old; }
            lists.Vector[lists.ActiveCount++]=new_list->release();
        }
    }
    model->HasBeenInUse=true;
    };
    ww3d_clone::Attempt::fault();
    if (source_registration_batch) source_registration_batch->publications.push_back(std::move(publication));
    else publication();
}
void DX8MeshRendererClass::Prepare_Mesh_Batch_Strong(const std::vector<MeshModelClass*>& models)
{
    if (source_registration_batch || models.size()>4096)
        throw std::runtime_error("original prop registration batch owner rejected");
    for (std::size_t i=0;i<models.size();++i)
        if (!models[i] || std::find(models.begin(),models.begin()+i,models[i])!=models.begin()+i)
            throw std::runtime_error("original prop registration duplicate/provider rejected");
    ww3d_clone::Attempt attempt;
    SourceRegistrationBatch batch;
    ww3d_clone::Attempt::fault();batch.containers.reserve(4096);
    ww3d_clone::Attempt::fault();batch.groups.reserve(4096);
    ww3d_clone::Attempt::fault();batch.categories.reserve(32768);
    ww3d_clone::Attempt::fault();batch.publications.reserve(models.size());
    struct Owner {
        explicit Owner(SourceRegistrationBatch *p) { source_registration_batch=p; }
        ~Owner() { source_registration_batch=nullptr; }
    } owner(&batch);
    // Duplicate identities are rejected before any candidate allocation or
    // semantic source mutation; callers supply the graph's unique model set.
    for (auto *model:models) Prepare_Mesh_Type_Strong(model);
    auto& lists=texture_category_container_lists_rigid;
    int needed=lists.ActiveCount;
    for (const auto& group:batch.groups) if (!group.skin) ++needed;
    if (needed>4096) throw std::runtime_error("original prop batch FVF count rejected");
    int capacity=lists.VectorMax;
    while (capacity<needed) {
        capacity=std::max(capacity+capacity/4,capacity+4);
        if (capacity>4096) throw std::runtime_error("original prop batch FVF growth rejected");
    }
    std::unique_ptr<FVFCategoryList*[]> replacement;
    if (capacity!=lists.VectorMax) {
        ww3d_clone::Attempt::fault();
        replacement.reset(new FVFCategoryList*[capacity]());
        if (lists.ActiveCount) std::copy(lists.Vector,lists.Vector+lists.ActiveCount,replacement.get());
    }
    // Complete graph and capacity admission precedes this allocation-free
    // publication sequence. No source/model callback can observe half state.
    ww3d_clone::Attempt::fault();
    if (replacement) { auto *old=lists.Vector;lists.Vector=replacement.release();lists.VectorMax=capacity;delete[] old; }
    for (auto& publish:batch.publications) publish();
    attempt.commit();
}
#endif


void DX8MeshRendererClass::Register_Mesh_Type(MeshModelClass* mmc)
{
	WWMEMLOG(MEM_GEOMETRY);
#ifdef ENABLE_CATEGORY_LOG
	WWDEBUG_SAY(("Registering mesh: %s (%d polys, %d verts + %d gap polygons)\n",mmc->Get_Name(),mmc->Get_Polygon_Count(),mmc->Get_Vertex_Count(),mmc->Get_Gap_Filler_Polygon_Count()));
#endif
	bool skin=(mmc->Get_Flag(MeshModelClass::SKIN) && mmc->VertexBoneLink);
	bool sorting=((!!mmc->Get_Flag(MeshModelClass::SORT)) && WW3D::Is_Sorting_Enabled() && (mmc->Get_Sort_Level() == SORT_LEVEL_NONE));

	if (skin) {

		/*
		** This mesh is a skin.  Add it to a DX8SkinFVFCategoryContainer.
		*/
		WWASSERT(texture_category_container_list_skin);

		FVFCategoryListIterator it(texture_category_container_list_skin);
		while (!it.Is_Done()) {
			DX8FVFCategoryContainer * container = it.Peek_Obj();
			if (sorting==container->Is_Sorting() && container->Check_If_Mesh_Fits(mmc)) {
				container->Add_Mesh(mmc);
				return;
			}
			it.Next();
		}

		DX8FVFCategoryContainer * new_container=W3DNEW DX8SkinFVFCategoryContainer(sorting);
		texture_category_container_list_skin->Add_Tail(new_container);
		new_container->Add_Mesh(mmc);
	
	} else {

		/*
		** We should never try to add the same mesh model to the system twice.
		*/
		WWASSERT_PRINT(_RegisteredMeshList.Contains(mmc) == false,("Mesh name: %s",mmc->Get_Name()));

		/*
		** If the previous step didn't add the mesh, then we have to actually process this mesh
		*/
		if (!_RegisteredMeshList.Contains(mmc)) {

			unsigned fvf=DX8FVFCategoryContainer::Define_FVF(mmc,enable_lighting);

			/*
			** Search for an existing FVF Category Container that matches this mesh
			*/
			int i;
			for (i=0;i<texture_category_container_lists_rigid.Count();++i) {
				FVFCategoryList * list=texture_category_container_lists_rigid[i];
				WWASSERT(list);
				DX8FVFCategoryContainer * container=list->Peek_Head();
				if (container && container->Get_FVF()!=fvf) continue;

				Add_Rigid_Mesh_To_Container(list,fvf,mmc);
				break;
			}

			if (i==texture_category_container_lists_rigid.Count()) {

				/*
				** We couldn't find an existing FVF category container so we have to add one.  Future
				** meshes that use this FVF will also be able to use this container.
				*/
				FVFCategoryList * new_fvf_category = W3DNEW FVFCategoryList();
				texture_category_container_lists_rigid.Add(new_fvf_category);
				Add_Rigid_Mesh_To_Container(new_fvf_category,fvf,mmc);
			}

			/*
			** Done processing the mesh, add its polygon renderers to the global registered mesh list
			*/
			if (mmc->PolygonRendererList.Is_Empty() == false) {
				_RegisteredMeshList.Add_Tail(mmc);
			}
			else {
				WWDEBUG_SAY(("Error: Register_Mesh_Type failed! file: %s line: %d\r\n",__FILE__,__LINE__));
			}
		}
	}

	return;
}

static unsigned statistics_requested=0;

void DX8MeshRendererClass::Request_Log_Statistics()
{
	statistics_requested=WW3D::Get_Frame_Count();
}


// ---------------------------------------------------------------------------
//
// Render all meshes that are added to visible lists
//
// ---------------------------------------------------------------------------

static void Render_FVF_Category_Container_List(FVFCategoryList& list)
{
	FVFCategoryListIterator it(&list);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Render();
		it.Next();
	}
}

static void Render_FVF_Category_Container_List_Delayed_Passes(FVFCategoryList& list)
{
	FVFCategoryListIterator it(&list);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Render_Delayed_Procedural_Material_Passes();
		it.Next();
	}
}

void DX8MeshRendererClass::Flush(void)
{
	int i;

	WWPROFILE("DX8MeshRenderer::Flush");
	if (!camera) return;
	#if defined(ZH_WW3D_CPU_ONLY)
	try {
	#endif
	Log_Statistics_String(true);

	/*
	** Render the FVF categories.  Note that it is critical that skins be 
	** rendered *after* the rigid meshes.  This is caused by the fact that an object may
	** have its base passes disabled and a translucent procedural material pass rendered
	** instead.  In this case, technically we have to delay rendering of the material pass but
	** for skins we just render these passes as we go because we can assume that the
	** bulk of the meshes have already been drawn (there would be extra overhead involved
	** in solving this for skins)
	*/
	for (i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Render_FVF_Category_Container_List(*texture_category_container_lists_rigid[i]);
	}

	Render_FVF_Category_Container_List(*texture_category_container_list_skin);

	Render_Decal_Meshes();

	/*
	** Render the translucent procedural material passes that were applied to meshes that
	** had their base passes disabled. 
	*/
	for (i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Render_FVF_Category_Container_List_Delayed_Passes(*texture_category_container_lists_rigid[i]);
	}
	#if defined(ZH_WW3D_CPU_ONLY)
	} catch (...) {
		DX8Wrapper::Set_Vertex_Buffer(NULL);
		DX8Wrapper::Set_Index_Buffer(NULL,0);
		throw;
	}
	#endif

	DX8Wrapper::Set_Vertex_Buffer(NULL);
	DX8Wrapper::Set_Index_Buffer(NULL,0);
}


void DX8MeshRendererClass::Add_To_Render_List(DecalMeshClass * decalmesh)
{
	WWASSERT(decalmesh != NULL);
	decalmesh->Set_Next_Visible(visible_decal_meshes);
	visible_decal_meshes = decalmesh;
}

void DX8MeshRendererClass::Render_Decal_Meshes(void)
{
	DecalMeshClass * decal_mesh = visible_decal_meshes;
	if (!decal_mesh) return;

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,8);
	#if defined(ZH_WW3D_CPU_ONLY)
	try {
	#endif
	while (decal_mesh != NULL) {
		decal_mesh->Render();
		decal_mesh = decal_mesh->Peek_Next_Visible();
	}
	visible_decal_meshes = NULL;

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,0);
	#if defined(ZH_WW3D_CPU_ONLY)
	} catch (...) {
		// The caller must abort the frame and requeue through original Mesh::Render.
		// Never retain a link into an owner it may release after the failed pass.
		visible_decal_meshes=NULL;
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,0);
		throw;
	}
#endif
}

// ----------------------------------------------------------------------------

static void Log_Container_List(FVFCategoryList& container_list,bool only_visible)
{
	FVFCategoryListIterator it(&container_list);
	while (!it.Is_Done()) {
		it.Peek_Obj()->Log(only_visible);
		it.Next();
	}
}

void DX8MeshRendererClass::Log_Statistics_String(bool only_visible)
{
	if (statistics_requested!=WW3D::Get_Frame_Count()) return;

	for (int i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Log_Container_List(*texture_category_container_lists_rigid[i],only_visible);
	}
	Log_Container_List(*texture_category_container_list_skin,only_visible);

}

static void Invalidate_FVF_Category_Container_List(FVFCategoryList& list)
{
	while (DX8FVFCategoryContainer* fvf_category=list.Remove_Head()) {
		delete fvf_category;
	}
}

void DX8MeshRendererClass::Invalidate( bool shutdown)
{
	WWMEMLOG(MEM_RENDERER);
	_RegisteredMeshList.Reset_List();
#if defined(ZH_WW3D_CPU_ONLY)
	// A Linux physical error unwinds through the original Flush. Discard the
	// abandoned scene's source-owned decal links before their meshes release.
	visible_decal_meshes=NULL;
#endif

	for (int i=0;i<texture_category_container_lists_rigid.Count();++i) {
		Invalidate_FVF_Category_Container_List(*texture_category_container_lists_rigid[i]);
		delete texture_category_container_lists_rigid[i];
	}
	if (texture_category_container_list_skin) {
		Invalidate_FVF_Category_Container_List(*texture_category_container_list_skin);
		delete texture_category_container_list_skin;
		texture_category_container_list_skin=NULL;
	}

	if (!shutdown)
		texture_category_container_list_skin = W3DNEW FVFCategoryList;

	texture_category_container_lists_rigid.Delete_All();
}
