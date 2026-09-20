# M12 slice 01: pin and compile miniaudio offline

## Goal and observable outcome

Vendor one exact reviewed miniaudio release with complete provenance and license records, compile it into `zh_audio_miniaudio`, and prove configuration/build performs no dependency download.

## Scope

- Pin miniaudio 0.11.25 at upstream commit `9634bedb5b5a2ca38c1ee7108a9358a4e233f14d`.
- Vendor the exact upstream `miniaudio.h` and license, verify their SHA-256 hashes, and select the MIT No Attribution license alternative.
- Record URL, version, commit, hashes, license choice, review notes, and compile-time options.
- Compile one implementation translation unit with only the Linux-relevant backends and supported built-in decoders enabled.
- Add an asset-free version/decoder capability test and an offline dependency guard.

## Non-scope

No engine VFS, voice manager, device initialization, or playback behavior is introduced in this slice.

## Dependencies and ordering

This is the first M12 task and produces PRE-011. Slice 02 cannot begin until this slice builds offline.

## Entry point and end-to-end behavior

CMake consumes only repository-owned files, builds `zh_audio_miniaudio`, and a capability test verifies the compiled miniaudio version and expected decoder features without opening a device.

## Data or state transitions

No runtime state. Dependency state transitions from an unimplemented bootstrap target to a reproducibly pinned local translation unit.

## Authorization and permissions

One-time network access is limited to retrieving official upstream release metadata/header/license. Ordinary configure, build, and tests are offline.

## Validation, errors, and recovery

The build fails if vendored hashes drift or a prohibited configure-time fetch primitive is introduced. The capability test fails on version or feature mismatch.

## Expected implementation surfaces

`third_party/miniaudio/`, `src/audio/miniaudio_impl.cpp`, CMake wiring, dependency documentation, and `tests/audio/test_miniaudio_dependency.cpp`.

## Positive tests

- Exact miniaudio version compiles and reports required decoder capabilities.
- A clean canonical configure and build uses only vendored sources.

## Negative tests

- Hash/provenance checker rejects modified vendored dependency bytes.
- Offline guard rejects `FetchContent`, `ExternalProject`, or download declarations for miniaudio.

## Required commands

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug --target audio_miniaudio_dependency_tests
ctest --preset linux-gcc-debug -R '^audio_miniaudio_dependency$' --output-on-failure
python -m unittest tests.audio.test_miniaudio_provenance
```

## Acceptance criteria

PRE-011 is recorded with exact provenance and hashes, the target compiles from local files, and positive/negative dependency tests pass with no network access.

## Delivered evidence

- Official GitHub release metadata resolved 0.11.25 to commit `9634bedb5b5a2ca38c1ee7108a9358a4e233f14d` before vendoring.
- `cmake --preset linux-gcc-debug` completed using only repository and distribution inputs.
- `cmake --build --preset linux-gcc-debug --target audio_miniaudio_dependency_tests` passed.
- `ctest --preset linux-gcc-debug -R '^audio_miniaudio_(dependency|provenance)$' --output-on-failure` passed (2/2).
- `python -m unittest tests.audio.test_miniaudio_provenance` passed (3/3), including the modified-byte rejection and configure-time-download guard.

## Commit boundary

Commit plans, vendored dependency/provenance, implementation TU, CMake wiring, and focused tests as `delivery: M12 slice 01 pin miniaudio offline`.
