# N2 slice 03 — actual original headless runtime

## Goal and dependencies

Original GameEngine startup reaches original GameLogic and executes deterministic
simulation on Linux, then shuts down and retries in the same process. Depends on
core owners and rooted original consumers. This is the N2 acceptance boundary;
core-only/RNG-only tests are not enough. Full rendering/media/match completion
are later milestones and must not be claimed here.

## Runtime and planned surfaces

Derive explicit CMake source graph from original GameEngine/GameEngineDevice
projects and reachable startup/object/module dependencies. Port shared native
services and extract settled presentation/platform seams without changing
simulation: headless output services may omit output, never replace logic.
Retain original GameEngine ordering, original object/module updates, original
map/data interpretation, fixed simulation time and RNG streams. Audit startup
failure/teardown across every constructed owner before retail execution.

Generate a complete legal minimal simulation fixture whose source setup reaches
original startup, object creation/update, and reproducible state checkpoints.
Use source-defined actor behavior, not an independently written toy. A native
entry point accepts explicit roots, headless mode and bounded test ticks.
Checkpoint outputs contain no private input-derived identifiers. Read-only retail
startup validates supplied content and integrity, with redacted error/status only.

## Tests and completion gate

Carry forward the reader slice's explicit remaining startup services: XDG
user/cache writers separated from immutable asset mounts; rooted MemoryPools.ini
profile loading; native FileInfo timestamp semantics used by map-cache owners;
complete original INI block table; non-BMP filter replacement-width compatibility.
The slice02 stage4/mask63 retail probe is only container/basic-family/text evidence,
not acceptance of these services or deterministic GameLogic execution.

Normal, GCC and Clang original executables run the same fixture with identical
checkpoints. Test independent presentation omission, multiple startup/run/shutdown
cycles, interrupted startup/late decode failure, missing roots/malformed input,
and corrected retry. All live owned resources settle; process-global initialized
services are measured separately. ASan/UBSan/leak detection remains enabled.
Record source→entry→GameLogic/object reachability and compiler/build identity.

Run the frozen complete applicable CTest matrix, required asset-integrity checks,
and artifact/authority freshness checks. Every N2 acceptance criterion maps to
direct evidence; caches/writes stay outside supplied roots. Commit the completed
slice and N2 delivery status. If required private data is absent, retain passing
generated evidence and precise PRE04 unblock condition rather than accepting N2.
