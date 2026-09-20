# M19 original-support evidence

Evidence grade is stated per result. Retail data, GPU/display access and the read-only retail symlink are outside this milestone and were not accessed.

## Slice 01 — source classification and identity contract

Grade: source-inventory assurance (not original-source runtime).

The canonical CMake inventory resolves to 3,293 unique source paths and the generated table classifies each exactly once. After slice 02, three original EAC RefPack translation units are production-compiled, 2,350 paths are deliberately excluded/deferred to their named source-integration owner, 857 are tool-only, and 83 are platform-replaced. Slice 03 promotes the accepted original WW support providers only after compiler, link and runtime proof exists.

Validation:

- `python3 tools/original_source_classification.py --check` — passed, 3,293 paths.
- `python3 -m unittest tests.original_support.test_source_classification` — passed, four positive/negative cases.
- `ctest --test-dir build/linux-gcc-debug -L 'original-support|source-identity' --output-on-failure` — passed, 2/2.

The negative controls reject a missing inventory policy and production providers whose path identifies bootstrap, toy or fixture code. The committed table contains repository-relative source names only; it includes no retail path, bytes or hashes.

## Production closure

### Slice 02 — original EAC RefPack

Grade: original-source compile/link and bounded runtime execution.

`zh_compression` compiles `refabout.cpp`, `refencode.cpp`, and `refdecode.cpp` from the repository-owned Zero Hour source tree. The project-owned harness passes owned bytes through the original encoder and decoder, emits metadata allocated by the original `REF_about`, and checks the output against the independently bounded native decoder. Truncation and configured-size limits fail before the historically unbounded decoder can access malformed input. The generic bootstrap source is absent from this target.

Validation:

- GCC Debug focused original-support CTest — passed, 5/5.
- Clang Debug focused original-support CTest — passed, 5/5.
- Clang ASan/UBSan original compression runtime — passed, 1/1 (`ASAN_OPTIONS=detect_leaks=0`, matching the repository's restricted ptrace environment).
- Compile-command + final-link-map + runtime witness — passed for all three original providers.
- Missing-provider negative identity control — passed by rejecting the absent provider.

### Remaining production closure

Incomplete by design at this checkpoint. Original WW support compiler/link/runtime evidence follows in slice 03. Original GameEngine and device providers remain owned by M20–M25; fixture/component results from M0–M14 do not establish their integration.
