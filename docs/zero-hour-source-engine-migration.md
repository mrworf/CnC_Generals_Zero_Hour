# Original-engine migration supplement

Status: implementation-ready architecture remediation, 2026-09-20.

Authority: the owner's architecture-revalidation and delivery-replanning request, preserving all behavior and constraints in `docs/zero-hour-linux-port-plan.md`. This supplement resolves engineering execution and assurance gaps found in `evidence/qa/cnc-generals-zero-hour/architecture-revalidation-2026-09-20.md`; it does not authorize a different game or relax original acceptance criteria.

```yaml
product_id: cnc-generals-zero-hour
change_id: SOURCE-ENGINE-2026-09-20
input_mode: CHANGE_PLAN
implementation_required: true
implementation_ready: true
delivery_planning_ready: true
decisions_required: []
remediation_ids: [SE-001, SE-002, SE-003, SE-004, SE-005, SE-006, SE-007, SE-008, SE-009, SE-010]
```

## Fixed decisions and boundaries

Native x86-64 Linux, primary Arch host with RTX 4070; retain the existing secondary Linux clean-build acceptance and optional Windows-compatibility boundary. No ARM64, Wine-dependent runtime, new gameplay model, online-service restoration or mixed-OS LAN. Original retail roots/symlinks are read-only; no retail bytes, private paths or asset hashes in committed evidence. Use logical asset names and format metadata. Existing verification prohibitions on copying/hashing retail files remain in force.

Production ownership: original GameEngine/GameLogic owns game state, update order, scripts, AI, objects, modules, player rules and serialization traversal; original GameClient/W3D/WWShade owns presentation producers; Linux adapters own OS, device and external-library boundaries. Existing native components may implement those adapters after contract verification. Test fixtures do not own production game state. Keep original memory-pool and subsystem lifecycle semantics unless a specific portability defect requires a narrow characterized change.

## Required remediation and acceptance contracts

### SE-001 — Auditable source ownership and production identity

Classify every source in the existing legacy inventories as production-compiled, platform-replaced, tool-only or deliberately excluded, with target/provider and justification. Distinguish the Zero Hour engine from tools and unrelated base-game duplication. Generate compiler-command and final-link evidence from actual build outputs, not target properties. Mark bootstrap and Alpha/Bravo paths fixture-only. Incremental migration targets may start incomplete, but final production closure must contain no bootstrap substitution, unresolved-symbol suppression or success-returning fake subsystem. Add positive and negative checks that distinguish original source compilation, final executable linkage, and runtime execution; disabling a required original implementation must fail the relevant gate. Preserve existing component tests and labels.

### SE-002 — Original portable foundation closure

Compile the required original compression, WWMath/WWLib/WWSaveLoad, strings, memory pools, chunk I/O and engine support through real target sources. Port original typedefs, pointer assumptions, inline assembly, ATL/Windows precompiled includes and platform CRT usage. Apply the original plan's fixed-width/UTF-16 (`char16_t`, never `-fshort-wchar`), bounded endian serialization and floating-point requirements to the original call sites, not only new helper APIs. Characterize old rounding/format rules and test malformed/boundary cases. Required original support closure must compile GCC/Clang Debug/Release; sanitizer evidence targets real support objects and allocator teardown. Dependency inventory must identify missing proprietary implementations and route them to the already-selected replacement adapters, never silently omit consumers.

### SE-003 — Original factory, data and lifecycle integration

Implement a Linux `CreateGameEngine` and original subsystem factories preserving `GameMain -> GameEngine::init/execute` ordering. Bind original FileSystem/LocalFileSystem/ArchiveFileSystem interfaces to validated BIG/VFS semantics and XDG writes. Initialize original INI, CSF, object/module factories and subsystem globals. Headless mode uses explicit null presentation services, not a substitute game loop. Verify real source initialization with owned retail configuration and diagnostic traces; test absent/malformed configuration, missing factories, initialization-stage failures and reverse shutdown. Verification/headless mode must not acquire a GPU/window. Integration wrappers must preserve ownership/lifetime and error propagation; do not maintain parallel authoritative globals.

### SE-004 — Original simulation and map execution

Compile and execute original GameLogic, map/chunk loading, terrain logic, object modules, scripting, AI, pathfinding, player/side setup, command/message routing and victory/defeat. Retain original update ordering and simulation rules. Load real corpus maps through original parsers; exercise a scripted mission and skirmish setup with actual object creation, commands, AI activity and measurable state changes. Test missing maps/modules, malformed chunks and invalid commands without partial-state success. No category manifest, toy entity loop or emitted success label qualifies. Headless deterministic checkpoints must be derived from original state and commands.

### SE-005 — Original W3D/WWShade presentation producers

Port original W3D/WWShade and GameClient rendering dependencies onto the renderer abstraction, including actual DX8Wrapper consumers. Audit actual state/material/resource semantics, not identifier coverage alone. Load real W3D/HLOD/animation/textures and map terrain; generate commands from original camera, terrain, object, lighting, fog/shroud, shadow, particle, water and effect producers. Recording-device tests must witness those producers and fail on unsupported required materials or missing assets. Then validate the same integrated scenes on the SDL_GPU Vulkan device with zero validation errors, resize/recreation tests and captured visual review. Synthetic shader scenes remain component evidence only. Preserve M14's backend fallback evidence gate if an actual required source behavior cannot be implemented.

### SE-006 — Original UI, input, audio and video adapters

Bind SDL events to original message translators, WND/gadget callbacks, command queues and camera controls; render the original menus/HUD with the font adapter and supplied locale. Bind original audio/video manager interfaces to miniaudio/FFmpeg, including event ownership, voice categories, positional audio, cinematic completion/skip and subtitles where the source uses them. Validate actual main-menu navigation, mission/skirmish selection, in-game command dispatch, pause/options and return-to-menu, with focus/resize/input failure and missing-media tests. A stand-alone synthetic menu or standalone audio probe is insufficient. All user writes remain in isolated XDG roots.

### SE-007 — Original snapshots, replay and determinism

Port original Xfer snapshot traversal, SaveGame, recorder/playback and CRC producers. Use shared bounded fixed-width codecs at these original boundaries and preserve source-established layouts as specified in the base plan. The existing toy `ZHSG` format may remain test-only; do not silently make it a game save or promise compatibility with it. Linux source-engine save/load, autosave and replay must restore actual objects, scripts, AI/player state and commands. Compare original-state checkpoints across GCC/Clang Debug/Release; test truncated/unknown-version saves, invalid references and replay mismatch with transactional load failure. Optional Windows fixture import remains separate and cannot block Linux-only acceptance.

### SE-008 — Original LAN integration

Connect original network command/message and packet/reliability consumers to POSIX transport and original simulation. Audit original field layouts, enum/bool/coordinate/UTF-16 encoding, timeouts and bounds. Integrate lobby, discovery/direct connect, map transfer where required by the original plan, start synchronization, lockstep/CRC and disconnect handling. Component UDP echo and the synthetic session are not game LAN. Acceptance uses local independent processes running the same original simulation, isolated XDG state, actual map/commands and fault injection (loss/reorder/duplicate/malformed/peer exit); preserve local-loopback fallback and no external-service requirement.

### SE-009 — Integrated retail gameplay and release reacceptance

After source providers are accepted, complete the original M15, M16 and M17 contracts on the integrated executable. Source-engine retail scene/GPU acceptance precedes playable acceptance. Cover actual campaign/skirmish, all factions, build/train/move/attack, AI and script behavior, victory/defeat, save/load/replay, audio/video/UI flows and the original duration/soak criteria. Record reproducible commands, logical assets, original-source runtime witnesses and observable outcomes. Validate original-engine multi-instance LAN, then clean packaged launch outside the source tree and the required distro/compiler matrix. Fail release if bootstrap/test simulation enters production linkage. Do not accept installed host dependencies as proof of secondary-distro portability. Windows compatibility remains optional M18.

### SE-010 — Correct historical assurance and delivery adoption

Retain M0-M14 completion history, commit references and test results. Add prominent machine-readable and human-readable assurance qualifications: historical component acceptance does not establish source-engine integration. Specifically reopen source assurance for support/startup, persistence, UI/world/effects, network and media adapters and real-scene GPU behavior through this remediation. Preserve useful M15 slices as scaffolding, but do not treat its blocked final slice as the only work left. Re-gate pending M15-M18 on applicable source providers while retaining stable IDs and unaffected scope. New acceptance must explicitly map each formerly unproven obligation to a real-source test.

The previous durable delivery goal has fixed membership M0-M18. Replanning must emit an explicit replacement-packet adoption instruction and pending execution scope; it must not silently mutate that old goal into a different scope or start implementation. On a later delivery resume, adopt the readiness-approved packet as the new remaining-work scope and preserve the prior goal/history for audit.

## Engineering order and readiness boundary

Source ownership informs original support closure; support precedes factory/data lifecycle; lifecycle enables original simulation and presentation adapters. Original state is required before snapshot/CRC or lockstep acceptance. Actual presentation producers and device validation, UI/media integration and persistence all precede playable acceptance; original LAN precedes LAN acceptance; integrated gameplay/LAN precede release. The compiler owns milestone grouping, IDs and graph, including any justified parallel branches.

The first source-port work must be executable with existing source, compilers, dependency setup and project-owned tests. Existing ATL compilation failures are work to implement, not an external blocker. Retail/display/GPU access is required only at its consuming integration checks. Readiness must verify current host prerequisites and record later external test environments with owners, without pretending those environments were tested. No implementation slices are authorized by this planning document alone.

## Evidence grades and anti-proxy checks

Every acceptance result names its grade: component/fixture, original-source compile/link, original-source integration, or retail runtime. Higher-grade claims require higher-grade evidence; a fixture count cannot substitute for them. Source lists and symbols alone cannot prove execution; runtime must demonstrate a source-owned state transition or output caused by real input. Required negative controls include missing original provider, missing/malformed actual asset, unsupported required render behavior, corrupt real save and mismatched real replay/peer state. Report exact failures, never fall back to toy simulation, placeholder assets or no-op managers in production to make acceptance pass.

No unresolved product decisions remain. Newly discovered required asset or backend limitations follow the base plan's evidence-gated escalation; do not invent silent substitutions. This supplement is ready for milestone compilation and readiness review, not a claim that the migration is already implemented.
