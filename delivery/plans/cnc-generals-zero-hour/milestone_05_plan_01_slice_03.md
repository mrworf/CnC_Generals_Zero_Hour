# M5 slice 03: deterministic scenario matrix and floating environment

## Goal and observable outcome

An asset-free single-threaded scenario can be run, saved, resumed, replayed, and checkpointed repeatedly with identical diagnosed results, while a changed floating-point rounding mode is detected and third-party calls cannot leak floating-environment changes into simulation.

## Scope

- Add a deterministic fixed-tick scenario using explicit replay commands and canonical checkpoint snapshots.
- Require `FE_TONEAREST` on scenario entry and before every deterministic tick.
- Add an isolation guard that restores the simulation floating environment after a third-party callback.
- Emit stable scenario/config/checkpoint/CRC diagnostics and compare result files across all four supported presets.
- Verify save/load continuation and replay execution reach the same checkpoints.

## Non-scope

Retail gameplay, graphical/audio execution, network simulation, Windows comparison, and process-wide x87 compatibility.

## Dependencies and ordering

Depends on slice 01 saves and slice 02 replay/snapshot codecs.

## Entry point and behavior

`run_scenario` accepts a bounded scenario configuration and command sequence and returns ordered checkpoints. It throws before simulation if rounding is not `FE_TONEAREST`. `with_isolated_floating_environment` executes external work and restores the caller's complete floating environment even when the callback throws. A test executable can write a canonical text result; a comparison script names preset/scenario/config/checkpoint for any divergence.

## State transitions

Commands update integer and IEEE-754 state in deterministic tick order. Save/load resumes from a validated snapshot. External callbacks may temporarily alter floating state but the guard restores it before simulation continues.

## Authorization and permissions

Not applicable. Tests use build-local files only and require no retail data, device, display, or network access.

## Validation and recovery

Reject changed rounding mode before mutation, excessive ticks/commands, out-of-order commands, invalid command kinds, non-finite results, and replay checkpoint divergence. Diagnostics include scenario, config, checkpoint tick, expected CRC, and actual CRC. Restore the prior floating environment on callback success or exception.

## Implementation surfaces

- `include/zh/simulation/determinism.h`
- `src/simulation/determinism.cpp`
- `tests/determinism/test_scenario.cpp`
- `tests/determinism/compare_presets.py`
- `CMakeLists.txt`
- `docs/persistence-linux.md`
- this slice plan

## Tests and commands

Positive: repeat runs, save/resume equivalence, replay equivalence, stable diagnostic output, and restoration after a callback changes rounding. Negative: changed entry rounding, invalid command/tick limits, forced checkpoint divergence, and throwing callback restoration.

Run `foundation|determinism` after building every preset, then compare build-local scenario outputs:

```sh
python3 tests/determinism/compare_presets.py \
  build/linux-gcc-debug/determinism-result.txt \
  build/linux-clang-debug/determinism-result.txt \
  build/linux-gcc-release/determinism-result.txt \
  build/linux-clang-release/determinism-result.txt
```

## Acceptance criteria

- Repeated and cross-preset checkpoint outputs are byte-identical.
- Save/resume and replay execution produce the direct-run checkpoints.
- Changed rounding is detected before simulation mutation.
- External floating-environment changes and exceptions are isolated/restored.
- Divergence diagnostics identify preset/source, scenario, config, checkpoint, expected, and actual values.
- Documentation explicitly keeps Windows import unverified until M18.

## Commit boundary

Commit deterministic scenario, comparison tooling, docs, tests, build wiring, and this plan as `delivery: M5 slice 03 prove deterministic persistence`.
