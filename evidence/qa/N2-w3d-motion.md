# N2 CPU motion-to-bone ownership — supporting cohort

Parent `e0789e5e`, existing N2 plan01/slice03. No new milestone/slice plan,
vendor modifications, proprietary data or asset writes. This cohort extends
the production W3D document boundary into owned animation and hierarchy queries.

## Scope and observable evidence

NativeW3DAnimation resolves a named hierarchy and independently owns raw
vector/bit, time-coded vector/bit and adaptive-delta decoded backing. Generated
raw/sparse/delta fixtures test authored metadata, pre3.0 shifts, absent/duplicate
and out-of-range bone channels, exporter trailing bytes, nearest/even raw frames,
generic interpolation/wrap, signed delta nibbles, three packet groups, all
quaternion components, filter construction, sparse binary movement, hemisphere
interpolation, visibility/defaults and independent channel durations. Sampling
continues after its borrowed document/hierarchy are destroyed.

The integrated family additionally uses original FileSystem/File reads of one
generated W3D file containing the actual typed hierarchy and raw channels. After
the VFS/File/document and source hierarchy retire, the independently owned motion
and pose still produce the source-derived named muzzle/current/pristine results.
The identical entire VFS/owner cohort is warmed before ownership baselines.

NativeW3DPose owns hierarchy names, authored bases, captures and two complete
pose buffers. Tests cover external root replacement, rotated parent/child
composition, local motion and scaling, local/world captured translation,
visibility, pristine queries, blend behavior and case-insensitive root/miss
distinction. Late numeric rejection leaves the whole accepted pose and borrowed
view intact. Pose evaluate/blend/pristine queries prove zero standard allocation
attempts with fault0 armed; all controls remain CPU-thread operations.

Malformed active channels fail transactionally: truncation sweeps for raw,
time-coded, adaptive and visibility backing; dimensions/counts/frame ordering,
nonfinite samples/reconstruction, incompatible hierarchy and invalid queries.
No malformed fixture uses undefined raw representations.

## Frozen matrix

For each CONFIG in gcc, gcc-sanitize and clang-sanitize:

```sh
cmake --build build/original-core-CONFIG --target original_w3d_animation_fixture original_w3d_document_fixture original_data_fixture original_data_audit original_core_fixture -j 4
ctest --test-dir build/original-core-CONFIG -R '^original_(w3d_(animation|document)_|data_|core_|header_paths$)' --output-on-failure
```

All final target builds and related32 tests PASS: GCC2.59s, GCC ASan/UBSan/LSan9.32s,
Clang ASan/UBSan/LSan6.89s; all process exits0, no live jobs. Leak checks and
halt-on-error remain enabled, with no suppressions or timeout increases.
Raw/sparse/delta replacement allocation terminals7/7/6 and pose3 match across
all configurations. Every prefix and successful exact terminal has a corrected
same-owner retry; storage retirement and accepted view identity checked on every
pair, across three repeated whole lifetimes. Document terminal51 remains covered
in the same frozen related matrix.

Configured413 tests; full413 unrun. Logs:
`/tmp/n2-w3d-motion-<CONFIG>-integrated-{build,related}.log`; CTest LastTest logs retain
terminal outputs. Source fingerprints, established baseline9e5c0e4f:

CTest dry-run enumeration replaced GCC LastTest.log after the passing matrix.
A targeted verbose repeat of only the two allocation-terminal families retained
7/7/6 and pose3 in `/tmp/n2-w3d-motion-gcc-terminal-record.log` (PASS2/2); no broad
matrix was repeated. Preserve required passing stdout before a future CTest
dry-run census, rather than treating its temporary LastTest log as immutable.

- Canonical668: `2755b260e30fa1f85522f42485d9e6c15716374702e290c7d55db89b6069cd8b`.
- Inclusive675: `2092614a745e304cec8c2663646ffa512ecf09a719610e3987858c05b842e138`.

## Iteration attribution and limits

Initial GCC compilation failed because the new fixture omitted explicit
Errors/GameMemory headers; production compiled with indentation warnings.
Corrected all fixture includes and the warnings, then complete motion5 passed.
Added the coupled pose/capture/blend cohort; focused7 passed. Source review then
removed an unjustified equal-channel/clip-duration restriction and added shorter
adaptive/extended sparse positive cases before the frozen matrix. Retained logs:
`/tmp/n2-w3d-animation-gcc-{iteration-build,cohort-build,cohort-focused,pose-build,
pose-focused}.log`. No failed case remains in the final matrix.

The earlier `frozen` matrix passed31 all3 (2.55s/8.94s/6.73s) before the final
explicit `-ffp-contract=off` original_data build configuration. Retain that as
prior evidence, not as the final source fingerprint above. The final matrix
rebuilds all affected original_data objects and repeats the whole related cohort.
That intermediate `final` related31 passed2.54s/8.94s/6.61s, before the generated
integrated File-to-model family was added. The accepted final `integrated`
related32 matrix includes that additional whole-input-retirement boundary.
Clang reports pre-existing RAMFile override annotations; no new motion/pose
compile errors or warnings remain. No unrelated annotation cleanup was included.

Source findings and two original adaptive cache/reconstruction defects are
documented in original-engine-formats.md. Native deterministic decoding corrects
those bugs. Quaternion interpolation follows the original portable Fast_Slerp
table algorithm; the generated mathematical oracle uses tolerance1e-4. This is
not retail renderer/animation parity evidence or proof of bitwise equivalence to
the old x87 assembly implementation.

Material/channel decoding, prototype/HLOD relationship admission, model caches,
condition binding and original production draw/bone callers are not wired yet.
Morph/combo animation, capture integration into original draw controls and actual
physical output are not accepted here. Full original native startup, loaded
terrain/ghost and nonempty deterministic simulation remain N2 requirements. N2
and slice03 remain in progress; no full-game or milestone acceptance claimed.
