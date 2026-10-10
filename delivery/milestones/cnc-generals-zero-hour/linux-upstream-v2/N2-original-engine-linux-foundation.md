# Milestone N2: Original engine Linux foundation

## Objective

Qualify the actual original Zero Hour Linux runtime through the finite H1 loaded-world
contract and a separate read-only original supplied-data initialization probe.

## User/System Outcome

A developer can initialize the original engine, load H1, observe real script/scheduled
object behavior for 120 updates, retire it and repeat with matching original-state
checkpoints across supported compiler configurations. Supplied data reaches actual
post-init and clean teardown without changing input. This qualifies the foundation.

## Scope

Retain original core/platform/allocator and rooted-data work already accepted.
Finish native startup/load/update/teardown, source-required logical providers,
generated H1, canonical observations, negative/retry ownership and retail startup.
[HQ-01–HQ-07](../../../../product/headless-qualification.md) is normative in full.

## Explicit Exclusions

No toy simulator, successful link-only probe, full retail match, physical rendering,
audio/video playback or complete draw-provider family as an automatic H1 prerequisite.
Drawless H1 does not request model/bone queries; any scenario that does must have correct
backing. Complete ordered model ticks/physical effects remain with N3, interactive/media
with N4, and combat/full-match/persistence/determinism with N5. Existing work and affected
regression obligations are retained. No universal rollback of every simulation tick is
introduced; existing operation-level rollback contracts remain binding.

## Source Requirements

R03/R04/R07 and shared R02–R05 in `docs/zero-hour-linux-port-plan.md`.
HQ-01–07; journeys HJ-01–05 and entry points HEP-01–04; UX-HQ-01–05 in
`product/ux/headless-experience.md`; scoped architecture/security constraints in
`product/architecture/headless-review.md` and `product/security/headless-review.md`.
Source revisions are recorded in `status.yaml::compiled_from` and the definition package.
The user-approved graph remains N0→N2; N1 is not a headless dependency.

## Preconditions

- N0 accepted; original baseline and active packet remain authoritative.
- PRE-01, PRE-04, PRE-05, PRE-10, PRE-11: see the prerequisite manifest under
  `delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/`.
- Generated H1 development does not wait for retail data. PRE-04 is mandatory before
  the separate supplied-data probe and before N2 acceptance.

## Readiness checks

- Inspect `status.yaml`: N0 completed with evidence; N2 remains in progress.
- Verify definition-package hashes and all HQ-01–07 mappings in `traceability.md`.
- Inspect `docs/build.md` headless commands: `ZH_BUILD_RENDERER=OFF`, GCC and Clang,
  `ZH_SANITIZE=ON` for both owner-sanitizer configurations.
- Before production edits, replan remaining N2 against this contract using a new governing
  minimal-slice plan. Preserve accepted core/data slices and inspect pre-existing dirty
  work without treating it as accepted. Old slice03 and Full475/Full509 resume pointers
  are historical progress, not authority to continue an unbounded task.

## Functional Requirements

- Native entry documents explicit roots, separate storage, scenario/seed/tick selection.
  Invoke actual `GameEngine::init`, `GameLogic::startNewGame`, `GameEngine::update` and
  common `GameLogic::update`; observe loading completion before counting steady ticks.
- H1: legal 65×65 two-cell-border map, height-20/24 plateaus, source-required sides/teams;
  two authored drawless actors H1Persistent/H1Expiring; registered original DeletionUpdate
  with both lifetimes 1000ms; original nonrepeating CONDITION_TRUE/SET_FLAG H1Started
  script; seed 0x5A170001 and 120 completed steady updates. Retain original victory owner.
- Classify every reached service per HQ-04 as required logical behavior, source-supported
  absence or deferred physical output. Use actual original parsers/factories and legitimate
  original client/Drawable/view/time/terrain services. Unsupported admission fails explicitly.
- Record post-load and every update: 121 canonical fixed-width records per lifetime,
  three complete lifetimes per configuration. Include every HQ-03 field and emit bytes
  plus digest; compare bytes across normal GCC, GCC ASan/UBSan and Clang ASan/UBSan.
- Independently assert source actor identity, survivor lifetime, 30-frame parsed deletion
  delay and actual scheduled callback/destroy-list frame D, original script flag, frame+1,
  call order, original RNG observations and two source-scaled nonuniform terrain samples.
- Missing roots, malformed required INI, truncated map, missing required provider and late
  startup/map-admission allocation failure produce non-success, complete partial retirement
  and corrected same-process retry. Preserve exact allocation terminals/every pair and
  separate pooled exhaustion/used-unit checks for changed owner boundaries.
- Failed unsupported tick recovery retires the world before fresh startup. Preserve valid
  parent singletons through cleanup and identical borrowed configuration/source cursor
  baselines. Controlled interruption is incomplete with cleanup, never a successful run.
- Run actual original initialization using supplied read-only roots through post-init,
  teardown and unchanged before/after integrity. The prior stage4/mask63 index result
  is component evidence only. Adapt bounded public audit output for this distinct probe.

## UX Constraints

Implement UX-HQ-01–05 and checklist UX-HQ-C01–05: help works without starting a world;
explain generated versus retail probe and inputs; report init/load/run/teardown and
completed/incomplete/failed distinctly. Missing input/provider/storage and execution
failures are actionable public categories and non-success. Corrected inputs can retry.
Plain text remains understandable without color, sound or pointer use. Success names
only its selected headless/startup scope, never game parity. No new graphical UI.

## Architecture / Security Constraints

Use stock dependencies and public APIs. Preserve original source semantics, external
encodings, allocator pairing and thread ownership; consult `docs/original-engine-formats.md`.
No retail writes or tracked private roots/names/addresses/content hashes/derived assets.
Separate required writable user storage from immutable roots and optional disposable
cache. Unwritable cache falls back to memory; required-storage failure is explicit.
Validate lengths/traversal and guard fallible acquisition/admission/constructor boundaries.

## Interfaces and Compatibility

Native CLI spelling and evidence codec are engineering choices to document. Checkpoint
encoding is qualification evidence, not a save/replay/network format. Fixed-width original
asset/persistence encodings remain unchanged. No legacy Windows/MSVC ABI obligation.

## Acceptance Criteria

- [ ] Actual native entry/load/update trace and complete reached-provider manifest prove HQ-01/04.
- [ ] H1 produces all HQ-02/03 semantic witnesses, 121×3 records per configuration and
  byte equality across all three configurations/lifetimes; no replacement callbacks.
- [ ] All HQ-05 negative/retry and ownership checks pass without omitted failure/retry pairs.
- [ ] Actual supplied-data post-init/teardown and unchanged integrity pass per HQ-06;
  missing private data remains an acceptance blocker, never a waived or synthetic pass.
- [ ] Revision-specific HQ-07 record binds source, fixture, providers, commands, checkpoints,
  both sanitizer results, retail result and the complete applicable frozen regression matrix.
  Accepted core/data behavior and affected model/client/provider tests remain correct.

## Required Validation

Use focused affected tests during implementation, then one coherent frozen normal-GCC,
GCC-ASan/UBSan and Clang-ASan/UBSan matrix for changed owners and all applicable milestone
regressions. Retain call traces, semantic witnesses, negative boundaries, exact fault
terminals/pairs, repeated lifetimes, public checkpoint bytes and redacted retail integrity.
An unchanged record needs no repeated expensive suite; changed lifecycle code needs fresh
affected evidence. A timeout is incomplete. No runtime tests are claimed by this packet.

## Known Risks / Deferred Work

Concrete native integration and H1 are remaining implementation, not accepted by this
amendment. Dirty component work must be dispositioned by delivery. The downstream planner
must choose a small set of independently closeable behavior outcomes with finite exits;
this packet creates no slices. Required source parents discovered on the H1 path stay
in scope; unrelated physical/provider completeness cannot silently enlarge H1. If the
normative workload cannot be implemented faithfully, return the named contradiction for
reconciliation instead of weakening acceptance or growing the workload without authority.
