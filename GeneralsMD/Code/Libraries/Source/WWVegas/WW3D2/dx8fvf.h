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


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DX8_FVF_H
#define DX8_FVF_H

#include "always.h"
#if !defined(ZH_WW3D_CPU_ONLY)
#include <d3d8.h>
#endif
#ifdef WWDEBUG
#include "wwdebug.h"
#endif

class StringClass;

#if defined(ZH_WW3D_CPU_ONLY)
// Original FVF layout bit patterns, retained as CPU data identifiers only;
// the Linux build does not import a Direct3D SDK or device interface.
enum {
	DX8_FVF_XYZ = 0x002,
	DX8_FVF_XYZN = 0x002 | 0x010,
	DX8_FVF_XYZNUV1 = 0x002 | 0x010 | 0x100,
	DX8_FVF_XYZNUV2 = 0x002 | 0x010 | 0x200,
	DX8_FVF_XYZNDUV1 = 0x002 | 0x010 | 0x100 | 0x040,
	DX8_FVF_XYZNDUV2 = 0x002 | 0x010 | 0x200 | 0x040,
	DX8_FVF_XYZDUV1 = 0x002 | 0x100 | 0x040,
	DX8_FVF_XYZDUV2 = 0x002 | 0x200 | 0x040,
	DX8_FVF_XYZUV1 = 0x002 | 0x100,
	DX8_FVF_XYZUV2 = 0x002 | 0x200,
	DX8_FVF_XYZNDUV1TG3 = 0x002 | 0x010 | 0x040 | 0x400
		| (1 << 18) | (1 << 20) | (1 << 22),
	DX8_FVF_XYZNUV2DMAP = 0x002 | 0x010 | 0x300
		| (3 << 16) | (2 << 18),
	DX8_FVF_XYZNDCUBEMAP = 0x002 | 0x010 | 0x040,
};
#else
enum {
	DX8_FVF_XYZ				= D3DFVF_XYZ,
	DX8_FVF_XYZN			= D3DFVF_XYZ|D3DFVF_NORMAL,
	DX8_FVF_XYZNUV1		= D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1,
	DX8_FVF_XYZNUV2		= D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2,
	DX8_FVF_XYZNDUV1		= D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1|D3DFVF_DIFFUSE,
	DX8_FVF_XYZNDUV2		= D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_DIFFUSE,
	DX8_FVF_XYZDUV1		= D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_DIFFUSE,
	DX8_FVF_XYZDUV2		= D3DFVF_XYZ|D3DFVF_TEX2|D3DFVF_DIFFUSE,
	DX8_FVF_XYZUV1			= D3DFVF_XYZ|D3DFVF_TEX1,
	DX8_FVF_XYZUV2			= D3DFVF_XYZ|D3DFVF_TEX2,
 	DX8_FVF_XYZNDUV1TG3	= (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX4|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE3(1)|D3DFVF_TEXCOORDSIZE3(2)|D3DFVF_TEXCOORDSIZE3(3)),
 	DX8_FVF_XYZNUV2DMAP	= (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX3 | D3DFVF_TEXCOORDSIZE1(0) | D3DFVF_TEXCOORDSIZE4(1) | D3DFVF_TEXCOORDSIZE2(2) ),
	DX8_FVF_XYZNDCUBEMAP	= D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE //|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE3(0)
};
#endif

// ----------------------------------------------------------------------------
//
// Util structures for vertex buffer handling. Cast the void pointer returned
// by the vertex buffer to one of these structures.
//
// ----------------------------------------------------------------------------

struct VertexFormatXYZ
{
	float x;
	float y;
	float z;
};

struct VertexFormatXYZNUV1
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float u1;
	float v1;
};

struct VertexFormatXYZNUV2
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float u1;
	float v1;
	float u2;
	float v2;
};

struct VertexFormatXYZN
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
};

struct VertexFormatXYZNDUV1
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned diffuse;
	float u1;
	float v1;
};

struct VertexFormatXYZNDUV2
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned diffuse;
	float u1;
	float v1;
	float u2;
	float v2;
};

struct VertexFormatXYZDUV1
{
	float x;
	float y;
	float z;
	unsigned diffuse;
	float u1;
	float v1;
};

struct VertexFormatXYZDUV2
{
	float x;
	float y;
	float z;
	unsigned diffuse;
	float u1;
	float v1;
	float u2;
	float v2;
};

struct VertexFormatXYZUV1
{
	float x;
	float y;
	float z;
	float u1;
	float v1;
};

struct VertexFormatXYZUV2
{
	float x;
	float y;
	float z;
	float u1;
	float v1;
	float u2;
	float v2;
};

// todo KJM compress
struct VertexFormatXYZNDUV1TG3
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned diffuse;
	float u1;
	float v1;
	float Sx;
	float Sy;
	float Sz;
	float Tx;
	float Ty;
	float Tz;
	float SxTx;
	float SxTy;
	float SxTz;
};


// displacement mapping format
struct VertexFormatXYZNUV2DMAP
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float T1x;
	float T1y;
	float T1z;
	float T1w;
	float T2x;
	float T2y;
};

// cube map format (texcoords are normally generated)
struct VertexFormatXYZNDCUBEMAP
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned diffuse;
//	float u1;
//	float v1;
//	float w1;
};

// FVF info class can be created for any legal FVF. It constructs information
// of offsets to various elements in the vertex buffer.

class FVFInfoClass : public W3DMPO
{
	W3DMPO_GLUE(FVFInfoClass)
	static constexpr unsigned max_texcoord_sets = 8;

	mutable unsigned						FVF;
	mutable unsigned						fvf_size;

	unsigned							location_offset;
	unsigned							normal_offset;
	unsigned							blend_offset;
	unsigned							texcoord_offset[max_texcoord_sets];
	unsigned							diffuse_offset;
	unsigned							specular_offset;
public:
	FVFInfoClass(unsigned FVF, unsigned vertex_size=0);

	inline unsigned Get_Location_Offset() const { return location_offset; }
	inline unsigned Get_Normal_Offset() const { return normal_offset; }
#ifdef WWDEBUG
	inline unsigned Get_Tex_Offset(unsigned int n) const { WWASSERT(n<max_texcoord_sets); return texcoord_offset[n]; }
#else
	inline unsigned Get_Tex_Offset(unsigned int n) const { return texcoord_offset[n]; }	
#endif

	inline unsigned Get_Diffuse_Offset() const { return diffuse_offset; }
	inline unsigned Get_Specular_Offset() const { return specular_offset; }
	inline unsigned Get_FVF() const { return FVF; }
	inline unsigned Get_FVF_Size() const { return fvf_size; }

	void Get_FVF_Name(StringClass& fvfname) const;	// For debug purposes

	// for enabling vertex shaders
	inline void Set_FVF(unsigned fvf) const { FVF=fvf; }
	inline void Set_FVF_Size(unsigned size) const { fvf_size=size; }
};


#endif
