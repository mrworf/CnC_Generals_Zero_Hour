# M22 plan 01 slice 08F4: post-radar display shroud-refresh prerequisite

## Goal and source boundary

Close the first missing source owner after the generated post-map radar stage.
`GameLogic::startNewGame` calls `PartitionManager::init`, then
`refreshShroudForLocalPlayer`. Native refresh calls display `clearShroud`,
radar clear, then per-cell display/radar shroud setters. The CPU display
methods are typed pending. Native display clear is a no-op; each display cell
setter maps the logical shroud category to the configured alpha, updates the
original terrain `W3DShroud`, and notifies the terrain object. The CPU shroud
grid already exists, but the CPU terrain notification method is also absent.

## Ordered owner slices

1. [08F4A](milestone_22_plan_01_slice_08f4a.md): exact map-owned terrain
   shroud notification, source-equivalent no-op when no prop buffer exists.
2. [08F4B](milestone_22_plan_01_slice_08f4b.md): exact display clear/per-cell
   dispatch into the accepted CPU shroud grid and 08F4A notification.

The methods form one source refresh route but have separate owner identities
and can be tested and reverted independently. 08F4B depends on accepted
08F4A. Both require accepted 08F3. Neither admits the 08F construction
selector or water reattachment. This plan-only checkpoint does not accept
either owner.

## Non-scope and acceptance

No frame, shroud pixel claim, active prop-buffer notification, arbitrary
display method, border setter, retail provider traversal, or private retail
identifiers, paths, bytes, hashes, images, or raw process output. Use generated
read-only map/shroud fixtures and exact source/global owner checks. Fail closed
for missing/foreign providers, no map, malformed cell/category, or broken
GPU/scene edge; prove retry and zero-resource teardown. Each child requires
its own focused tests, full six configured builds and canonical
`-LE 'gpu|lan|retail'` suites after production change, strict GCC/Clang host
LSan, physical Vulkan control, serial LAN, ledger and diff gates, and one
independent commit. 08F resumes only after both children are accepted.

## Result

Complete as an owner prerequisite: [08F4A](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f4a.md)
and [08F4B](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f4b.md)
each passed its independent generated controls and full required acceptance.
The [aggregate evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f4.md)
records the boundary. 08F still owns water reattachment and scenario
construction; no selector or retail route is accepted by this aggregate.
