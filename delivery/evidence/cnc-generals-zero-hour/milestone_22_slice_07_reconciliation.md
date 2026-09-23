# M22 slice 07 reconciliation after 07F8

07 is not complete. 07F8 proves only a generated flat map's two-pass base
terrain submission through Recording. It does not prove a map-loaded
`W3DTerrainVisual`, map-aware display/view traversal, active shroud, tracks,
water, shadows, particles/smudges, production map factory publication, retail
recording, or physical pixels.

## Source-mandatory findings

- Native `W3DTerrainVisual::load` constructs `WorldHeightMap`, initialises and
  attaches the height-map owner, then loads map lights/scorches and water.
- Native display update renders shroud before views when a map is present;
  native height-map render flushes terrain tracks after terrain passes.
- Source object draw modules request shadows through `W3DShadowManager`; the
  particle path renders and resets the smudge manager.
- The Linux map guards in `W3DView::updateView` and `W3DDisplay::draw` still
  reject a loaded map. Consequently retail recording cannot enter through the
  accepted no-map factory route.

Those calls make the listed systems source prerequisites of the M22 scene
contract, independently of whether a particular private map happens to enable
every optional feature. The contract explicitly includes terrain, fog/shroud,
tracks, water, shadows and effects for the representative scene families, so
they cannot be postponed to retail slice 08 or silently treated as absent.

## Replanned graph

`07F8 -> 07F8A -> 07F9 -> {07FA, 07FB, 07FC, 07FD, 07FE} -> 07FF -> 07FG -> 08 -> 09`

07F8A is the reusable terrain bounds and rigid scene-update registration
prerequisite exposed by the first 07F9 probe. The braces are independent source-producer children after transactional map
load. 07FA owns map frame/shroud traversal; 07FB tracks; 07FC water; 07FD
shadows; and 07FE particles/smudges. 07FF is the generated full-feature
Recording integration gate, and 07FG is production publication. 08 may read
private retail inputs only after 07FG; 09 remains the Vulkan/resize/visual
gate. This reconciliation read tracked source and existing evidence only; it
did not inspect or modify retail content.
