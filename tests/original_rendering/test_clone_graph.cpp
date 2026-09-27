#include "mesh.h"
#include "meshmdl.h"
#include "meshmatdesc.h"
#include "matinfo.h"
#include "texture.h"
#include "mapper.h"
#include "aabtree.h"
#include "clone_graph.h"
#include "ww3d.h"
#include "ini.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "Common/GameMemory.h"
#undef min
#undef max
#undef assert
#define assert(value) do { if (!(value)) throw std::runtime_error("generated clone invariant: " #value); } while(false)

struct W3DCloneGraphProbeAccess {
	static MeshModelClass* initialize(MeshClass* p) { assert(!p->Model);p->Model=new MeshModelClass;return p->Model; }
	static void fault(int n) { ww3d_clone::Attempt::fault_ordinal=n;ww3d_clone::Attempt::fault_count=0; }
	static unsigned faults() { return ww3d_clone::Attempt::fault_count; }
	static MaterialInfoClass* material(MeshModelClass* p) { return p->MatInfo; }
	static MeshMatDescClass* descriptor(MeshModelClass* p) { return p->DefMatDesc; }
	static void texture_count(MaterialInfoClass* p,int n) { p->Textures.Set_Active(n); }
	static int texture_capacity(MaterialInfoClass* p) { return p->Textures.Length(); }
	static AABTreeClass* tree(MeshModelClass* p) { return p->CullTree; }
	static const void* geometry(MeshModelClass* p) { return p->Vertex; }
	static int geometry_refs(MeshModelClass* p) { return p->Vertex->Num_Refs(); }
	static void fill_geometry(MeshModelClass* p) {
		p->Poly->Get_Array()[0]={0,1,2};p->Poly->Get_Array()[1]={1,3,2};
		p->Vertex->Get_Array()[0]=Vector3(0,0,0);p->Vertex->Get_Array()[1]=Vector3(1,0,0);
		p->Vertex->Get_Array()[2]=Vector3(0,1,0);p->Vertex->Get_Array()[3]=Vector3(1,1,0);
	}
	static void alternate(MeshModelClass* p) { p->AlternateMatDesc=new MeshMatDescClass(*p->DefMatDesc); }
	static void select_alternate(MeshModelClass* p) { p->CurMatDesc=p->AlternateMatDesc; }
	static bool alternate_distinct(MeshModelClass* a,MeshModelClass* b) {
		return (!a->AlternateMatDesc && !b->AlternateMatDesc) ||
			(a->AlternateMatDesc && b->AlternateMatDesc && a->AlternateMatDesc!=b->AlternateMatDesc);
	}
	static bool tree_matches(MeshModelClass* a,MeshModelClass* b) {
		if (!a->CullTree) return !b->CullTree;
		return b->CullTree && a->CullTree!=b->CullTree && b->CullTree->Mesh==b &&
			a->CullTree->NodeCount==b->CullTree->NodeCount && a->CullTree->PolyCount==b->CullTree->PolyCount &&
			std::memcmp(a->CullTree->Nodes,b->CullTree->Nodes,a->CullTree->NodeCount*sizeof(*a->CullTree->Nodes))==0 &&
			std::memcmp(a->CullTree->PolyIndices,b->CullTree->PolyIndices,a->CullTree->PolyCount*sizeof(uint32))==0;
	}
};

template<class T> struct Ref {
	T* p;
	explicit Ref(T* value):p(value) {}
	~Ref() { if (p) p->Release_Ref(); }
	Ref(const Ref&)=delete;
	Ref& operator=(const Ref&)=delete;
};

void assert_random_unchanged(Random4Class before)
{
	auto after=ww3d_clone::capture_mapper_random();
	for (int i=0;i<8;++i) assert(before()==after());
}

void generation(unsigned generation,bool extended)
{
	Ref<MeshClass> source(new MeshClass);
	auto* model=W3DCloneGraphProbeAccess::initialize(source.p);model->Reset(2,4,extended ? 4 : 1);
	W3DCloneGraphProbeAccess::fill_geometry(model);
	Ref<TextureClass> texture(new TextureClass("GENERATED.TGA"));
	Ref<VertexMaterialClass> material(new VertexMaterialClass);
	material.p->Set_Name("GENERATED.MATERIAL");
	Ref<TextureMapperClass> screen(new ScreenMapperClass(Vector2(0,0),Vector2(0,0),false,Vector2(1,1),0));
	material.p->Set_Mapper(screen.p,0);
	Ref<TextureMapperClass> random(new RandomTextureMapperClass(0,Vector2(1,1),1));
	if (extended) material.p->Set_Mapper(random.p,1);
	auto* info=W3DCloneGraphProbeAccess::material(model);
	info->Add_Vertex_Material(material.p);info->Add_Texture(texture.p);
	model->Set_Single_Material(material.p,0);model->Set_Single_Texture(texture.p,0,0);
	if (extended) {
		for (int pass=1;pass<4;++pass) {
			for (int vertex=0;vertex<4;++vertex) model->Set_Material(vertex,material.p,pass);
			for (int poly=0;poly<2;++poly) { model->Set_Texture(poly,texture.p,pass,0);model->Set_Texture(poly,texture.p,pass,1);model->Set_Shader(poly,ShaderClass(0),pass); }
		}
		W3DCloneGraphProbeAccess::alternate(model);source.p->Generate_Culling_Tree();
		// Exercise the direct cull-tree copy constructor, not only geometry's
		// guarded default-construction/assignment route.
		for (int ordinal=0;ordinal<3;++ordinal) {
			const int live=TheMemoryPoolFactory->getLiveAllocationCount();
			const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
			W3DCloneGraphProbeAccess::fault(ordinal);
			AABTreeClass* candidate=nullptr;
			try { candidate=new AABTreeClass(*W3DCloneGraphProbeAccess::tree(model)); }
			catch (const std::bad_alloc&) { assert(ordinal<2); }
			W3DCloneGraphProbeAccess::fault(-1);
			if (candidate) candidate->Release_Ref();
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
	}
	// Warm every pool/storage family before measuring immediate ownership counts.
	{ Ref<MeshClass> warm(static_cast<MeshClass*>(source.p->Clone()));warm.p->Make_Unique(true); }
	const int texture_refs=texture.p->Num_Refs(),material_refs=material.p->Num_Refs();
	const int geometry_refs=W3DCloneGraphProbeAccess::geometry_refs(model);
	unsigned boundaries=0;
	for (int ordinal=0;ordinal<512;++ordinal) {
		Ref<MeshClass> target(static_cast<MeshClass*>(source.p->Clone()));
		const int live=TheMemoryPoolFactory->getLiveAllocationCount();
		const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		const int model_refs=model->Num_Refs();
		auto random_before=ww3d_clone::capture_mapper_random();
		W3DCloneGraphProbeAccess::fault(ordinal);
		bool rejected=false;
		try { target.p->Make_Unique(true); } catch (const std::bad_alloc&) { rejected=true; }
		boundaries=W3DCloneGraphProbeAccess::faults();W3DCloneGraphProbeAccess::fault(-1);
		if (rejected) {
			assert(target.p->Peek_Model()==model && model->Num_Refs()==model_refs);
			assert(texture.p->Num_Refs()==texture_refs && material.p->Num_Refs()==material_refs);
			assert(W3DCloneGraphProbeAccess::geometry_refs(model)==geometry_refs);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			assert_random_unchanged(random_before);
			target.p->Make_Unique(true);
		}
		auto* clone=target.p->Peek_Model();
		assert(clone!=model && W3DCloneGraphProbeAccess::geometry(clone)==W3DCloneGraphProbeAccess::geometry(model));
		assert(std::memcmp(clone->Get_Vertex_Array(),model->Get_Vertex_Array(),4*sizeof(Vector3))==0);
		assert(clone->Peek_Single_Texture(0,0)==texture.p);
		assert(clone->Peek_Single_Material(0)!=material.p);
		assert(W3DCloneGraphProbeAccess::alternate_distinct(model,clone));
		assert(W3DCloneGraphProbeAccess::tree_matches(model,clone));
		if (!rejected) break;
		assert(ordinal<511);
	}
	assert(texture.p->Num_Refs()==texture_refs && material.p->Num_Refs()==material_refs);
	assert(W3DCloneGraphProbeAccess::geometry_refs(model)==geometry_refs);
	std::printf("clone generation=%u extended=%u boundaries=%u\n",generation,extended,boundaries);
}

void mapper_control(TextureMapperClass* value)
{
	Ref<TextureMapperClass> mapper(value);
	Ref<VertexMaterialClass> source(new VertexMaterialClass);
	source.p->Set_Name("GENERATED.MATERIAL.WITH.NAME.COPY");
	source.p->Set_Opacity(0.375f);source.p->Set_Mapper(mapper.p,0);
	{ Ref<VertexMaterialClass> warm(source.p->Clone()); }
	for (bool copy_constructor : {false,true}) {
		for (int ordinal=0;ordinal<32;++ordinal) {
			const int live=TheMemoryPoolFactory->getLiveAllocationCount();
			const int raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
			const int refs=mapper.p->Num_Refs();
			auto random=ww3d_clone::capture_mapper_random();
			W3DCloneGraphProbeAccess::fault(ordinal);
			VertexMaterialClass* candidate=nullptr;
			bool rejected=false;
			try { candidate=copy_constructor ? new VertexMaterialClass(*source.p) : source.p->Clone(); }
			catch (const std::bad_alloc&) { rejected=true; }
			W3DCloneGraphProbeAccess::fault(-1);
			if (rejected) {
				assert(mapper.p->Num_Refs()==refs);
				assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
				assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
				assert_random_unchanged(random);
				candidate=copy_constructor ? new VertexMaterialClass(*source.p) : source.p->Clone();
			}
			{
				Ref<VertexMaterialClass> clone(candidate);
				assert(clone.p->Get_Opacity()==source.p->Get_Opacity());
				assert(std::strcmp(clone.p->Get_Name(),source.p->Get_Name())==0);
				assert(clone.p->Peek_Mapper()!=mapper.p);
				assert(clone.p->Peek_Mapper()->Mapper_ID()==mapper.p->Mapper_ID());
				assert(clone.p->Peek_Mapper()->Get_Stage()==mapper.p->Get_Stage());
				if (mapper.p->Mapper_ID()==TextureMapperClass::MAPPER_ID_RANDOM) {
					random.Get_Float();random.Get_Float();random.Get_Float();
					assert_random_unchanged(random);
				}
			}
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			if (!rejected) break;
			assert(ordinal<31);
		}
	}
}

void alternate_array_control()
{
	Ref<MeshClass> source(new MeshClass);
	auto* model=W3DCloneGraphProbeAccess::initialize(source.p);model->Reset(2,4,1);
	W3DCloneGraphProbeAccess::fill_geometry(model);
	Ref<VertexMaterialClass> material(new VertexMaterialClass);
	Ref<TextureClass> texture(new TextureClass("GENERATED.ALTERNATE.TGA"));
	auto* info=W3DCloneGraphProbeAccess::material(model);
	info->Add_Vertex_Material(material.p);info->Add_Texture(texture.p);
	model->Set_Single_Material(material.p,0);model->Set_Single_Texture(texture.p,0,0);
	W3DCloneGraphProbeAccess::alternate(model);W3DCloneGraphProbeAccess::select_alternate(model);
	for (int vertex=0;vertex<4;++vertex) model->Set_Material(vertex,material.p,0);
	for (int poly=0;poly<2;++poly) model->Set_Texture(poly,texture.p,0,0);
	{ Ref<MeshClass> warm(static_cast<MeshClass*>(source.p->Clone()));warm.p->Make_Unique(true); }
	for (int ordinal=0;ordinal<128;++ordinal) {
		Ref<MeshClass> target(static_cast<MeshClass*>(source.p->Clone()));
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		const int texture_refs=texture.p->Num_Refs(),material_refs=material.p->Num_Refs();
		W3DCloneGraphProbeAccess::fault(ordinal);
		bool rejected=false;
		try { target.p->Make_Unique(true); } catch (const std::bad_alloc&) { rejected=true; }
		W3DCloneGraphProbeAccess::fault(-1);
		if (rejected) {
			assert(target.p->Peek_Model()==model);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
			assert(texture.p->Num_Refs()==texture_refs && material.p->Num_Refs()==material_refs);
			target.p->Make_Unique(true);
		}
		auto* clone=target.p->Peek_Model();
		assert(clone->Has_Material_Array(0) && clone->Has_Texture_Array(0,0));
		assert(clone->Peek_Material(0,0)!=material.p && clone->Peek_Texture(0,0,0)==texture.p);
		if (!rejected) break;
		assert(ordinal<127);
	}
}

void all_mappers()
{
	INIClass settings;char section[]="Generated";
	mapper_control(new ScaleTextureMapperClass(settings,section,0));
	mapper_control(new LinearOffsetTextureMapperClass(settings,section,0));
	mapper_control(new GridTextureMapperClass(settings,section,0));
	mapper_control(new RotateTextureMapperClass(settings,section,0));
	mapper_control(new SineLinearOffsetTextureMapperClass(settings,section,0));
	mapper_control(new StepLinearOffsetTextureMapperClass(settings,section,0));
	mapper_control(new ZigZagLinearOffsetTextureMapperClass(settings,section,0));
	mapper_control(new ClassicEnvironmentMapperClass(0));
	mapper_control(new EnvironmentMapperClass(0));
	mapper_control(new EdgeMapperClass(settings,section,0));
	mapper_control(new WSClassicEnvironmentMapperClass(settings,section,0));
	mapper_control(new WSEnvironmentMapperClass(settings,section,0));
	mapper_control(new GridClassicEnvironmentMapperClass(settings,section,0));
	mapper_control(new GridEnvironmentMapperClass(settings,section,0));
	mapper_control(new ScreenMapperClass(settings,section,0));
	mapper_control(new RandomTextureMapperClass(settings,section,0));
	mapper_control(new BumpEnvTextureMapperClass(settings,section,0));
	mapper_control(new GridWSClassicEnvironmentMapperClass(settings,section,0));
	mapper_control(new GridWSEnvironmentMapperClass(settings,section,0));
}

void negative_and_empty()
{
	Ref<MaterialInfoClass> empty(new MaterialInfoClass);
	Ref<MaterialInfoClass> empty_clone(empty.p->Clone());
	assert(empty_clone.p->Vertex_Material_Count()==0 && empty_clone.p->Texture_Count()==0);
	MaterialRemapperClass remapper(empty.p,empty_clone.p);
	MeshMatDescClass descriptor;
	MeshMatDescClass descriptor_copy(descriptor);
	remapper.Remap_Mesh(&descriptor,&descriptor_copy);
	for (int bad : {-1,5}) {
		descriptor.Set_Pass_Count(bad);
		bool rejected=false;
		try { MeshMatDescClass candidate(descriptor); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected);descriptor.Set_Pass_Count(1);
	}
	for (int bad : {-1,1}) {
		W3DCloneGraphProbeAccess::texture_count(empty.p,bad);
		bool rejected=false;
		try { Ref<MaterialInfoClass> candidate(empty.p->Clone()); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected);W3DCloneGraphProbeAccess::texture_count(empty.p,0);
	}
	bool rejected=false;
	try { MaterialRemapperClass candidate(nullptr,empty.p); } catch (const std::runtime_error&) { rejected=true; }
	assert(rejected);
	{
		Ref<VertexMaterialClass> foreign(new VertexMaterialClass);
		descriptor.Set_Single_Material(foreign.p,0);
		Ref<VertexMaterialClass> owned(new VertexMaterialClass);
		empty.p->Add_Vertex_Material(owned.p);empty_clone.p->Add_Vertex_Material(owned.p);
		MaterialRemapperClass checked(empty.p,empty_clone.p);
		bool failed=false;
		try { checked.Remap_Mesh(&descriptor,&descriptor_copy); } catch (const std::runtime_error&) { failed=true; }
		assert(failed && descriptor_copy.Peek_Single_Material(0)==nullptr);
		descriptor.Set_Single_Material(nullptr,0);
	}
	assert(ww3d_clone::Attempt::extent(64*1024*1024,1)==64u*1024u*1024u);
	for (int bad : {-1,64*1024*1024+1}) {
		bool failed=false;try { ww3d_clone::Attempt::extent(bad,1); } catch (const std::runtime_error&) { failed=true; }
		assert(failed);
	}
	{
		auto random=ww3d_clone::capture_mapper_random();
		ww3d_clone::Attempt budget;
		ww3d_clone::Attempt::reserve(64*1024*1024,1);
		bool failed=false;
		try { ww3d_clone::Attempt::reserve(1,1); } catch (const std::runtime_error&) { failed=true; }
		assert(failed);assert_random_unchanged(random);budget.commit();
	}
	for (std::size_t element : {std::size_t(0),std::size_t(64u*1024u*1024u+1)}) {
		bool failed=false;try { ww3d_clone::Attempt::extent(1,element); } catch (const std::runtime_error&) { failed=true; }
		assert(failed);
	}
}

int main()
{
	try {
		initMemoryManager();
		WW3D::Set_Thumbnail_Enabled(false);
		const int baseline=TheMemoryPoolFactory->getLiveAllocationCount();
		const int raw_baseline=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for (unsigned i=0;i<2;++i) {
			generation(i,false);generation(i,true);all_mappers();negative_and_empty();alternate_array_control();
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==baseline);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw_baseline);
		}
		shutdownMemoryManager();
		std::puts("original-rendering runtime provider=GeneralsMD WW3D2 strong cloned render graph");
		return 0;
	} catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what());return 1; }
}
