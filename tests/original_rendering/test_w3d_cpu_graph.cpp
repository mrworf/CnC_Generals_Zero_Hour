#include "assetmgr.h"
#include "mesh.h"
#include "meshmdl.h"
#include "hlod.h"
#include "chunkio.h"
#include "RAMFILE.H"
#include "w3d_file.h"
#include "texture.h"
#include "vertmaterial.h"
#include "camera.h"
#include "rinfo.h"
#include "dx8fvf.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <vector>

#undef assert
#define assert(condition) do { if (!(condition)) std::abort(); } while (false)

namespace {
template <typename T> void chunk(ChunkSaveClass &writer, unsigned id, const T &value)
{
	assert(writer.Begin_Chunk(id));
	assert(writer.Write(&value, sizeof(value)) == sizeof(value));
	assert(writer.End_Chunk());
}

void make_mesh(ChunkSaveClass &writer, bool supply_variant, bool tread_variant = false,
	bool skin_variant = false)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_MESH));
	W3dMeshHeader3Struct header{};
	header.Version = W3D_CURRENT_MESH_VERSION;
	std::strcpy(header.MeshName, skin_variant ? "SKIN01" : tread_variant ? "TREADSL01" :
		(supply_variant ? "SUPPLY01" : "TRIANGLE"));
	if (skin_variant) header.Attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN;
	std::strcpy(header.ContainerName, "TEST");
	header.NumVertices = 3;
	header.NumTris = 1;
	header.Min = {0, 0, 0};
	header.Max = {1, 1, 0};
	header.SphCenter = {0.5f, 0.5f, 0};
	header.SphRadius = 1;
	chunk(writer, W3D_CHUNK_MESH_HEADER3, header);
	const W3dVectorStruct vertices[3] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTICES));
	assert(writer.Write(vertices, sizeof(vertices)) == sizeof(vertices));
	assert(writer.End_Chunk());
	const W3dVectorStruct normals[3] = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_NORMALS));
	assert(writer.Write(normals, sizeof(normals)) == sizeof(normals));
	assert(writer.End_Chunk());
	W3dTriStruct triangle{};
	triangle.Vindex[0] = 0;
	triangle.Vindex[1] = 1;
	triangle.Vindex[2] = 2;
	triangle.Normal = {0, 0, 1};
	chunk(writer, W3D_CHUNK_TRIANGLES, triangle);
	if (skin_variant)
	{
		W3dVertInfStruct links[3]{};
		chunk(writer, W3D_CHUNK_VERTEX_INFLUENCES, links);
	}
	W3dMaterialInfoStruct material_info{};
	material_info.PassCount = 1;
	material_info.ShaderCount = 1;
	material_info.VertexMaterialCount = 1;
	material_info.TextureCount = 1;
	chunk(writer, W3D_CHUNK_MATERIAL_INFO, material_info);
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIALS));
	assert(writer.Begin_Chunk(W3D_CHUNK_VERTEX_MATERIAL));
	const char material_name[] = "LIT";
	chunk(writer, W3D_CHUNK_VERTEX_MATERIAL_NAME, material_name);
	W3dVertexMaterialStruct material{};
	W3d_Vertex_Material_Reset(&material);
	material.Opacity = 0.75f;
	material.Attributes = tread_variant ? W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET :
		W3DVERTMAT_STAGE0_MAPPING_SCREEN;
	chunk(writer, W3D_CHUNK_VERTEX_MATERIAL_INFO, material);
	assert(writer.End_Chunk());
	assert(writer.End_Chunk());
	assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURES));
	assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURE));
	const char texture_name[] = "MYTEX.TGA";
	chunk(writer, W3D_CHUNK_TEXTURE_NAME, texture_name);
	W3dTextureInfoStruct texture_info{};
	texture_info.Attributes = W3DTEXTURE_NO_LOD | W3DTEXTURE_CLAMP_U;
	chunk(writer, W3D_CHUNK_TEXTURE_INFO, texture_info);
	assert(writer.End_Chunk());
	assert(writer.End_Chunk());
	W3dShaderStruct shader{};
	W3d_Shader_Reset(&shader);
	W3d_Shader_Set_Dest_Blend_Func(&shader, W3DSHADER_DESTBLENDFUNC_ONE);
	chunk(writer, W3D_CHUNK_SHADERS, shader);
	assert(writer.Begin_Chunk(W3D_CHUNK_MATERIAL_PASS));
	const uint32 shader_index = 0;
	chunk(writer, W3D_CHUNK_SHADER_IDS, shader_index);
	chunk(writer, W3D_CHUNK_VERTEX_MATERIAL_IDS, shader_index);
	assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURE_STAGE));
	chunk(writer, W3D_CHUNK_TEXTURE_IDS, shader_index);
	assert(writer.End_Chunk());
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

void make_hlod(ChunkSaveClass &writer, bool supply_variant)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_HLOD));
	W3dHLodHeaderStruct header{};
	header.Version = W3D_CURRENT_HLOD_VERSION;
	header.LodCount = 1;
	std::strcpy(header.Name, "TEST.HLOD");
	std::strcpy(header.HierarchyName, "TESTTREE");
	chunk(writer, W3D_CHUNK_HLOD_HEADER, header);
	assert(writer.Begin_Chunk(W3D_CHUNK_HLOD_LOD_ARRAY));
	W3dHLodArrayHeaderStruct array{};
	array.ModelCount = supply_variant ? 2 : 1;
	array.MaxScreenSize = 1.0f;
	chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, array);
	W3dHLodSubObjectStruct subobject{};
	std::strcpy(subobject.Name, supply_variant ? "TEST.SUPPLY01" : "TEST.TRIANGLE");
	chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT, subobject);
	if (supply_variant)
	{
		W3dHLodSubObjectStruct tread{};
		std::strcpy(tread.Name, "TEST.TREADSL01");
		chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT, tread);
	}
	assert(writer.End_Chunk());
	assert(writer.End_Chunk());
}
}

int main(int argc, char **argv)
{
	std::vector<char> bytes(4096);
	RAMFileClass file(bytes.data(), static_cast<int>(bytes.size()));
	assert(file.Open(FileClass::WRITE));
	ChunkSaveClass writer(&file);
	const bool supply_variant = argc == 3 && std::strcmp(argv[1], "--emit") == 0;
	make_hierarchy(writer, supply_variant);
	make_animation(writer);
	make_mesh(writer, supply_variant);
	if (supply_variant) make_mesh(writer, true, true);
	else make_mesh(writer, false, false, true);
	make_hlod(writer, supply_variant);
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
	RenderObjClass *hlod = manager.Create_Render_Obj("TEST.HLOD");
	assert(hlod != nullptr);
	assert(hlod->Get_Num_Sub_Objects() == 1);
	hlod->Release_Ref();
	RAMFileClass duplicate_input(bytes.data(), size);
	assert(!manager.Load_3D_Assets(duplicate_input));
	assert(manager.Render_Obj_Exists("TEST.TRIANGLE"));
	RenderObjClass *object = manager.Create_Render_Obj("TEST.TRIANGLE");
	assert(object != nullptr && object->Class_ID() == RenderObjClass::CLASSID_MESH);
	auto *mesh = static_cast<MeshClass *>(object);
	CameraClass camera;
	RenderInfoClass render_info(camera);
	mesh->Set_Hidden(1);
	mesh->Render(render_info); // Original hidden state suppresses the device boundary.
	mesh->Set_Hidden(0);
	bool visible_device_rejected = false;
	try { mesh->Render(render_info); }
	catch (const std::runtime_error &) { visible_device_rejected = true; }
	assert(visible_device_rejected);
	MeshModelClass *model = mesh->Peek_Model();
	TheDX8MeshRenderer.Init();
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
	skin_model->Register_For_Rendering();
	assert(skin_model->Has_Polygon_Renderers());
	TheDX8MeshRenderer.Invalidate();
	TheDX8MeshRenderer.Clear_Pending_Delete_Lists();
	assert(VertexBufferClass::Get_Total_Buffer_Count() == 0);
	assert(IndexBufferClass::Get_Total_Buffer_Count() == 0);
	assert(!model->Has_Polygon_Renderers() && !skin_model->Has_Polygon_Renderers());
	model->Register_For_Rendering();
	skin_model->Register_For_Rendering();
	assert(model->Has_Polygon_Renderers() && skin_model->Has_Polygon_Renderers());
	skin_object->Release_Ref();
	bool gpu_edge_rejected = false;
	try { TheDX8MeshRenderer.Flush(); }
	catch (const std::runtime_error &) { gpu_edge_rejected = true; }
	assert(gpu_edge_rejected);
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
		(3, DX8IndexBufferClass::USAGE_DYNAMIC));
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
