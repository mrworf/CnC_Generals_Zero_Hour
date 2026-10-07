# Milestone N4: Interactive platform and media

## Objective

Users can navigate original menus and interact with supplied media using native Linux input/output.

## User/System Outcome

Users can navigate original menus and interact with supplied media using native Linux input/output.

## Scope

SDL window/input/IME, original UI/font/locale integration, audio/music/speech and movies, queue pressure and shutdown.

## Explicit Exclusions

No new UI design, online services, asset alteration or replacement media.

## Source Requirements

R05, R09 in docs/zero-hour-linux-port-plan.md. R02–R05 constrain all applicable work.
The user-approved replacement graph fixes this milestone's boundary.

## Preconditions

- N1 accepted before implementation consumes its output.
- N2 accepted before implementation consumes its output.
- PRE-03, PRE-04, PRE-06: see delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/prerequisite-manifest.md.

## Readiness checks

- Inspect status.yaml and the named dependency acceptance evidence; pending providers are not accepted.
- Inspect the governing plan and AGENTS.md; only linux-upstream-v2 is active.
- Check the prerequisite manifest's command results and milestone-owned deliverables before starting.

## Functional Requirements

- Connect original menus, keyboard/mouse/text flows and supplied locale with supported platform APIs.
- Decode media from read-only VFS and preserve looping/priority/volume/completion semantics.
- Handle audio queue saturation and shutdown without lost ownership or callback-thread simulation access.
- Preserve original loading/error/return-to-menu behavior and usable no-audio-device mode.

## UX Constraints

Preserve source-established UI/gameplay behavior. Render loading/error states truthfully;
never claim a scene or operation succeeded when required content or output is missing.
N0/N1 have developer-facing diagnostics; they do not redefine player-facing UX.

## Architecture / Security Constraints

Stock dependencies/public APIs only. Retail roots are read-only, diagnostics redacted,
no proprietary distributions. No extra privileges or network accounts are introduced.
Validate externally supplied lengths/paths and preserve allocator/thread ownership.

## Interfaces and Compatibility

Zero Hour internal interfaces may change; no legacy MSVC/internal Windows ABI mandate.
Preserve source asset formats, externally meaningful encodings and intended behavior.
External dependencies use their supported interfaces; do not publish custom SDK APIs.

## Acceptance Criteria

- [ ] Original UI flows, text and controls work through the actual process.
- [ ] Audio/video playback, loop/stop transitions and no-device behavior are verified.
- [ ] Pressure and repeated shutdown release each decoder/voice exactly once.

## Required Validation

Generated audio/video fixtures and original runtime journeys; queue saturation/callback tests, locale/input controls, sanitizer ownership coverage and read-only retail media checks.
Freeze coherent changes before broad checks; document unexecuted checks honestly.
Documentation-only work receives artifact review, not invented game tests.

## Known Risks / Deferred Work

System fonts may differ; preserve intended layout and supplied locale semantics. Do not infer audio correctness from silent mocks alone.
