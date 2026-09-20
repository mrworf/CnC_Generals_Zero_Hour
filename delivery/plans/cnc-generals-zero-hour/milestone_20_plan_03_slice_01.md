# M20 plan 03 slice 01: original W3D schema boundary

## Goal and observable outcome

The production `W3DModuleFactory` registers all 19 original names and constructs their original concrete ModuleData classes through the original ThingTemplate/INI consumer. Representative inherited defaults, fields, overrides, post-processing, and destruction are observable without Direct3D headers, libraries, a window, or GPU. Draw-instance creation remains a distinct factory operation and fails with an actionable diagnostic when the bounded headless profile has not supplied a physical renderer.

## Scope and explicit non-scope

Refactor original W3D draw sources only as needed to shared-compile their canonical ModuleData declarations and methods independently of physical rendering. Preserve registration names, module types, interface masks, tags, allocation, inheritance, field tables, callbacks, and destruction. Add the W3D production factory to Linux `createModuleFactory`. Record deferred physical operations and their M22/M23 owner. Do not implement hardware scenes, interactive rendering, dummy Direct3D APIs, generic schema substitution, a reduced registry, or a success/no-op draw instance.

## Dependencies and ordering constraints

Depends on accepted M28 and plan 02 slices 01-05. Must complete before archive/retail cumulative acceptance because retail `Object.ini` reaches W3D module declarations during ThingTemplate parsing.

## Entry point and end-to-end behavior

`LinuxGameEngine::createModuleFactory` returns the original W3D extension. `GameEngine::init` initializes the base registry plus all 19 W3D registrations. Original ThingTemplate parsing requests each named ModuleData through `ModuleFactory::newModuleDataFromINI`, parses original fields, owns the result, and releases it on normal or failed initialization. A missing registration produces a controlled diagnostic before dereference. If any bounded startup/update/reset path asks for a draw instance, it either executes the required original CPU behavior via a device-edge adapter or fails closed before reporting success.

## State, authorization, and permissions

ModuleData ownership stays in the original factory list and teardown order. Retail inputs are read-only and are used only by the separate gate; tests use project-owned fixtures. No special authorization is applicable. No display/GPU permission or network access is allowed.

## Validation and error recovery

Positive tests enumerate exactly 19 production registrations, parse representative base and derived schemas including inherited/default/override behavior, and verify teardown/allocation counts. Negative tests cover missing and unknown providers, malformed field values, and an attempted deferred instance creation. Compile/link identity proves original classes/methods and factory contribute to the production target; scans and link dependencies reject Direct3D. Initialization failures preserve the first diagnostic and unwind once.

## Expected implementation surfaces

Original W3D module factory/header and draw ModuleData sources/headers, common module-factory failure handling where needed, Linux engine factory selection, CMake target composition, focused W3D schema tests, dependency ledger, and source classification.

## Acceptance criteria and commands

- All 19 exact original registrations resolve concrete original ModuleData through production consumers.
- Representative nontrivial base/derived field parsing, defaults, overrides, callbacks, destruction, unknown/missing providers, and fail-closed instance creation pass.
- No legacy Direct3D include/library or physical acquisition enters the target.
- Focused configure/build/CTest passes under all four native presets, with positive and negative assertions active in Release.
- Commit boundary: `delivery: M20 plan03 slice 01 port W3D schemas`.

## Delivered evidence

Status: complete.

- The production Linux factory selects `W3DModuleFactory`; its exact 19 original registrations construct the canonical original ModuleData type for each registration. The four stateless original draw classes retain their source-defined base `ModuleData`; no custom schema is replaced by it.
- The owned production fixture parses nontrivial `W3DLaserDraw` fields through `ThingFactory`/`ThingTemplate`, while the focused test checks canonical default values and all 19 concrete type identities. Unknown providers, malformed W3D values, and physical instance creation fail closed.
- `original_w3d_schema_identity` proves all 15 original schema translation units are active in the provider and production links, use the schema-only shared-source boundary, expose representative original symbols, and acquire no Direct3D link dependency. Removing the extracted factory provider makes the production relink fail.
- Focused build/runtime, identity, provider-removal, production-entry, dependency-ledger, and source-classification tests passed in `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`. Release executes the same assertions; none rely on `assert`.
- The dependency ledger records the deferred instance lifecycle and assigns the physical scene/resource edge to M22. Source classification is generated from the authoritative inventory policy and now promotes the 15 compiled W3D schema translation units.
