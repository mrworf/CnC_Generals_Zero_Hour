# M22 plan 01 slice 07FA: map-frame shroud and tactical traversal

## Outcome and dependency

Requires 07F9A. Restore the source map-frame route needed before any map-loaded
scene can be observed: the bounded `W3DShroud::render` projection/update,
stationary map `W3DView` update, and map-aware `W3DDisplay`/scene traversal
through Recording. The terrain owner is the 07F9 source owner and emits its
existing 07F8 passes; no adapter terrain draw is permitted.

## Boundaries

The native display updates shroud before views, and the native view/display
branches currently reject a loaded map. 07F9A must first own the projected
shroud texture consumed by the source scene material pass. Implement only the default, one-view,
no-filter, no-motion camera state and the source shroud state needed for that
frame. The established published-but-unloaded terrain owner remains an
accepted no-op update/presentation path; a shroud projection is required only
for the loaded-map frame. Unsupported movement, reflections, extra passes and
stale loaded-map shroud state must reject before a successful frame. This remains Recording
only; public Vulkan pixels and resize are M22 09.

## Acceptance

Prove source-identified ordering of shroud update then terrain submission,
no-frame/missing-camera/stale-shroud rejection, injected projection/draw abort
and retry, map unload/reload and zero teardown. Include source identity,
provider-removal and canonical sanitizer controls. Active tracks, water,
shadows and effects stay outside this child.

## Delivered implementation

`W3DDisplay` now owns `W3DShaderManager` from successful WW3D init through
pre-WW3D shutdown. The Recording-only map frame has exact source order:
shroud projection, stationary view terrain-center update, then primary-scene
terrain traversal and the existing two terrain passes. The scene accepts only
the currently published map terrain in its primary scene; detached terrain,
foreign siblings and active water reject. A standalone atlas probe keeps its
own explicit shader-manager fixture because it deliberately has no display.

The dedicated generated read-only map probe covers initial full update,
stationary re-entry, injected draw abort/retry, shroud filter rejection/retry,
terrain detach/re-attach, active-water rejection, two generations and zero
resources after edge teardown. Existing map-less display/view fixtures now
explicitly prove a no-op terrain update, and the legacy shroud-material probe
proves it cannot retain a stale fixture edge. Retail and Vulkan remain out of
scope.

## Validation

- Focused coupled GCC and Clang Debug controls: 10/10 each, including ledger,
  map-frame, terrain, presentation, identity and provider-removal controls.
- Full non-GPU/non-retail/non-LAN suites: GCC Debug 200/200, GCC Release
  200/200, Clang Debug 200/200, Clang Release 200/200, GCC sanitized 200/200,
  and Clang sanitized 200/200. Sandbox sanitizer runs set
  `detect_leaks=0` only because its ptrace wrapper prevents LSan.
- Host LeakSanitizer (`detect_leaks=1`) focused coupled suite: 7/7 with both
  GCC and Clang sanitized builds.
- Host serial LAN acceptance: 4/4 in each of the six configured builds.

## Commit boundary

The independently reviewable payload is this slice commit.
