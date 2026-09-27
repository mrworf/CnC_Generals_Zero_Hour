/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : MatInfo.h                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/matinfo.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/15/01 5:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "matinfo.h"
#include "clone_graph.h"
#include "wwdebug.h"
#include "meshmdl.h"
#include "texture.h"

MaterialInfoClass::MaterialInfoClass(void)
{
}

MaterialInfoClass::MaterialInfoClass(const MaterialInfoClass & src)
{
#if defined(__linux__)
	ww3d_clone::Attempt attempt;
	const int materials=src.VertexMaterials.Count(),textures=src.Textures.Count();
	if (materials<0 || materials>src.VertexMaterials.Length() ||
		textures<0 || textures>src.Textures.Length())
		throw std::runtime_error("original clone node counts are not admitted");
	ww3d_clone::Attempt::extent(materials,sizeof(VertexMaterialClass*));
	ww3d_clone::Attempt::extent(textures,sizeof(TextureClass*));
	// Match the authored empty destination's ten-element growth policy.
	const int material_capacity=((materials+9)/10)*10;
	const int texture_capacity=((textures+9)/10)*10;
	ww3d_clone::RefArray<VertexMaterialClass> material_nodes(material_capacity);
	ww3d_clone::RefArray<TextureClass> texture_nodes(texture_capacity);
	for (int i=0;i<materials;++i) {
		if (!src.VertexMaterials[i]) throw std::runtime_error("original clone material is missing");
		ww3d_clone::Attempt::fault();
		ww3d_clone::Ref<VertexMaterialClass> node(src.VertexMaterials[i]->Clone());
		ww3d_clone::Attempt::fault();
		material_nodes.append(node.release());
	}
	for (int i=0;i<textures;++i) {
		if (!src.Textures[i]) throw std::runtime_error("original clone texture is missing");
		ww3d_clone::Attempt::fault();
		src.Textures[i]->Add_Ref();
		ww3d_clone::Ref<TextureClass> node(src.Textures[i]);
		ww3d_clone::Attempt::fault();
		texture_nodes.append(node.release());
	}
	VertexMaterials.Vector=material_nodes.release();
	VertexMaterials.VectorMax=material_capacity;
	VertexMaterials.ActiveCount=materials;
	VertexMaterials.IsAllocated=material_capacity!=0;
	Textures.Vector=texture_nodes.release();
	Textures.VectorMax=texture_capacity;
	Textures.ActiveCount=textures;
	Textures.IsAllocated=texture_capacity!=0;
	attempt.commit();
#else
	for (int mi=0; mi<src.VertexMaterials.Count(); mi++) {
		VertexMaterialClass * vmat;
		vmat = src.VertexMaterials[mi]->Clone();
		VertexMaterials.Add(vmat);
	}
	
	for (int ti=0; ti<src.Textures.Count(); ti++) {
		TextureClass * tex = src.Textures[ti];
		tex->Add_Ref();
		Textures.Add(tex);
	}
#endif
}


MaterialInfoClass::~MaterialInfoClass(void)
{
	Free();
}


MaterialInfoClass * MaterialInfoClass::Clone(void) const
{ 
	return W3DNEW MaterialInfoClass(*this); 
}

int MaterialInfoClass::Add_Texture(TextureClass * tex)
{
	WWASSERT(tex != NULL);
	tex->Add_Ref();
	int index = Textures.Count();
	Textures.Add(tex);
	return index;
}

int MaterialInfoClass::Get_Texture_Index(const char * name)
{
	for (int i=0; i<Textures.Count(); i++) {
		if (stricmp(name,Textures[i]->Get_Texture_Name()) == 0) {
			return i;
		}
	}
	return -1;
}

TextureClass * MaterialInfoClass::Get_Texture(int index)
{
	WWASSERT(index >= 0);
	WWASSERT(index < Textures.Count());
	Textures[index]->Add_Ref();
	return Textures[index];
}

/*

void MaterialInfoClass::Set_Texture_Reduction_Factor(float trf)
{
	for (int i = 0; i < Textures.Count(); i++) {
		Textures[i]->Set_Reduction_Factor(trf);
	}
}


void MaterialInfoClass::Process_Texture_Reduction(void)
{
	for (int i = 0; i < Textures.Count(); i++) {
		Textures[i]->Process_Reduction();
	}
}
*/
void MaterialInfoClass::Free(void) 
{
	int i;
	
	for (i=0; i<VertexMaterials.Count(); i++) {
		REF_PTR_RELEASE(VertexMaterials[i]);
	}
	VertexMaterials.Delete_All(); 

	for (i=0; i<Textures.Count(); i++) {
		REF_PTR_RELEASE(Textures[i]);
	}
	Textures.Delete_All();
}


MaterialRemapperClass::MaterialRemapperClass(MaterialInfoClass * src,MaterialInfoClass * dest) :
	TextureCount(0),
	TextureRemaps(NULL),
	VertexMaterialCount(0),
	VertexMaterialRemaps(NULL),
	LastSrcVmat(NULL),
	LastDestVmat(NULL),
	LastSrcTex(NULL),
	LastDestTex(NULL)
{
#if defined(__linux__)
	if (!src || !dest || src->Texture_Count()!=dest->Texture_Count() ||
		src->Vertex_Material_Count()!=dest->Vertex_Material_Count())
		throw std::runtime_error("original clone remap owners do not agree");
#endif
	WWASSERT(src);
	WWASSERT(dest);
	WWASSERT(src->Texture_Count() == dest->Texture_Count());
	WWASSERT(src->Vertex_Material_Count() == dest->Vertex_Material_Count());

#if defined(__linux__)
	ww3d_clone::Attempt attempt;
	const int materials=src->Vertex_Material_Count(),textures=src->Texture_Count();
	ww3d_clone::Attempt::reserve(materials,sizeof(VmatRemapStruct));
	ww3d_clone::Attempt::reserve(textures,sizeof(TextureRemapStruct));
	std::unique_ptr<VmatRemapStruct[]> material_nodes;
	std::unique_ptr<TextureRemapStruct[]> texture_nodes;
	if (materials) { ww3d_clone::Attempt::fault();material_nodes.reset(new VmatRemapStruct[materials]); }
	if (textures) { ww3d_clone::Attempt::fault();texture_nodes.reset(new TextureRemapStruct[textures]); }
	for (int i=0;i<materials;++i) {
		material_nodes[i].Src=src->Peek_Vertex_Material(i);
		material_nodes[i].Dest=dest->Peek_Vertex_Material(i);
	}
	for (int i=0;i<textures;++i) {
		texture_nodes[i].Src=src->Peek_Texture(i);
		texture_nodes[i].Dest=dest->Peek_Texture(i);
	}
	SrcMatInfo=src;src->Add_Ref();
	DestMatInfo=dest;dest->Add_Ref();
	VertexMaterialCount=materials;TextureCount=textures;
	VertexMaterialRemaps=material_nodes.release();
	TextureRemaps=texture_nodes.release();
	attempt.commit();
#else
	SrcMatInfo = src;
	SrcMatInfo->Add_Ref();
	DestMatInfo = dest;
	DestMatInfo->Add_Ref();

	if (src->Vertex_Material_Count() > 0) {
		VertexMaterialCount = src->Vertex_Material_Count();
		VertexMaterialRemaps = W3DNEWARRAY VmatRemapStruct[VertexMaterialCount];
		for (int i=0; i<src->Vertex_Material_Count(); i++) {
			VertexMaterialRemaps[i].Src = src->Peek_Vertex_Material(i);
			VertexMaterialRemaps[i].Dest = dest->Peek_Vertex_Material(i);
		}
	}

	if (src->Texture_Count() > 0) {
		TextureCount = src->Texture_Count();
		TextureRemaps = W3DNEWARRAY TextureRemapStruct[TextureCount];
		for (int i=0; i<src->Texture_Count(); i++) {
			TextureRemaps[i].Src = src->Peek_Texture(i);
			TextureRemaps[i].Dest = dest->Peek_Texture(i);
		}
	}
#endif
}

MaterialRemapperClass::~MaterialRemapperClass(void)
{
	SrcMatInfo->Release_Ref();
	DestMatInfo->Release_Ref();

	if (TextureRemaps) {
		delete[] TextureRemaps;
	}
	if (VertexMaterialRemaps) {
		delete[] VertexMaterialRemaps;
	}
}

TextureClass * MaterialRemapperClass::Remap_Texture(TextureClass * src)
{
	if (src == NULL) return src;
	if (src == LastSrcTex) return LastDestTex;
	for (int i=0; i<TextureCount; i++) {
		if (TextureRemaps[i].Src == src) {
			LastSrcTex = src;
			LastDestTex = TextureRemaps[i].Dest;
			return TextureRemaps[i].Dest;
		}
	}
#if defined(__linux__)
	throw std::runtime_error("original clone texture remap is missing");
#else
	WWASSERT(0); // uh-oh didn't find the texture, what happend???
#endif
	return NULL;
}

VertexMaterialClass * MaterialRemapperClass::Remap_Vertex_Material(VertexMaterialClass * src)
{
	if (src == NULL) return src;
	if (src == LastSrcVmat) return LastDestVmat;
	for (int i=0; i<VertexMaterialCount; i++) {
		if (VertexMaterialRemaps[i].Src == src) {
			LastSrcVmat = src;
			LastDestVmat = VertexMaterialRemaps[i].Dest;
			return VertexMaterialRemaps[i].Dest;
		}
	}
#if defined(__linux__)
	throw std::runtime_error("original clone material remap is missing");
#else
	WWASSERT(0); // uh-oh didn't find the material, what happend???
#endif
	return NULL;
}

void MaterialRemapperClass::Remap_Mesh(const MeshMatDescClass * srcmeshmatdesc, MeshMatDescClass * destmeshmatdesc)
{
#if defined(__linux__)
	if (!srcmeshmatdesc || !destmeshmatdesc || srcmeshmatdesc->PassCount<0 ||
		srcmeshmatdesc->PassCount>MeshMatDescClass::MAX_PASSES ||
		srcmeshmatdesc->VertexCount<0 || srcmeshmatdesc->PolyCount<0 ||
		destmeshmatdesc->VertexCount!=srcmeshmatdesc->VertexCount ||
		destmeshmatdesc->PolyCount!=srcmeshmatdesc->PolyCount)
		throw std::runtime_error("original clone remap descriptor is not admitted");
	// Validate the complete candidate mapping before even private pointer writes.
	for (int pass=0;pass<srcmeshmatdesc->PassCount;++pass) {
		if (SrcMatInfo->Vertex_Material_Count()) {
			if (srcmeshmatdesc->MaterialArray[pass]) {
				if (srcmeshmatdesc->MaterialArray[pass]->Get_Count()!=srcmeshmatdesc->VertexCount ||
					(destmeshmatdesc->MaterialArray[pass] &&
					destmeshmatdesc->MaterialArray[pass]->Get_Count()!=srcmeshmatdesc->VertexCount))
					throw std::runtime_error("original clone material array is not admitted");
				if (!destmeshmatdesc->MaterialArray[pass])
					ww3d_clone::Attempt::reserve(srcmeshmatdesc->VertexCount,sizeof(VertexMaterialClass*));
				for (int i=0;i<srcmeshmatdesc->VertexCount;++i) Remap_Vertex_Material(srcmeshmatdesc->Peek_Material(i,pass));
			} else Remap_Vertex_Material(srcmeshmatdesc->Peek_Single_Material(pass));
		}
		for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			if (SrcMatInfo->Texture_Count()) {
				if (srcmeshmatdesc->TextureArray[pass][stage]) {
					if (srcmeshmatdesc->TextureArray[pass][stage]->Get_Count()!=srcmeshmatdesc->PolyCount ||
						(destmeshmatdesc->TextureArray[pass][stage] &&
						 destmeshmatdesc->TextureArray[pass][stage]->Get_Count()!=srcmeshmatdesc->PolyCount))
						throw std::runtime_error("original clone texture array is not admitted");
					if (!destmeshmatdesc->TextureArray[pass][stage])
						ww3d_clone::Attempt::reserve(srcmeshmatdesc->PolyCount,sizeof(TextureClass*));
					for (int i=0;i<srcmeshmatdesc->PolyCount;++i) Remap_Texture(srcmeshmatdesc->Peek_Texture(i,pass,stage));
				} else Remap_Texture(srcmeshmatdesc->Peek_Single_Texture(pass,stage));
			}
		}
	}
	ww3d_clone::Attempt::fault();
#endif
	/*
	** Remap the vertex materials if there is at least one of them
	*/
	if (SrcMatInfo->Vertex_Material_Count() >= 1) {
	
		for (int pass = 0;pass < srcmeshmatdesc->Get_Pass_Count(); pass++) {

			if (srcmeshmatdesc->Has_Material_Array(pass)) {
				
				for (int vert_index = 0; vert_index < srcmeshmatdesc->Get_Vertex_Count(); vert_index++) {
#if defined(__linux__)
					ww3d_clone::Attempt::fault();
#endif
					VertexMaterialClass * src = srcmeshmatdesc->Peek_Material(vert_index, pass);
					destmeshmatdesc->Set_Material(vert_index, Remap_Vertex_Material(src),pass);
				}

			} else {
			
				VertexMaterialClass * src = srcmeshmatdesc->Peek_Single_Material(pass);
#if defined(__linux__)
				ww3d_clone::Attempt::fault();
#endif
				destmeshmatdesc->Set_Single_Material(Remap_Vertex_Material(src), pass);
				
			}
		}
	}
	
	/*
	** Remap the textures if there is at least one of them
	*/
	if (SrcMatInfo->Texture_Count() >= 1) {
	
		for (int pass = 0;pass < srcmeshmatdesc->Get_Pass_Count(); pass++) {

			for (int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			
				if (srcmeshmatdesc->Has_Texture_Array(pass, stage)) {
					
					for (int poly_index = 0; poly_index < srcmeshmatdesc->Get_Polygon_Count(); poly_index++) {
#if defined(__linux__)
						ww3d_clone::Attempt::fault();
#endif
						TextureClass * src = srcmeshmatdesc->Peek_Texture(poly_index, pass, stage);
						destmeshmatdesc->Set_Texture(poly_index, Remap_Texture(src), pass, stage);
					}

				} else {
				
					TextureClass * src = srcmeshmatdesc->Peek_Single_Texture(pass, stage);
#if defined(__linux__)
					ww3d_clone::Attempt::fault();
#endif
					destmeshmatdesc->Set_Single_Texture(Remap_Texture(src), pass, stage);

				}
			}
		}
	}
}

MaterialCollectorClass::MaterialCollectorClass(void)
{
	LastShader = ShaderClass(0xFFFFFFFF);
	LastMaterial = NULL;
	LastTexture = NULL;
}

MaterialCollectorClass::~MaterialCollectorClass(void)
{
	Reset();
}

void MaterialCollectorClass::Collect_Materials(MeshModelClass * mesh)
{
	for (int pass = 0;pass < mesh->Get_Pass_Count(); pass++) {

		// Vertex materials (either single or per vertex)
		if (mesh->Has_Material_Array(pass)) {
			
			for (int vert_index = 0;vert_index < mesh->Get_Vertex_Count(); vert_index++) {
				VertexMaterialClass * mat = mesh->Peek_Material(vert_index,pass);
				Add_Vertex_Material(mat);
			}

		} else {
			VertexMaterialClass * mat = mesh->Get_Single_Material(pass);
			Add_Vertex_Material(mat);
			REF_PTR_RELEASE(mat);
		}
		

		// Shaders (single or per poly...)
		if (mesh->Has_Shader_Array(pass)) {
			for (int poly_index=0; poly_index < mesh->Get_Polygon_Count(); poly_index++) {
				Add_Shader(mesh->Get_Shader(poly_index,pass));
			}
		} else {
			ShaderClass sh = mesh->Get_Single_Shader(pass);
			Add_Shader(sh);
		}
				
		
		// Textures per pass, per stage (either array or single...)
		for (int stage = 0; stage < MeshMatDescClass::MAX_TEX_STAGES; stage++) {

			if (mesh->Has_Texture_Array(pass,stage)) {
				
				for (int poly_index = 0;poly_index < mesh->Get_Polygon_Count(); poly_index++) {
					TextureClass * tex = mesh->Peek_Texture(poly_index,pass,stage);
					Add_Texture(tex);
				}

			} else {
			
				TextureClass * tex = mesh->Peek_Single_Texture(pass,stage);
				Add_Texture(tex);

			}
		}
	}
}

void MaterialCollectorClass::Reset(void)
{
	for (int ti=0; ti<Textures.Count(); ti++) {
		REF_PTR_RELEASE(Textures[ti]);
	}
	for (int vi=0; vi<VertexMaterials.Count(); vi++) {
		REF_PTR_RELEASE(VertexMaterials[vi]);
	}
	Textures.Clear();
	VertexMaterials.Clear();
	Shaders.Clear();
}

void MaterialCollectorClass::Add_Texture(TextureClass * tex)
{
	if (tex == NULL) return;
	if (tex == LastTexture) return;
	if (Find_Texture(tex) != -1) return;
	Textures.Add(tex);
	tex->Add_Ref();
	LastTexture = tex;
}

void MaterialCollectorClass::Add_Shader(ShaderClass shader)
{
	if (shader == LastShader) return;
	if (Find_Shader(shader) != -1) return;
	Shaders.Add(shader);
	LastShader = shader;
}

void MaterialCollectorClass::Add_Vertex_Material(VertexMaterialClass * vmat)
{
	if (vmat == NULL) return;
	if (vmat == LastMaterial) return;
	if (Find_Vertex_Material(vmat) != -1) return;
	VertexMaterials.Add(vmat);
	vmat->Add_Ref();
	LastMaterial = vmat;
}

int MaterialCollectorClass::Get_Shader_Count(void)
{
	return Shaders.Count();
}

int MaterialCollectorClass::Get_Vertex_Material_Count(void)
{
	return VertexMaterials.Count();
}

int MaterialCollectorClass::Get_Texture_Count(void)
{
	return Textures.Count();
}
	
ShaderClass MaterialCollectorClass::Peek_Shader(int i)
{
	return Shaders[i];
}

TextureClass * MaterialCollectorClass::Peek_Texture(int i)
{
	return Textures[i];
}

VertexMaterialClass * MaterialCollectorClass::Peek_Vertex_Material(int i)
{
	return VertexMaterials[i];
}

int MaterialCollectorClass::Find_Shader(const ShaderClass & shader)
{
	for (int si=0; si<Shaders.Count(); si++) {
		if (Shaders[si] == shader) {
			return si;
		}
	}
	return -1;
}

int MaterialCollectorClass::Find_Texture(TextureClass * tex)
{
	for (int ti=0; ti<Textures.Count(); ti++) {
		if (Textures[ti] == tex) {
			return ti;
		}
	}
	return -1;
}

int MaterialCollectorClass::Find_Vertex_Material(VertexMaterialClass * mat)
{
	for (int vi=0; vi<VertexMaterials.Count(); vi++) {
		if (VertexMaterials[vi] == mat) {
			return vi;
		}
	}
	return -1;
}
