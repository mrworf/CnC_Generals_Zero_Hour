# N1 — stock renderer qualification

Authority: docs/zero-hour-linux-port-plan.md and the committed linux-upstream-v2
packet. Planning payload f92a3e5c5b8a9d7433e13e80a8de83eda7399808; closeout
ade34a49. N0 baseline/specification outcome is complete; no original source has
been changed from the reset baseline. No archived implementation is linked.

## Outcome

Select bgfx only after generated physical tests demonstrate the mandatory source
behaviors with pristine sources/public APIs. A missing supported contract is a
go/no-go failure, not permission to add a patch or approximate the visual result.

## Slices

1. [Public API and rendering semantics](N1_plan_slice_01.md): official source
   provenance, reproducible stock tool/runtime build, supported shaders, exact
   clears/stencil/mip/copy behavior and representative shader/attribute pixels.
2. [Capacity and lifecycle](N1_plan_slice_02.md): source-derived scene workloads,
   repeated replacement/cancel/resize/shutdown and complete frozen acceptance.

## Validation

Source inspection precedes implementation. Check mandatory API contracts against
actual upstream source plus public documentation. Build shaders with unmodified
shaderc; public headers only. Test host RTX Vulkan with Khronos validation and
GCC/Clang owner sanitizers. Preserve complete logs in ignored build/evidence and
only attributable summaries/hashes in tracked evidence.

Use supported bgfx build configuration and allocator/callback interfaces. Do not
edit vendor code to repair build failures. If a verified upstream version fails,
identify whether an upstream version/configuration change solves it publicly.

Each slice has one commit after its required checks. A failed public API gate
instead produces one durable blocker/evidence checkpoint with no false acceptance.
