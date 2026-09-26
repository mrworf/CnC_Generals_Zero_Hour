# M22 slice 08P0C2D0: required device transaction aggregate

Parent: `3fe00c4cb49afb79ae791ad308482014fd9a0664`.
Executable baseline: `f93114b4b851dea8164cba03af98ef902d24e5ac`, unchanged.
Status: accepted, evidence-only Recording/Linux bgfx composition.

## Required capabilities and ownership

[D0A](milestone_22_slice_08p0c2d0a_recording_transaction.md) establishes the
optional bounded public idle-preparation/frame capability and deterministic
Recording checkpoint. [D0B](milestone_22_slice_08p0c2d0b_bgfx_transaction_aggregate.md)
independently proves both required modes on the actual Linux full-draw bgfx
route, including exact native ownership and physical pixel behavior. Other
devices, including SDL, remain unsupported/fail-closed through the public
default; Recording success never substitutes for a required native capability.

Common ownership is a single bounded device/sequence/caller-generation/mode
token, inactive-device admission, no overlapping transaction, no cross-mode
frame use, no-throw once-only finish and consumed failure-injection counters.
Rejected nested/malformed/foreign attempts leave an existing owner usable.
Candidates cannot alias restored handle generations. Every captured accepted
resource/byte/init/command/view identity survives failed preparation and abort.
Successful retry publishes/emits once; owner destruction cancels before cleanup.

Idle mode cannot consume pass/view/viewport/clear/draw/present. Recording owns
deterministic resource/payload/command rollback. Bgfx additionally owns exact
native create/reference multiplicity, COW known mip content and deferred
completion-safe retirement. Its frame mode validates/copies the complete source-
ordered batch before accepted target touch, requires uniquely final present/
CompleteFrame, then calls A's synchronous bounded native admission/replay once.
No private native checkpoint/cancel API, reset/frame rollback shortcut or claimed
worker/backend allocation reversal is introduced. Ordinary paths remain intact.

OriginalGpuEdge forwards capability/generation/token ownership and frame-phase
cancellation without duplicating either device checkpoint. It does not yet own
source texture refs/maps/transforms/revision/filter/samplers: dependency-next
B0 must deliver that complete independently testable state boundary before
exact shroud or tree preparation/scene integration.

## Unchanged final-source acceptance

Reuse [B2B's final acceptance](milestone_22_slice_08p0c2d0b2b_bgfx_frame_journal.md)
after Recording/native composition review. The focused set includes actual
Recording transaction plus real edge admission/owner-removal controls, as well
as native CPU admission and separate shipping-scope checks. All six complete
builds/canonicals pass274/274 with clean full-log audits; focused8/8 all four;
exact serial host strict LSan4/4 each; twelve generated native/sanitized physical
Vulkan controls; established host Vulkan3/3+2/2; six serial LAN4/4 and final
ledger/header/diff checks pass. Current source/test/native patch hashes remain
unchanged through both evidence-only aggregates. Historical A sanitizer
qualifications are closed by independent R1/R2 owners, never suppressed.

This aggregate changes documentation only and creates no new test workload.
Exact-owned staging excludes the unrelated renderer diagnostic. No tree state
advance, shroud semantics, source factory, retail input or M22 acceptance is
claimed. Commit boundary:
`delivery: M22 08P0C2D0 revalidate frame command transactions`.
