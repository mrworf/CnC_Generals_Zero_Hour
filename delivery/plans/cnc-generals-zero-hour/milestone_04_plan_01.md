# Milestone 04 plan 01 — retail VFS and corpus verification

## Authority and outcome

This plan implements [M4](../../milestones/cnc-generals-zero-hour/M4-retail-vfs.md) under `docs/zero-hour-linux-port-plan.md` sections 6, 8, 9, and 10. It delivers an in-process, device-free `zh_main --verify-data` path with deterministic retail-data selection, hardened loose/BIG lookup, safe metadata inventory, and a stable logical-only corpus manifest. Retail inputs remain user-owned and read-only; no retail byte, hash, or private absolute path is committed.

## Preconditions and constraints

- M1 and M3 are accepted at transaction start `39fb1fabba0b338e860f7195f8a2777b84e9831f`.
- The supplied Zero Hour root is `original_game_symlink`, the base Generals root is its `ZH_Generals` child, and the selected locale is `English`. These local paths are validation inputs only.
- Runtime lookup normalizes separators, rejects absolute/traversing/NUL logical names, and ASCII-folds identity.
- Mount priority is Zero Hour loose, selected mod BIGs, Zero Hour BIGs, Generals loose, Generals BIGs. Archive discovery is case-insensitive sorted and first-loaded wins. Case-only loose-path or archive-name ambiguity is fatal and names both sources.
- BIG parsing uses bounded 64-bit arithmetic and rejects bad identifiers, counts, unterminated names, invalid logical paths, offsets, sizes, overflow, and allocation limits before exposing entries.
- Verification initializes no SDL subsystem, window, GPU, audio, or video device and never writes either retail root.

## Slice index

1. [Slice 01 — data selection and device-free verification entry](milestone_04_plan_01_slice_01.md) — completed in `e15a527`.
2. [Slice 02 — hardened BIG and deterministic VFS](milestone_04_plan_01_slice_02.md) — completed in `e287326`.
3. [Slice 03 — metadata inventory and retail corpus manifest](milestone_04_plan_01_slice_03.md) — completed in `6d7bcaf`.

Slices are dependency ordered. All three plan artifacts are created and reviewed before production edits. Each slice is committed separately after focused validation; the full four-preset build/data suite and owned-corpus verification run after slice 03.

## Milestone acceptance

- `--verify-data` resolves roots/config/language independently of the current directory and reports actionable configuration failures.
- Synthetic tests cover all mount layers, stable ordering, locale ambiguity, collisions, traversal and malformed/boundary BIG inputs.
- Verification safely identifies required INI, CSF, W3D, DDS/TGA, audio, video, font, compression, and WWShade metadata without decoding or extracting content.
- A deterministic repository manifest records logical names, formats/effects, precedence expectations, and the selected English locale only.
- All four supported presets build and pass `ctest -L data`; the supplied installation passes a retail-enabled verification from an unrelated CWD without device initialization.
