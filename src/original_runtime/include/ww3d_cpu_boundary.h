#pragma once

#include "vector4.h"

// Exact packing conversion formerly supplied by DX8Wrapper.  Keeping this at
// the CPU/device boundary lets the original W3D material code retain ownership
// of all colour decisions without pulling Direct3D into the portable graph.
namespace ZHWW3DCpuBoundary
{
inline Vector4 Unpack_ARGB8(unsigned color)
{
	return Vector4(((color >> 16) & 0xff) / 255.0f, ((color >> 8) & 0xff) / 255.0f,
		(color & 0xff) / 255.0f, ((color >> 24) & 0xff) / 255.0f);
}

inline unsigned Pack_ARGB8(const Vector4 &color)
{
	return (unsigned(color.W * 255.0f) << 24) | (unsigned(color.X * 255.0f) << 16) |
		(unsigned(color.Y * 255.0f) << 8) | unsigned(color.Z * 255.0f);
}
}

struct D3DCOLORVALUE { float r, g, b, a; };
struct D3DMATERIAL8
{
	D3DCOLORVALUE Diffuse;
	D3DCOLORVALUE Ambient;
	D3DCOLORVALUE Specular;
	D3DCOLORVALUE Emissive;
	float Power;
};

struct D3DVECTOR { float x, y, z; };
struct D3DLIGHT8
{
	unsigned Type;
	D3DCOLORVALUE Diffuse, Specular, Ambient;
	D3DVECTOR Position, Direction;
	float Range, Falloff, Attenuation0, Attenuation1, Attenuation2, Theta, Phi;
};

enum { D3DMCS_MATERIAL = 0, D3DMCS_COLOR1 = 1, D3DMCS_COLOR2 = 2 };
