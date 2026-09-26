# M22 slice 08P0C2D0B: public bgfx idle/frame aggregate

Parent: `b4e0559b566e3d7570d0d351c549bd5e013b3448`.
Executable baseline: `f93114b4b851dea8164cba03af98ef902d24e5ac`, unchanged.
Status: accepted, evidence-only composition; no production merge.

## Shared owner, independent modes

[B1](milestone_22_slice_08p0c2d0b1_bgfx_resource_transaction.md) supplies exact
candidate/COW native reference units, bounded shadow/slot snapshots, generation-
advanced canceled handles and delayed completion-safe retirement. Its historical
frame capability was deliberately false; accepted B2B now supplies that mode.
Current B1 controls are rerun with only the capability expectation updated, not
weakened idle assertions. [B2](milestone_22_slice_08p0c2d0b2_bgfx_submission_aggregate.md)
supplies immutable source-order journal/native admission and once-only replay.
Both use one exact device/sequence/caller-generation/mode owner and bounded
command/resource/byte/view capacity. Nested/malformed/foreign admission preserves
the existing transaction; wrong-mode operations poison before diagnostics.

Idle mode admits only bounded resource/byte/marker effects, never pass/view/draw/
present/frame calls. Sampled-texture updates COW exact known mip bytes; prior
native accepted content survives abort. GPU-written render-target upload is
unsupported in transactions, rather than pretending a CPU shadow can restore it.
Fresh target identity can represent resize. Frame mode alone stages target/load/
clear/view/draw and unique final present. Before complete native admission, no
accepted target is touched; captured versions remain owned through replay even
when public slots disappear. Abort restores accepted slot/payload/init/view/window
identity and invalidates candidates without native destroy or frame calls.

Each native create/reference increment has one eventual destroy, even when
shader/program native indices deduplicate. Typed VB/IB/FBO candidate wrappers
borrow attachments, retain exact providers and drain in safe order at ordinary/
wait/shutdown boundaries. Native worker/backend device-loss/fatal behavior remains
outside reversible API-thread rollback. Ordinary rendering remains unchanged.
SDL and other devices retain unsupported/fail-closed optional capability.

## Unchanged final-source acceptance

Reuse the complete [B2B evidence](milestone_22_slice_08p0c2d0b2b_bgfx_frame_journal.md)
after composition review. All recorded source/test hashes and the accepted native
patch are unchanged. B1 resource plus B2B journal and A reservation physical
controls pass independently on both native/both sanitized toolchains (twelve
validation-clean runs), covering both modes across two generations. Source-order
pixels, rejection baseline, exact reference multiplicity and zero residuals are
not substituted by CPU-only Recording evidence.

Six complete builds/canonicals 274/274 have clean full-log audits; focused8/8
all four, exact serial host strict LSan4/4 both, established host Vulkan3/3+2/2,
six serial LAN4/4 and ledger/header/diff pass. No executable artifact changes
in this aggregate, so no new test workload is introduced. Exact-owned staging
preserves the unrelated renderer diagnostic. D0 Recording/bgfx aggregate follows;
B0 complete engine refs/maps/transform/revision/filter checkpoint and tree/factory/
retail admission remain closed. Commit boundary:
`delivery: M22 08P0C2D0B revalidate bgfx transactions`.
