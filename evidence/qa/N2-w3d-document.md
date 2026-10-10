# N2 W3D input ownership — validated supporting checkpoint

Parent: `d172c2fc`. This is an independently exercisable production File-to-owned
document boundary within the existing N2 plan01/slice03, not a completed slice
or milestone. User requested review and commitment of pending work.

## Delivered scope

`NativeW3DDocument` reads original File input, preserves the complete authored
bytes and unknown payloads, bounds nested chunks, and admits owned hierarchy,
HLOD relationships, MeshHeader3 and initialized declared geometric backing,
and raw/compressed animation headers. Replacement is transactional: rejection
or allocation failure preserves the accepted bytes and metadata view identity.
No framework/library sources, retail assets, links or recovery files changed.

Generated fixtures cover modern and pre3.0 hierarchies (including a legacy empty
hierarchy's synthetic root), source parent ordering, distinct LOD/aggregate/proxy
arrays, packed attributes, move ownership, unknown payload retention, malformed
extents/counts/names/parents/numerics, nested depth and truncated hierarchy
prefixes. Unused source center/Euler bit representations are retained, not newly
interpreted. Original read-only VFS reads generated content and rejects writes;
the generated source remains unchanged. No proprietary input was used.

## Frozen validation

Commands for each `CONFIG` in `gcc`, `gcc-sanitize`, `clang-sanitize`:

```sh
cmake --build build/original-core-CONFIG --target original_w3d_document_fixture original_data_fixture original_core_fixture -j 4
ctest --test-dir build/original-core-CONFIG -R '^original_(w3d_document_|data_|core_)' --output-on-failure
```

All three target builds and all 23 related tests per configuration passed:
GCC 0.98s, GCC ASan/UBSan/LSan 5.93s, Clang ASan/UBSan/LSan 4.15s. Processes
exited0; no live validation jobs remain. Sanitizers retain leak checks and
halt-on-error, with no suppressions or increased timeouts. Exact replacement
allocation terminal51 in all three configurations: each failure ordinal [0,51),
successful terminal51 and every same-owner corrected retry are checked, repeated
three times, including exact standard-allocation retirement and view retention.

Logs: `/tmp/n2-w3d-document-<CONFIG>-checkpoint-{build,related}.log`. The CTest
LastTest logs additionally retain the exact terminal output. Configured suite405;
full405 was not run for this supporting checkpoint.

Source fingerprints relative to the established census baseline `9e5c0e4f`:

- Canonical663: `6f70925e806cc48a87266bf771dada77ef3d9dec742353e1ddc4446a3415ab8a`.
- Toolchain-inclusive670: `4507d426d0944ac621840cf4099b3455dcf43bb5e34d59549bf60afd9d1bf451`.

## Failure attribution and limits

Initial GCC focused4 failed together in shared VFS warm-up: the fixture expected
a null result for prohibited write, but original NativeFileSystem throws
ERROR_BAD_ARG. Corrected the fixture to assert rejection; production VFS was
unchanged. The corrected focused4 passed, then the source-compatible empty
legacy/unused metadata cases were added before the final frozen matrix above.
Initial/retry/cohort logs remain `/tmp/n2-w3d-document-gcc-{focused,focused-retry,
cohort-focused}.log`; no failed case remains in the frozen related suites.

This document accepts animation headers, not decoded channels or sampled poses.
Material/channel interpretation, asset/prototype cross-reference resolution,
original draw/condition/bone query consumers, production model cache integration,
actual native factory/full startup, loaded ghost/terrain and nonempty simulation
remain pending under the same existing plan. Malformed channel payloads retained
opaque here are not evidence of usable-model admission. No GPU, retail, gameplay,
audiovisual or whole-game acceptance is claimed.
