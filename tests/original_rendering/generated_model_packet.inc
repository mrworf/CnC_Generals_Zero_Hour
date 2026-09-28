template <typename T> void chunk(ChunkSaveClass &writer, unsigned id, const T &value)
{
	assert(writer.Begin_Chunk(id));
	assert(writer.Write(&value, sizeof(value)) == sizeof(value));
	assert(writer.End_Chunk());
}

void make_mesh(ChunkSaveClass &writer, bool supply_variant, bool tread_variant = false,
	bool skin_variant = false, unsigned texture_stages = 1, bool lit_uv_variant = false,
	bool invalid_skin = false, unsigned large_vertex_count = 0, bool batch_second = false,
	bool cull_tree_variant = false, const char *container_name = "TEST",
	const char *mesh_name = nullptr, unsigned large_triangle_count = 0,
	bool raised_tree_variant = false, const char* first_texture = nullptr, bool texture_mips = false,
	int authored_sort_level = 0)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_MESH));
	W3dMeshHeader3Struct header{};
	header.Version = W3D_CURRENT_MESH_VERSION;
	std::strcpy(header.MeshName, mesh_name ? mesh_name : cull_tree_variant ? "CULLTREE" :
		lit_uv_variant ? (texture_stages==2 ? "LITTWO01" : "LITONE01") :
		batch_second ? "SKIN02" : invalid_skin ? "BADSKIN" :
		texture_stages == 0 && !skin_variant ? "ZERO01" : texture_stages == 2 && !skin_variant ? "TWO01" :
		skin_variant ? "SKIN01" : tread_variant ? "TREADSL01" :
		(supply_variant ? "SUPPLY01" : "TRIANGLE"));
	if (skin_variant) header.Attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN;
	if (cull_tree_variant) header.Attributes |= W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL;
	std::strcpy(header.ContainerName, container_name);
	header.NumVertices = large_vertex_count ? large_vertex_count : 3;
	header.NumTris = large_triangle_count ? large_triangle_count : 1;
	header.SortLevel = authored_sort_level;
	header.Min = {0, 0, 0};
	header.Max = {1, 1, raised_tree_variant ? 1.0f : 0.0f};
	header.SphCenter = {0.5f, 0.5f, raised_tree_variant ? 0.5f : 0.0f};
	header.SphRadius = raised_tree_variant ? 1.5f : 1.0f;
	chunk(writer, W3D_CHUNK_MESH_HEADER3, header);
	std::vector<W3dVectorStruct> vertices(header.NumVertices);
	vertices[0]={0,0,0}; vertices[1]={1,0,0};
	vertices[2]={0,1,raised_tree_variant ? 1.0f : 0.0f};
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
	if (large_triangle_count) {
		std::vector<W3dTriStruct> triangles(large_triangle_count, triangle);
		assert(writer.Begin_Chunk(W3D_CHUNK_TRIANGLES));
		assert(writer.Write(triangles.data(), triangles.size() * sizeof(triangles[0])) ==
			static_cast<int>(triangles.size() * sizeof(triangles[0])));
		assert(writer.End_Chunk());
	} else chunk(writer, W3D_CHUNK_TRIANGLES, triangle);
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
			const char *texture_name = stage == 0 ? (first_texture ? first_texture : "MYTEX.TGA") : "MYTEX2.TGA";
			assert(writer.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME));
			assert(writer.Write(texture_name, std::strlen(texture_name) + 1) ==
				static_cast<int>(std::strlen(texture_name) + 1));
			assert(writer.End_Chunk());
			W3dTextureInfoStruct texture_info{};
			texture_info.Attributes = (texture_mips ? 0 : W3DTEXTURE_NO_LOD) | W3DTEXTURE_CLAMP_U;
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
			std::vector<W3dTexCoordStruct> texcoords(header.NumVertices);
			texcoords[0]={0,0}; texcoords[1]={1,0}; texcoords[2]={0,1};
			if (stage==1) { texcoords[0]={0.25f,0}; texcoords[1]={1,0.25f}; }
			assert(writer.Begin_Chunk(W3D_CHUNK_STAGE_TEXCOORDS));
			assert(writer.Write(texcoords.data(), texcoords.size() * sizeof(texcoords[0])) ==
				static_cast<int>(texcoords.size() * sizeof(texcoords[0])));
			assert(writer.End_Chunk());
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
	bool invalid_skin = false, bool batch_skin = false, const char *name_override = nullptr,
	const char *child_override = nullptr, bool empty = false)
{
	assert(writer.Begin_Chunk(W3D_CHUNK_HLOD));
	W3dHLodHeaderStruct header{};
	header.Version = W3D_CURRENT_HLOD_VERSION;
	header.LodCount = 1;
	std::strcpy(header.Name, name_override ? name_override : batch_skin ? "TEST.BATCHHLOD" : invalid_skin ? "TEST.BADHLOD" :
		(skin_variant ? "TEST.SKINHLOD" : "TEST.HLOD"));
	std::strcpy(header.HierarchyName, "TESTTREE");
	chunk(writer, W3D_CHUNK_HLOD_HEADER, header);
	assert(writer.Begin_Chunk(W3D_CHUNK_HLOD_LOD_ARRAY));
	W3dHLodArrayHeaderStruct array{};
	array.ModelCount = empty ? 0 : (supply_variant || batch_skin) ? 2 : 1;
	array.MaxScreenSize = 1.0f;
	chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, array);
	W3dHLodSubObjectStruct subobject{};
	std::strcpy(subobject.Name, child_override ? child_override : invalid_skin ? "TEST.BADSKIN" : skin_variant ? "TEST.SKIN01" :
		(supply_variant ? "TEST.SUPPLY01" : "TEST.TRIANGLE"));
	if (!empty) chunk(writer, W3D_CHUNK_HLOD_SUB_OBJECT, subobject);
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
