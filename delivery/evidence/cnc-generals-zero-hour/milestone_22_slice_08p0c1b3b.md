# M22 slice 08P0C1B3B: committed tree position FX

The CPU terrain's exact-owner registry now retains collision-time list and
position intent with per-tree start/bounce sequence and consumed markers.
The shipping public Object movement callback preflights a new start before
collision publication. A candidate-only monotonic order preserves chronology
across separate callbacks; failed publication and bounded order overflow
consume neither order nor intent, cancellation retains surviving order gaps,
and a new registry epoch starts from the declared zero baseline.

Every successful visible-frame attempt revalidates the complete immutable
pending batch with B3A before state, geometry, atlas, Recording or GameClient
preview publication. Starts are stably ordered by collision chronology,
followed by source-order bounces. Native bounce position is the current
toppling matrix applied to `(0, 0, 21)`. Consumed markers publish with the
accepted frame, and replaced GPU/atlas resources retire before dispatch.
Native `FXList::doFXPos` retains the exact positional shroud gate and nugget
ordering. Each dispatch exception is an accepted, consumed partial effect;
the event cannot replay on frame retry. The immutable local batch performs
no registry access after dispatch begins, so an effect may remove the sole
registry without invalidating the later nugget/event traversal.

Generated coverage retains the 64-type/4000-instance workload and two terrain
generations. It proves public collision intent remains unconsumed until a
successful frame, late unsupported and stale/store-provider rejection,
collision publication/order overflow, queue/state/geometry/Recording/frame
publication rollback, clean retry, ordered multiple nuggets, exact start and
bounce position, paused/hidden/reentry no replay, reversed callback/insertion
chronology, cancellation with surviving order gaps, fresh-epoch baseline,
partial dispatch throw after an earlier nugget with no second RNG/effect on
retry, removal during first dispatch, removal/reset before frame and native
fog-gated consumption without visibility-return replay. Client/Logic RNG,
GPU identities/counts and pending markers are invariant on rejected admission.

The logical shroud adapter is notification-only and restores the real display
before any frame. Exact reveal/undo/cover notification coordinates/status
sequence and every original logical shroud cell are restored. The bounce-tip
fixture uses a larger reveal radius because the native transformed point is
21 units away, outside the earlier two-cell base-only reveal; production
shroud queries and native FX gating are unchanged. All temporary diagnostics
were removed before final validation. New state members have explicit
zero/default initialization; event aggregates are fully constructed and no
padding bytes are compared. Native terrain object layout is unchanged.

## Classified corrections

The superseded first-freeze GCC sanitizer generated witness failed after
65.65s (not timeout). Generated-only localization identified a production
heap-buffer-overflow at `MemoryPoolSingleBlock::getOwningBlob`, reached by
`std::stable_sort` temporary-buffer destruction through native ordinary delete.
The temporary nothrow allocator boundary was incompatible, not a fixture-owned
stale reference. The correction uses explicit bounded allocation-free insertion
sort of stack event values and indexed candidate writes, with a total key:
start chronology then bounce source ordinal. Four reverse public callbacks
and post-sort failure/retry cover ordering, unchanged markers/identities/RNG/GPU.
No allocator suppression or broader allocator mutation is introduced. All
first-freeze gates are superseded and are not used as final acceptance evidence.

The first corrected sanitizer attempt reached a fixture assertion, not a new
ASan report: the four-event upper-right `(54,50)` start was consumed once but
natively shroud-suppressed. Native debugger confirmed the real PartitionManager
cell size is 1 and its clear reveal radius 34; that diagonal is 42.4 units from
the base. The fixture now uses 20-unit independent-collision spacing so every
source point is within the declared reveal, and checks actual clear cell status
before collision and frame dispatch. Production shroud gating is unchanged;
the separate fog-gated consumed/no-visibility-replay control remains intact.

## Final-source validation

Corrected expanded GCC and Clang native focused pass generated terrain plus
real headless runtime 2/2 each (terrain 59.99s/60.82s). Their four-tree continuation checks source
bounce ordinal after reversed starts, exact transformed-tip clear admission
and native suppression, all consumed markers and no paused-frame replay.
Actual 8001-element heap-vector rejection preserves accepted markers, RNG,
GPU and identities. The explicit ordering helper has no engine allocator,
new/delete or nothrow/temporary-buffer entry in the compiled call audit; it
uses one stack value, not an 8000-element stack array. The narrow durable
AGENTS lesson records allocator pairing and explicit bounded allocation-free
ordering for future original-runtime integrations. Sanitizer witnesses and
canonical suites run serially without concurrent build/test load.

Corrected isolated GCC ASan+UBSan focused terrain and headless controls pass
2/2 with `ASAN_OPTIONS=detect_leaks=0` (terrain 263.91s, headless 0.16s), with
no sanitizer finding or timeout. No production/test source changed afterward.

Corrected isolated Clang ASan+UBSan focused terrain and headless controls also
pass 2/2 (terrain 223.17s, headless 0.07s), with no finding or timeout. Source
is refrozen: BaseHeightMap.cpp `7ee2019c974b77dbb383f02e2165fe3f291681d66b19f4818d43af0d0950ee58`,
probe `30f6b0bd71b6d860f82c3ec2f4bdfd9a07db361d24e78f9c5d9c718eb17f8c75`.
The header hash is `03ec7d400941be379b37f5b5ddd91381bb559cb5d1075b279aebc30dff8b7b3b`.
No first-freeze gate is used as final acceptance evidence.

Refrozen acceptance: all six complete builds pass. All four native Debug/
Release canonical nonretail suites pass 267/267 each: terrain GCC Debug
60.64s, Clang Debug 60.47s, GCC Release 25.03s, Clang Release 24.56s.
Fresh host Vulkan passes GCC display/bootstrap/map 3/3 and Clang display/map
2/2 with no validation-error or VUID category. Fresh serial host LAN passes
4/4 in all six configurations. The refrozen GCC ASan+UBSan canonical suite
passes 267/267 with `ASAN_OPTIONS=detect_leaks=0`, total 859.45s and terrain
262.98s, without finding or timeout. Clang ASan+UBSan canonical also passes
267/267, total 684.21s and terrain 222.71s. Thus all six final-source builds
and canonical suites pass. Strict host LSan passes display-owner and terrain
2/2 in each sanitizer configuration, serial and host-escalated with exactly
`ASAN_OPTIONS=detect_leaks=1` and no UBSan override: GCC 6.08s/262.33s,
Clang 5.69s/221.11s. There are no leaks, sanitizer findings or timeouts.
No production/test edits after refreeze. Dependency-ledger and diff checks
pass; the unrelated renderer diagnostic is untouched and excluded.

Delivered against parent commit `3946590c76ad0161f7aeee365e6d3eceeb656eec`
by `delivery: M22 08P0C1B3B dispatch committed tree FX`. Generated runtime
behavior only; terrain draw, projected tree decals, physical factory and
retail scene admission remain closed for their dependency-next slices.
