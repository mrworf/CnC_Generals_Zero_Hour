#!/usr/bin/env python3
"""Generated-only audit wrapper acceptance; no supplied assets are accessed."""
import argparse
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", required=True)
    args = parser.parse_args()
    wrapper = Path(__file__).resolve().parents[2] / "tools/run_data_audit.py"
    with tempfile.TemporaryDirectory(prefix="zh-public-audit-") as directory:
        root = Path(directory) / "input"
        root.mkdir()
        def run(binary):
            return subprocess.run([sys.executable, str(wrapper), "--binary", str(binary),
                                   "--data-root", str(root)], capture_output=True, text=True)
        incomplete = run(args.binary)
        assert incomplete.returncode != 0 and "status=INCOMPLETE" in incomplete.stdout
        assert "INPUT_INTEGRITY=UNCHANGED" in incomplete.stdout and not incomplete.stderr
        for name in ("Data/INI/GameData.ini", "Data/INI/Object/item.ini", "Art/W3D/item.w3d", "Maps/item.map"):
            file = root / name
            file.parent.mkdir(parents=True, exist_ok=True)
            file.write_bytes(b"public generated presence fixture")
        catalog = struct.pack("<6I", 0x43534620, 3, 1, 1, 0, 0)
        catalog += struct.pack("<3I", 0x4c424c20, 1, 5) + b"LABEL"
        catalog += struct.pack("<2IH", 0x53545220, 1, (~ord("X")) & 0xffff)
        file = root / "Data/English/Generals.csf"
        file.parent.mkdir(parents=True)
        file.write_bytes(catalog)
        success = run(args.binary)
        assert success.returncode == 0 and "stage=4 mask=63 status=PASS" in success.stdout
        assert "INPUT_INTEGRITY=UNCHANGED" in success.stdout and not success.stderr
        # A deliberately mutating fake runner is confined to this generated root.
        fake = Path(directory) / "fake"
        fake.write_text("#!/usr/bin/env python3\nimport pathlib,sys\n"
                        "(pathlib.Path(sys.argv[1])/'changed').write_bytes(b'public')\n"
                        "print('READ_ONLY_DATA_AUDIT stage=4 mask=63 status=PASS')\n")
        fake.chmod(0o700)
        changed = run(fake)
        assert changed.returncode != 0 and "INPUT_INTEGRITY=CHANGED" in changed.stdout
        fake.write_text("#!/usr/bin/env python3\nprint('private-looking untrusted output')\n")
        untrusted = run(fake)
        assert untrusted.returncode != 0 and "status=OUTPUT_REJECTED" in untrusted.stdout
        assert "private-looking" not in untrusted.stdout and not untrusted.stderr
        absent = run(Path(directory) / "absent")
        assert absent.returncode != 0 and "status=EXECUTION_REJECTED" in absent.stdout
        assert "INPUT_INTEGRITY=UNCHANGED" in absent.stdout
    print("PASS: generated audit wrapper status, rejection, redaction and integrity")


if __name__ == "__main__":
    main()
