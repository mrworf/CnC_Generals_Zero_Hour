/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** Original WW3D render-state layout shared by mutually exclusive device targets.
*/
#ifndef DX8_RENDER_STATE_H
#define DX8_RENDER_STATE_H

#include "refcount.h"

struct RenderStateStruct
{
	ShaderClass shader;
	VertexMaterialClass* material;
	TextureBaseClass* Textures[MAX_TEXTURE_STAGES];
	D3DLIGHT8 Lights[4];
	bool LightEnable[4];
	Matrix4x4 world;
	Matrix4x4 view;
	unsigned vertex_buffer_types[MAX_VERTEX_STREAMS];
	unsigned index_buffer_type;
	unsigned short vba_offset;
	unsigned short vba_count;
	unsigned short iba_offset;
	VertexBufferClass* vertex_buffers[MAX_VERTEX_STREAMS];
	IndexBufferClass* index_buffer;
	unsigned short index_base_offset;

	RenderStateStruct() : material(nullptr), index_buffer(nullptr)
	{
		for (unsigned i=0;i<MAX_VERTEX_STREAMS;++i) vertex_buffers[i]=nullptr;
		for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i) Textures[i]=nullptr;
	}
	~RenderStateStruct()
	{
		REF_PTR_RELEASE(material);
		for (unsigned i=0;i<MAX_VERTEX_STREAMS;++i) REF_PTR_RELEASE(vertex_buffers[i]);
		REF_PTR_RELEASE(index_buffer);
		for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i) REF_PTR_RELEASE(Textures[i]);
	}
	RenderStateStruct& operator=(const RenderStateStruct& src)
	{
		REF_PTR_SET(material,src.material);
		for (unsigned i=0;i<MAX_VERTEX_STREAMS;++i)
			REF_PTR_SET(vertex_buffers[i],src.vertex_buffers[i]);
		REF_PTR_SET(index_buffer,src.index_buffer);
		for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i)
			REF_PTR_SET(Textures[i],src.Textures[i]);
		for (unsigned i=0;i<4;++i) LightEnable[i]=src.LightEnable[i];
		if (LightEnable[0]) {
			Lights[0]=src.Lights[0];
			if (LightEnable[1]) {
				Lights[1]=src.Lights[1];
				if (LightEnable[2]) {
					Lights[2]=src.Lights[2];
					if (LightEnable[3]) Lights[3]=src.Lights[3];
				}
			}
		}
		shader=src.shader;
		world=src.world;
		view=src.view;
		for (unsigned i=0;i<MAX_VERTEX_STREAMS;++i)
			vertex_buffer_types[i]=src.vertex_buffer_types[i];
		index_buffer_type=src.index_buffer_type;
		vba_offset=src.vba_offset;
		vba_count=src.vba_count;
		iba_offset=src.iba_offset;
		index_base_offset=src.index_base_offset;
		return *this;
	}
};

#endif
