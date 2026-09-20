# Original runtime architecture reconciliation

## Scope

Owner-requested reconciliation of `M20_SOURCE_ORDER_DEFECT` at clean baseline `5f4949eaacb8e6836cdd0f2c190f10659db09099`. Reviewed original GameEngine lifecycle and factory declarations, M19 implementation/evidence, M20 blocker and migration authority. This review changes engineering work allocation, not game requirements.

## Executive Summary

The defect is real: M20 requires runtime providers assigned to its descendants. The previous readiness review checked milestone ordering without sufficiently tracing original registry, update and destruction dependencies. Resolve it by assigning the whole required runtime implementation closure to a shared provider before lifecycle acceptance. Later domain milestones retain their observable behavior tests and work beyond that closure. The resolved authority is `docs/zero-hour-runtime-closure-reconciliation.md`.

## What Is Good

Good: M19 now compiles ten original support translation units and preserves source changes in reviewable slices. Its component and bounded-runtime tests are reusable (`CMakeLists.txt`, `src/original_support/ww_support.cpp`). Good: delivery rejected the reduced GameEngine before retaining production changes; current source still exposes the real dependency chain.

## What Is Bad Or Risky

Risky: startup and module registration were treated as independent of simulation/client code. `GameEngine.cpp:447,482,493,504–512` creates the module/object factories, GameClient, AI, GameLogic, recorder and radar. `Win32GameEngine.h:93–107` binds those abstractions to concrete W3D and audio providers. Registry compilation requires implementations, not just forward declarations.

Risky: stopping the audit at M23 would reproduce the problem. `GameEngine.cpp:539–553` creates GameState and runs post-load traversal; `update:755–786` calls radar, audio, client, messages and GameLogic. Recorder/save support must therefore enter the initial closure. Network is different: source explicitly initializes its singleton to null outside multiplayer (`GameEngine.cpp:565–566`), so offline startup does not require complete LAN acceptance.

Risky: source filenames and test strings do not prove preserved semantics. The rejected alternative class would pass shallow identity checks while bypassing original factories. `src/original_support/ww_support.cpp` also tests UTF-16/rounding via foundation helpers; M19 is not evidence that original UnicodeString and memory-pool call sites are portable. `PreRTS.h:44` still imports ATL. The new scope must include that remaining source support work.

The earlier blocker report's GameEngine line numbers were collected while a temporary branch shifted them. Current source anchors are 447 (ModuleFactory), 482 (ThingFactory), 493 (GameClient), 504–505 (AI/GameLogic). The dependency finding remains valid; this review supplies corrected references.

## What Should Change

Change: use one integrated implementation owner for every concrete dependency required by original startup, update and teardown, with explicit permission to port their shared source support. This removes the authority conflict that prohibited required implementation in M20. Compiler grouping must follow that closure, with domain acceptance afterward. Implementation may grow its proven source set incrementally; the final lifecycle gate cannot pass until the real closure runs.

Change: validate source semantics, registry behavior and original state transitions alongside compiler/link evidence. Keep null/recording behavior at device boundaries and retain original class ownership. Preserve existing read-only retail and XDG rules, including removal of source startup's `DeleteFile` operation from the Linux path.

## What I Would Not Change Yet

Do not replace the engine architecture, selected libraries, game rules or x86-64 platform scope: no evidence implicates those choices. Do not rewrite M19 history or repeat its successful support tests as a planning ritual. Do not require a complete multiplayer match or hardware renderer to prove the source-defined offline startup mode.

## Overall Opinion

Architecture reconciled through RC-001–RC-005 with no unresolved product decision. Delivery replanning is authorized from the committed resolution, and must validate semantic provider ownership as well as graph order. This documentation review makes no new build/gameplay acceptance claim; executable migration remains future work.
