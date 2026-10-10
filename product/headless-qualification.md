# Original-engine headless qualification contract

Status: technical definition under delegated internal engineering authority in
[the approved port plan](../docs/zero-hour-linux-port-plan.md), R03/R05/R07.
Decision: REC-DEC-001. Scope: foundation qualification, not a game feature reduction.
The baseline R01–R12 requirements and N0–N7 graph remain unchanged. This document defines
an executable acceptance contract; it is neither a milestone nor an implementation slice.
No runtime acceptance is claimed. Source references below were inspected at `c5e1fcb3`
with pre-existing dirty code explicitly excluded from acceptance.

## HQ-01 — Entry, actors and lifecycle

The actor is a local developer/automated runner using the ordinary user's permissions.
A documented native executable entry accepts explicit data roots, separate user storage,
a generated scenario identity, a deterministic seed and a bounded tick budget. Exact CLI
spelling and executable name are implementation choices to document with the executable.
The entry must invoke actual original `GameEngine::init`, `GameLogic::startNewGame` and
`GameEngine::update`, including the original common `GameLogic::update`. A function-pointer
link probe, fixture-invented world update, replacement RNG loop, or direct manual module
update is insufficient. Native factories supply original owners or source-equivalent
platform providers, not constant-success placeholders.

Lifecycle: prepare generated roots → initialize engine → request/load original single-player
map → observe loaded world → execute bounded original updates → collect checkpoints →
retire world/root → repeat. Initial map loading may consume the source's pending-start
transition; qualification steady ticks begin only once loading is complete. The harness
must observe this state, not force readiness or rewrite the frame counter to hide setup.

Run three successful complete owner lifetimes in one process with identical fixture, seed,
configuration identity and initial source cursor policy. Each accepted run must have identical
semantic checkpoints. Reset source state only through legitimate startup/teardown; inspection
or an outer fixture restore cannot be used to hide failed production cursor advancement.

## HQ-02 — Finite generated scenario H1

H1 is a test-only, generated, non-retail single-player scenario. Its fixed workload is:

- One legal 65×65 height map, two-cell boundary, with nonuniform terrain: left half authored
  height 20, right half height 24. Complete the original required logical map chunks, sides,
  teams, script tables, terrain/boundary metadata and configuration parents. Use original
  parsers and format widths. Values are generated test data, not proprietary derivatives.
- One active human side plus source-required neutral/civilian sides. The fixture selects
  ordinary single-player mode with no multiplayer recorder or network session. No victory/
  defeat action is authored; retain and execute the original victory owner rather than skip it.
- Exactly two authored test actors owned by that active side: `H1Persistent` and `H1Expiring`.
  Both have valid original geometry and source-required body/metadata, no Draw blocks, no
  weapons, no movement orders, no ambient audio and no physical FX. Put them at distinct
  interior positions on opposite terrain plateaus. Record their source-assigned IDs.
- `H1Expiring` has exactly one original `DeletionUpdate` behavior with MinLifetime and
  MaxLifetime both 1000 milliseconds. It must be created through the actual registered
  original module factory and loaded-object path. `H1Persistent` has no timed deletion.
  There are no test-defined gameplay module creators or replacement update callbacks.
- One original active, non-repeating script with `CONDITION_TRUE` and `SET_FLAG` sets the
  generated flag `H1Started` true. No script changes time, player health, victory or data roots.
- Use seed `0x5A170001`; fixed seed initialization occurs through original initialization
  services. Run 120 completed steady simulation updates using the source fixed-step cadence.
  Wall-clock delays must not advance simulation or change seeds. No pause or camera freeze
  is requested. Reject a run that loads a shell world, never loads the actors, freezes time,
  terminates early or substitutes direct calls to `DeletionUpdate::update`.

These choices exercise an actual loaded map, original script and sleepy-module scheduling,
source object removal, original frame/RNG and repeated ownership. They do not claim movement,
combat, AI strategy, model animation or full-match coverage. Those existing requirements remain.
A legal source-required default actor/provider is permitted only if separately identified in the
manifest; the two authored actors and their assertions remain mandatory. Do not add unrelated
retail content or every provider type merely to enlarge this qualification fixture.

## HQ-03 — Checkpoints and semantic oracle

Capture after world load and after every completed steady update (121 records per lifetime).
Use a versioned, fixed-width, canonically ordered evidence encoding. Do not serialize C++ memory,
pointers, padding, wall-clock values, unordered-container order, OS handles or private roots.
Required fields are scenario/version, steady tick index, source frame, source logic RNG state
fingerprint, generated script flag, generated actors' IDs/template labels/team, object existence,
position/geometry and each retained scheduled module's next-call/deletion frame. Include terrain
samples at the two fixed actor positions and source player/side identity needed to interpret them.
Source RNG fingerprints must come from the original RNG state, without advancing it to inspect it.
Emit canonical bytes and a digest; compare bytes, not only an unexamined hash.

Assert semantic witnesses independently of cross-compiler equality:

1. Both actors exist in the initial loaded snapshot; IDs are distinct and valid; owners and
   module identities match the parsed fixture. The persistent actor survives all 120 updates.
2. The expiring actor's parsed delay is 30 source frames (1000ms at 30Hz). Its recorded deletion
   frame D is within the qualification window. It remains before the original scheduler reaches
   D and is removed by the end-of-frame destroy-list processing of the update entered at D.
   Capture source frame at update entry and exit to avoid an off-by-one oracle. A fixture must
   prove the scheduled callback ran; merely deleting an object from the harness does not pass.
3. The actual script sets H1Started during original script processing and it remains true.
4. Every steady update advances the source frame by exactly one. Source-required ordering
   remains scripts → terrain → modules → AI/build/partition → destruction/cleanup, inside the
   actual engine update order. An observation hook may record reachability but not replace work.
5. Terrain samples differ as encoded by the authored plateaus and original height conversion.
   Expected heights are calculated from authored values and the source height scale, separately
   from the provider under test. Constant-zero terrain or successful reads of absent backing fail.
6. Every corresponding record matches byte-for-byte across normal GCC, GCC ASan/UBSan and
   Clang ASan/UBSan on the same configuration, and across the three complete lifetimes.
   Floating fields use defined IEEE representations. Do not round away simulation divergence.

The implementation may choose a stable evidence codec and read-only accessors; it may not change
which facts are compared. A golden digest generated solely from one implementation is insufficient:
retain the semantic witnesses and original-source call trace. This evidence is not a new save,
replay or wire format and creates no Windows compatibility obligation.

## HQ-04 — Required and deferred provider boundary

| Cohort | Required for H1 | Physical output / broader behavior boundary |
|---|---|---|
| Native entry and original root | Actual startup order, INI registries, data roots, user-storage parent, option/error handling and reverse owner cleanup | No Win32 SDK emulation or replacement GameLogic |
| Original world | Actual map/sides/players/teams/scripts, templates, body/module ownership, partition, terrain height/layers/extents, ghost lifecycle, recorder/commands/AI/build/victory services reached by load/update | No mocked successful map load or flat base TerrainLogic stub |
| Client/display/view | Real source-owned Drawable registry/lifecycle and frame publication; coherent logical camera/view, time-freeze and terrain queries reached by H1 | Omit window/GPU presentation; do not fabricate visual success |
| Module registration | Original schemas/data constructors for definitions actually loaded; complete callbacks for modules actually instantiated by H1 | Uninstantiated draw providers do not need physical submission merely to qualify H1 |
| Models/bones | None requested by H1's two drawless actors | Any source consumer that does request a bone/bound/projectile query needs correct backing before its own scenario can pass; never substitute zeros |
| Audio/media | Required initialization/metadata, logical events and state reached by startup; use defined no-device/output behavior | No decoder/voice/playback success claimed; preserve relevant RNG and lifetime behavior |
| Storage | Original read-only data consumers plus separated game-owned XDG/user/cache writers reached by startup | No retail writes; cache remains disposable |
| LAN | No session in H1; original null-network single-player route | Discovery/map transfer/synchronized match remain R11 |

The native factory must classify every service actually requested on H1's original path as required
logical behavior, source-supported absent service, or deferred physical output. Link success alone
cannot establish that classification. Construction-only dependencies still require valid lifecycle
and cleanup; deep feature behavior is required only when reached by the qualification or its separate
existing acceptance. Never erase a real dependency merely because its name sounds graphical.

Retained components already verified in N2 remain useful and must not be reverted to simplify H1.
The 19 draw-provider family, complete ordered model tick, animated particle/audio effects, physical
terrain/water/shadows, UI/media and full-match parity are not automatically new prerequisites of a
**drawless** H1. Their required eventual behavior remains under R05/R08/R09/R10. Existing generated
provider tests remain regression obligations where changed code affects them.

Missing/unsupported provider admission must fail explicitly before pretending a requested scenario
is supported. A partial implementation may qualify H1 without claiming to support arbitrary game
scenarios. This is a qualification boundary, not permission to ship reduced game support.

## HQ-05 — Failure, retry and ownership

Test missing roots, malformed required INI, truncated map, missing required provider and a late
startup/map-admission allocation failure. Each reports a non-success stage/category, retires all
owned partial state and then accepts corrected H1 input in the same process. Keep accepted borrowed
configuration identity stable; guard raw acquisitions and stage fallible arguments before admission.
Exercise exact standard-allocation failure terminals and pool-exhaustion/used-unit retirement for
changed original-runtime owner boundaries, with every failure/retry pair retained under both compiler
sanitizers. Discovery and warmed baselines must use identical source cursor/configuration state.

A failed gameplay tick is not a successful partially executed tick. If no authoritative operation
contract promises recovery of an already-mutated world, retire that world, report failure and allow
fresh corrected startup; do not invent a universal transactional rollback of every simulation tick
as a prerequisite for H1. Existing per-operation rollback guarantees and tests remain binding.
Controlled interruption must retire owned resources; uncatchable process termination is not a
promise of in-process cleanup. No permission to weaken existing checks, timeouts or safe ownership.

## HQ-06 — Separate supplied-data startup probe

In the actual native entry, run original initialization against locally supplied read-only Generals/
Zero Hour roots, reach the post-initialization boundary and retire the engine. This verifies real
configuration/definition ingestion rather than only a BIG index or presence mask. It need not load
a full retail campaign/skirmish scene to qualify the foundation. Generated H1 remains the deterministic
loaded-world test; retail gameplay acceptance remains later and cannot be inferred from either probe.

Hash/snapshot input integrity before/after through the existing redacted audit mechanism. Retain only
public stage/category, completion and unchanged-integrity outcomes; no private selectors, names,
addresses, source hashes, files or derived caches in tracked artifacts. Missing supplied data is an
explicit external acceptance blocker, never PASS or permission to distribute data. User-storage is
separate; unwritable caches fall back to memory conversion as already required. Required writable
user-data failures are reported rather than silently writing into an asset mount.

## HQ-07 — Frozen validation and completion

Required evidence is one revision/configuration-specific record containing H1 source/call mapping,
full parser/provider manifest, generated input definition, 121×3 checkpoints per configuration,
semantic assertions, negative/retry ownership results, retail startup/integrity result and the complete
applicable frozen milestone regression matrix. Source identities distinguish committed and dirty code.
Run focused affected tests during implementation; run the broader matrix at a coherent frozen acceptance
boundary. An unchanged evidence record is not a reason to rerun an expensive matrix. Changed lifecycle
boundaries are not a reason to reuse stale acceptance. Timeouts mean incomplete execution.

H1, failure/retry, compiler comparison, repeated lifetimes and the supplied-data probe together qualify
R07's foundation. Existing N2 core/data and all applicable R02–R05 constraints must also hold.
No count of descriptors, source units, QA reports or completed subcomponents substitutes for this gate.
The final whole-game requirements and original N1 physical suitability gate remain unchanged.

## Source derivation and limits

- `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp::init/update`: actual root factories;
  update orders radar, audio, client, messages, optional network, then logic.
- `.../Source/GameLogic/System/GameLogic.cpp::startNewGame/update`: pending loading transition,
  original map setup, script/terrain/sleepy-module/AI/cleanup order, frame increment.
- `.../Source/GameLogic/Object/Update/DeletionUpdate.cpp` and
  `.../Include/GameLogic/Module/DeletionUpdate.h`: original parsed lifetime and scheduled destruction;
  `UpdateModule.cpp::setWakeFrame` binds source frame to scheduler.
- `.../Source/Common/INI/INIValueParsers.cpp::parseDurationUnsignedInt` and
  `.../Include/Common/GameCommon.h`: duration conversion and 30Hz fixed logic rate.
- `.../Include/GameLogic/Scripts.h`: CONDITION_TRUE and SET_FLAG are original script operations.
- `.../Source/GameLogic/ScriptEngine/VictoryConditions.cpp::update`: returns for non-multiplayer
  recorder, so an ordinary single-player fixture needs no invented victory suppression.
- `.../Include/Common/RandomValue.h`: original seed/state fingerprint interfaces.
- `.../Source/GameLogic/Object/Weapon.cpp::calcProjectileLaunchPosition`: client bone queries can
  affect gameplay outside H1; that dependency is preserved rather than declared irrelevant globally.
- [Runtime source graph](../docs/original-runtime-source-graph.md): base zero-height terrain is not
  shipped terrain; native client and module/data ownership are supporting, not startup acceptance.
- `tests/original/logical_world.cpp` shows generated original map chunks; `object_ownership.cpp`
  shows original parsed DeletionUpdate data. These are feasibility evidence, not H1 integration proof.

65×65/two-cell border, the seed and 120 updates are delegated test-fixture choices, not claims about
retail maps or production limits. The fixture must be encoded legally through the actual source
writers/readers; discovering a missing support parent is ordinary implementation work within the
finite path, not authority to replace its promised behavior. Exact generated binary bytes and CLI
syntax are implementation details, while the required state transitions and oracle above are fixed.
