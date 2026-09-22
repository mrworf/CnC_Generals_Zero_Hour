#!/usr/bin/env python3
"""Fail-closed offline inventory and bgfx/SPIR-V envelope check for M29."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile


class ShaderError(ValueError):
    pass


# These WW3D migration placeholders are explicitly multiplied by zero in the
# owned source. If their source expression changes, the exception fails.
DEAD_TEXTURES = {"renderer/world.frag.bin": {"texture1", "texture2", "shadow_texture"}}


def expected_outputs(source_root: Path) -> set[str]:
    expected = set()
    for group in ("renderer", "effects"):
        for source in (source_root / "shaders" / group).iterdir():
            if source.suffix not in (".vert", ".frag"):
                continue
            if source.name == "original_applied.vert":
                continue
            expected.add(f"{group}/{source.name}.bin")
    for variant in ("d1", "d2", "n0", "n1", "n2", "nd2"):
        expected.add(f"renderer/original_applied_{variant}.vert.bin")
        if variant in ("n0", "n1", "n2", "nd2"):
            expected.add(f"renderer/original_applied_{variant}_lit.vert.bin")
    return expected


def parse_shader(data: bytes, name: str) -> tuple[list[str], list[int], bytes]:
    stage = b"VSH" if name.endswith(".vert.bin") else b"FSH"
    if len(data) < 28 or data[:3] != stage or data[3] != 12:
        raise ShaderError(f"{name}: wrong bgfx shader stage/version envelope")
    offset = 20  # magic, input/output hashes, raw SRV/UAV masks

    def take(size: int) -> bytes:
        nonlocal offset
        if offset + size > len(data):
            raise ShaderError(f"{name}: truncated bgfx shader")
        value = data[offset:offset + size]
        offset += size
        return value

    def u8() -> int:
        return take(1)[0]

    def u16() -> int:
        return struct.unpack("<H", take(2))[0]

    def u32() -> int:
        return struct.unpack("<I", take(4))[0]

    uniforms = []
    for _ in range(u16()):
        size = u8()
        uniforms.append(take(size).decode("ascii"))
        take(10)  # type, array count, register, count, component, dimension, format
    spirv = take(u32())
    if len(spirv) < 20 or spirv[:4] != b"\x03\x02\x23\x07":
        raise ShaderError(f"{name}: payload is not SPIR-V")
    if u8() != 0:
        raise ShaderError(f"{name}: missing payload terminator")
    attributes = [u16() for _ in range(u8())]
    u16()  # reflected constant size
    if offset != len(data):
        raise ShaderError(f"{name}: trailing bytes in bgfx envelope")
    if stage == b"VSH" and not attributes:
        raise ShaderError(f"{name}: vertex shader lost its input reflection")
    if stage == b"FSH" and attributes:
        raise ShaderError(f"{name}: fragment shader has vertex attributes")
    live = [value for value in attributes if value != 65535]
    if stage == b"VSH" and (not live or len(live) != len(set(live))):
        raise ShaderError(f"{name}: unmapped or duplicate live vertex attributes")
    return uniforms, attributes, spirv


def spirv_descriptors(spirv: bytes, name: str) -> dict[str, tuple[int, int]]:
    """Read only the core SPIR-V names and descriptor decorations we require."""
    if len(spirv) % 4:
        raise ShaderError(f"{name}: unaligned SPIR-V payload")
    words = struct.unpack(f"<{len(spirv) // 4}I", spirv)
    names: dict[int, str] = {}
    sets: dict[int, int] = {}
    bindings: dict[int, int] = {}
    index = 5
    while index < len(words):
        opcode = words[index] & 0xffff
        count = words[index] >> 16
        if not count or index + count > len(words):
            raise ShaderError(f"{name}: malformed SPIR-V instruction")
        operands = words[index + 1:index + count]
        if opcode == 5 and len(operands) >= 2:  # OpName
            names[operands[0]] = struct.pack(f"<{len(operands) - 1}I", *operands[1:]).split(b"\0", 1)[0].decode("utf-8")
        elif opcode == 71 and len(operands) >= 3:  # OpDecorate
            if operands[1] == 34:  # DescriptorSet
                sets[operands[0]] = operands[2]
            elif operands[1] == 33:  # Binding
                bindings[operands[0]] = operands[2]
        index += count
    if index != len(words) or sets.keys() != bindings.keys():
        raise ShaderError(f"{name}: incomplete SPIR-V descriptor decorations")
    result = {}
    for descriptor_id, descriptor_set in sets.items():
        descriptor_name = names.get(descriptor_id)
        if not descriptor_name or descriptor_name in result:
            raise ShaderError(f"{name}: unnamed or duplicate descriptor")
        result[descriptor_name] = (descriptor_set, bindings[descriptor_id])
    return result


def check_mapping(name: str, uniforms: list[str], spirv: bytes, manifest: dict,
                  preprocessed: str) -> None:
    stage = "vertex" if name.endswith(".vert.bin") else "fragment"
    if manifest.get("stage") != stage:
        raise ShaderError(f"{name}: stage manifest mismatch")
    descriptors = spirv_descriptors(spirv, name)
    expected = {}
    blocks = manifest["uniform_blocks"]
    # glslang may eliminate an algebraically dead UBO while shaderc retains
    # source reflection (e.g. viewport * 0 in the fullscreen vertex family).
    if "zh_uniforms" in descriptors:
        if not any(block["used"] for block in blocks):
            raise ShaderError(f"{name}: unexpected stage UBO")
        expected["zh_uniforms"] = (0, 0 if stage == "vertex" else 1)
    for block in blocks:
        prefix = f"ZhStageUniforms.{block['instance']}."
        if block["used"] and not any(uniform.startswith(prefix) for uniform in uniforms):
            raise ShaderError(f"{name}: source block {block['instance']} lost bgfx uniform reflection")
    for texture in manifest["textures"]:
        image_name = texture["name"] + "_image"
        sampler_name = texture["name"] + "_sampler"
        if texture["used"]:
            if image_name not in descriptors and texture["name"] in DEAD_TEXTURES.get(name, set()):
                zero_sample = rf"\btexture\s*\(\s*{re.escape(texture['name'])}\s*,[^;]*?\)\s*\*\s*0\.0\b"
                references = re.findall(rf"\b{re.escape(texture['name'])}\b", preprocessed)
                if len(references) != 2 or not re.search(zero_sample, preprocessed, re.S):
                    raise ShaderError(f"{name}: dead-texture source exception changed for {texture['name']}")
                continue
            binding = texture["bgfx_stage"] + 2
            expected[image_name] = (0, binding)
            expected[sampler_name] = (0, binding + 16)
            if image_name not in uniforms:
                raise ShaderError(f"{name}: source texture {texture['name']} lost bgfx reflection")
    if descriptors != expected:
        raise ShaderError(f"{name}: descriptor mapping mismatch: {descriptors} != {expected}")


def verify(source_root: Path, build_root: Path, spirv_val: str) -> dict[str, list[str]]:
    output_root = build_root / "generated" / "bgfx"
    expected = expected_outputs(source_root)
    actual = {path.relative_to(output_root).as_posix() for path in output_root.rglob("*.bin")}
    if actual != expected:
        raise ShaderError(f"shader family mismatch; missing={sorted(expected - actual)}, extra={sorted(actual - expected)}")
    reflected = {}
    with tempfile.TemporaryDirectory(prefix="zh-bgfx-verify-") as directory:
        temporary = Path(directory) / "shader.spv"
        for name in sorted(expected):
            uniforms, attributes, spirv = parse_shader((output_root / name).read_bytes(), name)
            manifest_name = f"{name.removesuffix('.bin')}.json"
            manifest = json.loads((output_root / manifest_name).read_text(encoding="utf-8"))
            preprocessed_name = f"{name.removesuffix('.bin')}.pre"
            preprocessed = (output_root / preprocessed_name).read_text(encoding="utf-8")
            check_mapping(name, uniforms, spirv, manifest, preprocessed)
            temporary.write_bytes(spirv)
            result = subprocess.run([spirv_val, "--target-env", "vulkan1.0", str(temporary)],
                                    capture_output=True, text=True, check=False)
            if result.returncode:
                raise ShaderError(f"{name}: spirv-val failed: {result.stderr.strip()}")
            reflected[name] = uniforms
            if name.endswith(".vert.bin") and all(value == 65535 for value in attributes):
                raise ShaderError(f"{name}: no bgfx-mapped vertex attribute")
    for required in ("renderer/ui.vert.bin", "renderer/terrain.frag.bin",
                     "renderer/water.frag.bin", "renderer/points.vert.bin",
                     "renderer/wwshade.frag.bin", "effects/projected.frag.bin",
                     "renderer/original_applied_nd2_lit.vert.bin"):
        if required not in reflected:
            raise ShaderError(f"missing required material family {required}")
    required_integer_payloads = {
        "renderer/original_applied_nd2_lit.vert.bin":
            {"coordinate_modes", "uv_indices", "transform_flags", "lit_switches", "lit_material_sources"},
        "renderer/original_applied_0.frag.bin": {"stage_ops", "stage_args"},
    }
    for family, fields in required_integer_payloads.items():
        names = reflected[family]
        missing = {field for field in fields if not any(name.endswith("." + field) for name in names)}
        if missing:
            raise ShaderError(f"{family}: integer uniform payloads lost reflection: {sorted(missing)}")
    return reflected


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--build-root", type=Path, required=True)
    parser.add_argument("--spirv-val", required=True)
    args = parser.parse_args()
    try:
        reflected = verify(args.source_root, args.build_root, args.spirv_val)
    except (OSError, ShaderError) as error:
        print(f"bgfx shader closure: {error}", file=sys.stderr)
        return 1
    print(f"bgfx shader closure: {len(reflected)} pinned, mapped, valid SPIR-V families")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
