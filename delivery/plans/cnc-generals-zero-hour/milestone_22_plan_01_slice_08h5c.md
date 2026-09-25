# M22 plan 01 slice 08H5C: extra-blend material edge

## Outcome and source boundary

Requires accepted slice 08H5B. Extend the CPU `W3DShaderManager` only with
the source `ST_ROAD_BASE` minimum-profile material route used by a normal
three-way terrain blend: one alpha-atlas pass, vertex-alpha modulation,
source-alpha/inverse-source-alpha blending and no depth write.

## Implementation and tests

Preserve the existing terrain-base two-pass state machine. Validate the
published texture owner before delayed state changes; unsupported noise,
cloud/light-map and debug variants remain explicit failures. Missing texture,
invalid pass/order, injected selection/draw failure and reset/shutdown must
leave no selected texture, sampler or active shader state. Direct Recording
tests assert exact public state and successful retry over two generations.

This slice supplies no terrain geometry and does not schedule a draw. Run the
full proportional M22 gates and commit once as
`delivery: M22 08H5C add extra terrain material`.
