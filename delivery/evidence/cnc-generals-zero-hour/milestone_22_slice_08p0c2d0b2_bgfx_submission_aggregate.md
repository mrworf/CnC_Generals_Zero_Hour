# M22 slice 08P0C2D0B2: admitted native frame replay aggregate

Parent and unchanged implementation: `f93114b4b851dea8164cba03af98ef902d24e5ac`.
Status: accepted, evidence-only composition; no executable surface changes.

## Exact composition

[A](milestone_22_slice_08p0c2d0b2a_bgfx_submission_reservation.md) owns one public
synchronous immutable manifest deep-copy/admit/reserve/replay boundary. Its
complete encoded/provider/capacity checks precede accepted-target touch. API-thread
replay has admitted storage for native encoder/cache/uniform/frame/resource/view
paths; worker/backend fatal allocation/device loss is explicitly not reversible.
[B2B](milestone_22_slice_08p0c2d0b2b_bgfx_frame_journal.md) owns bounded packet
construction/source order, exact B1 native reference-unit leases, generation,
copied aligned payloads, final completion and device-token consumption. It
does not duplicate A's reservation or expose an intervening native token window.

Whole capture preflights exact source aliases, written byte spans and representable
view coordinates. No native view/touch/clear/submit/frame happens while staging.
One final present supplies uniquely last CompleteFrame. Commit invokes A once;
recoverable rejection preserves packets and prior pixels for same-batch retry.
Success consumes once. External encoder overlap remains unsupported: abort and
an ordinary readiness boundary precede a fresh attempt. No live frame/reset is
used to manufacture admission. Candidate/framebuffer and captured provider
ownership retire at ordinary/wait/shutdown boundaries with exact multiplicity.

A's historical unchanged non-bgfx sanitizer qualifications are not rewritten.
Independent [R1](milestone_22_slice_08p0c2d0b2r1_presentation_services.md) and
[R2](milestone_22_slice_08p0c2d0b2r2_texture_filter_admission.md) correct those
owners before B2B. The current aggregate inherits the genuinely clean B2B matrix,
not historical qualified evidence relabeled clean.

## Unchanged final-source acceptance

Composition review verifies A's native patch remains byte-identical and B2B's
thirteen source/test hashes unchanged. Reuse B2B's complete frozen-source gates:
six builds and six canonical suites 274/274 with clean full-log audits; focused
8/8 on both native and both sanitizer toolchains; exact serial host LSan 4/4
each; twelve generated physical Vulkan controls; established Vulkan GCC3/3 and
Clang2/2; six serial LAN selections 4/4; ledger/public-header/diff checks.
Generated source-order pixels and rejected-batch prior pixels prove the actual
public reservation/journal composition across two generations, not exit alone.

This documentation-only child adds no test workload or production merge.
Exact-owned review excludes the unrelated renderer diagnostic. B aggregate is
next; source-stage B0, shroud, tree preparation/draw, factory and retail remain
closed. Commit boundary:
`delivery: M22 08P0C2D0B2 revalidate admitted bgfx frame replay`.
