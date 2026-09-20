# M20 plan 01 slice 02: VFS/XDG data and factory-stage initialization

Status: cancelled pending upstream replanning. This plan must not execute while slice 01's source-order dependency blocker remains unresolved.

## Goal and outcome

The source-owned lifecycle initializes from real logical resources through the existing read-only VFS and isolated XDG paths, validating INI, CSF, name-key, module-factory and object-factory stages before execution can reach the unavailable-simulation boundary.

## Scope

Implement the Linux data adapter, bounded structural validation for required INI/CSF inputs, durable in-memory factory registries derived from project-owned fixture configuration, XDG state-directory creation, and positive/negative fixture tests.

## Non-scope

No retail bytes are copied or committed; no object simulation, W3D producers, UI, audio, video, saves, replays or networking are implemented. Structural validation does not claim complete M21 gameplay parsing.

## Dependencies and ordering

Requires slice 01 lifecycle ownership and the M4 VFS/config contracts. Runs before process/identity acceptance in slice 03.

## Entry point and behavior

`GameEngine::init(argc, argv)` parses the existing explicit data arguments, mounts the VFS, prepares the XDG state directory, loads required logical INI/CSF resources, derives nonempty module/object registries, and records source-owned outputs. `execute` remains explicitly non-gameplay.

## Data/state transitions

Each completed data/factory stage owns a concrete resource or registry and is released once in reverse order. Writes are confined to the isolated XDG state root; retail resources are opened read-only.

## Authorization

Only caller-supplied/project-owned fixture roots are used by default. Optional retail validation follows the existing explicit gated contract and must not persist private paths, bytes, or hashes.

## Validation and error handling

Positive fixtures cover valid loose INI/CSF data. Negative cases cover absent resource, malformed INI, malformed CSF, empty module/object registries, invalid/relative roots and injected factory-stage failure. All failures must expose the failing logical stage and unwind once.

## Expected surfaces

Linux lifecycle adapter/implementation, project-owned fixture generator or test fixture, focused tests, CMake and source classification.

## Validation commands

- Build and run `original_lifecycle_tests` under GCC and Clang Debug.
- Run data and original-lifecycle CTest labels.
- Run with isolated XDG variables and an arbitrary working directory.

## Acceptance and commit boundary

Valid project-owned data reaches explicit simulation-unavailable execution; invalid data/factories fail deterministically and unwind; tests prove no presentation acquisition. Commit the adapter/data behavior and its tests together.
