# M22 slice 08T0R1: declared sampled-mip range

Status: accepted in this slice commit after all required gates and exact audits.
Implementation provenance: exact subject
`delivery: M22 08T0R1 bound declared sampled mip range`.
Plan checkpoint: `b49183ff2b2f5434290ad409ab11b142ae29cddd`.
Implementation authority: one backend corrective leaf before08T0A; camera and
active08 executable/test changes remain preserved and unstaged.

## Frozen implementation and ownership

Ordinary binding exposes firstMip0/count equal to the logical descriptor;
unknown non-target backing rejects before draw/native candidate publication.
Render targets retain write-derived readiness. No mip generation, filter/camera
policy or format admission changed. Native layer-range defaults are preserved.
Deferred capture freezes the exact range with the accepted native lease, and
alias equality includes it. The opt-in native manifest adds first/count with
legacy full-remaining defaults, overflow-safe early and copied-model validation,
and existing subresource replay. No API-thread replay allocation is introduced.
Native worker image-view creation remains outside reversible API-thread ownership.

| Owned source/test | Frozen SHA256 |
|---|---|
| src/renderer/bgfx_device.cpp | 0ced8638244b4228912c65be34028b76027c5e5db6b814b5e7f307910240c466 |
| src/renderer/bgfx_transaction_state.h | feaa38a6e4ceb035ee4de28afdfe0dd7f27879cc9801099f9a2556d8b6770415 |
| tests/renderer/test_bgfx_mip_texture.cpp | d0e7ccee90d9322330d4efa12b98e8d53201b9299313f0b1b1afa0e7d3c91752 |
| tests/renderer/test_bgfx_mip_range.cpp | bccbfe13626d7f7fad0cec7ea87702e6ba423d1f189a67e829d6bf8c8c0e73fa |
| tests/renderer/test_bgfx_submission_reservation.cpp | a620d25b05e787a346cf4ca7c1a1ccb6cf5895d01893f995bbbe1f03397e75b7 |
| tests/renderer/test_bgfx_transaction_resource.cpp | 37489e38602321f97c85e6bec4c9b6778b13bba52455a76e0168def6b4652bac |
| tests/renderer/test_bgfx_transaction.cpp | 914e2345ba04bba9e8d532dbf2b37688b614c0d99976db57f7337a7ffe872002 |
| third_party/bgfx_bounded_submission.patch | f69810e479fdd8e431511ceaf24596fa94e1fe9aefe92a8463e72c5c1e69a555 |

## Completed focused controls

Frozen GCC/Clang debug exact8 CPU focus:8/8 each,1.34/1.31s. Selection is exactly
the plan's range/reservation/resource/journal/shader-scope and three source
texture identity/provider controls. Frozen validation-enabled host physical
exact4:4/4 each,3.89/3.89s. Both use the approved generated fixtures, no camera
override or private data. These component results do not replace post-six-build
physical reruns or the required final acceptance matrix.

- Ordinary pixel proof:72 cases across two generations, RGBA8/BGRA8/BGR5A1,
  square/rectangular/singleton/partial/full chains and nearest/linear mip policy.
  Extreme minification sees only the last declared authored green level;
 60 unknown-last-level negatives reject, with total resource teardown zero.
- Native immutable manifest: range input mutated only after the complete private
  copy; accepted pixels remain unchanged. Empty/count-bound+1/first-bound+1/
  overflow and mixed early-valid/late-invalid ranges reject before reservation
  allocation, receipt change or target touch. Exact firstMip1 and2 views select
  different authored colors; alternating range views reuse only full equality.
  Existing generated hash-collision proof and zero API replay allocation pass.
- COW owner: full and partial padded mip rows in all three formats; partial
  native-publication/early-operation/late-operation faults preserve accepted
  source identity and all known mip pixels, then abort/drain/retry succeeds.
- Journal owner:12 partial-chain cases across two generations and both policies;
  unknown exposed mip rejects with exact packet/native/target baseline, captured
  range survives COW/removal/different-range logical handle reuse, submission
  failure retains the same batch, retry emits once with ordinary-identical pixels
  and no pending retirement. Existing lifecycle/sibling controls remain active.

## Classified superseded setup evidence

An initial CTest GPU selection found no tests because ordinary presets disable
GPU registration; it is not evidence. Existing registrations are enabled in the
build cache for the exact approved physical selection. Two expansion failures
were fixture-only: native cache receipt includes its empty binding entry
(Before0/After3 for two distinct sampled ranges), and a deferred frame requires
the existing final present/CompleteFrame boundary. The range lease proof moved
to the existing journal/presentation fixture, not a new owner or relaxed commit.
A subsequent unknown-mip negative compared against the first red frame rather
than the immediately preceding accepted green frame in the second policy case;
read-only localization proved the exact case, and the witness now snapshots the
then-current prior pixels. No production behavior or assertion workload was
weakened; all are superseded by the frozen passing controls above.

## Preservation and final acceptance

All28 nonoverlapping preserved active08/T0A/renderer paths matched the pre-R1 byte
baseline at the initial freeze. The later approved T0A identity correction changes
only its two private parameter lines, excluded from R1; the other26 remain exact.
Root CMake gains only the isolated R1 test registration. Parent approved
exactly eight hash-only T0A ledger refreshes to unchanged trial bytes; they are
preserved bookkeeping, excluded from R1 staging. R1 owns only the existing native
patch ledger row. Updated whole-worktree ledger snapshot:
`1a536e5778871ef4f45613e2d159219224ea8b20aaebc8c81f69e8060a6f4272`.
After the approved T0A correction, its two existing ledger row hashes alone are
refreshed, also excluded; current whole-worktree ledger snapshot is
`3fed7348d136e43fb7edacb176c9dffd673b5fcbe3b683fb25a65fc226463e44`.
Ledger check passes. No retail content, metadata or hashes were inspected for R1.

Exact applied native submission diff equals the40445-byte reviewed artifact;
non-patch source whitespace checks pass. Offline runtime Release bootstrap,
separate pinned Debug runtime build and offline shader compiler bootstrap pass.
Six complete GCC/Clang debug/release/sanitizer builds pass; all six manifests
contain the exact8 registered focus IDs. Independent Debug relink/physical proof
passes under validation and retains zero API-thread replay allocations.
Fresh post-build exact8 focus passes GCC/Clang debug and both sanitizers;
strict8 passes both sanitizer builds under host escalation with exactly
`ASAN_OPTIONS=detect_leaks=1`, no strict UBSan override. All sanitizer/validation
category audits are clean, not inferred from exit status.

| Configuration | Exact8 CPU focus | Exact4 physical | Strict8 |
|---|---|---|---|
| GCC debug | 8/8,1.31s | 4/4,3.93s | not sanitizer |
| Clang debug | 8/8,1.30s | 4/4,3.83s | not sanitizer |
| GCC sanitizer | 8/8,1.85s | 4/4,27.71s | 8/8,1.95s |
| Clang sanitizer | 8/8,1.67s | 4/4,26.17s | 8/8,1.76s |

Established isolated host Vulkan passes GCC3/3 and Clang2/2; minimal8 passes
both native toolchains; serial LAN passes4/4 in all six configurations. Ledger
and frozen source checks pass after these gates. Six fresh serial297-control
canonicals run from zero on the exact unchanged frozen composition. GCC debug
passes297/297 in674.12s and Clang debug passes297/297 in673.41s, both with
clean category audits. GCC release passes297/297 in327.05s with a clean category
audit; Clang release passes297/297 in266.52s with a clean category audit.
GCC sanitizer passes297/297 in2561.66s with no sanitizer categories. Its scene
attachment/tree draw/decal/prop witnesses complete in267.76/84.35/131.99/1143.09s.
Clang sanitizer is active. Its preserved in-progress T0A camera-startup CPU
control214 fails: generation0 returns0 and the exact completion marker is
present, but UBSan identifies a misaligned HeightMapRenderObjClass upcast at
`w3d_camera_startup_cpu.inc:150`. Independent unchanged generated-wrapper
reproduction confirms the category. The malformed-provider negative passes a
raw invalid derived pointer into the protected base-pointer identity peek,
causing an implicit upcast before rejection. This is T0A admission code, not
R1 sampled-range behavior; localization changed no source/assertion. Approved
T0A-local disposition changes only the protected terrain identity parameter to
const void*, so raw identities reject before any typed conversion. After this
suite completes296/297 in2344.45s with only control214 failed, exactly the two
private parameter lines are corrected outside R1 ownership. Six targeted
full-probe relinks and unchanged registered camera controls pass, with clean
category audits: GCC/Clang debug0.50/0.51s, release0.39/0.37s, and
sanitizers3.67/3.23s. All181 static archives across six builds are byte-identical
to pre-correction; all36 selected R1 binary hashes and eight frozen R1 source
hashes also match. Only the admitted helper/full-draw OBJECT and full-probe
bytes change, with no fields/virtuals or assertion/workload change.
Under the approved proportional boundary, the five clean canonicals and all
unchanged focused/strict/physical/common gates are preserved by that identity
proof. The fresh full Clang sanitizer canonical passes297/297 in2373.66s with
the same exact selection/environment and a clean complete category audit; the
failed run is superseded diagnostic evidence only. Final frozen source, selected
binary, helper/full-probe, archive, ledger, whitespace/privacy and staged checks
pass. Neither the T0A correction nor its ledger rows are included in this commit.

| Final canonical | Result | Wall time |
|---|---|---|
| GCC debug | 297/297, clean | 674.12s |
| Clang debug | 297/297, clean | 673.41s |
| GCC release | 297/297, clean | 327.05s |
| Clang release | 297/297, clean | 266.52s |
| GCC sanitizer | 297/297, category-clean | 2561.66s |
| Clang sanitizer, corrected rerun | 297/297, category-clean | 2373.66s |

Stage only the eight frozen R1 source/test/patch files, AGENTS, renderer contract,
R1 plan/evidence/index, and the isolated four-line CMake registration/native-patch
ledger row. All31 original preserved active08/T0A/renderer paths remain dirty and
excluded; their approved two-line T0A correction/hash refresh is preserved too.
No private corpus was read for this leaf. Retail continuation remains gated on
independent T0A acceptance and separate T0 aggregate closure.
No completion claim or T0A acceptance is implied by this checkpoint.

Native public layout inspection of GCC and Clang binaries agrees: sampled
manifest size12 bytes, sampler0/texture2/flags4/stage8/firstMip9/numMips10.
Existing fields keep their offsets; new range fields occupy prior padding, with
one remaining padding byte. All public consumers still require the same amended
header/native rebuild; no engine class/vtable/serialized layout changes occur.
The independent Debug fixture was relinked with BX_CONFIG_DEBUG=1 and its actual
DT_NEEDED names `libbgfx-shared-libDebug.so`; ordinary fixtures retain the exact
Release DT_NEEDED. No LD_LIBRARY_PATH substitution is counted as Debug proof.
AGENTS records the general declared-range/colored-mip lesson only.

Exact generated Debug fixture commands, derived from the normal Ninja target
and executed from `build/linux-gcc-debug`; only debug definition, output/dependency
paths and native library differ:

```sh
/usr/bin/g++ -DBX_CONFIG_DEBUG=1 -DZH_BGFX_SHADER_DIR=\"/home/ha/projects/CnC_Generals_Zero_Hour/build/linux-gcc-debug/generated/bgfx\" -I/home/ha/projects/CnC_Generals_Zero_Hour/include -isystem /usr/include/freetype2 -isystem /home/ha/projects/CnC_Generals_Zero_Hour/build/bgfx-toolchain/source/bgfx/include -isystem /home/ha/projects/CnC_Generals_Zero_Hour/build/bgfx-toolchain/source/bx/include -isystem /home/ha/projects/CnC_Generals_Zero_Hour/build/bgfx-toolchain/source/bimg/include -g -std=c++20 -fno-fast-math -ffp-contract=off -MD -MT /tmp/m22_t0r1_native_debug.o -MF /tmp/m22_t0r1_native_debug.o.d -o /tmp/m22_t0r1_native_debug.o -c /home/ha/projects/CnC_Generals_Zero_Hour/tests/renderer/test_bgfx_submission_reservation.cpp
/usr/bin/g++ -g -Wl,--dependency-file=/tmp/m22_t0r1_native_debug_link.d /tmp/m22_t0r1_native_debug.o -o renderer_bgfx_submission_reservation_native_debug_tests  -Wl,-rpath,/home/ha/projects/CnC_Generals_Zero_Hour/build/bgfx-toolchain/source/bgfx/.build/linux64_gcc/bin  libzh_renderer_bgfx.a  libzh_renderer_sdl_gpu.a  /usr/lib/libSDL3.so  libzh_wwshade.a  libzh_w3d.a  libzh_wwsupport.a  /usr/lib/libfreetype.so  /usr/lib/libfontconfig.so  /home/ha/projects/CnC_Generals_Zero_Hour/build/bgfx-toolchain/source/bgfx/.build/linux64_gcc/bin/libbgfx-shared-libDebug.so
```
