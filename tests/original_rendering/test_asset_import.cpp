#include "PreRTS.h"
#include "asset_import.h"
#include "assetmgr.h"
#include "texture.h"
#include "proto.h"
#include "boxrobj.h"
#include "mesh.h"
#include "meshmdl.h"
#include "meshmatdesc.h"
#include "aabtree.h"
#include "ringobj.h"
#include "sphereobj.h"
#include "vertmaterial.h"
#include "clone_graph.h"
#include "ini.h"
#include "xstraw.h"
#include "htree.h"
#include "htreemgr.h"
#include "hanim.h"
#include "w3d_util.h"
#include "ww3d.h"
#include "statistics.h"
#include "dx8wrapper.h"
#include "chunkio.h"
#include "RAMFILE.H"
#include "ffactory.h"
#include "Common/GameMemory.h"
#include "Common/FileSystem.h"
#include "Common/NameKeyGenerator.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
#include <array>
#include <cstring>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstdlib>
#include <limits>
#undef assert
#define assert(value) do { if (!(value)) throw std::runtime_error("generated import invariant: " #value); } while (false)
#include "generated_model_packet.inc"

struct W3DAssetImportProbeAccess {
	static void fault(int value) { ww3d_import::Attempt::fault_ordinal=value;ww3d_import::Attempt::fault_count=0; }
	static unsigned faults() { return ww3d_import::Attempt::fault_count; }
	static void reserve_prototypes(WW3DAssetManager& manager) { assert(manager.Prototypes.Resize(65536)); }
	static void seed_tree(WW3DAssetManager& manager,HTreeClass* tree) {
		auto& trees=manager.HTreeManager;assert(trees.NumTrees<trees.MAX_TREES);
		StringClass name(tree->Get_Name());_strlwr(name.Peek_Buffer());trees.TreeHash.Insert(name,tree);trees.TreePtr[trees.NumTrees++]=tree;
	}
	static void seed_texture(WW3DAssetManager& manager,const char* key,TextureClass* texture) {
		manager.TextureHash.Insert(StringClass(key),texture);texture->Add_Ref();
	}
};
struct W3DCloneGraphProbeAccess {
	static AABTreeClass* tree(MeshModelClass* model) { return model->CullTree; }
	static std::vector<unsigned char> bytes(AABTreeClass* tree) {
		assert(tree && tree->Mesh && tree->NodeCount>1 && tree->PolyCount==8);
		std::vector<unsigned char> out(sizeof(int)*2+tree->NodeCount*sizeof(*tree->Nodes)+tree->PolyCount*sizeof(uint32));
		unsigned char* cursor=out.data();std::memcpy(cursor,&tree->NodeCount,sizeof(int));cursor+=sizeof(int);
		std::memcpy(cursor,&tree->PolyCount,sizeof(int));cursor+=sizeof(int);
		std::memcpy(cursor,tree->Nodes,tree->NodeCount*sizeof(*tree->Nodes));cursor+=tree->NodeCount*sizeof(*tree->Nodes);
		std::memcpy(cursor,tree->PolyIndices,tree->PolyCount*sizeof(uint32));return out;
	}
};
struct W3DTerrainPropLifecycleProbeAccess {
	static void generation(zh::original_runtime::OriginalGpuEdge& edge,std::uint64_t value) { edge.generation_=value; }
};

struct Packet {
	std::array<unsigned char,16384> data{};
	int bytes=0;
	explicit Packet(bool duplicate=false,bool unknown=false,const char* first="GENERATED.FIRST",const char* second="GENERATED.SECOND",bool graph=false,float adaptive_scale=1)
	{
		RAMFileClass file(data.data(),data.size());assert(file.Open(FileClass::WRITE));ChunkSaveClass writer(&file);
		for (int i=0;i<2;++i) {
			W3dBoxStruct box{};std::strcpy(box.Name,i && !duplicate ? second : first);
			box.Extent={1,2,3};assert(writer.Begin_Chunk(W3D_CHUNK_BOX));
			assert(writer.Write(&box,sizeof(box))==sizeof(box));assert(writer.End_Chunk());
		}
		if (graph) {
			make_mesh(writer,false,false,false,2,true,false,0,false,false,"GENERATED","MESH1");
			make_hierarchy(writer,false);make_hlod(writer,false,false,false,false,"GENERATED.HLOD1","GENERATED.MESH1");
			assert(writer.Begin_Chunk(W3D_CHUNK_HMODEL));
			W3dHModelHeaderStruct model{};model.Version=W3D_CURRENT_HMODEL_VERSION;
			std::strcpy(model.Name,"G.HMODEL");std::strcpy(model.HierarchyName,"TESTTREE");model.NumConnections=1;
			chunk(writer,W3D_CHUNK_HMODEL_HEADER,model);
			W3dHModelNodeStruct node{};std::strcpy(node.RenderObjName,"MESH1");chunk(writer,W3D_CHUNK_NODE,node);
			W3dVectorStruct snap{};snap.X=1;snap.Y=2;snap.Z=3;chunk(writer,W3D_CHUNK_POINTS,snap);assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_COLLECTION));
			W3dCollectionHeaderStruct collection{};collection.Version=W3D_CURRENT_COLLECTION_VERSION;
			std::strcpy(collection.Name,"G.COLLECTION");collection.RenderObjectCount=2;chunk(writer,W3D_CHUNK_COLLECTION_HEADER,collection);
			const char child0[]="G.HMODEL",child1[]="GENERATED.HLOD1";
			chunk(writer,W3D_CHUNK_COLLECTION_OBJ_NAME,child0);chunk(writer,W3D_CHUNK_COLLECTION_OBJ_NAME,child1);
			W3dPlaceholderStruct placeholder{};placeholder.version=W3D_CURRENT_PLACEHOLDER_VERSION;
			placeholder.transform[0][0]=placeholder.transform[1][1]=placeholder.transform[2][2]=1;
			const char proxy[]="GENERATED.PROXY";placeholder.name_len=sizeof(proxy)-1;
			assert(writer.Begin_Chunk(W3D_CHUNK_PLACEHOLDER));assert(writer.Write(&placeholder,sizeof(placeholder))==sizeof(placeholder));
			assert(writer.Write(proxy,placeholder.name_len)==placeholder.name_len);assert(writer.End_Chunk());
			chunk(writer,W3D_CHUNK_POINTS,snap);assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_LODMODEL));
			W3dLODModelHeaderStruct lod{};std::strcpy(lod.Name,"GENERATED.LOD");lod.NumLODs=1;chunk(writer,W3D_CHUNK_LODMODEL_HEADER,lod);
			W3dLODStruct lod_node{};std::strcpy(lod_node.RenderObjName,"GENERATED.MESH1");lod_node.LODMax=100;chunk(writer,W3D_CHUNK_LOD,lod_node);assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_AGGREGATE));
			W3dAggregateHeaderStruct aggregate{};aggregate.Version=W3D_MAKE_VERSION(1,2);std::strcpy(aggregate.Name,"GENERATED.AGG");chunk(writer,W3D_CHUNK_AGGREGATE_HEADER,aggregate);
			W3dAggregateInfoStruct info{};std::strcpy(info.BaseModelName,"GENERATED.HLOD1");info.SubobjectCount=1;
			W3dAggregateSubobjectStruct subobject{};std::strcpy(subobject.SubobjectName,"GENERATED.FIRST");std::strcpy(subobject.BoneName,"ROOT");
			assert(writer.Begin_Chunk(W3D_CHUNK_AGGREGATE_INFO));assert(writer.Write(&info,sizeof(info))==sizeof(info));assert(writer.Write(&subobject,sizeof(subobject))==sizeof(subobject));assert(writer.End_Chunk());
			W3dAggregateMiscInfo misc{};chunk(writer,W3D_CHUNK_AGGREGATE_CLASS_INFO,misc);assert(writer.End_Chunk());
			W3dNullObjectStruct null{};std::strcpy(null.Name,"GENERATED.NULL");chunk(writer,W3D_CHUNK_NULL_OBJECT,null);
			W3dRingStruct ring{};std::strcpy(ring.Name,"GENERATED.RING");ring.DefaultAlpha=1;ring.Extent={1,1,1};
			assert(writer.Begin_Chunk(W3D_CHUNK_RING));chunk(writer,1,ring);
			PrimitiveAnimationChannelClass<float>::KeyClass primitive_key(0.5f,0);
			assert(writer.Begin_Chunk(3));assert(writer.Begin_Chunk(0x03150809));assert(writer.Begin_Micro_Chunk(1));
			assert(writer.Write(&primitive_key,sizeof(primitive_key))==sizeof(primitive_key));assert(writer.End_Micro_Chunk());assert(writer.End_Chunk());assert(writer.End_Chunk());assert(writer.End_Chunk());
			W3dSphereStruct sphere{};std::strcpy(sphere.Name,"GENERATED.SPHERE");sphere.DefaultAlpha=1;sphere.Extent={1,1,1};
			assert(writer.Begin_Chunk(W3D_CHUNK_SPHERE));chunk(writer,1,sphere);assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_DAZZLE));const char dazzle[]="GENERATED.DAZZLE",type[]="GENERATED.DEFAULT";
			chunk(writer,W3D_CHUNK_DAZZLE_NAME,dazzle);chunk(writer,W3D_CHUNK_DAZZLE_TYPENAME,type);assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_HIERARCHY));
			W3dHierarchyStruct header{};header.Version=W3D_MAKE_VERSION(3,0);std::strcpy(header.Name,"GENROOT");header.NumPivots=1;
			assert(writer.Begin_Chunk(W3D_CHUNK_HIERARCHY_HEADER));assert(writer.Write(&header,sizeof(header))==sizeof(header));assert(writer.End_Chunk());
			W3dPivotStruct pivot{};std::strcpy(pivot.Name,"ROOT");pivot.ParentIdx=0xffffffffu;pivot.Rotation.Q[3]=1;
			assert(writer.Begin_Chunk(W3D_CHUNK_PIVOTS));assert(writer.Write(&pivot,sizeof(pivot))==sizeof(pivot));assert(writer.End_Chunk());assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_ANIMATION));
			W3dAnimHeaderStruct animation{};animation.Version=W3D_MAKE_VERSION(3,0);std::strcpy(animation.Name,"ANIM1");std::strcpy(animation.HierarchyName,"GENROOT");animation.NumFrames=1;animation.FrameRate=30;
			assert(writer.Begin_Chunk(W3D_CHUNK_ANIMATION_HEADER));assert(writer.Write(&animation,sizeof(animation))==sizeof(animation));assert(writer.End_Chunk());
			W3dAnimChannelStruct motion{};motion.VectorLen=1;motion.Flags=ANIM_CHANNEL_X;motion.Data[0]=3;
			assert(writer.Begin_Chunk(W3D_CHUNK_ANIMATION_CHANNEL));assert(writer.Write(&motion,sizeof(motion))==sizeof(motion));assert(writer.End_Chunk());
			W3dBitChannelStruct bit{};bit.Flags=BIT_CHANNEL_VIS;bit.DefaultVal=1;bit.Data[0]=1;
			assert(writer.Begin_Chunk(W3D_CHUNK_BIT_CHANNEL));assert(writer.Write(&bit,sizeof(bit))==sizeof(bit));assert(writer.End_Chunk());assert(writer.End_Chunk());
			for (unsigned flavor=0;flavor<ANIM_FLAVOR_VALID;++flavor) {
				assert(writer.Begin_Chunk(W3D_CHUNK_COMPRESSED_ANIMATION));
				W3dCompressedAnimHeaderStruct compressed{};compressed.Version=W3D_MAKE_VERSION(3,0);std::strcpy(compressed.Name,flavor ? "AD1" : "TC1");
				std::strcpy(compressed.HierarchyName,"GENROOT");compressed.NumFrames=1;compressed.FrameRate=30;compressed.Flavor=flavor;
				assert(writer.Begin_Chunk(W3D_CHUNK_COMPRESSED_ANIMATION_HEADER));assert(writer.Write(&compressed,sizeof(compressed))==sizeof(compressed));assert(writer.End_Chunk());
				assert(writer.Begin_Chunk(W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL));
				if (!flavor) {
					W3dTimeCodedAnimChannelStruct channel{};channel.NumTimeCodes=1;channel.VectorLen=1;channel.Flags=ANIM_CHANNEL_X;
					float value=3;assert(writer.Write(&channel,sizeof(channel))==sizeof(channel));assert(writer.Write(&value,sizeof(value))==sizeof(value));
				} else {
					W3dAdaptiveDeltaAnimChannelStruct channel{};channel.NumFrames=1;channel.VectorLen=1;channel.Flags=ANIM_CHANNEL_X;channel.Scale=adaptive_scale;
					float value=3;std::memcpy(channel.Data,&value,sizeof(value));assert(writer.Write(&channel,sizeof(channel))==sizeof(channel));
				}
				assert(writer.End_Chunk());
				W3dTimeCodedBitChannelStruct visibility{};visibility.NumTimeCodes=1;visibility.Flags=BIT_CHANNEL_VIS;visibility.DefaultVal=1;visibility.Data[0]=W3D_TIMECODED_BIT_MASK;
				assert(writer.Begin_Chunk(W3D_CHUNK_COMPRESSED_BIT_CHANNEL));assert(writer.Write(&visibility,sizeof(visibility))==sizeof(visibility));assert(writer.End_Chunk());assert(writer.End_Chunk());
			}
			assert(writer.Begin_Chunk(W3D_CHUNK_MORPH_ANIMATION));
			W3dMorphAnimHeaderStruct morph{};morph.Version=W3D_MAKE_VERSION(3,0);std::strcpy(morph.Name,"MORPH1");std::strcpy(morph.HierarchyName,"GENROOT");morph.FrameCount=1;morph.FrameRate=30;morph.ChannelCount=1;
			assert(writer.Begin_Chunk(W3D_CHUNK_MORPHANIM_HEADER));assert(writer.Write(&morph,sizeof(morph))==sizeof(morph));assert(writer.End_Chunk());
			assert(writer.Begin_Chunk(W3D_CHUNK_MORPHANIM_CHANNEL));
			const char pose[]="GENROOT.ANIM1";assert(writer.Begin_Chunk(W3D_CHUNK_MORPHANIM_POSENAME));assert(writer.Write(pose,sizeof(pose))==sizeof(pose));assert(writer.End_Chunk());
			W3dMorphAnimKeyStruct key{};assert(writer.Begin_Chunk(W3D_CHUNK_MORPHANIM_KEYDATA));assert(writer.Write(&key,sizeof(key))==sizeof(key));assert(writer.End_Chunk());assert(writer.End_Chunk());
			unsigned pivot_channel=0;assert(writer.Begin_Chunk(W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA));assert(writer.Write(&pivot_channel,sizeof(pivot_channel))==sizeof(pivot_channel));assert(writer.End_Chunk());assert(writer.End_Chunk());
		}
		if (unknown) { assert(writer.Begin_Chunk(0x7ffffffe));assert(writer.End_Chunk()); }
		bytes=file.Seek(0,SEEK_CUR);file.Close();
	}
};
struct Provider : FileFactoryClass {
	struct File : RAMFileClass {
		Provider& provider;
		File(Provider& p,Packet& packet) : RAMFileClass(packet.data.data(),packet.bytes),provider(p) {}
		int Read(void* buffer,int size) override
		{
			++provider.reads;
			if (provider.cut==provider.reads) throw std::bad_alloc();
			if (provider.manager && provider.baseline) assert(std::memcmp(provider.manager,provider.baseline->data(),provider.baseline->size())==0);
			if (provider.nested && !provider.did_nested && provider.manager->Find_Prototype("GENERATED.FIRST")) {
				provider.did_nested=true;
				assert(ww3d_import::Attempt::load(*provider.manager,"GENERATED.CHILD.w3d"));
				assert(provider.manager->Find_Prototype("GENERATED.CHILD1"));
			}
			return RAMFileClass::Read(buffer,size);
		}
		void Close() override { if (Is_Open()) ++provider.closes;RAMFileClass::Close(); }
	};
	Packet& packet;
	int gets=0,returns=0,closes=0,reads=0,cut=-1;
	bool available=true;
	Packet* nested=nullptr;
	bool did_nested=false;
	WW3DAssetManager* manager=nullptr;
	const std::vector<unsigned char>* baseline=nullptr;
	explicit Provider(Packet& p):packet(p) {}
	FileClass* Get_File(const char* name) override { ++gets;return available ? new File(*this,nested && !std::strcmp(name,"GENERATED.CHILD.w3d") ? *nested : packet) : nullptr; }
	void Return_File(FileClass* file) override { ++returns;delete file; }
};

struct CapacityPrototype : PrototypeClass {
	char name[48];
	explicit CapacityPrototype(unsigned ordinal) { std::snprintf(name,sizeof(name),"GENERATED.CAPACITY%u",ordinal); }
	const char* Get_Name() const override { return name; }
	int Get_Class_ID() const override { return RenderObjClass::CLASSID_NULL; }
	RenderObjClass* Create() override { return nullptr; }
	void DeleteSelf() override { delete this; }
};

void hierarchy_packet(Packet& packet,const char* name)
{
	RAMFileClass file(packet.data.data(),packet.data.size());assert(file.Open(FileClass::WRITE));ChunkSaveClass writer(&file);
	assert(writer.Begin_Chunk(W3D_CHUNK_HIERARCHY));
	W3dHierarchyStruct header{};header.Version=W3D_MAKE_VERSION(3,0);std::strcpy(header.Name,name);header.NumPivots=1;chunk(writer,W3D_CHUNK_HIERARCHY_HEADER,header);
	W3dPivotStruct pivot{};std::strcpy(pivot.Name,"ROOT");pivot.ParentIdx=0xffffffffu;pivot.Rotation.Q[3]=1;chunk(writer,W3D_CHUNK_PIVOTS,pivot);assert(writer.End_Chunk());
	packet.bytes=file.Seek(0,SEEK_CUR);file.Close();
}

void cull_packet(Packet& packet,bool late_invalid=false)
{
	RAMFileClass file(packet.data.data(),packet.data.size());assert(file.Open(FileClass::WRITE));ChunkSaveClass writer(&file);
	make_mesh(writer,false,false,false,0,false,false,24,false,true,"G","CULL",8);
	if (late_invalid) { assert(writer.Begin_Chunk(0x7ffffffe));assert(writer.End_Chunk()); }
	packet.bytes=file.Seek(0,SEEK_CUR);file.Close();
	// Use separated triangles so random candidates really partition the mesh.
	unsigned offset=8;
	while (offset+8<=unsigned(packet.bytes)) {
		unsigned id,length;std::memcpy(&id,packet.data.data()+offset,4);std::memcpy(&length,packet.data.data()+offset+4,4);
		length&=0x7fffffffu;
		if (id==W3D_CHUNK_VERTICES) for (unsigned poly=0;poly<8;++poly) {
			const W3dVectorStruct vertices[3]={{float(poly*2),0,0},{float(poly*2+1),0,0},{float(poly*2),1,0}};
			std::memcpy(packet.data.data()+offset+8+poly*sizeof(vertices),vertices,sizeof(vertices));
		}
		if (id==W3D_CHUNK_TRIANGLES) for (unsigned poly=0;poly<8;++poly) {
			W3dTriStruct triangle{};triangle.Vindex[0]=poly*3;triangle.Vindex[1]=poly*3+1;triangle.Vindex[2]=poly*3+2;triangle.Normal.Z=1;
			std::memcpy(packet.data.data()+offset+8+poly*sizeof(triangle),&triangle,sizeof(triangle));
		}
		offset+=8+length;
	}
}
AABTreeClass* imported_tree(WW3DAssetManager& manager)
{
	auto* prototype=manager.Find_Prototype("G.CULL");assert(prototype && prototype->Get_Class_ID()==RenderObjClass::CLASSID_MESH);
	return W3DCloneGraphProbeAccess::tree(static_cast<MeshClass*>(static_cast<PrimitivePrototypeClass*>(prototype)->Proto)->Peek_Model());
}
std::array<int,20> cull_queries(AABTreeClass* tree)
{
	std::array<int,20> result{};
	for (int i=0;i<10;++i) { unsigned char flags=0;
		result[i*2]=tree->Cast_Semi_Infinite_Axis_Aligned_Ray(Vector3(float(i*2)-1.8f,0.2f,1),5,flags);
		result[i*2+1]=flags;
	}
	return result;
}
void cull_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;cull_packet(packet);
	std::vector<unsigned char> native_bytes;std::array<int,20> queries{};int next=0;
	{
		WW3DAssetManager manager;std::srand(173);assert(manager.Load_3D_Assets("GENERATED.CULL.w3d"));
		native_bytes=W3DCloneGraphProbeAccess::bytes(imported_tree(manager));queries=cull_queries(imported_tree(manager));next=std::rand();
		assert(queries[0]==0 && queries[2]==1 && queries[16]==1 && queries[18]==0);
	}
	{
		WW3DAssetManager manager;std::srand(173);assert(ww3d_import::Attempt::load(manager,"GENERATED.CULL.w3d"));
		assert(W3DCloneGraphProbeAccess::bytes(imported_tree(manager))==native_bytes && std::rand()==next);
		assert(cull_queries(imported_tree(manager))==queries);
	}
	native_bytes.clear();native_bytes.shrink_to_fit();
	{
		WW3DAssetManager manager;manager.Add_Prototype(new CapacityPrototype(0));
		std::array<unsigned char,sizeof(manager)> baseline{};std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		unsigned boundaries=0;
		for (int ordinal=0;ordinal<512;++ordinal) {
			bool rejected=false;std::srand(173);W3DAssetImportProbeAccess::fault(ordinal);
			try { assert(ww3d_import::Attempt::load(manager,"GENERATED.CULL.w3d")); }
			catch (const std::bad_alloc&) { rejected=true; }
			boundaries=W3DAssetImportProbeAccess::faults();W3DAssetImportProbeAccess::fault(-1);
			if (!rejected) break;
			assert(std::memcmp(&manager,baseline.data(),baseline.size())==0 && !manager.Find_Prototype("G.CULL"));
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		assert(boundaries>20 && manager.Find_Prototype("G.CULL"));
	}
	for (unsigned seed=1;seed<=4;++seed) {
		WW3DAssetManager manager;manager.Add_Prototype(new CapacityPrototype(0));
		std::array<unsigned char,sizeof(manager)> baseline{};std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		cull_packet(packet,true);std::srand(seed);
		for (int repeat=0;repeat<2;++repeat) {
			assert(!ww3d_import::Attempt::load(manager,"GENERATED.CULL.w3d"));
			assert(std::memcmp(&manager,baseline.data(),baseline.size())==0 && !manager.Find_Prototype("G.CULL"));
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		cull_packet(packet);assert(ww3d_import::Attempt::load(manager,"GENERATED.CULL.w3d"));assert(cull_queries(imported_tree(manager))==queries);
	}
	_TheFileFactory=previous;
}

void copy_mesh_chunk(ChunkSaveClass& writer,const unsigned char* data,unsigned start,unsigned end,int mutation=0)
{
	while (start<end) {
		unsigned id,encoded;std::memcpy(&id,data+start,4);std::memcpy(&encoded,data+start+4,4);
		const unsigned length=encoded&0x7fffffffu;assert(start+8+length<=end);
		assert(writer.Begin_Chunk(id));
		if (encoded&0x80000000u) {
			copy_mesh_chunk(writer,data,start+8,start+8+length,mutation);
			if (mutation==7 && id==W3D_CHUNK_MATERIAL_PASS) { const unsigned zero=0;chunk(writer,W3D_CHUNK_SHADER_IDS,zero);chunk(writer,W3D_CHUNK_VERTEX_MATERIAL_IDS,zero); }
		}
		else if ((mutation==1 && id==W3D_CHUNK_SHADER_IDS) || (mutation==2 && id==W3D_CHUNK_VERTEX_MATERIAL_IDS) || (mutation==3 && id==W3D_CHUNK_TEXTURE_IDS)) {
			const unsigned invalid=999;assert(length==4 && writer.Write(&invalid,4)==4);
		} else if (mutation==4 && id==W3D_CHUNK_VERTICES) {
			std::vector<unsigned char> bytes(data+start+8,data+start+8+length);const float bad=std::numeric_limits<float>::infinity();
			std::memcpy(bytes.data(),&bad,4);assert(writer.Write(bytes.data(),length)==int(length));
		} else if (mutation==5 && id==W3D_CHUNK_TRIANGLES) {
			std::vector<unsigned char> bytes(data+start+8,data+start+8+length);const unsigned bad=999;
			std::memcpy(bytes.data(),&bad,4);assert(writer.Write(bytes.data(),length)==int(length));
		} else if (mutation==7 && id==W3D_CHUNK_STAGE_TEXCOORDS) {
			const W3dTexCoordStruct uvs[6]={{0,0},{1,0},{0,1},{0.25f,0},{1,0.25f},{0.25f,1}};assert(writer.Write(uvs,sizeof(uvs))==sizeof(uvs));
		} else assert(writer.Write(data+start+8,length)==int(length));
		assert(writer.End_Chunk());start+=8+length;
	}
}
void material_mesh_packet(Packet& packet,int mode,int mutation=0)
{
	Packet base;RAMFileClass input(base.data.data(),base.data.size());assert(input.Open(FileClass::WRITE));ChunkSaveClass source(&input);
	make_mesh(source,false,false,mode==5,2,true,false,0,false,false,"G","MATERIAL");base.bytes=input.Seek(0,SEEK_CUR);input.Close();
	RAMFileClass output(packet.data.data(),packet.data.size());assert(output.Open(FileClass::WRITE));ChunkSaveClass writer(&output);
	assert(writer.Begin_Chunk(W3D_CHUNK_MESH));unsigned offset=8;bool wrapper=false;
	const unsigned wrappers[]={0,W3D_CHUNK_PRELIT_UNLIT,W3D_CHUNK_PRELIT_VERTEX,W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS,W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE};
	const unsigned flags[]={0,W3D_MESH_FLAG_PRELIT_UNLIT,W3D_MESH_FLAG_PRELIT_VERTEX,W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS,W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE};
	while (offset<unsigned(base.bytes)) {
		unsigned id,length;std::memcpy(&id,base.data.data()+offset,4);std::memcpy(&length,base.data.data()+offset+4,4);length&=0x7fffffffu;
		if (id==W3D_CHUNK_MESH_HEADER3 && mode>0 && mode<5) {
			W3dMeshHeader3Struct header;std::memcpy(&header,base.data.data()+offset+8,sizeof(header));header.Attributes|=flags[mode];chunk(writer,id,header);
		} else {
			if (id==W3D_CHUNK_MATERIAL_INFO && mode>0 && mode<5) { assert(writer.Begin_Chunk(wrappers[mode]));wrapper=true; }
			copy_mesh_chunk(writer,base.data.data(),offset,offset+8+length,mode==6 ? 7 : mutation);
		}
		offset+=8+length;
	}
	if (wrapper) assert(writer.End_Chunk());
	if (mode==0) {
		assert(writer.Begin_Chunk(W3D_CHUNK_AABTREE));W3dMeshAABTreeHeader header{};header.NodeCount=1;header.PolyCount=1;chunk(writer,W3D_CHUNK_AABTREE_HEADER,header);
		unsigned index=mutation==6 ? 1 : 0;chunk(writer,W3D_CHUNK_AABTREE_POLYINDICES,index);
		W3dMeshAABTreeNode node{};node.Max={1,1,0};node.FrontOrPoly0=AABTREE_LEAF_FLAG;node.BackOrPolyCount=1;chunk(writer,W3D_CHUNK_AABTREE_NODES,node);assert(writer.End_Chunk());
	}
	assert(writer.End_Chunk());packet.bytes=output.Seek(0,SEEK_CUR);output.Close();
}
void material_mesh_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;const auto mode=WW3D::Get_Prelit_Mode();
	for (int branch=0;branch<7;++branch) {
		WW3D::Set_Prelit_Mode(branch==3 ? WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS : branch==4 ? WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE : WW3D::PRELIT_MODE_VERTEX);
		material_mesh_packet(packet,branch);WW3DAssetManager manager;manager.Add_Prototype(new CapacityPrototype(0));
		std::array<unsigned char,sizeof(manager)> baseline{};std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();unsigned boundaries=0;
		for (int ordinal=0;ordinal<512;++ordinal) {
			bool rejected=false;W3DAssetImportProbeAccess::fault(ordinal);
			try { assert(ww3d_import::Attempt::load(manager,"GENERATED.MATERIAL.w3d")); } catch (const std::bad_alloc&) { rejected=true; }
			boundaries=W3DAssetImportProbeAccess::faults();W3DAssetImportProbeAccess::fault(-1);if (!rejected) break;
			assert(std::memcmp(&manager,baseline.data(),baseline.size())==0 && !manager.Find_Prototype("G.MATERIAL"));
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		assert(boundaries>20 && manager.Find_Prototype("G.MATERIAL"));
		auto* mesh=static_cast<MeshClass*>(static_cast<PrimitivePrototypeClass*>(manager.Find_Prototype("G.MATERIAL"))->Proto)->Peek_Model();
		assert(mesh->Get_Vertex_Count()==3 && mesh->Get_Polygon_Count()==1 && mesh->Peek_Single_Texture(0,0));
		if (!branch) assert(mesh->Has_Cull_Tree());
	}
	WW3D::Set_Prelit_Mode(mode);
	for (int mutation=1;mutation<=6;++mutation) {
		material_mesh_packet(packet,0,mutation);WW3DAssetManager manager;
		std::array<unsigned char,sizeof(manager)> baseline{};std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();bool rejected=false;
		try { rejected=!ww3d_import::Attempt::load(manager,"GENERATED.MATERIAL.w3d"); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		material_mesh_packet(packet,0);assert(ww3d_import::Attempt::load(manager,"GENERATED.MATERIAL.w3d"));
	}
	_TheFileFactory=previous;
}

void uv_capacity_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;WW3DAssetManager manager;
	MeshMatDescClass descriptor;descriptor.Set_Vertex_Count(3);descriptor.Set_Polygon_Count(1);descriptor.Set_Pass_Count(4);
	ww3d_import::Attempt attempt(manager);
	for (int i=0;i<8;++i) {
		Vector2 values[3]={{float(i),0},{float(i+1),0},{float(i),1}};
		descriptor.Install_UV_Array(i/2,i%2,values,3);
		assert(descriptor.Get_UV_Array_Count()==i+1 && descriptor.Get_UV_Source(i/2,i%2)==i);
	}
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();bool rejected=false;
	Vector2 extra[3]={{99,0},{100,0},{99,1}};
	try { descriptor.Install_UV_Array(0,0,extra,3); } catch (const std::runtime_error&) { rejected=true; }
	assert(rejected && descriptor.Get_UV_Array_Count()==8 && descriptor.Get_UV_Source(0,0)==0);
	assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	assert(attempt.commit());_TheFileFactory=previous;
}

void legacy_mesh_packet(Packet& packet,bool bad_name=false)
{
	RAMFileClass file(packet.data.data(),packet.data.size());assert(file.Open(FileClass::WRITE));ChunkSaveClass writer(&file);
	assert(writer.Begin_Chunk(W3D_CHUNK_MESH));W3dMeshHeader3Struct header{};header.Version=W3D_MAKE_VERSION(3,0);
	std::strcpy(header.ContainerName,"G");std::strcpy(header.MeshName,"LEGACY");header.NumVertices=3;header.NumTris=1;header.NumMaterials=2;
	header.Max={1,1,0};header.SphRadius=1;chunk(writer,W3D_CHUNK_MESH_HEADER3,header);
	const W3dVectorStruct vertices[3]={{0,0,0},{1,0,0},{0,1,0}},normals[3]={{0,0,1},{0,0,1},{0,0,1}};
	chunk(writer,W3D_CHUNK_VERTICES,vertices);chunk(writer,W3D_CHUNK_VERTEX_NORMALS,normals);
	W3dTriStruct triangle{};triangle.Vindex[1]=1;triangle.Vindex[2]=2;triangle.Normal.Z=1;chunk(writer,W3D_CHUNK_TRIANGLES,triangle);
	const W3dTexCoordStruct coordinates[3]={{0,0},{1,0},{0,1}};chunk(writer,W3D_CHUNK_TEXCOORDS,coordinates);
	assert(writer.Begin_Chunk(W3D_CHUNK_MATERIALS3));
	for (int i=0;i<2;++i) {
		assert(writer.Begin_Chunk(W3D_CHUNK_MATERIAL3));const char name[]="GENERATED.LEGACY";chunk(writer,W3D_CHUNK_MATERIAL3_NAME,name);
		W3dMaterial3Struct material{};material.Opacity=1;material.Shininess=1;material.DiffuseColor.R=i ? 0 : 255;material.DiffuseCoefficients.R=255;
		chunk(writer,W3D_CHUNK_MATERIAL3_INFO,material);
		for (int map=0;map<2;++map) {
			assert(writer.Begin_Chunk(map ? W3D_CHUNK_MATERIAL3_SI_MAP : W3D_CHUNK_MATERIAL3_DC_MAP));
			const char filename[]="GENERATED.LEGACY.TGA";
			if (bad_name) { assert(writer.Begin_Chunk(W3D_CHUNK_MAP3_FILENAME));assert(writer.Write(filename,sizeof(filename)-1)==sizeof(filename)-1);assert(writer.End_Chunk()); }
			else chunk(writer,W3D_CHUNK_MAP3_FILENAME,filename);
			W3dMap3Struct info{};info.FrameCount=1;chunk(writer,W3D_CHUNK_MAP3_INFO,info);assert(writer.End_Chunk());
		}
		assert(writer.End_Chunk());
	}
	assert(writer.End_Chunk());const uint16 material_index=1;chunk(writer,W3D_CHUNK_PER_TRI_MATERIALS,material_index);
	const W3dRGBStruct colors[3]={{255,255,255},{255,255,255},{255,255,255}};chunk(writer,W3D_CHUNK_VERTEX_COLORS,colors);
	const char user[]="GENERATED.USER";chunk(writer,W3D_CHUNK_MESH_USER_TEXT,user);
	const unsigned shade[3]={0,1,2};chunk(writer,W3D_CHUNK_VERTEX_SHADE_INDICES,shade);assert(writer.End_Chunk());
	packet.bytes=file.Seek(0,SEEK_CUR);file.Close();
}
void legacy_mesh_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;legacy_mesh_packet(packet);
	WW3DAssetManager manager;manager.Add_Prototype(new CapacityPrototype(0));
	std::array<unsigned char,sizeof(manager)> baseline{};std::memcpy(baseline.data(),&manager,baseline.size());
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();unsigned boundaries=0;
	for (int ordinal=0;ordinal<512;++ordinal) {
		bool rejected=false;W3DAssetImportProbeAccess::fault(ordinal);
		try { assert(ww3d_import::Attempt::load(manager,"GENERATED.LEGACY.w3d")); } catch (const std::bad_alloc&) { rejected=true; }
		boundaries=W3DAssetImportProbeAccess::faults();W3DAssetImportProbeAccess::fault(-1);if (!rejected) break;
		assert(std::memcmp(&manager,baseline.data(),baseline.size())==0 && !manager.Find_Prototype("G.LEGACY"));
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
	assert(boundaries>20 && manager.Find_Prototype("G.LEGACY"));
	std::memcpy(baseline.data(),&manager,baseline.size());legacy_mesh_packet(packet,true);bool rejected=false;
	try { rejected=!ww3d_import::Attempt::load(manager,"GENERATED.LEGACY.w3d"); } catch (const std::runtime_error&) { rejected=true; }
	assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0);
	_TheFileFactory=previous;
}
void edge_provider_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;
	zh::renderer::RecordingGpuDevice device;
	using Edge=zh::original_runtime::OriginalGpuEdge;std::unique_ptr<Edge> edge(new Edge(device));
	WW3DAssetManager manager;std::array<unsigned char,sizeof(manager)> baseline{};std::memcpy(baseline.data(),&manager,baseline.size());
	{
		ww3d_import::Attempt attempt(manager);assert(manager.Load_3D_Assets("GENERATED.EDGE.w3d"));
		const auto generation=edge->generation();W3DTerrainPropLifecycleProbeAccess::generation(*edge,generation+1);bool rejected=false;
		try { attempt.validate(); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && !attempt.commit() && std::memcmp(&manager,baseline.data(),baseline.size())==0);
		W3DTerrainPropLifecycleProbeAccess::generation(*edge,generation);assert(attempt.commit());
	}
	std::memcpy(baseline.data(),&manager,baseline.size());
	{
		ww3d_import::Attempt attempt(manager);edge.reset();bool rejected=false;
		try { attempt.validate(); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && !attempt.commit());edge.reset(new Edge(device));rejected=false;
		try { attempt.validate(); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && !attempt.commit() && std::memcmp(&manager,baseline.data(),baseline.size())==0);
	}
	{ ww3d_import::Attempt attempt(manager);assert(attempt.commit()); }
	for (int kind=0;kind<3;++kind) {
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();bool rejected=false;
		try {
			ww3d_import::Attempt attempt(manager);
			if (!kind) manager.Load_Procedural_Textures();
			else manager.Get_Texture("GENERATED.UNSUPPORTED.TGA",MIP_LEVELS_1,WW3D_FORMAT_UNKNOWN,true,kind==1 ? TextureBaseClass::TEX_CUBEMAP : TextureBaseClass::TEX_VOLUME);
		} catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
	zh::renderer::TextureDesc description;description.width=description.height=4;description.render_target=true;
	auto color=device.create_texture(description,"generated import owner target");description.format=zh::renderer::TextureFormat::depth24_stencil8;
	auto depth=device.create_texture(description,"generated import owner depth");edge->bind_frame_targets(color,depth,4,4);
	for (bool active:{false,true}) {
		if (active) edge->begin_source_frame(true,true,0,0,0,1);else assert(edge->begin_tree_source_frame());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();const auto resources=device.resource_counts();bool rejected=false;
		try { ww3d_import::Attempt attempt(manager); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0 && device.resource_counts()==resources);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		if (active) edge->abort_source_frame();else assert(edge->abort_tree_source_frame());
		ww3d_import::Attempt retry(manager);assert(retry.commit());
	}
	edge.reset();device.destroy(depth);device.destroy(color);assert(device.resource_counts()==zh::renderer::ResourceCounts{});
	_TheFileFactory=previous;
}

void capacity_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;
	{
		WW3DAssetManager manager;W3DAssetImportProbeAccess::reserve_prototypes(manager);
		for (unsigned i=0;i<65535;++i) manager.Add_Prototype(new CapacityPrototype(i));
		// The final exact slot succeeds; bound+1 preserves the complete registry.
		{
			ww3d_import::Attempt attempt(manager);std::unique_ptr<PrototypeClass,ww3d_import::PrototypeDelete> candidate(new CapacityPrototype(65535));
			manager.Add_Prototype(candidate.get());candidate.release();assert(attempt.commit());
		}
		std::vector<unsigned char> baseline(sizeof(manager));std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for (int repeat=0;repeat<2;++repeat) {
			bool rejected=false;
			try { ww3d_import::Attempt attempt(manager);std::unique_ptr<PrototypeClass,ww3d_import::PrototypeDelete> candidate(new CapacityPrototype(65536));manager.Add_Prototype(candidate.get()); }
			catch (const std::runtime_error&) { rejected=true; }
			assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0);
			assert(manager.Find_Prototype("GENERATED.CAPACITY0") && manager.Find_Prototype("GENERATED.CAPACITY65535") && !manager.Find_Prototype("GENERATED.CAPACITY65536"));
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
	}
	{
		WW3DAssetManager manager;
		for (unsigned i=0;i<15999;++i) {
			char name[16];std::snprintf(name,sizeof(name),"G%u",i);hierarchy_packet(packet,name);
			RAMFileClass file(packet.data.data(),packet.bytes);assert(file.Open(FileClass::READ));ChunkLoadClass reader(&file);assert(reader.Open_Chunk());
			std::unique_ptr<HTreeClass> candidate(new HTreeClass);assert(candidate->Load_W3D(reader)==HTreeClass::OK);
			W3DAssetImportProbeAccess::seed_tree(manager,candidate.get());candidate.release();
		}
		hierarchy_packet(packet,"G15999");assert(ww3d_import::Attempt::load(manager,"GENERATED.LAST.w3d"));
		assert(manager.Get_HTree("G0") && manager.Get_HTree("G15999"));
		std::vector<unsigned char> baseline(sizeof(manager));std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		hierarchy_packet(packet,"G16000");
		for (int repeat=0;repeat<2;++repeat) {
			bool rejected=false;try { rejected=!ww3d_import::Attempt::load(manager,"GENERATED.EXTRA.w3d"); }
			catch (const std::runtime_error&) { rejected=true; }
			assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
	}
	{
		WW3DAssetManager manager;TextureClass* sibling=manager.Get_Texture("GENERATED.CACHE.TGA");sibling->Release_Ref();
		for (unsigned i=1;i<65535;++i) { char key[48];std::snprintf(key,sizeof(key),"generated.alias%u",i);W3DAssetImportProbeAccess::seed_texture(manager,key,sibling); }
		{ ww3d_import::Attempt attempt(manager);TextureClass* last=manager.Get_Texture("GENERATED.LAST.TGA");last->Release_Ref();assert(attempt.commit()); }
		std::vector<unsigned char> baseline(sizeof(manager));std::memcpy(baseline.data(),&manager,baseline.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for (int repeat=0;repeat<2;++repeat) {
			bool rejected=false;try { ww3d_import::Attempt attempt(manager);manager.Get_Texture("GENERATED.EXTRA.TGA"); }
			catch(const std::runtime_error&) { rejected=true; }
			assert(rejected && std::memcmp(&manager,baseline.data(),baseline.size())==0 && sibling->Num_Refs()==65535);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
	}
	_TheFileFactory=previous;
}

void mapper_argument_controls()
{
	Packet packet;Provider provider(packet);auto* previous=_TheFileFactory;_TheFileFactory=&provider;
	WW3DAssetManager manager;
	std::string text="[Args]\nFirst=7\nFirst=99\n";
	for (unsigned i=0;i<24;++i) text+="Key"+std::to_string(i)+"="+std::to_string(i)+"\n";
	for (unsigned i=0;i<12;++i) text+="[Extra"+std::to_string(i)+"]\nValue="+std::to_string(i)+"\n";
	std::vector<unsigned char> baseline(sizeof(manager));std::memcpy(baseline.data(),&manager,baseline.size());
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	bool complete=false;
	for (int ordinal=0;ordinal<1024 && !complete;++ordinal) {
		W3DAssetImportProbeAccess::fault(ordinal);
		try {
			ww3d_import::Attempt attempt(manager);
			{
				INIClass args;BufferStraw straw(text.data(),text.size()+1);assert(args.Load(straw));
				const unsigned faults=W3DAssetImportProbeAccess::faults();
				assert(args.Get_Int("Args","First",0)==7 && args.Get_Int("Args","Key23",0)==23 && args.Section_Count()==13);
				assert(W3DAssetImportProbeAccess::faults()==faults); // readonly lookup has no hook/allocation checkpoint
			}
			complete=attempt.commit();
		} catch(const std::bad_alloc&) {}
		W3DAssetImportProbeAccess::fault(-1);
		assert(std::memcmp(&manager,baseline.data(),baseline.size())==0);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
	}
	assert(complete);
	// A constructor failure restores the installed hook only when its exact
	// outer attempt ends; a no-hook standalone caller then sees native grammar.
	{
		ww3d_import::Attempt attempt(manager);W3DAssetImportProbeAccess::fault(0);bool rejected=false;
		try { INIClass args; } catch(const std::bad_alloc&) { rejected=true; }assert(rejected);
		W3DAssetImportProbeAccess::fault(-1);{ INIClass args;assert(!args.Is_Loaded()); }assert(attempt.commit());
	}
	W3DAssetImportProbeAccess::fault(-1);
	{ INIClass args;BufferStraw straw(text.data(),text.size()+1);assert(args.Load(straw) && args.Get_Int("Args","First",0)==7); }
	assert(!W3DAssetImportProbeAccess::faults());
	// Actual VertexMaterial parsing covers null args and bounded nonempty args
	// on both stages, including mapper initialization and temporary INI teardown.
	for (bool populated:{false,true}) {
		std::array<unsigned char,1024> bytes{};RAMFileClass file(bytes.data(),bytes.size());assert(file.Open(FileClass::WRITE));ChunkSaveClass writer(&file);
		assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL));const char name[]="GENERATED.MAPPER";chunk(writer,W3D_CHUNK_VERTEX_MATERIAL_NAME,name);
		W3dVertexMaterialStruct material{};W3d_Vertex_Material_Reset(&material);
		material.Attributes=W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET|W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET;chunk(writer,W3D_CHUNK_VERTEX_MATERIAL_INFO,material);
		if (populated) { const char args[]="UPerSec=1\nVPerSec=2\nUPerSec=99\n";chunk(writer,W3D_CHUNK_VERTEX_MAPPER_ARGS0,args);chunk(writer,W3D_CHUNK_VERTEX_MAPPER_ARGS1,args); }
		assert(writer.End_Chunk());const int size=file.Seek(0,SEEK_CUR);file.Close();complete=false;
		for (int ordinal=0;ordinal<512 && !complete;++ordinal) {
			W3DAssetImportProbeAccess::fault(ordinal);
			try {
				ww3d_import::Attempt attempt(manager);ww3d_import::Attempt::boundary();
				ww3d_clone::Ref<VertexMaterialClass> value(new VertexMaterialClass);
				RAMFileClass input(bytes.data(),size);assert(input.Open(FileClass::READ));ChunkLoadClass reader(&input);assert(reader.Open_Chunk());
				assert(value.get()->Load_W3D(reader)==WW3D_ERROR_OK && value.get()->Peek_Mapper(0) && value.get()->Peek_Mapper(1));complete=true;
			} catch(const std::bad_alloc&) {}
			W3DAssetImportProbeAccess::fault(-1);
			assert(std::memcmp(&manager,baseline.data(),baseline.size())==0);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		assert(complete);
	}
	_TheFileFactory=previous;
}

void generation()
{
	Packet packet;Provider provider(packet);
	struct FactoryRestore { FileFactoryClass* before;~FactoryRestore() { _TheFileFactory=before; } } restore{_TheFileFactory};
	_TheFileFactory=&provider;
	std::unique_ptr<WW3DAssetManager> manager(new WW3DAssetManager);
	provider.manager=manager.get();
	// First insertion starts from the exact native empty cache, not a prewarm.
	{
		std::vector<unsigned char> empty(sizeof(WW3DAssetManager));std::memcpy(empty.data(),manager.get(),empty.size());
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		bool complete=false;
		for (int ordinal=0;ordinal<4096 && !complete;++ordinal) {
			W3DAssetImportProbeAccess::fault(ordinal);
			try {
				ww3d_import::Attempt attempt(*manager);
				TextureClass* candidate=manager->Get_Texture("GENERATED.COLD.TGA");
				assert(candidate && manager->Texture_Hash().Get("generated.cold.tga")==nullptr);
				candidate->Release_Ref();complete=true;
			} catch (const std::bad_alloc&) {}
			W3DAssetImportProbeAccess::fault(-1);
			assert(std::memcmp(manager.get(),empty.data(),empty.size())==0);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		assert(complete);
		ww3d_import::Attempt attempt(*manager);
		TextureClass* candidate=manager->Get_Texture("GENERATED.COLD.TGA");candidate->Release_Ref();assert(attempt.commit());
		assert(manager->Texture_Hash().Get("generated.cold.tga")==candidate && candidate->Num_Refs()==1);
	}
	TextureClass* sibling=manager->Get_Texture("GENERATED.SIBLING.TGA");sibling->Release_Ref();
	const auto filter=sibling->Get_Filter();
	std::vector<unsigned char> baseline(sizeof(WW3DAssetManager));std::memcpy(baseline.data(),manager.get(),baseline.size());
	provider.baseline=&baseline;
	const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
	// Every read failure closes/returns once and preserves accepted identities.
	for (int cut=1;cut<32;++cut) {
		provider.reads=0;provider.cut=cut;
		const int gets=provider.gets,returns=provider.returns,closes=provider.closes;
		bool completed=false;
		try { ww3d_import::Attempt attempt(*manager);completed=manager->Load_3D_Assets("GENERATED.w3d"); }
		catch (const std::bad_alloc&) {}
		assert(provider.gets-gets==provider.returns-returns && provider.returns-returns==provider.closes-closes);
		assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
		assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		if (completed) break;
	}
	provider.cut=-1;
	// A late-invalid root withdraws earlier candidates, including nested loads.
	{
		Packet parent(false,true),child(false,false,"GENERATED.CHILD1","GENERATED.CHILD2");
		Provider nested(parent);nested.nested=&child;nested.manager=manager.get();nested.baseline=&baseline;
		_TheFileFactory=&nested;
		assert(!ww3d_import::Attempt::load(*manager,"GENERATED.PARENT.w3d") && nested.did_nested);
		assert(nested.gets==nested.returns && nested.returns==nested.closes);
		assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
		assert(!manager->Find_Prototype("GENERATED.CHILD1"));
		_TheFileFactory=&provider;
	}
	// Truncated headers/payloads fail before any registry publication.
	const int full=packet.bytes;
	for (int bytes=0;bytes<full;++bytes) {
		packet.bytes=bytes;
		if (bytes==0) continue; // Native empty W3D is a successful no-op.
		bool result=false;
		{ ww3d_import::Attempt attempt(*manager);result=manager->Load_3D_Assets("GENERATED.w3d"); }
		if (result) assert(bytes==full/2); // One complete independent box chunk is valid.
		assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
	}
	packet.bytes=full;
	// Cold hierarchy/raw-channel graph participates in the same root transaction.
	{
		Packet graph(false,false,"GENERATED.GRAPH1","GENERATED.GRAPH2",true);Provider reader(graph);
		reader.manager=manager.get();reader.baseline=&baseline;_TheFileFactory=&reader;
		bool complete=false;
		for (int ordinal=0;ordinal<4096 && !complete;++ordinal) {
			W3DAssetImportProbeAccess::fault(ordinal);
			try {
				ww3d_import::Attempt attempt(*manager);complete=manager->Load_3D_Assets("GENERATED.GRAPH.w3d");
				if (complete) {
					assert(manager->Get_HTree("GENROOT") && manager->Get_HTree("GENROOT")->Num_Pivots()==1);
					HAnimClass* animation=manager->Get_HAnim("GENROOT.ANIM1");assert(animation && animation->Get_Num_Frames()==1);animation->Release_Ref();
					for (const char* name:{"GENROOT.TC1","GENROOT.AD1","GENROOT.MORPH1"}) {
						HAnimClass* animation=manager->Get_HAnim(name);assert(animation && animation->Get_Num_Frames()==1);animation->Release_Ref();
					}
					assert(manager->Find_Prototype("GENERATED.MESH1") && manager->Find_Prototype("GENERATED.HLOD1"));
					for (const char* name:{"G.HMODEL","G.COLLECTION","GENERATED.LOD","GENERATED.AGG","GENERATED.NULL","GENERATED.RING","GENERATED.SPHERE","GENERATED.DAZZLE"}) assert(manager->Find_Prototype(name));
					TextureClass* first=manager->Get_Texture("MYTEX.TGA");assert(first && !first->Is_Initialized());first->Release_Ref();
				}
			} catch (const std::bad_alloc&) {}
			W3DAssetImportProbeAccess::fault(-1);
			assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		assert(complete && reader.gets==reader.returns && reader.returns==reader.closes);
		for (float scale:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}) {
			graph=Packet(false,false,"GENERATED.GRAPH1","GENERATED.GRAPH2",true,scale);
			for (int repeat=0;repeat<2;++repeat) {
				assert(!ww3d_import::Attempt::load(*manager,"GENERATED.GRAPH.w3d"));
				assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
				assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
				assert(reader.gets==reader.returns && reader.returns==reader.closes);
			}
			graph=Packet(false,false,"GENERATED.GRAPH1","GENERATED.GRAPH2",true);
			{ ww3d_import::Attempt retry(*manager);assert(manager->Load_3D_Assets("GENERATED.GRAPH.w3d")); }
			assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
		_TheFileFactory=&provider;
	}
	// Owner-local depth/cycle/byte rejection leaves the live root usable.
	{
		ww3d_import::Attempt attempt(*manager);
		std::vector<std::unique_ptr<ww3d_import::Attempt::FileScope>> stack;
		for (int i=0;i<64;++i) { std::string name="GENERATED.DEPTH"+std::to_string(i);stack.emplace_back(new ww3d_import::Attempt::FileScope(name.c_str())); }
		bool rejected=false;try { ww3d_import::Attempt::FileScope overflow("GENERATED.DEPTH64"); } catch (const std::runtime_error&) { rejected=true; }assert(rejected);
		while (!stack.empty()) stack.pop_back();
		{ ww3d_import::Attempt::FileScope file("GENERATED.CYCLE");rejected=false;
			try { ww3d_import::Attempt::FileScope cycle("generated.cycle"); } catch (const std::runtime_error&) { rejected=true; }assert(rejected); }
		rejected=false;try { ww3d_import::Attempt::reserve(64u*1024u*1024u+1,1); } catch (const std::runtime_error&) { rejected=true; }assert(rejected);
		assert(attempt.commit());
	}
	// A removed provider is never dereferenced or used for publication; restoring
	// the exact provider permits the original no-op owner to commit once.
	{
		ww3d_import::Attempt attempt(*manager);_TheFileFactory=nullptr;bool rejected=false;
		try { attempt.validate(); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && !attempt.commit());_TheFileFactory=&provider;assert(attempt.commit());
	}
	// The importer bounds the 256-byte authored name even though this Linux
	// target's native filesystem path storage is 4096 bytes. Exact missing
	// lookup entries avoid any OS or archive provider/data in this unit control.
	{
		struct KnownMissingFileSystem : FileSystem {
			void missing(const std::string& path) { m_fileExist[TheNameKeyGenerator->nameToLowercaseKey(path.c_str())]=false; }
		} file_system;
		NameKeyGenerator keys;auto* saved_keys=TheNameKeyGenerator;TheNameKeyGenerator=&keys;keys.init();
		FileSystem* saved=TheFileSystem;TheFileSystem=&file_system;
		GameFileClass file;std::vector<unsigned char> before(sizeof(file));std::memcpy(before.data(),&file,before.size());
		ww3d_import::Attempt attempt(*manager);bool rejected=false;
		std::string overlong(253,'G');overlong+=".w3d";
		try { file.Set_Name(overlong.c_str()); } catch(const std::runtime_error&) { rejected=true; }
		assert(rejected && std::memcmp(&file,before.data(),before.size())==0);
		std::string exact(250,'G');exact+=".w3d";
		file_system.missing("Data/english/Art/W3D/"+exact);file_system.missing(std::string(W3D_DIR_PATH)+exact);file_system.missing(std::string(TEST_W3D_DIR_PATH)+exact);
		assert(file.Set_Name(exact.c_str()) && !std::strcmp(file.File_Name(),exact.c_str()) && !file.Is_Available());
		TheFileSystem=nullptr;rejected=false;try { attempt.validate(); } catch(const std::runtime_error&) { rejected=true; }
		assert(rejected && !attempt.commit());TheFileSystem=&file_system;assert(attempt.commit());TheFileSystem=saved;TheNameKeyGenerator=saved_keys;
	}
	bool success=false;
	for (int ordinal=0;ordinal<4096 && !success;++ordinal) {
		const int gets=provider.gets,returns=provider.returns,closes=provider.closes;
		W3DAssetImportProbeAccess::fault(ordinal);
		try { success=ww3d_import::Attempt::load(*manager,"GENERATED.w3d"); }
		catch (const std::bad_alloc&) {}
		W3DAssetImportProbeAccess::fault(-1);
		assert(provider.gets-gets==provider.returns-returns && provider.returns-returns==provider.closes-closes);
		if (!success) {
			assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
			assert(!manager->Find_Prototype("GENERATED.FIRST") && !manager->Find_Prototype("GENERATED.SECOND"));
			assert(manager->Texture_Hash().Get("generated.sibling.tga")==sibling && sibling->Num_Refs()==1);
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live && TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw);
		}
	}
	assert(success && manager->Find_Prototype("GENERATED.FIRST") && manager->Find_Prototype("GENERATED.SECOND"));
	provider.baseline=nullptr;
	// A rejected overlap neither publishes candidates nor poisons the first owner.
	{
		ww3d_import::Attempt attempt(*manager);bool rejected=false;
		try { ww3d_import::Attempt nested(*manager); } catch (const std::runtime_error&) { rejected=true; }
		assert(rejected && attempt.commit() && !attempt.commit());
	}
	// Filter rollback is direct typed restoration, without guarded setter calls.
	{
		ww3d_import::Attempt attempt(*manager);
		for (unsigned action=0;action<5;++action) {
			bool rejected=false;
			try {
				switch(action) {
				case 0: manager->Free_Assets();break;
				case 1: manager->Release_Unused_Assets();break;
				case 2: manager->Release_All_Textures();break;
				case 3: manager->Release_Texture(sibling);break;
				case 4: manager->Remove_Prototype(manager->Find_Prototype("GENERATED.FIRST"));break;
				}
			} catch(const std::runtime_error&) { rejected=true; }
			assert(rejected && manager->Find_Prototype("GENERATED.FIRST") && sibling->Num_Refs()==1);
		}
		assert(attempt.commit());
	}
	{
		ww3d_import::Attempt attempt(*manager);
		TextureClass* same=manager->Get_Texture("GENERATED.SIBLING.TGA");assert(same==sibling);
		same->Get_Filter().Set_Min_Filter(TextureFilterClass::FILTER_TYPE_FAST);same->Release_Ref();
	}
	assert(sibling->Get_Filter().Get_Min_Filter()==filter.Get_Min_Filter());
	// Cache-hit commit must not replace any accepted registry allocation/identity.
	std::memcpy(baseline.data(),manager.get(),baseline.size());
	{ ww3d_import::Attempt attempt(*manager);assert(attempt.commit()); }
	assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
	// Missing and duplicate root imports retain native false without partial publication.
	provider.available=false;assert(!ww3d_import::Attempt::load(*manager,"MISSING.w3d"));provider.available=true;
	assert(!ww3d_import::Attempt::load(*manager,"GENERATED.w3d"));
	assert(std::memcmp(manager.get(),baseline.data(),baseline.size())==0);
	manager.reset();provider.manager=nullptr;
}
int main()
{
	try {
		initMemoryManager();WW3D::Set_Thumbnail_Enabled(false);DX8Wrapper::Reset_Source_State();
		const int live=TheMemoryPoolFactory->getLiveAllocationCount(),raw=TheDynamicMemoryAllocator->getRawUsedBlockCount();
		for (int i=0;i<2;++i) { generation();capacity_controls();mapper_argument_controls();cull_controls();material_mesh_controls();uv_capacity_controls();legacy_mesh_controls();
			edge_provider_controls();
			// Frame checkpoint restoration retains the process statistics string's
			// capacity. Retire that public process owner before exact allocator checks.
			Debug_Statistics::Shutdown_Statistics();
			assert(TheMemoryPoolFactory->getLiveAllocationCount()==live);
			assert(TheDynamicMemoryAllocator->getRawUsedBlockCount()==raw); }
		shutdownMemoryManager();std::puts("original-rendering strong bounded native asset import");return 0;
	} catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what());return 1; }
}
