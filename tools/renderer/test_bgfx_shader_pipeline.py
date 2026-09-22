#!/usr/bin/env python3
"""Focused positive/negative contract tests for M29's offline shader bridge."""

from __future__ import annotations

from pathlib import Path
import struct
import tempfile
import unittest

from prepare_bgfx_glsl import LoweringError, lower
from verify_bgfx_shaders import ShaderError, check_mapping, parse_shader, spirv_descriptors, verify


FRAGMENT = """#version 450
layout(set=2,binding=1) uniform sampler2D stage1;
layout(set=3,binding=0,std140) uniform Frame { vec4 tint; } frame;
layout(location=0) out vec4 color;
void main() { color = texture(stage1, vec2(0.5)) * frame.tint; }
"""


class ShaderBridgeTests(unittest.TestCase):
    def test_lowering_preserves_named_source_blocks_and_texture_slots(self) -> None:
        source, manifest = lower(FRAGMENT, "fragment")
        self.assertIn("layout(set=0,binding=9) uniform texture2D stage1_image", source)
        self.assertIn("layout(set=0,binding=9) uniform sampler stage1_sampler", source)
        self.assertIn("ZhBlock0 frame;", source)
        self.assertIn("texture(sampler2D(stage1_image,stage1_sampler)", source)
        self.assertEqual(manifest["textures"][0]["bgfx_stage"], 9)

    def test_rejects_unknown_and_duplicate_layouts(self) -> None:
        fixtures = (
            FRAGMENT.replace("set=2,binding=1", "set=4,binding=1"),
            FRAGMENT.replace("set=2,binding=1", "set=2,binding=8"),
            FRAGMENT.replace("layout(set=3,binding=0", "layout(set=3,binding=4"),
            FRAGMENT.replace("layout(location=0)", "layout(set=7,binding=0) uniform samplerCube alien;\nlayout(location=0)"),
            FRAGMENT.replace("layout(location=0)", "layout(set=2,binding=1) uniform sampler2D duplicate;\nlayout(location=0)"),
        )
        for fixture in fixtures:
            with self.subTest(fixture=fixture[:90]), self.assertRaises(LoweringError):
                lower(fixture, "fragment")

    def test_mapping_rejects_wrong_set_and_missing_texture(self) -> None:
        _, manifest = lower(FRAGMENT, "fragment")
        # Small SPIR-V containing only descriptor names/decorations. The
        # public verifier separately runs spirv-val on real shader payloads.
        def instruction(opcode: int, *operands: int) -> list[int]:
            return [((len(operands) + 1) << 16) | opcode, *operands]

        def name(identifier: int, value: str) -> list[int]:
            raw = (value.encode() + b"\0").ljust((len(value) + 4) // 4 * 4, b"\0")
            return instruction(5, identifier, *struct.unpack(f"<{len(raw)//4}I", raw))

        words = [0x07230203, 0x10000, 0, 10, 0]
        for identifier, descriptor_name, binding in (
            (1, "zh_uniforms", 1), (2, "stage1_image", 11), (3, "stage1_sampler", 27)
        ):
            words += name(identifier, descriptor_name)
            words += instruction(71, identifier, 34, 0)
            words += instruction(71, identifier, 33, binding)
        payload = struct.pack(f"<{len(words)}I", *words)
        uniforms = ["ZhStageUniforms.frame.tint", "stage1_image"]
        check_mapping("renderer/test.frag.bin", uniforms, payload, manifest, FRAGMENT)
        wrong_set = payload.replace(struct.pack("<4I", (4 << 16) | 71, 2, 34, 0),
                                    struct.pack("<4I", (4 << 16) | 71, 2, 34, 2))
        with self.assertRaises(ShaderError):
            check_mapping("renderer/test.frag.bin", uniforms, wrong_set, manifest, FRAGMENT)
        with self.assertRaises(ShaderError):
            check_mapping("renderer/test.frag.bin", ["ZhStageUniforms.frame.tint"], payload, manifest, FRAGMENT)
        self.assertEqual(spirv_descriptors(payload, "fixture")["stage1_sampler"], (0, 27))

    def test_missing_family_and_malformed_envelope_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory(prefix="zh-bgfx-negative-") as directory:
            with self.assertRaisesRegex(ShaderError, "shader family mismatch"):
                verify(Path(__file__).resolve().parents[2], Path(directory), "spirv-val")
        with self.assertRaisesRegex(ShaderError, "wrong bgfx shader stage/version"):
            parse_shader(b"not a bgfx shader", "renderer/ui.vert.bin")


if __name__ == "__main__":
    unittest.main()
