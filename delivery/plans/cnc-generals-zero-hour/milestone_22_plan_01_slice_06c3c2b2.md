# M22 plan 01 slice 06C3C2B2: exception-safe static-sort drain

## Goal and boundary

Requires B1. Preserve original descending static-sort levels, hook and mesh renderer Flush order, and reentrant disable behavior. Repair only Linux failure paths so a throwing hook/object/Flush cannot strand a dequeued ref, an enabled/disabled state, or stale list entries. B3 owns physical mixed-frame pixels.

## Failure contract

The active popped node is released exactly once on all paths; remaining queued nodes are either retained for an explicit retry or cleared with bounded refs when the enclosing WW3D frame aborts. Original successful order and priority remain unchanged. Static-list disable flag always restores the entry value, even for nested/reentrant callbacks and throws. Existing retained object refs and scene membership are independently accounted for. No callback replay is inferred from a failed frame.

## Verification and commit

Owned source objects at at least two sort levels, callbacks that enqueue during drain and inject a throw, count original hook/render order and `Num_Refs`, inspect list enable before/after, abort and start a fresh source frame. Run GCC/Clang focused original-rendering/ABI/provider tests, dependency ledger and both sanitizer toolchains. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b2.md`. One commit: `delivery: M22 slice 06C3C2B2 make static sort drain exception safe`.
