#!/usr/bin/env python3
"""Drive one generated map through the real original display factory."""

import argparse
import os
from pathlib import Path
import re
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import fixture, make_read_only, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import tga
from test_w3d_visual_height_map import visual_map
from test_w3d_status_scene import has_validation_diagnostic


MARKER = re.compile(
    r"mode=original display=1 view=1 terrain=1 empty=(\d+) changed=(\d+) "
    r"aliases=(\d+) .* residual=(\d+) baseline=(\d+) map=(\d+)"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    executable = args.executable.resolve()
    with TemporaryDirectory(prefix="zh-m22-factory-map-") as scratch:
        root = Path(scratch)
        packet = root / "factory.map"
        packet.write_bytes(visual_map(texture_name="Flat"))
        packet.chmod(0o444)
        malformed = root / "malformed.map"
        malformed.write_bytes(b"not a generated map")
        malformed.chmod(0o444)

        def source_tree(root: Path) -> Path:
            source = root / "readonly-input"
            fixture(source)
            (source / "Data/INI/Default/Terrain.ini").write_text(
                "Terrain DefaultTerrain\n Texture = Flat.tga\n Class = NONE\nEnd\n"
                "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n")
            terrain = source / "Art/Terrain/Flat.tga"
            terrain.parent.mkdir(parents=True, exist_ok=True)
            terrain.write_bytes(tga("valid"))
            make_read_only(source)
            return source

        def invoke(source: Path, map_path: Path | None, extra: str | None = None):
            overrides = {"ZH_M22_ORIGINAL_FACTORY_PROFILE": "1"}
            if map_path is not None:
                overrides["ZH_M22_FACTORY_MAP"] = str(map_path)
            if extra is not None:
                overrides[extra] = "1"
            result = run(str(executable), root / f"run-{source.name}-{map_path and map_path.name}-{extra}",
                         source, env_overrides=overrides)
            output = result.stdout + result.stderr
            if has_validation_diagnostic(output):
                raise SystemExit(f"factory map emitted Vulkan validation diagnostics:\n{output}")
            return result, output

        no_map_source = source_tree(root / "no-map-source")
        absent, output = invoke(no_map_source, None)
        absent_marker = MARKER.search(output)
        if absent.returncode or not absent_marker or tuple(map(int, absent_marker.groups()[:3])) != (1, 0, 0) or \
                int(absent_marker.group(6)) != 0:
            raise SystemExit(f"factory map absence changed the mapless route ({absent.returncode}):\n{output}")
        baseline = tuple(map(int, absent_marker.groups()[3:5]))

        for generation in range(2):
            source = source_tree(root / f"valid-source-{generation}")
            present, output = invoke(source, packet)
            marker = MARKER.search(output)
            if present.returncode or not marker:
                raise SystemExit(f"factory map generation {generation} failed ({present.returncode}):\n{output}")
            empty, _changed, aliases, residual, allocation_baseline, map_frames = map(int, marker.groups())
            if not empty or aliases or map_frames != 1 or (residual, allocation_baseline) != baseline:
                raise SystemExit(f"factory map frame/teardown contract changed: {marker.group(0)}\n{output}")

        missing, output = invoke(source_tree(root / "missing-source"), root / "missing.map")
        if missing.returncode != 3 or "factory map packet unreadable or rejected" not in output or \
                f"residual={baseline[0]} baseline={baseline[1]} owners=0" not in output:
            raise SystemExit(f"factory map missing-input rollback failed ({missing.returncode}):\n{output}")
        rejected, output = invoke(source_tree(root / "malformed-source"), malformed)
        if rejected.returncode != 3 or "owners=0" not in output:
            raise SystemExit(f"factory map malformed-input rollback failed ({rejected.returncode}):\n{output}")
        terrain_failure, output = invoke(source_tree(root / "terrain-failure-source"),
                                         packet, "ZH_M22_FACTORY_FAIL_TERRAIN")
        if terrain_failure.returncode != 3 or "forced original terrain factory failure" not in output or \
                "owners=0" not in output:
            raise SystemExit(f"factory map terrain rollback failed ({terrain_failure.returncode}):\n{output}")
        device_failure, output = invoke(source_tree(root / "device-failure-source"),
                                        packet, "ZH_M22_FACTORY_FAIL_DEVICE")
        if device_failure.returncode != 3 or "forced original graphics device failure" not in output or \
                "owners=0" not in output:
            raise SystemExit(f"factory map device rollback failed ({device_failure.returncode}):\n{output}")

    print("original factory map: present/absent/rollback/reentry/zero-ownership ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
