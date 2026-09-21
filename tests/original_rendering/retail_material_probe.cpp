#ifndef ZH_WW3D_CPU_ONLY
#error Retail material probe must use the original WW3D CPU ABI.
#endif

#include "mesh.h"
#include "meshmdl.h"
#include "mapper.h"
#include "vertmaterial.h"
#include "shader.h"
#include "dx8renderer.h"
#include "original_gpu_edge.h"

#include <cstdio>
#include <map>
#include <set>
#include <stdexcept>

namespace {
struct Families
{
	unsigned meshes=0;
	unsigned materials=0;
	std::map<int,unsigned> mappers;
	std::set<const VertexMaterialClass*> seen;
	std::set<unsigned long> shader_bits;
	std::map<int,unsigned> primary_gradients;
	std::map<int,unsigned> detail_colors;
	std::map<int,unsigned> detail_alphas;
	std::map<int,unsigned> fog_modes;
	std::map<int,unsigned> blend_modes;
	std::map<unsigned,unsigned> fvf_formats;
};

void collect(RenderObjClass *object,Families &families,unsigned depth)
{
	if (!object || depth>=16)
		throw std::runtime_error("original retail material hierarchy exceeds bounded depth");
	if (object->Class_ID()==RenderObjClass::CLASSID_MESH)
	{
		++families.meshes;
		const MeshModelClass *mesh=static_cast<MeshClass*>(object)->Peek_Model();
		const unsigned fvf=DX8FVFCategoryContainer::Define_FVF(
			const_cast<MeshModelClass*>(mesh),true);
		(void)zh::original_runtime::OriginalGpuEdge::layout_for_fvf(fvf);
		++families.fvf_formats[fvf];
		for (int pass=0;pass<mesh->Get_Pass_Count();++pass)
		{
			for (int polygon=0;polygon<mesh->Get_Polygon_Count();++polygon)
			{
				ShaderClass shader=mesh->Get_Shader(polygon,pass);
				if (!families.shader_bits.insert(shader.Get_Bits()).second) continue;
				++families.primary_gradients[shader.Get_Primary_Gradient()];
				++families.detail_colors[shader.Get_Post_Detail_Color_Func()];
				++families.detail_alphas[shader.Get_Post_Detail_Alpha_Func()];
				++families.fog_modes[shader.Get_Fog_Func()];
				++families.blend_modes[shader.Get_Src_Blend_Func()*
					ShaderClass::DSTBLEND_MAX+shader.Get_Dst_Blend_Func()];
			}
			for (int vertex=0;vertex<mesh->Get_Vertex_Count();++vertex)
			{
			VertexMaterialClass *material=mesh->Peek_Material(vertex,pass);
			if (!material || !families.seen.insert(material).second) continue;
			++families.materials;
			for (int stage=0;stage<2;++stage)
				if (TextureMapperClass *mapper=material->Peek_Mapper(stage))
				{
					const int id=mapper->Mapper_ID();
					if (id<=TextureMapperClass::MAPPER_ID_UNKNOWN ||
						id>TextureMapperClass::MAPPER_ID_GRID_WS_ENVIRONMENT)
						throw std::runtime_error("original retail material mapper family unsupported");
					++families.mappers[id];
				}
			}
		}
	}
	for (int index=0;index<object->Get_Num_Sub_Objects();++index)
	{
		RenderObjClass *child=object->Get_Sub_Object(index);
		try { collect(child,families,depth+1); }
		catch (...) { child->Release_Ref(); throw; }
		child->Release_Ref();
	}
}
}

extern "C" void zh_probe_retail_material_families(RenderObjClass *object)
{
	Families families;
	collect(object,families,0);
	std::printf("original retail material families: meshes=%u materials=%u",
		families.meshes,families.materials);
	for (const auto &[id,count]:families.mappers)
		std::printf(" mapper-%d=%u",id,count);
	std::puts("");
	std::printf("original retail shader families: variants=%zu",families.shader_bits.size());
	const auto report=[](const char* family,const std::map<int,unsigned>& counts) {
		for (const auto &[id,count]:counts) std::printf(" %s-%d=%u",family,id,count);
	};
	report("primary",families.primary_gradients);
	report("detail-color",families.detail_colors);
	report("detail-alpha",families.detail_alphas);
	report("fog",families.fog_modes);
	report("blend",families.blend_modes);
	std::puts("");
	std::printf("original retail FVF families: variants=%zu",families.fvf_formats.size());
	for (const auto &[fvf,count]:families.fvf_formats)
		std::printf(" fvf-%u=%u",fvf,count);
	std::puts("");
}
