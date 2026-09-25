# M22 plan 01 slice 08H5A: authored static terrain vertices

## Outcome and source boundary

Requires accepted slice 08H4. Replace the flat-only guard in the CPU
`HeightMapRenderObjClass` with the bounded source static-terrain consumer.
Each populated cell retains base-atlas UVs in channel one, primary blend UVs
in channel two and blend alpha in vertex diffuse alpha while preserving the
accepted white minimum-profile lighting and fixed base topology.

## Implementation and tests

Use separate base and alpha coordinate arrays, validate every map query before
publishing buffers, and keep unused padded cells deterministic. Nonzero base
classes, primary blends and cliffs must pass through the existing two terrain
passes and exact public vertex upload; missing atlases, invalid queries,
buffer failure and retry must restore map, buffers, shader state and device
resources to baseline. Generated coverage checks exact vertices for multiple
classes, alpha/orientation cases and height/cliff data over two generations.

This slice does not add global/dynamic terrain lighting, extra-blend inventory
or a third material pass. Run the full proportional M22 gates and commit once
as `delivery: M22 08H5A consume authored terrain vertices`.

## Result

Complete. The source height-map owner now materializes its accepted atlas pair
before geometry construction and writes base and primary-blend UVs to their
separate channels with authored blend alpha in diffuse alpha. Generated flat
and multi-class authored fixtures prove exact payload, two base passes,
failure rollback and device-generation-safe map/atlas teardown. Extra-blend
inventory and submission remain 08H5B–08H5D. See the
[08H5A evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08h5a.md).
