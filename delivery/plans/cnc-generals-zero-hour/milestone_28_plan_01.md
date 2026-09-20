# M28: Original CPU presentation and media resources

This plan governs exactly one milestone transaction.

## Outcome

Asset-free x86-64 Linux executables exercise source-owned Zero Hour CPU presentation, UI resource, layout, and audio-definition behavior through the accepted recording/VFS/font/audio seams. The consumers acquire no window, GPU, or audio device, report missing and unsupported operations, and release resources and callbacks deterministically.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Milestone: `delivery/milestones/cnc-generals-zero-hour/M28-original-cpu-resources.md`
- Active immutable packet fingerprint: `sha256:0d5f1fef5dcec3bbbbb26656c5aaf591d653e2454501b51cfb09059fbb6acd7a`
- Transaction-start `HEAD`: `3f97699af7674b3d05d6dbaa7e57066346574aff`
- Required predecessors: accepted M27, M6, M7, M8, M12, and M13

## Governing authority

- `docs/zero-hour-linux-port-plan.md`
- `docs/zero-hour-source-engine-migration.md` (`SE-005`, `SE-006`, `SE-010`)
- `docs/zero-hour-runtime-closure-reconciliation.md` (`RC-002`, `RC-004`, `RC-006`, `RC-007`, `RC-009`, `RC-010`)
- M28 milestone contract and readiness prerequisite manifest
- No repository-local `AGENTS.md` exists; CMake presets and established asset-free CTest conventions govern.

## Current-state findings and decisions

The original `W3DDisplay.cpp`, asset manager, window manager, and `GameAudio.cpp` mix independently executable CPU work with Win32/device acquisition and complete GameClient registries. Compiling them whole here would either pull M20/M22/M23 forward or require reduced registries. M28 therefore adds narrow original-tree extraction units for named source methods and tables, checked against the authoritative sources and exposed by one production-facing provider library. These extractions invoke the accepted native VFS, recording renderer, font, and audio seams rather than becoming an alternate device implementation.

The provider preserves logical resource ownership, loader and callback registration, image coordinate/status semantics, bounded WND parsing (including the reset `Menus/BlankWindow.wnd` fixture), and audio definition/music lookup. Physical display/GPU/audio acquisition is prohibited by injected recording/null adapters. The complete GameClient factory/registry and display lifecycle remain M20-owned; real rendering remains M22 and interactive menu/cinematic acceptance remains M23.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [CPU scene, loaders, and device lifecycle](milestone_28_plan_01_slice_01.md) | Original CPU scene/resource ownership drives the recording device through registered loaders with explicit DX8 initialization/destruction capabilities. | M27, M7 | complete | recorded after commit | 4/4 focused tests in all four presets |
| 02 | [Font, image, and WND resources](milestone_28_plan_01_slice_02.md) | Original UI resource semantics load owned fonts/images/layouts through native providers, including `BlankWindow.wnd`. | slice 01, M8 | pending | — | — |
| 03 | [Audio definitions, music, and assurance](milestone_28_plan_01_slice_03.md) | Original audio tables parse and resolve owned music without CD/modal waits, with full identity, lifecycle, and milestone evidence. | slices 01-02, M12/M13 | pending | — | — |

## Cross-slice constraints

- Native x86-64 Linux only; no ARM64 compatibility work.
- Owned fixtures only. No retail bytes, paths, hashes, or metadata enter evidence or commits.
- Source-owned declarations and extraction provenance are checked; required-provider removal and source drift fail closed.
- No window, GPU, Vulkan, audio endpoint, browser/service path, or complete GameClient initialization is permitted in focused consumers.
- Unsupported required operations and missing/malformed logical resources fail with actionable diagnostics; production never fabricates success.
- Callbacks, loaders, resources, and workers are destroyed before their owners and global registrations are cleared exactly once.
- M20 retains coupled GameClient/complete registry lifecycle, M21 map-start producers, M22 hardware rendering, M23 interactive/media flows, and M25 CRC error UI integration.

## Milestone completion gate

- Build and run all focused M28 consumers in GCC/Clang Debug/Release.
- Run the complete asset-free CTest suite once after slice 03 stabilizes.
- Run focused Clang ASan/UBSan with exact options and explicit resource/callback/worker counts.
- Verify compile-command, live-symbol, runtime-witness, extraction-provenance, dependency-ledger, source-classification, and provider-removal controls separately.
- Reconcile all acceptance criteria in durable QA evidence without claiming M20/M22/M23 acceptance.

## Rollback and recovery

Each slice is independently revertible. Slice 03 removes media definitions and assurance while retaining UI and CPU resource providers; slice 02 removes UI resources while retaining the CPU scene seam; slice 01 removes the M28 library. Generated build directories and owned fixtures are disposable.

## Execution notes

Planning and all slice files were created before production edits. The transaction started from clean `HEAD` `3f97699af7674b3d05d6dbaa7e57066346574aff`.

Slice 01 isolates original W3D CPU initialization, loader/resource ownership, scene/light lifetime, and DX8 CPU/device capability semantics behind the accepted VFS and recording-device interfaces. Owned model, texture, and animation fixtures reach a real recorded draw, while physical device and browser capabilities remain unavailable. Six partial-failure stages, duplicate/missing/malformed inputs, repeated shutdown, live-symbol identity, authoritative-source provenance, ledger freshness, and exact zero resource restoration pass in all four presets.
