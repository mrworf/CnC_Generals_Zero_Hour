# M22 plan 01 slice 08J1: detached disabled-water map continuation

## Goal and boundary

Restore the previously passing generated physical factory-map control after
08J's active-water correction. `W3DDisplay::reset` detaches all scene objects,
including the published disabled-water owner. `W3DTerrainVisual::load` now
rejects that owner before map binding because its reset-detached predicate is
specific to enabled water. The disabled owner deliberately has no GPU buffers
and retains the CPU `resourcesPending` sentinel; treating it as active water
would weaken both lifecycle contracts.

Add a separate exact-state disabled-water predicate and map-load continuation
for the original terrain visual and its own water render object. Require the
published display/scene, GPU edge, empty terrain/map state, exact terrain and
water identities, detached water, disabled plane/cloud and zero extents, and
the expected absent disabled-water GPU resources. Do not depend on generated
or retail selector variables. Keep the accepted active-water predicate,
resource readiness, update, frame and reattachment unchanged.

On a successful generated map load, attach terrain first and the same
disabled-water owner second, without creating water GPU resources or a water
draw command. The native no-river map override call is a checked no-op for
that exact attached disabled-water owner; detached, foreign or malformed
owners still reject. On missing/malformed map, failed terrain binding, scene
attachment or later setup, remove only links created by this attempt and
return to the exact detached, mapless retry state. Reject foreign/missing or
already-attached water, pending active-water resources, malformed disabled
configuration and stale terrain without mutation. Preserve accepted
active-water retry and all existing map/bridge/tree owners.

## Validation and commit

Extend generated source-owner tests for disabled-water preflight, source
scene-link order, fault rollback, retry, two generations and zero immediate
Recording residual. Re-run the existing physical Vulkan
`original_w3d_factory_map` control with validation enabled, including its
absent/missing/malformed/failure cases; do not mark this slice complete while
that test fails. Retain physical display/bootstrap and active-water controls.
Run six complete GCC/Clang Debug, Release and ASan+UBSan builds and canonical
nonretail suites, strict host LSan focus, serial host LAN 4/4 each six,
dependency ledgers and diff checks on final source. No retail data, original
symlink change, renderer diagnostic change or tree-provider admission.

One independently reviewable implementation commit:
`delivery: M22 08J1 restore disabled-water map continuation`.
