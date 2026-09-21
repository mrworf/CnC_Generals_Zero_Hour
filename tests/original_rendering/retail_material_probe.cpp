#ifndef ZH_WW3D_CPU_ONLY
#error Retail material probe must use the original WW3D CPU ABI.
#endif

#include "mesh.h"
#include "meshmdl.h"
#include "mapper.h"
#include "vertmaterial.h"

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
};

void collect(RenderObjClass *object,Families &families,unsigned depth)
{
	if (!object || depth>=16)
		throw std::runtime_error("original retail material hierarchy exceeds bounded depth");
	if (object->Class_ID()==RenderObjClass::CLASSID_MESH)
	{
		++families.meshes;
		const MeshModelClass *mesh=static_cast<MeshClass*>(object)->Peek_Model();
		for (int pass=0;pass<mesh->Get_Pass_Count();++pass)
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
}
