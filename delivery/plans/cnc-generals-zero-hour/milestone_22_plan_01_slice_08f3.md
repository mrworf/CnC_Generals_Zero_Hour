# M22 plan 01 slice 08F3: original water map-override owner prerequisite

## Goal and source boundary

Close the missing original `WaterRenderObjClass::updateMapOverrides` CPU owner
method reached by native `W3DTerrainVisual::load` after terrain and water are
attached to the primary scene. Native source calls `enableWaterGrid(FALSE)`
then `updateMapOverrides`; the latter compares an already-owned river texture
with parsed water transparency settings and replaces it only when changed.
The accepted bounded Linux water owner publishes no river texture, so its
source-equivalent result is an owner-checked no-op, not a general texture
override implementation. The method currently has no Linux CPU definition.

## Scope and non-scope

Implement only the original water owner's map-override method for the
published, scene-attached, resource-ready generated active-water owner.
Reject missing/foreign water or scene providers, missing active GPU edge,
pending resources, and any non-null river-texture state rather than silently
skipping an active override. Preserve native optional behavior when the river
texture is absent: do not open a texture provider or mutate water settings.
Repeated calls and two fresh generations retain zero extra aliases/resources.

Do not reattach water in `W3DTerrainVisual::load`, admit the 08F construction
selector, render a frame, implement active river textures, traverse a retail
provider, or record private identifiers, paths, bytes, hashes, images, or raw
process output. 08F owns the later source `W3DTerrainVisual::load` reattachment
and must verify call order and rollback with generated fixtures. The accepted
08D/08F0 stops and production default remain unchanged.

## Validation and commit boundary

Extend a generated read-only terrain-water source probe: positive attached
no-river call and repeat; negative pre-init, provider removal, detached water,
and pending-resource call; recovery after provider restoration and failed
resource acquisition; zero Recording resources after two independent runs.
Check source/link identity and provider removal. Run focused GCC/Clang,
canonical six complete builds and non-GPU/non-LAN/non-retail suites, strict
host LSan for GCC and Clang, physical Vulkan validation, serial LAN in all
six, dependency-ledger and `git diff --check` after production changes.

Requires accepted 08F2. One independent production/test/evidence commit for
08F3; this discovery plan alone does not accept the owner or 08F.
