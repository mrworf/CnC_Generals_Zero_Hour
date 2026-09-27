# M22 slice 08Q0R1: strong cloned-render graph evidence

Status: complete; final acceptance and exact staged-path review passed.
Plan checkpoint: `1da23ed765d5d5b1f485801ee0df204299eb9600`.
Scope: public original WW3D cloned-render construction only. Q0 house-color and
active08 trial changes remain separate and unstaged; no retail input is used.

## Implemented ownership boundary

Actual `MeshClass::Make_Unique(true)` retains the original Model until the entire
candidate geometry/cull-tree/descriptor/material/mapper/remapper graph completes.
Constructor-local guards clean partial arrays and exact acquired refs; raw model
members publish nonthrowingly only after complete admission. MaterialInfo storage
is built offside with authored ten-slot destination growth; ordinary vector,
hash, ShareBuffer and assignment behavior outside clone scopes is unchanged.
Missing alternate-to-default remap arrays use the authored setters, with complete
lookup preflight, checked bytes and private-candidate unwind. Random mapper's
owner-local stream is restored on any failed attempt and advances as authored on
success; GameClient/GameLogic/audio RNGs are not involved.

All added class friendship/declarations are Linux-wide, not a CPU macro fork.
No fields, virtuals, serialization or class layout change. Windows successful
branches retain the existing construction/clone behavior. Internal once-only
fault access is generated-friend-only, with no environment/retail selector.

## Generated witness

The independent witness uses real Make_Unique and concrete material/mapper copy
routes, not Q0's uncommitted recolor fixture. Two generations cover 20 basic and
73 extended graph injection boundaries, all 19 concrete mapper families through
both Clone and copy construction, optional copied cull trees, four passes/two
stages, shared aliases, alternate descriptors and absent destination-array
creation. Every rejected ordinal checks immediate raw/pool/source-ref baselines,
original Model identity and RNG sequence, then retries on the same instance.
Success checks distinct material/mapper/tree identities, unchanged shared texture
and geometry identities/bytes, and exact tree owner rebinding. Generation teardown
returns to the entry raw/pool baseline. Empty and malformed owners, pass limits,
missing remap nodes, and exact 64MiB/bound+1 admission are covered.

Fault seams precede owner allocation/clone/ref-publication boundaries. Generic
ShareBuffer performs one array allocation before its nonthrowing element/ref loop;
its templates are intentionally unchanged. Actual allocator failure inside that
single allocation is governed by the enclosing constructor guard, rather than a
new generic allocator injection mechanism. The allocation-free publication and
cleanup are reviewed directly and exercised under both sanitizer toolchains.

The initial development SIG11 was localized to the generated default Mesh having
its authored null Model before test setup, before entry into Make_Unique. The
fixture now explicitly installs its owned generated Model. That superseded
development failure is not final acceptance evidence.

## Frozen composition and gates

The initial executable source and witness freeze is superseded by the approved
Q0 finite-scale correction below. All six builds validate the R1+Q0+active08
workspace composition; only R1-owned
files/hunks will be committed, without changing executable bytes. This does not
accept or close Q0 or slice08. CMake and Vector/ledger overlap is exact-hunk staged.

Exact nine-control focus:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_clone_graph|original_w3d_clone_graph_identity|original_w3d_clone_graph_provider_removal|original_w3d_cpu_graph|original_w3d_material_abi_isolation|original_w3d_abi|original_w3d_texture_decisions|original_w3d_presentation_identity|original_w3d_presentation_provider_removal)$' --output-on-failure -j1 -V`.
Strict host LSan uses the same selection with exactly
`ASAN_OPTIONS=detect_leaks=1`, no UBSan override, with host escalation.
Canonical suites use `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset>
-LE 'gpu|lan|retail' --output-on-failure -j1 -V` and run serially.

Superseded diagnostic checkpoints (not final acceptance):

| Configuration | Complete build | Nine-control focus | Seconds |
| --- | --- | --- | --- |
| GCC Debug | pass |9/9;category-clean|3.08|
| Clang Debug | pass |9/9;category-clean|3.58|
| GCC Release | pass |9/9;category-clean|2.61|
| Clang Release | pass |9/9;category-clean|2.35|
| GCC sanitizer | pass |9/9;category-clean|6.42|
| Clang sanitizer | pass |9/9;category-clean|5.54|

Strict host LSan passes GCC 9/9 in 6.72s and Clang 9/9 in 5.91s with the exact
options above, no inherited UBSan override, and complete log category scans clean.
Established native host Vulkan controls pass GCC 3/3 in 5.30s (display owner,
factory bootstrap and map) and Clang 2/2 in 4.49s (display owner and map), with
configured Khronos validation and no validation category. All run after complete
builds under isolated load. Serial host LAN passes 4/4 all six in 1.71–1.83s,
with complete logs category-clean. Canonical results remain pending and will be
recorded only after completion and full log audit. Source ledger validates at
freeze; owned diff and exact staged-path audits remain required before commit.

Frozen independent witness SHA256:
The independent R1 witness/guard remain unchanged. All final gates must rerun
after the approved Q0 correction and composition refreeze.

`60ffb0453d28e61ccbc632fe2075b00b4e51a4cedc002f0dce2957da17cf0ffa`.
Frozen internal guard SHA256:
`b154ec5378852200487c6c5b1c812d3b32657898fd0ce96fb79cac8f6cb919d6`.

## Superseded composition failure and correction

The first GCC Debug canonical run finished 274/284 in 337.41s; the serial queue
stopped before the remaining configurations started. All ten failed shadow,
borrowed-file and bridge variants rejected Q0's newly introduced positive-only
scale preflight before the request could call Make_Unique. Five child categories
were verified through their unchanged generated wrapper commands; the other
five reported the fixed scale category directly. No R1 clone defect was found.
After the approved finite-scale correction, five bridge/borrowed controls passed;
the five shadow variants exposed unconditional Edge admission during pre-device
geometry-only preload. This second Q0 restriction also preceded clone entry.

Architecture-approved Q0-only corrections preserve zero and finite-negative
native scale semantics, rejecting only NaN/±Inf; permit pre-device geometry-only
prototype preparation without an Edge; retain idle/poison/retirement checks if
an Edge exists; and require matching active physical ownership for recolor,
replacement or private-backing residency before geometry/cache mutation.
Existing generated templates remain unchanged. New Q0 controls prove exact
scale bytes, absent/busy rejection residuals, idle retry and cached private
backing rejection followed by fresh-generation replay without input reread.
Affected ten-control GCC focus passes 10/10 in 8.57s, and the eleven-control
Clang correction focus passes 11/11 in 8.71s. The independent R1 source and
witness hashes are unchanged. All prior gate tables above are superseded
diagnostics; final six-build/focus/strict/physical/LAN/canonical gates restart
on the refrozen corrected composition, with no discretionary executable edits.

## Corrected final-source matrix

Six complete builds pass; Clang sanitizer recovered its Ninja dependency-cache
warning by rebuilding all affected tasks. There is no compiler error category.
All runtime gates below run serially after builds, with no concurrent compilation.

| Configuration | Nine-control focus | Seconds |
| --- | --- | --- |
| GCC Debug |9/9;category-clean|3.10|
| Clang Debug |9/9;category-clean|3.19|
| GCC Release |9/9;category-clean|2.59|
| Clang Release |9/9;category-clean|2.11|
| GCC sanitizer |9/9;category-clean|6.19|
| Clang sanitizer |9/9;category-clean|5.54|

The eleven-control correction union (Q0 witness plus the ten affected existing
controls) passes GCC Debug/Clang Debug/GCC sanitizer/Clang sanitizer in
8.56/8.74/35.49/21.99s; complete logs are sanitizer-category-clean.
Strict host LSan exact nine-control focus passes GCC 9/9 in 6.29s and Clang 9/9
in 5.40s. The Q0 correction witness also passes separately under strict host
LSan, 1/1 each in 0.28/0.12s. Options are exactly `ASAN_OPTIONS=detect_leaks=1`,
with no UBSan override. Established native host Vulkan passes GCC3/3 in 5.15s
and Clang2/2 in 4.28s with configured validation and no validation category.
Final serial host LAN passes 4/4 all six in 1.71–1.82s; full logs are
sanitizer-category-clean. All six serial 284-test canonical suites pass from this
corrected freeze; complete failure/timeout/ASan/UBSan/LSan category scans are empty.
Frozen composition hashes match after all gates, and the dependency ledger
validates. The reviewed 24-path staged diff owns only R1 construction, its
independent witness, durable lesson, status/evidence, and the exact CMake/Vector/
ledger hunks. Reverse patch checks prove shared-file hunk identity; staged
whitespace checks pass. All Q0/active08 executable changes, their remaining shared
file hunks, and the unrelated renderer diagnostic remain unstaged. Committing R1
does not change any validated workspace executable byte.

| Final canonical configuration | Result | Seconds |
| --- | --- | --- |
| GCC Debug |284/284;category-clean|336.15|
| Clang Debug |284/284;category-clean|328.78|
| GCC Release |284/284;category-clean|177.15|
| Clang Release |284/284;category-clean|128.86|
| GCC sanitizer |284/284;category-clean|1150.73|
| Clang sanitizer |284/284;category-clean|949.35|
