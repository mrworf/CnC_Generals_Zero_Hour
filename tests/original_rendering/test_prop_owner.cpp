#include "PreRTS.h"
#include "clone_graph.h"
#include "hlod.h"
#include "collect.h"
#include "nullrobj.h"
#include "htree.h"
#include "pivot.h"
#include "ww3d.h"
#include "assetmgr.h"
#include "proto.h"
#include "prop_graph.h"
#include "prop_frame.h"
#include "dx8renderer.h"
#include "static_sort_list.h"
#include "rinfo.h"
#include "camera.h"
#include "mesh.h"
#include "meshmdl.h"
#include "lightenvironment.h"
#include "light.h"
#include "dx8polygonrenderer.h"
#include "mapper.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include "chunkio.h"
#include "RAMFILE.H"
#include "w3d_file.h"
#include "distlod.h"
#include "proxy.h"
#include "agg_def.h"
#include "ringobj.h"
#include "sphereobj.h"
#include "dazzle.h"
#include "Common/SubsystemInterface.h"
#include "W3DDevice/GameClient/W3DPropBuffer.h"
#include "Common/GameMemory.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <limits>
#include <memory>
#include <array>
#include <vector>
#undef assert
#define assert(value) do { if (!(value)) throw std::runtime_error("generated prop invariant: " #value); } while (false)

struct LightEnvironmentProbe : LightEnvironmentClass {
	void neutral() const
	{
		const Vector3 zero(0,0,0);
		const auto input=[&](const InputLightStruct& light) {
			assert(light.Direction==zero && light.Ambient==zero && light.Diffuse==zero);
			assert(!light.DiffuseRejected && !light.m_point && light.m_center==zero);
			assert(light.m_innerRadius==0 && light.m_outerRadius==0 && light.m_ambient==zero && light.m_diffuse==zero);
		};
		assert(LightCount==0 && ObjectCenter==zero && OutputAmbient==zero && FillIntensity==0);
		for (int i=0;i<MAX_LIGHTS;++i) {
			input(InputLights[i]);assert(OutputLights[i].Direction==zero && OutputLights[i].Diffuse==zero);
		}
		input(FillLight);
	}
	void equivalent(const LightEnvironmentProbe& other) const
	{
		const auto input=[](const InputLightStruct& a,const InputLightStruct& b) {
			assert(a.Direction==b.Direction && a.Ambient==b.Ambient && a.Diffuse==b.Diffuse);
			assert(a.DiffuseRejected==b.DiffuseRejected && a.m_point==b.m_point && a.m_center==b.m_center);
			assert(a.m_innerRadius==b.m_innerRadius && a.m_outerRadius==b.m_outerRadius
				&& a.m_ambient==b.m_ambient && a.m_diffuse==b.m_diffuse);
		};
		assert(LightCount==other.LightCount && ObjectCenter==other.ObjectCenter
			&& OutputAmbient==other.OutputAmbient && FillIntensity==other.FillIntensity);
		for (int i=0;i<MAX_LIGHTS;++i) {
			input(InputLights[i],other.InputLights[i]);
			assert(OutputLights[i].Direction==other.OutputLights[i].Direction && OutputLights[i].Diffuse==other.OutputLights[i].Diffuse);
		}
		input(FillLight,other.FillLight);
	}
};

void lighting_initialization_generation()
{
	// Deliberately nonzero backing proves construction, not allocator contents,
	// defines inactive slots before native full-member copies evaluate them.
	alignas(LightEnvironmentProbe) std::array<unsigned char,sizeof(LightEnvironmentProbe)> storage;
	storage.fill(0x7e);
	auto *fresh=new(storage.data()) LightEnvironmentProbe;
	fresh->neutral();LightEnvironmentProbe copied(*fresh);copied.neutral();
	LightEnvironmentProbe assigned;assigned=*fresh;assigned.neutral();
	fresh->~LightEnvironmentProbe();
	Matrix3D identity;identity.Make_Identity();
	LightClass directional(LightClass::DIRECTIONAL),point(LightClass::POINT);
	directional.Set_Transform(identity);directional.Set_Ambient(Vector3(0.1f,0.2f,0.3f));directional.Set_Diffuse(Vector3(0.8f,0.9f,1));
	Matrix3D translated=identity;translated.Set_Translation(Vector3(3,4,5));point.Set_Transform(translated);
	point.Set_Ambient(Vector3(0,0,0));point.Set_Diffuse(Vector3(1,1,1));point.Set_Far_Attenuation_Range(1,8);
	copied.Reset(Vector3(1,2,3),Vector3(0.1f,0.2f,0.3f));copied.Add_Light(directional);copied.Add_Light(point);
	assert(copied.Get_Light_Count()==2);copied.Pre_Render_Update(identity);
	assigned=copied;assigned.equivalent(copied);
	copied.Reset(Vector3(4,5,6),Vector3(0.4f,0.5f,0.6f));
	assert(copied.Get_Light_Count()==0);assigned=copied;assigned.equivalent(copied);
	LightEnvironmentProbe reset_copy(copied);reset_copy.equivalent(copied);
}

struct W3DCloneGraphProbeAccess {
	static void fault(int n) { ww3d_clone::Attempt::fault_ordinal=n;ww3d_clone::Attempt::fault_count=0; }
	static unsigned faults() { return ww3d_clone::Attempt::fault_count; }
};
struct DynamicFrameGeneratedProbeAccess {
	static unsigned vertexOffset(const DynamicVBAccessClass& value) { return value.VertexBufferOffset; }
	static unsigned indexOffset(const DynamicIBAccessClass& value) { return value.IndexBufferOffset; }
	static auto captureVertices() { return DynamicVBAccessClass::Capture_Source_Frame(); }
	static auto captureIndices() { return DynamicIBAccessClass::Capture_Source_Frame(); }
	template<class T> static void restoreVertices(T& value) { DynamicVBAccessClass::Restore_Source_Frame(value); }
	template<class T> static void restoreIndices(T& value) { DynamicIBAccessClass::Restore_Source_Frame(value); }
};

struct W3DPropOwnerProbeAccess {
	static std::vector<unsigned char> registrationImage(DX8MeshRendererClass& owner,const std::vector<MeshModelClass*>& models)
	{
		std::vector<unsigned char> result;
		const auto bytes=[&](const auto& value) { const auto *p=reinterpret_cast<const unsigned char*>(&value);result.insert(result.end(),p,p+sizeof(value)); };
		const auto buffer=[&](const void *p,std::size_t count) { const auto *value=static_cast<const unsigned char*>(p);result.insert(result.end(),value,value+count); };
		const auto containers=[&](FVFCategoryList *list) {
			bytes(list);if (!list) return;
			const int count=list->Count();bytes(count);
			FVFCategoryListIterator iterator(list);
			for (;!iterator.Is_Done();iterator.Next()) {
				auto *p=iterator.Peek_Obj();bytes(p);bytes(p->used_indices);bytes(p->index_buffer);
				if (p->index_buffer && p->index_buffer->Type()==BUFFER_TYPE_DX8)
					buffer(static_cast<DX8IndexBufferClass*>(p->index_buffer)->Get_CPU_Index_Buffer(),p->index_buffer->Get_Index_Count()*sizeof(unsigned short));
				if (auto *rigid=dynamic_cast<DX8RigidFVFCategoryContainer*>(p)) {
					bytes(rigid->used_vertices);bytes(rigid->vertex_buffer);
					if (rigid->vertex_buffer && rigid->vertex_buffer->Type()==BUFFER_TYPE_DX8)
						buffer(static_cast<DX8VertexBufferClass*>(rigid->vertex_buffer)->Get_CPU_Vertex_Buffer(),
							std::size_t(rigid->vertex_buffer->Get_Vertex_Count())*rigid->vertex_buffer->FVF_Info().Get_FVF_Size());
				}
				for (auto& categories:p->texture_category_list) {
					const int count=categories.Count();bytes(count);
					TextureCategoryListIterator cats(&categories);
					for (;!cats.Is_Done();cats.Next()) {
						auto *category=cats.Peek_Obj();bytes(category);bytes(category->material);
						const int refs=category->material?category->material->Num_Refs():0;bytes(refs);
						const int polygons=category->PolygonRendererList.Count();bytes(polygons);
						DX8PolygonRendererListIterator polys(&category->PolygonRendererList);
						for (;!polys.Is_Done();polys.Next()) { auto *poly=polys.Peek_Obj();bytes(poly);auto *node=poly->Get_List_Node();bytes(node); }
					}
				}
			}
		};
		const int count=owner.texture_category_container_lists_rigid.Count(),capacity=owner.texture_category_container_lists_rigid.Length();bytes(count);bytes(capacity);
		if (count) { auto *storage=&owner.texture_category_container_lists_rigid[0];bytes(storage); }
		for (int i=0;i<count;++i) containers(owner.texture_category_container_lists_rigid[i]);
		containers(owner.texture_category_container_list_skin);
		for (auto *model:models) { bytes(model->HasBeenInUse);auto *head=model->PolygonRendererList.Peek_Head();bytes(head);auto *node=model->Get_List_Node();bytes(node);const int refs=model->Num_Refs();bytes(refs); }
		return result;
	}
	static bool registered(MeshModelClass *model) { return !model->PolygonRendererList.Is_Empty(); }
	static DX8PolygonRendererClass* polygon(MeshModelClass *model) { return model->PolygonRendererList.Peek_Head(); }
	static auto skinScratch(bool normals=false) { return DX8MeshRendererClass::Peek_Source_Skin_Scratch(normals); }
	static void mutateRegistrationBuffers(DX8MeshRendererClass& owner)
	{
		for (int i=0;i<owner.texture_category_container_lists_rigid.Count();++i) {
			FVFCategoryListIterator it(owner.texture_category_container_lists_rigid[i]);
			for (;!it.Is_Done();it.Next()) {
				auto *container=it.Peek_Obj();++container->used_indices;
				if (container->index_buffer && container->index_buffer->Type()==BUFFER_TYPE_DX8)
					static_cast<DX8IndexBufferClass*>(container->index_buffer)->Get_CPU_Index_Buffer()[0]^=1;
				if (auto *rigid=dynamic_cast<DX8RigidFVFCategoryContainer*>(container)) {
					++rigid->used_vertices;
					if (rigid->vertex_buffer && rigid->vertex_buffer->Type()==BUFFER_TYPE_DX8)
						static_cast<DX8VertexBufferClass*>(rigid->vertex_buffer)->Get_CPU_Vertex_Buffer()[0]^=64;
				}
			}
		}
	}
	static std::vector<unsigned char> propFrameImage(const W3DPropBuffer& owner)
	{
		std::vector<unsigned char> result(sizeof(TProp)*owner.m_numProps+2*sizeof(Bool));
		if (owner.m_numProps) std::memcpy(result.data(),owner.m_props,sizeof(TProp)*owner.m_numProps);
		std::memcpy(result.data()+sizeof(TProp)*owner.m_numProps,&owner.m_anythingChanged,sizeof(Bool));
		std::memcpy(result.data()+sizeof(TProp)*owner.m_numProps+sizeof(Bool),&owner.m_doCull,sizeof(Bool));
		return result;
	}
	static void externalCombo(Animatable3DObjClass& p,bool enabled)
	{ p.CurMotionMode=enabled?Animatable3DObjClass::MULTIPLE_ANIM:Animatable3DObjClass::BASE_POSE;
	  p.ModeCombo.AnimCombo=enabled?reinterpret_cast<HAnimComboClass*>(1):nullptr; }
	static bool externalComboUnchanged(const Animatable3DObjClass& p)
	{ return p.CurMotionMode==Animatable3DObjClass::MULTIPLE_ANIM && p.ModeCombo.AnimCombo==reinterpret_cast<HAnimComboClass*>(1); }
	static void hierarchy(HTreeClass& p)
	{
		p.Free();p.Pivot=new PivotClass[3];p.NumPivots=3;
		for(int i=0;i<3;++i) {
			p.Pivot[i].Index=i;p.Pivot[i].Parent=i?&p.Pivot[i-1]:nullptr;
			p.Pivot[i].BaseTransform.Make_Identity();p.Pivot[i].Transform.Make_Identity();
			std::snprintf(p.Pivot[i].Name,sizeof(p.Pivot[i].Name),"Generated%d",i);
		}
	}
	static PivotClass* parent(HTreeClass& p,int i,PivotClass* next)
	{ auto* old=p.Pivot[i].Parent;p.Pivot[i].Parent=next;return old; }
	static PivotClass* pivot(HTreeClass& p,int i) { return &p.Pivot[i]; }
	static int count(HTreeClass& p,int next) { const int old=p.NumPivots;p.NumPivots=next;return old; }
	static void compare(const HTreeClass& source,const HTreeClass& candidate)
	{
		assert(source.NumPivots==candidate.NumPivots && source.Pivot!=candidate.Pivot);
		for(int i=0;i<source.NumPivots;++i) {
			assert(candidate.Pivot[i].Index==i);
			assert(candidate.Pivot[i].Parent==(i?&candidate.Pivot[i-1]:nullptr));
			assert(!std::strcmp(source.Pivot[i].Name,candidate.Pivot[i].Name));
		}
	}
	static RenderObjClass* replace(CollectionClass& p,int index,RenderObjClass* next)
	{ auto* old=p.SubObjects[index];p.SubObjects[index]=next;return old; }
	static void repeated(CollectionClass& p,RenderObjClass* model,int count)
	{
		assert(p.SubObjects.Resize(count+1));
		for(int i=0;i<count;++i) { model->Add_Ref();assert(p.SubObjects.Add(model)); }
		model->Set_Container(&p);
	}
	static int instances(const W3DPropBuffer& p) { return p.m_numProps; }
	static bool hasSnapPoints(const CollectionClass& p) { return p.SnapPoints!=nullptr; }
	static void proxy(CollectionClass& p)
	{
		Matrix3D matrix(true);matrix.Set_Translation(Vector3(3,4,5));
		assert(p.ProxyList.Add(ProxyClass("GENERATED.PROXY",matrix)));
	}
	static int types(const W3DPropBuffer& p) { return p.m_numPropTypes; }
	static RenderObjClass* instance(const W3DPropBuffer& p,int i) { return p.m_props[i].m_robj; }
	static RenderObjClass* type(const W3DPropBuffer& p,int i) { return p.m_propTypes[i].m_robj; }
	static const TProp& state(const W3DPropBuffer& p,int i) { return p.m_props[i]; }
	static void empty(const W3DPropBuffer& p)
	{
		assert(p.m_initialized && p.m_numProps==0 && p.m_numPropTypes==0);
		assert(p.m_doCull && !p.m_anythingChanged && p.m_light && p.m_propShroudMaterialPass);
		for (const auto& instance:p.m_props)
			assert(!instance.m_robj && instance.propType==-1 && !instance.visible && instance.ss==OBJECTSHROUD_INVALID);
		for (const auto& type:p.m_propTypes) assert(!type.m_robj && type.m_robjName.isEmpty());
	}
};

template<class T> using Ref=ww3d_clone::Ref<T>;
#include "generated_model_packet.inc"

template<class F> void expect_rejection(F action)
{
	bool rejected=false;
	try { action(); } catch (const std::runtime_error&) { rejected=true; }
	assert(rejected);
}

void compare_children(RenderObjClass* source,RenderObjClass* candidate)
{
	assert(source!=candidate && source->Class_ID()==candidate->Class_ID());
	assert(std::strcmp(source->Get_Name(),candidate->Get_Name())==0);
	assert(source->Get_Num_Sub_Objects()==candidate->Get_Num_Sub_Objects());
	if(auto* collection=dynamic_cast<CollectionClass*>(source)) {
		auto* copy=dynamic_cast<CollectionClass*>(candidate);assert(copy);
		assert(collection->Get_Proxy_Count()==copy->Get_Proxy_Count());
		for(int i=0;i<collection->Get_Proxy_Count();++i) {
			ProxyClass a,b;assert(collection->Get_Proxy(i,a) && copy->Get_Proxy(i,b));
			assert(std::strcmp(a.Get_Name(),b.Get_Name())==0 && a.Get_Transform()==b.Get_Transform());
		}
	}
	for (int i=0;i<source->Get_Num_Sub_Objects();++i) {
		Ref<RenderObjClass> a(source->Get_Sub_Object(i)),b(candidate->Get_Sub_Object(i));
		assert(a.get()!=b.get() && a.get()->Class_ID()==b.get()->Class_ID());
		assert(std::strcmp(a.get()->Get_Name(),b.get()->Get_Name())==0);
		assert(a.get()->Get_Container()==source && b.get()->Get_Container()==candidate);
	}
}

void clone_faults(RenderObjClass* source)
{
	{ Ref<RenderObjClass> warm(source->Clone()); }
	const int live=TheMemoryPoolFactory->getLiveAllocationCount();
	const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	const int refs=source->Num_Refs();
	unsigned boundaries=0;
	for (int ordinal=0;ordinal<512;++ordinal) {
		W3DCloneGraphProbeAccess::fault(ordinal);
		RenderObjClass* candidate=nullptr;
		bool rejected=false;
		try { ww3d_clone::Attempt attempt;candidate=source->Clone();attempt.commit(); }
		catch (const std::bad_alloc&) { rejected=true; }
		boundaries=W3DCloneGraphProbeAccess::faults();W3DCloneGraphProbeAccess::fault(-1);
		if (rejected) {
			assert(source->Num_Refs()==refs);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			candidate=source->Clone();
		}
		{ Ref<RenderObjClass> clone(candidate);compare_children(source,candidate); }
		assert(source->Num_Refs()==refs);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
		assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		if (!rejected) break;
		assert(ordinal<511);
	}
	std::printf("prop graph class=%d boundaries=%u\n",source->Class_ID(),boundaries);
}

void graph_generation()
{
	Ref<Null3DObjClass> first(new Null3DObjClass("GENERATED.FIRST"));
	Ref<Null3DObjClass> second(new Null3DObjClass("GENERATED.SECOND"));
	RenderObjClass* lods[]={first.get(),second.get()};
	Ref<HLodClass> hierarchy(new HLodClass("GENERATED.HLOD",lods,2));
	clone_faults(hierarchy.get());
	Ref<CollectionClass> collection(new CollectionClass);
	collection.get()->Set_Name("GENERATED.COLLECTION");
	Ref<RenderObjClass> nested(hierarchy.get()->Clone());
	Ref<Null3DObjClass> sibling(new Null3DObjClass("GENERATED.SIBLING"));
	assert(collection.get()->Add_Sub_Object(nested.get()));
	assert(collection.get()->Add_Sub_Object(sibling.get()));
	W3DPropOwnerProbeAccess::proxy(*collection.get());
	clone_faults(collection.get());
}

void frame_graph_generation()
{
	WW3DAssetManager manager;
	Ref<Null3DObjClass> child(new Null3DObjClass("GENERATED.FRAME.CHILD"));
	RenderObjClass* lods[]={child.get()};
	Ref<HLodClass> root(new HLodClass("GENERATED.FRAME.ROOT",lods,1));
	const int refs=root.get()->Num_Refs(),child_refs=child.get()->Num_Refs();
	const auto transform=root.get()->Get_Transform();
	const auto child_transform=child.get()->Get_Transform();
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	// An externally owned combo cannot prove the bounded sound-free provider
	// contract. Even an invalid raw pointer must reject without dereference.
	W3DPropOwnerProbeAccess::externalCombo(*root.get(),true);
	for (int retry=0;retry<2;++retry) {
		expect_rejection([&] { ww3d_prop::FrameGraph::capture({root.get()}); });
		assert(W3DPropOwnerProbeAccess::externalComboUnchanged(*root.get()));
		assert(root.get()->Num_Refs()==refs && child.get()->Num_Refs()==child_refs);
		assert(root.get()->Get_Transform()==transform && child.get()->Get_Transform()==child_transform);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
	W3DPropOwnerProbeAccess::externalCombo(*root.get(),false);
	{
		auto checkpoint=ww3d_prop::FrameGraph::capture({root.get()});
		Matrix3D moved(true);moved.Set_Translation(Vector3(11,12,13));
		root.get()->Set_Transform(moved);root.get()->Set_Hidden(true);
		child.get()->Set_Transform(moved);child.get()->Set_Hidden(true);
		checkpoint->restore();
		assert(root.get()->Get_Transform()==transform && child.get()->Get_Transform()==child_transform);
		assert(!root.get()->Is_Hidden() && !child.get()->Is_Hidden());
	}
	assert(root.get()->Num_Refs()==refs && child.get()->Num_Refs()==child_refs);
	assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
}

void consumed_node_generation()
{
	{
		WW3DAssetManager manager;
		Ref<Null3DObjClass> first(new Null3DObjClass("GENERATED.FRAME.FIRST"));
		Ref<Null3DObjClass> second(new Null3DObjClass("GENERATED.FRAME.SECOND"));
		DefaultStaticSortListClass sorted;
		sorted.Add_To_List(first.get(),5);sorted.Add_To_List(second.get(),5);
		auto *first_node=first.get()->Get_List_Node(),*second_node=second.get()->Get_List_Node();
		const int first_refs=first.get()->Num_Refs(),second_refs=second.get()->Num_Refs();
		CameraClass camera;RenderInfoClass info(camera);
		for (int retry=0;retry<2;++retry) {
			auto checkpoint=TheDX8MeshRenderer.Capture_Source_Frame(2,&sorted);
			// Baseline units and newly admitted units are consumed in real source
			// order, but cannot be freed until the transaction's disposition.
			sorted.Add_To_List(first.get(),5);sorted.Add_To_List(second.get(),5);
			expect_rejection([&] { sorted.Add_To_List(first.get(),5); });
			sorted.Render_And_Clear(info);
			assert(DX8MeshRendererClass::Source_Frame_Ready_To_Commit(*checkpoint));
			DX8MeshRendererClass::Restore_Source_Frame(*checkpoint);checkpoint.reset();
			assert(first.get()->Get_List_Node()==first_node && second.get()->Get_List_Node()==second_node);
			assert(first_node->Next==second_node && second_node->Prev==first_node);
			assert(first.get()->Num_Refs()==first_refs && second.get()->Num_Refs()==second_refs);
		}
		{
			auto checkpoint=TheDX8MeshRenderer.Capture_Source_Frame(2,&sorted);
			sorted.Add_To_List(first.get(),5);sorted.Render_And_Clear(info);
			assert(DX8MeshRendererClass::Source_Frame_Ready_To_Commit(*checkpoint));
			DX8MeshRendererClass::Commit_Source_Frame(*checkpoint);checkpoint.reset();
			assert(!first.get()->Get_List_Node() && !second.get()->Get_List_Node());
			assert(first.get()->Num_Refs()==first_refs-1 && second.get()->Num_Refs()==second_refs-1);
		}
	}
	TheDX8MeshRenderer.Shutdown();
	assert(MultiListNodeClass::Release_Empty_Blocks());
}

void prop_checkpoint_generation()
{
	WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);
	Ref<Null3DObjClass> leaf(new Null3DObjClass("GENERATED.PROP.FRAME"));
	manager.Add_Prototype(new PrimitivePrototypeClass(leaf.get()));
	W3DPropBuffer props;props.addProp(11,{2,3,4},0,1,AsciiString("GENERATED.PROP.FRAME"));
	auto *instance=W3DPropOwnerProbeAccess::instance(props,0);
	const auto baseline=W3DPropOwnerProbeAccess::propFrameImage(props);
	const auto transform=instance->Get_Transform();const int refs=instance->Num_Refs();
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	for (int ordinal=0;ordinal<5;++ordinal) {
		W3DCloneGraphProbeAccess::fault(ordinal);bool rejected=false;
		try { auto checkpoint=props.captureSourceFrame(); } catch (const std::bad_alloc&) { rejected=true; }
		W3DCloneGraphProbeAccess::fault(-1);assert(rejected==(ordinal<4));
		assert(W3DPropOwnerProbeAccess::propFrameImage(props)==baseline && instance->Num_Refs()==refs);
		assert(instance->Get_Transform()==transform);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
	for (int retry=0;retry<2;++retry) {
		auto checkpoint=props.captureSourceFrame();
		assert(props.updatePropPosition(11,{8,9,10},0.5f,-2));props.notifyShroudChanged();
		props.restoreSourceFrame(*checkpoint);checkpoint.reset();
		assert(W3DPropOwnerProbeAccess::propFrameImage(props)==baseline && instance->Get_Transform()==transform);
		assert(instance->Num_Refs()==refs);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
}

void sorting_buffer_checkpoint_generation()
{
	WW3DAssetManager manager;
	VertexBufferClass *vertex=nullptr;IndexBufferClass *index=nullptr;
	{
		DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,3);
		expect_rejection([&] { DX8Wrapper::Set_Vertex_Buffer(access); });
		DynamicVBAccessClass::WriteLockClass lock(&access);
		vertex=access.Peek_Buffer();lock.Get_Formatted_Vertex_Array()[0].x=17;
	}
	{
		DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,3);
		expect_rejection([&] { DX8Wrapper::Set_Index_Buffer(access,0); });
		DynamicIBAccessClass::WriteLockClass lock(&access);
		index=access.Peek_Buffer();lock.Get_Index_Array()[0]=23;
	}
	const auto vertex_baseline=static_cast<SortingVertexBufferClass*>(vertex)->VertexBuffer[0];
	for (int retry=0;retry<2;++retry) {
		auto vertices=DynamicFrameGeneratedProbeAccess::captureVertices();
		auto indices=DynamicFrameGeneratedProbeAccess::captureIndices();
		{
			DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,6000);
			DynamicVBAccessClass::WriteLockClass lock(&access);
			assert(access.Peek_Buffer()!=vertex);lock.Get_Formatted_Vertex_Array()[0].x=99;
		}
		{
			DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,6000);
			DynamicIBAccessClass::WriteLockClass lock(&access);
			assert(access.Peek_Buffer()!=index);lock.Get_Index_Array()[0]=99;
		}
		DynamicFrameGeneratedProbeAccess::restoreIndices(*indices);DynamicFrameGeneratedProbeAccess::restoreVertices(*vertices);
		indices.reset();vertices.reset();
		assert(vertex->Num_Refs()==1 && index->Num_Refs()==1);
		assert(std::memcmp(&static_cast<SortingVertexBufferClass*>(vertex)->VertexBuffer[0],&vertex_baseline,sizeof(vertex_baseline))==0);
		{
			DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,3);
			assert(access.Peek_Buffer()==vertex && DynamicFrameGeneratedProbeAccess::vertexOffset(access)==3);
		}
		{
			IndexBufferClass::WriteLockClass lock(index);assert(lock.Get_Index_Array()[0]==23);
			DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,3);
			assert(access.Peek_Buffer()==index && DynamicFrameGeneratedProbeAccess::indexOffset(access)==3);
		}
		// Return to the same declared offset baseline after the verification
		// accesses, so the second rollback observes exact identical input.
		DynamicVBAccessClass::_Reset(false);DynamicIBAccessClass::_Reset(false);
		{ DynamicVBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,dynamic_fvf_type,3); }
		{ DynamicIBAccessClass access(BUFFER_TYPE_DYNAMIC_SORTING,3); }
	}
	DynamicVBAccessClass::_Deinit();DynamicIBAccessClass::_Deinit();
}

void material_checkpoint_generation()
{
	std::vector<char> bytes(65536);RAMFileClass file(bytes.data(),static_cast<int>(bytes.size()));assert(file.Open(FileClass::WRITE));
	ChunkSaveClass writer(&file);make_mesh(writer,false,false,false,0,false,false,0,false,false,"FRAME","MATERIAL");file.Close();
	WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);assert(manager.Load_3D_Assets(file));
	Ref<RenderObjClass> object(manager.Create_Render_Obj("FRAME.MATERIAL"));
	auto *mesh=dynamic_cast<MeshClass*>(object.get());assert(mesh);
	auto *material=mesh->Peek_Model()->Peek_Single_Material();assert(material);
	const auto release=[](TextureMapperClass *p) { p->Release_Ref(); };
	const auto own=[&](TextureMapperClass *p) { return std::unique_ptr<TextureMapperClass,decltype(release)>(p,release); };
	std::array<std::unique_ptr<TextureMapperClass,decltype(release)>,10> mappers{{
		own(new LinearOffsetTextureMapperClass({1,2},{0,0},false,{1,1},0)),
		own(new ScreenMapperClass({1,2},{0,0},false,{1,1},0)),
		own(new GridTextureMapperClass(10,1,4,0,0)),
		own(new RotateTextureMapperClass(1,{0,0},{1,1},0)),
		own(new SineLinearOffsetTextureMapperClass({1,1,0},{1,2,0},{1,1},0)),
		own(new StepLinearOffsetTextureMapperClass({0.1f,0.2f},2,false,{1,1},0)),
		own(new ZigZagLinearOffsetTextureMapperClass({1,2},1,{1,1},0)),
		own(new EdgeMapperClass(0)),own(new RandomTextureMapperClass(10,{1,1},0)),
		own(new BumpEnvTextureMapperClass(1,1,{1,2},{0,0},false,{1,1},0))
	}};
	const auto old_sync=WW3D::Get_Sync_Time();WW3D::Sync(old_sync+1000);
	zh::renderer::RecordingGpuDevice device;zh::original_runtime::OriginalGpuEdge edge(device);
	DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
	for (const auto& mapper:mappers) {
		material->Set_Mapper(mapper.get());
		const auto crc=material->Get_CRC();const auto opacity=material->Get_Opacity();Vector3 diffuse;material->Get_Diffuse(&diffuse);
		const int refs=material->Num_Refs(),mapper_refs=mapper->Num_Refs();
		Matrix4x4 expected;
		for (int retry=0;retry<2;++retry) {
			auto checkpoint=ww3d_prop::FrameGraph::capture({mesh});
			auto random=ww3d_clone::capture_mapper_random();
			material->Set_Opacity(0.125f);material->Set_Diffuse(0.25f,0.5f,0.75f);
			(void)material->Get_CRC();
			Matrix4x4 calculated;mapper->Calculate_Texture_Matrix(calculated);mapper->Apply(0);
			if (!retry) expected=calculated;else assert(calculated==expected);
			checkpoint->restore();checkpoint.reset();
			assert(material->Get_CRC()==crc && material->Get_Opacity()==opacity);
			Vector3 restored;material->Get_Diffuse(&restored);assert(restored==diffuse);
			assert(material->Num_Refs()==refs && mapper->Num_Refs()==mapper_refs);
			auto restored_random=ww3d_clone::capture_mapper_random();
			assert(random.Get_Float()==restored_random.Get_Float());
		}
	}
	WW3D::Sync(old_sync);
}

void registration_batch_generation(bool shared_material,bool skin=false)
{
	{
		std::vector<char> bytes(65536);
		RAMFileClass file(bytes.data(),static_cast<int>(bytes.size()));assert(file.Open(FileClass::WRITE));
		ChunkSaveClass writer(&file);
		for (const char *name:{"FIRST","SECOND","THIRD"}) make_mesh(writer,false,false,skin,0,false,false,0,false,false,"FRAME",name);
		file.Close();WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);assert(manager.Load_3D_Assets(file));
		Ref<RenderObjClass> first(manager.Create_Render_Obj("FRAME.FIRST")),second(manager.Create_Render_Obj("FRAME.SECOND")),third(manager.Create_Render_Obj("FRAME.THIRD"));
		auto *a=dynamic_cast<MeshClass*>(first.get()),*b=dynamic_cast<MeshClass*>(second.get()),*c=dynamic_cast<MeshClass*>(third.get());assert(a && b && c);
		std::vector<MeshModelClass*> models{a->Peek_Model(),b->Peek_Model(),c->Peek_Model()};
		// Native material matching includes texture-mapper identity in its CRC.
		// Independent SCREEN mappers intentionally stay distinct; an explicitly
		// shared material must coalesce. First establish both via ordinary source
		// registration, then require the strong batch to preserve that decision.
		if (shared_material) {
			models[1]->Set_Single_Material(models[0]->Peek_Single_Material());
			models[2]->Set_Single_Material(models[0]->Peek_Single_Material());
		}
		assert((models[0]->Peek_Single_Material()->Get_CRC()==models[1]->Peek_Single_Material()->Get_CRC())==shared_material);
		assert((models[1]->Peek_Single_Material()->Get_CRC()==models[2]->Peek_Single_Material()->Get_CRC())==shared_material);
		zh::renderer::RecordingGpuDevice device;
		zh::original_runtime::OriginalGpuEdge edge(device);
		TheDX8MeshRenderer.Init();
		for (auto *model:models) TheDX8MeshRenderer.Register_Mesh_Type(model);
		auto *native_a=W3DPropOwnerProbeAccess::polygon(models[0]),*native_b=W3DPropOwnerProbeAccess::polygon(models[1]),*native_c=W3DPropOwnerProbeAccess::polygon(models[2]);
		assert(native_a && native_b && native_c);
		assert((native_a->Get_Texture_Category()==native_b->Get_Texture_Category())==shared_material);
		assert((native_b->Get_Texture_Category()==native_c->Get_Texture_Category())==shared_material);
		for (auto i=models.rbegin();i!=models.rend();++i) TheDX8MeshRenderer.Unregister_Mesh_Type(*i);
		TheDX8MeshRenderer.Shutdown();TheDX8MeshRenderer.Init();
		TheDX8MeshRenderer.Prepare_Mesh_Batch_Strong({models[0]});
		// Warm the exact offside candidate families; ordinary unregister keeps
		// native append counters, which become the explicit sibling baseline.
		TheDX8MeshRenderer.Prepare_Mesh_Batch_Strong({models[1],models[2]});
		TheDX8MeshRenderer.Unregister_Mesh_Type(models[2]);TheDX8MeshRenderer.Unregister_Mesh_Type(models[1]);
		const auto baseline=W3DPropOwnerProbeAccess::registrationImage(TheDX8MeshRenderer,models);
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		expect_rejection([&] { TheDX8MeshRenderer.Prepare_Mesh_Batch_Strong({models[1],models[1]}); });
		assert(W3DPropOwnerProbeAccess::registrationImage(TheDX8MeshRenderer,models)==baseline);
		unsigned boundaries=0;
		for (int ordinal=0;ordinal<256;++ordinal) {
			W3DCloneGraphProbeAccess::fault(ordinal);bool rejected=false;
			try { TheDX8MeshRenderer.Prepare_Mesh_Batch_Strong({models[1],models[2]}); }
			catch (const std::bad_alloc&) { rejected=true; }
			boundaries=W3DCloneGraphProbeAccess::faults();W3DCloneGraphProbeAccess::fault(-1);
			if (!rejected) break;
			assert(W3DPropOwnerProbeAccess::registrationImage(TheDX8MeshRenderer,models)==baseline);
			assert(!W3DPropOwnerProbeAccess::registered(models[1]) && !W3DPropOwnerProbeAccess::registered(models[2]));
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			assert(device.resource_counts().total()==0);
			assert(ordinal<255);
		}
		assert(boundaries>20);
		auto *pa=W3DPropOwnerProbeAccess::polygon(models[0]),*pb=W3DPropOwnerProbeAccess::polygon(models[1]),*pc=W3DPropOwnerProbeAccess::polygon(models[2]);
		assert(pa && pb && pc);
		assert((pa->Get_Texture_Category()==pb->Get_Texture_Category())==shared_material);
		assert((pb->Get_Texture_Category()==pc->Get_Texture_Category())==shared_material);
		if (skin) assert(pb->Get_Vertex_Offset()==0 && pc->Get_Vertex_Offset()==0);
		else assert(pb->Get_Vertex_Offset()+3==pc->Get_Vertex_Offset());
		const auto published=W3DPropOwnerProbeAccess::registrationImage(TheDX8MeshRenderer,models);
		const auto original_scratch=W3DPropOwnerProbeAccess::skinScratch();
		const auto original_normals=W3DPropOwnerProbeAccess::skinScratch(true);
		for (int retry=0;retry<2;++retry) {
			auto checkpoint=TheDX8MeshRenderer.Capture_Source_Frame(2);
			if (skin) {
				const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
				expect_rejection([&] { TheDX8MeshRenderer.Prepare_Source_Skin_Scratch(4); });
				expect_rejection([&] { TheDX8MeshRenderer.Prepare_Source_Skin_Scratch(65536); });
				TheDX8MeshRenderer.Prepare_Source_Skin_Scratch(3);
				W3DPropOwnerProbeAccess::skinScratch().first[0].X=17;
				W3DPropOwnerProbeAccess::skinScratch(true).first[0].X=23;
				assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			}
			W3DPropOwnerProbeAccess::mutateRegistrationBuffers(TheDX8MeshRenderer);
			DX8MeshRendererClass::Restore_Source_Frame(*checkpoint);checkpoint.reset();
			assert(W3DPropOwnerProbeAccess::registrationImage(TheDX8MeshRenderer,models)==published);
			assert(W3DPropOwnerProbeAccess::skinScratch()==original_scratch && W3DPropOwnerProbeAccess::skinScratch(true)==original_normals);
		}
		if (skin) {
			auto checkpoint=TheDX8MeshRenderer.Capture_Source_Frame(2);
			TheDX8MeshRenderer.Prepare_Source_Skin_Scratch(3);
			W3DPropOwnerProbeAccess::skinScratch().first[0].X=17;
			W3DPropOwnerProbeAccess::skinScratch(true).first[0].X=23;
			DX8MeshRendererClass::Commit_Source_Frame(*checkpoint);checkpoint.reset();
			assert(W3DPropOwnerProbeAccess::skinScratch().second==3 && W3DPropOwnerProbeAccess::skinScratch(true).second==3);
			assert(W3DPropOwnerProbeAccess::skinScratch().first[0].X==17 && W3DPropOwnerProbeAccess::skinScratch(true).first[0].X==23);
		}
		std::printf("prop frame registration shared=%d skin=%d boundaries=%u\n",shared_material,skin,boundaries);
		TheDX8MeshRenderer.Unregister_Mesh_Type(models[2]);TheDX8MeshRenderer.Unregister_Mesh_Type(models[1]);TheDX8MeshRenderer.Unregister_Mesh_Type(models[0]);
		TheDX8MeshRenderer.Shutdown();
	}
	assert(MultiListNodeClass::Release_Empty_Blocks());
}

void hierarchy_generation()
{
	HTreeClass source;W3DPropOwnerProbeAccess::hierarchy(source);
	{ HTreeClass warm(source); }
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	for(int ordinal=0;ordinal<8;++ordinal) {
		W3DCloneGraphProbeAccess::fault(ordinal);bool rejected=false;
		try { ww3d_clone::Attempt attempt;HTreeClass candidate(source);W3DPropOwnerProbeAccess::compare(source,candidate);attempt.commit(); }
		catch(const std::bad_alloc&) { rejected=true; }
		W3DCloneGraphProbeAccess::fault(-1);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		if(!rejected) break;
		{ HTreeClass retry(source);W3DPropOwnerProbeAccess::compare(source,retry); }
		assert(ordinal<7);
	}
	PivotClass foreign;
	auto* parent=W3DPropOwnerProbeAccess::parent(source,2,&foreign);
	bool rejected=false;try { HTreeClass invalid(source); } catch(const std::runtime_error&) { rejected=true; }
	assert(rejected);W3DPropOwnerProbeAccess::parent(source,2,parent);
	parent=W3DPropOwnerProbeAccess::parent(source,2,W3DPropOwnerProbeAccess::pivot(source,2));
	rejected=false;try { WW3DAssetManager manager;ww3d_prop::Audit audit(manager);audit.hierarchy(&source); }
	catch(const std::runtime_error&) { rejected=true; }
	assert(rejected);W3DPropOwnerProbeAccess::parent(source,2,parent);
	WW3DAssetManager manager;
	const auto audit=[&] { ww3d_prop::Audit check(manager);check.hierarchy(&source); };
	W3DPropOwnerProbeAccess::pivot(source,1)->Index=7;
	expect_rejection(audit);W3DPropOwnerProbeAccess::pivot(source,1)->Index=1;
	W3DPropOwnerProbeAccess::pivot(source,1)->BaseTransform[0][0]=std::numeric_limits<Real>::infinity();
	expect_rejection(audit);W3DPropOwnerProbeAccess::pivot(source,1)->BaseTransform[0][0]=1;
	const int count=W3DPropOwnerProbeAccess::count(source,-1);
	expect_rejection(audit);W3DPropOwnerProbeAccess::count(source,count);
	W3DPropOwnerProbeAccess::count(source,static_cast<int>(64u*1024u*1024u/sizeof(PivotClass)+1));
	expect_rejection(audit);W3DPropOwnerProbeAccess::count(source,count);
}

void constructor_generation()
{
	{ W3DPropBuffer warm;W3DPropOwnerProbeAccess::empty(warm); }
	const int live=TheMemoryPoolFactory->getLiveAllocationCount();
	const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	for (int ordinal=0;ordinal<3;++ordinal) {
		W3DCloneGraphProbeAccess::fault(ordinal);
		bool rejected=false;
		try { W3DPropBuffer candidate;W3DPropOwnerProbeAccess::empty(candidate); }
		catch (const std::bad_alloc&) { rejected=true; }
		W3DCloneGraphProbeAccess::fault(-1);
		assert(rejected==(ordinal<2));
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
		assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		{ W3DPropBuffer retry;W3DPropOwnerProbeAccess::empty(retry);retry.clearAllProps(); }
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
		assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
}

template<class Base> class ForbiddenPrototype final:public Base {
public:
	unsigned calls=0;
	const char* Get_Name() const override { return "GENERATED.FORBIDDEN"; }
	RenderObjClass* Create() override { ++calls;throw std::runtime_error("forbidden constructor reached"); }
};
class ForbiddenAggregate final:public AggregatePrototypeClass {
public:
	ForbiddenAggregate():AggregatePrototypeClass(new AggregateDefClass) {}
	unsigned calls=0;
	const char* Get_Name() const override { return "GENERATED.FORBIDDEN"; }
	RenderObjClass* Create() override { ++calls;throw std::runtime_error("forbidden constructor reached"); }
};
template<class T> void forbidden_kind()
{
	WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);
	auto* prototype=new T;manager.Add_Prototype(prototype);
	W3DPropBuffer props;
	const int live=TheMemoryPoolFactory->getLiveAllocationCount();
	const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	expect_rejection([&] { props.addProp(1,{0,0,0},0,1,AsciiString("GENERATED.FORBIDDEN")); });
	assert(!prototype->calls && !W3DPropOwnerProbeAccess::types(props) && !W3DPropOwnerProbeAccess::instances(props));
	assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
}

void graph_admission_generation()
{
	WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);
	expect_rejection([&] { ww3d_prop::Attempt missing(nullptr); });
	Ref<Null3DObjClass> leaf(new Null3DObjClass("GENERATED.LEAF"));
	Ref<CollectionClass> a(new CollectionClass),b(new CollectionClass);
	a.get()->Set_Name("GENERATED.A");b.get()->Set_Name("GENERATED.B");
	assert(a.get()->Add_Sub_Object(leaf.get()));assert(b.get()->Add_Sub_Object(leaf.get()));
	{ ww3d_prop::Audit audit(manager);audit.render(a.get()); }
	auto* saved=W3DPropOwnerProbeAccess::replace(*a.get(),0,a.get());
	expect_rejection([&] { ww3d_prop::Audit audit(manager);audit.render(a.get()); });
	W3DPropOwnerProbeAccess::replace(*a.get(),0,b.get());
	auto* saved_b=W3DPropOwnerProbeAccess::replace(*b.get(),0,a.get());
	expect_rejection([&] { ww3d_prop::Audit audit(manager);audit.render(a.get()); });
	W3DPropOwnerProbeAccess::replace(*a.get(),0,saved);
	W3DPropOwnerProbeAccess::replace(*b.get(),0,saved_b);
	// A repeated non-owning graph edge is legal; each existing owning slot
	// retains its native independent ref. No node is shared between destructors.
	assert(a.get()->Add_Sub_Object(leaf.get()));
	{ ww3d_prop::Audit audit(manager);audit.render(a.get()); }
	expect_rejection([&] { ww3d_prop::Audit audit(manager);audit.named("GENERATED.MISSING"); });
	{ ww3d_prop::Audit audit(manager);audit.named("GENERATED.MISSING",true); }
	std::vector<CollectionClass*> chain;
	for (unsigned i=0;i<256;++i) {
		auto* node=new CollectionClass;node->Set_Name("GENERATED.DEPTH");
		assert(node->Add_Sub_Object(chain.empty()?static_cast<RenderObjClass*>(leaf.get()):chain.back()));
		chain.push_back(node);
	}
	{ ww3d_prop::Audit audit(manager);audit.render(chain[254]); }
	expect_rejection([&] { ww3d_prop::Audit audit(manager);audit.render(chain[255]); });
	for (auto i=chain.rbegin();i!=chain.rend();++i) (*i)->Release_Ref();
	Ref<CollectionClass> wide(new CollectionClass);wide.get()->Set_Name("GENERATED.NODE.BOUND");
	W3DPropOwnerProbeAccess::repeated(*wide.get(),leaf.get(),65535);
	{ ww3d_prop::Audit audit(manager);audit.render(wide.get()); }
	assert(wide.get()->Add_Sub_Object(leaf.get()));
	expect_rejection([&] { ww3d_prop::Audit audit(manager);audit.render(wide.get()); });
}

void legacy_generation()
{
	WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);
	Ref<Null3DObjClass> first(new Null3DObjClass("GENERATED.LEGACY.FIRST"));
	Ref<Null3DObjClass> second(new Null3DObjClass("GENERATED.LEGACY.SECOND"));
	manager.Add_Prototype(new PrimitivePrototypeClass(first.get()));
	manager.Add_Prototype(new PrimitivePrototypeClass(second.get()));
	DistLODNodeDefStruct nodes[2];
	nodes[0].Name=const_cast<char*>("GENERATED.LEGACY.FIRST");nodes[1].Name=const_cast<char*>("GENERATED.LEGACY.SECOND");
	nodes[0].ResDownDist=20;nodes[0].ResUpDist=10;nodes[1].ResDownDist=50;nodes[1].ResUpDist=40;
	const DistLODDefClass direct("GENERATED.LEGACY.DIRECT",2,nodes);
	Ref<DistLODClass> legacy(new DistLODClass(direct));clone_faults(legacy.get());
	manager.Add_Prototype(new DistLODPrototypeClass(new DistLODDefClass("GENERATED.LEGACY.CONVERTED",2,nodes)));
	{ ww3d_prop::Attempt attempt(&manager);Ref<RenderObjClass> warm(manager.Create_Render_Obj("GENERATED.LEGACY.CONVERTED"));attempt.commit(); }
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	const int first_refs=first.get()->Num_Refs(),second_refs=second.get()->Num_Refs();
	for (int ordinal=0;ordinal<128;++ordinal) {
		W3DCloneGraphProbeAccess::fault(ordinal);bool rejected=false;
		try {
			ww3d_prop::Attempt attempt(&manager);Ref<RenderObjClass> converted(manager.Create_Render_Obj("GENERATED.LEGACY.CONVERTED"));
			assert(converted.get()->Class_ID()==RenderObjClass::CLASSID_HLOD);attempt.commit();
		} catch (const std::bad_alloc&) { rejected=true; }
		W3DCloneGraphProbeAccess::fault(-1);
		assert(first.get()->Num_Refs()==first_refs && second.get()->Num_Refs()==second_refs);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		if (!rejected) break;
		{ ww3d_prop::Attempt attempt(&manager);Ref<RenderObjClass> retry(manager.Create_Render_Obj("GENERATED.LEGACY.CONVERTED"));attempt.commit(); }
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		assert(ordinal<127);
	}
}

void definition_generation()
{
	WW3DAssetManager manager;manager.Set_WW3D_Load_On_Demand(false);
	for(const char* name:{"PROP_HMODEL.FIRST","PROP_HMODEL.SECOND"}) {
		Ref<Null3DObjClass> leaf(new Null3DObjClass(name));manager.Add_Prototype(new PrimitivePrototypeClass(leaf.get()));
	}
	std::vector<char> bytes(65536);
	RAMFileClass file(bytes.data(),static_cast<int>(bytes.size()));assert(file.Open(FileClass::WRITE));
	ChunkSaveClass writer(&file);
	make_mesh(writer,false,false,false,0,false,false,0,false,false,"TEST","TRIANGLE");
	make_hierarchy(writer,true);make_hlod(writer,false);
	assert(writer.Begin_Chunk(W3D_CHUNK_HMODEL));
	W3dHModelHeaderStruct header{};header.Version=W3D_CURRENT_HMODEL_VERSION;
	std::strcpy(header.Name,"PROP_HMODEL");std::strcpy(header.HierarchyName,"TESTTREE");header.NumConnections=2;
	chunk(writer,W3D_CHUNK_HMODEL_HEADER,header);
	W3dHModelNodeStruct node{};std::strcpy(node.RenderObjName,"FIRST");node.PivotIdx=1;chunk(writer,W3D_CHUNK_NODE,node);
	std::strcpy(node.RenderObjName,"SECOND");node.PivotIdx=2;chunk(writer,W3D_CHUNK_NODE,node);
	assert(writer.End_Chunk());
	assert(writer.Begin_Chunk(W3D_CHUNK_COLLECTION));
	W3dCollectionHeaderStruct collection{};collection.Version=W3D_CURRENT_COLLECTION_VERSION;
	std::strcpy(collection.Name,"PROP_COLLECTION");collection.RenderObjectCount=2;chunk(writer,W3D_CHUNK_COLLECTION_HEADER,collection);
	const char child0[]="PROP_HMODEL",child1[]="TEST.HLOD";
	chunk(writer,W3D_CHUNK_COLLECTION_OBJ_NAME,child0);chunk(writer,W3D_CHUNK_COLLECTION_OBJ_NAME,child1);
	W3dPlaceholderStruct placeholder{};placeholder.version=W3D_CURRENT_PLACEHOLDER_VERSION;
	placeholder.transform[0][0]=placeholder.transform[1][1]=placeholder.transform[2][2]=1;
	placeholder.transform[3][0]=3;placeholder.transform[3][1]=4;placeholder.transform[3][2]=5;
	const char proxy_name[]="GENERATED.DEFINITION.PROXY";placeholder.name_len=sizeof(proxy_name)-1;
	assert(writer.Begin_Chunk(W3D_CHUNK_PLACEHOLDER));
	assert(writer.Write(&placeholder,sizeof(placeholder))==sizeof(placeholder));
	assert(writer.Write(proxy_name,placeholder.name_len)==placeholder.name_len);assert(writer.End_Chunk());
	W3dVectorStruct snap{};snap.X=7;snap.Y=8;snap.Z=9;chunk(writer,W3D_CHUNK_POINTS,snap);
	assert(writer.End_Chunk());
	file.Close();assert(manager.Load_3D_Assets(file));
	for(const char* name:{"TEST.TRIANGLE","TEST.HLOD","PROP_HMODEL","PROP_COLLECTION"}) {
		{ ww3d_prop::Attempt attempt(&manager);Ref<RenderObjClass> warm(manager.Create_Render_Obj(name));assert(warm.get());attempt.commit(); }
		if(!std::strcmp(name,"PROP_COLLECTION")) {
			Ref<RenderObjClass> resource(manager.Create_Render_Obj(name));
			auto* composite=dynamic_cast<CollectionClass*>(resource.get());assert(composite && composite->Get_Proxy_Count()==1);
			ProxyClass proxy;assert(composite->Get_Proxy(0,proxy));
			assert(!std::strcmp(proxy.Get_Name(),proxy_name) && proxy.Get_Transform().Get_Translation()==Vector3(3,4,5));
			assert(W3DPropOwnerProbeAccess::hasSnapPoints(*composite));clone_faults(composite);
		}
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for(int ordinal=0;ordinal<256;++ordinal) {
			W3DCloneGraphProbeAccess::fault(ordinal);bool rejected=false;
			try { ww3d_prop::Attempt attempt(&manager);Ref<RenderObjClass> model(manager.Create_Render_Obj(name));assert(model.get());attempt.commit(); }
			catch(const std::bad_alloc&) { rejected=true; }
			W3DCloneGraphProbeAccess::fault(-1);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			if(!rejected) break;
			{ ww3d_prop::Attempt attempt(&manager);Ref<RenderObjClass> retry(manager.Create_Render_Obj(name));attempt.commit(); }
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			assert(ordinal<255);
		}
	}
	// Exact native provider withdrawal must release definition storage without
	// stealing the independent refs held by existing composite/sibling owners.
	Ref<RenderObjClass> surviving_collection(manager.Create_Render_Obj("PROP_COLLECTION"));
	Ref<RenderObjClass> surviving_mesh(manager.Create_Render_Obj("TEST.TRIANGLE"));
	auto* composite=dynamic_cast<CollectionClass*>(surviving_collection.get());assert(composite);
	Ref<RenderObjClass> child(composite->Get_Sub_Object(0));
	const int child_refs=child.get()->Num_Refs(),mesh_refs=surviving_mesh.get()->Num_Refs();
	manager.Remove_Prototype("PROP_COLLECTION");
	assert(!manager.Find_Prototype("PROP_COLLECTION") && manager.Find_Prototype("TEST.TRIANGLE"));
	assert(child.get()->Num_Refs()==child_refs && surviving_mesh.get()->Num_Refs()==mesh_refs);
	assert(composite->Get_Proxy_Count()==1 && W3DPropOwnerProbeAccess::hasSnapPoints(*composite));
	manager.Free_Assets();
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	manager.Free_Assets();
	assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	assert(child.get()->Num_Refs()==child_refs && composite->Get_Proxy_Count()==1 && W3DPropOwnerProbeAccess::hasSnapPoints(*composite));
	{ Ref<RenderObjClass> retry(composite->Clone());compare_children(composite,retry.get()); }
}

struct AdmissionFixture { unsigned calls=0;bool reject=false; };
void admission_callback(void* p)
{ auto& owner=*static_cast<AdmissionFixture*>(p);++owner.calls;if(owner.reject) throw std::runtime_error("generated lifecycle rejection"); }
void registration_generation()
{
	AdmissionFixture owner,foreign;
	preflightClientTerrainRemovalAdmission();
	assert(!installClientTerrainRemovalAdmission(nullptr,admission_callback));
	assert(!installClientTerrainRemovalAdmission(&owner,nullptr));
	const auto token=installClientTerrainRemovalAdmission(&owner,admission_callback);assert(token);
	assert(!installClientTerrainRemovalAdmission(&owner,admission_callback));
	assert(!installClientTerrainRemovalAdmission(&foreign,admission_callback));
	assert(!removeClientTerrainRemovalAdmission(&foreign,token));
	assert(!removeClientTerrainRemovalAdmission(&owner,token+1));
	owner.reject=true;expect_rejection([&] { preflightSubsystemResetAdmission(); });
	assert(owner.calls==1);
	owner.reject=false;preflightClientTerrainRemovalAdmission();assert(owner.calls==2);
	assert(removeClientTerrainRemovalAdmission(&owner,token));
	assert(!removeClientTerrainRemovalAdmission(&owner,token));
	preflightClientTerrainRemovalAdmission();assert(owner.calls==2);
	const auto next=installClientTerrainRemovalAdmission(&owner,admission_callback);assert(next>token);
	assert(!removeClientTerrainRemovalAdmission(&owner,token));assert(removeClientTerrainRemovalAdmission(&owner,next));
}

void admission_generation()
{
	WW3DAssetManager manager;
	manager.Set_WW3D_Load_On_Demand(false);
	for (int i=0;i<65;++i) {
		char name[32];std::snprintf(name,sizeof(name),"GENERATED.TYPE.%02d",i);
		Ref<Null3DObjClass> model(new Null3DObjClass(name));
		manager.Add_Prototype(new PrimitivePrototypeClass(model.get()));
	}
	AsciiString first("GENERATED.TYPE.00"),second("GENERATED.TYPE.01");
	Coord3D position={3,4,5};
	W3DPropBuffer props;
	props.addProp(11,position,0.125f,0,first);
	assert(W3DPropOwnerProbeAccess::types(props)==1 && W3DPropOwnerProbeAccess::instances(props)==1);
	auto* sibling=W3DPropOwnerProbeAccess::instance(props,0);
	auto* sibling_type=W3DPropOwnerProbeAccess::type(props,0);
	for (Real invalid:{std::numeric_limits<Real>::quiet_NaN(),std::numeric_limits<Real>::infinity(),-std::numeric_limits<Real>::infinity()}) {
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		expect_rejection([&] { props.addProp(12,position,0,invalid,first); });
		expect_rejection([&] { props.addProp(12,position,invalid,1,first); });
		Coord3D malformed=position;malformed.x=invalid;
		expect_rejection([&] { props.addProp(12,malformed,0,1,first); });
		expect_rejection([&] { props.updatePropPosition(11,position,0,invalid); });
		assert(W3DPropOwnerProbeAccess::instance(props,0)==sibling && W3DPropOwnerProbeAccess::types(props)==1 && W3DPropOwnerProbeAccess::instances(props)==1);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
	// Warm the exact candidate type/instance families before immediate residuals.
	{ W3DPropBuffer warm;warm.addProp(12,position,0.25f,-1,second); }
	const int live=TheMemoryPoolFactory->getLiveAllocationCount();
	const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	for (int ordinal=0;ordinal<64;++ordinal) {
		W3DCloneGraphProbeAccess::fault(ordinal);
		bool rejected=false;
		try { props.addProp(12,position,0.25f,-1,second); }
		catch (const std::bad_alloc&) { rejected=true; }
		W3DCloneGraphProbeAccess::fault(-1);
		assert(W3DPropOwnerProbeAccess::instance(props,0)==sibling);
		assert(W3DPropOwnerProbeAccess::type(props,0)==sibling_type);
		if (!rejected) break;
		assert(W3DPropOwnerProbeAccess::types(props)==1 && W3DPropOwnerProbeAccess::instances(props)==1);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
		assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		{ W3DPropBuffer retry;retry.addProp(12,position,0.25f,-1,second); }
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
		assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		assert(ordinal<63);
	}
	assert(W3DPropOwnerProbeAccess::types(props)==2 && W3DPropOwnerProbeAccess::instances(props)==2);
	for (int i=2;i<64;++i) {
		char name[32];std::snprintf(name,sizeof(name),"GENERATED.TYPE.%02d",i);
		props.addProp(20+i,position,0,1,AsciiString(name));
	}
	assert(W3DPropOwnerProbeAccess::types(props)==64);
	bool rejected=false;
	try { props.addProp(84,position,0,1,AsciiString("GENERATED.TYPE.64")); }
	catch (const std::runtime_error&) { rejected=true; }
	assert(rejected && W3DPropOwnerProbeAccess::types(props)==64 && W3DPropOwnerProbeAccess::instances(props)==64);
	while (W3DPropOwnerProbeAccess::instances(props)<4000) props.addProp(99,position,0,1,first);
	rejected=false;
	try { props.addProp(99,position,0,1,first); } catch (const std::runtime_error&) { rejected=true; }
	assert(rejected && W3DPropOwnerProbeAccess::instances(props)==4000);
	props.clearAllProps();W3DPropOwnerProbeAccess::empty(props);
	props.addProp(11,position,0,1,first);
	assert(W3DPropOwnerProbeAccess::types(props)==1 && W3DPropOwnerProbeAccess::instances(props)==1);
	props.addProp(11,position,0,1,first);props.removeProp(11);
	assert(!W3DPropOwnerProbeAccess::instance(props,0) && !W3DPropOwnerProbeAccess::instance(props,1));
	assert(!props.updatePropPosition(11,position,0,1));
	props.addProp(11,position,0,-1,first);
	Coord3D moved={7,8,9};assert(props.updatePropPosition(11,moved,0.5f,-2));
	assert(W3DPropOwnerProbeAccess::state(props,2).location.x==7 && W3DPropOwnerProbeAccess::state(props,2).bounds.Center.X==7);
	props.notifyShroudChanged();
	assert(W3DPropOwnerProbeAccess::state(props,2).ss==OBJECTSHROUD_CLEAR);
}

int main()
{
	try {
		initMemoryManager();WW3D::Set_Thumbnail_Enabled(false);
		// OriginalGpuEdge uses this public process-global source baseline too.
		// Establish its default map nodes before measuring two-generation leaks.
		DX8Wrapper::Reset_Source_State();
		const int baseline=TheMemoryPoolFactory->getLiveAllocationCount();
		const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for (int generation=0;generation<2;++generation) {
			lighting_initialization_generation();
			graph_generation();hierarchy_generation();constructor_generation();admission_generation();legacy_generation();definition_generation();graph_admission_generation();registration_generation();frame_graph_generation();consumed_node_generation();prop_checkpoint_generation();sorting_buffer_checkpoint_generation();material_checkpoint_generation();registration_batch_generation(false);registration_batch_generation(true);registration_batch_generation(true,true);
			forbidden_kind<ForbiddenPrototype<RingPrototypeClass>>();
			forbidden_kind<ForbiddenPrototype<SpherePrototypeClass>>();
			forbidden_kind<ForbiddenPrototype<DazzlePrototypeClass>>();
			forbidden_kind<ForbiddenAggregate>();
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==baseline);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		shutdownMemoryManager();
		std::puts("original-rendering runtime provider=GeneralsMD strong terrain prop owner");
		return 0;
	} catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what());return 1; }
}
