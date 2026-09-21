#!/usr/bin/env python3
"""Guard the one original render-state layout consumed by both WW3D targets."""

import argparse
import json
import pathlib


FIELDS = (
    "ShaderClass shader;",
    "VertexMaterialClass* material;",
    "TextureBaseClass* Textures[MAX_TEXTURE_STAGES];",
    "D3DLIGHT8 Lights[4];",
    "bool LightEnable[4];",
    "Matrix4x4 world;",
    "Matrix4x4 view;",
    "unsigned vertex_buffer_types[MAX_VERTEX_STREAMS];",
    "unsigned index_buffer_type;",
    "unsigned short vba_offset;",
    "unsigned short vba_count;",
    "unsigned short iba_offset;",
    "VertexBufferClass* vertex_buffers[MAX_VERTEX_STREAMS];",
    "IndexBufferClass* index_buffer;",
    "unsigned short index_base_offset;",
)


def canonical(wrapper: str, layout: str | None) -> bool:
    if not layout or wrapper.count('#include "dx8renderstate.h"') != 2:
        return False
    if "struct RenderStateStruct" in wrapper or layout.count("struct RenderStateStruct") != 1:
        return False
    positions = [layout.find(field) for field in FIELDS]
    return all(position >= 0 for position in positions) and positions == sorted(positions)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--wrapper", type=pathlib.Path, required=True)
    parser.add_argument("--layout", type=pathlib.Path, required=True)
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    args = parser.parse_args()
    wrapper = args.wrapper.read_text()
    layout = args.layout.read_text()
    assert canonical(wrapper, layout)
    assert not canonical(wrapper, None)  # Provider removal must fail closed.
    assert not canonical(wrapper + "struct RenderStateStruct {};", layout)
    commands = json.loads(args.compile_commands.read_text())
    providers = [entry for entry in commands if entry["file"].endswith("/WW3D2/dx8wrapper.cpp")]
    assert len(providers) == 1 and "ZH_WW3D_CPU_ONLY" in providers[0]["command"]
    print("original WW3D shared sort-state ABI/provider removal: ok")


if __name__ == "__main__":
    main()
