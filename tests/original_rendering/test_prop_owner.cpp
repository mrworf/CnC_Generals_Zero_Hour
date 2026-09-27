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
#include <vector>
#undef assert
#define assert(value) do { if (!(value)) throw std::runtime_error("generated prop invariant: " #value); } while (false)

struct W3DCloneGraphProbeAccess {
	static void fault(int n) { ww3d_clone::Attempt::fault_ordinal=n;ww3d_clone::Attempt::fault_count=0; }
	static unsigned faults() { return ww3d_clone::Attempt::fault_count; }
};

struct W3DPropOwnerProbeAccess {
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
		const int baseline=TheMemoryPoolFactory->getLiveAllocationCount();
		const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for (int generation=0;generation<2;++generation) {
			graph_generation();hierarchy_generation();constructor_generation();admission_generation();legacy_generation();definition_generation();graph_admission_generation();registration_generation();
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
