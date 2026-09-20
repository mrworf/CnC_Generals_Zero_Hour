# M4 slice 01 — data selection and device-free verification entry

## Goal and observable outcome

Users can run `zh_main --verify-data` with explicit `--zh-data`, `--generals-data`, and `--language` arguments or XDG configuration. CLI values override configuration, both roots may be identical, locale auto-selection accepts exactly one viable locale, and errors name the relevant option/configuration key. The path exits before any device subsystem is constructed.

## Scope

- Define the data-selection API, configuration parser, locale discovery, and verification exit codes.
- Read `$XDG_CONFIG_HOME/generals-zero-hour/options.ini` (or the HOME fallback) with the three canonical keys.
- Integrate `--verify-data` into the existing executable without changing headless behavior.
- Add synthetic positive/negative tests including CLI precedence, identical roots, missing roots, invalid config, zero/multiple locales, and arbitrary CWD.

## Non-scope

BIG parsing, archive lookup, format inventory, retail traversal, automatic installation scanning, and gameplay initialization are deferred to later M4 slices.

## Dependencies and ordering

Depends on accepted M1 XDG/path primitives and M3 startup separation. It is the first M4 slice; slice 02 consumes its resolved selection.

## Entry point and end-to-end behavior

`zh_main --verify-data [--zh-data PATH] [--generals-data PATH] [--language NAME]` parses each option once, reads optional XDG configuration, applies CLI-over-config precedence, validates readable directories, selects a locale, reports resolved values without exposing config/private paths in committed artifacts, and returns a stable code. Unknown/duplicate/missing-value options fail as usage errors. Verification does not construct headless or real devices.

## Data/state transitions

Raw CLI plus optional configuration become an immutable resolved data selection. No persistent state and no retail writes occur.

## Authorization and permissions

No authorization model applies. Retail roots require read permission only; configuration is read-only.

## Validation, error handling, and recovery

Reject relative/unreadable roots and malformed configuration with key-specific diagnostics. If language is absent, accept only one detected locale; list viable names for ambiguity and report none explicitly. Recovery is correcting CLI/config/root contents and rerunning.

## Expected implementation surfaces

`include/zh/data/config.h`, `src/data/config.cpp`, `src/bootstrap/main.cpp`, `CMakeLists.txt`, `tests/data/test_data_config.cpp`, and build documentation.

## Tests and commands

- Positive: explicit selection, config selection, CLI overrides, identical roots, one-locale automatic selection, invocation from unrelated CWD.
- Negative: duplicate/unknown/missing options, relative/missing root, malformed/unknown config key, zero/multiple locales.
- Run focused data-config tests and existing headless/foundation tests under `linux-gcc-debug`.

## Acceptance criteria

- Resolution behavior and diagnostics match the authority.
- `--verify-data` reaches only data verification code, not any device initialization path.
- Existing headless behavior remains green.

## Commit boundary

Commit this plan artifact with the complete selection/entry behavior, its tests, CMake wiring, and documentation as `delivery: M4 slice 01 resolve retail data`.
