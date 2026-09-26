# M22 plan 01 slice 08P0C1B3A: position FX graph admission

## Goal and boundary

Before any tree FX dispatch, establish a side-effect-free, read-only
capability check for the entire `FXList` graph reachable from a positional
call. Own `FXList`/`FXListStore` pointer liveness, recursive graph traversal,
position-call compatibility and provider readiness. Do not dispatch any
nugget, advance RNG, alter a tree or implement dynamic light-pulse rendering.
B3B consumes this admission result; C2/C3/C4 remain closed.

## Source ownership and failure contract

Native `FXList::doFXPos` shroud-gates then immediately iterates nuggets.
Sound uses Audio; tracer/ray require a secondary position and ThingFactory,
ray creation also uses GameClient; light pulse uses Display (the CPU-only
`W3DDisplay::createLightPulse` is pending); view shake optionally uses
TacticalView; scorch uses GameClient RNG and scorch creation; particle
systems use ParticleSystemManager, GameClient RNG and optional TerrainLogic;
`FXListAtBonePos` is object-only and holds another `FXList` pointer.
Traverse every reachable nested list before dispatch, validating each
non-null pointer against its live store before dereference, with an explicit
finite depth/node bound and cycle rejection. The tree's absent optional FX
root is a no-op, not an invalid pointer. Reject foreign/stale list
identities, missing required providers, invalid primary position, unknown
or object-only nugget kinds, positional tracer/ray without a secondary, and
CPU-only light-pulse requests. A later unsupported nugget must reject the
entire list before an earlier nugget can act. Keep the original source
dispatch order and do not change `FXList`/`FXNugget` object layout unless an
ABI audit proves a narrowly scoped need.

## Acceptance and commit

Generated positives: empty, one and multi-nugget compatible positional
lists, exact order and no-effect readiness query. The source's only nested
list nugget is object-only for a positional call: traverse its acyclic
reference chain safely, then reject it before effects. Negatives: late
unsupported nugget, absent provider, foreign/stale nested pointer,
self/cross-list cycle, over-depth/node graph,
nonfinite primary and object-only/light-pulse categories. Every rejection
leaves Audio/GameClient/GameLogic RNG, tree/frame/GPU and external effect
counts unchanged, and a clean retry after provider restoration succeeds.
Run focused GCC/Clang and sanitizer witnesses, six complete builds/canonical
nonretail suites, strict host LSan, physical Vulkan, serial LAN 4/4 all six,
ledger and diff checks. Commit one slice:
`delivery: M22 08P0C1B3A admit position FX graph`.

## Delivered evidence

The Linux-only source preflight validates live store identities before
dereference, recursively checks non-owning list edges with active-stack
cycles and explicit depth/node bounds, and rejects unsupported positional
categories before any dispatch. Exact ThingFactory membership peeks avoid
the test-asset lookup's synthesizing branch; PlayerList's nullable peek
preserves its asserting getter. No object fields or Windows declarations
change, and both minimal/headless and full-draw provider consumers link.
Generated null/empty/multi, DAG/cycle/bounds, foreign/stale, provider,
tracer/ray/template, object-only/light-pulse and unknown/null controls pass
with zero dispatch and unchanged factory identity, Client/Logic RNG, frame,
GPU resources and audio owner. Distinct pooled edge nuggets are cleared
before each of two terrain generations tears down. Six complete builds and
canonical suites, focused GCC/Clang native and sanitizer witnesses, strict
host LSan, physical Vulkan, serial host LAN, ledger and diff checks pass;
see linked evidence in the governing index. B3B dispatch remains closed.
