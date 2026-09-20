# M3: headless engine and staged startup

## Outcome

The native executable gains an asset-free headless mode with explicit argument parsing, XDG or caller-selected writable state, null devices, deterministic controlled ticks, staged initialization and reverse teardown. It does not initialize SDL video, GPU, audio, networking, retail data, or any removed Windows service.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Governing contract: `delivery/milestones/cnc-generals-zero-hour/M3-headless-startup.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`, sections 6 and 8/M3
- Transaction start: `6107d3bac9df993b35350d9c2cefc70ba6fa5145`
- This companion transaction implements M3 only; the outer orchestrator owns milestone acceptance and status.

## Current-state findings

- M0 provides `main(int, char **)`, the final null-device target names, build metadata, and four canonical presets, but the executable only accepts `--bootstrap-smoke`.
- M1 provides x86-64-safe foundation types, XDG path resolution, steady-clock helpers, atomic file writes, and joining threads.
- Null platform, renderer, audio, and video targets are bootstrap placeholders. No headless lifecycle, fixed-tick runner, writable-state selection, or startup failure seam exists.
- The current native bootstrap path already excludes legacy `WinMain` and all retail game sources, which is the safe seam for replacing Windows startup without broad legacy rewrites.

## Decisions

- Keep `--bootstrap-smoke` as the M0 compatibility path and add an explicit `--headless` mode. Headless supports `--ticks <0..1000000>`, `--state-dir <absolute-path>`, and test-only `--fail-init <stage>`.
- Define typed headless configuration, result/exit codes, lifecycle events, and null-device interfaces under `zh::headless`. Each null target owns its concrete device implementation; the runner owns only orchestration.
- Stages are `paths`, `logging`, `platform`, `renderer`, `audio`, `video`, and `engine`. Successfully initialized stages are torn down exactly once in reverse order on success or injected failure.
- A per-run log and completion record are written only below the resolved state root. Capability output identifies build/architecture/SDL/FFmpeg-Bink context, unconfigured retail roots/locale, and intentionally skipped display/GPU/audio/video/network services.

## Slice index

| Slice | Plan | Outcome | Dependency | Status | Commit |
|---|---|---|---|---|---|
| 01 | [headless entry and null devices](milestone_03_plan_01_slice_01.md) | Parsed headless entry, staged lifecycle, null devices, controlled ticks, logs, exit codes, and capability report | M1 | completed | `631d7eb` |
| 02 | [failure-safe lifecycle](milestone_03_plan_01_slice_02.md) | Every initialization stage can fail deterministically and proves reverse partial teardown | 01 | completed | `3b51378` |
| 03 | [isolated concurrent processes](milestone_03_plan_01_slice_03.md) | Simultaneous processes use isolated writable roots and full GCC/Clang validation is documented | 01, 02 | completed | `856626b` |

## Cross-slice constraints

- Headless execution must not initialize SDL, a display, GPU, audio device, retail VFS, or network and must not depend on launcher, DRM, registry, splash, named mutex, browser, GameSpy, CD, SEH, or current-working-directory changes.
- No test reads `original_game_symlink`, requires private media, opens a listener, or uses an interactive device.
- Initialization and shutdown ordering are observable and deterministic. Invalid arguments and injected failures produce actionable diagnostics and stable nonzero exit codes.
- Preserve all M0-M2 targets, options, labels, shaders, inventories, and tests.

## Milestone completion gate

- Build all four canonical presets and run `ctest --preset <preset> -L 'foundation|headless' --output-on-failure`.
- Verify positive, invalid-argument, tick boundary, XDG/state-root, every-stage failure, reverse teardown, and simultaneous isolated-process cases.
- Run with deliberately unusable SDL video/audio environment settings to demonstrate the null path has no device dependency.
- Confirm tracked artifacts contain no retail bytes or absolute private paths and run `git diff --check`.

## Rollback and recovery

Each slice is independently revertible. Slice 01 is a complete successful headless path; slice 02 adds only controlled negative-path coverage; slice 03 adds process-level isolation coverage and documentation.

## Execution notes

All slice plans were created and inspected before production edits. The boundaries retain one functioning positive path before adding exhaustive injected failures and then cross-process validation.

All four canonical presets configured and built successfully. GCC Debug, Clang Debug, GCC Release, and Clang Release each passed the same twelve-test `foundation|headless` selection: nine preserved foundation tests and three headless tests. The process test confirms invalid SDL display/audio drivers are never consulted, two explicitly rooted runs remain isolated, and no current-working-directory file is created. New headless sources contain no SDL initialization, GPU, socket, launcher, DRM, registry, mutex, browser, GameSpy, CD, SEH, or `chdir` call path; they create no worker thread.
