# N2 source model bounds through actual Drawable

Parent f9abc2dea527729349b70b99610327db0f6bbf84. Existing N2 plan01/slice03;
supporting bounds checkpoint, not slice03, full startup or N2 acceptance.

BOX0x740 metadata retains version, flags, name, color padding, center and extent.
Aligned/oriented prototype admission preserves source root order and first-winner
duplicates. Unknown flag bits and trailing payload bytes are retained, while
truncation, unterminated fixed names and negative/nonfinite extents reject
transactionally.

Game-owned bounds derive mesh/null/box and nested HLOD all-LOD/aggregate bounds
from the final scaled clone contract. Source special selection scans only the
last LOD backwards for oriented BOUNDINGBOX suffixes. Current pose updates its
override; ordinary bounds remain base-hierarchy bounds. The generated nested
fixture verifies scale and both LOD extrema; its aggregate participates in the
merge but does not independently determine an extremum.

Actual Drawable first-render-info dispatch returns root position/transform and
source sphere radius. Current-module OBB publication retains extents and uses the
root basis, not a world AABB. Geometry scale and ObjectScale remain distinct;
special HLOD override radius ignores ObjectScale. Accepted bound/info reads
allocate nothing. Absent models, bad pointers, logic phase, malformed scale and
late radius overflow retain outputs and permit corrected same-owner retry.
Candidate model/clip/pose/bounds are admitted together under the asset transaction.

The initial unfinished batch imported a nonexistent debug-header spelling.
Reviewed original math/debug imports now use case-correct source paths; OBB
publication follows its original public field representation without linking
private renderer internals. These are original Zero Hour sources, not changes
to bgfx or other adopted dependencies.

## Validation

Final focused model-bounds test PASS2.22s (CTest total2.23s).
Frozen canonical720 SHA256
e79b9a4a78612f953880425beec2200e500b0cd964fcbdf4374d542e0ec98faf;
inclusive727 SHA256
4aacaa907ac8e18503be919430b03348005aa0b3ba1fcea4be41ccf8f2fd31cf.
Hash procedure relative to9e5c0e4f matches N2-animation-clock.md.
Related121 includes the N2-display-time.md target/selector cohort plus
original_drawable_queries_model-bounds. Frozen results PASS GCC113.13s,
GCCsan441.47s and Clangsan288.91s; all process exits0, no live jobs.
All218 prefix/composed-client manifest lines agree across configurations.
Current model cold182/seeded185 retain every failure/retry and production cursor
proof before fixture rollback, with three complete owner lifetimes. Accepted
bound/info reads demand0 standard allocations; independent constructor
standard1/module-pool2 and composed-client205 terminals remain covered.
Logs: /tmp/n2-model-bounds-<configuration>-frozen-{build,related}.log.
All ASan/UBSan/LSan checks and independent60s family timeouts remain enabled.

Configured458 full suite is unrun. Concrete doDraw ObjectScale publication,
source condition selection/transitions/pause/events, runtime19 callbacks,
condition transfer/material/prototypes, camera actor-follow end-to-end, complete
GameClient/root startup and loaded deterministic simulation remain pending.
Internal LOD cost spheres and physical/GPU/retail audiovisual parity are not
claimed. Retail assets, adopted dependencies and recovery storage are untouched.
