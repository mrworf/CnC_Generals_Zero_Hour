#!/usr/bin/env python3
"""Fail-closed translation of owned GLSL 450 bindings to bgfx's public model.

The original source remains authoritative. bgfx Vulkan uses one UBO per shader
stage and separate image/sampler descriptors in set 0. This build-only lowering
retains per-block member names in a nested std140 UBO and records the mapping
for the device edge to consume in M30.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys


class LoweringError(ValueError):
    pass


BLOCK = re.compile(
    r"layout\s*\(\s*set\s*=\s*(\d+)\s*,\s*binding\s*=\s*(\d+)"
    r"(?:\s*,\s*std140)?\s*\)\s*uniform\s+(\w+)\s*\{([^{}]*)\}\s*(\w+)\s*;",
    re.S,
)
SAMPLER = re.compile(
    r"layout\s*\(\s*set\s*=\s*(\d+)\s*,\s*binding\s*=\s*(\d+)\s*\)"
    r"\s*uniform\s+(sampler2D|samplerCube)\s+(\w+)\s*;"
)


def lower(source: str, stage: str) -> tuple[str, dict]:
    if stage not in ("vertex", "fragment"):
        raise LoweringError("stage must be vertex or fragment")
    expected_block_set = 1 if stage == "vertex" else 3
    blocks = []
    for match in BLOCK.finditer(source):
        source_set, binding, typename, body, instance = match.groups()
        if int(source_set) != expected_block_set or int(binding) >= 4:
            raise LoweringError(f"unsupported {stage} uniform block set/binding {source_set}/{binding}")
        blocks.append((int(binding), typename, body, instance))
    if len(blocks) > 4 or len({block[0] for block in blocks}) != len(blocks):
        raise LoweringError("uniform block count/bindings exceed the four-slot contract")
    blocks.sort()
    source = BLOCK.sub("", source)
    used_blocks = {instance: bool(re.search(rf"\b{re.escape(instance)}\s*\.", source))
                   for _, _, _, instance in blocks}
    textures = []
    for match in SAMPLER.finditer(source):
        source_set, binding, kind, name = match.groups()
        if int(source_set) != 2 or int(binding) >= 8:
            raise LoweringError(f"unsupported texture set/binding {source_set}/{binding}")
        textures.append((int(binding), kind, name))
    if len({texture[0] for texture in textures}) != len(textures):
        raise LoweringError("duplicate texture binding")
    source = SAMPLER.sub("", source)
    used_textures = {name: bool(re.search(rf"\b{re.escape(name)}\b", source))
                     for _, _, name in textures}
    declarations = []
    for binding, kind, name in sorted(textures):
        # shaderc shifts images by 2 and samplers by 18. Equal source
        # bindings therefore become the bgfx-required 16-apart pair.
        physical = 8 + binding
        image_kind = "texture2D" if kind == "sampler2D" else "textureCube"
        declarations.append(f"layout(set=0,binding={physical}) uniform {image_kind} {name}_image;")
        declarations.append(f"layout(set=0,binding={physical}) uniform sampler {name}_sampler;")
        source = re.sub(rf"\b{re.escape(name)}\b",
                        f"{kind}({name}_image,{name}_sampler)", source)
    for index, (binding, typename, body, instance) in enumerate(blocks):
        declarations.append(f"struct ZhBlock{index} {{ {body} }};")
    if blocks:
        members = " ".join(f"ZhBlock{index} {instance};" for index, (_, _, _, instance) in enumerate(blocks))
        declarations.append(f"layout(set=0,binding=0,std140) uniform ZhStageUniforms {{ {members} }} zh_uniforms;")
        declarations.extend(f"#define {instance} zh_uniforms.{instance}" for _, _, _, instance in blocks)
    if re.search(r"\blayout\s*\(\s*set\s*=", source) or re.search(
        r"\buniform\s+(?:sampler|texture)(?:2D|Cube)?\b", source
    ):
        raise LoweringError("unhandled source descriptor declaration")
    lines = source.splitlines(keepends=True)
    insertion = 0
    while insertion < len(lines) and (lines[insertion].startswith("#version") or
                                      lines[insertion].startswith("#extension") or
                                      lines[insertion].strip() == ""):
        insertion += 1
    lines.insert(insertion, "\n".join(declarations) + "\n")
    result = "".join(lines)
    manifest = {
        "stage": stage,
        "uniform_blocks": [{"source_binding": binding, "type": typename, "instance": instance,
                            "used": used_blocks[instance]}
                           for binding, typename, _, instance in blocks],
        "textures": [{"source_binding": binding, "kind": kind, "name": name,
                      "bgfx_stage": 8 + binding, "used": used_textures[name]}
                     for binding, kind, name in sorted(textures)],
    }
    return result, manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--stage", choices=("vertex", "fragment"), required=True)
    args = parser.parse_args()
    try:
        source, manifest = lower(args.input.read_text(encoding="utf-8"), args.stage)
        args.output.write_text(source, encoding="utf-8")
        args.manifest.write_text(json.dumps(manifest, sort_keys=True, separators=(",", ":")) + "\n", encoding="utf-8")
    except (OSError, LoweringError) as error:
        print(f"bgfx GLSL lowering: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
