# M30 slice 01: pinned public runtime and resource lifecycle

## Goal and outcome

An asset-free client can initialize the pinned public bgfx Vulkan runtime, create/upload/destroy buffers, textures and samplers through opaque generation-safe `GpuDevice` handles, and shut down/re-enter without leaking or silently accepting invalid input.

## Scope / non-scope

Own offline runtime build/link, device construction, capability query, resource creation/upload/destruction and focused tests. Do not add render-pass clear, shader draw or window presentation here; those are later slices. No system package install, network fetch during configure/build, retail files, or raw Vulkan.

## Dependencies / order

M29 shader/toolchain pins and manifests; no production edits until the full governing and four slice plans are persisted and inspected. This is first implementation slice.

## End-to-end behavior and state

Client constructs bgfx device → verified pinned runtime initializes Vulkan → resource descriptor/preflight validates → public bgfx handles are allocated and indexed by opaque engine handles → upload is bounded → destroy retires generation → re-entry returns resource/worker counts to baseline. Error paths leave no live handle and publish actionable `last_error`. Creation when runtime/device is unavailable fails closed.

Implementation fact: `BufferDesc` does not carry the vertex layout or index width required to allocate the correct public bgfx buffer. Slice 01 therefore keeps bounded, generation-safe buffer bytes in an owned shadow; slice 03 materializes correctly typed native buffers at the draw boundary. A sampler is likewise a validated descriptor until `setTexture` applies its flags. Only texture allocation/upload and Vulkan runtime lifecycle are physically exercised in this slice; neither buffers nor draws are claimed as GPU-accepted yet.

## Authorization / validation

No user authorization surface; only project-owned shader/build files and memory uploads enter. Reject invalid dimensions, unsupported format/usage, stale/foreign handles, oversize/misaligned uploads, exhausted slots and missing/wrong source pin before public submission. Never access retail roots.

## Implementation surfaces

Expected: `CMakeLists.txt`, `cmake/` pinned-runtime check/build helper, `third_party/` lock reuse, new `include/zh/platform/bgfx_device.h`, `src/renderer/bgfx_device.cpp`, focused `tests/renderer/` tests. Retain backend-neutral headers and historical SDL_GPU code.

## Tests and commands

- Positive: exact pin/license offline build and init → buffer/texture/sampler upload → destroy/recreate generation differs → shutdown/re-entry.
- Negative: wrong/missing pin, invalid format/extent/upload, stale handle and unavailable device fail without a partial live resource.
- Validate focused bgfx device tests and related renderer contract tests in GCC Debug; inspect public API diagnostics and resource baseline. No full suite until milestone gate.

Focused evidence: `delivery/evidence/cnc-generals-zero-hour/milestone_30_slice_01.md`.

## Acceptance / commit boundary

One reviewable commit includes this plan, runtime integration, resource behavior, focused tests and evidence; no merely declared API or unexercised resource layer. Update governing index with commit reference.
