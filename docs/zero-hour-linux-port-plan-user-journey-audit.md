# PRD User Journey Audit: Zero Hour foundation qualification

> Source PRD: [zero-hour-linux-port-plan.md](zero-hour-linux-port-plan.md)
> Contract: [headless-qualification.md](../product/headless-qualification.md)
> Generated: 2026-10-10T17:13:44.947436+00:00 (UTC)
> Review mode: Initial audit
> Compared against: REC-001 evidence; no prior journey audit
> Evaluation scope: HQ-01–07 definition plus bounded source inspection, no runtime execution.

## Executive verdict

All five scoped journeys are specification-complete. Runtime reachability is indeterminate:
the native concrete root/H1 integration remains unaccepted. No product-definition gap remains
from REC-001 after the technical contract. This is not a full visual, campaign or LAN journey audit.

## Audit method and status definitions

Independently trace entry, local permissions, source owner, transition, success, failure, recovery
and exit. Verified code is evidence of a component only. Complete describes specification;
Partial/Not verified describes implementation; Indeterminate never implies actor success.
Search scope: GeneralsMD original root/logic/client, cmake/OriginalRuntime.cmake and original/toolchain
tests, plus the current source graph and scoped QA. The link probe explicitly cannot execute gameplay.

## Actors and starting contexts

| Actor | Begins At | Authentication State | Prerequisites / Provisioning | Intended Capabilities | Available Entry Points |
|---|---|---|---|---|---|
| Local developer | Generated world or supplied-data probe | Ordinary OS user; no game account | Local build and generated/explicit roots | HJ-01,04 | Specified HEP-01; complete native entry pending |
| Automated runner | Frozen generated fixtures | Inherited local test permissions | Same seed/configuration/cursors; compiler builds | HJ-02 | Specified HEP-02 |
| Asset owner | Private original data | Local file owner; no upload | Original files provided locally | HJ-03 | Specified HEP-03; component root audit evidenced |
| Maintainer | Candidate acceptance evidence | Local repository access | Frozen records/configurations | HJ-05 | Specified HEP-04 |

## Required entry points

| Entry Point | Actor | Capability / Journey | Required or Evidenced Surface | Discoverability | Status | Evidence / Gap |
|---|---|---|---|---|---|---|
| HEP-01 | Developer | HJ-01,04 | Native bounded headless/retail invocation | HQ-01 requires executable documentation | Required, not complete | J-GAP-01; root probe is link-only |
| HEP-02 | Runner | HJ-02 | Generated failure/fault test invocation | HQ-05/07 evidence commands | Specified; component tests only | tests/original allocation fixtures |
| HEP-03 | Asset owner | HJ-03 | Private explicit-root configuration | HQ-06 and existing audit wrapper | Specified; historical component evidence | tools/run_data_audit.py, N2-original-data.md |
| HEP-04 | Maintainer | HJ-05 | Frozen matrix/evidence comparison | HQ-07 requires commands/configurations | Specified; unexecuted integrated matrix | HQ-03/07 |

## Capability reachability

| Capability | Actor | Journey | PRD Evidence | Spec Coverage | Implementation Evidence | Reachability | Primary Gap |
|---|---|---|---|---|---|---|---|
| HJ-01 | Local developer | Generated original-world execution | HQ-01–04 | Complete | Partial | Indeterminate | J-GAP-01 |
| HJ-02 | Automated runner | Failure and corrected retry | HQ-05 | Complete | Partial | Indeterminate | J-GAP-01 |
| HJ-03 | Asset owner | Provide private original input | HQ-06 / R04 | Complete | Partial | Indeterminate | None; current retail availability unverified |
| HJ-04 | Local developer | Original supplied-data startup | HQ-06 | Complete | Partial | Indeterminate | J-GAP-01 |
| HJ-05 | Maintainer | Compare evidence and qualify foundation | HQ-03/07 | Complete | Not verified | Indeterminate | None; acceptance not executed |

## User journeys

### HJ-01 — Generated original-world execution (Local developer)

- **Promise and evidence:** HQ-01–04.
- **Begins:** Prepared generated data; ordinary local account; no runtime world.
- **Entry points:** HEP-01 documented native headless entry.
- **Permissions and preconditions:** Read generated roots; write only separate user storage; source-supported single-player providers.
- **Happy path:** Select H1/seed/ticks → original init/load → observe two actors and script → 120 real updates → 121 records → shutdown; repeat three lifetimes.
- **Alternate paths:** Different invocation CWD preserves explicit-root behavior; no physical GPU/audio device is required for correctly defined absent-output services.
- **Failure paths:** Invalid fixture/provider or failed tick reports non-success; no synthetic load/update or false completed result.
- **State transitions:** Prepared→initialized→loaded→running→retired; only successful real updates advance steady counter.
- **Recovery:** Retire failed world and correct input before fresh same-process initialization.
- **Exit or reversal:** Stop after bounded updates and release complete owner graph; controlled interruption is non-success with cleanup.
- **Assessment:** Spec Complete; implementation Partial; reachability Indeterminate; gap J-GAP-01.

### HJ-02 — Failure and corrected retry (Automated runner)

- **Promise and evidence:** HQ-05.
- **Begins:** Same generated inputs/seed/configuration; fault discovery outside measured baseline.
- **Entry points:** HEP-02 generated negative/fault invocation.
- **Permissions and preconditions:** Same local permissions; no borrowed-global replacement that bypasses legitimate cleanup.
- **Happy path:** Select malformed/fault prefix → original failure → verify retirement/preservation → corrected same-owner or fresh-root retry as defined → compare H1.
- **Alternate paths:** Separate exact standard allocation terminal and explicit pool-exhaustion checks; divide independent sweeps while retaining every pair.
- **Failure paths:** Timeout is incomplete; mismatch/leak/wrong provider/partially executed successful tick fails.
- **State transitions:** Ready→failed partial owner→retired→corrected original startup→accepted lifetime.
- **Recovery:** Preserve per-operation promised rollback; otherwise failed world retires; never fabricate universal tick atomicity.
- **Exit or reversal:** Retire all jobs/roots after success or failure, retaining failure/retry record.
- **Assessment:** Spec Complete; implementation Partial; reachability Indeterminate; gap J-GAP-01.

### HJ-03 — Provide private original input (Asset owner)

- **Promise and evidence:** HQ-06 / R04.
- **Begins:** Locally owned original data, no upload authorization.
- **Entry points:** HEP-03 explicit local data-root configuration.
- **Permissions and preconditions:** OS read permission only; inputs immutable; generated fixture/storage separate.
- **Happy path:** Identify local roots privately → configure native invocation/audit locally → make data readable → receive only redacted availability/integrity outcome.
- **Alternate paths:** Root independent of CWD; generated qualification can proceed while retail inputs are unavailable.
- **Failure paths:** Missing/unreadable/incomplete input is external retail-acceptance blocker; no copying proprietary data into repository.
- **State transitions:** Unconfigured→locally configured→readable or unavailable; input bytes unchanged.
- **Recovery:** Correct local configuration/permissions/content; rerun read-only probe.
- **Exit or reversal:** Remove local configuration without changing source data; no cloud account or upload.
- **Assessment:** Spec Complete; implementation Partial; reachability Indeterminate; gap None; current retail availability unverified.

### HJ-04 — Original supplied-data startup (Local developer)

- **Promise and evidence:** HQ-06.
- **Begins:** Configured read-only retail roots and separate user storage.
- **Entry points:** HEP-01 same native entry; retail-probe selection.
- **Permissions and preconditions:** Read original data; no input-derived identifiers or hashes in tracked diagnostics.
- **Happy path:** Integrity snapshot → actual original init/configuration load → post-init checkpoint → teardown → compare integrity → redacted result.
- **Alternate paths:** Absent/corrupt/unwritable disposable cache rebuilds/falls back in memory; arbitrary CWD supported.
- **Failure paths:** Required user-storage failure or data/initialization failure is non-success; assets unchanged; correct and retry.
- **State transitions:** Input-ready→initialized→retired→integrity verified; no claim of loaded retail gameplay.
- **Recovery:** Retire partial root, correct local prerequisite, repeat same process where contract requires.
- **Exit or reversal:** Terminate cleanly; do not persist retail derivatives into tracked evidence.
- **Assessment:** Spec Complete; implementation Partial; reachability Indeterminate; gap J-GAP-01.

### HJ-05 — Compare evidence and qualify foundation (Maintainer)

- **Promise and evidence:** HQ-03/07.
- **Begins:** Frozen coherent revision/configuration and completed candidate records.
- **Entry points:** HEP-04 documented validation/evidence collection commands.
- **Permissions and preconditions:** Only generated public traces and redacted retail outcomes may be retained.
- **Happy path:** Verify source/configuration identities → check semantic witnesses → compare checkpoint bytes across compilers/lifetimes → check failures/integrity/full applicable regression → report bounded R07 result.
- **Alternate paths:** Component evidence remains reusable for unchanged scopes; later parity gates retain separate status.
- **Failure paths:** Missing logs, incomplete execution, divergent frame/RNG/world state or stale source identity prevents acceptance.
- **State transitions:** Candidate→verified bounded acceptance or incomplete/failed; no automatic milestone promotion here.
- **Recovery:** Correct evidence/implementation in delivery and rerun affected frozen checks, never edit expected behavior to match defect.
- **Exit or reversal:** Publish public report; retire test resources; retain prior failure/retry pairs.
- **Assessment:** Spec Complete; implementation Not verified; reachability Indeterminate; gap None; acceptance not executed.

## Action-to-surface mapping

| Action ID | Journey / Path / Step | Actor Action | UI or Entry Point | Backend/API | Permission Check | State Change | Feedback / Next State | Evidence / Gap |
|---|---|---|---|---|---|---|---|---|
| HA-01 | HJ-01 setup | Select H1 roots/seed | HEP-01 | Original root factories/parsers | Local reads and separate writes | Prepared→initialized | Stage or non-success | HQ-01; J-GAP-01 |
| HA-02 | HJ-01 load | Request single-player map | HEP-01 | GameLogic::startNewGame | Declared generated roots | Initialized→loaded actors/scripts | Loaded snapshot | HQ-02; original GameLogic.cpp |
| HA-03 | HJ-01 run | Request bounded updates | HEP-01 | GameEngine::update→GameLogic::update | Main-thread owned world | Script sets flag; scheduled actor removed | Checkpoints after each actual tick | HQ-03; DeletionUpdate.cpp |
| HA-04 | HJ-01 exit | Finish/repeat | HEP-01 | Root teardown/restart | Legitimate singleton identities through cleanup | World/root retired | Three matching lifetimes | HQ-01/05 |
| HA-05 | HJ-02 reject | Arm one negative/fault | HEP-02 | Original startup/map admission | Fixture-owned fault controls | Partial owner→failure | Failure category, no success | HQ-05 |
| HA-06 | HJ-02 recover | Correct same input | HEP-02 | Same-owner retry/fresh-root init | Identical borrowed configuration | Retired/recoverable→accepted | Preserved pair and exact terminal | HQ-05 |
| HA-07 | HJ-02 exit | End test cohort | HEP-02 | Whole-owner destruction | No orphaned globals/handles | No live measured owners | Sanitizer/resource result | HQ-05/07 |
| HA-08 | HJ-03 supply | Configure original roots | HEP-03 | Root configuration | Owner grants OS read access only | Private configuration exists | Redacted availability | HQ-06 |
| HA-09 | HJ-03 repair/exit | Fix/remove local configuration | HEP-03 | Local root selection | No retail write/upload | Retry or unconfigured | Missing-input category if absent | HQ-06 |
| HA-10 | HJ-04 initialize | Run original retail startup | HEP-01 | GameEngine::init and original INI stores | Immutable mounts; separate user-data root | Ready→post-init | Public stage status only | HQ-06; J-GAP-01 |
| HA-11 | HJ-04 retire | End probe | HEP-01 | Root teardown and integrity wrapper | Read-only snapshot mechanism | Retired; unchanged input | Redacted integrity result | HQ-06 |
| HA-12 | HJ-04 recover | Correct missing/corrupt prerequisite | HEP-01 | Original startup retry | No write into source root | Failed→fresh attempt | Explicit recovery result | HQ-05/06 |
| HA-13 | HJ-05 inspect | Bind source/configurations | HEP-04 | Evidence manifest | Generated public records only | Candidate pinned | Mismatch rejects | HQ-07 |
| HA-14 | HJ-05 compare | Check semantics and bytes | HEP-04 | Original-state observations and comparison | Read-only to simulation | Candidate→pass/fail | No hash-only or stub success | HQ-03 |
| HA-15 | HJ-05 finish | Publish bounded conclusion | HEP-04 | Evidence reporting | Redacted retail result | Accepted or incomplete record | R07 scope, later gates pending | HQ-07 |

## Lifecycle completeness

| Lifecycle Area | Actor | Required Flow | Entry Point | Success / Terminal State | Failure / Recovery | Status | Evidence / Gap |
|---|---|---|---|---|---|---|---|
| Setup/init/load | Developer | HJ-01 | HEP-01 | Real loaded world | Retirement/corrected retry | Specified complete | HQ-01/02, J-GAP-01 |
| Execute/observe | Developer, runner | HJ-01,02 | HEP-01/02 | Real ticks and source facts | Non-success, never fake tick | Specified complete | HQ-03/05 |
| Input provisioning | Asset owner | HJ-03 | HEP-03 | Read-only local access | External availability blocker | Specified complete | HQ-06 |
| Teardown/repeat | Developer, runner | HJ-01,02,04 | HEP-01/02 | Three independent owner lifetimes | Cleanup then retry | Specified complete | HQ-01/05/06 |
| Evidence publication | Maintainer | HJ-05 | HEP-04 | Revision-specific public report | Stale/missing evidence rejects | Specified complete | HQ-07 |

Accounts, role grants, remote authentication, invitations and logout are N/A: this amendment
introduces no account service. New save/replay/LAN/visual journeys are N/A to this amendment;
the existing R08–R11 promises remain required and unmodified.

## Interface and reachability mismatches

### Unreachable capabilities

No runtime reachability assertion is made; HJ-01/04 lack completed native integration (J-GAP-01).
### Backend capabilities without user-facing interfaces

Original owners exist; the native CLI is required and incomplete. No GUI is required for this developer workflow.
### Interfaces without supporting backend behavior

The contract specifies an entry yet to be delivered. Existing link-only root probe is not that interface.
### Orphaned behavior

None established. Existing CPU model/audio work can support future fidelity; it is not deleted or reclassified as orphaned.

## Coverage matrix

| Capability | User Role | UI or Entry Point | Backend/API | Data/State | Authorization | Error States | Tests |
|---|---|---|---|---|---|---|---|
| HJ-01 | Developer | Specified: HEP-01 | Verified: original root/logic; Gap (implementation): J-GAP-01 | Specified: H1 loaded→120 updates→retired | Specified: local reads/separate writes | Specified: missing/malformed/provider/failure | Specified: real script/deletion/terrain oracle; Unverified: integration |
| HJ-02 | Runner | Specified: HEP-02 | Verified: component owner fixtures; Unverified: composed retry | Specified: exact failure/retry pairs | Specified: identical source/configuration | Specified: leaks/stale owners/timeouts fail | Specified: both sanitizers and pool/standard terminals |
| HJ-03 | Asset owner | Specified: HEP-03 | Verified: rooted data audit | Specified: immutable inputs | Specified: owner local read access only | Specified: absent data blocks private probe | Verified: historical integrity probe; Unverified: current availability |
| HJ-04 | Developer | Specified: HEP-01 | Gap (implementation): J-GAP-01 | Specified: original retail init→retire | Specified: no private tracked data | Specified: cache fallback/required-write failure | Specified: original init and input integrity; Unverified: execution |
| HJ-05 | Maintainer | Specified: HEP-04 | Specified: evidence comparison | Specified: bounded qualification record | Specified: generated/redacted evidence | Specified: no stale or incomplete PASS | Specified: all HQ-01–07 plus existing constraints |

## Gap register

| Gap | Severity | Type / Surface | Affected Actors and Capabilities | Evidence | User Impact | Required Resolution | Disposition |
|---|---|---|---|---|---|---|---|
| J-GAP-01 | Blocker for implementation acceptance, not definition | Native runtime integration | Developer/runner HJ-01,02,04 | Explicit link-only root target; no concrete native GameEngine subclass | Cannot claim executable complete journey | Deliver the already specified original native startup/loaded-world contract | Open implementation obligation |

## Remediation verification

No prior journey audit; formal prior-gap dispositions N/A. REC-001 is addressed by HQ-01–07's
finite path and oracle; canonical reconciliation must independently converge that finding.

## Unmapped or uncertain PRD statements

No HQ-01–07 statement unmapped. Inherited R01–R12 baseline is unchanged; full game journey
recertification is outside this bounded amendment and no such acceptance is inferred.

## Assumptions and open questions

No material product assumption within H1. Legal fixture encoding and concrete factories remain
implementation responsibilities. Supplied-data availability and full runtime behavior are unverified,
not product choices requiring a new interview.

## Coverage summary

4 distinct actors, 4 context rows, 5 capabilities/journeys, 5 coverage rows, 4 entry points, 15 actions.
5 specification-complete journeys; 0 specification gaps. Runtime reachability: 5 Indeterminate.
1 implementation-acceptance blocker, 0 major and 0 minor gaps. No accepted-risk waiver or prior
journey remediation history. Required implementation tests remain unexecuted here.
