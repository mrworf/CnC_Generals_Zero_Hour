# Linux source-engine architecture revalidation

## Scope

Review at baseline `3c055cb997375d1181d851cdfeeb00f6ffa199b3`, requested by the owner on 2026-09-20. Revalidate native implementation against `docs/zero-hour-linux-port-plan.md`, especially original-engine preservation and M15's recorded failure. This is architecture and delivery remediation, not a change of game requirements or implementation acceptance. No retail content was modified.

## Executive Summary

The chosen Linux stack remains appropriate, but the build has not ported the original engine. The principal failure is implementation/acceptance drift from the existing plan, not a need to redesign gameplay. Historical green tests establish reusable component behavior; they do not establish original campaign, skirmish, rendering, persistence, or LAN integration. Resume delivery only through the source-engine migration supplement and a readiness-reviewed replacement packet.

## What Is Good

- Good: explicit data roots, bounded VFS, XDG paths and portable codecs provide useful adapter foundations (`CMakeLists.txt:114`, `src/data/vfs.cpp`, `src/persistence/save.cpp`). Preserve their negative tests and read-only retail policy.
- Good: the SDL_GPU backend now has real Vulkan validation evidence on the host RTX 4070. M14's corrected synthetic-scene result is useful backend evidence; GPU drivers are not the explanation for the missing gameplay (`evidence/qa/cnc-generals-zero-hour/M15-single-player.md`, M14 evidence and commit `2e0df9c`).
- Good: the original architecture already supplies an engine factory and central lifecycle. `GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp:40-44` calls `CreateGameEngine`, `init`, and `execute`. Keep these semantics and adapt platform services instead of inventing another simulation.

## What Is Bad Or Risky

1. Critical — target names conceal missing code. `CMakeLists.txt:70` stores source lists as a custom inventory property; lines 73-77 and 107 instantiate bootstrap components for compression, Westwood support, W3D, WWShade, GameEngine and GameEngineDevice. Inventory is not compilation or linkage. The M15 evidence records 586 engine and 259 GameLogic translation units absent from the executable.
2. Critical — passing simulation/save tests do not exercise the game. `src/simulation/determinism.cpp:34-38` creates Alpha/Bravo entities; `CMakeLists.txt:118-129` routes the new single-player flow through this simulation and custom persistence. `src/persistence/save.cpp:14` defines `ZHSG`. These are harness implementations, not the original `GameLogic::startNewGame` / `update` (`GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:1105,3572`) or the original Xfer graph.
3. High — platform portability is unproven at the source boundary. `GeneralsMD/Code/GameEngine/Include/Precompiled/PreRTS.h:44` unconditionally includes ATL; the factory in `GeneralsMD/Code/Main/WinMain.cpp:1107` is Windows-specific. Standalone native types and managers cannot remove these dependencies without original-source migration and factories.
4. High — rendering and UX labels overstate integration. `src/world/corpus.cpp` validates manifest categories, not a real map-to-W3D scene. New UI/world/effect commands and synthetic GPU scenes do not prove the original WND/gadget, terrain, animation, material or camera producers call the backend. Retail file readability is necessary but not execution.
5. High — historical milestone acceptance conflates component and product evidence. M1-M14 records must remain auditable, but their completed status cannot discharge missing source-integration obligations. M16 and M17 must not consume the synthetic coordinator as the game.

## What Should Change

Change: adopt `docs/zero-hour-source-engine-migration.md` as the implementation-ready engineering supplement. It requires explicit source ownership, real compiled/link/runtime witnesses, original subsystem adapters, original simulation/serialization/network integration, and retail-backed acceptance. This closes the observed boundary failures without discarding useful work.

Change: compile a dependency-ordered remediation delta, retain historical milestone identities and commits, and re-gate pending single-player/LAN/release work. Record a new delivery-scope adoption handoff rather than silently adding milestones to the old fixed-membership goal. Each source-integration claim must fail if its source implementation is replaced with a stub; fixture tests remain separately identified.

Change: permit incremental source compilation with explicit unresolved dependency diagnostics. Full source-engine compilation is a deliverable, not an environmental prerequisite to starting that work. Missing interfaces must not be satisfied by production no-op methods that hide absent behavior.

## What I Would Not Change Yet

Do not change SDL3/SDL_GPU, miniaudio, FFmpeg, FreeType/Fontconfig, x86-64-only scope, the original simulation model, INI/module architecture, or game UI semantics. The evidence indicates missing integration, not that these choices failed. Do not require Windows fixtures for the Linux release or add ARM64. Do not rewrite source algorithms to fit the toy snapshot. Keep synthetic fixtures as fast component tests, but separate them from production acceptance.

## Overall Opinion

Architecture revalidated with mandatory source-integration remediation. The original product direction is settled; no owner decision is needed to restore its implementation. The game is not currently playable. Documentation/source inspection and the existing M15 failure evidence support this finding; no new gameplay or cross-distribution acceptance is claimed by this review. Delivery planning can proceed from the supplement; implementation must wait for its readiness-approved handoff.
