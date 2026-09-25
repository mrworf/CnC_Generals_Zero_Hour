# M22 plan 01 slice 08P0C1B2B: bounded sink and deletion

## Goal and boundary

After B2A publishes a DOWN tree, advance native kill-when-toppled sink
distance for bounded positive frames, then remove it from active geometry
and partition traversal without stale links. Own the CPU terrain registry's
type/atlas/Recording cleanup when the final instance of a type disappears;
this must not use the current immediate `removeTree` path before fallible
frame publication. Keep topple/bounce FX closed until B3, and keep C2/C3/C4
closed. Authorization is not applicable to generated local simulation.

## State, failure and surfaces

Trace native DOWN/sink frame order, `BaseHeightMap::removeTree`, source atlas
ref and GPU retirement, and the CPU registry's last-user type indexing.
Preflight finite positive sink distance and positive frame count when kill
is enabled. Stage sink location/translation, deletion, remaining type users,
atlas and visible geometry/Recording resources together. Publish only after
allocation/texture/upload succeeds; reverse cleanup on failure preserves
accepted DOWN state, frame, registry/partition/type identity and all source
refs. Owner reset/removal clears pending sink without replay. No GameClient,
GameLogic or audio RNG consumption, and no FX dispatch.

## Acceptance and commit

Generated positives: kill and no-kill DOWN trees, bounded sink countdown,
final-frame deletion, surviving shared type, last-user atlas cleanup,
partition unlink, visible/hidden frames, pause and two generations.
Negatives: zero/nonfinite sink parameters, type/atlas/geometry/Recording
faults and owner removal; failed deletion must retain partition/type/atlas/GPU
and surviving identities exactly, then clean retry retires each once with
immediate pre-teardown residuals and no duplicate removal. Run focused
GCC/Clang and sanitizer witnesses, six complete builds/canonical nonretail
suites, strict host LSan, physical Vulkan, serial LAN 4/4 all six, ledger and
diff checks. Commit one slice:
`delivery: M22 08P0C1B2B own tree sink deletion`.
