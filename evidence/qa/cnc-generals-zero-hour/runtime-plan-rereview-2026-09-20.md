# Runtime plan second review

## Scope

Reviewed the committed architecture resolution and M20–M25 packet at `98e62060d8250c75cfe595b5299629cba3878ddc`, with original GameClient/GameLogic/GameEngine, subsystem ownership, command-line profile controls and existing identity tests. The user authorized corrections to this planning work. No product implementation is changed by this review.

## Executive Summary

The integrated runtime owner fixes the earlier backward dependency. Retain it. The practical gaps are the undefined headless startup profile, hidden UI/assets required during reset, cleanup before subsystem registration, and limits in existing identity/sanitizer evidence. RC-006 in the architecture authority resolves these gaps before delivery resumes.

## What Is Good

Good: M20 owns required implementations across simulation, client, registries and save state; later milestones retain complete domain acceptance. There is no longer a rule requiring a missing provider to arrive from a dependent milestone. Explicit ownership is useful even though the integration milestone is large. The plan permits incremental slices and does not require complete runtime validation before support work starts.

## What Is Bad Or Risky

- Risky: generic headless initialization may implicitly play movies or load a shell map. `GameClient.cpp:532–581` requests these during updates; the existing `-noshellmap` and debug-only logo controls have different availability. All four presets need the same explicit test semantics.
- Risky: reset itself loads `Menus/BlankWindow.wnd` (`GameEngine.cpp:713–720`). The plan must preserve real CPU UI/parser/resource handling even with no GPU, and distinguish owned fixture assets from required retail assets.
- Risky: initialization pushes a subsystem into the cleanup list only after `init` and INI loading (`SubsystemInterface.cpp:159–172`). A reverse-order list alone cannot clean a failed pending object. The engine destructor also assumes some late globals exist. Failure ownership requires narrow repairs and direct tests.
- Risky: `tools/check_original_identity.py` searches source suffixes and object basenames anywhere in a link map, then matches a witness string. That does not prove a particular target contributed live original code or state. The integration gate needs stronger checks.
- Risky: `CMakeLists.txt:100` forces NDEBUG for existing WW support; leak detection was disabled for the prior sanitizer run. Neither fact invalidates its bounded recorded evidence, but neither should be used to claim all original debug invariants or leak checks passed.

Source filenames abbreviated above are under `GeneralsMD/Code/GameEngine/Source/`, with GameClient in `GameClient/`, GameEngine in `Common/`, and SubsystemInterface in `Common/System/`.

## What Should Change

Change: add RC-006 to define the bounded test profile, actual state observations, resource prerequisites, pending-object rollback and accurate debug/sanitizer evidence. Propagate it into M20 and readiness. Keep registry completeness and original methods; the profile exercises the source-defined no-match path with original updates and reset. Runtime implementation must establish exact expected values from the ported original path.

## What I Would Not Change Yet

Do not add another layer of milestones just to defer hard compile work: the identified consumers still need one integrated acceptance boundary. Do not reopen M19 completion or broaden this review into implementation fixes. Do not weaken later map, GPU, media, persistence, LAN or release acceptance.

## Overall Opinion

The approach makes sense with RC-006 incorporated. It is executable as an incremental source port, but remains substantial work; readiness approves starting it, not its duration or success. Recompile and recheck the affected packet after the source update, then hand off M20 plan 02. Existing plan 01 remains superseded.
