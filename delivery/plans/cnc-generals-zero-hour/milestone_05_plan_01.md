# Milestone 05 delivery plan: persistence and determinism

## Governing authority

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M5-persistence.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially sections 5, 8/M5, and 10.
- Parent commit: `05dc3ff570d5cb2131444347865cad986f0dec37`.

## Outcome

Linux-native saves and replays use explicit, versioned, fixed-width encodings; malformed input is rejected before caller state changes or attacker-controlled allocation; and an asset-free single-threaded scenario produces matching diagnosed checkpoints in all four supported compiler/build presets. Windows 1.04 import remains unverified and belongs only to optional M18.

## Constraints

- Reuse M1 byte/endian/UTF-16 codecs; never serialize host object memory or host-sized types.
- Keep collection and payload limits explicit and validate them before allocation.
- Decode into temporary values and publish state only after the complete document validates.
- Keep simulation single-threaded and require `FE_TONEAREST` at deterministic boundaries.
- No retail files, display, GPU, audio device, network, or Windows fixture is required.

## Slice index

1. [Slice 01 — Linux saves and autosave metadata](milestone_05_plan_01_slice_01.md)
2. [Slice 02 — replay commands and checkpoint snapshots](milestone_05_plan_01_slice_02.md)
3. [Slice 03 — deterministic scenario matrix and floating environment](milestone_05_plan_01_slice_03.md)

The slices are dependency ordered. Slice 01 establishes the versioned document envelope and transactional decode behavior. Slice 02 reuses that persistence boundary for replay commands and CRC-bearing snapshots. Slice 03 exercises both formats through a repeatable simulation and produces comparable cross-preset evidence.

## Milestone validation

For each of `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`:

```sh
cmake --preset <preset>
cmake --build --preset <preset>
ctest --preset <preset> -L 'foundation|determinism' --output-on-failure
```

Run the deterministic scenario result comparison across the four build trees and require byte-identical checkpoint output. The comparison must report preset/scenario/config/checkpoint on divergence. Review committed source and fixtures to confirm no Windows fixture or private path was introduced.

## Completion boundary

M5 is complete only after all slice criteria and the full four-preset validation pass. This plan does not claim Windows save/replay compatibility; M18 remains explicitly unverified.
