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
#include "mapper.h"
#include "camera.h"
#include "light.h"
#include "lightenvironment.h"
#include "rinfo.h"
#include "dx8fvf.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "statistics.h"
#include "static_sort_list.h"
#include "ww3d.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"
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

#undef assert
#define assert(condition) do { if (!(condition)) { std::fprintf(stderr,"original W3D CPU invariant %s:%d: %s\n",__FILE__,__LINE__,#condition); std::abort(); } } while (false)

namespace {
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
	if (argc==2 && std::strcmp(argv[1],"--skin-batch")==0)
		return test_original_skin_batch();
	std::vector<char> bytes(16384);
	RAMFileClass file(bytes.data(), static_cast<int>(bytes.size()));
	assert(file.Open(FileClass::WRITE));
	ChunkSaveClass writer(&file);
	const bool supply_variant = argc == 3 && std::strcmp(argv[1], "--emit") == 0;
	make_hierarchy(writer, supply_variant);
	make_animation(writer);
	make_mesh(writer, supply_variant);
	if (supply_variant) make_mesh(writer, true, true);
	else make_mesh(writer, false, false, true);
	if (!supply_variant) make_mesh(writer,false,false,true,1,false,true);
	if (argc == 2 && (std::strcmp(argv[1], "--device-edge") == 0 ||
		std::strcmp(argv[1], "--vulkan-category") == 0)) {
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
	WW3DAssetManager manager;
	RAMFileClass input(bytes.data(), size);
	assert(manager.Load_3D_Assets(input));
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
#if defined(ZH_GPU_SHADER_DIR)
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
	assert(VertexBufferClass::Get_Total_Buffer_Count() == 0);
	assert(IndexBufferClass::Get_Total_Buffer_Count() == 0);
	return 0;
}
