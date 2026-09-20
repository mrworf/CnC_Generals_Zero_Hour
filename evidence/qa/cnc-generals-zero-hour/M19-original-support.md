# M19 original-support evidence

Evidence grade is stated per result. Retail data, GPU/display access and the read-only retail symlink are outside this milestone and were not accessed.

## Slice 01 — source classification and identity contract

Grade: source-inventory assurance (not original-source runtime).

The canonical CMake inventory resolves to 3,293 unique source paths and the generated table classifies each exactly once. At M19 closure, ten original translation units are production-compiled, 2,343 paths are deliberately excluded/deferred to their named source-integration owner, 857 are tool-only, and 83 are platform-replaced. The production set is the three EAC RefPack providers plus seven dependency-closed WWLib/WWMath/WWSaveLoad providers. Bootstrap registrations and the Alpha/Bravo simulation remain fixture/component evidence only and cannot satisfy the production-provider policy.

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

### Slice 03 — original WWLib/WWMath/WWSaveLoad support

Grade: original-source compile/link and bounded runtime execution.

`zh_wwsupport` compiles the repository-owned `FastAllocator.cpp`, `chunkio.cpp`, `gcd_lcm.cpp`, `nstrdup.cpp`, `ramfile.cpp`, `tri.cpp`, and `pointerremap.cpp`; the generic bootstrap source is absent. The project-owned probe exercises original GCD/LCM, string duplication, allocator allocation/free teardown, triangle containment, RAM-backed chunk I/O, and pointer remapping. The chunk contract is fixed to the x86-compatible 32-bit little-endian header and rejects a truncated header. The adapter also verifies nearest-even numeric conversion and two-byte `char16_t` UTF-16 round-trip without `-fshort-wchar`. Missing remaps become null rather than an invented provider result.

Narrow portability changes preserve the original algorithms while replacing compiler/platform assumptions: fixed-width legacy integer aliases, GCC/Clang calling-convention and inline spellings, portable locking, dependent-base lookup, case-correct includes, and non-MSVC allocation/debug declarations. The original allocator reports zero live allocations after teardown. Compile commands, the final link map, and the runtime witness jointly identify every required provider.

Validation:

- `cmake --build build/linux-gcc-debug` and `ctest --test-dir build/linux-gcc-debug --output-on-failure` — passed, 62/62.
- `cmake --build build/linux-clang-debug` and `ctest --test-dir build/linux-clang-debug --output-on-failure` — passed, 62/62.
- `cmake --build build/linux-gcc-release` and `ctest --test-dir build/linux-gcc-release --output-on-failure` — passed, 62/62.
- `cmake --build build/linux-clang-release` and `ctest --test-dir build/linux-clang-release --output-on-failure` — passed, 62/62.
- `cmake --build build/linux-clang-sanitized --target original_wwsupport_tests original_compression_tests -j2` followed by `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/linux-clang-sanitized -L original-support --output-on-failure` — passed, 7/7.
- `python3 tools/original_source_classification.py --check` — passed, 3,293 paths covered exactly once.

The identity suite includes a negative provider-removal case that fails before runtime when a required source is absent from compiler commands; production classification also rejects bootstrap, fixture, or toy providers. The complete regression exposed a pre-existing M11 harness race in which a host could accept, receive the command, and observe disconnect in one pump before printing its accepted-peer witness. The narrow repair emits that already-established peer identity before validating the command and disconnect; both loopback tests and every full preset suite pass afterward.

### Remaining production closure

M19's support boundary is complete, but whole-game production closure is intentionally incomplete. Original GameEngine and device providers remain owned by M20–M25; fixture/component results from M0–M14 do not establish their integration. No retail content, retail path, retail hash, GPU/display session, or persistent user data was used by this milestone.
