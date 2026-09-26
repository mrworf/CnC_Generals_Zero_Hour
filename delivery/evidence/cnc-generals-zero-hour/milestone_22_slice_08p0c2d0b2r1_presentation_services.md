# M22 slice 08P0C2D0B2R1: generated presentation filesystem services

Parent: `a63668d7a7f0100c76da7c9e38a15a01bd75717a`.
Status: accepted; presentation service category is corrected. Unrelated R2
texture-enum categories remain explicitly qualified until R2; B2B remains closed.

## Ownership and source classification

The historical null FileSystem category came from the generated presentation
fixture, not the accepted native submission owner. Shipping GameEngine already
constructs FileSystem/local/archive before GlobalData. The fixture now admits
and owns real FileSystem and PosixLocalFileSystem providers, rooted solely in
its mkdtemp generated directory, before checked GlobalData construction. No
source CRC algorithm, null-this fallback, private file, archive, native draw,
factory or serialized/class ABI change occurs.

Admission rejects absent FileSystem/local and overlapping providers before
GlobalData construction or service publication. Failed overlap leaves borrowed
or already admitted identities unchanged. GlobalData is destroyed before the
longer-lived services are removed; no-throw reverse scope cleanup also handles
an injected generated consumer exception. The archive remains absent by
declared generated baseline; source FileSystem::openFile correctly handles
missing script files through the valid generated local provider.

Two isolated service/GlobalData generations assert exact prior global identity,
missing scripts/empty CRC, unchanged admitted ownership after overlap, and exact
memory-pool live allocation baseline after teardown. Additional controls cover
borrowed FileSystem-only and local-only providers, missing counterpart admission,
consumer failure unwind and clean retry. The existing two-generation presentation
route remains unchanged and now explicitly removes all generated services at
its final boundary.

Frozen fixture SHA256:
`98509f17acd205a23a56451a47c61986f7a04bc1f75bd9b0596c50db59a86279`.
Only this fixture is executable-changing; ledger records the new real provider
consumer/lifetime. No global allocator, runtime service semantics or diagnostic
suppression is added. The unrelated renderer test remains unchanged/unstaged.

## Focused acceptance

Regex:
`^(original_w3d_presentation|original_w3d_presentation_identity|original_w3d_presentation_provider_removal)$`.

| Configuration | Result | Seconds |
| --- | --- | ---: |
| GCC Debug | 3/3 | 1.37 |
| Clang Debug | 3/3 | 1.49 |
| GCC ASan+UBSan, detect_leaks=0 | 3/3 | 3.74 |
| Clang ASan+UBSan, detect_leaks=0 | 3/3 | 3.05 |

Complete selected output is scanned for nonfatal UBSan as well as fatal findings.
Both final selections have no runtime-error/Sanitizer category. Exact strict
host LSan uses `ASAN_OPTIONS=detect_leaks=1`, the same three CPU/identity tests,
no UBSan override and host escalation: GCC 3/3 in 3.97s, Clang 3/3 in 3.15s.

## Complete final gates

All six complete builds pass on the frozen fixture. Clang sanitizer target and
complete builds recover the existing premature Ninja log (569/810 work items)
and finish without an actionable error; this is not a source/test failure.
No heavy generated acceptance runs
concurrently with rebuild load. Historical texture address/profile enum
categories remain explicitly owned by R2, not accepted as globally clean or
suppressed. R2 requires a clean complete matrix before B2B opens.

Exact validation-enabled native host controls pass: GCC display/bootstrap/map
3/3 in 5.38s and Clang display/map 2/2 in 4.28s. Six exact serial host LAN
selections (`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -L lan
-j1 --output-on-failure`) pass 4/4 in usual preset order: 1.72s, 1.71s, 1.71s,
1.72s, 1.83s, 1.79s. Ledger, public-header and whitespace checks pass; final exact
stage review completes before commit. Six serial complete canonical selections
finish after rebuild load clears, with complete-log category audits at every
configuration boundary.

Command: `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-LE 'gpu|lan|retail' --output-on-failure`.

| Configuration | Result | Total seconds | Terrain seconds |
| --- | --- | ---: | ---: |
| GCC Debug | 272/272 | 278.21 | 59.89 |
| Clang Debug | 272/272 | 263.41 | 59.90 |
| GCC Release | 272/272 | 147.81 | 24.77 |
| Clang Release | 272/272 | 99.87 | 23.69 |
| GCC ASan+UBSan | 272/272; R2 category below | 874.04 | 266.07 |
| Clang ASan+UBSan | 272/272; R2 categories below | 691.64 | 222.68 |

GCC complete-log audit confirms the repaired presentation null-FileSystem
category is absent. The sole nonfatal report is unchanged CTest 251 texture
decision value 9 at texturefilter.h:112, explicitly owned by the durable R2
plan. No R1/native submission finding occurs. This is qualified successful
canonical execution, not a claim that the unrelated texture fixture is clean.

Clang's full audit also contains no null FileSystem/GlobalData report. Its only
categories are unchanged CTest 276 texture-decision address value 9 at
texturefilter.h:114/112 and profile value 99 at texturefilter.cpp:133/134.
The unchanged source/fixture and non-bgfx linkage proof in A applies exactly;
this fixture-only correction cannot reach that separate process/owner. No new
sanitizer report occurs. R2 is the dependency-next mandatory corrective owner,
not a suppression or a globally clean-suite claim.

All six canonical suites finish 272/272; four native complete logs have no
sanitizer category, both sanitizer logs prove R1's category eliminated. Focused
and strict host LSan are fully clean on the corrected route. Exact source hash,
ledger, public headers, whitespace and owned-diff review pass. Commit only this
independent service lifetime and its artifacts; preserve the renderer diagnostic.
