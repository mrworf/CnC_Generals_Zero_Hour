#include "prop_graph.h"
#if defined(__linux__)
#include "assetmgr.h"
#include "assetstatus.h"
#include "proto.h"
#include "hlod.h"
#include "collect.h"
#include "distlod.h"
#include "hmdldef.h"
#include "htree.h"
#include "mesh.h"
#include "meshmdl.h"
#include "nullrobj.h"
#include "boxrobj.h"
#include "agg_def.h"
#include "dazzle.h"
#include "ringobj.h"
#include "sphereobj.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <typeinfo>

namespace ww3d_prop {
namespace {
std::size_t name_extent(const char* value)
{
	if (!value) throw std::runtime_error("original prop name is absent");
	const std::size_t n=strnlen(value,1024);
	if (!n || n==1024) throw std::runtime_error("original prop name is not admitted");
	return n;
}
bool finite(const Vector3& v) noexcept
{ return std::isfinite(v.X) && std::isfinite(v.Y) && std::isfinite(v.Z); }
PrototypeClass* resolve(WW3DAssetManager& manager,const char* name)
{
	const std::size_t n=name_extent(name);
	PrototypeClass* p=manager.Find_Prototype(name);
	if (!p && manager.Get_WW3D_Load_On_Demand()) {
		const char* dot=strchr(name,'.');
		const std::size_t prefix=dot ? static_cast<std::size_t>(dot-name) : n;
		if (prefix>MAX_PATH-5) throw std::runtime_error("original prop load name is not admitted");
		char filename[MAX_PATH];
		memcpy(filename,name,prefix);memcpy(filename+prefix,".w3d",5);
		AssetStatusClass::Peek_Instance()->Report_Load_On_Demand_RObj(name);
		if (!manager.Load_3D_Assets(filename)) {
			StringClass parent(StringClass("..\\"),true);parent+=filename;
			manager.Load_3D_Assets(parent);
		}
		p=manager.Find_Prototype(name);
	}
	return p;
}
}

void Audit::enter(const void* identity)
{
	if (!identity || depth==path.size() || nodes==65536)
		throw std::runtime_error("original prop graph bound is not admitted");
	for (unsigned i=0;i<depth;++i)
		if (path[i]==identity) throw std::runtime_error("original prop graph cycle is not admitted");
	path[depth++]=identity;++nodes;
}

void Audit::named(const char* name,bool optional)
{
	PrototypeClass* p=resolve(manager,name);
	if (!p) {
		if (optional) return;
		throw std::runtime_error("original prop prototype is absent");
	}
	prototype(p);
}

void Audit::hierarchy(const HTreeClass* p)
{
	if (!p) return; // Authored absent hierarchy is handled by the constructor.
	ww3d_clone::Attempt::extent(p->NumPivots,sizeof(PivotClass));
	if (p->NumPivots<=0 || !p->Pivot || !std::isfinite(p->ScaleFactor))
		throw std::runtime_error("original prop hierarchy is not admitted");
	const auto first=reinterpret_cast<std::uintptr_t>(p->Pivot);
	const std::size_t bytes=static_cast<std::size_t>(p->NumPivots)*sizeof(PivotClass);
	for (int i=0;i<p->NumPivots;++i) {
		if (p->Pivot[i].Index!=i || strnlen(p->Pivot[i].Name,sizeof(p->Pivot[i].Name))==sizeof(p->Pivot[i].Name))
			throw std::runtime_error("original prop hierarchy pivot is not admitted");
		const auto parent=reinterpret_cast<std::uintptr_t>(p->Pivot[i].Parent);
		if (parent && (parent<first || parent-first>=bytes ||
			(parent-first)%sizeof(PivotClass) || (parent-first)/sizeof(PivotClass)>=static_cast<unsigned>(i)))
			throw std::runtime_error("original prop hierarchy parent is not admitted");
		for (int r=0;r<3;++r) for (int c=0;c<4;++c)
			if (!std::isfinite(p->Pivot[i].BaseTransform[r][c]))
				throw std::runtime_error("original prop hierarchy matrix is not admitted");
	}
}

void Audit::prototype(PrototypeClass* p)
{
	enter(p);
	try {
		// Aggregate intentionally aliases a render class ID. Exact type identity
		// must precede class-ID or definition access, including nested children.
		if (dynamic_cast<AggregatePrototypeClass*>(p) || dynamic_cast<DazzlePrototypeClass*>(p) ||
			dynamic_cast<RingPrototypeClass*>(p) || dynamic_cast<SpherePrototypeClass*>(p))
			throw std::runtime_error("original prop prototype kind is not admitted");
		if (typeid(*p)==typeid(PrimitivePrototypeClass)) render(static_cast<PrimitivePrototypeClass*>(p)->Proto);
		else if (typeid(*p)==typeid(HLodPrototypeClass)) {
			auto* typed=static_cast<HLodPrototypeClass*>(p);
			const auto* d=typed->Get_Definition();
			if (!d || d->LodCount<=0 || d->LodCount>1024 || !d->Lod)
				throw std::runtime_error("original prop composite definition is not admitted");
			const HTreeClass *tree=NULL;
			if (d->HierarchyTreeName && d->HierarchyTreeName[0]) {
				name_extent(d->HierarchyTreeName);tree=manager.Get_HTree(d->HierarchyTreeName);hierarchy(tree);
			}
			const int bones=tree?tree->Num_Pivots():1;
			for (int l=0;l<d->LodCount;++l) {
				ww3d_clone::Attempt::extent(d->Lod[l].ModelCount,sizeof(RenderObjClass*));
				if (d->Lod[l].ModelCount && (!d->Lod[l].ModelName || !d->Lod[l].BoneIndex))
					throw std::runtime_error("original prop child storage is absent");
				for (int m=0;m<d->Lod[l].ModelCount;++m) {
					if (d->Lod[l].BoneIndex[m]<0 || d->Lod[l].BoneIndex[m]>=bones)
						throw std::runtime_error("original prop bone is not admitted");
					named(d->Lod[l].ModelName[m],true);
				}
			}
			ww3d_clone::Attempt::extent(d->Aggregates.ModelCount,sizeof(RenderObjClass*));
			if (d->Aggregates.ModelCount && (!d->Aggregates.ModelName || !d->Aggregates.BoneIndex))
				throw std::runtime_error("original prop aggregate storage is absent");
			for (int m=0;m<d->Aggregates.ModelCount;++m) {
				if (d->Aggregates.BoneIndex[m]<0 || d->Aggregates.BoneIndex[m]>=bones)
					throw std::runtime_error("original prop bone is not admitted");
				named(d->Aggregates.ModelName[m],true);
			}
		} else if (typeid(*p)==typeid(DistLODPrototypeClass)) {
			auto* typed=static_cast<DistLODPrototypeClass*>(p);
			const auto* d=typed->Definition;
			if (!d || d->LodCount<=0 || d->LodCount>=256 || !d->Lods)
				throw std::runtime_error("original prop legacy definition is not admitted");
			for (int i=0;i<d->LodCount;++i) {
				if (!std::isfinite(d->Lods[i].ResUpDist) || !std::isfinite(d->Lods[i].ResDownDist))
					throw std::runtime_error("original prop legacy distance is not admitted");
				named(d->Lods[i].Name);
			}
		} else if (typeid(*p)==typeid(NullPrototypeClass) || typeid(*p)==typeid(BoxPrototypeClass)) {
			// Their admitted source construction is scalar-only; resulting bounds
			// are checked before any type/instance publication.
		} else if (!inspect_hmodel(p,*this) && !inspect_collection(p,*this))
			throw std::runtime_error("original prop foreign prototype is not admitted");
		leave();
	} catch (...) { leave();throw; }
}

void Audit::render(RenderObjClass* p)
{
	enter(p);
	try {
		name_extent(p->Get_Name());
		if (typeid(*p)==typeid(HLodClass)) {
			auto* typed=static_cast<HLodClass*>(p);
			if (typed->LodCount<=0 || typed->LodCount>1024 || !typed->Lod)
				throw std::runtime_error("original prop composite source is not admitted");
			hierarchy(typed->Get_HTree());
			const int bones=typed->Get_HTree()?typed->Get_HTree()->Num_Pivots():1;
			for (int l=0;l<typed->LodCount;++l) {
				ww3d_clone::Attempt::extent(typed->Lod[l].Count(),sizeof(RenderObjClass*));
				for (int m=0;m<typed->Lod[l].Count();++m) {
					if(typed->Lod[l][m].BoneIndex<0 || typed->Lod[l][m].BoneIndex>=bones)
						throw std::runtime_error("original prop bone is not admitted");
					render(typed->Lod[l][m].Model);
				}
			}
			ww3d_clone::Attempt::extent(typed->AdditionalModels.Count(),sizeof(RenderObjClass*));
			for (int m=0;m<typed->AdditionalModels.Count();++m) {
				if(typed->AdditionalModels[m].BoneIndex<0 || typed->AdditionalModels[m].BoneIndex>=bones)
					throw std::runtime_error("original prop bone is not admitted");
				render(typed->AdditionalModels[m].Model);
			}
		} else if (typeid(*p)==typeid(CollectionClass)) {
			auto* typed=static_cast<CollectionClass*>(p);
			ww3d_clone::Attempt::extent(typed->SubObjects.Count(),sizeof(RenderObjClass*));
			ww3d_clone::Attempt::extent(typed->ProxyList.Count(),sizeof(ProxyClass));
			for (int i=0;i<typed->ProxyList.Count();++i) {
				name_extent(typed->ProxyList[i].Get_Name());
				const Matrix3D& matrix=typed->ProxyList[i].Get_Transform();
				for(int r=0;r<3;++r) for(int c=0;c<4;++c)
					if(!std::isfinite(matrix[r][c])) throw std::runtime_error("original prop proxy matrix is not admitted");
			}
			for (int i=0;i<typed->SubObjects.Count();++i) render(typed->SubObjects[i]);
		} else if (typeid(*p)==typeid(DistLODClass)) {
			auto* typed=static_cast<DistLODClass*>(p);
			if (typed->LodCount<=0 || typed->LodCount>=256 || !typed->Lods)
				throw std::runtime_error("original prop legacy source is not admitted");
			for (int i=0;i<typed->LodCount;++i) render(typed->Lods[i].Model);
		} else if (typeid(*p)==typeid(MeshClass)) {
			auto* typed=static_cast<MeshClass*>(p);
			MeshModelClass* model=typed->Peek_Model();
			if (!model) throw std::runtime_error("original prop mesh provider is absent");
			ww3d_clone::Attempt::extent(model->Get_Vertex_Count(),sizeof(Vector3));
			ww3d_clone::Attempt::extent(model->Get_Polygon_Count(),sizeof(Vector3i));
		} else if (typeid(*p)==typeid(Null3DObjClass) || typeid(*p)==typeid(AABoxRenderObjClass) ||
			typeid(*p)==typeid(OBBoxRenderObjClass)) {
			SphereClass bounds;p->Get_Obj_Space_Bounding_Sphere(bounds);
			if (!finite(bounds.Center) || !std::isfinite(bounds.Radius))
				throw std::runtime_error("original prop bounds are not admitted");
		} else throw std::runtime_error("original prop render kind is not admitted");
		leave();
	} catch (...) { leave();throw; }
}

void Audit::hmodel(const HModelDefClass& d)
{
	ww3d_clone::Attempt::extent(d.SubObjectCount,sizeof(RenderObjClass*));
	if (d.SubObjectCount && !d.SubObjects) throw std::runtime_error("original prop hierarchy storage is absent");
	const HTreeClass *tree=NULL;
	if (strnlen(d.BasePoseName,sizeof(d.BasePoseName))==sizeof(d.BasePoseName))
		throw std::runtime_error("original prop hierarchy name is not admitted");
	if (d.BasePoseName[0]) { tree=manager.Get_HTree(d.BasePoseName);hierarchy(tree); }
	const int bones=tree?tree->Num_Pivots():1;
	for (int i=0;i<d.SubObjectCount;++i) {
		if (d.SubObjects[i].PivotID<0 || d.SubObjects[i].PivotID>=bones)
			throw std::runtime_error("original prop bone is not admitted");
		named(d.SubObjects[i].RenderObjName,true);
	}
}

Attempt::Attempt(WW3DAssetManager* p):previous(current),owner(p)
{
	if (!p || p!=WW3DAssetManager::Get_Instance() || current)
		throw std::runtime_error("original prop construction owner is not admitted");
	// Plain prop clones share mesh/material/mapper ownership; they do not clone
	// mapper state. Accepted load-on-demand cache publication retains its native
	// mapper initialization stream, rather than rewinding that provider's work.
	clone.commit();
	current=this;
}

void Attempt::check(WW3DAssetManager& manager,PrototypeClass* p,const char* name)
{
	if (!current) return;
	if (current->owner!=&manager || &manager!=WW3DAssetManager::Get_Instance() ||
		!p || manager.Find_Prototype(name)!=p)
		throw std::runtime_error("original prop construction provider is not admitted");
	if (!current->admitted) {
		Audit audit(manager);audit.named(name);current->admitted=true;
	}
}
}
#endif
