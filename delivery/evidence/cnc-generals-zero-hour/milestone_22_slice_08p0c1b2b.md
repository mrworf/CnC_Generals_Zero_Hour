# M22 slice 08P0C1B2B: bounded tree sink and deletion

The full-draw CPU terrain registry stages native kill-when-toppled DOWN
countdown as a copied visible-frame candidate. Positive finite sink distance
and nonzero frame count are checked before each unpaused step; position and
topple-matrix translation move by one bounded increment, with deletion on
the following unpaused frame after the count reaches zero. Non-kill DOWN
trees remain registered and stationary. No FX is dispatched and no new RNG
owner is introduced.

Deletion stages instance removal, partition-bucket disappearance, type user
recount and index compaction, atlas rebuild only when a surviving type set
requires it, and final geometry/Recording upload before publication. The
last instance erases its registry without preparing an obsolete empty atlas.
All fallible state/type/atlas/geometry/texture/upload steps precede the
commit point; a failed attempt retains the accepted DOWN countdown, tree
and survivor IDs, partition buckets, type/atlas/GPU owners and resources.

The generated fixture uses the shipping `Object::setPosition` crusher route.
It covers no-kill stability, paused and hidden sink frames, exact two-step
distance/countdown, shared-type and distinct-type survivors, visible
survivor reentry with exact source vertex position after type reindex,
idempotent post-delete frames, last-tree registry and Recording release,
pending-sink owner removal, zero frames and nonfinite distance rejection,
and state/type/publish/atlas-read/geometry/Recording-upload failure rollback
and retry. Two outer generated scenarios each run two internal terrain
generations; resource baselines are asserted before teardown. The 64-type
and 4000-instance workload and redacted per-generation timeout remain.

Final-source focused GCC/Clang Debug generated witnesses passed 1/1 each
(57.52s and 59.32s); isolated GCC/Clang ASan+UBSan focused witnesses passed
1/1 each with `ASAN_OPTIONS=detect_leaks=0` (256.71s and 215.64s). All six
GCC/Clang Debug, Release and ASan+UBSan complete builds pass. The four native
canonical nonretail suites pass 267/267 each with `-LE gpu|lan|retail`.
Both GCC/Clang ASan+UBSan canonical nonretail suites also pass 267/267 each
serially, with generated terrain cases passing in 256.30s and 215.77s.
Host-escalated physical Vulkan passes GCC display/bootstrap/map 3/3 and
Clang display/map 2/2 under `VK_LAYER_KHRONOS_validation`, with no validation
error category; the host NVIDIA driver reports 615.71.09. Serial host LAN
passes 4/4 in all six configurations. The dependency-ledger checker and
`git diff --check` pass; the unrelated renderer diagnostic remains unstaged.
Strict host LSan, using exactly `ASAN_OPTIONS=detect_leaks=1` without a
UBSan override, passes display-owner and generated terrain 2/2 on GCC and
2/2 on Clang under host escalation; terrain runs took 254.97s and 215.17s.
