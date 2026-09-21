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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8fvf.h                               $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 5:06p                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                          $*
 *                                                                                             *
 * 06/26/02 KM VB Vertex format update for shaders                                       *
 * 07/17/02 KM VB Vertex format update for displacement mapping                               *
 * 08/01/02 KM VB Vertex format update for cube mapping                               *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dx8fvf.h"
#include "wwstring.h"
#if defined(ZH_WW3D_CPU_ONLY)
#include <stdexcept>
#endif
#if !defined(ZH_WW3D_CPU_ONLY)
#include <D3dx8core.h>
#endif

namespace {
#if defined(ZH_WW3D_CPU_ONLY)
constexpr unsigned fvf_xyz = 0x002;
constexpr unsigned fvf_xyzb4 = 0x00c;
constexpr unsigned fvf_lastbeta_ubyte4 = 0x1000;
constexpr unsigned fvf_normal = 0x010;
constexpr unsigned fvf_diffuse = 0x040;
constexpr unsigned fvf_specular = 0x080;
constexpr unsigned maximum_texcoords = 8;
constexpr unsigned texture_bits(unsigned index) { return index * 2 + 16; }
constexpr unsigned texture_size1(unsigned index) { return 3u << texture_bits(index); }
constexpr unsigned texture_size2(unsigned) { return 0; }
constexpr unsigned texture_size3(unsigned index) { return 1u << texture_bits(index); }
constexpr unsigned texture_size4(unsigned index) { return 2u << texture_bits(index); }
#else
constexpr unsigned fvf_xyz = D3DFVF_XYZ;
constexpr unsigned fvf_xyzb4 = D3DFVF_XYZB4;
constexpr unsigned fvf_lastbeta_ubyte4 = D3DFVF_LASTBETA_UBYTE4;
constexpr unsigned fvf_normal = D3DFVF_NORMAL;
constexpr unsigned fvf_diffuse = D3DFVF_DIFFUSE;
constexpr unsigned fvf_specular = D3DFVF_SPECULAR;
constexpr unsigned maximum_texcoords = D3DDP_MAXTEXCOORD;
constexpr unsigned texture_size1(unsigned index) { return D3DFVF_TEXCOORDSIZE1(index); }
constexpr unsigned texture_size2(unsigned index) { return D3DFVF_TEXCOORDSIZE2(index); }
constexpr unsigned texture_size3(unsigned index) { return D3DFVF_TEXCOORDSIZE3(index); }
constexpr unsigned texture_size4(unsigned index) { return D3DFVF_TEXCOORDSIZE4(index); }
#endif
} // namespace

static unsigned Get_FVF_Vertex_Size(unsigned FVF)
{
#if defined(ZH_WW3D_CPU_ONLY)
	static_assert(sizeof(unsigned) == 4, "original FVF packed color requires 32 bits");
	unsigned coordinates = 0;
	switch (FVF & 0x400e) {
	case 0x002: coordinates = 3; break; // XYZ
	case 0x004: coordinates = 4; break; // XYZRHW
	case 0x006: coordinates = 4; break; // XYZB1
	case 0x008: coordinates = 5; break; // XYZB2
	case 0x00a: coordinates = 6; break; // XYZB3
	case 0x00c: coordinates = 7; break; // XYZB4
	case 0x00e: coordinates = 8; break; // XYZB5
	case 0x4002: coordinates = 4; break; // XYZW
	default: throw std::runtime_error("unsupported original FVF position layout");
	}
	unsigned size = coordinates * sizeof(float);
	if (FVF & fvf_normal) size += 3 * sizeof(float);
	if (FVF & fvf_diffuse) size += sizeof(unsigned);
	if (FVF & fvf_specular) size += sizeof(unsigned);
	const unsigned texcount = (FVF >> 8) & 0xf;
	if (texcount > maximum_texcoords)
		throw std::runtime_error("unsupported original FVF texture-coordinate count");
	for (unsigned index = 0; index < texcount; ++index) {
		unsigned dimensions = (FVF >> texture_bits(index)) & 3u;
		size += (dimensions == 3u ? 1u : dimensions == 1u ? 3u :
			dimensions == 2u ? 4u : 2u) * sizeof(float);
	}
	return size;
#else
	return D3DXGetFVFVertexSize(FVF);
#endif
}

FVFInfoClass::FVFInfoClass(unsigned FVF_, unsigned vertex_size) 
	:
	FVF(FVF_),
	fvf_size(FVF!=0 ? Get_FVF_Vertex_Size(FVF) : vertex_size)
{
	location_offset=0;
	blend_offset=location_offset;
	
	if ((FVF&fvf_xyz)==fvf_xyz) blend_offset+=3*sizeof(float);
	normal_offset=blend_offset;

	if ( ((FVF&fvf_xyzb4)==fvf_xyzb4) &&
		  ((FVF&fvf_lastbeta_ubyte4)==fvf_lastbeta_ubyte4) ) normal_offset+=3*sizeof(float)+sizeof(unsigned);
	diffuse_offset=normal_offset;

	if ((FVF&fvf_normal)==fvf_normal) diffuse_offset+=3*sizeof(float);
	specular_offset=diffuse_offset;

	if ((FVF&fvf_diffuse)==fvf_diffuse) specular_offset+=sizeof(unsigned);
	texcoord_offset[0]=specular_offset;

	if ((FVF&fvf_specular)==fvf_specular) texcoord_offset[0]+=sizeof(unsigned);

	for (unsigned int i=1; i<maximum_texcoords; i++)
	{
		texcoord_offset[i]=texcoord_offset[i-1];

		if ((int(FVF)&texture_size1(i-1))==texture_size1(i-1)) texcoord_offset[i]+=sizeof(float);
		else if ((int(FVF)&texture_size2(i-1))==texture_size2(i-1)) texcoord_offset[i]+=2*sizeof(float);
		else if ((int(FVF)&texture_size3(i-1))==texture_size3(i-1)) texcoord_offset[i]+=3*sizeof(float);
		else if ((int(FVF)&texture_size4(i-1))==texture_size4(i-1)) texcoord_offset[i]+=4*sizeof(float);
	}
}

void FVFInfoClass::Get_FVF_Name(StringClass& fvfname) const
{
	switch (Get_FVF()) {
	case DX8_FVF_XYZ: fvfname="D3DFVF_XYZ"; break;
	case DX8_FVF_XYZN: fvfname="D3DFVF_XYZ|D3DFVF_NORMAL"; break;
	case DX8_FVF_XYZNUV1: fvfname="D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1"; break;
	case DX8_FVF_XYZNUV2: fvfname="D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2"; break;
	case DX8_FVF_XYZNDUV1: fvfname="D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_DIFFUSE"; break;
	case DX8_FVF_XYZNDUV2: fvfname="D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_DIFFUSE"; break;
	case DX8_FVF_XYZDUV1: fvfname="D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_DIFFUSE"; break;
	case DX8_FVF_XYZDUV2: fvfname="D3DFVF_XYZ|D3DFVF_TEX2|D3DFVF_DIFFUSE"; break;
	case DX8_FVF_XYZUV1: fvfname="D3DFVF_XYZ|D3DFVF_TEX1"; break;
	case DX8_FVF_XYZUV2: fvfname="D3DFVF_XYZ|D3DFVF_TEX2"; break;
	case DX8_FVF_XYZNDUV1TG3 : fvfname="(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX4|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE3(1)|D3DFVF_TEXCOORDSIZE3(2)|D3DFVF_TEXCOORDSIZE3(3))"; break;
	case DX8_FVF_XYZNUV2DMAP :	fvfname="(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE1(0)|D3DFVF_TEXCOORDSIZE4(1)|D3DFVF_TEXCOORDSIZE2(2))"; break;
	case DX8_FVF_XYZNDCUBEMAP : fvfname="(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVFTEXCOORDSIZE3(0)"; break;
	default: fvfname="Unknown!";
	}
}
