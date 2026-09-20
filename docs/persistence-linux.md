# Linux persistence and determinism

The native port uses project-owned, versioned Linux save (`ZHSG`) and replay (`ZHRP`) envelopes. Every persisted number has an explicit width and little-endian encoding through the shared foundation codec. Text is length-prefixed UTF-16LE and validated for complete surrogate pairs. Collections and embedded payloads have fixed limits that are checked before allocation. Readers decode into temporary state and publish it only after the complete document, including trailing-byte and snapshot-CRC checks, succeeds.

Autosave metadata is part of the save document and does not change the state encoding. Replay checkpoints carry canonical snapshots and CRC-32 values calculated from their encoded bytes, never from C++ object memory. A CRC divergence reports scenario, configuration, checkpoint tick, expected value, and actual value.

The asset-free determinism scenario is single-threaded. Deterministic entry points require `FE_TONEAREST`; they reject a changed rounding mode before simulation mutation. Calls into third-party code use a floating-environment isolation boundary that restores the complete simulation environment on normal return and exceptions. The build already applies `-fno-fast-math` and `-ffp-contract=off` to native targets.

Each supported GCC/Clang Debug/Release build writes `determinism-result.txt`. Compare all four with:

```sh
python3 tests/determinism/compare_presets.py \
  build/linux-gcc-debug/determinism-result.txt \
  build/linux-clang-debug/determinism-result.txt \
  build/linux-gcc-release/determinism-result.txt \
  build/linux-clang-release/determinism-result.txt
```

These formats promise Linux-to-Linux persistence and deterministic replay. Importing Windows 1.04 saves or replays is not inferred from source-compatible field choices and remains explicitly unverified until optional milestone M18 supplies provenance-recorded legal fixtures.
