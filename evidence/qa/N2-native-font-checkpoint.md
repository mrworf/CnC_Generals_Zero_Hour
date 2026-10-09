# N2 real CPU font backing checkpoint

Existing N2 plan01/slice03; parent `fc457ec3`. This is not milestone or slice
acceptance. Framework sources, retail assets and recovery storage are untouched.

Frozen canonical636:
`11635dfb940da566597f5739fe2f7a65fdf0fc3b1efb80eb84ae943b2b389841`.
Toolchain-inclusive643:
`b6cad4cdfeeaf8a80f8ccf2cd671aa666e35b843bf5dbe8d9def96945d07287e`.
Hash recipe is the prior text checkpoint recipe, including new untracked source.

## Executed scope

NativeFontLibrary uses stock public FreeType/Fontconfig APIs, explicit private
configuration and original read-only VFS font bytes. Metadata faces retain their
bytes; matching obtains independent faces. Font-set patterns retire before their
borrowed metadata, faces before buffers/library. Whole derived destruction retires
the original pooled font list while native backing and derived dispatch are live.
No global font configuration replacement or FcFini occurs.

`tests/original/native_font.cpp` generates two BDF fonts and malformed input.
It passes explicit backend configuration through actual GlobalLanguage fields;
it does **not** claim Language.ini parsing/startup. Actual VFS and stock font
metadata/matching/rasterization prove local faces, source primary below256 versus
alternate above255, exact generated mono coverage, owned copied pixels, W extra
spacing, cache identity, initialized metrics, idempotent init/reset/reuse and
three whole-owner lifetimes. Host matching separately exercises Arial, bold and
Generals outline sizing without printing host paths. Its narrower advance proves
that requested width is applied, not a Windows/GDI metric or raster oracle.

Missing/malformed data and invalid font/codepoint/foreign owner inputs reject;
failed init permits corrected same-owner retry. An explicit capacity8 GameFont
pool proves exhaustion before native acquisition and retry after one unit returns.
Standard game allocation and pool counts retire exactly after every repeated
owner and whole memory manager. Native malloc ownership is independently checked
by unsuppressed sanitizer leak detection, not inferred from game counters.

Complete acquisition terminals (normal host; every ordinal plus exact terminal,
every corrected retry retained, repeated3): native init7, matched admission6,
glyph acquisition1; standard init13, admission2, copied glyph1. Each native
observer is a game-owned post-acquisition hook behind a guard, not library
interposition. Discovery retires before baselines. No private selector/address
or asset derivative is retained.

## Focused results on frozen source

Command per configuration:
`cmake --build build/original-core-<configuration> --target original_native_font_fixture -j 4`
then `ctest --test-dir build/original-core-<configuration> -R '^original_native_font_' --output-on-failure`.

| Configuration | Focused6 | Wall time |
| --- | --- | --- |
| GCC | PASS | 2.18s |
| GCC ASan/UBSan/LSan | PASS | 6.81s |
| Clang ASan/UBSan/LSan | PASS | 6.17s |

Both sanitizer fixtures retain detect_leaks=1, halt_on_error=1, UBSan stack
traces and unchanged60s test timeout. No suppression or vendor modification.
Installed dependencies are the same stock FreeType2.14.3/Fontconfig2.18.3 route
qualified in the preceding text checkpoint; no dependency revision was changed.

Development failures retained: initial backend compile omitted Common/INI.h for
the original file error code, corrected include. A GCC sanitizer build overlapped
addition of AllocationFault.cpp and failed fixture linkage; after source/CMake
were frozen the final focused matrix passed. The initial malformed fixture
remounted a changed VFS after the baseline and legitimately retained new index
storage; move creation/mount into the complete initial fixture, rather than waive
counts or change production behavior. Corrected focused6 passes all3.

## Broader matrix results

All3 full configured builds passed. GCC's prior related153 expression plus
`native_font` passed159/159 in148.46s on frozen source (execution97206 terminal0).
Clang sanitizer related159 passed159/159 in393.10s (execution25971 terminal0).
GCC sanitizer related159 passed159/159 in642.98s (execution90193 terminal0).
All3 full builds/focused6/related159 therefore PASS on the frozen source.
Commands/logs:
`/tmp/n2-native-font-{gcc,gcc-sanitize,clang-sanitize}-related-{build,test}.log`.
All original execution handles are now verified terminal0. No live validation
job remains and no full milestone matrix has been run. Commit the verified
supporting checkpoint without declaring slice03 or N2 complete.
Current configured test count is349 (CTestTestfile add_test census), not the
preceding343. The passed159 subset does not establish full349 acceptance.

## Remaining gates

Actual original native display strings, wrapping/hotkey/char-position metrics,
UI/client/root admission and retirement, popup/snapshot cohorts, camera phase,
original startup and deterministic GameLogic frames remain in the existing plan.
Exact intended visual/locale coverage and supplied-retail font behavior are not
accepted by generated glyphs. Generals average-width scaling uses public OS/2
average width; comparison with GDI semantics is still required. Point0 currently
rejects rather than inventing GDI default-size behavior; inspect legitimate
source/data consumers before integrating, do not silently accept a compatibility
change. Negative bitmap pitch follows the public FreeType offset-to-next-row
contract, but generated fixtures currently exercise positive pitch. Physical
rendering and whole-game parity remain later acceptance gates.
