# Original dependency deep review

## Scope

Baseline `e72587449d6bac95682126532b73e901ec47bbf3`. Owner requested another dependency/behavior review and permits adding/reordering unfinished milestones. Read-only source audit; no retail traversal, product implementation or acceptance execution. Paths below are under `GeneralsMD/Code/`. Current plan already owns generic runtime closure; this review tests concrete phase and side-effect assumptions, not just the milestone DAG.

## Executive Summary

The single integrated acceptance boundary is useful, but the single large implementation owner concealed independently provable prerequisites. Split where real original-consumer tests are possible, and retain final coupled engine acceptance. New source findings include pre-GameMain allocation setup, map metadata and writes during startup, a music/CD prompt loop, Release-only fingerprint quits, swallowed initialization errors, and CPU/device work interleaved in W3D startup. RC-007–010 resolves these engineering requirements without changing product scope.

## What Is Good

Good: preserving original classes, forbidding later-domain prerequisites and requiring original state witnesses prevents the prior reduced-engine workaround. Existing native components and bounded M19 remain reusable. Separate device/session and retail acceptance gates are appropriate, provided null devices do not erase CPU behavior.

## What Is Bad Or Risky

This seed dependency ledger is source-inspected, not compile/link/runtime-proven. `GE` means `GameEngine/Source/`; `GD` means `GameEngineDevice/Source/`. Owning capability is an architecture allocation for compilation, not a preassigned milestone ID.

| Consumer / evidence | Hidden edge or behavior | Required capability / validation |
|---|---|---|
| `Main/WinMain.cpp:893–897,1001–1009,1067–1098` | Critical sections, logging, memory manager and Version precede GameMain; teardown follows it | Process foundation; original allocation before/after entry, ordered cleanup, no CWD/window side effects |
| `GE/Common/System/GameMemory.cpp:3418–3545` | New/delete linkage self-test, pre-main allocation, conditional skipped shutdown | Original allocator provider; cross-target free, aligned/pointer-safe allocations, static lifetime tests and honest live counts |
| `GE/Common/System/Xfer.cpp:112–209`; `GE/Common/RandomValue.cpp:145–156` | Host type sizes enter serialized/CRC data, client/logic RNG independence matters | Original codec/ABI provider; exact owned bytes and stream isolation; later replay/LAN integration |
| `GE/Common/INI/INI.cpp:353`; `GE/GameLogic/System/GameLogic.cpp:198–220` | INI and map cache call setFPMode defined in the large GameLogic translation unit; actual code chooses NEAR/24-bit despite CHOP comment | Lower support must extract/port the one original helper through permitted shared-method refactoring; no whole-GameLogic dependency or duplicate definition; characterize actual caller rounding |
| `GE/Common/INI/INI.cpp:217–259` | Directory load is two passes: sorted root-level INIs, then nested files | Original data tests must preserve this precedence, not replace it with a single global lexical sort |
| `GE/Common/INI/INI.cpp:78–160` | Static theTypeTable references callbacks across AI/UI/audio/etc. regardless of fixture contents | Bounded data tests require narrow shared original parser-core/field extraction; preserve full production dispatch at integrated acceptance. No missing-callback stubs, garbage-collection trick or reduced production registry |
| `GE/Common/GameEngine.cpp:349–418,453–482` | Global data before CLI/LOD; real INI callbacks and object registrations establish order | Original data services plus integrated registry; original parser precedence and required provider failure |
| `GE/Common/GameLOD.cpp:267–309` | Options/preset loading, hardware probe, optional benchmark write before display | Linux capability/config adapters; isolated writes, bounded deterministic test profile |
| `GE/Common/Audio/GameAudio.cpp:218–263,963–990`; `GE/Common/GameEngine.cpp:435–436` | Music absence enters CD/modal loop and requests quit even without audio hardware | Original audio parsing and resource checks; valid owned music fixture and bounded missing-resource error |
| `GE/Common/GameEngine.cpp:581–607` | Release-only archive fingerprint checks set quitting | Remove excluded protection consistently; separately prove real resource failures |
| `GE/Common/GameEngine.cpp:607–698` | Cache update, .map/.rep initial dispatch, error handlers then reset | Integrated init; explicit no-match profile and failures must not fall through to reset/execute |
| `GE/GameClient/MapUtil.cpp:238–278,459–477,702–715` | Startup cache parses chunks/objects/height metadata and CRC; Debug rebuilds standard cache | Original metadata parser before full scenario milestone; malformed map and warm/cold equivalence |
| `GE/GameClient/MapUtil.cpp:129–175,702–750`; `GE/Common/INI/INIMapCache.cpp:122–203` | Object metadata calls ThingFactory/findTemplate and MapObject; localized cache refresh uses GameText/map.str and name keys | Lower provider owns reusable original parsing/cache-path pieces; integrated owner proves full registry-backed classification/localization and cache refresh. Empty fixture maps cannot prove this closure |
| `GE/GameClient/MapUtil.cpp:372–394,504–575` | Direct fopen cache writes; standard/user precedence and recursion | XDG write routing, read-only roots, denied-write tests and no committed retail checksums |
| `GD/W3DDevice/GameClient/W3DDisplay.cpp:655–835` | Real scenes/lights/asset loaders interleaved with WW3D/DX device and browser initialization | Original CPU-resource provider plus native device seam; recording-device state/ownership tests, later hardware scene |
| `GE/GameClient/GameClient.cpp:249–430` | Translators, image/animation/font/window/IME/shell/campaign/video setup is eager | Original CPU UI and callbacks before headless integration; device absence cannot skip managers |
| `GE/Common/System/SubsystemInterface.cpp:159–172`; `GE/Common/GameEngine.cpp:674–704` | Registration follows init; some caught errors can proceed toward dereferences/reset | Pending-owner rollback plus explicit propagation through native entry, exactly-once destruction |
| `GE/GameLogic/System/GameLogic.cpp:1291–1294,1871,2016–2034` | Map start calls terrain props, client asset preloading and recorder controls, not just simulation | M21 must own extra scenario CPU paths or consume earlier CPU providers; cannot wait for M22/M23 hardware/interaction acceptance |
| `GE/Common/System/SaveGame/GameState.cpp:652–765` | Resets before reading, postprocesses after a read error, resets again and opens an error dialog | M24 must prove no mixed/partially published state and headless diagnostics; source reset alone is not transactional-load proof |
| `GE/Common/Recorder.cpp:1053–1164` | Playback mode set before header validation; DEBUG_LOGGING checks build/version/exe identity; MSG_NEW_GAME and seed dispatch follow | M24 invalid-header mode rollback and cross-preset replay compatibility must exercise these actual branches, not only codec tests |
| `GE/GameNetwork/Network.cpp:372–384` | CRC mismatch calls script actions, loads `Menus/CRCMismatch.wnd`, starts timer and logs recorder/RNG state | M25 negative path needs original CPU UI/script/recorder resources even on headless loopback; no later device dependency |

Risky: calling everything “M20 work” resolves allocation but gives weak early feedback. Prior audits stopped too close to subsystem registration; initialization continues into cache/resource handling, while prerequisites also precede GameMain. Windows-specific work is not confined to platform folders. Debug and Release have materially different startup branches.

Risky: filename classification and text in link maps cannot prove selected branches, live providers or callback coverage. Static inspection cannot enumerate all data-driven registry invocations. Discovery must continue during builds/tests with an owned ledger that fails for unassigned required providers rather than repeated broad replanning.

## What Should Change

Change: RC-007 authorizes independently testable prerequisite milestones and a source/target/phase/asset/write/test ledger with honest evidence grades. RC-008 requires real bootstrap, allocator, codec and data-consumer proof. RC-009 owns map-cache metadata/writes, music gating, LOD and failure propagation before complete initialization. RC-010 separates CPU resources from devices and requires later map/save/replay/network/return-to-menu edges to be tracked at their actual consumers. Compiler chooses IDs/grouping; no milestone may defer an implementation needed by an earlier accepted provider.

Tests should run owned source fixtures before private-data acceptance: cold/warm map caches, malformed chunks, missing audio resource, non-D3D init failure, read-only data roots and denied XDG writes, static allocation lifetime, original codec bytes, client/logic RNG isolation, CPU scene/resource construction and teardown. These are future implementation gates, not passing results of this review.

## What I Would Not Change Yet

Do not change completed milestones, gameplay rules, target OS/architecture, chosen backends or optional Windows compatibility. Do not split tightly coupled module/vtable closure into mutually dependent domain milestones. Do not replace original parsers/managers with fixtures in production, disable real checks, or claim all unknown dependencies are eliminated. Keep real hardware/playable/save/LAN/release acceptance independent of headless startup success.

## Overall Opinion

The plan should move from a broad catch-all runtime task to independently evidenced prerequisites feeding a coupled integration gate. This review finds specific work that was previously only implied and a repeatable way to detect additional edges during implementation. Recompile the unfinished packet and independently audit the actual provider boundaries before handing off; planning does not establish that the engine builds or runs.
