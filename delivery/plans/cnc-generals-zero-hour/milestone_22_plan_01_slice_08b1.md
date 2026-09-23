# M22 plan 01 slice 08B1: bounded multi-track source pool

## Goal

Close aggregate-mask bit 1 only by restoring the original terrain-track
system's bounded module pool: every positive source-configured cardinality
that fits its existing 16-bit vertex budget owns that many modules, uploads
the shared source buffers after map publication, and flushes each active
module in source order. Zero tracks remains the existing no-resource owner.

## Boundaries

This is not cloud, shadow-volume, retail-scene, or pixel work. Negative,
overflowing, malformed, foreign-scene, duplicate, and unowned track bindings
remain rejected. The terrain guard continues to reject mask bits 2–4 and all
water/owner invalid states. No retail input is opened by this slice.

## Proof

Use a generated read-only map and Recording edge to prove two modules create,
upload, update, flush and retire independently; a third binding fails at
capacity. Prove create/upload/draw rollback, reset/retry, two fresh
generations, foreign scene rejection, invalid/overflow cardinalities, and
zero resources. Preserve the original one-module test as a compatibility
control. Run focused source/provider/ledger tests, proportional configured
suites and host ownership/Vulkan/LAN gates. Stage only 08B1 files and make
one commit.
