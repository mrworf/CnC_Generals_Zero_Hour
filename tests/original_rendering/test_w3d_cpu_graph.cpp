#include "assetmgr.h"
#include "mesh.h"
#include "meshmdl.h"
#include "hlod.h"
#include "chunkio.h"
#include "RAMFILE.H"
#include "ffactory.h"
#include "TARGA.H"
#include "w3d_file.h"
#include "texture.h"
#include "vertmaterial.h"
#include "matpass.h"
#include "decalsys.h"
#include "decalmsh.h"
#include "mapper.h"
#include "camera.h"
#include "scene.h"
#include "light.h"
#include "lightenvironment.h"
#include "rinfo.h"
#include "dx8fvf.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "statistics.h"
#include "stripoptimizer.h"
#include "static_sort_list.h"
#include "sortingrenderer.h"
#include "ww3d.h"
#include "animatedsoundmgr.h"
#include "textureloader.h"
#include "wwmemlog.h"
#include "hanim.h"
#include "trim.h"
#include "original_gpu_edge.h"
#include "zh/original_process.h"
#include "zh/renderer/recording_device.h"
#if defined(ZH_BGFX_SHADER_DIR)
#include "zh/platform/bgfx_device.h"
#endif
#if defined(ZH_GPU_SHADER_DIR)
#include "zh/platform/sdl_gpu_device.h"
#include <SDL3/SDL.h>
#endif

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <array>

class CriticalSection;
extern CriticalSection* TheDmaCriticalSection;
extern CriticalSection* TheMemoryPoolCriticalSection;

#undef assert
#define assert(condition) do { if (!(condition)) { std::fprintf(stderr,"original W3D CPU invariant %s:%d: %s\n",__FILE__,__LINE__,#condition); std::abort(); } } while (false)

namespace {
int texture_frame_network_ticks=0;
void texture_frame_network_callback() { ++texture_frame_network_ticks; }
class OwnedFile final : public FileClass {
public:
	OwnedFile(std::string name,std::vector<unsigned char> bytes)
		:name_(std::move(name)),bytes_(std::move(bytes)) {}
	const char* File_Name() const override { return name_.c_str(); }
	const char* Set_Name(const char* name) override { name_=name; return name_.c_str(); }
	int Create() override { return 0; }
	int Delete() override { return 0; }
	bool Is_Available(int=0) override { return !bytes_.empty(); }
	bool Is_Open() const override { return open_; }
	int Open(const char* name,int rights=READ) override { Set_Name(name); return Open(rights); }
	int Open(int rights=READ) override { open_=!bytes_.empty() && rights==READ; pos_=0; return open_; }
	int Read(void* out,int count) override {
		if (!open_ || count<0) return 0;
		const size_t n=std::min(static_cast<size_t>(count),bytes_.size()-pos_);
		std::memcpy(out,bytes_.data()+pos_,n); pos_+=n; return static_cast<int>(n);
	}
	int Seek(int offset,int mode=SEEK_CUR) override {
		const auto base=mode==SEEK_SET?0LL:mode==SEEK_END?
			static_cast<long long>(bytes_.size()):static_cast<long long>(pos_);
		const auto next=base+offset;
		if (next<0 || next>static_cast<long long>(bytes_.size())) throw std::runtime_error("owned file seek");
		pos_=static_cast<size_t>(next); return static_cast<int>(pos_);
	}
	int Size() override { return static_cast<int>(bytes_.size()); }
	int Write(const void*,int) override { return 0; }
	void Close() override { open_=false; }
private:
	std::string name_; std::vector<unsigned char> bytes_; size_t pos_=0; bool open_=false;
};
class OwnedFactory final : public FileFactoryClass {
public:
	std::map<std::string,std::vector<unsigned char>> files;
	int owners=0;
	FileClass* Get_File(const char* name) override { ++owners; return new OwnedFile(name,files[name]); }
	void Return_File(FileClass* file) override { --owners; delete file; }
};
std::vector<unsigned char> original_targa()
{
	TGAHeader header{};
	header.ImageType=TGA_TRUECOLOR; header.Width=2; header.Height=2;
	header.PixelDepth=32; header.ImageDescriptor=0x28;
	std::vector<unsigned char> bytes(sizeof(header)+16+14,0);
	std::memcpy(bytes.data(),&header,sizeof(header));
	for (unsigned i=0;i<4;++i) {
		const std::array<unsigned char,4> pixel{{0,96,160,255}};
		std::memcpy(bytes.data()+sizeof(header)+4*i,pixel.data(),4);
	}
	return bytes;
}
template <typename T> void chunk(ChunkSaveClass &writer, unsigned id, const T &value)
{
	assert(writer.Begin_Chunk(id));
	assert(writer.Write(&value, sizeof(value)) == sizeof(value));
	assert(writer.End_Chunk());
}

void make_mesh(ChunkSaveClass &writer, bool supply_variant, bool tread_variant = false,
	bool skin_variant = false, unsigned texture_stages = 1, bool lit_uv_variant = false,
	bool invalid_skin = false, unsigned large_vertex_count = 0, bool batch_second = false,
	bool cull_tree_variant = false)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_MESH));
	W3dMeshHeader3Struct header{};
	header.Version = W3D_CURRENT_MESH_VERSION;
	std::strcpy(header.MeshName, cull_tree_variant ? "CULLTREE" :
		lit_uv_variant ? (texture_stages==2 ? "LITTWO01" : "LITONE01") :
		batch_second ? "SKIN02" : invalid_skin ? "BADSKIN" :
		texture_stages == 0 && !skin_variant ? "ZERO01" : texture_stages == 2 && !skin_variant ? "TWO01" :
		skin_variant ? "SKIN01" : tread_variant ? "TREADSL01" :
		(supply_variant ? "SUPPLY01" : "TRIANGLE"));
	if (skin_variant) header.Attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN;
	if (cull_tree_variant) header.Attributes |= W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL;
	std::strcpy(header.ContainerName, "TEST");
	header.NumVertices = large_vertex_count ? large_vertex_count : 3;
	header.NumTris = 1;
	header.Min = {0, 0, 0};
	header.Max = {1, 1, 0};
	header.SphCenter = {0.5f, 0.5f, 0};
	header.SphRadius = 1;
	chunk(writer, W3D_CHUNK_MESH_HEADER3, header);
	std::vector<W3dVectorStruct> vertices(header.NumVertices);
	vertices[0]={0,0,0}; vertices[1]={1,0,0}; vertices[2]={0,1,0};
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTICES));
	assert(writer.Write(vertices.data(), vertices.size()*sizeof(vertices[0])) ==
		static_cast<int>(vertices.size()*sizeof(vertices[0])));
	assert(writer.End_Chunk());
	std::vector<W3dVectorStruct> normals(header.NumVertices,{0,0,1});
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_NORMALS));
	assert(writer.Write(normals.data(), normals.size()*sizeof(normals[0])) ==
		static_cast<int>(normals.size()*sizeof(normals[0])));
	assert(writer.End_Chunk());
	W3dTriStruct triangle{};
	triangle.Vindex[0] = 0;
	triangle.Vindex[1] = 1;
	triangle.Vindex[2] = 2;
	triangle.Normal = {0, 0, 1};
	chunk(writer, W3D_CHUNK_TRIANGLES, triangle);
	if (skin_variant)
	{
		std::vector<W3dVertInfStruct> links(header.NumVertices);
		if (invalid_skin) links[0].BoneIdx=7;
		assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_INFLUENCES));
		assert(writer.Write(links.data(), links.size()*sizeof(links[0])) ==
			static_cast<int>(links.size()*sizeof(links[0])));
		assert(writer.End_Chunk());
	}
	W3dMaterialInfoStruct material_info{};
	material_info.PassCount = 1;
	material_info.ShaderCount = 1;
	material_info.VertexMaterialCount = 1;
	material_info.TextureCount = texture_stages;
	chunk(writer, W3D_CHUNK_MATERIAL_INFO, material_info);
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIALS));
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL));
	const char material_name[] = "LIT";
	chunk(writer, W3D_CHUNK_VERTEX_MATERIAL_NAME, material_name);
	W3dVertexMaterialStruct material{};
	W3d_Vertex_Material_Reset(&material);
	material.Opacity = 0.75f;
	material.Attributes = (tread_variant ? W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET :
		W3DVERTMAT_STAGE0_MAPPING_SCREEN) |
		(texture_stages == 2 ? W3DVERTMAT_STAGE1_MAPPING_SCREEN : 0);
	chunk(writer, W3D_CHUNK_VERTEX_MATERIAL_INFO, material);
	assert(writer.End_Chunk());
	assert(writer.End_Chunk());
	if (texture_stages) {
		assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURES));
		for (unsigned stage = 0; stage < texture_stages; ++stage) {
			assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURE));
			const char *texture_name = stage == 0 ? "MYTEX.TGA" : "MYTEX2.TGA";
			assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME));
			assert(writer.Write(texture_name, std::strlen(texture_name) + 1) ==
				static_cast<int>(std::strlen(texture_name) + 1));
			assert(writer.End_Chunk());
			W3dTextureInfoStruct texture_info{};
			texture_info.Attributes = W3DTEXTURE_NO_LOD | W3DTEXTURE_CLAMP_U;
			chunk(writer, W3D_CHUNK_TEXTURE_INFO, texture_info);
			assert(writer.End_Chunk());
		}
		assert(writer.End_Chunk());
	}
	W3dShaderStruct shader{};
	W3d_Shader_Reset(&shader);
	W3d_Shader_Set_Dest_Blend_Func(&shader, W3DSHADER_DESTBLENDFUNC_ONE);
	W3d_Shader_Set_Texturing(&shader, texture_stages ? W3DSHADER_TEXTURING_ENABLE :
		W3DSHADER_TEXTURING_DISABLE);
	if (texture_stages == 2) W3d_Shader_Set_Detail_Color_Func(&shader, W3DSHADER_DETAILCOLORFUNC_ADD);
	if (skin_variant) W3d_Shader_Set_Src_Blend_Func(&shader, W3DSHADER_SRCBLENDFUNC_SRC_ALPHA);
	chunk(writer, W3D_CHUNK_SHADERS, shader);
	assert(writer.Begin_Chunk(W3D_CHUNK_MATERIAL_PASS));
	const uint32 shader_index = 0;
	chunk(writer, W3D_CHUNK_SHADER_IDS, shader_index);
	chunk(writer, W3D_CHUNK_VERTEX_MATERIAL_IDS, shader_index);
	for (unsigned stage = 0; stage < texture_stages; ++stage) {
		assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURE_STAGE));
		chunk(writer, W3D_CHUNK_TEXTURE_IDS, stage);
		if (lit_uv_variant || (tread_variant && stage==0)) {
			W3dTexCoordStruct texcoords[3]={{0,0},{1,0},{0,1}};
			if (stage==1) { texcoords[0]={0.25f,0}; texcoords[1]={1,0.25f}; }
			chunk(writer,W3D_CHUNK_STAGE_TEXCOORDS,texcoords);
		}
		assert(writer.End_Chunk());
	}
	assert(writer.End_Chunk());
	assert(writer.End_Chunk());
}

void make_hierarchy(ChunkSaveClass &writer, bool supply_variant)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_HIERARCHY));
	W3dHierarchyStruct header{};
	header.Version = W3D_CURRENT_HTREE_VERSION;
	std::strcpy(header.Name, "TESTTREE");
	header.NumPivots = supply_variant ? 5 : 1;
	chunk(writer, W3D_CHUNK_HIERARCHY_HEADER, header);
	W3dPivotStruct pivot{};
	std::strcpy(pivot.Name, supply_variant ? "SUPPLY01" : "ROOT");
	pivot.ParentIdx = 0xffffffffu;
	pivot.Rotation.Q[0] = 1.0f;
	if (supply_variant)
	{
		W3dPivotStruct pivots[5]{};
		pivots[0] = pivot;
		const char *names[] = {"TIRE_FL", "TIRE_FR", "TIRE_RL", "TIRE_RR"};
		for (int i = 1; i != 5; ++i)
		{
			std::strcpy(pivots[i].Name, names[i - 1]);
			pivots[i].ParentIdx = 0;
			pivots[i].Rotation.Q[0] = 1.0f;
		}
		chunk(writer, W3D_CHUNK_PIVOTS, pivots);
	}
	else chunk(writer, W3D_CHUNK_PIVOTS, pivot);
	assert(writer.End_Chunk());
}

void make_animation(ChunkSaveClass &writer)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_ANIMATION));
	W3dAnimHeaderStruct header{};
	header.Version = W3D_CURRENT_HANIM_VERSION;
	std::strcpy(header.Name, "IDLE");
	std::strcpy(header.HierarchyName, "TESTTREE");
	header.NumFrames = 2;
	header.FrameRate = 30;
	chunk(writer, W3D_CHUNK_ANIMATION_HEADER, header);
	assert(writer.End_Chunk());
}

void make_hlod(ChunkSaveClass &writer, bool supply_variant, bool skin_variant = false,
	bool invalid_skin = false, bool batch_skin = false)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_HLOD));
	W3dHLodHeaderStruct header{};
	header.Version = W3D_CURRENT_HLOD_VERSION;
	header.LodCount = 1;
	std::strcpy(header.Name, batch_skin ? "TEST.BATCHHLOD" : invalid_skin ? "TEST.BADHLOD" :
		(skin_variant ? "TEST.SKINHLOD" : "TEST.HLOD"));
	std::strcpy(header.HierarchyName, "TESTTREE");
	chunk(writer, W3D_CHUNK_HLOD_HEADER, header);
	assert(writer.Begin_Chunk(W3D_CHUNK_HLOD_LOD_ARRAY));
	W3dHLodArrayHeaderStruct array{};
	array.ModelCount = (supply_variant || batch_skin) ? 2 : 1;
	array.MaxScreenSize = 1.0f;
	chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, array);
	W3dHLodSubObjectStruct subobject{};
	std::strcpy(subobject.Name, invalid_skin ? "TEST.BADSKIN" : skin_variant ? "TEST.SKIN01" :
		(supply_variant ? "TEST.SUPPLY01" : "TEST.TRIANGLE"));
	chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT, subobject);
	if (supply_variant)
	{
		W3dHLodSubObjectStruct tread{};
		std::strcpy(tread.Name, "TEST.TREADSL01");
		chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT, tread);
	}
	if (batch_skin) {
		W3dHLodSubObjectStruct second{};
		std::strcpy(second.Name,"TEST.SKIN02");
		chunk(writer,W3D_CHUNK_HLOD_SUB_OBJECT,second);
	}
	assert(writer.End_Chunk());
	assert(writer.End_Chunk());
}

int test_original_skin_batch()
{
	// Two authentic HLOD skin children exceed the original 16-bit batch cap
	// together while each child remains below it. No forged container or VB.
	std::vector<char> bytes(4U*1024U*1024U);
	RAMFileClass file(bytes.data(),static_cast<int>(bytes.size()));
	assert(file.Open(FileClass::WRITE));
	ChunkSaveClass writer(&file);
	make_hierarchy(writer,false);
	make_mesh(writer,false,false,true,0,false,false,32769);
	make_mesh(writer,false,false,true,0,false,false,32769,true);
	make_hlod(writer,false,true,false,true);
	const int size=file.Size();
	file.Close();
	WW3DAssetManager manager;
	RAMFileClass input(bytes.data(),size);
	assert(manager.Load_3D_Assets(input));
	RenderObjClass* hlod=manager.Create_Render_Obj("TEST.BATCHHLOD");
	assert(hlod && hlod->Get_HTree() && hlod->Get_Num_Sub_Objects()==2);
	CameraClass camera;
	RenderInfoClass render_info(camera);
	TheDX8MeshRenderer.Init();
	TheDX8MeshRenderer.Set_Camera(&camera);
	for (unsigned i=0;i<2;++i) {
		RenderObjClass* child=hlod->Get_Sub_Object(i);
		assert(child && child->Class_ID()==RenderObjClass::CLASSID_MESH);
		auto* skin=static_cast<MeshClass*>(child);
		assert(skin->Peek_Model()->Get_Vertex_Count()==32769);
		skin->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		skin->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		skin->Peek_Model()->Register_For_Rendering();
		child->Release_Ref();
	}
	hlod->Set_Position(Vector3(0,0,-10));
	zh::renderer::RecordingGpuDevice recorder(32);
	zh::renderer::TextureDesc target;
	target.width=32; target.height=32; target.render_target=true; target.sampled=false;
	const auto color=recorder.create_texture(target,"source 16-bit skin batch color");
	target.format=zh::renderer::TextureFormat::depth24_stencil8;
	const auto depth=recorder.create_texture(target,"source 16-bit skin batch depth");
	zh::renderer::RenderPassDesc pass;
	pass.color_targets[0]=color; pass.color_target_count=1;
	pass.depth_target=depth; pass.width=32; pass.height=32;
	{
		zh::original_runtime::OriginalGpuEdge edge(recorder);
		DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
		Debug_Statistics::Begin_Statistics();
		hlod->Render(render_info);
		assert(recorder.begin_pass(pass,"source 65535 skin batch partition"));
		TheDX8MeshRenderer.Flush();
		assert(recorder.end_pass());
		Debug_Statistics::End_Statistics();
		assert(Debug_Statistics::Get_DX8_Skin_Renders()==2 &&
			Debug_Statistics::Get_DX8_Skin_Vertices()==65538);
		const auto commands=recorder.snapshot();
		const auto first=commands.find("draw pipeline=");
		const auto second=commands.find("draw pipeline=",first+1);
		assert(first!=std::string::npos && second!=std::string::npos &&
			commands.find("draw pipeline=",second+1)==std::string::npos &&
			commands.find("index_bits=16",first)!=std::string::npos &&
			commands.find("index_bits=16",second)!=std::string::npos);
		TheDX8MeshRenderer.Invalidate();
		TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
	}
	recorder.destroy(depth); recorder.destroy(color);
	assert(recorder.resource_counts().total()==0);
	hlod->Release_Ref();
	manager.Free_Assets();
	std::puts("original HLOD skin 16-bit partition: 2 source draws/65538 vertices");
	return 0;
}
}

int main(int argc, char **argv)
{
	if (argc==2 && std::strcmp(argv[1],"--load-w3d-stdin")==0) {
		std::vector<char> input;
		char block[4096];
		for (;;) {
			const auto count=std::fread(block,1,sizeof block,stdin);
			if (input.size()+count>32U*1024U*1024U) return 2;
			input.insert(input.end(),block,block+count);
			if (count<sizeof block) {
				if (std::ferror(stdin) || input.empty()) return 2;
				break;
			}
		}
		WW3DAssetManager manager;
		RAMFileClass source(input.data(),static_cast<int>(input.size()));
		const bool accepted=manager.Load_3D_Assets(source);
		manager.Free_Assets();
		std::puts(accepted ? "accepted" : "rejected");
		return 0;
	}
	struct ProcessServicesGuard {
		bool active=false;
		~ProcessServicesGuard() {
			if (!active) return;
			zh::original_process::shutdown_services();
			assert(!TheDmaCriticalSection && !TheMemoryPoolCriticalSection);
		}
	} services;
	// Keep every threaded source-GPU mode here: original allocator locks must
	// exist before bgfx workers, asset loading, or device-owned allocations.
	const bool threaded_source_device=argc==2 &&
		(std::strcmp(argv[1],"--bgfx-source-static-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-owned-pass-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-fault-retry")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-viewport-clear")==0);
	if (threaded_source_device) {
		assert(!TheDmaCriticalSection && !TheMemoryPoolCriticalSection);
		char reason[128]{};
		services.active=zh::original_process::initialize_services(-1,reason,sizeof reason);
		assert(services.active && TheDmaCriticalSection && TheMemoryPoolCriticalSection);
	}
	if (argc==2 && std::strcmp(argv[1],"--skin-batch")==0)
		return test_original_skin_batch();
	// Keep the original strip provider runtime-witnessed even in optimized
	// builds where the mesh path can inline away its only other reference.
	const int source_strips[] = {3, 0, 1, 2, 2, 3, 4};
	assert(StripOptimizerClass::Get_Strip_Index_Count(source_strips, 2) == 5);
	std::vector<char> bytes(16384);
	RAMFileClass file(bytes.data(), static_cast<int>(bytes.size()));
	assert(file.Open(FileClass::WRITE));
	ChunkSaveClass writer(&file);
	const bool supply_variant = argc == 3 && std::strcmp(argv[1], "--emit") == 0;
	const bool focused_static_scene = argc==2 &&
		(std::strcmp(argv[1],"--source-static-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-static-scene")==0 ||
		 std::strcmp(argv[1],"--source-mixed-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-scene")==0 ||
		 std::strcmp(argv[1],"--source-owned-pass-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-owned-pass-scene")==0 ||
		 std::strcmp(argv[1],"--source-mixed-fault-matrix")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-fault-retry")==0);
	const bool focused_mixed_scene = argc==2 &&
		(std::strcmp(argv[1],"--source-mixed-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-scene")==0 ||
		 std::strcmp(argv[1],"--source-owned-pass-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-owned-pass-scene")==0 ||
		 std::strcmp(argv[1],"--source-mixed-fault-matrix")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-fault-retry")==0);
	const bool focused_owned_pass_scene = argc==2 &&
		(std::strcmp(argv[1],"--source-owned-pass-scene")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-owned-pass-scene")==0);
	const bool focused_fault_scene = argc==2 &&
		(std::strcmp(argv[1],"--source-mixed-fault-matrix")==0 ||
		 std::strcmp(argv[1],"--bgfx-source-mixed-fault-retry")==0);
	if (focused_static_scene) {
		// This gate owns only the three original W3D mesh families it renders.
		make_mesh(writer,false); // TEST.TRIANGLE
		make_mesh(writer,false,false,false,0); // TEST.ZERO01
		make_mesh(writer,true); // TEST.SUPPLY01
		if (focused_mixed_scene) {
			make_hierarchy(writer,false);
			make_mesh(writer,false,false,true,0); // TEST.SKIN01
			make_hlod(writer,false,true); // TEST.SKINHLOD
			make_mesh(writer,false,false,false,2); // TEST.TWO01
			if (focused_owned_pass_scene)
				make_mesh(writer,false,false,false,0,false,false,0,false,true); // TEST.CULLTREE
		}
	} else {
	make_hierarchy(writer, supply_variant);
	make_animation(writer);
	make_mesh(writer, supply_variant);
	if (supply_variant) make_mesh(writer, true, true);
	else make_mesh(writer, false, false, true);
	if (!supply_variant) make_mesh(writer,false,false,true,1,false,true);
	if (argc == 2 && (std::strcmp(argv[1], "--device-edge") == 0 ||
		std::strcmp(argv[1], "--vulkan-category") == 0 ||
		std::strcmp(argv[1], "--vulkan-sorting") == 0)) {
		make_mesh(writer, false, false, false, 0);
		make_mesh(writer, false, false, false, 2);
		make_mesh(writer, false, true);
		make_mesh(writer, false, false, false, 1, true);
		make_mesh(writer, false, false, false, 2, true);
		make_mesh(writer, false, false, false, 0, false, false, 0, false, true);
	}
	make_hlod(writer, supply_variant);
	if (!supply_variant) {
		make_hlod(writer,false,true);
		make_hlod(writer,false,true,true);
	}
	}
	const int size = file.Size();
	file.Close();
	if (argc == 3 && std::strcmp(argv[1], "--emit") == 0)
	{
		FILE *output = std::fopen(argv[2], "wb");
		assert(output != nullptr);
		assert(std::fwrite(bytes.data(), 1, size, output) == static_cast<std::size_t>(size));
		assert(std::fclose(output) == 0);
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--texture-frame-missing-manager")==0) {
		const bool previous=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(true);
		bool missing=false;
		try { TextureLoader::Update(nullptr); }
		catch (const std::runtime_error&) { missing=true; }
		assert(missing);
		WW3D::Set_Thumbnail_Enabled(previous);
		return 0;
	}
	WW3DAssetManager manager;
	RAMFileClass input(bytes.data(), size);
	assert(manager.Load_3D_Assets(input));
	if (focused_static_scene) {
		const bool physical=std::strcmp(argv[1],"--bgfx-source-static-scene")==0 ||
			std::strcmp(argv[1],"--bgfx-source-mixed-scene")==0 ||
			std::strcmp(argv[1],"--bgfx-source-owned-pass-scene")==0 ||
			std::strcmp(argv[1],"--bgfx-source-mixed-fault-retry")==0;
		OwnedFactory factory;
		factory.files["mytex.tga"]=original_targa();
		factory.files["MYTEX.TGA"]=factory.files["mytex.tga"];
		if (focused_mixed_scene) {
			factory.files["mytex2.tga"]=factory.files["mytex.tga"];
			factory.files["MYTEX2.TGA"]=factory.files["mytex.tga"];
		}
		auto* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		manager.Free_Assets(); // Each WW3D::Shutdown owns its asset-manager generation.
		struct LevelHook final : RenderHookClass {
			int level;
			std::vector<int>& drained;
			LevelHook(int value,std::vector<int>& out):level(value),drained(out) {}
			bool Pre_Render(RenderObjClass*,RenderInfoClass&) override {
				if (!WW3D::Are_Static_Sort_Lists_Enabled()) drained.push_back(level);
				return true;
			}
			void Post_Render(RenderObjClass*,RenderInfoClass&) override {}
		};
		struct OwnedPassHook final : RenderHookClass {
			enum Mode { disabled, immediate, delayed, culled, culled_outside } mode=disabled;
			MaterialPassClass* pass=NEW_REF(MaterialPassClass,());
			OBBoxClass inside{Vector3(0,0,-10),Vector3(100,100,100),Matrix3x3(true)};
			OBBoxClass outside{Vector3(1000,1000,-10),Vector3(1,1,1),Matrix3x3(true)};
			OwnedPassHook() {
				auto* material=NEW_REF(VertexMaterialClass,());
				material->Set_Lighting(true);
				material->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
				material->Set_Diffuse(Vector3(0,0,0));
				material->Set_Emissive(Vector3(0,1,0));
				pass->Set_Material(material);
				material->Release_Ref();
				ShaderClass shader;
				shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
				shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
				pass->Set_Shader(shader);
			}
			~OwnedPassHook() override { pass->Release_Ref(); }
			bool Pre_Render(RenderObjClass*,RenderInfoClass& info) override {
				if (mode==disabled) return true;
				pass->Set_Cull_Volume(mode==culled ? &inside :
					mode==culled_outside ? &outside : nullptr);
				info.Push_Material_Pass(pass);
				if (mode==delayed) info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
				return true;
			}
			void Post_Render(RenderObjClass*,RenderInfoClass& info) override {
				if (mode==disabled) return;
				if (mode==delayed) info.Pop_Override_Flags();
				info.Pop_Material_Pass();
			}
		};
		auto run=[&](zh::renderer::GpuDevice& device,auto readback,unsigned width,
			zh::renderer::TextureFormat color_format) {
			const char* control=std::getenv("ZH_M22_STATIC_CONTROL");
			const bool rigid_only=control && std::strcmp(control,"rigid_only")==0;
			assert(!control || rigid_only);
			const unsigned height=width*3/4;
			zh::renderer::TextureDesc target;
			target.width=width; target.height=height; target.render_target=true;
			target.format=color_format;
			const auto color=device.create_texture(target,"original source static scene color");
			assert(device.describe_texture_format(color)==color_format);
			target.format=zh::renderer::TextureFormat::depth24_stencil8;
			auto depth=device.create_texture(target,"original source static scene depth");
			assert(color && depth);
			std::vector<unsigned char> baseline,with_both,without_one;
			std::vector<unsigned char> with_skin,with_decal,with_sorted,with_front,with_back,with_mixed;
			std::vector<unsigned char> pass_base,pass_immediate,pass_delayed,pass_culled,pass_outside;
			std::vector<unsigned char> pass_recovered,pass_removed;
			{
				zh::original_runtime::OriginalGpuEdge edge(device);
				assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
				RAMFileClass generated(bytes.data(),size);
				assert(manager.Load_3D_Assets(generated));
				auto* rigid=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.TRIANGLE"));
				auto* level1=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.ZERO01"));
				auto* level2=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.SUPPLY01"));
				assert(rigid && level1 && level2);
				RenderObjClass* skin_hlod=nullptr;
				MeshClass* sorted=nullptr;
				MeshClass* front=nullptr;
				if (focused_mixed_scene) {
					skin_hlod=manager.Create_Render_Obj("TEST.SKINHLOD");
					sorted=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.TWO01"));
					front=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.ZERO01"));
					assert(skin_hlod && sorted && front && skin_hlod->Get_HTree());
					auto* skin_child=skin_hlod->Get_Sub_Object(0);
					assert(skin_child && skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
					static_cast<MeshClass*>(skin_child)->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
					auto* skin_model=static_cast<MeshClass*>(skin_child)->Peek_Model();
					auto* skin_material=skin_model->Peek_Single_Material();
					skin_material->Set_Lighting(false);
					skin_material->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
					skin_material->Set_Diffuse(Vector3(0,1,0));
					ShaderClass skin_shader;
					skin_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
					skin_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
						skin_model->Set_Single_Shader(skin_shader);
						skin_child->Release_Ref();
						skin_hlod->Set_ObjectScale(5.0f);
						skin_hlod->Set_Position(Vector3(0,0,-10));
						sorted->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,true);
						sorted->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
						sorted->Set_Position(Vector3(1,0,-10));
						WW3D::Enable_Sorting(true);
				}
				for (auto* mesh:{rigid,level1,level2}) {
					mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
					mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
				}
				rigid->Set_Position(Vector3(-1,0,-10));
				level1->Set_Position(Vector3(0,0,-10));
				level2->Set_Position(Vector3(1,0,-10));
				level1->Set_Sort_Level(1);
				level2->Set_Sort_Level(2);
				std::vector<int> drained;
				level1->Set_Render_Hook(new LevelHook(1,drained));
				level2->Set_Render_Hook(new LevelHook(2,drained));
				edge.bind_frame_targets(color,depth,width,height);
				CameraClass camera;
				camera.Set_Clip_Planes(1,100);
				camera.Set_Viewport(Vector2(0.125f,0.125f),Vector2(0.875f,0.875f));
				SimpleSceneClass scene;
				scene.Add_Render_Object(rigid);
				WW3D::Enable_Static_Sort_Lists(true);
					auto frame=[&]() {
						assert(WW3D::Begin_Render(true,true,Vector3(0.8f,0.1f,0.1f),1)==WW3D_ERROR_OK);
						if (focused_mixed_scene && !physical) {
							bool refused=false;
							try { edge.release_source_buffers(); }
							catch (const std::runtime_error&) { refused=true; }
							assert(refused); // An active source frame still owns its bound buffers.
						}
				assert(WW3D::Render(&scene,&camera,true,true,Vector3(0.05f,0.05f,0.2f))==WW3D_ERROR_OK);
				assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
				return readback(color);
				};
				baseline=frame();
				if (focused_fault_scene) {
					if (!physical) {
						auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
						assert(recorder);
						bool bad_clear=false;
						try { (void)WW3D::Begin_Render(true,true,Vector3(1.2f,0,0),1); }
						catch (const std::runtime_error&) { bad_clear=true; }
						assert(bad_clear && !WW3D::Is_Rendering() && !recorder->pass_active());
						assert(frame().empty());
					}
					device.destroy(depth);
					bool stale=false;
					try { (void)frame(); }
					catch (const std::runtime_error&) { stale=true; }
					assert(stale && !WW3D::Is_Rendering());
					if (!physical) {
						auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
						assert(recorder && !recorder->pass_active());
					}
					depth=device.create_texture(target,"recreated original scene depth");
					assert(depth);
					edge.bind_frame_targets(color,depth,width,height);
					const auto recovered=frame();
					assert(physical ? recovered==baseline : recovered.empty());
				}
				auto reject_scene_draw=[&](const char* stage) {
					assert(focused_fault_scene && !physical);
					auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
					assert(recorder);
					const auto marker=recorder->snapshot().size();
					edge.record_source_state(stage);
					recorder->fail_draw_after(1); // First rigid succeeds; next category/static/sort draw fails.
					bool rejected=false;
					try { (void)frame(); }
					catch (const std::runtime_error& error) {
						rejected=std::strstr(error.what(),"draw")!=nullptr;
					}
					const auto failure=recorder->snapshot().substr(marker);
					assert(rejected && failure.find(stage)!=std::string::npos &&
						failure.find("injected physical draw failure")!=std::string::npos &&
						!WW3D::Is_Rendering() && !recorder->pass_active());
					RenderStateStruct released;
					DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
					DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
					DX8Wrapper::Get_Render_State(released);
					assert(!released.vertex_buffers[0] && !released.index_buffer);
					assert(rigid->Num_Refs()==2);
					if (std::strcmp(stage,"static")==0)
						assert(level1->Num_Refs()==2 && level2->Num_Refs()==2);
					else if (std::strcmp(stage,"category")==0)
						assert(skin_hlod && skin_hlod->Num_Refs()==2);
					else if (std::strcmp(stage,"sorting")==0)
						assert(sorted && sorted->Num_Refs()==2);
					else assert(false);
					if (std::strcmp(stage,"static")==0) drained.clear();
					assert(frame().empty());
					if (std::strcmp(stage,"static")==0)
						assert(drained.size()==2 && drained[0]==2 && drained[1]==1);
				};
				if (!physical) {
					auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
					assert(recorder);
					recorder->fail_next_draw();
					assert(WW3D::Begin_Render(true,true,Vector3(0.8f,0.1f,0.1f),1)==WW3D_ERROR_OK);
					bool failed=false;
					try { WW3D::Render(&scene,&camera,true,true,Vector3(0.05f,0.05f,0.2f)); }
					catch (const std::runtime_error& error) {
						failed=std::strstr(error.what(),"draw")!=nullptr;
					}
					assert(failed && !recorder->pass_active());
					RenderStateStruct released;
					DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
					DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
					DX8Wrapper::Get_Render_State(released);
					assert(!released.vertex_buffers[0] && !released.index_buffer &&
						VertexBufferClass::Get_Total_Buffer_Count()==1 &&
						IndexBufferClass::Get_Total_Buffer_Count()==1);
					assert(frame().empty()); // Fresh source registration and frame retry.
				}
				if (!rigid_only) {
					scene.Add_Render_Object(level1);
					scene.Add_Render_Object(level2);
					drained.clear();
					with_both=frame();
					assert(drained.size()==2 && drained[0]==2 && drained[1]==1);
					if (focused_fault_scene && !physical) reject_scene_draw("static");
					scene.Remove_Render_Object(level1);
					drained.clear();
					without_one=frame();
					assert(drained.size()==1 && drained[0]==2);
					scene.Remove_Render_Object(level2);
				}
				if (focused_mixed_scene) {
					assert(skin_hlod && sorted && front);
					const bool old_decals=WW3D::Are_Decals_Enabled();
					WW3D::Enable_Decals(true);
					DecalSystemClass decal_system;
					auto* generator=decal_system.Lock_Decal_Generator();
					assert(generator);
					auto* decal_material=generator->Get_Material();
					assert(decal_material);
					auto* vertex_material=NEW_REF(VertexMaterialClass,());
					vertex_material->Set_Lighting(true);
					vertex_material->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
					vertex_material->Set_Diffuse(Vector3(0.0f,0.0f,0.0f));
					vertex_material->Set_Emissive(Vector3(0.0f,1.0f,0.0f));
					decal_material->Set_Material(vertex_material);
					vertex_material->Release_Ref();
					ShaderClass decal_shader;
					decal_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
					decal_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
					decal_material->Set_Shader(decal_shader);
					decal_material->Release_Ref();
					generator->Set_Ortho_Projection(-2,2,-2,2,0,20);
					generator->Set_Transform(Matrix3D(true));
					generator->Set_Backface_Threshhold(-1.0f);
					generator->Apply_To_Translucent_Meshes(true);
					rigid->Create_Decal(generator);
					assert(generator->Get_Mesh_List().Peek_Head()==rigid);
					edge.record_source_state("fixture mixed decal frame");
					with_decal=frame();
					scene.Add_Render_Object(level1);
					scene.Add_Render_Object(level2);
					scene.Add_Render_Object(skin_hlod);
					scene.Add_Render_Object(sorted);
					drained.clear();
					edge.record_source_state("fixture mixed all frame");
					with_mixed=frame();
					assert(drained.size()==2 && drained[0]==2 && drained[1]==1);
					scene.Remove_Render_Object(sorted);
					scene.Remove_Render_Object(skin_hlod);
					scene.Remove_Render_Object(level2);
					scene.Remove_Render_Object(level1);
					const uint32 decal_id=generator->Get_Decal_ID();
					decal_system.Unlock_Decal_Generator(generator);
					rigid->Delete_Decal(decal_id);
					WW3D::Enable_Decals(old_decals);
					scene.Add_Render_Object(skin_hlod);
					edge.record_source_state("fixture mixed skin frame");
					with_skin=frame();
					if (focused_fault_scene && !physical) reject_scene_draw("category");
					scene.Remove_Render_Object(skin_hlod);
					scene.Add_Render_Object(sorted);
					edge.record_source_state("fixture mixed sort frame");
					with_sorted=frame();
					if (focused_fault_scene && !physical) reject_scene_draw("sorting");
					scene.Remove_Render_Object(sorted);
					front->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
					front->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
					front->Set_Position(Vector3(-1,0,-9));
					scene.Add_Render_Object(front);
					with_front=frame();
					front->Set_Position(Vector3(-1,0,-11));
					with_back=frame();
					scene.Remove_Render_Object(front);
					if (focused_owned_pass_scene) {
						assert(manager.Create_Render_Obj("TEST.MISSING")==nullptr);
						auto* pass_mesh=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.CULLTREE"));
						assert(pass_mesh && pass_mesh->Peek_Model()->Has_Cull_Tree());
						pass_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
						pass_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
						pass_mesh->Set_Position(Vector3(0,0,-10));
						auto* hook=new OwnedPassHook();
						pass_mesh->Set_Render_Hook(hook);
						scene.Add_Render_Object(pass_mesh);
						edge.record_source_state("fixture owned base pass");
						pass_base=frame();
						hook->mode=OwnedPassHook::immediate;
						edge.record_source_state("fixture owned immediate pass");
						pass_immediate=frame();
						hook->mode=OwnedPassHook::delayed;
						edge.record_source_state("fixture owned delayed pass");
						pass_delayed=frame();
						if (!physical) {
							auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
							assert(recorder && pass_mesh->Num_Refs()==2 && hook->pass->Num_Refs()==1);
							edge.record_source_state("fixture owned delayed draw reject");
							recorder->fail_draw_after(1);
							bool rejected=false;
							try { (void)frame(); }
							catch (const std::runtime_error& error) {
								rejected=std::strstr(error.what(),"draw")!=nullptr;
							}
							assert(rejected && !WW3D::Is_Rendering() && !recorder->pass_active() &&
								pass_mesh->Num_Refs()==2 && hook->pass->Num_Refs()==1);
							(void)frame();
							assert(pass_mesh->Num_Refs()==2 && hook->pass->Num_Refs()==1);
						}
						const bool old_culling=MaterialPassClass::Is_Per_Polygon_Culling_Enabled();
						MaterialPassClass::Enable_Per_Polygon_Culling(true);
						hook->mode=OwnedPassHook::culled;
						edge.record_source_state("fixture owned cull pass");
						pass_culled=frame();
						if (!physical) {
							auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
							assert(recorder);
							for (unsigned failure=0;failure<2;++failure) {
								const auto marker=recorder->snapshot().size();
								edge.record_source_state(failure==0 ?
									"fixture owned cull upload reject" : "fixture owned cull draw reject");
								if (failure==0) recorder->fail_next_buffer_upload();
								else recorder->fail_draw_after(2);
								bool rejected=false;
								try { (void)frame(); }
								catch (const std::runtime_error& error) {
									rejected=std::strstr(error.what(),failure==0 ? "upload" : "draw")!=nullptr;
								}
								const auto trace=recorder->snapshot().substr(marker);
								assert(rejected && trace.find("fixture owned cull")!=std::string::npos &&
									!WW3D::Is_Rendering() && !recorder->pass_active() &&
									pass_mesh->Num_Refs()==2 && hook->pass->Num_Refs()==1);
								(void)frame(); // Original scene rebuilds its owned pass after an aborted frame.
								assert(pass_mesh->Num_Refs()==2 && hook->pass->Num_Refs()==1);
							}
						}
						hook->mode=OwnedPassHook::culled_outside;
						edge.record_source_state("fixture owned outside cull pass");
						pass_outside=frame();
						device.destroy(depth);
						bool stale_target=false;
						try { (void)frame(); }
						catch (const std::runtime_error&) { stale_target=true; }
						assert(stale_target && !WW3D::Is_Rendering());
						if (!physical) {
							auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
							assert(recorder && !recorder->pass_active());
						}
						depth=device.create_texture(target,"rebound owned-pass depth");
						assert(depth);
						edge.bind_frame_targets(color,depth,width,height);
						edge.record_source_state("fixture owned stale target recovery");
						pass_recovered=frame();
						MaterialPassClass::Enable_Per_Polygon_Culling(old_culling);
						scene.Remove_Render_Object(pass_mesh);
						assert(pass_mesh->Num_Refs()==1);
						pass_mesh->Release_Ref();
						edge.record_source_state("fixture owned pass removed");
						pass_removed=frame();
					}
					assert(skin_hlod->Num_Refs()==1 && sorted->Num_Refs()==1 && front->Num_Refs()==1);
					front->Release_Ref(); sorted->Release_Ref(); skin_hlod->Release_Ref();
				}
				scene.Remove_Render_Object(rigid);
				assert(rigid->Num_Refs()==1 && level1->Num_Refs()==1 && level2->Num_Refs()==1);
				level2->Release_Ref(); level1->Release_Ref(); rigid->Release_Ref();
				assert(WW3D::Shutdown()==WW3D_ERROR_OK);
			}
			if (physical) {
#if defined(ZH_BGFX_SHADER_DIR)
				auto* bgfx=dynamic_cast<zh::renderer::BgfxGpuDevice*>(&device);
				assert(bgfx && bgfx->live_resource_count()==2 &&
					VertexBufferClass::Get_Total_Buffer_Count()==0 &&
					IndexBufferClass::Get_Total_Buffer_Count()==0);
#endif
				assert(baseline.size()==std::size_t(width)*height*4 &&
					(rigid_only || (with_both.size()==baseline.size() && without_one.size()==baseline.size())));
				if (focused_mixed_scene) assert(with_skin.size()==baseline.size() &&
					with_decal.size()==baseline.size() && with_sorted.size()==baseline.size() &&
					with_front.size()==baseline.size() && with_back.size()==baseline.size() &&
					with_mixed.size()==baseline.size());
				const auto outer=(height-1)*width*4U+(width-1)*4U;
				assert(baseline[outer]==204 && baseline[outer+1]==25 && baseline[outer+2]==25);
				unsigned rigid_pixels=0,static_pixels=0,level1_pixels=0;
				for (unsigned y=height/8;y<height*7/8;++y)
					for (unsigned x=width/8;x<width*7/8;++x) {
						const auto i=(y*width+x)*4U;
						if (baseline[i]!=12 || baseline[i+1]!=12 || baseline[i+2]!=51) ++rigid_pixels;
						if (!rigid_only && (baseline[i]!=with_both[i] || baseline[i+1]!=with_both[i+1] || baseline[i+2]!=with_both[i+2])) ++static_pixels;
						if (!rigid_only && (with_both[i]!=without_one[i] || with_both[i+1]!=without_one[i+1] || with_both[i+2]!=without_one[i+2])) ++level1_pixels;
					}
				assert(rigid_pixels>0 && (rigid_only || (static_pixels>0 && level1_pixels>0)));
				if (focused_mixed_scene) {
					auto changed=[&](const auto& a,const auto& b,unsigned x_begin,unsigned x_end) {
						unsigned count=0;
						for (unsigned y=height/8;y<height*7/8;++y)
							for (unsigned x=x_begin;x<x_end;++x) {
								const auto i=(y*width+x)*4U;
								if (a[i]!=b[i] || a[i+1]!=b[i+1] || a[i+2]!=b[i+2]) ++count;
							}
						return count;
					};
					const auto outer_unchanged=[&](const auto& pixels) {
						return pixels[outer]==baseline[outer] && pixels[outer+1]==baseline[outer+1] &&
							pixels[outer+2]==baseline[outer+2];
					};
					assert(changed(baseline,with_skin,width*3/8,width*5/8)>0 &&
						changed(baseline,with_decal,width/8,width/2)>0 &&
						changed(baseline,with_sorted,width/2,width*7/8)>0 &&
						changed(with_front,with_back,width/8,width/2)>0 &&
						changed(baseline,with_mixed,width/8,width*7/8)>0 &&
						outer_unchanged(with_skin) && outer_unchanged(with_decal) &&
						outer_unchanged(with_sorted) && outer_unchanged(with_mixed) &&
						outer_unchanged(with_front) &&
						outer_unchanged(with_back));
					if (focused_owned_pass_scene) {
						assert(pass_base.size()==baseline.size() &&
							pass_immediate.size()==baseline.size() &&
							pass_delayed.size()==baseline.size() &&
							pass_culled.size()==baseline.size() &&
							pass_outside.size()==baseline.size() &&
							pass_recovered.size()==baseline.size() &&
							pass_removed.size()==baseline.size());
						const auto center_left=width*3/8;
						const auto center_right=width*5/8;
						assert(changed(baseline,pass_base,center_left,center_right)>0 &&
							changed(pass_base,pass_immediate,center_left,center_right)>0 &&
							changed(pass_base,pass_delayed,center_left,center_right)>0 &&
							changed(pass_base,pass_culled,center_left,center_right)>0 &&
							pass_outside==pass_base && pass_recovered==pass_outside &&
							pass_removed==baseline &&
							outer_unchanged(pass_immediate) && outer_unchanged(pass_delayed) &&
							outer_unchanged(pass_culled) && outer_unchanged(pass_outside));
					}
				}
			} else {
				auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
				assert(recorder);
				const auto trace=recorder->snapshot();
				assert(trace.find("CameraClass::Apply viewport")!=std::string::npos &&
					trace.find("clear_viewport")!=std::string::npos &&
					trace.find("DX8Wrapper::Draw indexed")!=std::string::npos);
				if (focused_mixed_scene) {
					const auto decal=trace.find("fixture mixed decal frame");
					const auto skin=trace.find("fixture mixed skin frame");
					const auto sorted=trace.find("fixture mixed sort frame");
					const auto all=trace.find("fixture mixed all frame");
					assert(decal!=std::string::npos && skin>decal && sorted>skin &&
						all>decal && all<skin && trace.find("draw pipeline=",all)<skin &&
						trace.find("draw pipeline=",decal)<skin &&
						trace.find("draw pipeline=",skin)<sorted &&
						trace.find("draw pipeline=",sorted)!=std::string::npos);
					if (focused_owned_pass_scene) {
						const auto base=trace.find("fixture owned base pass");
						const auto immediate=trace.find("fixture owned immediate pass");
						const auto delayed=trace.find("fixture owned delayed pass");
						const auto cull=trace.find("fixture owned cull pass");
						const auto outside=trace.find("fixture owned outside cull pass");
						const auto recovery=trace.find("fixture owned stale target recovery");
						const auto removed=trace.find("fixture owned pass removed");
						assert(sorted<base && base<immediate && immediate<delayed &&
							delayed<cull && cull<outside && outside<recovery && recovery<removed);
						const auto draws=[&](std::size_t first,std::size_t last) {
							unsigned count=0;
							for (auto at=trace.find("draw pipeline=",first);at!=std::string::npos && at<last;
								at=trace.find("draw pipeline=",at+1)) ++count;
							return count;
						};
						assert(draws(base,immediate)>0 && draws(immediate,delayed)>draws(base,immediate) &&
							draws(delayed,cull)>0 && draws(cull,outside)>draws(base,immediate) &&
							draws(outside,recovery)==draws(base,immediate) &&
							draws(recovery,removed)==draws(base,immediate) &&
							draws(removed,trace.size())<draws(base,immediate));
					}
				}
			}
			device.destroy(depth); device.destroy(color);
			assert(!device.describe_texture_format(color));
			if (focused_fault_scene && !physical) {
				auto* recorder=dynamic_cast<zh::renderer::RecordingGpuDevice*>(&device);
				assert(recorder && recorder->resource_counts().total()==0);
			}
			if (physical) {
#if defined(ZH_BGFX_SHADER_DIR)
				auto* bgfx=dynamic_cast<zh::renderer::BgfxGpuDevice*>(&device);
				assert(bgfx && bgfx->live_resource_count()==0);
#endif
			}
		};
		if (!physical) {
				zh::renderer::RecordingGpuDevice device;
				run(device,[](auto) { return std::vector<unsigned char>{}; },160,
					zh::renderer::TextureFormat::bgra8);
				assert(device.resource_counts().total()==0);
		}
#if defined(ZH_BGFX_SHADER_DIR)
		else {
			unsigned completed_generations=0;
			for (unsigned width:{160U,200U}) {
			for (auto format:{zh::renderer::TextureFormat::bgra8,
				zh::renderer::TextureFormat::rgba8}) {
				zh::renderer::BgfxOptions options;
				options.shader_root=ZH_BGFX_SHADER_DIR;
				zh::renderer::BgfxGpuDevice device(options);
				run(device,[&](auto color) { return device.readback_rgba(color); },width,format);
				assert(device.wait_idle());
				++completed_generations;
			}
			}
			assert(completed_generations==4);
		}
#endif
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		_TheFileFactory=previous_factory;
		assert(factory.owners==0);
		return 0;
	}
#if defined(ZH_BGFX_SHADER_DIR)
	if (argc==2 && std::strcmp(argv[1],"--bgfx-source-viewport-clear")==0) {
		OwnedFactory factory;
		FileFactoryClass* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		struct Vertex { float x,y; unsigned color; float u,v; };
		for (unsigned width : {160U,200U}) {
			const unsigned height=width*3/4;
			zh::renderer::BgfxOptions options;
			options.shader_root=ZH_BGFX_SHADER_DIR;
			zh::renderer::BgfxGpuDevice device(options);
			auto vertex=device.create_shader({zh::renderer::ShaderStage::vertex,
				"renderer/acceptance.vert",0,0},"source clear vertex");
			auto fragment=device.create_shader({zh::renderer::ShaderStage::fragment,
				"renderer/acceptance.frag",0,0},"source clear fragment");
			assert(vertex && fragment);
			zh::renderer::PipelineDesc pipeline_desc;
			pipeline_desc.vertex_shader=vertex; pipeline_desc.fragment_shader=fragment;
			pipeline_desc.vertex_layout=zh::renderer::VertexLayout::position_color_uv;
			pipeline_desc.color_format=zh::renderer::TextureFormat::bgra8;
			pipeline_desc.raster.cull=zh::renderer::CullMode::none;
			auto pipeline=device.create_pipeline(zh::renderer::PipelineKey(pipeline_desc),"source clear pipeline");
			const std::array<Vertex,3> triangle{{
				{-0.9f,0.9f,0xff00ff00U,0,0},
				{-0.6f,0.9f,0xff00ff00U,1,0},
				{-0.75f,0.6f,0xff00ff00U,0.5f,1},
			}};
			auto vertex_buffer=device.create_buffer({sizeof(triangle),zh::renderer::BufferUsage::vertex,true},
				"source clear triangles");
			assert(pipeline && vertex_buffer);
			assert(device.upload({vertex_buffer,sizeof(triangle),0,sizeof(triangle)},triangle.data()));
			zh::renderer::DrawDesc draw;
			draw.pipeline=pipeline; draw.vertex_buffer=vertex_buffer; draw.vertex_or_index_count=3;
			zh::renderer::TextureDesc target;
			target.width=width; target.height=height; target.render_target=true;
			target.format=zh::renderer::TextureFormat::bgra8;
			auto color=device.create_texture(target,"source clear color");
			target.format=zh::renderer::TextureFormat::depth24_stencil8;
			auto depth=device.create_texture(target,"source clear depth");
			target.format=zh::renderer::TextureFormat::depth32;
			auto depth_only=device.create_texture(target,"source clear depth without stencil");
			assert(color && depth && depth_only);
			{
				zh::original_runtime::OriginalGpuEdge edge(device);
				assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
				edge.bind_frame_targets(color,depth,width,height);
				DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
				assert(WW3D::Begin_Render(true,true,Vector3(1,0,0),1)==WW3D_ERROR_OK);
				assert(device.draw(draw));
				CameraClass source_camera;
				source_camera.Set_Viewport(Vector2(0.25f,0.25f),Vector2(0.75f,0.75f));
				source_camera.Set_Clip_Planes(1,100);
				source_camera.Apply();
				DX8Wrapper::Clear(true,true,Vector3(0,0,1));
				assert(device.draw(draw));
				assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
				auto pixels=device.readback_rgba(color);
				assert(pixels.size()==static_cast<std::size_t>(width)*height*4);
				auto pixel=[&](unsigned x,unsigned y) { return pixels.data()+(y*width+x)*4; };
				const auto* outer=pixel(width-1,height-1);
				const auto* inner=pixel(width/2,height/2);
				assert(outer[0]==255 && outer[1]==0 && outer[2]==0 && outer[3]==255);
				// The original DX8Wrapper::Clear default destination alpha is zero.
				assert(inner[0]==0 && inner[1]==0 && inner[2]==255 && inner[3]==0);
				edge.bind_frame_targets(color,depth_only,width,height);
				DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
				assert(WW3D::Begin_Render(true,true,Vector3(1,0,0),1)==WW3D_ERROR_OK);
				source_camera.Apply();
				DX8Wrapper::Clear(false,true,Vector3(2,2,2),2,0.5f);
				assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
				pixels=device.readback_rgba(color);
				const auto* retained=pixels.data()+((height/2)*width+width/2)*4;
				assert(retained[0]==255 && retained[1]==0 && retained[2]==0 && retained[3]==255);
				assert(WW3D::Shutdown()==WW3D_ERROR_OK);
			}
			device.destroy(depth_only); device.destroy(depth); device.destroy(color); device.destroy(vertex_buffer);
			device.destroy(pipeline); device.destroy(fragment); device.destroy(vertex);
			assert(device.wait_idle());
			assert(factory.owners==0);
		}
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		_TheFileFactory=previous_factory;
		manager.Free_Assets();
		return 0;
	}
#endif
#if defined(ZH_GPU_SHADER_DIR)
	if (argc==2 && std::strcmp(argv[1],"--vulkan-source-frame")==0) {
		assert(SDL_Init(SDL_INIT_VIDEO));
		OwnedFactory factory;
		FileFactoryClass* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		for (unsigned width : {160U,240U}) {
			const unsigned height=width*3/4;
			zh::renderer::SdlGpuOptions options;
			options.debug=true; options.shader_root=ZH_GPU_SHADER_DIR;
			zh::renderer::SdlGpuDevice device(options);
			assert(device.capabilities().backend=="vulkan");
			zh::renderer::TextureDesc target;
			target.width=width; target.height=height;
			target.render_target=true; target.sampled=false;
			const auto color=device.create_texture(target,"original WW3D source frame color");
			target.format=zh::renderer::TextureFormat::depth24_stencil8;
			const auto depth=device.create_texture(target,"original WW3D source frame depth");
			assert(color && depth);
			{
				zh::original_runtime::OriginalGpuEdge edge(device);
				assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
				edge.bind_frame_targets(color,depth,width,height);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
				assert(WW3D::Begin_Render(true,true,Vector3(0.201f,0.403f,0.607f),0.77f)==WW3D_ERROR_OK);
				assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
				const auto pixels=device.readback_rgba(color);
				assert(pixels.size()==std::size_t(width)*height*4);
				for (std::size_t i=0;i<pixels.size();i+=4)
					assert(pixels[i]==51 && pixels[i+1]==102 && pixels[i+2]==154 && pixels[i+3]==196);
				assert(WW3D::Shutdown()==WW3D_ERROR_OK);
			}
			device.destroy(depth); device.destroy(color);
			assert(factory.owners==0);
		}
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		_TheFileFactory=previous_factory;
		SDL_Quit();
		return 0;
	}
#endif
	if (argc==2 && std::strcmp(argv[1],"--ww3d-source-frame")==0) {
		OwnedFactory factory;
		FileFactoryClass* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=48; target.height=32; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"caller-owned source frame color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		auto depth=recorder.create_texture(target,"caller-owned source frame depth");
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			assert(!WW3D::Is_Initted());
			const auto uninitialized_count=WW3D::Get_Frame_Count();
			assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			assert(WW3D::End_Render(true)==WW3D_ERROR_OK);
			assert(WW3D::Get_Frame_Count()==uninitialized_count && !recorder.pass_active());
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			assert(!WW3D::Is_Rendering());
			bool missing=false;
			try { (void)WW3D::Begin_Render(true,true,Vector3(0.2f,0.4f,0.6f),0.8f); }
			catch (const std::runtime_error&) { missing=true; }
			assert(missing && !WW3D::Is_Rendering() && !recorder.pass_active());
			edge.bind_frame_targets(color,depth,48,32);
			WW3D::Test_Inject_Capture_For_Negative(true);
			bool capture_unavailable=false;
			try { (void)WW3D::Begin_Render(true,true,Vector3(0,0,0),1); }
			catch (const std::runtime_error&) { capture_unavailable=true; }
			assert(capture_unavailable && !WW3D::Is_Rendering() && !recorder.pass_active());
			WW3D::Test_Inject_Capture_For_Negative(false);
			const auto first_frame=WW3D::Get_Frame_Count();
			bool bad_source_clear=false;
			try { (void)WW3D::Begin_Render(true,true,Vector3(1.2f,0,0),1); }
			catch (const std::runtime_error&) { bad_source_clear=true; }
			assert(bad_source_clear && !WW3D::Is_Rendering() && !recorder.pass_active());
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(true,true,Vector3(0.201f,0.403f,0.607f),0.77f,
				texture_frame_network_callback)==WW3D_ERROR_OK);
			assert(WW3D::Is_Rendering() && recorder.pass_active());
			assert(texture_frame_network_ticks==0 &&
				WW3D::Get_Last_Frame_Memory_Allocation_Count()==0 &&
				WW3D::Get_Last_Frame_Memory_Free_Count()==0);
			bool reentrant=false;
			try { (void)WW3D::Begin_Render(false,false,Vector3(0,0,0),1); }
			catch (const std::runtime_error&) { reentrant=true; }
			assert(reentrant && WW3D::Is_Rendering());
			assert(WW3D::End_Render(true)==WW3D_ERROR_OK);
			assert(!WW3D::Is_Rendering() && !recorder.pass_active());
			assert(WW3D::Get_Frame_Count()==first_frame+1);
			bool no_frame=false;
			try { (void)WW3D::End_Render(false); }
			catch (const std::runtime_error&) { no_frame=true; }
			assert(no_frame);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(false,false,Vector3(1,0,0),1)==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			const auto before_failed_end=WW3D::Get_Frame_Count();
			assert(recorder.end_pass()); // reject a stolen physical pass at original End
			bool lost_pass=false;
			try { (void)WW3D::End_Render(false); }
			catch (const std::runtime_error&) { lost_pass=true; }
			assert(lost_pass && !WW3D::Is_Rendering() && !recorder.pass_active() &&
				WW3D::Get_Frame_Count()==before_failed_end);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			recorder.destroy(depth);
			bool stale=false;
			try { (void)WW3D::Begin_Render(false,true,Vector3(0,0,0),1); }
			catch (const std::runtime_error&) { stale=true; }
			assert(stale && !WW3D::Is_Rendering() && !recorder.pass_active());
			zh::renderer::TextureDesc replacement;
			replacement.width=48; replacement.height=32;
			replacement.render_target=true; replacement.sampled=false;
			replacement.format=zh::renderer::TextureFormat::depth24_stencil8;
			depth=recorder.create_texture(replacement,"recreated caller-owned source depth");
			edge.bind_frame_targets(color,depth,48,32);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(false,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			const auto commands=recorder.snapshot();
			assert(commands.find("WW3D::Begin_Render source frame")!=std::string::npos &&
				commands.find("color_load=0")!=std::string::npos &&
				commands.find("color_load=1")!=std::string::npos &&
				commands.find("clear=0.200000,0.400000,0.603922,0.768627")!=std::string::npos);
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
		}
		recorder.destroy(depth); recorder.destroy(color);
		assert(recorder.resource_counts().total()==0 && factory.owners==0);
		// The original device clears only depth for a surface without stencil.
		zh::renderer::TextureDesc depth_only;
		depth_only.width=160; depth_only.height=120;
		depth_only.render_target=true; depth_only.format=zh::renderer::TextureFormat::depth32;
		zh::renderer::TextureDesc color_only=depth_only;
		color_only.format=zh::renderer::TextureFormat::bgra8;
		const auto no_stencil_color=recorder.create_texture(color_only,"depth-only clear color");
		const auto no_stencil_depth=recorder.create_texture(depth_only,"depth-only clear depth");
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			edge.bind_frame_targets(no_stencil_color,no_stencil_depth,160,120);
			edge.begin_source_frame(true,true,0,0,0,1);
			edge.set_source_viewport(40,30,80,60,0,1);
			DX8Wrapper::Clear(false,true,Vector3(2,2,2),2,0.5f);
			const auto trace=recorder.snapshot();
			const auto depth_clear=trace.find("clear_viewport rect=40,30,80,60");
			assert(depth_clear!=std::string::npos && trace.find("flags=-D-",depth_clear)!=std::string::npos);
			edge.abort_source_frame();
		}
		recorder.destroy(no_stencil_depth); recorder.destroy(no_stencil_color);
		assert(!recorder.describe_texture_format(no_stencil_depth) &&
			!recorder.describe_texture_format(no_stencil_color));
		assert(recorder.resource_counts().total()==0);
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		_TheFileFactory=previous_factory;
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--texture-frame-update")==0) {
		OwnedFactory factory;
		factory.files["mytex.tga"]=original_targa();
		FileFactoryClass* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false); // original Win32 thumbnail lookup is not a CPU loader
		zh::renderer::RecordingGpuDevice recorder;
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			TextureClass* texture=manager.Get_Texture("mytex.tga",MIP_LEVELS_1);
			assert(texture && !texture->Is_Initialized());
			texture->Set_Inactivation_Time(10);
			WWMemoryLogClass::Reset_Counters();
			assert(WWMemoryLogClass::Get_Allocate_Count()==0 && WWMemoryLogClass::Get_Free_Count()==0);
			TextureClass procedural(2,2,WW3D_FORMAT_A8R8G8B8,MIP_LEVELS_1);
			procedural.Apply_Gpu_Texture(WW3D_FORMAT_A8R8G8B8,2,2);
			assert(procedural.Is_Initialized() && procedural.Is_Procedural());
			procedural.Invalidate();
			assert(procedural.Is_Initialized()); // authored procedural exclusion
			WW3D::Sync(100);
			texture->Init();
			WW3D::Set_Thumbnail_Enabled(true);
			assert(texture->Is_Initialized() && factory.owners==0);
			const auto first_physical=edge.texture_handle(texture);
			TextureLoader::Update(texture_frame_network_callback);
			assert(texture->Is_Initialized());
			assert(texture_frame_network_ticks==0);
			TextureLoader::Suspend_Texture_Load();
			WW3D::Sync(111);
			TextureLoader::Update(texture_frame_network_callback);
			assert(texture->Is_Initialized());
			assert(texture_frame_network_ticks==0);
			TextureLoader::Continue_Texture_Load();
			TextureLoader::Update(nullptr);
			assert(!texture->Is_Initialized());
			bool invalidated_owner=false;
			try { (void)edge.texture_handle(texture); }
			catch (const std::runtime_error&) { invalidated_owner=true; }
			assert(invalidated_owner);
			WW3D::Sync(112);
			WW3D::Set_Thumbnail_Enabled(false);
			texture->Init();
			WW3D::Set_Thumbnail_Enabled(true);
			assert(texture->Is_Initialized());
			assert(edge.texture_handle(texture)!=first_physical);
			WW3D::Sync(135);
			TextureLoader::Update(nullptr);
			assert(texture->Is_Initialized()); // authored rapid-reload extension
			WW3D::Sync(153);
			TextureLoader::Update(nullptr);
			assert(!texture->Is_Initialized());
			WW3D::Set_Thumbnail_Enabled(false);
			texture->Init();
			WW3D::Set_Thumbnail_Enabled(true);
			TextureLoader::Set_Texture_Inactive_Override_Time(2);
			WW3D::Sync(156);
			TextureLoader::Update(nullptr);
			assert(!texture->Is_Initialized());
			TextureLoader::Set_Texture_Inactive_Override_Time(0);
			WW3D::Set_Thumbnail_Enabled(false);
			texture->Init();
			WW3D::Sync(1000);
			TextureLoader::Update(nullptr);
			assert(texture->Is_Initialized()); // original thumbnail-disabled gate
			texture->Invalidate();
			texture->Release_Ref();
			manager.Release_All_Textures();
			assert(recorder.resource_counts().total()==0 && factory.owners==0);
		}
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		_TheFileFactory=previous_factory;
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--ww3d-init")==0) {
		char trim_fixture[]=" \t SOURCE \r\n";
		assert(strtrim(trim_fixture)==trim_fixture && std::strcmp(trim_fixture,"SOURCE")==0);
		OwnedFactory factory;
		const std::string ini="[TESTTREE.IDLE]\nBoneName=root\nping=1, beep,2D\n";
		factory.files["w3danimsound.ini"]={ini.begin(),ini.end()};
		factory.files["DAZZLE.INI"]={'\n'};
		FileFactoryClass* previous=_TheFileFactory;
		_TheFileFactory=&factory;
		assert(!WW3D::Is_Initted());
		assert(WW3D::Init(nullptr,nullptr,true)==WW3D_ERROR_OK);
		VertexMaterialClass* lite_preset=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
		assert(lite_preset && lite_preset->Num_Refs()==2 && !lite_preset->Get_Lighting());
		lite_preset->Release_Ref();
		assert(!WW3D::Is_Initted()); // authored lite semantics
		assert(WW3D::Test_Default_Static_Sort_List()!=nullptr);
		assert(WW3D::Test_Current_Static_Sort_List()==WW3D::Test_Default_Static_Sort_List());
		assert(WW3D::Shutdown()==WW3D_ERROR_OK);
		assert(WW3D::Test_Default_Static_Sort_List()==nullptr);
		assert(WW3D::Test_Current_Static_Sort_List()==nullptr);
		assert(factory.owners==0);
		assert(manager.Load_3D_Assets(input));
		bool unavailable=false;
		try { (void)WW3D::Init(nullptr,nullptr,false); }
		catch (const std::runtime_error&) { unavailable=true; }
		assert(unavailable && !WW3D::Is_Initted());
		zh::renderer::RecordingGpuDevice recorder;
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			assert(WW3D::Init(reinterpret_cast<void*>(1),nullptr,false)==WW3D_ERROR_INITIALIZATION_FAILED);
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			VertexMaterialClass* full_preset=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
			assert(full_preset && full_preset->Num_Refs()==2 && !full_preset->Get_Lighting());
			full_preset->Release_Ref();
			assert(WW3D::Is_Initted() && !WW3D::Is_Rendering());
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_INITIALIZATION_FAILED);
			assert(WW3D::Is_Initted());
			assert(WW3D::Test_Default_Static_Sort_List()!=nullptr);
			assert(WW3D::Test_Current_Static_Sort_List()==WW3D::Test_Default_Static_Sort_List());
			HAnimClass* anim=manager.Get_HAnim("TESTTREE.IDLE");
			assert(anim && std::strcmp(AnimatedSoundMgrClass::Get_Embedded_Sound_Name(anim),"root")==0);
			assert(AnimatedSoundMgrClass::Trigger_Sound(anim,0,2,Matrix3D(true))==0);
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
			assert(!WW3D::Is_Initted() && factory.owners==0);
			assert(WW3D::Test_Default_Static_Sort_List()==nullptr);
			assert(WW3D::Test_Current_Static_Sort_List()==nullptr);
			assert(manager.Get_HAnim("TESTTREE.IDLE")==nullptr);
			const std::string malformed="[TESTTREE.IDLE]\nping=malformed\n";
			factory.files["w3danimsound.ini"]={malformed.begin(),malformed.end()};
			bool rejected=false;
			try { (void)WW3D::Init(nullptr,nullptr,false); }
			catch (const std::runtime_error&) { rejected=true; }
			assert(rejected && !WW3D::Is_Initted());
			assert(WW3D::Test_Default_Static_Sort_List()==nullptr && factory.owners==0);
			factory.files["w3danimsound.ini"]={ini.begin(),ini.end()};
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			VertexMaterialClass* retry_preset=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
			assert(retry_preset && retry_preset->Num_Refs()==2);
			retry_preset->Release_Ref();
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
			factory.files.erase("w3danimsound.ini");
			factory.files.erase("DAZZLE.INI");
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
		}
		assert(recorder.resource_counts().total()==0);
		_TheFileFactory=previous;
		assert(factory.owners==0);
		return 0;
	}
	assert(manager.Get_HTree("TESTTREE") != nullptr);
	assert(manager.Get_HAnim("TESTTREE.IDLE") != nullptr);
	assert(manager.Render_Obj_Exists("TEST.TRIANGLE"));
	assert(manager.Render_Obj_Exists("TEST.HLOD"));
	if (!supply_variant) assert(manager.Render_Obj_Exists("TEST.SKINHLOD"));
	RenderObjClass *hlod = manager.Create_Render_Obj("TEST.HLOD");
	assert(hlod != nullptr);
	assert(hlod->Get_Num_Sub_Objects() == 1);
	hlod->Release_Ref();
	if (!supply_variant) {
		// The original HLOD publishes its own HTree and assigns the skin child
		// Container. A bare MeshClass would not have an authoritative bone tree.
		RenderObjClass* original_skin_hlod=manager.Create_Render_Obj("TEST.SKINHLOD");
		assert(original_skin_hlod && original_skin_hlod->Get_HTree() &&
			original_skin_hlod->Get_Num_Sub_Objects()==1);
		RenderObjClass* original_skin_child=original_skin_hlod->Get_Sub_Object(0);
		assert(original_skin_child && original_skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
		auto* original_skin_mesh=static_cast<MeshClass*>(original_skin_child);
		assert(original_skin_mesh->Peek_Model()->Get_Flag(MeshGeometryClass::SKIN));
		std::array<Vector3,3> rest{}, moved{}, normal{};
		(void)static_cast<HLodClass*>(original_skin_hlod)->Get_Bone_Transform(0);
		original_skin_mesh->Get_Deformed_Vertices(rest.data(),normal.data());
		original_skin_hlod->Set_Position(Vector3(0,0,-10));
		(void)static_cast<HLodClass*>(original_skin_hlod)->Get_Bone_Transform(0);
		original_skin_mesh->Get_Deformed_Vertices(moved.data(),normal.data());
		for (unsigned i=0;i<3;++i) assert(std::abs(moved[i].Z-rest[i].Z+10.0f)<0.01f);
		original_skin_child->Release_Ref();
		original_skin_hlod->Release_Ref();
		RenderObjClass* invalid_skin_hlod=manager.Create_Render_Obj("TEST.BADHLOD");
		assert(invalid_skin_hlod && invalid_skin_hlod->Get_HTree());
		RenderObjClass* invalid_skin_child=invalid_skin_hlod->Get_Sub_Object(0);
		assert(invalid_skin_child && invalid_skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
		bool invalid_bone_rejected=false;
		try {
			static_cast<MeshClass*>(invalid_skin_child)->Get_Deformed_Vertices(moved.data(),normal.data());
		} catch (const std::runtime_error& error) {
			invalid_bone_rejected=std::strstr(error.what(),"bone index")!=nullptr;
		}
		assert(invalid_bone_rejected);
		invalid_skin_child->Release_Ref();
		invalid_skin_hlod->Release_Ref();
	}
	RAMFileClass duplicate_input(bytes.data(), size);
	assert(!manager.Load_3D_Assets(duplicate_input));
	assert(manager.Render_Obj_Exists("TEST.TRIANGLE"));
	RenderObjClass *object = manager.Create_Render_Obj("TEST.TRIANGLE");
	assert(object != nullptr && object->Class_ID() == RenderObjClass::CLASSID_MESH);
	auto *mesh = static_cast<MeshClass *>(object);
	CameraClass camera;
	RenderInfoClass render_info(camera);
	if (argc==2 && std::strcmp(argv[1],"--static-sort-failure")==0) {
		struct CountingLight final : LightClass {
			int renders=0;
			bool saw_disabled=false;
			bool throw_render=false;
			CountingLight():LightClass(LightClass::DIRECTIONAL) {}
			void Render(RenderInfoClass&) override {
				++renders;
				saw_disabled=!WW3D::Are_Static_Sort_Lists_Enabled();
				if (throw_render) throw std::runtime_error("injected static sort object");
			}
		};
		struct FailHook final : RenderHookClass {
			bool fail=true;
			bool fail_post=false;
			int pre=0,post=0;
			bool Pre_Render(RenderObjClass*,RenderInfoClass&) override {
				++pre;
				if (fail) throw std::runtime_error("injected static sort hook");
				return true;
			}
			void Post_Render(RenderObjClass*,RenderInfoClass&) override {
				++post;
				if (fail_post) throw std::runtime_error("injected static sort post hook");
			}
		};
		auto* top=new CountingLight;
		auto* lower=new CountingLight;
		auto* hook=new FailHook;
		top->Set_Render_Hook(hook);
		const int top_refs=top->Num_Refs(),lower_refs=lower->Num_Refs();
		DefaultStaticSortListClass list;
		WW3D::Override_Current_Static_Sort_Lists(&list);
		WW3D::Enable_Static_Sort_Lists(true);
		list.Add_To_List(top,2);
		list.Add_To_List(lower,1);
		assert(top->Num_Refs()==top_refs+1 && lower->Num_Refs()==lower_refs+1);
		bool failed=false;
		try { WW3D::Render_And_Clear_Static_Sort_Lists(render_info); }
		catch (const std::runtime_error&) { failed=true; }
		assert(failed && WW3D::Are_Static_Sort_Lists_Enabled() &&
			top->Num_Refs()==top_refs && lower->Num_Refs()==lower_refs+1 &&
			top->renders==0 && lower->renders==0 && hook->pre==1 && hook->post==0);
		hook->fail=false;
		WW3D::Render_And_Clear_Static_Sort_Lists(render_info);
		assert(lower->renders==1 && lower->saw_disabled &&
			top->renders==0 && lower->Num_Refs()==lower_refs);
		WW3D::Enable_Static_Sort_Lists(false);
		list.Add_To_List(top,2);
		hook->fail=true;
		failed=false;
		try { WW3D::Render_And_Clear_Static_Sort_Lists(render_info); }
		catch (const std::runtime_error&) { failed=true; }
		assert(failed && !WW3D::Are_Static_Sort_Lists_Enabled() && top->Num_Refs()==top_refs);
		hook->fail=false;
		hook->fail_post=true;
		list.Add_To_List(top,2);
		failed=false;
		try { WW3D::Render_And_Clear_Static_Sort_Lists(render_info); }
		catch (const std::runtime_error&) { failed=true; }
		assert(failed && top->Num_Refs()==top_refs && !WW3D::Are_Static_Sort_Lists_Enabled());
		hook->fail_post=false;
		lower->throw_render=true;
		list.Add_To_List(lower,1);
		failed=false;
		try { WW3D::Render_And_Clear_Static_Sort_Lists(render_info); }
		catch (const std::runtime_error&) { failed=true; }
		assert(failed && lower->Num_Refs()==lower_refs && !WW3D::Are_Static_Sort_Lists_Enabled());
		lower->throw_render=false;
		list.Add_To_List(top,2);
		list.Add_To_List(lower,1);
		list.Discard_Without_Rendering();
		assert(top->Num_Refs()==top_refs && lower->Num_Refs()==lower_refs);
		hook->fail=true;
		WW3D::Reset_Current_Static_Sort_Lists_To_Default();
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		OwnedFactory factory;
		factory.files["mytex.tga"]=original_targa();
		factory.files["MYTEX.TGA"]=factory.files["mytex.tga"];
		auto* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_Position(Vector3(0,0,-10));
		SimpleSceneClass scene;
		scene.Add_Render_Object(mesh);
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=32; target.height=32; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"static failure color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"static failure depth");
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			edge.bind_frame_targets(color,depth,32,32);
			camera.Set_Clip_Planes(1,100);
			WW3D::Enable_Static_Sort_Lists(true);
			WW3D::Add_To_Static_Sort_List(top,2);
			WW3D::Add_To_Static_Sort_List(lower,1);
			assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			failed=false;
			try { (void)WW3D::Render(&scene,&camera,false,false,Vector3(0,0,0)); }
			catch (const std::runtime_error&) { failed=true; }
			assert(failed && !WW3D::Is_Rendering() &&
				WW3D::Are_Static_Sort_Lists_Enabled() &&
				top->Num_Refs()==top_refs && lower->Num_Refs()==lower_refs);
			assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			assert(WW3D::Render(&scene,&camera,false,false,Vector3(0,0,0))==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
		}
		scene.Remove_All_Render_Objects();
		recorder.destroy(depth); recorder.destroy(color);
		assert(recorder.resource_counts().total()==0);
		_TheFileFactory=previous_factory;
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		top->Release_Ref(); lower->Release_Ref();
		object->Release_Ref(); manager.Free_Assets();
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--ww3d-scene-wrapper")==0) {
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		OwnedFactory factory;
		factory.files["mytex.tga"]=original_targa();
		factory.files["MYTEX.TGA"]=factory.files["mytex.tga"];
		auto* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_Position(Vector3(0,0,-10));
		SimpleSceneClass scene;
		scene.Add_Render_Object(mesh);
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=64; target.height=64; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"source WW3D scene color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"source WW3D scene depth");
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			edge.bind_frame_targets(color,depth,64,64);
			camera.Set_Clip_Planes(1,100);
			bool inactive=false;
			try { (void)WW3D::Render(&scene,&camera,true,true,Vector3(0,0,0)); }
			catch (const std::runtime_error&) { inactive=true; }
			assert(inactive);
			assert(WW3D::Begin_Render(true,true,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			scene.Set_Polygon_Mode(SceneClass::LINE);
			const auto pre_line=recorder.snapshot();
			bool unsupported=false;
			try { (void)WW3D::Render(&scene,&camera,true,true,Vector3(0,0,0)); }
			catch (const std::runtime_error&) { unsupported=true; }
			assert(unsupported && WW3D::Is_Rendering() && recorder.snapshot()==pre_line);
			scene.Set_Polygon_Mode(SceneClass::POINT);
			unsupported=false;
			try { (void)WW3D::Render(&scene,&camera,false,false,Vector3(0,0,0)); }
			catch (const std::runtime_error&) { unsupported=true; }
			assert(unsupported && recorder.snapshot()==pre_line);
			scene.Set_Polygon_Mode(SceneClass::FILL);
			bool missing_camera=false;
			try { (void)WW3D::Render(&scene,nullptr,false,false,Vector3(0,0,0)); }
			catch (const std::runtime_error&) { missing_camera=true; }
			assert(missing_camera && recorder.snapshot()==pre_line);
			camera.Set_Viewport(Vector2(0,0),Vector2(0.5f,1));
			assert(WW3D::Render(&scene,&camera,true,true,Vector3(0.1f,0.2f,0.3f))==WW3D_ERROR_OK);
			camera.Set_Viewport(Vector2(0.5f,0),Vector2(1,1));
			assert(WW3D::Render(&scene,&camera,true,true,Vector3(0.3f,0.2f,0.1f))==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			const auto trace=recorder.snapshot();
			const auto first=trace.find("CameraClass::Apply viewport");
			const auto first_clear=trace.find("clear_viewport",first);
			const auto fill=trace.find("DX8Wrapper::Set_DX8_Render_State=8:3",first_clear);
			const auto first_draw=trace.find("draw pipeline=",fill);
			const auto second=trace.find("CameraClass::Apply viewport",first_draw);
			const auto second_clear=trace.find("clear_viewport",second);
			assert(first!=std::string::npos && first_clear!=std::string::npos &&
				fill!=std::string::npos && first_draw!=std::string::npos &&
				second!=std::string::npos && second_clear!=std::string::npos);
			assert(WW3D::Begin_Render(false,false,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			LayerClass missing_layer;
			bool missing_scene=false;
			try { (void)WW3D::Render(missing_layer); }
			catch (const std::runtime_error&) { missing_scene=true; }
			assert(missing_scene && WW3D::Is_Rendering());
			{
				LayerListClass layers;
				LayerClass back(&scene,&camera,false,false,Vector3(0,0,0));
				LayerClass front(&scene,&camera,false,false,Vector3(0,0,0));
				layers.Add_Head(&back);
				layers.Add_Head(&front);
				assert(WW3D::Render(layers)==WW3D_ERROR_OK);
			}
			RenderInfoClass object_info(camera);
			assert(WW3D::Render(*mesh,object_info)==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			const auto wrappers=recorder.snapshot();
			assert(wrappers.find("draw pipeline=",trace.size())!=std::string::npos);
			struct RejectOnce final : RenderHookClass {
				bool reject=true;
				bool Pre_Render(RenderObjClass*,RenderInfoClass&) override {
					if (reject) throw std::runtime_error("injected WW3D scene hook");
					return true;
				}
				void Post_Render(RenderObjClass*,RenderInfoClass&) override {}
			};
			auto* reject=new RejectOnce;
			mesh->Set_Render_Hook(reject);
			assert(WW3D::Begin_Render(false,false,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			const auto before_failure=recorder.snapshot();
			bool hook_failed=false;
			try { (void)WW3D::Render(&scene,&camera,false,false,Vector3(0,0,0)); }
			catch (const std::runtime_error&) { hook_failed=true; }
			assert(hook_failed && !WW3D::Is_Rendering() &&
				recorder.snapshot().find("draw pipeline=",before_failure.size())==std::string::npos);
			reject->reject=false;
			assert(WW3D::Begin_Render(false,false,Vector3(0,0,0),1)==WW3D_ERROR_OK);
			assert(WW3D::Render(&scene,&camera,false,false,Vector3(0,0,0))==WW3D_ERROR_OK);
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
		}
		scene.Remove_All_Render_Objects();
		recorder.destroy(depth); recorder.destroy(color);
		assert(recorder.resource_counts().total()==0);
		object->Release_Ref(); manager.Free_Assets();
		_TheFileFactory=previous_factory;
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--scene-traversal")==0) {
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		auto* previous_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		struct Hook final : RenderHookClass {
			int pre=0,post=0;
			bool fail=false;
			bool Pre_Render(RenderObjClass*,RenderInfoClass&) override {
				++pre;
				if (fail) throw std::runtime_error("injected source render hook");
				return true;
			}
			void Post_Render(RenderObjClass*,RenderInfoClass&) override { ++post; }
		};
		auto* hook=new Hook;
		mesh->Set_Render_Hook(hook);
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_Position(Vector3(0,0,-10));
		RenderObjClass* hidden=manager.Create_Render_Obj("TEST.TRIANGLE");
		assert(hidden);
		hidden->Set_Position(Vector3(10000,0,-10));
		struct ExposedSimpleScene final : SimpleSceneClass { using SceneClass::Render; } scene;
		scene.Add_Render_Object(mesh);
		scene.Add_Render_Object(hidden);
		scene.Register(mesh,SceneClass::ON_FRAME_UPDATE);
		auto* source_light=new LightClass(LightClass::DIRECTIONAL);
		const int initial_light_refs=source_light->Num_Refs();
		scene.Add_Render_Object(source_light);
		assert(source_light->Num_Refs()>initial_light_refs);
		scene.Set_Fog_Enable(true);
		scene.Set_Fog_Color(Vector3(0.1f,0.2f,0.3f));
		scene.Set_Fog_Range(1,100);
		bool missing=false;
		try { scene.Render(render_info); }
		catch (const std::runtime_error&) { missing=true; }
		assert(missing && hook->pre==0);
		scene.Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_LINE);
		bool extra=false;
		try { scene.Render(render_info); }
		catch (const std::runtime_error&) { extra=true; }
		assert(extra && hook->pre==0);
		scene.Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_DISABLE);
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=32; target.height=32; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"source scene color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"source scene depth");
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=32; pass.height=32;
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			TheDX8MeshRenderer.Init();
			TheDX8MeshRenderer.Set_Camera(&camera);
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			assert(recorder.begin_pass(pass,"original scene traversal"));
			const auto before_bad_fog=recorder.snapshot();
			const auto selected_fog=DX8Wrapper::Get_Fog_Color();
			scene.Set_Fog_Color(Vector3(2,0,0));
			bool invalid_fog=false;
			try { scene.Render(render_info); }
			catch (const std::runtime_error&) { invalid_fog=true; }
			assert(invalid_fog && hook->pre==0 && recorder.snapshot()==before_bad_fog &&
				DX8Wrapper::Get_Fog_Color()==selected_fog);
			scene.Set_Fog_Color(Vector3(0.1f,0.2f,0.3f));
			hook->fail=true;
			bool injected=false;
			try { scene.Render(render_info); }
			catch (const std::runtime_error&) { injected=true; }
			assert(injected && hook->pre==1 && hook->post==0 && !render_info.light_environment);
			hook->fail=false;
			scene.Render(render_info);
			assert(hook->pre==2 && hook->post==1 && render_info.light_environment &&
				render_info.light_environment->Get_Light_Count()==1);
			assert(mesh->Is_Really_Visible() && !hidden->Is_Really_Visible());
			assert(DX8Wrapper::Get_Fog_Enable());
			const auto commands=recorder.snapshot();
			assert(commands.find("DX8Wrapper::Set_Light slot=0 disabled")!=std::string::npos);
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
		}
		scene.Unregister(mesh,SceneClass::ON_FRAME_UPDATE);
		scene.Remove_All_Render_Objects();
		assert(source_light->Num_Refs()==initial_light_refs);
		TheDX8MeshRenderer.Shutdown();
		recorder.destroy(depth); recorder.destroy(color);
		assert(recorder.resource_counts().total()==0);
		source_light->Release_Ref(); hidden->Release_Ref(); object->Release_Ref(); manager.Free_Assets();
		_TheFileFactory=previous_factory;
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--source-viewport-clear")==0) {
		OwnedFactory factory;
		FileFactoryClass* previous_factory=_TheFileFactory;
		_TheFileFactory=&factory;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=160; target.height=120; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"source clear color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"source clear depth");
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			assert(WW3D::Init(nullptr,nullptr,false)==WW3D_ERROR_OK);
			edge.bind_frame_targets(color,depth,160,120);
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			camera.Set_Viewport(Vector2(0.25f,0.25f),Vector2(0.75f,0.75f));
			camera.Set_Clip_Planes(1.0f,100.0f);
			bool inactive=false;
			try { DX8Wrapper::Clear(true,true,Vector3(0,0,1)); }
			catch (const std::runtime_error&) { inactive=true; }
			assert(inactive);
			assert(WW3D::Begin_Render(true,true,Vector3(1,0,0),1)==WW3D_ERROR_OK);
			camera.Apply();
			const auto before=recorder.snapshot();
			bool invalid=false;
			try { DX8Wrapper::Clear(true,true,Vector3(2,0,0)); }
			catch (const std::runtime_error&) { invalid=true; }
			assert(invalid && recorder.snapshot()==before);
			invalid=false;
			try { DX8Wrapper::Clear(true,true,Vector3(0,0,1),0,2); }
			catch (const std::runtime_error&) { invalid=true; }
			assert(invalid && recorder.snapshot()==before);
			// Depth-only does not read color/alpha; color-only does not read Z.
			DX8Wrapper::Clear(false,false,Vector3(2,2,2),2,2);
			assert(recorder.snapshot()==before);
			DX8Wrapper::Clear(true,false,Vector3(0,0,1),0,2);
			DX8Wrapper::Clear(true,true,Vector3(0,0,1));
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			const auto commands=recorder.snapshot();
			const auto begin=commands.find("WW3D::Begin_Render source frame");
			const auto camera_marker=commands.find("CameraClass::Apply viewport");
			const auto source_clear=commands.find("clear_viewport rect=40,30,80,60");
			assert(begin!=std::string::npos && camera_marker>begin &&
				source_clear>camera_marker && commands.find("flags=CDS",source_clear)!=std::string::npos);
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(true,true,Vector3(1,0,0),1)==WW3D_ERROR_OK);
			camera.Apply();
			assert(recorder.end_pass()); // inject a lost physical pass at source Clear
			bool lost_clear=false;
			try { DX8Wrapper::Clear(true,true,Vector3(0,0,1)); }
			catch (const std::runtime_error&) { lost_clear=true; }
			assert(lost_clear);
			bool lost_frame=false;
			try { (void)WW3D::End_Render(false); }
			catch (const std::runtime_error&) { lost_frame=true; }
			assert(lost_frame && !WW3D::Is_Rendering() && !recorder.pass_active());
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			assert(WW3D::Begin_Render(true,true,Vector3(1,0,0),1)==WW3D_ERROR_OK);
			camera.Apply();
			DX8Wrapper::Clear(true,true,Vector3(0,0,1));
			assert(WW3D::End_Render(false)==WW3D_ERROR_OK);
			assert(WW3D::Shutdown()==WW3D_ERROR_OK);
		}
		recorder.destroy(depth); recorder.destroy(color);
		assert(recorder.resource_counts().total()==0 && factory.owners==0);
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		_TheFileFactory=previous_factory;
		object->Release_Ref(); manager.Free_Assets();
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--camera-apply")==0) {
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=160; target.height=120; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"original camera target color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"original camera target depth");
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=160; pass.height=120;
		camera.Set_Viewport(Vector2(0.125f,0.25f),Vector2(0.875f,0.75f));
		camera.Set_Zbuffer_Range(0.2f,0.8f);
		camera.Set_Clip_Planes(1.0f,100.0f);
		Matrix3D moved(true); moved.Set_Translation(Vector3(1,2,3));
		camera.Set_Transform(moved);
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			bool no_pass=false;
			try { camera.Apply(); }
			catch (const std::runtime_error&) { no_pass=true; }
			assert(no_pass && !recorder.pass_active());
			assert(recorder.begin_pass(pass,"original CameraClass::Apply source decisions"));
			camera.Apply();
			const auto marker=recorder.snapshot();
			assert(marker.find("viewport=20.000000,30.000000,120.000000,60.000000 depth=0.200000:0.800000")!=
				std::string::npos && marker.find("CameraClass::Apply viewport")!=std::string::npos);
			Matrix4x4 source_view,source_projection,expected_projection;
			DX8Wrapper::Get_Transform(D3DTS_VIEW,source_view);
			DX8Wrapper::Get_Transform(D3DTS_PROJECTION,source_projection);
			camera.Get_D3D_Projection_Matrix(&expected_projection);
			for (unsigned row=0;row<4;++row)
				for (unsigned col=0;col<4;++col)
					assert(std::abs(source_projection[row][col]-expected_projection[row][col])<0.00001f);
			assert(std::abs(source_view[0].W+1.0f)<0.00001f &&
				std::abs(source_view[1].W+2.0f)<0.00001f &&
				std::abs(source_view[2].W+3.0f)<0.00001f);
			bool unsupported_bias=false;
			try { DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,1); }
			catch (const std::runtime_error&) { unsupported_bias=true; }
			assert(unsupported_bias && recorder.snapshot()==marker);
			camera.Set_Zbuffer_Range(-0.1f,0.9f);
			bool invalid_depth=false;
			try { camera.Apply(); }
			catch (const std::runtime_error&) { invalid_depth=true; }
			assert(invalid_depth && recorder.snapshot()==marker);
			camera.Set_Zbuffer_Range(0.2f,0.8f);
			camera.Set_Viewport(Vector2(0.75f,0.75f),Vector2(0.25f,0.25f));
			bool invalid_viewport=false;
			try { camera.Apply(); }
			catch (const std::runtime_error&) { invalid_viewport=true; }
			assert(invalid_viewport && recorder.snapshot()==marker);
			camera.Set_Viewport(Vector2(0.125f,0.25f),Vector2(0.875f,0.75f));
			camera.Set_Clip_Planes(1.0f,1.0f);
			bool invalid_clip=false;
			try { camera.Apply(); }
			catch (const std::runtime_error&) { invalid_clip=true; }
			assert(invalid_clip && recorder.snapshot()==marker);
			camera.Set_Clip_Planes(1.0f,100.0f);
			camera.Apply();
			assert(recorder.snapshot().find("CameraClass::Apply viewport",marker.size())!=std::string::npos);
			assert(recorder.end_pass());
			camera.Set_Viewport(Vector2(0,0),Vector2(1,1));
			bool ended=false;
			try { camera.Apply(); }
			catch (const std::runtime_error&) { ended=true; }
			assert(ended);
		}
		recorder.destroy(color); recorder.destroy(depth);
		assert(recorder.resource_counts().total()==0);
		object->Release_Ref(); manager.Free_Assets();
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--sorting-cpu")==0) {
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		const bool previous_sorting=WW3D::Is_Sorting_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		auto* old_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		TheDX8MeshRenderer.Init();
		WW3D::Enable_Sorting(true);
		TheDX8MeshRenderer.Set_Camera(&camera);
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,true);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_Position(Vector3(0,0,-10));
		RenderObjClass* back=manager.Create_Render_Obj("TEST.TRIANGLE");
		assert(back && back->Class_ID()==RenderObjClass::CLASSID_MESH);
		auto* back_mesh=static_cast<MeshClass*>(back);
		back_mesh->Set_Position(Vector3(0,0,-12));
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=32; target.height=32; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"original sorted source color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"original sorted source depth");
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=32; pass.height=32;
		for (unsigned attempt=0;attempt<5;++attempt) {
			mesh->Render(render_info);
			back_mesh->Render(render_info);
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
				assert(recorder.begin_pass(pass,"original queued CPU sorting source"));
				TheDX8MeshRenderer.Flush();
				// Original Flush now unbinds its buffers. Bind separate bounded
				// sorting inputs only for Insert_Triangles negative controls;
				// queued source-mesh draws retain their own original snapshots.
				auto* control_vb=NEW_REF(SortingVertexBufferClass,(3));
				auto* control_ib=NEW_REF(SortingIndexBufferClass,(3));
				{
					VertexBufferClass::WriteLockClass lock(control_vb);
					auto* vertices=static_cast<VertexFormatXYZNDUV2*>(lock.Get_Vertex_Array());
					for (int i=0;i<3;++i) vertices[i].x=vertices[i].y=vertices[i].z=0;
				}
				{
					IndexBufferClass::WriteLockClass lock(control_ib);
					auto* indices=static_cast<unsigned short*>(lock.Get_Index_Array());
					indices[0]=0; indices[1]=1; indices[2]=2;
				}
				DX8Wrapper::Set_Vertex_Buffer(control_vb);
				DX8Wrapper::Set_Index_Buffer(control_ib,0);
				const SphereClass bad_sphere(Vector3(0,0,
					std::numeric_limits<float>::quiet_NaN()),1.0f);
				bool invalid_index=false,invalid_depth=false;
				try { SortingRendererClass::Insert_Triangles(
					SphereClass(Vector3(0,0,0),1.0f),60000,1,0,3); }
				catch (const std::runtime_error& error) {
					invalid_index=std::strstr(error.what(),"sorting source range")!=nullptr;
				}
				try { SortingRendererClass::Insert_Triangles(bad_sphere,0,1,0,3); }
				catch (const std::runtime_error& error) {
					invalid_depth=std::strstr(error.what(),"nonfinite depth")!=nullptr;
				}
				assert(invalid_index && invalid_depth);
				RenderStateStruct selected_sort_state;
				DX8Wrapper::Get_Render_State(selected_sort_state);
				assert(selected_sort_state.index_buffer &&
					selected_sort_state.index_buffer_type==BUFFER_TYPE_SORTING);
				bool invalid_source_element=false;
				unsigned short original_element=0;
				DX8Wrapper::Set_Index_Buffer(nullptr,0);
				{
					IndexBufferClass::WriteLockClass lock(selected_sort_state.index_buffer);
					auto* elements=static_cast<unsigned short*>(lock.Get_Index_Array());
					original_element=elements[0];
					elements[0]=65000;
				}
				DX8Wrapper::Set_Index_Buffer(selected_sort_state.index_buffer,
					selected_sort_state.index_base_offset);
				try { SortingRendererClass::Insert_Triangles(0,1,0,3); }
				catch (const std::runtime_error& error) {
					invalid_source_element=std::strstr(error.what(),"index is outside")!=nullptr;
				}
				DX8Wrapper::Set_Index_Buffer(nullptr,0);
				{
					IndexBufferClass::WriteLockClass lock(selected_sort_state.index_buffer);
					static_cast<unsigned short*>(lock.Get_Index_Array())[0]=original_element;
				}
				DX8Wrapper::Set_Index_Buffer(selected_sort_state.index_buffer,
					selected_sort_state.index_base_offset);
				assert(invalid_source_element);
				DX8Wrapper::Set_Index_Buffer(nullptr,0);
				DX8Wrapper::Set_Vertex_Buffer(nullptr);
				control_ib->Release_Ref(); control_vb->Release_Ref();
				if (attempt==0) recorder.fail_next_buffer_upload();
				if (attempt==1) recorder.fail_next_pipeline_create();
				if (attempt==2) recorder.fail_next_draw();
				bool rejected=false;
				try { SortingRendererClass::Flush(); }
				catch (const std::runtime_error& error) {
					rejected=true;
				}
				assert(rejected==(attempt<3));
				assert(recorder.end_pass());
				SortingRendererClass::SortedTriangleWitness sorted[8]{};
				assert(SortingRendererClass::Copy_Last_Sorted_Triangles(sorted,8)==2U);
				{
				assert(sorted[0].depth<sorted[1].depth);
				assert(std::abs(sorted[0].depth+12.0f)<0.01f);
				assert(std::abs(sorted[1].depth+10.0f)<0.01f);
				assert(sorted[0].node!=sorted[1].node);
				if (!rejected) {
					const auto commands=recorder.snapshot();
					const auto first_draw=commands.find("draw pipeline=");
					const auto next_draw=commands.find("draw pipeline=",first_draw+1);
					const auto first_marker=commands.find("DX8Wrapper::Draw indexed first=");
					const auto second_marker=commands.find("DX8Wrapper::Draw indexed first=",first_marker+1);
					assert(first_draw!=std::string::npos && next_draw!=std::string::npos &&
						first_marker!=std::string::npos && second_marker!=std::string::npos);
					const auto last_indices=recorder.last_draw_index_bytes();
					assert(last_indices.size()==3*sizeof(unsigned short));
					unsigned short last_triangle[3]{};
					std::memcpy(last_triangle,last_indices.data(),sizeof(last_triangle));
					assert(last_triangle[0]==sorted[1].i &&
						last_triangle[1]==sorted[1].j && last_triangle[2]==sorted[1].k);
				}
				}
				WW3D::Enable_Sorting(false);
				bool direct_boundary=false;
				try { SortingRendererClass::Insert_Triangles(0,1,0,3); }
				catch (const std::runtime_error&) { direct_boundary=true; }
				assert(direct_boundary &&
					SortingRendererClass::Copy_Last_Sorted_Triangles(nullptr,0)==2U);
				WW3D::Enable_Sorting(true);
				DX8Wrapper::Set_Vertex_Buffer(nullptr);
				bool invalid_type=false;
				try { SortingRendererClass::Insert_Triangles(0,1,0,3); }
				catch (const std::runtime_error& error) {
					invalid_type=std::strstr(error.what(),"buffer type is unsupported")!=nullptr;
				}
				assert(invalid_type);
			}
			assert(recorder.resource_counts().total()==2);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			mesh->Peek_Model()->Register_For_Rendering();
		}
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			assert(recorder.begin_pass(pass,"original sorting nonfinite triangle negative"));
			auto* vertices=NEW_REF(SortingVertexBufferClass,(3));
			auto* indices=NEW_REF(SortingIndexBufferClass,(3));
			{
				VertexBufferClass::WriteLockClass lock(vertices);
				auto* values=static_cast<VertexFormatXYZNDUV2*>(lock.Get_Vertex_Array());
				for (int i=0;i<3;++i) values[i].x=values[i].y=values[i].z=0;
				values[0].z=std::numeric_limits<float>::quiet_NaN();
			}
			{
				IndexBufferClass::WriteLockClass lock(indices);
				auto* values=static_cast<unsigned short*>(lock.Get_Index_Array());
				values[0]=0; values[1]=1; values[2]=2;
			}
			DX8Wrapper::Set_Vertex_Buffer(vertices);
			DX8Wrapper::Set_Index_Buffer(indices,0);
			SortingRendererClass::Insert_Triangles(SphereClass(Vector3(0,0,0),1),0,1,0,3);
			DX8Wrapper::Set_Index_Buffer(nullptr,0);
			DX8Wrapper::Set_Vertex_Buffer(nullptr);
			bool rejected_nonfinite_triangle=false;
			try { SortingRendererClass::Flush(); }
			catch (const std::runtime_error& error) {
				rejected_nonfinite_triangle=std::strstr(error.what(),"sorting triangle has nonfinite depth")!=nullptr;
			}
			assert(rejected_nonfinite_triangle && recorder.end_pass());
			indices->Release_Ref(); vertices->Release_Ref();
		}
		SortingRendererClass::Deinit();
		WW3D::Enable_Sorting(previous_sorting);
		TheDX8MeshRenderer.Set_Camera(nullptr);
		recorder.destroy(color); recorder.destroy(depth);
		assert(recorder.resource_counts().total()==0 && textures.owners==0);
		_TheFileFactory=old_factory;
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		back->Release_Ref(); object->Release_Ref(); manager.Free_Assets();
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--sort-state")==0) {
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		auto* old_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		TheDX8MeshRenderer.Init();
		TheDX8MeshRenderer.Set_Camera(&camera);
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_Position(Vector3(0,0,-10));
		mesh->Render(render_info);
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=32; target.height=32; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"original sorting source state color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"original sorting source state depth");
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=32; pass.height=32;
		RenderStateStruct source;
		decltype(recorder.last_draw_index_bytes()) source_indices;
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			assert(recorder.begin_pass(pass,"original selected render-state snapshot"));
			TheDX8MeshRenderer.Flush();
			source_indices=recorder.last_draw_index_bytes();
			assert(source_indices.size()==3*sizeof(unsigned short));
			// Canonical source Flush releases its selected VB/IB. Exercise the
			// state snapshot/replay contract with explicitly owned source buffers.
			DX8Wrapper::Get_Render_State(source);
			assert(!source.vertex_buffers[0] && !source.index_buffer);
			auto* snapshot_vb=new DX8VertexBufferClass(DX8_FVF_XYZN,3,
				DX8VertexBufferClass::USAGE_DEFAULT);
			auto* snapshot_ib=new DX8IndexBufferClass(3);
			const float vertices[3][6]={{-1,-1,-10,0,0,1},
				{1,-1,-10,0,0,1},{0,1,-10,0,0,1}};
			std::memcpy(snapshot_vb->Get_CPU_Vertex_Buffer(),vertices,sizeof(vertices));
			unsigned short indices[3]{};
			std::memcpy(indices,source_indices.data(),sizeof(indices));
			std::memcpy(snapshot_ib->Get_CPU_Index_Buffer(),indices,sizeof(indices));
			DX8Wrapper::Set_Vertex_Buffer(snapshot_vb);
			DX8Wrapper::Set_Index_Buffer(snapshot_ib,0);
			snapshot_vb->Release_Ref();
			snapshot_ib->Release_Ref();
			DX8Wrapper::Apply_Render_State_Changes();
			DX8Wrapper::Get_Render_State(source);
			assert(source.vertex_buffers[0] && source.index_buffer &&
				source.vertex_buffer_types[0]==source.vertex_buffers[0]->Type() &&
				source.index_buffer_type==source.index_buffer->Type() &&
				source.material && source.Textures[0] &&
				source.vertex_buffers[0]->Engine_Refs()>0 &&
				source.index_buffer->Engine_Refs()>0);
			const auto vertex_refs=source.vertex_buffers[0]->Num_Refs();
			const auto index_refs=source.index_buffer->Num_Refs();
			const auto texture_refs=source.Textures[0]->Num_Refs();
			{
				RenderStateStruct copied;
				copied=source;
				assert(source.vertex_buffers[0]->Num_Refs()==vertex_refs+1 &&
					source.index_buffer->Num_Refs()==index_refs+1 &&
					source.Textures[0]->Num_Refs()==texture_refs+1);
			}
			assert(source.vertex_buffers[0]->Num_Refs()==vertex_refs &&
				source.index_buffer->Num_Refs()==index_refs &&
				source.Textures[0]->Num_Refs()==texture_refs);
			const auto selected_before=DX8Wrapper::Peek_Material();
			source.vertex_buffer_types[0]=BUFFER_TYPE_SORTING;
			bool invalid_type=false;
			try { DX8Wrapper::Set_Render_State(source); }
			catch (const std::runtime_error&) { invalid_type=true; }
			assert(invalid_type && DX8Wrapper::Peek_Material()==selected_before);
			source.vertex_buffer_types[0]=source.vertex_buffers[0]->Type();
			REF_PTR_SET(source.Textures[2],source.Textures[0]);
			bool unsupported_stage=false;
			try { DX8Wrapper::Set_Render_State(source); }
			catch (const std::runtime_error&) { unsupported_stage=true; }
			assert(unsupported_stage && DX8Wrapper::Peek_Texture(2)==nullptr);
			REF_PTR_RELEASE(source.Textures[2]);
			source.LightEnable[0]=true;
			source.Lights[0]={};
			bool invalid_light=false;
			try { DX8Wrapper::Set_Render_State(source); }
			catch (const std::runtime_error&) { invalid_light=true; }
			assert(invalid_light && DX8Wrapper::Peek_Material()==selected_before);
			source.LightEnable[0]=false;
			DX8Wrapper::Reset_Source_State();
			assert(source.vertex_buffers[0]->Num_Refs()>0 &&
				source.index_buffer->Num_Refs()>0 &&
				source.material->Num_Refs()>0);
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			DX8Wrapper::Set_Render_State(source);
			DX8Wrapper::Apply_Render_State_Changes();
			assert(DX8Wrapper::Peek_Material()==selected_before &&
				DX8Wrapper::Peek_Texture(0)==source.Textures[0]);
			recorder.fail_next_draw();
			bool failed_draw=false;
			try { DX8Wrapper::Draw_Triangles(0,1,0,3); }
			catch (const std::runtime_error&) { failed_draw=true; }
			assert(failed_draw && DX8Wrapper::Peek_Material()==selected_before);
			const auto before_replay=recorder.snapshot().size();
			DX8Wrapper::Draw_Triangles(0,1,0,3);
			assert(recorder.snapshot().find("draw pipeline=",before_replay)!=std::string::npos &&
				recorder.last_draw_index_bytes()==source_indices);
			const bool sorting_before=WW3D::Is_Sorting_Enabled();
			WW3D::Enable_Sorting(false);
			const auto before_direct=recorder.snapshot().size();
			SortingRendererClass::Insert_Triangles(0,1,0,3);
			assert(recorder.snapshot().find("draw pipeline=",before_direct)!=std::string::npos &&
				recorder.last_draw_index_bytes()==source_indices &&
				SortingRendererClass::Copy_Last_Sorted_Triangles(nullptr,0)==0);
			WW3D::Enable_Sorting(sorting_before);
			DX8Wrapper::Release_Render_State();
			assert(recorder.end_pass() && textures.owners==0);
		}
		assert(recorder.resource_counts().total()==2);
		DX8Wrapper::Reset_Source_State();
		{
			zh::original_runtime::OriginalGpuEdge recreated(recorder);
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			DX8Wrapper::Set_Render_State(source);
			DX8Wrapper::Apply_Render_State_Changes();
			const auto before_new_generation=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original source state reupload new edge generation"));
			DX8Wrapper::Draw_Triangles(0,1,0,3);
			assert(recorder.end_pass() &&
				recorder.snapshot().find("draw pipeline=",before_new_generation)!=std::string::npos &&
				recorder.last_draw_index_bytes()==source_indices);
			DX8Wrapper::Release_Render_State();
		}
		assert(recorder.resource_counts().total()==2);
		bool unbound=false;
		try { DX8Wrapper::Get_Render_State(source); }
		catch (const std::runtime_error&) { unbound=true; }
		assert(unbound);
		TheDX8MeshRenderer.Invalidate();
		TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
		TheDX8MeshRenderer.Set_Camera(nullptr);
		DX8Wrapper::Reset_Source_State();
		recorder.destroy(color); recorder.destroy(depth);
		assert(recorder.resource_counts().total()==0);
		_TheFileFactory=old_factory;
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		object->Release_Ref(); manager.Free_Assets();
		return 0;
	}
	if (argc==2 && (std::strcmp(argv[1],"--decal-cpu")==0 ||
		std::strcmp(argv[1],"--decal-aggregate")==0 ||
		std::strcmp(argv[1],"--vulkan-decal-aggregate")==0 ||
		std::strcmp(argv[1],"--decal-physical")==0 ||
		std::strcmp(argv[1],"--vulkan-decal-physical")==0)) {
		const bool vulkan_decal=std::strcmp(argv[1],"--vulkan-decal-physical")==0;
		const bool aggregate_decal=std::strcmp(argv[1],"--decal-aggregate")==0 ||
			std::strcmp(argv[1],"--vulkan-decal-aggregate")==0;
		const bool vulkan_aggregate=std::strcmp(argv[1],"--vulkan-decal-aggregate")==0;
		const bool decal_physical=vulkan_decal || std::strcmp(argv[1],"--decal-physical")==0;
		const bool previously_enabled=WW3D::Are_Decals_Enabled();
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		WW3D::Enable_Decals(true);
		WW3D::Set_Thumbnail_Enabled(false);
		mesh->Set_Position(Vector3(0,0,-10));
		DecalSystemClass source_system;
		DecalGeneratorClass* generator=source_system.Lock_Decal_Generator();
		assert(generator && generator->Peek_Decal_System()==&source_system);
		MaterialPassClass* authored_decal_material=generator->Get_Material();
		assert(authored_decal_material && authored_decal_material->Peek_Material()==nullptr &&
			authored_decal_material->Peek_Texture()==nullptr);
		if (decal_physical) {
			auto* authored_vertex_material=NEW_REF(VertexMaterialClass,());
			authored_vertex_material->Set_Lighting(false);
			authored_vertex_material->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
			authored_decal_material->Set_Material(authored_vertex_material);
			authored_vertex_material->Release_Ref();
			ShaderClass authored_shader;
			authored_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
			authored_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
			authored_decal_material->Set_Shader(authored_shader);
		}
		authored_decal_material->Release_Ref();
		generator->Set_Ortho_Projection(-2,2,-2,2,0,20);
		generator->Set_Transform(Matrix3D(true));
		generator->Set_Backface_Threshhold(-1.0f);
		WW3D::Enable_Decals(false);
		mesh->Create_Decal(generator);
		assert(generator->Get_Mesh_List().Peek_Head()==nullptr);
		WW3D::Enable_Decals(true);
		mesh->Create_Decal(generator);
		assert(generator->Get_Mesh_List().Peek_Head()==nullptr);
		generator->Apply_To_Translucent_Meshes(true);
		Matrix3D far_projector(true);
		far_projector.Set_Translation(Vector3(1000,1000,1000));
		generator->Set_Transform(far_projector);
		mesh->Create_Decal(generator);
		assert(generator->Get_Mesh_List().Peek_Head()==nullptr);
		generator->Set_Transform(Matrix3D(true));
		TriIndex* mutable_polys=const_cast<TriIndex*>(mesh->Peek_Model()->Get_Polygon_Array());
		const TriIndex original_poly=mutable_polys[0];
		mutable_polys[0].I=mesh->Peek_Model()->Get_Vertex_Count();
		bool invalid_decal_rejected=false;
		try { mesh->Create_Decal(generator); }
		catch (const std::runtime_error& error) {
			invalid_decal_rejected=std::strstr(error.what(),"source polygon index")!=nullptr;
		}
		mutable_polys[0]=original_poly;
		assert(invalid_decal_rejected && generator->Get_Mesh_List().Peek_Head()==nullptr);
		mesh->Create_Decal(generator);
		RenderObjClass* skin_hlod=manager.Create_Render_Obj("TEST.SKINHLOD");
		assert(skin_hlod && skin_hlod->Get_HTree());
		skin_hlod->Set_Position(Vector3(0,0,-10));
		(void)static_cast<HLodClass*>(skin_hlod)->Get_Bone_Transform(0);
		RenderObjClass* skin_child=skin_hlod->Get_Sub_Object(0);
		assert(skin_child && skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
		auto* skin_mesh=static_cast<MeshClass*>(skin_child);
		skin_mesh->Create_Decal(generator);
		unsigned authored_meshes=0;
		NonRefRenderObjListIterator owned(&generator->Get_Mesh_List());
		while (!owned.Is_Done()) { ++authored_meshes; owned.Next(); }
		assert(authored_meshes==2);
		if (decal_physical) {
			assert(mesh->Peek_Decal_Mesh() && skin_mesh->Peek_Decal_Mesh());
			WW3D::Enable_Decals(false);
			mesh->Peek_Decal_Mesh()->Render();
			WW3D::Enable_Decals(true);
			skin_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,true);
			skin_mesh->Peek_Decal_Mesh()->Render();
			mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			skin_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			zh::renderer::RecordingGpuDevice recorder;
			zh::renderer::TextureDesc target;
			target.width=32; target.height=32; target.render_target=true; target.sampled=false;
			const auto color=recorder.create_texture(target,"original direct decal color");
			target.format=zh::renderer::TextureFormat::depth24_stencil8;
			const auto depth=recorder.create_texture(target,"original direct decal depth");
			zh::renderer::RenderPassDesc pass;
			pass.color_targets[0]=color; pass.color_target_count=1;
			pass.depth_target=depth; pass.width=32; pass.height=32;
			auto select_camera=[&] {
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				Matrix4x4 projection;
				camera.Get_D3D_Projection_Matrix(&projection);
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,projection);
			};
			RenderObjClass* missing_object=manager.Create_Render_Obj("TEST.TRIANGLE");
			assert(missing_object && missing_object->Class_ID()==RenderObjClass::CLASSID_MESH);
			auto* missing_mesh=static_cast<MeshClass*>(missing_object);
			missing_mesh->Set_Position(Vector3(0,0,-10));
			DecalGeneratorClass* missing_generator=source_system.Lock_Decal_Generator();
			missing_generator->Set_Ortho_Projection(-2,2,-2,2,0,20);
			missing_generator->Set_Transform(Matrix3D(true));
			missing_generator->Set_Backface_Threshhold(-1.0f);
			missing_generator->Apply_To_Translucent_Meshes(true);
			MaterialPassClass* missing_pass=missing_generator->Get_Material();
			ShaderClass missing_shader;
			missing_shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
			missing_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
			missing_pass->Set_Shader(missing_shader);
			missing_pass->Release_Ref();
			missing_mesh->Create_Decal(missing_generator);
			assert(missing_mesh->Peek_Decal_Mesh());
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				select_camera();
				assert(recorder.begin_pass(pass,"original required decal source texture missing"));
				bool missing_required_texture=false;
				try { missing_mesh->Peek_Decal_Mesh()->Render(); }
				catch (const std::runtime_error& error) {
					missing_required_texture=std::strstr(error.what(),
						"texture stage is absent from active device generation")!=nullptr;
				}
				assert(missing_required_texture && recorder.end_pass());
			}
			const uint32 missing_id=missing_generator->Get_Decal_ID();
			source_system.Unlock_Decal_Generator(missing_generator);
			missing_mesh->Delete_Decal(missing_id);
			missing_object->Release_Ref();
			assert(recorder.resource_counts().total()==2);
			for (unsigned failure=0;failure<2;++failure) {
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				select_camera();
				if (failure==0) recorder.fail_next_buffer_upload();
				else recorder.fail_draw_after(1);
				const auto before=recorder.snapshot().size();
				assert(recorder.begin_pass(pass,"original direct decal injected failure"));
				bool rejected=false;
				try {
					mesh->Peek_Decal_Mesh()->Render();
					skin_mesh->Peek_Decal_Mesh()->Render();
				} catch (const std::runtime_error&) { rejected=true; }
				assert(rejected && recorder.end_pass());
				const auto partial=recorder.snapshot().substr(before);
				assert((partial.find("draw pipeline=")!=std::string::npos)==(failure==1));
			}
			assert(recorder.resource_counts().total()==2);
			const auto before_success=recorder.snapshot().size();
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				select_camera();
				assert(recorder.begin_pass(pass,"direct original concrete decal mesh owners"));
				mesh->Peek_Decal_Mesh()->Render();
				assert(std::fabs(DX8Wrapper::Snapshot_Source_State().transforms.at(D3DTS_WORLD)[2].W+10.0f)<0.001f);
				skin_mesh->Peek_Decal_Mesh()->Render();
				assert(std::fabs(DX8Wrapper::Snapshot_Source_State().transforms.at(D3DTS_WORLD)[2].W)<0.001f);
				assert(recorder.end_pass());
				const auto commands=recorder.snapshot().substr(before_success);
				const auto first=commands.find("draw pipeline=");
				const auto second=commands.find("draw pipeline=",first+1);
				assert(first!=std::string::npos && second!=std::string::npos &&
					commands.find("draw pipeline=",second+1)==std::string::npos &&
					commands.find("DX8Wrapper::Set_Transform=256")<first &&
					commands.find("DX8Wrapper::Set_Transform=256",first)<second &&
					commands.find("index_bits=16",first)<second &&
					commands.find("index_bits=16",second)!=std::string::npos);
				const auto drawn_indices=recorder.last_draw_index_bytes();
				assert(drawn_indices.size()==3*sizeof(unsigned short));
				unsigned short source_indices[3]{};
				std::memcpy(source_indices,drawn_indices.data(),sizeof(source_indices));
				assert(source_indices[0]==0 && source_indices[1]==1 && source_indices[2]==2);
			}
			recorder.destroy(color); recorder.destroy(depth);
			assert(recorder.resource_counts().total()==0);
#if defined(ZH_GPU_SHADER_DIR)
			if (vulkan_decal) {
				assert(SDL_Init(SDL_INIT_VIDEO));
				{
				zh::renderer::SdlGpuOptions options;
				options.shader_root=ZH_GPU_SHADER_DIR;
				options.debug=true;
				zh::renderer::SdlGpuDevice device(options);
				assert(device.capabilities().backend=="vulkan");
				zh::renderer::TextureDesc target;
				target.width=160; target.height=120; target.render_target=true; target.sampled=false;
				const auto color=device.create_texture(target,"original direct decal Vulkan color");
				target.format=zh::renderer::TextureFormat::depth24_stencil8;
				const auto depth=device.create_texture(target,"original direct decal Vulkan depth");
				assert(color && depth);
				zh::renderer::RenderPassDesc pass;
				pass.color_targets[0]=color; pass.color_target_count=1;
				pass.depth_target=depth; pass.width=160; pass.height=120;
				{
					zh::original_runtime::OriginalGpuEdge edge(device);
					DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
					Matrix4x4 projection;
					camera.Get_D3D_Projection_Matrix(&projection);
					DX8Wrapper::Set_Transform(D3DTS_PROJECTION,projection);
					unsigned decal_owner_index=0;
					for (DecalMeshClass* original_owner :
						{mesh->Peek_Decal_Mesh(),skin_mesh->Peek_Decal_Mesh()}) {
						assert(device.begin_pass(pass,"original direct concrete decal Vulkan owner"));
						original_owner->Render();
						assert(device.end_pass());
						const auto pixels=device.readback_rgba(color);
						assert(pixels.size()==160U*120U*4U);
						unsigned covered=0;
						for (size_t pixel=0;pixel<pixels.size();pixel+=4)
							if (pixels[pixel]>100 || pixels[pixel+1]>100 || pixels[pixel+2]>100)
								++covered;
						assert(covered>0 && covered<160U*120U);
						std::printf("original-decal-vulkan-%s-covered=%u\n",
							decal_owner_index++==0 ? "rigid" : "skin",covered);
					}
				}
				device.destroy(color); device.destroy(depth);
			}
				SDL_Quit();
			}
#else
			assert(!vulkan_decal);
#endif
			const uint32 id=generator->Get_Decal_ID();
			source_system.Unlock_Decal_Generator(generator);
			mesh->Delete_Decal(id);
			skin_mesh->Delete_Decal(id);
			skin_child->Release_Ref(); skin_hlod->Release_Ref();
			WW3D::Enable_Decals(previously_enabled);
			WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
			object->Release_Ref(); manager.Free_Assets();
			return 0;
		}
		TheDX8MeshRenderer.Init();
		TheDX8MeshRenderer.Set_Camera(&camera);
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		skin_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		LightEnvironmentClass decal_environment;
		decal_environment.Reset(Vector3(0,0,-10),Vector3(0.2f,0.2f,0.2f));
		decal_environment.Pre_Render_Update(Matrix3D(true));
		render_info.light_environment=&decal_environment;
		mesh->Render(render_info);
		skin_hlod->Render(render_info);
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		auto* old_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		zh::renderer::RecordingGpuDevice recorder;
		zh::renderer::TextureDesc target;
		target.width=32; target.height=32; target.render_target=true; target.sampled=false;
		const auto color=recorder.create_texture(target,"original CPU decal edge color");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(target,"original CPU decal edge depth");
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=32; pass.height=32;
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			assert(recorder.begin_pass(pass,"original decal CPU first physical edge"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
		}
		const auto queue_commands=recorder.snapshot();
		const auto first_queue_draw=queue_commands.find("draw pipeline=");
		const auto bias_begin=queue_commands.find("DX8Wrapper::Set_DX8_Render_State=47:8");
		const auto bias_end=queue_commands.find("DX8Wrapper::Set_DX8_Render_State=47:0",bias_begin);
		assert(first_queue_draw!=std::string::npos && bias_begin!=std::string::npos &&
			bias_end!=std::string::npos && first_queue_draw<bias_begin &&
			queue_commands.find("draw pipeline=",first_queue_draw+1)<bias_begin &&
			queue_commands.find("draw pipeline=",bias_begin)>bias_begin &&
			queue_commands.find("draw pipeline=",queue_commands.find("draw pipeline=",bias_begin)+1)<bias_end &&
			queue_commands.find("draw pipeline=",bias_end)==std::string::npos &&
			textures.owners==0);
		auto queue_original=[&] {
			mesh->Render(render_info);
			skin_hlod->Render(render_info);
		};
		for (unsigned decal_draw=0;decal_draw<2;++decal_draw) {
			queue_original();
			const auto before_failure=recorder.snapshot().size();
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
				recorder.fail_draw_after(2+decal_draw);
				assert(recorder.begin_pass(pass,"original queued decal failed draw"));
				bool rejected=false;
				try { TheDX8MeshRenderer.Flush(); }
				catch (const std::runtime_error& error) {
					rejected=std::strstr(error.what(),"draw")!=nullptr;
				}
				assert(rejected && recorder.end_pass());
			}
			const auto failed=recorder.snapshot().substr(before_failure);
			const auto failed_bias=failed.find("DX8Wrapper::Set_DX8_Render_State=47:8");
			const auto reset_bias=failed.find("DX8Wrapper::Set_DX8_Render_State=47:0",failed_bias);
			assert(failed_bias!=std::string::npos && reset_bias!=std::string::npos &&
				failed.find("draw pipeline=",reset_bias)==std::string::npos &&
				textures.owners==0);
			// Abandon the partial source frame, then let the original mesh and
			// HLOD owners rebuild their category/decal lists in a fresh session.
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			mesh->Peek_Model()->Register_For_Rendering();
			skin_mesh->Peek_Model()->Register_For_Rendering();
			queue_original();
			const auto before_retry=recorder.snapshot().size();
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
				assert(recorder.begin_pass(pass,"original queued decal source retry"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
			}
			const auto retried=recorder.snapshot().substr(before_retry);
			const auto retry_bias=retried.find("DX8Wrapper::Set_DX8_Render_State=47:8");
			assert(retry_bias!=std::string::npos &&
				retried.find("draw pipeline=",retry_bias)!=std::string::npos &&
				retried.find("draw pipeline=",retried.find("draw pipeline=",retry_bias)+1)!=std::string::npos &&
				retried.find("DX8Wrapper::Set_DX8_Render_State=47:0",retry_bias)!=std::string::npos &&
				textures.owners==0);
		}
		if (aggregate_decal) {
			auto* immediate=NEW_REF(MaterialPassClass,());
			auto* unlit=NEW_REF(VertexMaterialClass,());
			unlit->Set_Lighting(false);
			unlit->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
			immediate->Set_Material(unlit);
			unlit->Release_Ref();
			ShaderClass shader;
			shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
			immediate->Set_Shader(shader);
			auto* culled=NEW_REF(MaterialPassClass,());
			culled->Set_Shader(shader);
			OBBoxClass source_volume(Vector3(0,0,-10),Vector3(100,100,100),Matrix3x3(true));
			culled->Set_Cull_Volume(&source_volume);
			const bool old_culling=MaterialPassClass::Is_Per_Polygon_Culling_Enabled();
			MaterialPassClass::Enable_Per_Polygon_Culling(true);
			RenderObjClass* delayed_object=manager.Create_Render_Obj("TEST.TRIANGLE");
			assert(delayed_object && delayed_object->Class_ID()==RenderObjClass::CLASSID_MESH);
			auto* delayed_mesh=static_cast<MeshClass*>(delayed_object);
			delayed_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			delayed_mesh->Set_Position(Vector3(0,0,-10));
			auto queue_aggregate=[&] {
				render_info.Push_Material_Pass(immediate);
				skin_hlod->Render(render_info);
				render_info.Pop_Material_Pass();
				render_info.Push_Material_Pass(immediate);
				render_info.Push_Material_Pass(culled);
				mesh->Render(render_info);
				render_info.Pop_Material_Pass();
				render_info.Pop_Material_Pass();
				render_info.Push_Material_Pass(immediate);
				render_info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
				delayed_mesh->Render(render_info);
				render_info.Pop_Override_Flags();
				render_info.Pop_Material_Pass();
			};
			queue_aggregate();
			const auto before_aggregate=recorder.snapshot().size();
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
				assert(recorder.begin_pass(pass,"original complete decal/material aggregate"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
			}
			const auto aggregate=recorder.snapshot().substr(before_aggregate);
			const auto first_bias=aggregate.find("DX8Wrapper::Set_DX8_Render_State=47:8");
			const auto last_bias=aggregate.find("DX8Wrapper::Set_DX8_Render_State=47:0",first_bias);
			const auto count_draws=[](const std::string& commands) {
				unsigned count=0;
				for (auto pos=commands.find("draw pipeline=");pos!=std::string::npos;
					pos=commands.find("draw pipeline=",pos+1)) ++count;
				return count;
			};
			const unsigned category_draws=count_draws(aggregate.substr(0,first_bias));
			assert(first_bias!=std::string::npos && last_bias!=std::string::npos &&
				category_draws>=5 &&
				aggregate.find("DX8Wrapper::Set_Vertex_Buffer dynamic offset=")<first_bias &&
				aggregate.find("DX8Wrapper::Set_Index_Buffer dynamic offset=")<first_bias &&
				count_draws(aggregate.substr(first_bias,last_bias-first_bias))==2 &&
				aggregate.find("draw pipeline=",last_bias)!=std::string::npos &&
				mesh->Peek_Model()->Peek_Single_Texture()!=nullptr &&
				!mesh->Peek_Model()->Peek_Single_Texture()->Is_Missing_Texture() &&
				recorder.last_draw_index_bytes().size()==3*sizeof(unsigned short) &&
				textures.owners==0);
			queue_aggregate();
			const auto before_failure=recorder.snapshot().size();
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
				recorder.fail_draw_after(category_draws+1);
				assert(recorder.begin_pass(pass,"original aggregate later decal failure"));
				bool rejected=false;
				try { TheDX8MeshRenderer.Flush(); }
				catch (const std::runtime_error& error) {
					rejected=std::strstr(error.what(),"draw")!=nullptr;
				}
				assert(rejected && recorder.end_pass());
			}
			const auto failed=recorder.snapshot().substr(before_failure);
			const auto failed_bias=failed.find("DX8Wrapper::Set_DX8_Render_State=47:8");
			const auto failed_reset=failed.find("DX8Wrapper::Set_DX8_Render_State=47:0",failed_bias);
			assert(failed_bias!=std::string::npos && failed_reset!=std::string::npos &&
				count_draws(failed.substr(failed_bias,failed_reset-failed_bias))==1 &&
				count_draws(failed.substr(failed_reset))==0);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			mesh->Peek_Model()->Register_For_Rendering();
			skin_mesh->Peek_Model()->Register_For_Rendering();
			delayed_mesh->Peek_Model()->Register_For_Rendering();
			queue_aggregate();
			const auto before_retry=recorder.snapshot().size();
			{
				zh::original_runtime::OriginalGpuEdge edge(recorder);
				DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
				DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
				assert(recorder.begin_pass(pass,"original complete aggregate requeued"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
			}
			const auto retried=recorder.snapshot().substr(before_retry);
			const auto retry_bias=retried.find("DX8Wrapper::Set_DX8_Render_State=47:8");
			const auto retry_reset=retried.find("DX8Wrapper::Set_DX8_Render_State=47:0",retry_bias);
			assert(retry_bias!=std::string::npos && retry_reset!=std::string::npos &&
				count_draws(retried.substr(retry_bias,retry_reset-retry_bias))==2 &&
				count_draws(retried.substr(retry_reset))>0 && textures.owners==0);
#if defined(ZH_GPU_SHADER_DIR)
			if (vulkan_aggregate) {
				assert(SDL_Init(SDL_INIT_VIDEO));
				for (unsigned width : {160U,240U}) {
					zh::renderer::SdlGpuOptions options;
					options.debug=true;
					options.shader_root=ZH_GPU_SHADER_DIR;
					zh::renderer::SdlGpuDevice device(options);
					assert(device.capabilities().backend=="vulkan");
					const unsigned height=width*3/4;
					zh::renderer::TextureDesc target;
					target.width=width; target.height=height;
					target.render_target=true; target.sampled=false;
					const auto gpu_color=device.create_texture(target,"original full aggregate color");
					target.format=zh::renderer::TextureFormat::depth24_stencil8;
					const auto gpu_depth=device.create_texture(target,"original full aggregate depth");
					assert(gpu_color && gpu_depth);
					zh::renderer::RenderPassDesc gpu_pass;
					gpu_pass.color_targets[0]=gpu_color; gpu_pass.color_target_count=1;
					gpu_pass.depth_target=gpu_depth; gpu_pass.width=width; gpu_pass.height=height;
					TheDX8MeshRenderer.Invalidate();
					TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
					mesh->Peek_Model()->Register_For_Rendering();
					skin_mesh->Peek_Model()->Register_For_Rendering();
					delayed_mesh->Peek_Model()->Register_For_Rendering();
					queue_aggregate();
					{
						zh::original_runtime::OriginalGpuEdge edge(device);
						DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
						Matrix4x4 projection;
						camera.Get_D3D_Projection_Matrix(&projection);
						DX8Wrapper::Set_Transform(D3DTS_PROJECTION,projection);
						assert(device.begin_pass(gpu_pass,"original full aggregate Vulkan Flush"));
						TheDX8MeshRenderer.Flush();
						assert(device.end_pass());
					}
					const auto pixels=device.readback_rgba(gpu_color);
					assert(pixels.size()==width*height*4U);
					unsigned covered=0;
					for (size_t pixel=0;pixel<pixels.size();pixel+=4)
						if (pixels[pixel]>100 || pixels[pixel+1]>100 || pixels[pixel+2]>100)
							++covered;
					assert(covered>0 && covered<width*height && textures.owners==0);
					std::printf("original-full-aggregate-%ux%u-covered=%u\n",width,height,covered);
					device.destroy(gpu_color); device.destroy(gpu_depth);
				}
				SDL_Quit();
			}
#else
			assert(!vulkan_aggregate);
#endif
			MaterialPassClass::Enable_Per_Polygon_Culling(old_culling);
			delayed_object->Release_Ref();
			culled->Release_Ref();
			immediate->Release_Ref();
		}
		_TheFileFactory=old_factory;
		TheDX8MeshRenderer.Invalidate();
		TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
		TheDX8MeshRenderer.Set_Camera(nullptr);
		recorder.destroy(color); recorder.destroy(depth);
		assert(recorder.resource_counts().total()==0);
		const uint32 decal_id=generator->Get_Decal_ID();
		source_system.Unlock_Decal_Generator(generator);
		mesh->Delete_Decal(decal_id);
		skin_mesh->Delete_Decal(decal_id);
		DecalGeneratorClass* retry_generator=source_system.Lock_Decal_Generator();
		assert(retry_generator && retry_generator->Get_Decal_ID()!=decal_id);
		retry_generator->Set_Ortho_Projection(-2,2,-2,2,0,20);
		retry_generator->Set_Transform(Matrix3D(true));
		retry_generator->Set_Backface_Threshhold(-1.0f);
		retry_generator->Apply_To_Translucent_Meshes(true);
		mesh->Create_Decal(retry_generator);
		assert(retry_generator->Get_Mesh_List().Peek_Head()==mesh);
		const uint32 retry_id=retry_generator->Get_Decal_ID();
		source_system.Unlock_Decal_Generator(retry_generator);
		mesh->Delete_Decal(retry_id);
		skin_child->Release_Ref();
		skin_hlod->Release_Ref();
		WW3D::Enable_Decals(previously_enabled);
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		object->Release_Ref();
		manager.Free_Assets();
		return 0;
	}
#if defined(ZH_GPU_SHADER_DIR)
	if (argc==2 && std::strcmp(argv[1],"--vulkan-sorting")==0) {
		assert(SDL_Init(SDL_INIT_VIDEO));
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		auto* old_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		const bool previous_thumbnail=WW3D::Get_Thumbnail_Enabled();
		const bool previous_sorting=WW3D::Is_Sorting_Enabled();
		WW3D::Set_Thumbnail_Enabled(false);
		TheDX8MeshRenderer.Init();
		WW3D::Enable_Sorting(true);
		TheDX8MeshRenderer.Set_Camera(&camera);
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,true);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_ObjectScale(5.0f);
		mesh->Set_Position(Vector3(-1,0,-10));
		RenderObjClass* back=manager.Create_Render_Obj("TEST.ZERO01");
		assert(back && back->Class_ID()==RenderObjClass::CLASSID_MESH);
		auto* back_mesh=static_cast<MeshClass*>(back);
		back_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,true);
		back_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		back_mesh->Set_ObjectScale(5.0f);
		back_mesh->Set_Position(Vector3(1,0,-12));
		camera.Set_Viewport(Vector2(0.25f,0.25f),Vector2(0.75f,0.75f));
		camera.Set_Zbuffer_Range(0.1f,0.9f);
		Matrix3D camera_world(true);
		camera_world.Set_Translation(Vector3(0.5f,0.0f,0.0f));
		camera.Set_Transform(camera_world);
		for (unsigned generation=0;generation<2;++generation) {
			zh::renderer::SdlGpuOptions options;
			options.debug=true; options.shader_root=ZH_GPU_SHADER_DIR;
			zh::renderer::SdlGpuDevice device(options);
			assert(device.capabilities().backend=="vulkan");
			zh::renderer::TextureDesc target;
			target.width=generation ? 240 : 160;
			target.height=generation ? 160 : 120;
			target.format=zh::renderer::TextureFormat::rgba8;
			target.render_target=true;
			const auto color=device.create_texture(target,"original sorted meshes Vulkan color");
			target.format=zh::renderer::TextureFormat::depth24_stencil8;
			const auto depth=device.create_texture(target,"original sorted meshes Vulkan depth");
			assert(color && depth);
			zh::renderer::RenderPassDesc pass;
			pass.color_targets[0]=color; pass.color_target_count=1;
			pass.depth_target=depth; pass.width=target.width; pass.height=target.height;
			{
				zh::original_runtime::OriginalGpuEdge edge(device);
				auto frame=[&](bool both) {
					assert(device.begin_pass(pass,"original source sorted meshes Vulkan"));
					zh::renderer::ViewportDesc invalid_viewport;
					invalid_viewport.width=static_cast<float>(pass.width+1);
					invalid_viewport.height=static_cast<float>(pass.height);
					assert(!device.set_viewport(invalid_viewport));
					camera.Apply();
					mesh->Render(render_info);
					if (both) back_mesh->Render(render_info);
					TheDX8MeshRenderer.Flush();
					SortingRendererClass::Flush();
					assert(device.end_pass());
					return device.readback_rgba(color);
				};
				const auto pixels=frame(true);
				SortingRendererClass::SortedTriangleWitness order[8]{};
				assert(SortingRendererClass::Copy_Last_Sorted_Triangles(order,8)==2 &&
					order[0].depth<order[1].depth && order[0].node!=order[1].node);
				assert(pixels.size()==static_cast<std::size_t>(pass.width)*pass.height*4U);
				unsigned covered=0;
				for (std::size_t i=0;i<pixels.size();i+=4)
					if (pixels[i]!=5 || pixels[i+1]!=5 || pixels[i+2]!=10) ++covered;
				assert(covered>0 && covered<pass.width*pass.height && textures.owners==0);
				for (unsigned y=0;y<pass.height;++y)
					for (unsigned x=0;x<pass.width;++x)
						if (x<pass.width/4 || x>=3*pass.width/4 ||
							y<pass.height/4 || y>=3*pass.height/4) {
							const auto p=4U*(y*pass.width+x);
							assert(pixels[p]==5 && pixels[p+1]==5 && pixels[p+2]==10);
						}
				const auto single_pixels=frame(false);
				assert(SortingRendererClass::Copy_Last_Sorted_Triangles(nullptr,0)==1 &&
					pixels!=single_pixels);
				TheDX8MeshRenderer.Invalidate();
				TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
				mesh->Peek_Model()->Register_For_Rendering();
				back_mesh->Peek_Model()->Register_For_Rendering();
			}
			device.destroy(depth); device.destroy(color);
			assert(device.wait_idle());
		}
		SortingRendererClass::Deinit();
		WW3D::Enable_Sorting(previous_sorting);
		TheDX8MeshRenderer.Set_Camera(nullptr);
		_TheFileFactory=old_factory;
		WW3D::Set_Thumbnail_Enabled(previous_thumbnail);
		back->Release_Ref(); object->Release_Ref(); manager.Free_Assets();
		SDL_Quit();
		return 0;
	}
	if (argc==2 && std::strcmp(argv[1],"--vulkan-category")==0) {
		assert(SDL_Init(SDL_INIT_VIDEO));
		{
		zh::renderer::SdlGpuOptions options;
		options.debug=true;
		options.shader_root=ZH_GPU_SHADER_DIR;
		zh::renderer::SdlGpuDevice device(options);
		assert(device.capabilities().backend=="vulkan");
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		textures.files["mytex2.tga"]=original_targa();
		textures.files["MYTEX2.TGA"]=textures.files["mytex2.tga"];
		auto* old_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		WW3D::Set_Thumbnail_Enabled(false);
		DefaultStaticSortListClass active_sort_list;
		WW3D::Override_Current_Static_Sort_Lists(&active_sort_list);
		TheDX8MeshRenderer.Init();
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
		mesh->Set_ObjectScale(5.0f);
		mesh->Set_Position(Vector3(0,0,-10));
		auto* tree_gpu=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.CULLTREE"));
		assert(tree_gpu && tree_gpu->Peek_Model()->Has_Cull_Tree());
		tree_gpu->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
		tree_gpu->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
		tree_gpu->Set_ObjectScale(5.0f);
		tree_gpu->Set_Position(Vector3(0,0,-10));
		tree_gpu->Peek_Model()->Register_For_Rendering();
		zh::renderer::TextureDesc target;
		target.width=160; target.height=120;
		target.format=zh::renderer::TextureFormat::rgba8;
		target.render_target=true;
		auto color=device.create_texture(target,"original category lit color target");
		target.format=zh::renderer::TextureFormat::depth24_stencil8;
		auto depth=device.create_texture(target,"original category lit depth target");
		assert(color && depth);
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=160; pass.height=120;
		{
			zh::original_runtime::OriginalGpuEdge edge(device);
			TheDX8MeshRenderer.Set_Camera(&camera);
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			Matrix4x4 projection;
			camera.Get_D3D_Projection_Matrix(&projection);
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,projection);
			LightEnvironmentClass environment;
			auto frame=[&](MeshClass* selected, const Vector3& ambient,
				LightClass* light=nullptr,unsigned repeats=1) {
				environment.Reset(Vector3(0,0,-10),ambient);
				if (light) for (unsigned i=0;i<repeats;++i) environment.Add_Light(*light);
				assert(environment.Get_Light_Count()==(light ? static_cast<int>(repeats):0));
				environment.Pre_Render_Update(Matrix3D(true));
				render_info.light_environment=&environment;
				selected->Render(render_info);
				assert(selected->Get_Lighting_Environment() &&
					selected->Get_Lighting_Environment()->Get_Light_Count()==
						environment.Get_Light_Count());
				if (light) assert(selected->Get_Lighting_Environment()->isPointLight(0)==
					(light->Get_Type()==LightClass::POINT));
				assert(selected->Peek_Model()->Has_Polygon_Renderers());
				assert(device.begin_pass(pass,"original owned W3D lit category Vulkan frame"));
				TheDX8MeshRenderer.Flush();
				// The original alpha-override tail restores a pending material and ALPHAREF
				// *after* its draw; only inspect the settled source snapshot otherwise.
				if (render_info.alphaOverride==1.0f) {
					const auto source=DX8Wrapper::Snapshot_Source_State();
					assert(source.light_environment_selected);
					assert(source.light_enabled[0]==(light!=nullptr));
					if (light) assert(source.lights[0].Type==
						(light->Get_Type()==LightClass::POINT ? D3DLIGHT_POINT:D3DLIGHT_DIRECTIONAL));
				}
				assert(device.end_pass());
				const auto pixels=device.readback_rgba(color);
				assert(pixels.size()==static_cast<std::size_t>(pass.width)*pass.height*4U);
				unsigned lit_pixels=0;
				for (std::size_t i=0;i<pixels.size();i+=4)
					if (pixels[i]!=5 || pixels[i+1]!=5 || pixels[i+2]!=10) ++lit_pixels;
				assert(lit_pixels>0 && textures.owners==0);
				return pixels;
			};
			auto* zero_mesh=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.ZERO01"));
			assert(zero_mesh);
			zero_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			zero_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
			zero_mesh->Set_ObjectScale(5.0f);
			zero_mesh->Set_Position(Vector3(0,0,-10));
			zero_mesh->Peek_Model()->Register_For_Rendering();
			std::array<MeshClass*,2> uv_meshes{};
			for (unsigned family=0;family<uv_meshes.size();++family) {
				uv_meshes[family]=static_cast<MeshClass*>(manager.Create_Render_Obj(
					family==0 ? "TEST.LITONE01":"TEST.LITTWO01"));
				assert(uv_meshes[family]);
				uv_meshes[family]->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
				uv_meshes[family]->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
				uv_meshes[family]->Set_ObjectScale(5.0f);
				uv_meshes[family]->Set_Position(Vector3(0,0,-10));
				uv_meshes[family]->Peek_Model()->Register_For_Rendering();
			}
			const auto ambient_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f));
			const auto tree_base_pixels=frame(tree_gpu,Vector3(0.15f,0.2f,0.25f));
			RenderObjClass* skin_hlod=manager.Create_Render_Obj("TEST.SKINHLOD");
			assert(skin_hlod && skin_hlod->Get_HTree());
			RenderObjClass* skin_child=skin_hlod->Get_Sub_Object(0);
			assert(skin_child && skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
			auto* owned_skin=static_cast<MeshClass*>(skin_child);
			owned_skin->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			owned_skin->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
			owned_skin->Peek_Model()->Register_For_Rendering();
			skin_hlod->Set_ObjectScale(5.0f);
			skin_hlod->Set_Position(Vector3(0,0,-10));
			auto skin_frame=[&]() {
				environment.Reset(Vector3(0,0,-10),Vector3(0.15f,0.2f,0.25f));
				environment.Pre_Render_Update(Matrix3D(true));
				render_info.light_environment=&environment;
				skin_hlod->Render(render_info);
				assert(device.begin_pass(pass,"original HLOD skin category Vulkan frame"));
				TheDX8MeshRenderer.Flush();
				assert(device.end_pass());
				const auto pixels=device.readback_rgba(color);
				assert(pixels.size()==static_cast<std::size_t>(pass.width)*pass.height*4U);
				return pixels;
			};
			const auto original_skin_pixels=skin_frame();
			unsigned skin_coverage=0;
			for (std::size_t i=0;i<original_skin_pixels.size();i+=4)
				if (original_skin_pixels[i]!=5 || original_skin_pixels[i+1]!=5 ||
					original_skin_pixels[i+2]!=10) ++skin_coverage;
			assert(skin_coverage>0);
			auto* immediate=NEW_REF(MaterialPassClass,());
			auto* pass_material=NEW_REF(VertexMaterialClass,());
			pass_material->Set_Lighting(true);
			pass_material->Set_Emissive(Vector3(0.65f,0.05f,0.02f));
			immediate->Set_Material(pass_material);
			pass_material->Release_Ref();
			ShaderClass pass_shader;
			pass_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
			pass_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
			immediate->Set_Shader(pass_shader);
			render_info.Push_Material_Pass(immediate);
			const auto additional_rigid_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f));
			assert(additional_rigid_pixels!=ambient_pixels);
			const auto additional_skin_pixels=skin_frame();
			render_info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
			mesh->Render(render_info);
			render_info.Pop_Override_Flags();
			assert(device.begin_pass(pass,"original rigid delayed-only Vulkan frame"));
			TheDX8MeshRenderer.Flush();
			assert(device.end_pass());
			const auto delayed_pixels=device.readback_rgba(color);
			assert(delayed_pixels.size()==ambient_pixels.size() &&
				delayed_pixels!=ambient_pixels && delayed_pixels!=original_skin_pixels);
			unsigned delayed_coverage=0;
			for (std::size_t i=0;i<delayed_pixels.size();i+=4)
				if (delayed_pixels[i]!=5 || delayed_pixels[i+1]!=5 || delayed_pixels[i+2]!=10)
					++delayed_coverage;
			assert(delayed_coverage>0);
			const bool prior_per_polygon=MaterialPassClass::Is_Per_Polygon_Culling_Enabled();
			MaterialPassClass::Enable_Per_Polygon_Culling(true);
			OBBoxClass covered_volume(Vector3(0,0,-10),Vector3(100,100,100),Matrix3x3(true));
			immediate->Set_Cull_Volume(&covered_volume);
			const auto selected_apt_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f));
			assert(selected_apt_pixels!=ambient_pixels);
			const auto tree_covered_pixels=frame(tree_gpu,Vector3(0.15f,0.2f,0.25f));
			assert(tree_covered_pixels!=tree_base_pixels);
			OBBoxClass tree_outside(Vector3(1000,1000,-10),Vector3(1,1,1),Matrix3x3(true));
			immediate->Set_Cull_Volume(&tree_outside);
			assert(frame(tree_gpu,Vector3(0.15f,0.2f,0.25f))==tree_base_pixels);
			Matrix3x3 backface_basis(true);
			backface_basis.Rotate_X(WWMATH_PI);
			OBBoxClass backface_cull(Vector3(0,0,-10),Vector3(100,100,100),backface_basis);
			immediate->Set_Cull_Volume(&backface_cull);
			const auto rejected_apt_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f));
			assert(rejected_apt_pixels==ambient_pixels);
			assert(frame(tree_gpu,Vector3(0.15f,0.2f,0.25f))==tree_base_pixels);
			immediate->Set_Cull_Volume(nullptr);
			MaterialPassClass::Enable_Per_Polygon_Culling(prior_per_polygon);
			render_info.Pop_Material_Pass();
			assert(additional_skin_pixels!=original_skin_pixels);
			immediate->Release_Ref();
			skin_hlod->Set_Position(Vector3(1,0,-10));
			const auto shifted_skin_pixels=skin_frame();
			assert(shifted_skin_pixels!=original_skin_pixels);
			skin_hlod->Set_Position(Vector3(0,0,-10));
			assert(skin_frame()==original_skin_pixels);
			skin_child->Release_Ref();
			skin_hlod->Release_Ref();
			const auto zero_ambient_pixels=frame(zero_mesh,Vector3(0.15f,0.2f,0.25f));
			Matrix3D source_direction(true);
			source_direction.Rotate_X(WWMATH_PI);
			LightClass directional(LightClass::DIRECTIONAL);
			directional.Set_Transform(source_direction);
			directional.Set_Diffuse(Vector3(0.3f,0.2f,0.1f));
			directional.Set_Ambient(Vector3(0,0,0));
			const auto directional_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f),&directional);
			assert(ambient_pixels!=directional_pixels);
			const auto ambient_retry_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f));
			assert(ambient_pixels==ambient_retry_pixels);
			LightClass point(LightClass::POINT);
			point.Set_Position(Vector3(0,0,-100));
			point.Set_Diffuse(Vector3(0.06f,0.08f,0.9f));
			point.Set_Ambient(Vector3(0,0,0));
			point.Set_Near_Attenuation_Range(0,2);
			point.Set_Far_Attenuation_Range(2,10);
			const auto point_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f),&point);
			const auto zero_point_pixels=frame(zero_mesh,Vector3(0.15f,0.2f,0.25f),&point);
			assert(zero_point_pixels==zero_ambient_pixels);
			assert(point_pixels==ambient_pixels && directional_pixels!=point_pixels);
			point.Set_Position(Vector3(0,0,-9));
			const auto near_point_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f),&point);
			assert(near_point_pixels!=ambient_pixels && near_point_pixels!=directional_pixels);
			const auto four_pixels=frame(zero_mesh,Vector3(0.15f,0.2f,0.25f),&directional,4);
			assert(four_pixels!=zero_ambient_pixels);
			render_info.alphaOverride=0.5f;
			const auto alpha_pixels=frame(mesh,Vector3(0.15f,0.2f,0.25f),&directional);
			assert(alpha_pixels!=directional_pixels);
			render_info.alphaOverride=1.0f;
			for (auto* selected:uv_meshes) {
				const auto uv_pixels=frame(selected,Vector3(0.15f,0.2f,0.25f),&directional);
				assert(uv_pixels!=ambient_pixels);
			}
			// Recreate physical attachments while original mesh/category owners remain live.
			device.destroy(depth); device.destroy(color);
			assert(device.wait_idle());
			pass.width=240; pass.height=160;
			target.width=pass.width; target.height=pass.height;
			target.format=zh::renderer::TextureFormat::rgba8;
			color=device.create_texture(target,"resized original lit color target");
			target.format=zh::renderer::TextureFormat::depth24_stencil8;
			depth=device.create_texture(target,"resized original lit depth target");
			assert(color && depth);
			pass.color_targets[0]=color; pass.depth_target=depth;
			assert(frame(mesh,Vector3(0.15f,0.2f,0.25f),&directional).size()==
				240U*160U*4U);
			for (auto* selected:uv_meshes) selected->Release_Ref();
			zero_mesh->Release_Ref();
			tree_gpu->Release_Ref();
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
		}
		device.destroy(depth); device.destroy(color);
		assert(device.wait_idle());
		WW3D::Reset_Current_Static_Sort_Lists_To_Default();
		_TheFileFactory=old_factory;
		object->Release_Ref();
		manager.Free_Assets();
		}
		SDL_Quit();
		return 0;
	}
#endif
	if (argc == 2 && std::strcmp(argv[1], "--device-edge") == 0)
	{
		OwnedFactory textures;
		textures.files["mytex.tga"]=original_targa();
		textures.files["MYTEX.TGA"]=textures.files["mytex.tga"];
		textures.files["mytex2.tga"]=original_targa();
		textures.files["MYTEX2.TGA"]=textures.files["mytex2.tga"];
		auto* old_factory=_TheFileFactory;
		_TheFileFactory=&textures;
		WW3D::Set_Thumbnail_Enabled(false);
		DefaultStaticSortListClass active_sort_list;
		WW3D::Override_Current_Static_Sort_Lists(&active_sort_list);
		TheDX8MeshRenderer.Init();
		mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT, false);
		mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
		mesh->Set_Position(Vector3(0, 0, -10));
		std::array<RenderObjClass *, 5> variant_objects{};
		mesh->Render(render_info);
		assert(mesh->Peek_Model()->Has_Polygon_Renderers());
		TheDX8MeshRenderer.Set_Camera(&camera);
		zh::renderer::RecordingGpuDevice recorder(32);
		bool unbound_rejected = false;
		try { TheDX8MeshRenderer.Flush(); }
		catch (const std::runtime_error &error) {
			unbound_rejected = std::strstr(error.what(), "GPU translation session") != nullptr;
		}
		assert(unbound_rejected && recorder.resource_counts().total() == 0);
		mesh->Render(render_info);
		zh::renderer::TextureDesc color_desc;
		color_desc.width=32; color_desc.height=32;
		color_desc.render_target=true; color_desc.sampled=false;
		const auto color=recorder.create_texture(color_desc,"owned original category pass target");
		auto depth_desc=color_desc;
		depth_desc.format=zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth=recorder.create_texture(depth_desc,"owned original category depth");
		zh::renderer::RenderPassDesc pass;
		pass.color_targets[0]=color; pass.color_target_count=1;
		pass.depth_target=depth; pass.width=32; pass.height=32;
		{
			zh::original_runtime::OriginalGpuEdge edge(recorder);
			const std::array<const char*,5> fixture_names{{
				"TEST.ZERO01","TEST.TWO01","TEST.TREADSL01",
				"TEST.LITONE01","TEST.LITTWO01"}};
			for (unsigned i=0; i<variant_objects.size(); ++i) {
				variant_objects[i]=manager.Create_Render_Obj(fixture_names[i]);
				assert(variant_objects[i] && variant_objects[i]->Class_ID()==RenderObjClass::CLASSID_MESH);
				auto *variant_mesh=static_cast<MeshClass *>(variant_objects[i]);
				variant_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
				variant_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
				variant_mesh->Set_Position(Vector3(0,0,-10));
			}
			const auto fvf_one=DX8FVFCategoryContainer::Define_FVF(
				static_cast<MeshClass*>(variant_objects[3])->Peek_Model(),true);
			const auto fvf_two=DX8FVFCategoryContainer::Define_FVF(
				static_cast<MeshClass*>(variant_objects[4])->Peek_Model(),true);
			assert(fvf_one==DX8_FVF_XYZNUV1 && fvf_two==DX8_FVF_XYZNUV2);
			for (auto *variant_object : variant_objects)
				static_cast<MeshClass *>(variant_object)->Peek_Model()->Register_For_Rendering();
			auto* tree_mesh=static_cast<MeshClass*>(manager.Create_Render_Obj("TEST.CULLTREE"));
			assert(tree_mesh && tree_mesh->Peek_Model()->Has_Cull_Tree());
			tree_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			tree_mesh->Set_Position(Vector3(0,0,-10));
			tree_mesh->Peek_Model()->Register_For_Rendering();
			DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
			DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));
			assert(recorder.begin_pass(pass,"caller-owned original rigid/category frame"));
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				std::fprintf(stderr,"original category diagnostic: %s\n",error.what());
				throw;
			}
			assert(recorder.end_pass());
			const auto commands=recorder.snapshot();
			const auto texture_order=commands.find("DX8Wrapper::Set_Texture stage=0");
			const auto material_order=commands.find("DX8Wrapper::Set_Material",texture_order);
			const auto shader_order=commands.find("DX8Wrapper::Set_Shader",material_order);
			const auto world_order=commands.find("DX8Wrapper::Set_Transform=256",shader_order);
			const auto draw_order=commands.find("draw pipeline=",world_order);
			assert(texture_order!=std::string::npos && material_order!=std::string::npos &&
				shader_order!=std::string::npos && world_order!=std::string::npos &&
				draw_order!=std::string::npos &&
				commands.find("original_applied_1.frag",world_order)!=std::string::npos &&
				commands.find("fragment_textures=T",draw_order)!=std::string::npos &&
				mesh->Peek_Model()->Peek_Single_Texture()!=nullptr &&
				!mesh->Peek_Model()->Peek_Single_Texture()->Is_Missing_Texture());
			RenderObjClass* original_skin_hlod=manager.Create_Render_Obj("TEST.SKINHLOD");
			assert(original_skin_hlod && original_skin_hlod->Get_HTree());
			RenderObjClass* original_skin_child=original_skin_hlod->Get_Sub_Object(0);
			assert(original_skin_child && original_skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
			auto* original_skin_mesh=static_cast<MeshClass*>(original_skin_child);
			original_skin_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			original_skin_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
			original_skin_mesh->Peek_Model()->Register_For_Rendering();
			original_skin_hlod->Set_Position(Vector3(0,0,-10));
			LightEnvironmentClass skin_environment;
			skin_environment.Reset(Vector3(0,0,-10),Vector3(0.1f,0.2f,0.3f));
			skin_environment.Pre_Render_Update(Matrix3D(true));
			render_info.light_environment=&skin_environment;
			original_skin_hlod->Render(render_info);
			const auto skin_failure_start=recorder.snapshot().size();
			recorder.fail_next_buffer_upload();
			assert(recorder.begin_pass(pass,"original HLOD skin rejected upload"));
			bool rejected_skin_upload=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				rejected_skin_upload=std::strstr(error.what(),"upload")!=nullptr;
			}
			assert(rejected_skin_upload && recorder.end_pass());
			assert(recorder.snapshot().substr(skin_failure_start).find("draw pipeline=")==
				std::string::npos);
			const auto skin_draw_failure_start=recorder.snapshot().size();
			recorder.fail_next_draw();
			assert(recorder.begin_pass(pass,"original HLOD skin rejected physical draw"));
			bool rejected_skin_draw=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				rejected_skin_draw=std::strstr(error.what(),"draw")!=nullptr;
			}
			assert(rejected_skin_draw && recorder.end_pass());
			assert(recorder.snapshot().substr(skin_draw_failure_start).find("draw pipeline=")==
				std::string::npos);
			const auto skin_log_start=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original HLOD-owned skin category frame"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto skin_commands=recorder.snapshot().substr(skin_log_start);
			const auto dynamic_order=skin_commands.find("DX8Wrapper::Set_Vertex_Buffer dynamic offset=");
			const auto skin_material_order=skin_commands.find("DX8Wrapper::Set_Material",dynamic_order);
			const auto skin_identity_order=skin_commands.find("DX8Wrapper::Set_Transform=16",skin_material_order);
			const auto skin_draw_order=skin_commands.find("draw pipeline=",skin_material_order);
			assert(dynamic_order!=std::string::npos && skin_material_order!=std::string::npos &&
				skin_identity_order!=std::string::npos && skin_draw_order>skin_identity_order &&
				skin_commands.find("index_bits=16",skin_draw_order)!=std::string::npos &&
				skin_commands.find("original_applied_nd2_lit.vert",dynamic_order)!=std::string::npos);
			auto* immediate=NEW_REF(MaterialPassClass,());
			auto* pass_material=NEW_REF(VertexMaterialClass,());
			pass_material->Set_Lighting(true);
			pass_material->Set_Emissive(Vector3(0.5f,0.1f,0.05f));
			immediate->Set_Material(pass_material);
			pass_material->Release_Ref();
			ShaderClass pass_shader;
			pass_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
			pass_shader.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
			immediate->Set_Shader(pass_shader);
			TextureClass* source_texture=mesh->Peek_Model()->Peek_Single_Texture();
			assert(source_texture);
			immediate->Set_Texture(source_texture,2);
			const auto unsupported_stage_start=recorder.snapshot().size();
			const auto* material_before_stage=DX8Wrapper::Peek_Material();
			bool unsupported_stage_rejected=false;
			try { immediate->Install_Materials(); }
			catch (const std::runtime_error& error) {
				unsupported_stage_rejected=std::strstr(error.what(),"texture stage")!=nullptr;
			}
			assert(unsupported_stage_rejected && DX8Wrapper::Peek_Material()==material_before_stage &&
				recorder.snapshot().size()==unsupported_stage_start);
			immediate->Set_Texture(nullptr,2);
			auto* optional_material=NEW_REF(MaterialPassClass,());
			optional_material->Set_Shader(pass_shader);
			optional_material->Install_Materials();
			DX8Wrapper::Apply_Render_State_Changes();
			assert(DX8Wrapper::Snapshot_Source_State().material_applied);
			optional_material->Release_Ref();
			render_info.Push_Material_Pass(immediate);
			const auto count_draws=[](const std::string& commands) {
				unsigned count=0;
				for (std::size_t pos=commands.find("draw pipeline=");pos!=std::string::npos;
					pos=commands.find("draw pipeline=",pos+1)) ++count;
				return count;
			};
			for (auto* owner : {original_skin_hlod,variant_objects[0]}) {
				owner->Render(render_info);
				const auto before_additional=recorder.snapshot().size();
				assert(recorder.begin_pass(pass,"original source immediate additional material"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
				const auto additional_commands=recorder.snapshot().substr(before_additional);
				const auto base=additional_commands.find("draw pipeline=");
				const auto installed=additional_commands.find("DX8Wrapper::Set_Material",base);
				const auto additional=additional_commands.find("draw pipeline=",installed);
				assert(count_draws(additional_commands)==2 && base!=std::string::npos &&
					installed>base && additional>installed &&
					additional_commands.find("original_applied_0.frag",installed)!=std::string::npos &&
					additional_commands.find("index_bits=16",additional)!=std::string::npos);
			}
			auto* delayed_rigid=static_cast<MeshClass*>(variant_objects[0]);
			render_info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
			delayed_rigid->Render(render_info);
			render_info.Pop_Override_Flags();
			original_skin_hlod->Render(render_info);
			const auto before_delayed=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original skin-before-rigid-delayed source flush"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto delayed_commands=recorder.snapshot().substr(before_delayed);
			const auto skin_dynamic=delayed_commands.find("DX8Wrapper::Set_Vertex_Buffer dynamic offset=");
			const auto delayed_vertex=delayed_commands.find("DX8Wrapper::Set_Vertex_Buffer",skin_dynamic+1);
			const auto delayed_install=delayed_commands.find("DX8Wrapper::Set_Material",delayed_vertex);
			const auto first_skin_draw=delayed_commands.find("draw pipeline=",skin_dynamic);
			const auto second_skin_draw=delayed_commands.find("draw pipeline=",first_skin_draw+1);
			const auto delayed_draw=delayed_commands.find("draw pipeline=",delayed_install);
			assert(count_draws(delayed_commands)==3 && skin_dynamic!=std::string::npos &&
				first_skin_draw>skin_dynamic && second_skin_draw>first_skin_draw &&
				second_skin_draw<delayed_vertex && delayed_vertex<delayed_install &&
				delayed_draw>delayed_install);
			const auto before_empty_delay=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original empty delayed queue"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_empty_delay))==0);
			render_info.Pop_Material_Pass();
			immediate->Release_Ref();
			original_skin_child->Release_Ref();
			original_skin_hlod->Release_Ref();
			render_info.alphaOverride=0.5f;
			mesh->Render(render_info);
			assert(recorder.begin_pass(pass,"caller-owned original alpha-override frame"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto alpha_commands=recorder.snapshot();
			assert(alpha_commands.find("DX8Wrapper::Set_DX8_Render_State=24:48",draw_order)!=
				std::string::npos &&
				alpha_commands.find("draw pipeline=",draw_order+1)!=std::string::npos);
			for (unsigned i=0; i<2; ++i) {
				const auto *variant=i==0 ? "TEST.ZERO01" : "TEST.TWO01";
				auto *variant_mesh=static_cast<MeshClass *>(variant_objects[i]);
				variant_mesh->Render(render_info);
				const auto before=recorder.snapshot().size();
				assert(recorder.begin_pass(pass,variant));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
				const auto issued=recorder.snapshot().substr(before);
				assert(issued.find("DX8Wrapper::Set_Texture stage=0")!=std::string::npos &&
					issued.find("DX8Wrapper::Set_Material")!=std::string::npos &&
					issued.find("DX8Wrapper::Set_Shader")!=std::string::npos &&
					issued.find("draw pipeline=")!=std::string::npos);
				assert(issued.find(i==0 ? "original_applied_0.frag" :
					"original_applied_3.frag")!=std::string::npos);
				if (i==0) assert(issued.find("fragment_textures=")!=std::string::npos &&
					issued.find("fragment_textures=T")==std::string::npos);
				else {
					const auto bindings=issued.substr(issued.find("fragment_textures=T"));
					assert(bindings.find("fragment_textures=T")==0 &&
						bindings.find("/S")!=std::string::npos &&
						bindings.find(",T")!=std::string::npos &&
						bindings.find("/S",bindings.find(",T"))!=std::string::npos);
				}
			}
			auto *override_mesh=static_cast<MeshClass *>(variant_objects[2]);
			auto *mapper=static_cast<LinearOffsetTextureMapperClass *>(
				override_mesh->Peek_Model()->Peek_Single_Material()->Peek_Mapper());
			assert(mapper && mapper->Mapper_ID()==TextureMapperClass::MAPPER_ID_LINEAR_OFFSET);
			Vector2 original_offset;
			mapper->Get_Current_UV_Offset(original_offset);
			RenderObjClass::Material_Override material_override;
			material_override.customUVOffset=Vector2(0.25f,0.5f);
			override_mesh->Set_User_Data(&material_override);
			override_mesh->Set_Additive(true);
			override_mesh->Set_ObjectScale(1.25f);
			override_mesh->Render(render_info);
			const auto before_override=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original additive/material/UV override"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto override_commands=recorder.snapshot().substr(before_override);
			assert(override_commands.find("DX8Wrapper::Set_DX8_Render_State=143:1")!=std::string::npos &&
				override_commands.find("DX8Wrapper::Set_DX8_Render_State=143:0")!=std::string::npos &&
				override_commands.find("DX8Wrapper::Set_Transform=16")!=std::string::npos &&
				override_commands.find("draw pipeline=")!=std::string::npos);
			Matrix4x4 override_uv;
			DX8Wrapper::Get_Transform(D3DTS_TEXTURE0,override_uv);
			assert(std::fabs(override_uv[0].Z-0.25f)<0.0001f &&
				std::fabs(override_uv[1].Z-0.5f)<0.0001f);
			Vector2 restored_offset;
			mapper->Get_Current_UV_Offset(restored_offset);
			assert(restored_offset.X==original_offset.X && restored_offset.Y==original_offset.Y);
			override_mesh->Set_User_Data(nullptr);
			auto *retry_mesh=static_cast<MeshClass *>(variant_objects[0]);
			retry_mesh->Render(render_info);
			recorder.fail_next_buffer_upload();
			const auto before_failure=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original category injected upload failure"));
			bool upload_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error &error) {
				upload_rejected=std::strstr(error.what(),"upload")!=nullptr;
			}
			assert(upload_rejected && recorder.end_pass());
			assert(recorder.snapshot().find("draw pipeline=",before_failure)==std::string::npos);
			retry_mesh->Render(render_info);
			const auto before_retry=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original category retry"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			assert(recorder.snapshot().find("draw pipeline=",before_retry)!=std::string::npos);
			camera.Set_Position(Vector3(0,0,10));
			retry_mesh->Set_Transform(Matrix3D(true));
			retry_mesh->Render(render_info);
			const auto before_identity=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original rigid identity world"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto identity_commands=recorder.snapshot().substr(before_identity);
			assert(identity_commands.find("DX8Wrapper::Set_World_Identity")!=std::string::npos &&
				identity_commands.find("draw pipeline=")!=std::string::npos);
			camera.Set_Position(Vector3(3,1,10));
			retry_mesh->Set_Position(Vector3(0,0,-10));
			for (auto flag : {MeshGeometryClass::ALIGNED,MeshGeometryClass::ORIENTED}) {
				retry_mesh->Peek_Model()->Set_Flag(flag,true);
				retry_mesh->Render(render_info);
				const auto before_facing=recorder.snapshot().size();
				assert(recorder.begin_pass(pass,"original camera-facing category"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
				const auto facing_commands=recorder.snapshot().substr(before_facing);
				assert(facing_commands.find("DX8Wrapper::Set_Transform=256")!=std::string::npos &&
					facing_commands.find("draw pipeline=")!=std::string::npos);
				retry_mesh->Peek_Model()->Set_Flag(flag,false);
			}
			LightEnvironmentClass selected_environment;
			selected_environment.Reset(Vector3(0,0,-10),Vector3(0.12f,0.24f,0.36f));
			selected_environment.Pre_Render_Update(Matrix3D(true));
			render_info.light_environment=&selected_environment;
			retry_mesh->Render(render_info);
			const auto before_source_lights=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original category-selected source light environment"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto light_commands=recorder.snapshot().substr(before_source_lights);
			const auto light_selection=light_commands.find("DX8Wrapper::Set_Light_Environment");
			const auto light_world=light_commands.find("DX8Wrapper::Set_Transform=256",light_selection);
			assert(light_selection!=std::string::npos && light_world!=std::string::npos &&
				light_commands.find("draw pipeline=",light_world)!=std::string::npos);
			retry_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
			retry_mesh->Render(render_info);
			const auto before_category_lit=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original lit rigid mesh/category source frame"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto lit_commands=recorder.snapshot().substr(before_category_lit);
			const auto original_light=lit_commands.find("DX8Wrapper::Set_Light_Environment");
			const auto original_world=lit_commands.find("DX8Wrapper::Set_Transform=256",original_light);
			const auto lit_pipeline=lit_commands.find("original_applied_n0_lit.vert",original_world);
			assert(original_light!=std::string::npos && original_world!=std::string::npos &&
				lit_pipeline!=std::string::npos &&
				lit_commands.find("draw pipeline=",lit_pipeline)!=std::string::npos);
			for (unsigned family=1;family<=2;++family) {
				auto* lit_mesh=static_cast<MeshClass*>(variant_objects[family+2]);
				lit_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(true);
				lit_mesh->Render(render_info);
				const auto before_family=recorder.snapshot().size();
				assert(recorder.begin_pass(pass,family==1?
					"original lit N1 single-stage category":"original lit N2 two-stage category"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
				const auto source_family=recorder.snapshot().substr(before_family);
				const auto light=source_family.find("DX8Wrapper::Set_Light_Environment");
				const auto world=source_family.find("DX8Wrapper::Set_Transform=256",light);
				const auto variant=source_family.find(family==1 ?
					"original_applied_n1_lit.vert":"original_applied_n2_lit.vert",world);
				assert(light!=std::string::npos && world!=std::string::npos &&
					variant!=std::string::npos &&
					source_family.find(family==1 ? "original_applied_1.frag" :
						"original_applied_3.frag",variant)!=std::string::npos &&
					source_family.find("draw pipeline=",variant)!=std::string::npos);
				lit_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
			}
			Matrix3D source_direction(true);
			source_direction.Rotate_X(WWMATH_PI);
			LightClass lit_directional(LightClass::DIRECTIONAL);
			lit_directional.Set_Transform(source_direction);
			lit_directional.Set_Diffuse(Vector3(0.3f,0.2f,0.1f));
			LightClass lit_point(LightClass::POINT);
			lit_point.Set_Position(Vector3(0,0,-9));
			lit_point.Set_Diffuse(Vector3(0.4f,0.3f,0.2f));
			lit_point.Set_Near_Attenuation_Range(0,2);
			lit_point.Set_Far_Attenuation_Range(2,10);
			for (unsigned family=0;family<3;++family) {
				selected_environment.Reset(Vector3(0,0,-10),Vector3(0.06f,0.12f,0.18f));
				if (family==0) selected_environment.Add_Light(lit_directional);
				else if (family==1) selected_environment.Add_Light(lit_point);
				else for (unsigned i=0;i<4;++i) selected_environment.Add_Light(lit_directional);
				selected_environment.Pre_Render_Update(Matrix3D(true));
				retry_mesh->Render(render_info);
				const auto before_lights=recorder.snapshot().size();
				assert(recorder.begin_pass(pass,"original selected dynamic lit category frame"));
				TheDX8MeshRenderer.Flush();
				assert(recorder.end_pass());
				const auto light_frame=recorder.snapshot().substr(before_lights);
				const auto issued_light=light_frame.find("DX8Wrapper::Set_Light_Environment");
				const auto issued_world=light_frame.find("DX8Wrapper::Set_Transform=256",issued_light);
				const auto issued_lit=light_frame.find("original_applied_n0_lit.vert",issued_world);
				assert(issued_light!=std::string::npos && issued_world!=std::string::npos &&
					issued_lit!=std::string::npos &&
					light_frame.find("draw pipeline=",issued_lit)!=std::string::npos &&
					light_frame.find("DX8Wrapper::Set_Light slot=0 enabled",issued_light)!=
						std::string::npos &&
					(family!=2 || (selected_environment.Get_Light_Count()==4 &&
						light_frame.find("DX8Wrapper::Set_Light slot=3 enabled",issued_light)!=
							std::string::npos)));
			}
			retry_mesh->Render(render_info);
			recorder.fail_next_buffer_upload();
			const auto before_lit_failure=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original lit category injected upload failure"));
			bool lit_upload_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				lit_upload_rejected=std::strstr(error.what(),"upload")!=nullptr;
			}
			assert(lit_upload_rejected && recorder.end_pass() &&
				recorder.snapshot().find("draw pipeline=",before_lit_failure)==std::string::npos);
			retry_mesh->Render(render_info);
			const auto before_lit_retry=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original lit category original requeue"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto lit_retry=recorder.snapshot().substr(before_lit_retry);
			const auto retry_light=lit_retry.find("DX8Wrapper::Set_Light_Environment");
			const auto retry_world=lit_retry.find("DX8Wrapper::Set_Transform=256",retry_light);
			assert(retry_light!=std::string::npos && retry_world!=std::string::npos &&
				lit_retry.find("original_applied_n0_lit.vert",retry_world)!=std::string::npos &&
				lit_retry.find("draw pipeline=",retry_world)!=std::string::npos);
			Vector3 invalid_ambient(std::numeric_limits<float>::quiet_NaN(),0,0);
			selected_environment.Set_Output_Ambient(invalid_ambient);
			retry_mesh->Render(render_info);
			const auto before_invalid=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original lit category invalid source ambient"));
			bool invalid_light_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				invalid_light_rejected=std::strstr(error.what(),"source ambient")!=nullptr;
			}
			assert(invalid_light_rejected && recorder.end_pass() &&
				recorder.snapshot().find("draw pipeline=",before_invalid)==std::string::npos);
			selected_environment.Reset(Vector3(0,0,-10),Vector3(0.12f,0.24f,0.36f));
			selected_environment.Pre_Render_Update(Matrix3D(true));
			retry_mesh->Render(render_info);
			const auto before_valid=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original lit category valid source replay"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass() &&
				recorder.snapshot().find("draw pipeline=",before_valid)!=std::string::npos);
			auto* override_pass=NEW_REF(MaterialPassClass,());
			auto* shared_material=NEW_REF(VertexMaterialClass,());
			shared_material->Set_Lighting(true);
			shared_material->Set_Opacity(0.75f);
			shared_material->Set_Emissive(Vector3(0.3f,0.2f,0.1f));
			override_pass->Set_Material(shared_material);
			shared_material->Release_Ref();
			ShaderClass override_shader;
			override_shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
			override_pass->Set_Shader(override_shader);
			render_info.Push_Material_Pass(override_pass);
			render_info.materialPassAlphaOverride=0.4f;
			render_info.materialPassEmissiveOverride=0.5f;
			retry_mesh->Render(render_info);
			recorder.fail_draw_after(1);
			const auto before_pass_failure=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original additional material draw failure"));
			bool additional_draw_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				additional_draw_rejected=std::strstr(error.what(),"draw")!=nullptr;
			}
			assert(additional_draw_rejected && recorder.end_pass());
			const auto failed_pass=recorder.snapshot().substr(before_pass_failure);
			assert(failed_pass.find("draw pipeline=")!=std::string::npos &&
				failed_pass.find("draw pipeline=",failed_pass.find("draw pipeline=")+1)==
					std::string::npos &&
				std::fabs(shared_material->Get_Opacity()-0.75f)<0.0001f);
			Vector3 restored_emissive;
			shared_material->Get_Emissive(&restored_emissive);
			assert(std::fabs(restored_emissive.X-0.3f)<0.0001f &&
				std::fabs(restored_emissive.Y-0.2f)<0.0001f &&
				std::fabs(restored_emissive.Z-0.1f)<0.0001f);
			// A partially emitted frame is aborted; requeue through the original
			// model/category owner rather than replaying an adapter-owned command.
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			retry_mesh->Peek_Model()->Register_For_Rendering();
			tree_mesh->Peek_Model()->Register_For_Rendering();
			retry_mesh->Render(render_info);
			const auto before_pass_retry=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original additional material requeue"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto retried_pass=recorder.snapshot().substr(before_pass_retry);
			assert(retried_pass.find("draw pipeline=")!=std::string::npos &&
				retried_pass.find("draw pipeline=",retried_pass.find("draw pipeline=")+1)!=
					std::string::npos &&
				std::fabs(shared_material->Get_Opacity()-0.75f)<0.0001f);
			render_info.materialPassAlphaOverride=1.0f;
			render_info.materialPassEmissiveOverride=1.0f;
			render_info.Pop_Material_Pass();
			override_pass->Release_Ref();
			// The original override branches intentionally skip absent material.
			// The default material remains selected by the original wrapper.
			auto* default_pass=NEW_REF(MaterialPassClass,());
			default_pass->Set_Shader(override_shader);
			render_info.Push_Material_Pass(default_pass);
			render_info.materialPassAlphaOverride=0.4f;
			render_info.materialPassEmissiveOverride=0.5f;
			retry_mesh->Render(render_info);
			const auto before_default=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original null material override guard"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto default_commands=recorder.snapshot().substr(before_default);
			assert(default_commands.find("DX8Wrapper::Set_Material")!=std::string::npos &&
				default_commands.find("draw pipeline=")!=std::string::npos &&
				default_commands.find("draw pipeline=",default_commands.find("draw pipeline=")+1)!=
					std::string::npos);
			render_info.materialPassAlphaOverride=1.0f;
			render_info.materialPassEmissiveOverride=1.0f;
			render_info.Pop_Material_Pass();
			default_pass->Release_Ref();
			auto* culled_pass=NEW_REF(MaterialPassClass,());
			OBBoxClass source_cull_volume(Vector3(0,0,-10),Vector3(100,100,100),Matrix3x3(true));
			culled_pass->Set_Cull_Volume(&source_cull_volume);
			const bool old_per_polygon_culling=MaterialPassClass::Is_Per_Polygon_Culling_Enabled();
			MaterialPassClass::Enable_Per_Polygon_Culling(true);
			render_info.Push_Material_Pass(culled_pass);
			retry_mesh->Render(render_info);
			const auto before_cull_edge=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original cull-volume APT dynamic physical draw"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto cull_commands=recorder.snapshot().substr(before_cull_edge);
			assert(count_draws(cull_commands)==2 &&
				cull_commands.find("DX8Wrapper::Set_Index_Buffer dynamic offset=")!=std::string::npos);
			assert(retry_mesh->Peek_Model()->Get_Polygon_Count()==1);
			const auto dynamic_index_bytes=recorder.last_draw_index_bytes();
			assert(dynamic_index_bytes.size()==3*sizeof(unsigned short));
			unsigned short dynamic_indices[3]{};
			std::memcpy(dynamic_indices,dynamic_index_bytes.data(),sizeof(dynamic_indices));
			const auto selected_poly=retry_mesh->Peek_Model()->Get_Polygon_Array()[0];
			assert(dynamic_indices[0]==selected_poly.I && dynamic_indices[1]==selected_poly.J &&
				dynamic_indices[2]==selected_poly.K &&
				cull_commands.find("index_bits=16")!=std::string::npos);
			OBBoxClass outside_volume(Vector3(1000,1000,-10),Vector3(1,1,1),Matrix3x3(true));
			culled_pass->Set_Cull_Volume(&outside_volume);
			retry_mesh->Render(render_info);
			const auto before_empty_apt=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original cull-volume empty APT"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto empty_apt_commands=recorder.snapshot().substr(before_empty_apt);
			// This fixture has no original cull tree: the authored fallback uses
			// view-facing APT, so moving the volume alone does not reject a poly.
			assert(!retry_mesh->Peek_Model()->Has_Cull_Tree() &&
				count_draws(empty_apt_commands)==2 &&
				empty_apt_commands.find("DX8Wrapper::Set_Index_Buffer dynamic offset=")!=
					std::string::npos);
			Matrix3x3 flipped_basis(true);
			flipped_basis.Rotate_X(WWMATH_PI);
			OBBoxClass backface_volume(Vector3(0,0,-10),Vector3(100,100,100),flipped_basis);
			culled_pass->Set_Cull_Volume(&backface_volume);
			retry_mesh->Render(render_info);
			const auto before_backface_apt=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original no-tree backface empty APT"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto backface_commands=recorder.snapshot().substr(before_backface_apt);
			assert(count_draws(backface_commands)==1 &&
				backface_commands.find("DX8Wrapper::Set_Index_Buffer dynamic offset=")==
					std::string::npos);
			culled_pass->Set_Cull_Volume(&source_cull_volume);
			tree_mesh->Render(render_info);
			const auto before_tree_apt=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original loader-built cull-tree APT"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto tree_apt_commands=recorder.snapshot().substr(before_tree_apt);
			assert(count_draws(tree_apt_commands)==2 &&
				tree_apt_commands.find("DX8Wrapper::Set_Index_Buffer dynamic offset=")!=
					std::string::npos);
			culled_pass->Set_Cull_Volume(&outside_volume);
			tree_mesh->Render(render_info);
			const auto before_tree_outside=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original cull-tree outside volume"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			const auto tree_outside_commands=recorder.snapshot().substr(before_tree_outside);
			assert(count_draws(tree_outside_commands)==1 &&
				tree_outside_commands.find("DX8Wrapper::Set_Index_Buffer dynamic offset=")==
					std::string::npos);
			culled_pass->Set_Cull_Volume(&source_cull_volume);
			auto* source_polygons=const_cast<TriIndex*>(retry_mesh->Peek_Model()->Get_Polygon_Array());
			const auto authored_polygon=source_polygons[0];
			source_polygons[0].I=retry_mesh->Peek_Model()->Get_Vertex_Count();
			retry_mesh->Render(render_info);
			const auto before_bad_polygon=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original malformed cull polygon rejection"));
			bool bad_polygon_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				bad_polygon_rejected=std::strstr(error.what(),"polygon index")!=nullptr;
			}
			assert(bad_polygon_rejected && recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_bad_polygon))==1);
			source_polygons[0]=authored_polygon;
			render_info.Pop_Material_Pass();
			MaterialPassClass::Enable_Per_Polygon_Culling(old_per_polygon_culling);
			culled_pass->Release_Ref();
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			const bool authored_sorting=WW3D::Is_Sorting_Enabled();
			WW3D::Enable_Sorting(true);
			retry_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,true);
			auto* sorted_cull_pass=NEW_REF(MaterialPassClass,());
			sorted_cull_pass->Set_Cull_Volume(&source_cull_volume);
			render_info.Push_Material_Pass(sorted_cull_pass);
			retry_mesh->Peek_Model()->Register_For_Rendering();
			render_info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
			retry_mesh->Render(render_info);
			render_info.Pop_Override_Flags();
			const auto before_sorting_cull=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original sorted cull-volume typed route"));
			bool sorting_cull_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				sorting_cull_rejected=std::strstr(error.what(),"06C sorting renderer")!=nullptr;
			}
			assert(sorting_cull_rejected && recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_sorting_cull))==0);
			render_info.Pop_Material_Pass();
			sorted_cull_pass->Release_Ref();
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			retry_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			WW3D::Enable_Sorting(authored_sorting);
			retry_mesh->Peek_Model()->Register_For_Rendering();
			retry_mesh->Render(render_info);
			assert(recorder.begin_pass(pass,"original cull owner warm base frame"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			auto* retry_cull_pass=NEW_REF(MaterialPassClass,());
			retry_cull_pass->Set_Cull_Volume(&source_cull_volume);
			render_info.Push_Material_Pass(retry_cull_pass);
			retry_mesh->Render(render_info);
			// Arm after the original base draw's uploads, then reject the
			// APT-owned physical index upload.
			const auto first_cull_draw=cull_commands.find("draw pipeline=");
			unsigned base_uploads=0;
			for (auto pos=cull_commands.find("upload B");pos!=std::string::npos &&
				pos<first_cull_draw;pos=cull_commands.find("upload B",pos+1)) ++base_uploads;
			assert(base_uploads>0);
			recorder.fail_buffer_upload_after(base_uploads);
			const auto before_cull_upload=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original dynamic APT index upload failure"));
			bool cull_upload_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				cull_upload_rejected=std::strstr(error.what(),"upload")!=nullptr;
			}
			assert(cull_upload_rejected && recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_cull_upload))==1);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			retry_mesh->Peek_Model()->Register_For_Rendering();
			retry_mesh->Render(render_info);
			const auto before_cull_retry=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original APT owner upload retry"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_cull_retry))==2);
			retry_mesh->Render(render_info);
			recorder.fail_draw_after(1);
			const auto before_cull_draw_failure=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original APT dynamic draw failure"));
			bool cull_draw_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				cull_draw_rejected=std::strstr(error.what(),"draw")!=nullptr;
			}
			assert(cull_draw_rejected && recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_cull_draw_failure))==1);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			retry_mesh->Peek_Model()->Register_For_Rendering();
			retry_mesh->Render(render_info);
			const auto before_cull_draw_retry=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original APT owner draw retry"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass() &&
				count_draws(recorder.snapshot().substr(before_cull_draw_retry))==2);
			render_info.Pop_Material_Pass();
			retry_cull_pass->Release_Ref();
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			auto* delayed_pass=NEW_REF(MaterialPassClass,());
			auto* delayed_material=NEW_REF(VertexMaterialClass,());
			delayed_material->Set_Lighting(true);
			delayed_material->Set_Opacity(0.7f);
			delayed_pass->Set_Material(delayed_material);
			delayed_material->Release_Ref();
			delayed_pass->Set_Shader(override_shader);
			render_info.Push_Material_Pass(delayed_pass);
			const auto queue_delayed=[&]() {
				retry_mesh->Peek_Model()->Register_For_Rendering();
				render_info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
				retry_mesh->Render(render_info);
				render_info.Pop_Override_Flags();
			};
			queue_delayed();
			recorder.fail_next_buffer_upload();
			const auto before_delayed_upload=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original delayed upload failure"));
			bool delayed_upload_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				delayed_upload_rejected=std::strstr(error.what(),"upload")!=nullptr;
			}
			assert(delayed_upload_rejected && recorder.end_pass() &&
				recorder.snapshot().substr(before_delayed_upload).find("draw pipeline=")==
					std::string::npos);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			queue_delayed();
			recorder.fail_next_draw();
			const auto before_delayed_failure=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original delayed physical draw failure"));
			bool delayed_draw_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				delayed_draw_rejected=std::strstr(error.what(),"draw")!=nullptr;
			}
			assert(delayed_draw_rejected && recorder.end_pass() &&
				recorder.snapshot().substr(before_delayed_failure).find("draw pipeline=")==
					std::string::npos &&
				std::fabs(delayed_material->Get_Opacity()-0.7f)<0.0001f);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			queue_delayed();
			const auto before_delayed_retry=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original delayed owner reset and retry"));
			TheDX8MeshRenderer.Flush();
			assert(recorder.end_pass());
			assert(count_draws(recorder.snapshot().substr(before_delayed_retry))==1);
			render_info.Pop_Material_Pass();
			delayed_pass->Release_Ref();
			render_info.light_environment=nullptr;
			retry_mesh->Peek_Model()->Peek_Single_Material()->Set_Lighting(false);
			RenderObjClass* bad_skin_hlod=manager.Create_Render_Obj("TEST.BADHLOD");
			assert(bad_skin_hlod && bad_skin_hlod->Get_HTree());
			RenderObjClass* bad_skin_child=bad_skin_hlod->Get_Sub_Object(0);
			assert(bad_skin_child && bad_skin_child->Class_ID()==RenderObjClass::CLASSID_MESH);
			auto* bad_skin_mesh=static_cast<MeshClass*>(bad_skin_child);
			bad_skin_mesh->Peek_Model()->Set_Flag(MeshGeometryClass::SORT,false);
			bad_skin_mesh->Peek_Model()->Register_For_Rendering();
			bad_skin_hlod->Set_Position(Vector3(0,0,-10));
			bad_skin_hlod->Render(render_info);
			const auto bad_skin_start=recorder.snapshot().size();
			assert(recorder.begin_pass(pass,"original invalid HLOD bone link rejection"));
			bool bad_skin_rejected=false;
			try { TheDX8MeshRenderer.Flush(); }
			catch (const std::runtime_error& error) {
				bad_skin_rejected=std::strstr(error.what(),"bone index")!=nullptr;
			}
			assert(bad_skin_rejected && recorder.end_pass());
			assert(recorder.snapshot().substr(bad_skin_start).find("draw pipeline=")==
				std::string::npos);
			assert(textures.owners==0);
			TheDX8MeshRenderer.Invalidate();
			TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
			bad_skin_child->Release_Ref();
			bad_skin_hlod->Release_Ref();
			tree_mesh->Release_Ref();
			for (auto *variant_object : variant_objects) variant_object->Release_Ref();
		}
		recorder.destroy(color); recorder.destroy(depth);
		assert(recorder.resource_counts().total() == 0);
		WW3D::Reset_Current_Static_Sort_Lists_To_Default();
		_TheFileFactory=old_factory;
		object->Release_Ref();
		manager.Free_Assets();
		std::puts("original-rendering runtime provider=GeneralsMD WW3D2 first GPU edge");
		return 0;
	}
	mesh->Set_Hidden(1);
	mesh->Render(render_info); // Original hidden state suppresses the device boundary.
	assert(!mesh->Peek_Model()->Has_Polygon_Renderers());
	mesh->Set_Hidden(0);
	MeshModelClass *model = mesh->Peek_Model();
	TheDX8MeshRenderer.Init();
	mesh->Render(render_info); // Original frustum rejects this default-camera rigid fixture.
	assert(!model->Has_Polygon_Renderers());
	model->Register_For_Rendering();
	assert(model->Has_Polygon_Renderers());
	assert(DX8FVFCategoryContainer::Define_FVF(model, true) == DX8_FVF_XYZNDUV2);
	assert(VertexBufferClass::Get_Total_Buffer_Count() > 0);
	assert(IndexBufferClass::Get_Total_Buffer_Count() > 0);
	// Authored sorting and rigid registrations use different original category
	// decisions; the second route must allocate the original DX8-kind CPU bytes.
	TheDX8MeshRenderer.Unregister_Mesh_Type(model);
	model->Set_Flag(MeshGeometryClass::SORT, false);
	model->Register_For_Rendering();
	assert(model->Has_Polygon_Renderers());
	assert(DX8FVFCategoryContainer::Define_FVF(model, true) == DX8_FVF_XYZN);
	assert(VertexBufferClass::Get_Total_Buffer_Count() > 1);
	assert(manager.Render_Obj_Exists("TEST.SKIN01"));
	RenderObjClass *skin_object = manager.Create_Render_Obj("TEST.SKIN01");
	assert(skin_object != nullptr && skin_object->Class_ID() == RenderObjClass::CLASSID_MESH);
	auto *skin_model = static_cast<MeshClass *>(skin_object)->Peek_Model();
	assert(skin_model->Get_Flag(MeshGeometryClass::SKIN));
	std::array<Vector3,3> orphan_positions{},orphan_normals{};
	bool missing_skin_hierarchy=false;
	try { static_cast<MeshClass*>(skin_object)->Get_Deformed_Vertices(
		orphan_positions.data(),orphan_normals.data()); }
	catch (const std::runtime_error& error) {
		missing_skin_hierarchy=std::strstr(error.what(),"HLOD hierarchy")!=nullptr;
	}
	assert(missing_skin_hierarchy);
	static_cast<MeshClass *>(skin_object)->Render(render_info); // Skins bypass rigid frustum rejection.
	assert(skin_model->Has_Polygon_Renderers());
	skin_model->Register_For_Rendering();
	assert(skin_model->Has_Polygon_Renderers());
	DefaultStaticSortListClass sort_list;
	WW3D::Override_Current_Static_Sort_Lists(&sort_list);
	WW3D::Enable_Static_Sort_Lists(true);
	TheDX8MeshRenderer.Set_Camera(&camera);
	skin_model->Set_Sort_Level(1);
	static_cast<MeshClass *>(skin_object)->Render(render_info); // Source defers this sorted instance.
	bool sorted_physical_edge = false;
	try { WW3D::Render_And_Clear_Static_Sort_Lists(render_info); }
	catch (const std::runtime_error &) { sorted_physical_edge = true; }
	assert(sorted_physical_edge); // Original list re-enters mesh then reaches physical flush.
	WW3D::Enable_Static_Sort_Lists(false);
	WW3D::Reset_Current_Static_Sort_Lists_To_Default();
	skin_model->Set_Sort_Level(0);
	TheDX8MeshRenderer.Invalidate();
	TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
	// The source static-sort retry reached and retained its recyclable dynamic
	// sorting VB. Retire that source pool before asserting full test teardown.
	DynamicVBAccessClass::_Deinit();
	assert(VertexBufferClass::Get_Total_Buffer_Count() == 0);
	assert(IndexBufferClass::Get_Total_Buffer_Count() == 0);
	assert(!model->Has_Polygon_Renderers() && !skin_model->Has_Polygon_Renderers());
	model->Register_For_Rendering();
	skin_model->Register_For_Rendering();
	assert(model->Has_Polygon_Renderers() && skin_model->Has_Polygon_Renderers());
	render_info.alphaOverride = 0.25f;
	render_info.Push_Override_Flags(RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY);
	static_cast<MeshClass *>(skin_object)->Render(render_info);
	render_info.Pop_Override_Flags();
	assert(static_cast<MeshClass *>(skin_object)->Get_Alpha_Override() == 0.25f);
	TheDX8MeshRenderer.Set_Camera(&camera);
	TheDX8MeshRenderer.Flush(); // No base or additional pass: no physical submission.
	assert(skin_model->Get_Single_Shader().Get_Src_Blend_Func() == ShaderClass::SRCBLEND_SRC_ALPHA);
	render_info.Push_Override_Flags(static_cast<RenderInfoClass::RINFO_OVERRIDE_FLAGS>(
		RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY |
		RenderInfoClass::RINFO_OVERRIDE_SHADOW_RENDERING));
	static_cast<MeshClass *>(skin_object)->Render(render_info);
	render_info.Pop_Override_Flags();
	bool shadow_alpha_edge = false;
	try { TheDX8MeshRenderer.Flush(); }
	catch (const std::runtime_error &) { shadow_alpha_edge = true; }
	assert(shadow_alpha_edge); // Original alpha-shadow exception restores base passes.
	static_cast<MeshClass *>(skin_object)->Render(render_info);
	TheDX8MeshRenderer.Set_Camera(nullptr);
	TheDX8MeshRenderer.Flush(); // Original no-camera branch cannot submit a pass.
	TheDX8MeshRenderer.Set_Camera(&camera);
	bool gpu_edge_rejected = false;
	try { WW3D::Flush(render_info); }
	catch (const std::runtime_error &) { gpu_edge_rejected = true; }
	assert(gpu_edge_rejected);
	skin_object->Release_Ref();
	FVFInfoClass original_layout(DX8_FVF_XYZNUV2);
	assert(original_layout.Get_FVF_Size() == 40);
	assert(original_layout.Get_Location_Offset() == 0);
	assert(original_layout.Get_Normal_Offset() == 12);
	assert(original_layout.Get_Tex_Offset(0) == 24);
	assert(original_layout.Get_Tex_Offset(1) == 32);
	FVFInfoClass multi_stage_layout(DX8_FVF_XYZNDUV1TG3);
	assert(multi_stage_layout.Get_FVF_Size() == 72);
	bool invalid_layout_rejected = false;
	try { FVFInfoClass invalid(0x002 | 0x900); (void)invalid; }
	catch (const std::runtime_error &) { invalid_layout_rejected = true; }
	assert(invalid_layout_rejected);
	const unsigned buffers_before = VertexBufferClass::Get_Total_Buffer_Count();
	const unsigned index_buffers_before = IndexBufferClass::Get_Total_Buffer_Count();
	auto *vertex_buffer = NEW_REF(DX8VertexBufferClass, (DX8_FVF_XYZNUV2, 4));
	auto *index_buffer = NEW_REF(DX8IndexBufferClass, (6));
	assert(VertexBufferClass::Get_Total_Buffer_Count() == buffers_before + 1);
	assert(IndexBufferClass::Get_Total_Buffer_Count() == index_buffers_before + 1);
	{
		VertexBufferClass::AppendLockClass lock(vertex_buffer, 2, 2);
		auto *vertices = static_cast<unsigned char *>(lock.Get_Vertex_Array());
		assert(vertices == vertex_buffer->Get_CPU_Vertex_Buffer() + 80);
		vertices[0] = 17;
		IndexBufferClass::AppendLockClass indices(index_buffer, 3, 3);
		indices.Get_Index_Array()[0] = 2;
	}
	assert(vertex_buffer->Get_CPU_Vertex_Buffer()[80] == 17);
	assert(index_buffer->Get_CPU_Index_Buffer()[3] == 2);
	vertex_buffer->Add_Engine_Ref();
	bool stale_lock_rejected = false;
	try { VertexBufferClass::WriteLockClass invalid(vertex_buffer); }
	catch (const std::runtime_error &) { stale_lock_rejected = true; }
	assert(stale_lock_rejected);
	vertex_buffer->Release_Engine_Ref();
	bool bad_append_rejected = false;
	try { VertexBufferClass::AppendLockClass invalid(vertex_buffer, 3, 2); }
	catch (const std::runtime_error &) { bad_append_rejected = true; }
	assert(bad_append_rejected);
	bool unsupported_usage_rejected = false;
	try { auto *invalid = NEW_REF(DX8VertexBufferClass,
		(DX8_FVF_XYZ, 3, DX8VertexBufferClass::USAGE_NPATCHES));
		invalid->Release_Ref(); }
	catch (const std::runtime_error &) { unsupported_usage_rejected = true; }
	assert(unsupported_usage_rejected);
	unsupported_usage_rejected = false;
	try { auto *invalid = NEW_REF(DX8IndexBufferClass,
		(3, DX8IndexBufferClass::USAGE_NPATCHES));
		invalid->Release_Ref(); }
	catch (const std::runtime_error &) { unsupported_usage_rejected = true; }
	assert(unsupported_usage_rejected);
	bad_append_rejected = false;
	try { IndexBufferClass::AppendLockClass invalid(index_buffer, 5, 2); }
	catch (const std::runtime_error &) { bad_append_rejected = true; }
	assert(bad_append_rejected);
	vertex_buffer->Release_Ref();
	index_buffer->Release_Ref();
	assert(VertexBufferClass::Get_Total_Buffer_Count() == buffers_before);
	assert(IndexBufferClass::Get_Total_Buffer_Count() == index_buffers_before);
	invalid_layout_rejected = false;
	try { FVFInfoClass invalid(0x100); (void)invalid; }
	catch (const std::runtime_error &) { invalid_layout_rejected = true; }
	assert(invalid_layout_rejected);
	assert(model->Get_Vertex_Count() == 3 && model->Get_Polygon_Count() == 1);
	assert(std::fabs(model->Get_Vertex_Array()[1].X - 1.0f) < 0.0001f);
	assert(model->Get_Polygon_Array()[0].K == 2);
	assert(model->Get_Pass_Count() == 1);
	assert(model->Get_Shader(0).Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE);
	VertexMaterialClass *stage_material = model->Peek_Single_Material();
	assert(stage_material != nullptr);
	assert(std::fabs(stage_material->Get_Opacity() - 0.75f) < 0.0001f);
	assert(stage_material->Get_Mapper(0) != nullptr);
	{
		zh::renderer::RecordingGpuDevice device;
		zh::original_runtime::OriginalGpuEdge edge(device);
		assert(stage_material->Num_Refs() >= 1);
		const int initial_material_refs=stage_material->Num_Refs();
		DX8Wrapper::Set_Material(stage_material);
		assert(stage_material->Num_Refs()==initial_material_refs+1);
		assert(DX8Wrapper::Peek_Material()==stage_material);
		assert(DX8Wrapper::Pending_Changes() & (1U<<8));
		bool missing_camera_source=false;
		try { DX8Wrapper::Apply_Render_State_Changes(); }
		catch (const std::runtime_error&) { missing_camera_source=true; }
		assert(missing_camera_source && (DX8Wrapper::Pending_Changes() & (1U<<8)));
		Matrix4x4 source_projection;
		camera.Get_D3D_Projection_Matrix(&source_projection);
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION,source_projection);
		DX8Wrapper::Apply_Render_State_Changes();
		Matrix4x4 original_uv;
		DX8Wrapper::Get_Transform(D3DTS_TEXTURE0,original_uv);
		const std::string source_commands=device.snapshot();
		assert(source_commands.find("DX8Wrapper::Set_DX8_Material")!=std::string::npos);
		assert(source_commands.find("DX8Wrapper::Set_Transform=16")!=std::string::npos);
		assert(source_commands.find("DX8Wrapper::Set_DX8_Texture_Stage_State=0:11")!=std::string::npos);
		DX8Wrapper::Set_Material(nullptr);
		DX8Wrapper::Apply_Render_State_Changes();
		assert(stage_material->Num_Refs()==initial_material_refs);
		assert(DX8Wrapper::Peek_Material()==nullptr);
		WW3D::Sync(100);
		auto* linear_material=NEW_REF(VertexMaterialClass,());
		linear_material->Set_Lighting(false);
		linear_material->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
		linear_material->Set_UV_Source(1,1);
		auto* linear_mapper=NEW_REF(LinearOffsetTextureMapperClass,
			(Vector2(1.0f,2.0f),Vector2(0.0f,0.0f),false,Vector2(1.0f,1.0f),0));
		linear_material->Set_Mapper(linear_mapper);
		linear_mapper->Release_Ref();
		assert(linear_material->Num_Refs()==1);
		DX8Wrapper::Set_Material(linear_material);
		assert(linear_material->Num_Refs()==2);
		WW3D::Sync(350);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Get_Transform(D3DTS_TEXTURE0,original_uv);
		assert(std::fabs(original_uv[0].Z-0.75f)<0.0001f);
		assert(std::fabs(original_uv[1].Z-0.5f)<0.0001f);
		const std::string linear_commands=device.snapshot();
		assert(linear_commands.find("DX8Wrapper::Set_DX8_Render_State=137:0")!=std::string::npos);
		assert(linear_commands.find("DX8Wrapper::Set_DX8_Render_State=145:1")!=std::string::npos);
		assert(linear_commands.find("DX8Wrapper::Set_DX8_Texture_Stage_State=1:11:1")!=std::string::npos);
		DX8Wrapper::Set_Material(nullptr);
		DX8Wrapper::Apply_Render_State_Changes();
		assert(linear_material->Num_Refs()==1);
		linear_material->Release_Ref();
		TextureClass *texture=model->Peek_Texture(0);
		assert(texture!=nullptr);
		const int initial_texture_refs=texture->Num_Refs();
		DX8Wrapper::Set_Texture(0,texture);
		assert(texture->Num_Refs()==initial_texture_refs+1);
		assert(DX8Wrapper::Peek_Texture(0)==texture);
		DX8Wrapper::Set_Texture(0,nullptr);
		assert(texture->Num_Refs()==initial_texture_refs);
		assert(DX8Wrapper::Pending_Changes()&1U);
		bool unsupported_stage=false;
		try { DX8Wrapper::Set_DX8_Texture_Stage_State(0,999,1); }
		catch (const std::runtime_error&) { unsupported_stage=true; }
		assert(unsupported_stage);
		bool unsupported_index=false;
		try { DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_TEXCOORDINDEX,8); }
		catch (const std::runtime_error&) { unsupported_index=true; }
		assert(unsupported_index);
		DX8Wrapper::Set_Shader(model->Get_Shader(0));
		assert(DX8Wrapper::Pending_Changes()&(1U<<9));
		DX8Wrapper::Apply_Render_State_Changes();
		assert(DX8Wrapper::Pending_Changes()==0);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,8);
		const auto biased_source=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		assert(biased_source.pipeline.raster.depth_bias==-8.0f &&
			DX8Wrapper::Snapshot_Source_State().render.at(D3DRS_ZBIAS)==8);
		bool invalid_source_bias=false;
		try { DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,7); }
		catch (const std::runtime_error&) { invalid_source_bias=true; }
		assert(invalid_source_bias && DX8Wrapper::Snapshot_Source_State().render.at(D3DRS_ZBIAS)==8);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS,0);
		const auto unbiased_source=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		assert(unbiased_source.pipeline.raster.depth_bias==0.0f &&
			zh::renderer::PipelineKey(biased_source.pipeline)!=
			zh::renderer::PipelineKey(unbiased_source.pipeline));
		assert(device.snapshot().find("DX8Wrapper::Set_DX8_Render_State=47:8")!=std::string::npos &&
			device.snapshot().find("DX8Wrapper::Set_DX8_Render_State=47:0")!=std::string::npos);
		assert(device.snapshot().find("original TextureClass::Apply stage=0 disabled")!=
			std::string::npos);
		assert(device.snapshot().find("DX8Wrapper::Set_DX8_Texture_Stage_State=0:1")!=std::string::npos);
		assert(!device.pass_active());
	}
	assert(DX8Wrapper::Peek_Texture(0)==nullptr);
	assert(DX8Wrapper::Pending_Changes()==0);
	{
		zh::renderer::RecordingGpuDevice retry_device;
		zh::original_runtime::OriginalGpuEdge retry_edge(retry_device);
		DX8Wrapper::Set_Material(stage_material);
		DX8Wrapper::Set_Shader(model->Get_Shader(0));
		Matrix4x4 projection;
		camera.Get_D3D_Projection_Matrix(&projection);
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION,projection);
		DX8Wrapper::Apply_Render_State_Changes();
		const auto mapped=zh::original_runtime::OriginalGpuEdge::map_applied_state(
			DX8FVFCategoryContainer::Define_FVF(model,true));
		assert(mapped.stages[0].transform_set &&
			mapped.stages[0].transform_flags==(D3DTTFF_PROJECTED|D3DTTFF_COUNT3) &&
			mapped.stages[0].coordinate_mode==D3DTSS_TCI_CAMERASPACEPOSITION);
		assert(mapped.lighting);
		assert(std::fabs(mapped.diffuse[3]-0.75f)<0.0001f);
		assert(retry_device.snapshot().find("DX8Wrapper::Set_DX8_Material")!=std::string::npos);
		assert(DX8Wrapper::Pending_Changes()==0);
	}
	assert(DX8Wrapper::Peek_Material()==nullptr && DX8Wrapper::Pending_Changes()==0);
	{
		zh::renderer::RecordingGpuDevice lighting_device;
		zh::original_runtime::OriginalGpuEdge lighting_edge(lighting_device);
		DX8Wrapper::Set_Material(stage_material);
		DX8Wrapper::Set_Shader(model->Get_Shader(0));
		Matrix4x4 lighting_projection;
		camera.Get_D3D_Projection_Matrix(&lighting_projection);
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION,lighting_projection);
		Matrix4x4 lighting_world(true);
		lighting_world[0].X=2.0f;
		lighting_world[1].Y=0.5f;
		DX8Wrapper::Set_Transform(D3DTS_WORLD,lighting_world);
		DX8Wrapper::Set_Transform(D3DTS_VIEW,Matrix4x4(true));
		DX8Wrapper::Apply_Render_State_Changes();
		const auto reset_lighting=DX8Wrapper::Snapshot_Source_State();
		assert(reset_lighting.render.at(D3DRS_SPECULARMATERIALSOURCE)==D3DMCS_MATERIAL &&
			reset_lighting.render.at(D3DRS_COLORVERTEX)==TRUE &&
			reset_lighting.render.at(D3DRS_LOCALVIEWER)==TRUE &&
			reset_lighting.render.at(D3DRS_NORMALIZENORMALS)==FALSE);
		LightEnvironmentClass environment;
		environment.Reset(Vector3(0,0,0),Vector3(0.2f,0.4f,0.6f));
		environment.Pre_Render_Update(Matrix3D(true));
		DX8Wrapper::Set_Light_Environment(&environment);
		auto empty_lights=DX8Wrapper::Snapshot_Source_State();
		assert(empty_lights.light_environment_selected &&
			empty_lights.render.at(D3DRS_AMBIENT)==0x00336699U &&
			std::none_of(empty_lights.light_enabled.begin(),empty_lights.light_enabled.end(),
				[](bool enabled) { return enabled; }));
		const auto selected_lighting=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		bool absent_normal=false;
		try { (void)zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZDUV1); }
		catch (const std::runtime_error& error) {
			absent_normal=std::strstr(error.what(),"requires source FVF normal")!=nullptr;
		}
		assert(absent_normal && DX8Wrapper::Snapshot_Source_State().light_environment_selected);
		assert(selected_lighting.lighting && selected_lighting.light_environment_selected &&
			selected_lighting.source_world_set && selected_lighting.source_view_set &&
			selected_lighting.source_world[0]==2.0f && selected_lighting.source_world[5]==0.5f &&
			selected_lighting.color_vertex && selected_lighting.local_viewer &&
			!selected_lighting.normalize_normals &&
			selected_lighting.specular_source==D3DMCS_MATERIAL &&
			selected_lighting.global_ambient[0]>0.19f &&
			selected_lighting.global_ambient[2]>0.59f);
		const auto prior_ambient=stage_material->Get_Ambient_Color_Source();
		const auto prior_diffuse=stage_material->Get_Diffuse_Color_Source();
		const auto prior_emissive=stage_material->Get_Emissive_Color_Source();
		stage_material->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
		stage_material->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR2);
		stage_material->Set_Emissive_Color_Source(VertexMaterialClass::COLOR1);
		DX8Wrapper::Set_Material(stage_material);
		DX8Wrapper::Apply_Render_State_Changes();
		const auto material_selection=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		assert(material_selection.ambient_source==D3DMCS_COLOR1 &&
			material_selection.diffuse_source==D3DMCS_COLOR2 &&
			material_selection.emissive_source==D3DMCS_COLOR1);
		stage_material->Set_Ambient_Color_Source(prior_ambient);
		stage_material->Set_Diffuse_Color_Source(prior_diffuse);
		stage_material->Set_Emissive_Color_Source(prior_emissive);
		DX8Wrapper::Set_Material(stage_material);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS,TRUE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LOCALVIEWER,FALSE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORVERTEX,FALSE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_COLOR2);
		const auto changed_lighting=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		assert(changed_lighting.normalize_normals && !changed_lighting.local_viewer &&
			!changed_lighting.color_vertex && changed_lighting.specular_source==D3DMCS_COLOR2);
		bool bad_lighting_state=false;
		try { DX8Wrapper::Set_DX8_Render_State(D3DRS_LOCALVIEWER,2); }
		catch (const std::runtime_error&) { bad_lighting_state=true; }
		assert(bad_lighting_state && !zh::original_runtime::OriginalGpuEdge::map_applied_state(
			DX8_FVF_XYZNUV1).local_viewer);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS,FALSE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LOCALVIEWER,TRUE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORVERTEX,TRUE);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL);
		LightClass directional(LightClass::DIRECTIONAL);
		directional.Set_Diffuse(Vector3(0.5f,0.25f,0.125f));
		environment.Reset(Vector3(0,0,0),Vector3(0.1f,0.2f,0.3f));
		environment.Add_Light(directional);
		environment.Pre_Render_Update(Matrix3D(true));
		DX8Wrapper::Set_Light_Environment(&environment);
		auto directional_lights=DX8Wrapper::Snapshot_Source_State();
		assert(directional_lights.light_enabled[0] && !directional_lights.light_enabled[1] &&
			directional_lights.lights[0].Type==D3DLIGHT_DIRECTIONAL &&
			std::fabs(directional_lights.lights[0].Diffuse.r-0.5f)<0.0001f &&
			directional_lights.lights[0].Specular.r==1.0f);
		LightClass point(LightClass::POINT);
		point.Set_Position(Vector3(0,0,1));
		point.Set_Diffuse(Vector3(0.6f,0.4f,0.2f));
		point.Set_Ambient(Vector3(0.01f,0.02f,0.03f));
		point.Set_Near_Attenuation_Range(0,2);
		point.Set_Far_Attenuation_Range(2,10);
		environment.Add_Light(point);
		environment.Pre_Render_Update(Matrix3D(true));
		DX8Wrapper::Set_Light_Environment(&environment);
		auto mixed_lights=DX8Wrapper::Snapshot_Source_State();
		const auto mapped_mixed=zh::original_runtime::OriginalGpuEdge::map_applied_state(DX8_FVF_XYZNUV1);
		assert(mapped_mixed.light_enabled[0] && mapped_mixed.light_enabled[1] &&
			mapped_mixed.lights[0].Type==mixed_lights.lights[0].Type &&
			mapped_mixed.lights[1].Type==mixed_lights.lights[1].Type &&
			mapped_mixed.lights[1].Attenuation2==mixed_lights.lights[1].Attenuation2);
		assert(mixed_lights.light_enabled[0] && mixed_lights.light_enabled[1] &&
			std::any_of(mixed_lights.lights.begin(),mixed_lights.lights.begin()+2,
				[](const D3DLIGHT8 &light) { return light.Type==D3DLIGHT_POINT &&
					light.Range==10.0f && light.Attenuation0==1.0f &&
					std::fabs(light.Attenuation1-0.05f)<0.0001f &&
					std::fabs(light.Attenuation2-0.08f)<0.0001f; }));
		environment.Reset(Vector3(0,0,0),Vector3(0.05f,0.1f,0.15f));
		for (int light_index=0;light_index<4;++light_index) {
			LightClass selected(LightClass::DIRECTIONAL);
			selected.Set_Diffuse(Vector3(0.2f+0.1f*light_index,0.1f,0.05f));
			environment.Add_Light(selected);
		}
		environment.Pre_Render_Update(Matrix3D(true));
		assert(environment.Get_Light_Count()==4);
		DX8Wrapper::Set_Light_Environment(&environment);
		const auto full_lights=DX8Wrapper::Snapshot_Source_State();
		assert(std::all_of(full_lights.light_enabled.begin(),full_lights.light_enabled.end(),
			[](bool enabled) { return enabled; }));
		DX8Wrapper::Set_Light_Environment(nullptr);
		assert(!DX8Wrapper::Snapshot_Source_State().light_environment_selected &&
			DX8Wrapper::Snapshot_Source_State().light_enabled[3]);
		assert(!zh::original_runtime::OriginalGpuEdge::map_applied_state(
			DX8_FVF_XYZNUV1).light_environment_selected);
		bool missing_lighting_environment=false;
		try { (void)lighting_edge.prepare_applied_state(DX8_FVF_XYZNUV1); }
		catch (const std::runtime_error&) { missing_lighting_environment=true; }
		assert(missing_lighting_environment);
		DX8Wrapper::Set_Light_Environment(&environment);
		Vector3 invalid_ambient(NAN,0,0);
		environment.Set_Output_Ambient(invalid_ambient);
		bool invalid_light_rejected=false;
		try { DX8Wrapper::Set_Light_Environment(&environment); }
		catch (const std::runtime_error&) { invalid_light_rejected=true; }
		assert(invalid_light_rejected && DX8Wrapper::Snapshot_Source_State().lights[0].Type==
			full_lights.lights[0].Type);
		const auto point_source=std::find_if(mixed_lights.lights.begin(),
			mixed_lights.lights.end(),[](const D3DLIGHT8& light){return light.Type==D3DLIGHT_POINT;});
		assert(point_source!=mixed_lights.lights.end());
		D3DLIGHT8 invalid_physical=*point_source;
		invalid_physical.Attenuation0=invalid_physical.Attenuation1=
			invalid_physical.Attenuation2=0;
		invalid_light_rejected=false;
		try { DX8Wrapper::Set_Light(0,&invalid_physical); }
		catch (const std::runtime_error&) { invalid_light_rejected=true; }
		assert(invalid_light_rejected && DX8Wrapper::Snapshot_Source_State().lights[0].Type==
			full_lights.lights[0].Type);
		invalid_physical=full_lights.lights[0];
		invalid_physical.Type=D3DLIGHT_POINT;
		invalid_physical.Range=-1.0f;
		invalid_light_rejected=false;
		try { DX8Wrapper::Set_Light(0,&invalid_physical); }
		catch (const std::runtime_error&) { invalid_light_rejected=true; }
		assert(invalid_light_rejected && DX8Wrapper::Snapshot_Source_State().lights[0].Type==
			full_lights.lights[0].Type);
		bool out_of_bounds=false;
		try { DX8Wrapper::Set_Light(4,&mixed_lights.lights[0]); }
		catch (const std::runtime_error&) { out_of_bounds=true; }
		assert(out_of_bounds);
		DX8Wrapper::Set_Light_Environment(nullptr);
		assert(!DX8Wrapper::Snapshot_Source_State().light_environment_selected &&
			DX8Wrapper::Snapshot_Source_State().light_enabled[0]);
		assert(lighting_device.snapshot().find("DX8Wrapper::Set_Light slot=0 enabled")!=
			std::string::npos);
	}
	bool missing_gpu_edge=false;
	try { DX8Wrapper::Set_Material(stage_material); }
	catch (const std::runtime_error&) { missing_gpu_edge=true; }
	assert(missing_gpu_edge && DX8Wrapper::Pending_Changes()==0);
	LightEnvironmentClass orphan_environment;
	orphan_environment.Reset(Vector3(0,0,0),Vector3(0.1f,0.2f,0.3f));
	bool orphan_lights_rejected=false;
	try { DX8Wrapper::Set_Light_Environment(&orphan_environment); }
	catch (const std::runtime_error&) { orphan_lights_rejected=true; }
	assert(orphan_lights_rejected);
	{
		zh::renderer::RecordingGpuDevice fresh_device;
		zh::original_runtime::OriginalGpuEdge fresh_edge(fresh_device);
		DX8Wrapper::Set_Shader(model->Get_Shader(0));
		DX8Wrapper::Apply_Render_State_Changes();
		orphan_environment.Pre_Render_Update(Matrix3D(true));
		DX8Wrapper::Set_Light_Environment(&orphan_environment);
		const auto fresh_lights=DX8Wrapper::Snapshot_Source_State();
		assert(fresh_lights.light_environment_selected &&
			std::none_of(fresh_lights.light_enabled.begin(),fresh_lights.light_enabled.end(),
				[](bool enabled) { return enabled; }));
	}
	TextureClass *stage_texture = model->Peek_Texture(0);
	assert(stage_texture != nullptr);
	assert(std::strcmp(stage_texture->Get_Texture_Name(), "mytex.tga") == 0);
	assert(stage_texture->Get_Filter().Get_U_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	std::vector<char> texture_bytes(512);
	RAMFileClass texture_file(texture_bytes.data(), static_cast<int>(texture_bytes.size()));
	assert(texture_file.Open(FileClass::WRITE));
	ChunkSaveClass texture_writer(&texture_file);
	assert(texture_writer.Begin_Chunk(W3D_CHUNK_TEXTURE));
	const char texture_name[] = "MYTEX.TGA";
	assert(texture_writer.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME));
	assert(texture_writer.Write(texture_name, sizeof(texture_name)) == sizeof(texture_name));
	assert(texture_writer.End_Chunk());
	W3dTextureInfoStruct texture_info{};
	texture_info.Attributes = W3DTEXTURE_NO_LOD | W3DTEXTURE_CLAMP_U;
	chunk(texture_writer, W3D_CHUNK_TEXTURE_INFO, texture_info);
	assert(texture_writer.End_Chunk());
	const int texture_size = texture_file.Size();
	texture_file.Close();
	RAMFileClass texture_input(texture_bytes.data(), texture_size);
	assert(texture_input.Open(FileClass::READ));
	ChunkLoadClass texture_loader(&texture_input);
	TextureClass *texture = Load_Texture(texture_loader);
	assert(texture != nullptr);
	assert(std::strcmp(texture->Get_Texture_Name(), "mytex.tga") == 0);
	assert(texture->Get_Mip_Level_Count() == MIP_LEVELS_1);
	assert(texture->Get_Filter().Get_U_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	assert(texture->Get_Filter().Get_V_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
	bool device_rejected = false;
	try { texture->Apply(0); } catch (const std::runtime_error &) { device_rejected = true; }
	assert(device_rejected);
	texture->Release_Ref();
	std::puts("original-rendering runtime provider=GeneralsMD WW3D2 CPU graph");
	texture_input.Close();
	std::vector<char> oversized_name(300, 'a');
	std::vector<char> malformed_texture_bytes(512);
	RAMFileClass malformed_texture_file(malformed_texture_bytes.data(), static_cast<int>(malformed_texture_bytes.size()));
	assert(malformed_texture_file.Open(FileClass::WRITE));
	ChunkSaveClass malformed_writer(&malformed_texture_file);
	assert(malformed_writer.Begin_Chunk(W3D_CHUNK_TEXTURE));
	assert(malformed_writer.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME));
	assert(malformed_writer.Write(oversized_name.data(), static_cast<unsigned>(oversized_name.size())) == oversized_name.size());
	assert(malformed_writer.End_Chunk());
	assert(malformed_writer.End_Chunk());
	const int malformed_texture_size = malformed_texture_file.Size();
	malformed_texture_file.Close();
	RAMFileClass malformed_texture_input(malformed_texture_bytes.data(), malformed_texture_size);
	assert(malformed_texture_input.Open(FileClass::READ));
	ChunkLoadClass malformed_loader(&malformed_texture_input);
	assert(Load_Texture(malformed_loader) == nullptr);
	malformed_texture_input.Close();
	std::vector<char> bad_material_bytes(512);
	RAMFileClass bad_material_file(bad_material_bytes.data(), static_cast<int>(bad_material_bytes.size()));
	assert(bad_material_file.Open(FileClass::WRITE));
	ChunkSaveClass bad_material_writer(&bad_material_file);
	assert(bad_material_writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL));
	assert(bad_material_writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL_NAME));
	assert(bad_material_writer.Write(oversized_name.data(), static_cast<unsigned>(oversized_name.size())) == oversized_name.size());
	assert(bad_material_writer.End_Chunk());
	assert(bad_material_writer.End_Chunk());
	const int bad_material_size = bad_material_file.Size();
	bad_material_file.Close();
	RAMFileClass bad_material_input(bad_material_bytes.data(), bad_material_size);
	assert(bad_material_input.Open(FileClass::READ));
	ChunkLoadClass bad_material_loader(&bad_material_input);
	assert(bad_material_loader.Open_Chunk());
	auto *bad_material = new VertexMaterialClass();
	assert(bad_material->Load_W3D(bad_material_loader) == WW3D_ERROR_LOAD_FAILED);
	bad_material->Release_Ref();
	bad_material_input.Close();
	object->Release_Ref();
	manager.Free_Assets();
	assert(!manager.Render_Obj_Exists("TEST.TRIANGLE"));
	std::vector<char> malformed_mesh_bytes(512);
	RAMFileClass malformed_mesh_file(malformed_mesh_bytes.data(), static_cast<int>(malformed_mesh_bytes.size()));
	assert(malformed_mesh_file.Open(FileClass::WRITE));
	ChunkSaveClass malformed_mesh_writer(&malformed_mesh_file);
	assert(malformed_mesh_writer.Begin_Chunk(W3D_CHUNK_MESH));
	W3dMeshHeader3Struct malformed_header{};
	malformed_header.Version = W3D_CURRENT_MESH_VERSION;
	std::strcpy(malformed_header.MeshName, "BAD");
	malformed_header.NumVertices = 1000001;
	chunk(malformed_mesh_writer, W3D_CHUNK_MESH_HEADER3, malformed_header);
	assert(malformed_mesh_writer.End_Chunk());
	const int malformed_mesh_size = malformed_mesh_file.Size();
	malformed_mesh_file.Close();
	RAMFileClass malformed_mesh_input(malformed_mesh_bytes.data(), malformed_mesh_size);
	assert(!manager.Load_3D_Assets(malformed_mesh_input));
	assert(!manager.Render_Obj_Exists("BAD"));
	std::vector<char> malformed_hlod_bytes(512);
	RAMFileClass malformed_hlod_file(malformed_hlod_bytes.data(), static_cast<int>(malformed_hlod_bytes.size()));
	assert(malformed_hlod_file.Open(FileClass::WRITE));
	ChunkSaveClass malformed_hlod_writer(&malformed_hlod_file);
	assert(malformed_hlod_writer.Begin_Chunk(W3D_CHUNK_HLOD));
	W3dHLodHeaderStruct malformed_hlod_header{};
	malformed_hlod_header.Version = W3D_CURRENT_HLOD_VERSION;
	malformed_hlod_header.LodCount = 1000000;
	std::strcpy(malformed_hlod_header.Name, "BADLOD");
	chunk(malformed_hlod_writer, W3D_CHUNK_HLOD_HEADER, malformed_hlod_header);
	assert(malformed_hlod_writer.End_Chunk());
	const int malformed_hlod_size = malformed_hlod_file.Size();
	malformed_hlod_file.Close();
	RAMFileClass malformed_hlod_input(malformed_hlod_bytes.data(), malformed_hlod_size);
	assert(!manager.Load_3D_Assets(malformed_hlod_input));
	assert(!manager.Render_Obj_Exists("BADLOD"));
	std::vector<char> invalid_bytes(32);
	RAMFileClass invalid_file(invalid_bytes.data(), static_cast<int>(invalid_bytes.size()));
	assert(invalid_file.Open(FileClass::WRITE));
	ChunkSaveClass invalid_writer(&invalid_file);
	assert(invalid_writer.Begin_Chunk(0x7ffffffeu));
	assert(invalid_writer.End_Chunk());
	const int invalid_size = invalid_file.Size();
	invalid_file.Close();
	RAMFileClass invalid_input(invalid_bytes.data(), invalid_size);
	assert(!manager.Load_3D_Assets(invalid_input));
	assert(!manager.Render_Obj_Exists("TEST.TRIANGLE"));
	TheDX8MeshRenderer.Shutdown();
	DynamicVBAccessClass::_Deinit();
	assert(VertexBufferClass::Get_Total_Buffer_Count() == 0);
	assert(IndexBufferClass::Get_Total_Buffer_Count() == 0);
	return 0;
}
