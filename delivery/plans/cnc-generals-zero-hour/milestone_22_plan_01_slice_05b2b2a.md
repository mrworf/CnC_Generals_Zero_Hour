# M22 plan 01 slice 05B2B2A: original texture stage and sampler choices

## Outcome and dependencies

Requires accepted 05B2B1. The original `TextureClass::Apply`,
`TextureFilterClass::Apply` and reached null/disabled stage decisions run
before the device edge, preserving original initialization, texture owner,
stage index, addressing, min/mag/mip filter and source call order. A scoped
translator retains the selected texture handle and sampler state pending
the actual original category pass/draw in 06; there is no synthetic pass,
generic shader or completed-frame claim.

## Canonical producer and physical edge

Use the original W3D texture and filter source methods under the mutually
exclusive full-ABI target. Share one source implementation across native
and Linux configurations where original method bodies are reused. Add the
narrow public `GpuDevice` stage/sampler capability only if source needs it;
Recording and SDL_GPU must agree about supported address/filter/mip and
stage limits, including explicit rejection where SDL_GPU cannot honor a
legacy choice. The adapter only maps source decisions to handles/state;
never owns format, pixel, optional missing fallback, or stage order.
Pending state survives the exact source command sequence, resets on
source owner/device generation invalidation and is consumed at the real
pass entry in 06. No dummy draw or implicit binding is permitted.

## Acceptance and negatives

Owned DDS/Targa/W3D fixtures drive actual source `TextureClass::Apply` and
`TextureFilterClass::Apply` choices for stage zero and an additional reached
stage, optional authored MissingTexture and explicit null/disabled texture.
Recording witnesses source order, exact handle/sampler/address/filter,
invalidation, failure/retry and zero resources after teardown. Test an
unsupported stage, addressing/filter/mip mode, missing required source
owner, stale generation, injected sampler creation/pending-state failure
and no premature render-pass/draw command. Physical pass-time bind failure
belongs slice 06. SDL_GPU Vulkan capability-bound sampler
creation and reset/recreation must run without validation errors; actual
scene texture binding remains 06. Read-only retail family aggregate is
classification only; no private filenames/paths/bytes/hashes. Enforce
canonical provider-removal/link-map ABI and source hash ledger, GCC/Clang
full suites, focused ASan+UBSan/LeakSanitizer, source classification and
`git diff --check`. The material/shader 05B2B2B and 06–09 gates remain.

## Commit boundary

One independently validated commit:
`delivery: M22 slice 05B2B2A preserve original texture stages`.
