# M30 slice 02: ordered render targets and camera clears

## Goal and outcome

The `GpuDevice` accepts target/pass commands and public-bgfx ordered viewport color/depth/stencil clears during one active frame; rejected timing/generation/view-budget cases are observable and never turn into proxy draws.

## Scope / non-scope

Own target attachment creation/validation, begin/end pass, bounded view sequencing, `set_viewport`, `clear_viewport`, load/clear independence and physical readback/clear test. Do not implement full shader pipeline/draw or SDL3 presentation in this slice. M22 owns actual original scene.

## Dependencies / order

Slice 01 resource/lifecycle path; M29 `ViewportClearDesc`, recorder and target-generation semantics.

## End-to-end behavior and state

An asset-free caller creates color+D24S8 target → begins a pass → full target initializes → camera viewport clear submits its own ordered bgfx view in the active frame → later view sees changed inset while outer attachments remain unchanged → end pass finalizes target initialization. View IDs are bounded, monotonically ordered and reclaimed per frame. Error leaves command state unchanged or fails the frame explicitly.

## Authorization / validation

No permission surface. Preflight target format, dimensions, attachment selection, initialized LOAD, rectangle clipping/empty rectangle, target generation, active-pass timing and view budget. No retail content or private Vulkan.

## Implementation surfaces

Expected: bgfx device implementation, target/clear tests, checked public probe integration, any narrowly necessary build definitions and renderer documentation.

## Tests and commands

- Positive: checked 160×120 outer/inset color/depth/stencil sequence and independent depth/stencil preservation draws; CPU source-shaped ordering and physical target readback.
- Negative: inactive clear, stale target, invalid/incompatible attachments, view exhaustion and uninitialized LOAD reject with no silent submission.
- Run focused bgfx target tests and exact checked probe on RTX with validation layer where available; compare trace order with M29 recorder.

## Acceptance / commit boundary

One coherent commit includes physical target/clear behavior, positive/negative tests and evidence. Do not count only the standalone checked probe as device acceptance.
