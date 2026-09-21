"""Compare the canonical original draw-class layout in mutually exclusive configs."""

import argparse
import subprocess


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--schema", required=True)
    parser.add_argument("--full", required=True)
    args = parser.parse_args()
    schema = subprocess.check_output([args.schema], text=True)
    full = subprocess.check_output([args.full], text=True)
    if schema != full:
        raise SystemExit(f"M20/M21 vs M22 draw-class ABI mismatch:\nschema:\n{schema}\nfull:\n{full}")
    if len(full.strip().splitlines()) != 10:
        raise SystemExit("expected ten original draw-class layouts")
    print("M20/M21 schema and M22 full original ten-class size/alignment ABI agree")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
