# M20 plan 02 slice 01: original subsystem ownership and lifecycle spine

## Goal and observable outcome

Compile and execute the original lifecycle spine rooted in `SubsystemInterfaceList`, `GameMain`, and `GameEngine` behind a Linux `CreateGameEngine` boundary. An executable test observes original subsystem registration/order and proves pending initialization failure plus registered-subsystem failure unwind exactly once in reverse order without publishing stale globals.

## Scope

- Port original headers/PCH/CRT calls required by the lifecycle spine using narrow Linux compatibility definitions.
- Correct `SubsystemInterfaceList::initSubsystem` so an object that fails before list registration is released exactly once and its publication can be cleared by the caller.
- Introduce the Linux concrete engine factory and build target structure needed to consume real providers in later slices.
- Compile actual `GameMain.cpp`, `GameEngine.cpp`, and subsystem implementation bodies; temporary link boundaries may isolate not-yet-promoted provider archives but may not supply fake success, empty registries, or alternate lifecycle methods.
- Add active positive/negative lifecycle ownership tests and compile/live-symbol identity checks.

## Explicit non-scope

Complete configuration callbacks, registries, bounded updates, retail initialization, and final executable acceptance belong to later slices. This slice does not claim full engine initialization and must not edit plan 01 history.

## Dependencies and ordering

Requires accepted M26-M28 provider libraries. It is the first plan-02 slice and supplies the ownership/factory spine consumed by slices 02-06.

## Entry point and behavior

The source entry is original `GameMain(argc, argv)`, which calls `CreateGameEngine`, original `GameEngine::init/execute`, and deletes the published engine. Focused ownership tests enter actual `SubsystemInterfaceList::initSubsystem`, `postProcessLoadAll`, `resetAll`, and `shutdownAll` with source-derived test subsystems. Successful initialization records source order; injected constructor/init/data-load/post-load failures preserve the initiating diagnostic and release pending plus registered instances once.

## Data/state transitions

Unpublished -> pending subsystem -> initialized/data-loaded -> registered -> post-loaded -> reset -> reverse shutdown. Failure before registration transitions pending directly to destroyed and clears its caller-owned publication; failure after registration unwinds the registered list in reverse. Original `TheGameEngine`/`TheSubsystemList` publication is null before and after a failed run.

## Authorization and permissions

Not applicable to in-process ownership. Tests use owned fixtures and isolated temporary directories; no retail root, window, GPU, audio endpoint, or network is accessed.

## Validation and error handling

Positive: ordered multi-subsystem init/post-load/reset/shutdown and repeated destruction. Negative: construction, init, first/second INI load, post-load, and teardown-diagnostic injection. Preserve the primary failure when cleanup also reports an error. Active counters must return to zero in every preset.

## Expected implementation surfaces

`CMakeLists.txt`, lifecycle compatibility headers/sources, actual original lifecycle/subsystem sources, Linux factory source, tests under `tests/original_lifecycle/`, dependency ledger/classification, and identity/provider-removal controls.

## Required validation

- Configure/build focused lifecycle targets in all four presets.
- Run the focused `original-lifecycle` ownership and identity tests in all four presets.
- Run classification and dependency-ledger checks.
- Run focused Clang ASan/UBSan for ownership paths before slice completion if the slice is the last point at which these paths change; otherwise defer the cumulative sanitizer run to slice 06 and record exact ownership counts now.

## Acceptance criteria

- The actual original subsystem/lifecycle translation units compile with no alternate `GameEngine` implementation.
- Source-owned order and state, not trace-only counters, establish lifecycle execution.
- Every pending/registered failure releases exactly once in reverse order and clears publications.
- Removing an original lifecycle provider fails compile/link/runtime identity.
- No hardware/private-data access occurs.

## Commit boundary

Commit the plan artifacts, lifecycle spine, tests, ledger/classification changes, and passing focused evidence as one slice: `delivery: M20 slice 01 port original lifecycle spine`.

## Completion record

Status: complete.

- The production build compiles the actual `GameMain.cpp` and `GameEngine.cpp` bodies into `zh_original_lifecycle_spine`; the object-identity check requires their exact compile commands, object files, and strong lifecycle text symbols, so dead/discarded sections cannot satisfy the check.
- The linked focused executable runs the actual `SubsystemInterface.cpp` implementation. Positive ordering plus missing-loader, injected data-load, init, post-load, and reset failures prove fail-closed behavior and exactly-once reverse shutdown.
- The initially broad compiler-closure edit was reduced to an auditable mechanical set. `docs/original-runtime-enum-abi.tsv` inventories all 47 changed opaque enum pairs, while `original_lifecycle_enum_abi` compares every definition body with activation commit `4c79b0c`, compiles every type at `sizeof(int)`, and rejects an unpaired declaration.
- One Clang-only standard violation discovered by the four-preset gate (a default argument in a callback typedef) was removed without changing the callback signature or behavior.
- `original_lifecycle_subsystem`, `original_lifecycle_subsystem_identity`, `original_lifecycle_spine_objects`, `original_lifecycle_enum_abi`, and `original_lifecycle_provider_removal` pass in GCC Debug/Release and Clang Debug/Release. Source classification and the checked dependency ledger pass. Sanitizer coverage remains cumulative in slice 06 because these lifecycle paths continue to be integrated in slices 02-05.
