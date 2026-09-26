#!/usr/bin/env python3
"""Generated transaction shaders are not shipping/installed shader families."""
from pathlib import Path
import argparse


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-root", type=Path, required=True)
    args = parser.parse_args()
    root = args.build_root.resolve()
    shipping = root / "generated/bgfx"
    fixture = root / "generated/bgfx-transaction"
    names = {"transaction_alias.vert", "transaction_alias.frag", "transaction_distinct.vert"}
    assert fixture != shipping and shipping not in fixture.parents, "fixture root overlaps shipping"
    assert {item.name for item in fixture.glob("*.bin")} == {name + ".bin" for name in names}
    for name in names:
        for suffix in ("bin", "pre", "lower", "json", "layout"):
            assert (fixture / f"{name}.{suffix}").is_file(), "incomplete generated fixture"
    for name in ("video.vert", "video.frag"):
        for suffix in ("bin", "json", "layout"):
            path = Path("renderer") / f"{name}.{suffix}"
            assert (fixture / path).read_bytes() == (shipping / path).read_bytes(), \
                "fixture baseline/present provider differs from current shipping bytes"
    assert not any(item.name.removesuffix(".bin") in names for item in shipping.rglob("*.bin")), \
        "transaction shader escaped into strict shipping enumeration"
    install = (root / "cmake_install.cmake").read_text(encoding="utf-8")
    assert "bgfx-transaction" not in install and not any(name in install for name in names), \
        "generated fixture is installed as shipping content"
    print("transaction shader scope: separate generated, shipping and install owners")


if __name__ == "__main__":
    main()
