# M22 slice 08P0C2D0B2R2: defined texture-filter tuple admission

Parent: `05d00c519013981345987d63f7bd42542c2aa15c`.
Status: accepted; independent corrective R1/R2 clean matrix closes historical
sanitizer categories before B2B.

## Source contract and layout

Linux FilterType, TextureFilterMode and TxtAddrMode use fixed unsigned underlying
representations under the platform macro, not an optional archive macro. All
raw unsigned inputs now have defined enum representations but unsupported
semantics reject. Windows declarations/enumerators/member order remain unchanged;
no virtual/serialized field or retail ABI changes. Six read-only compile controls
(GCC/Clang, each prior HEAD header/current Linux/current non-Linux selection)
prove enum size/alignment 4/4, class 20/4 and offsets 0/4/8/12/16. Every Linux
archive/consumer uses the same platform-selected definition.

Pure complete tuple admission precedes TextureClass Init, LastAccessed and pending
texture selection, and direct Filter.Apply provider/table/stage effects. Stage,
min/mag/mip and both address values must all pass before accepted publication in
the unchanged min/mag/mip/U/V order. Profile rejection was already before table
writes; its formerly undefined synthetic representation is now defined. Default
setters reject their indices before table lookup or any default-table mutation.
No bgfx/journal, renderer sampler, allocator, factory or private input changes.

Two missing-provider generations assert fresh uninitialized rejection, no file
requests/content changes, exact LastAccessed sentinel and filter tables. Two real
initialized-stage generations additionally assert identical Recording snapshot,
resources, pending texture/sampler identities, prepared-state revision and no
pass/frame activity, followed by clean retry and exact stage-7 admission. Every
tuple field has bound+1, 9 and UINT_MAX controls; profiles cover 3, 99 and UINT_MAX;
stage covers 8 and UINT_MAX; default setters cover count, 9 and UINT_MAX. All valid
default choices and profile/stage defaults are checked exactly, preserving
stage-zero anisotropic downgrade and mip semantics. No implicit-padding compare.

Frozen SHA256:

| Surface | SHA256 |
| --- | --- |
| texturefilter.h | c8dbd36ce703881ed0ed3ac974e56f6b0b1bb2110e250844cda2e7d7e145de8b |
| texturefilter.cpp | 3ba637fa57ffb66eb1a20574c2d082e35716593f060e60d4eace2044ae8c663d |
| texture_apply.inc | b8bdbd7aa829a9495cc471ae57cac7f5d671ec4e11005f3aa76b02afeb279b81 |
| test_original_texture_decisions.cpp | cb8a82c63753880270d781aada78da8b1505eb6e58802017f0f382e2ce905f75 |

## Focused final-source acceptance

Exact regex: `^(original_w3d_texture_decisions|original_w3d_texture_identity|original_w3d_texture_provider_removal|original_w3d_presentation)$`.
Sanitizers use `ASAN_OPTIONS=detect_leaks=0`; complete output/log audits accompany
the successful exits. Former Addr9/Profile99 and all other findings are absent.

| Configuration | Result | Seconds |
| --- | --- | ---: |
| GCC Debug | 4/4 | 1.13 |
| Clang Debug | 4/4 | 1.12 |
| GCC ASan+UBSan | 4/4, clean | 1.67 |
| Clang ASan+UBSan | 4/4, clean | 1.43 |

Exact serial strict host LSan uses `ASAN_OPTIONS=detect_leaks=1`, no UBSan
override, the same four focused tests and host escalation. GCC 4/4 in 1.86s
and Clang 4/4 in 1.54s pass with clean complete logs. Ledger/public-header/diff
checks pass. The unrelated renderer diagnostic remains untouched/unstaged.

## Host physical and transport controls

Host escalation and `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation` retain the
exact established GCC display/bootstrap/map 3/3 plus native A CPU resource and
reservation 2/2 (combined 5/5 in 5.34s), Clang display/map 2/2 plus the same CPU
2/2 (combined 4/4 in 4.29s). Both toolchains' direct generated reservation and
resource `--gpu` controls pass through the unchanged validation-clean wrapper,
2/2 each, with no validation category.

The six canonical presets intentionally disable optional GPU-test registration.
The existing original texture control remains in its established GPU-enabled
`build/m14-gpu`: a scoped current-source target rebuild passes, then exact
`original_w3d_texture_gpu_upload` passes 1/1 in 0.55s with host validation. It
checks generated BC1/BGRA/mips and unchanged valid source sampler transport,
including two device generations. This is an additional existing texture owner
control, not a replacement for the canonical or original public tuple witnesses.
Six serial host LAN selections (`ASAN_OPTIONS=detect_leaks=0`, `ctest -L lan -j1`)
pass 4/4 with clean complete logs, in preset order: 1.71s, 1.71s, 1.71s, 1.71s,
1.82s and 1.78s.

## Complete build and canonical matrix

All six complete builds pass on the frozen source. The Clang sanitizer build
recovers its existing premature Ninja log and completes 842 work items without
an actionable source error. All generated heavy tests run only after build load
clears and are serialized. Complete canonical selections use
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail'
--output-on-failure`, auditing each full LastTest.log for nonfatal findings
before advancing.

| Configuration | Result | Total seconds | Terrain seconds |
| --- | --- | ---: | ---: |
| GCC Debug | 272/272, clean complete-log audit | 278.95 | 60.00 |
| Clang Debug | 272/272, clean complete-log audit | 264.15 | 59.55 |
| GCC Release | 272/272, clean complete-log audit | 148.49 | 24.79 |
| Clang Release | 272/272, clean complete-log audit | 100.51 | 23.84 |
| GCC ASan+UBSan | 272/272, clean complete-log audit | 876.91 | 267.38 |
| Clang ASan+UBSan | 272/272, clean complete-log audit | 691.46 | 221.48 |

All six complete logs receive a final broad `Sanitizer|runtime error:` audit,
not only exit-code checks; there are zero findings. Former presentation
null-FileSystem and texture address-9/profile-99 categories are absent on both
sanitizer configurations. `UBSAN_OPTIONS` is unset; no suppression or strict
UBSan override is introduced. Historical A/R1 qualifications remain accurate
historical evidence, while this independently corrected matrix is fully clean.

Frozen source hashes remain exact after all gates. Final ledger/public-header
and ordinary whitespace checks pass. The durable AGENTS lesson records that
unfixed synthetic enum inputs can be undefined before rejection and requires
defined raw admission plus platform/ABI checks; no allocator lesson is used to
broaden this owner. Exact stage review includes only this admission owner,
witness, ledger and acceptance/plan artifacts, leaving the renderer diagnostic
untouched. Commit boundary: `delivery: M22 08P0C2D0B2R2 admit defined texture filter values`.
