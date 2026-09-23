# M22 slice 08B: aggregate retail terrain-configuration reachability

08B re-runs the read-only campaign/skirmish Recording audit through the
accepted track, cloud, decal and volume closures.  Both consumers reach the
same sanitized post-owner boundary: `mask=30 families=4 owners=1`.  The marker
is emitted only after source terrain, tracks, shadow, water/cloud and smudge
owners have been constructed; it contains no retail path, filename, hash or
byte data.

The audited cloud-only setting has no water-plane extent.  It is admitted only
with the explicit host retail-route selector, remains a fixed internal 1x1
source owner, and does not mutate the original configuration or enable a water
draw.  Generated coverage proves the default zero-extent cloud owner rejects,
the selected owner succeeds, and teardown returns to zero resources.  The
next unsupported water-helper boundary still rejects and rolls back cleanly;
08B makes no retail-frame or pixel claim.

Focused acceptance:

- Read-only redacted retail campaign/skirmish reachability: 1/1, two modes and
  two fresh generations, with input metadata unchanged, factory rollback,
  provider/owner rollback and Recording teardown at zero.
- Generated cloud-selector composition: `original_w3d_terrain_water` 1/1,
  including ordinary/cloud ordering, failure/retry, reset/re-entry, foreign
  rejection, default zero-extent rejection, selected fixed-owner acceptance,
  and zero ownership.

Final-tree acceptance recorded 213/213 fresh non-GPU/non-LAN CTests in each
of GCC Debug, GCC Release, Clang Debug, Clang Release, GCC Sanitized and
Clang Sanitized.  Broad sanitizer suites used `detect_leaks=0` only for the
host ptrace limitation; focused `detect_leaks=1` ownership checks passed 3/3
in both sanitizers.  The accepted public stencil edge passed physical Vulkan
validation 1/1, and LAN passed 4/4 in all six configurations.  The dependency
ledger and whitespace diff checks pass.  No retail path, filename, hash or
byte is retained in this evidence.
