# N2 original particle overflow ownership

Continuation of N2 plan01/slice03, parent
cd91e1c447096f2ab5192aac4296d730ef056952. Related144 was already frozen and
passed all3 at e4a1d97d; this extension changes only the generated particle
fixture and its CMake family registration, not production engine or dependencies.

## Actual owner behavior

The growth family configures the public ParticleSystemPool as initial64,
overflow4, distinct from the bounded initial64/overflow0 families. Production
defaults remain initial1024/overflow32. It keeps one accepted actual particle
system and holds every remaining initial slot using a fully constructed cleanup
guard before each raw acquisition. The next real manager chain therefore must
acquire an overflow blob. No alternate particle creator or allocator is used.

A public pool probe derives metadata terminal3 and proves its three persistent
standard allocations, capacity+4, used-slot return, and exact releaseEmpties
retirement. Whole chain creation then runs under the production admission scope
as a later tracker-rejection boundary. Every standard failure prefix and terminal
preserves the accepted owner's identity/destroyed state, list/count, original
ID/client RNG and used units before fixture lease restoration. A successfully
acquired empty overflow blob may remain after rollback: the test proves exactly
zero or four retained slots and exactly zero or three retained metadata owners,
then public empty-blob cleanup restores the exact complete baseline. It does
not exempt differing live counts or treat retained capacity as leaked systems.

Each failed prefix has a corrected retry with identical cold-cache demand and
existing slot state. Warmed-template terminal13 is separate from cold-template
terminal17, repeated over three complete generated owner lifetimes. Both derive
their own exact terminal; neither reuses the other's constant. Finally, a real
accepted overflow chain advances manager IDs and the exact original client RNG
stream, destroy remains deferred and preserves the previous owner, and original
manager reset retires its actual pooled systems before overflow backing cleanup.
Whole fixture destruction returns standard-live ownership to its warmed baseline.

## Final validation

All7 particle families pass on frozen final test source:

- GCC0.53s, exit0.
- GCC ASan/UBSan/LSan4.74s, exit0.
- Clang ASan/UBSan/LSan2.60s, exit0.

All three fixture builds pass; no live owned jobs remain. Normal-host sanitizer
CTest retains leak detection and60s per-family limits. There were no failures,
suppressions, timeout increases or production changes. The earlier7-family runs
also passed before the explicit accepted-ID/RNG witness was added; final results
above validate that witness and an explicit raw-probe cleanup guard as well.
Selector `^original_particle_admission_`,
target original_particle_admission_fixture, original-core GCC/GCC-sanitize/
Clang-sanitize build configurations. Logs:
`/tmp/n2-particle-growth-<configuration>-guarded-{build,tests}.log`.

Canonical731 SHA256
b25dc2a4d7c9524a90a06a1d0fa732e93d1818801289253df56fcef63cbf4dcd;
inclusive738 SHA256
45ef57cd30a7f11a90f03388c38c63a18261d97f428ac2d913cfd45613a02880.
Fingerprint procedure remains relative to9e5c0e4f. All22 normalized allocation
manifest rows match, SHA256
ec5c91fa4e14f4b09553bdf1673e7d42e354969f59b98bed07c60a0c486c3c27.
Normalization removes only numeric CTest prefixes and preserves row order.

## Limits and continuation

The new family validates the real overflow mechanism under supported explicit
counts, not the exact production startup profile or arbitrary scene capacity.
Standard fault injection covers blob-object/live/free-slot metadata acquisitions,
not direct malloc exhaustion; ASan/LSan checks backing cleanup. Existing bounded
tests separately cover typed system-slot exhaustion. Creation-only savepoints
still do not restore mutations/removals of pre-existing systems.

The prior related144 result is evidence for its recorded source. Since this
extension changes no production owner, proportionate validation reruns all7
affected families rather than repeating that whole matrix for a test-only
extension. Current related145 and full482 have not run; the next coupled
production freeze must include this family with concrete effect integration.
Do not claim a current full matrix, concrete19, startup, audiovisual acceptance,
slice03 or N2 completion. Continue the existing concrete tracker/bone/effect
lifecycle contract under actual source parents, then camera and actual startup.
