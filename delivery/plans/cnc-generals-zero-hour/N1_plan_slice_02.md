# N1 slice 02 — capacity and lifecycle qualification

## Goal and scope

The stock renderer handles source-derived scene demand and repeated lifetimes
without omitted draws, accumulating ownership or unwanted simulation progress.
Depends on slice 01's accepted public APIs and physical semantic tests.
No proprietary input or original world integration is necessary for these fixtures.

## State, interfaces and ownership

Build a generated scene workload census from original terrain/material/camera
source consumers, then drive loading→ready→draw→replace/cancel→resize→shutdown.
Use game-owned resource tracking, public capabilities/stats and public completion
semantics. Persist source holders until release and readback destinations until
ready. Never equate a release callback with GPU completion or force private arena
shrink. No GPU transaction rollback contract is introduced.

## Surfaces and implementation choices

Extend the generated fixtures, source requirement/census manifest, runner and
evidence. Select resource batching and public capacity settings from the observed
complete workload; preserve independent maximum dimensions/cardinality/bytes.
Game-owned allocations may be bounded; backend diagnostics must remain truthful
estimates when upstream provides estimates. No engine/thread authority changes.

## Validation and acceptance

Test repeated generations/context teardown and mixed/all-active frames; compare
expected draw counts and physical outputs, not just handle counts. Exercise
invalid capacities, canceled preparations, stale/destroyed handles at our own
admission boundary, and corrected retries without issuing invalid upstream calls.
Require GCC/Clang sanitizer ownership checks, Vulkan validation, stable resource
use after completed lifetimes, and meaningful workload timing/memory evidence.
Run the full N1 suite on frozen source and record all unexecuted checks explicitly.

An upstream public capability gap stops qualification and triggers architecture
reassessment. A watchdog timeout remains incomplete execution. Commit this slice
and mark N1 accepted only when the complete governing gate passes.

## Implementation detail established before edits

Persist the source census in `docs/renderer-workload-census.md`, with formulas,
reachable consumers and explicit limits on claims. Use a 1024-cell terrain
pressure workload (4096 16-cell tiles): real grid vertex/index backing, distinct
128-square base textures and two ordered passes. Upload in bounded loading
batches without any simulation clock. Add source-sized higher-resolution LOD,
tree/bridge/particle buffers and multi-consumer draw pressure; verify all draw
identities through physical pixel coverage plus public submission statistics.
This workload is a qualification example, not a newly imposed game/map limit.

Keep capacity, upload and lifecycle tests as independent manifest-registered
families so high-water arena allocations need not coexist artificially.
Exercise repeated scene replacement, cancel-before-publication, partial failure,
changed buffer/texture data, generation-based stale-token rejection and retry.
Retain upload holders until supported release callbacks, verify release counts
and callback thread-safe ownership, and compare settled public resource counts
against the same owner's baseline. Public GPU memory estimates are diagnostics;
do not interpret them as exact allocator inventories or demand arena shrink.

Exercise SDL3 native-window Vulkan presentation and real resize with public
bgfx swap-chain configuration; verify offscreen targets after every resize as
well. Use supported SDL3 properties for the selected X11/Wayland environment.
Run normal-host functionality and both owner sanitizer variants with the
documented isolated-bus option. Canonical validation remains CMake/Ninja/CTest
from `docs/build.md`, expanded with these named families. Stock Clang runtime
and Release configuration verification may be added as separate build variants;
do not label a GCC-built dependency as a Clang-built one.

All added code, tests, census findings, build instructions, evidence and delivery
status form this one coherent slice commit after required acceptance passes.
If source investigation reveals a mandatory public API gap, persist the failed
gate and architecture handoff rather than accepting incomplete evidence.

The first complete family batch identified fixture/adaptation corrections:
stock handle limits are 4096, including other consumers. Group 32 terrain tiles
into one vertex/index owner and one texture array, preserving each tile's layer,
index range and authored bytes. Mutable textures must be created without initial
memory, then uploaded through updateTexture2D; memory-backed creation is immutable.
Native SwapChain requires an explicit supported color format. Public requested
draw peak is compared with the exact fixture census; completed physical witnesses
prove no drop. Rotating previous-frame numDraw is reported only as a diagnostic,
not sampled at an invented fixed-latency frame. Keep all corrections in this slice.

The frozen sanitizer batch passed every headless family, but native window
teardown reported 224 bytes after the fixture repeatedly initialized/quit SDL
video. Correct the service lifetime to one SDL owner surrounding three window
and bgfx contexts, matching a real process. Add an initialization-only native
window control to classify residual host-stack evidence without dropping the
full resize and rendering coverage. Leak detection stays enabled.

The initialization-only control reproduced the residual. Loader diagnostics
locate allocation return addresses in unloaded NVIDIA driver mappings, not
fixture scene owners. Pair the documented SDL_Vulkan_LoadLibrary/UnloadLibrary
public service ownership around all windows (including constructor failure
cleanup); validate before deciding whether this addresses repeated loader
retirement. Do not pin a driver library or suppress its allocations.

## Completion

Completed with the commit introducing
`evidence/qa/N1-stock-renderer-capacity-lifecycle.md`. Normal-host and both owner
sanitizer matrices passed 18/18 on frozen source. Balanced public loader ownership
resolved the native-window control and full resize case. All seven N1 gates map
to physical evidence; no dependency changes or retail reads. Scope limitations
remain explicit in the census/evidence. Commit boundary is this complete
qualification slice, including N1 completion and the next delivery resume point.
