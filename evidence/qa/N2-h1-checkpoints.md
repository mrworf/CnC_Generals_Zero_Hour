# N2 plan02 slice04 — canonical H1 checkpoints

Source base: `a91ca750` plus this slice. Renderer-off Debug builds. The input
is a generated non-retail H1 map with separate user storage. Three complete
original-engine lifetimes run in one process for each configuration. Every
lifetime sets original RNG seed `0x5A170001` after engine init and before map
start; no fixture rewinds the script cursor. The test observes production
cursor advancement before teardown and compares all three record sequences.

Each lifetime emits 121 records: one after the pending-load update and one
after each of 120 steady `GameEngine::update` calls. `H1R1` is exactly 224
bytes with explicit little-endian 32-bit fields, defined IEEE float words,
and fixed generated ASCII labels. Offsets 0–71 hold magic/version, tick,
source frame, original logic RNG fingerprint, script flag, active side/local
player identity and terrain heights at the two actor sites. Offsets 72–147
and 148–223 hold the persistent and expiring actor respectively: authored
label, source ID, existence, team, position, geometry and retained module
next-call/deletion frame. An absent actor has a fixed zero state for fields
that no longer exist. No C++ memory image, padding, pointer, handle, path,
wall-clock value or unordered-container order enters the codec.

Independent semantic checks remain live in every run: two exact actor IDs,
parsed one-module deletion schedule at source frame 30, source script flag,
one source-frame increment per steady update, persistent actor survival,
deadline-bracketed expiring actor retirement and terrain heights derived from
authored raw 20/24 values and source `MAP_HEIGHT_SCALE`. The canonical digest
is only an evidence identifier; acceptance compares all 81,312 bytes per
configuration, not just hashes.

```sh
cmake --build build/original-core-gcc --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc -R '^original_native_headless_root_h1-records$' --output-on-failure
cmake --build build/original-core-gcc-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc-sanitize -R '^original_native_headless_root_h1-records$' --output-on-failure
cmake --build build/original-core-clang-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-clang-sanitize -R '^original_native_headless_root_h1-records$' --output-on-failure
python3 tools/compare_h1_checkpoints.py \
  build/original-core-gcc/original_native_headless_root_fixture \
  build/original-core-gcc-sanitize/original_native_headless_root_fixture \
  build/original-core-clang-sanitize/original_native_headless_root_fixture
```

Normal GCC, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan each passed their
three-lifetime test. The independent comparator matched all 363 records in
each configuration byte-for-byte and rejected tampered, reordered, truncated
and divergent-seed records. SHA256 of one configuration's 81,312 canonical
bytes: `d9b52c40d02661238631a01d0b5a6977162904d27bd65c39ab90aee814211d1a`.
All 45 related normal native-root, original-world and native-terrain regressions
passed after the checkpoint changes.
Fault terminals, supplied-data startup and the frozen N2 matrix remain open.
