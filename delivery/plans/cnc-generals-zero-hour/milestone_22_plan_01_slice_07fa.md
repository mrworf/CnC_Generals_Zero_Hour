# M22 plan 01 slice 07FA: map-frame shroud and tactical traversal

## Outcome and dependency

Requires 07F9. Restore the source map-frame route needed before any map-loaded
scene can be observed: the bounded `W3DShroud::render` projection/update,
stationary map `W3DView` update, and map-aware `W3DDisplay`/scene traversal
through Recording. The terrain owner is the 07F9 source owner and emits its
existing 07F8 passes; no adapter terrain draw is permitted.

## Boundaries

The native display updates shroud before views, and the native view/display
branches currently reject a loaded map. Implement only the default, one-view,
no-filter, no-motion camera state and the source shroud state needed for that
frame. Unsupported movement, reflections, extra passes and unloaded/stale
shroud state must reject before a successful frame. This remains Recording
only; public Vulkan pixels and resize are M22 09.

## Acceptance

Prove source-identified ordering of shroud update then terrain submission,
no-frame/missing-camera/stale-shroud rejection, injected projection/draw abort
and retry, map unload/reload and zero teardown. Include source identity,
provider-removal and canonical sanitizer controls. Active tracks, water,
shadows and effects stay outside this child.
