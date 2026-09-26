# M22 plan 01 slice 08P0C1B2: topple and sink aggregate

## Dependency-ordered children

Native `W3DTreeBuffer::unitMoved/applyTopplingForce/updateTopplingTree`
own crusher admission, fog, angle and bounce state. The subsequent DOWN sink
and deletion path has a separate resource boundary: the CPU terrain owner's
existing `removeTree` retires atlas, type and Recording owners immediately,
which cannot be called before a fallible frame upload. Deliver
[B2A](milestone_22_plan_01_slice_08p0c1b2a.md) for topple state and
transformed geometry, then
[B2B](milestone_22_plan_01_slice_08p0c1b2b.md) for sink/deletion and
transactional partition/type/atlas cleanup. This file is their aggregate;
do not merge production changes into one commit. External `FXList`
dispatch remains B3.

## Aggregate validation and commit

On generated inputs, compose mobile crusher collision, source minimum
speed/direction, fog freeze/reveal, angular acceleration/bounce/down,
pause, bounded sink/deletion, hidden and visible frames, relocation,
owner removal and two generations. Reject missing providers, degenerate or
nonfinite parameters, zero sink frames, geometry/Recording faults and injected
publication without consuming accepted frame, tree/partition/type identity,
GPU resources or RNG. Verify no external FX dispatch before B3, no stale
partition link after deletion, exact immediate residuals and clean retry.
Run six complete builds/canonical nonretail suites, focused strict host LSan,
physical Vulkan controls, serial LAN 4/4 all six, ledger and diff checks.
Commit only aggregate evidence:
`delivery: M22 08P0C1B2 revalidate tree topple and sink`.

## Delivered evidence

B2A and B2B compose the public crusher, shroud, angular/bounce/DOWN and
bounded sink/deletion transaction with survivor reentry, exact resource
rollback, owner removal and two-generation retry. No production or fixture
changed after B2B; its six complete final-source builds/canonical suites,
focused sanitizer witnesses, strict host LSan, physical Vulkan, serial LAN,
ledger and diff gates carry to this evidence-only aggregate. External FX
dispatch remains B3. See linked aggregate evidence in the governing index.
