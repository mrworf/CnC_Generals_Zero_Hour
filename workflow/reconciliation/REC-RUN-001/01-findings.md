# Findings

| Finding | Title | Classification | Severity | Reproduction | Confidence | Decision | Implementation | Definition revalidation |
|---|---|---|---|---|---|---|---|---|
| REC-001 | Headless qualification has no bounded scenario and provider boundary | PRODUCT_SPEC_GAP | HIGH | NOT_ATTEMPTED | High for documentary gap; medium for causal contribution | REC-DEC-001 | Blocked for dependent scope | Required |
| REC-002 | Component progress is real and N2 acceptance is correctly withheld | EXPECTED_BEHAVIOR | LOW | NOT_ATTEMPTED | High within inspected source and historical reports | No | No new remediation | No |

### REC-001: Headless qualification has no bounded scenario and provider boundary

Classification: PRODUCT_SPEC_GAP
Severity: HIGH
Confidence: High for missing qualification detail; Medium for its contribution to elapsed work
Reproduction: NOT_ATTEMPTED (document/history/source inspection, not runtime reproduction)

Evidence:
- [REPORTED] OBS-001, user message in this invocation, October 10, 2026: “Rereview current state and progress of this project using the $product-reconciliation skill, because the last work done here indicates that the milestones were not defined properly and the prior work got stuck for a long time on a single milestone and a single slice.”
- [DOCUMENTED] OBS-002, [docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md), R07 and Delivery order: original startup/headless simulation, with rendering and interactive/media outcomes in N3/N4.
- [REPOSITORY_EVIDENCED] OBS-003, [delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01.md](../../../delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01.md), Scope and slices / Recovery and execution: two completed slices, one remaining integration slice; ordinary port/build/fixture work stays in that transaction.
- [REPOSITORY_EVIDENCED] OBS-004, [delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01_slice_03.md](../../../delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01_slice_03.md), initial goal and latest ordered-tick batch: 41 initial lines, 4,752 at HEAD, 4,771 dirty; 100 post-creation commits edit the file. Repeated prohibitions on replacement micro-plans coexist with many distinct independently verified cohorts.
- [REPOSITORY_EVIDENCED] OBS-005, [cmake/OriginalRuntime.cmake](../../../cmake/OriginalRuntime.cmake), original_bootstrap/original_logic/original_runtime_link_probe; [tests/toolchain/original_runtime_link_probe.cpp](../../../tests/toolchain/original_runtime_link_probe.cpp): archives and a nonexecuting link diagnostic, not a native startup test.
- [REPOSITORY_EVIDENCED] OBS-006, [GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Weapon.cpp](../../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Weapon.cpp), Weapon::calcProjectileLaunchPosition calls Drawable::getProjectileLaunchOffset; [docs/original-runtime-source-graph.md](../../../docs/original-runtime-source-graph.md), final factory correction: common terrain's zero ground-height stub is insufficient for shipped semantics. Client/device-named code cannot be blindly deferred.
- [REPOSITORY_EVIDENCED] OBS-007, [workflow/delivery/state.yaml](../../../workflow/delivery/state.yaml), resume_point/next/active_implementation: Full509 versus Full475 and old active checkpoint. Supporting history has no single consistently current resumption view.
- [DOCUMENTED] OBS-008, [delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/readiness-report.md](../../../delivery/readiness/cnc-generals-zero-hour/linux-upstream-v2/readiness-report.md): structural dependencies/environment readiness were checked; no executed future integration or finite N2 workload was established.

Product Owner feedback: None canonical. OBS-001 is raw observational evidence, not a directive to remove fidelity or validation requirements.

Current observed behavior:
N2 accepts only core and data slices. The remaining slice repeatedly extends into native client,
terrain, camera, model, particle and audio ownership prerequisites. No finite N2 runtime qualification
cohort establishes which of these are necessary for that gate versus later whole-game parity. This
finding does not establish that every current subtask is unnecessary. Work duration is reported;
Git history establishes concentration and growth, not a wall-clock efficiency measurement.

Relevant implementation:
The build compiles original GameEngine/GameMain/GameLogic and native client support. The root probe
is EXCLUDE_FROM_ALL and explicitly never executes gameplay. Source declaration census finds only
Win32GameEngine deriving from GameEngine. NativeHeadlessGameClient supplies logical services.
Existing gameplay queries require model data; actual factory/startup/loaded-world acceptance is pending.
Dirty lazy-pose/locomotor modifications were inspected selectively and are unvalidated here.

Relevant UX rule:
R05/R09 and the plan's preservation/loading/error rules remain applicable and coherent. No redesign
or new visual rule follows from this evidence; headless observability belongs to qualification.

Relevant user journey:
Slice03 describes native entry with explicit roots → original initialization → legal generated map/
actor setup → bounded ticks/checkpoints → shutdown → same-process retry. Its exact workload, provider
coverage and checkpoint fields are not settled. No standalone canonical journey artifact was found.

Relevant product requirement:
R07 in [docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md) establishes real original-engine execution; R03 requires both compilers and R05
preserves semantics. N2's objective and acceptance bullets repeat that broad outcome. They do not
select the finite demonstration or adjudicate the growing cross-layer provider closure.

Architecture/security constraint:
The approved architecture permits game-internal redesign and headless omission of physical output
while retaining simulation semantics. Source queries cross the client/logic boundary. Upstream-clean
libraries, read-only assets, safe input boundaries and owner/compiler checks remain mandatory.

Root source of inconsistency:
The first unresolved semantic layer is the R07 qualification boundary: preservation intent is clear,
but there is no agreed finite contract for proving it at N2. Missing slice decomposition and conflicting
resume fields are delivery-process defects that amplify this gap; they are not misclassified as game
bugs. No evidence proves the approved high-level graph impossible, a renderer conflict, or a need
for new product scope. An omission alone would not justify this finding; the expanding provider
closure and still-absent executable acceptance make it materially consequential.

Authoritative desired state:
Actual original-engine startup, source gameplay updates and compiler-matching checkpoints with safe
repeat teardown remain required. Exact finite qualification scenario and headless provider contract:
unresolved pending REC-DEC-001. A small fixture must not replace gameplay or falsely accept full parity.

Authoritative artifact requiring change:
[docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md), R07 / Validation and completion: candidate bounded qualification clarification only.
The corresponding headless architecture detail may need an accepted technical contract; reconciliation
will not invent it or modify architecture authority.

Changed automatically: No
Decision required: REC-DEC-001, technical acceptance-boundary resolution
Implementation remediation required: Blocked for dependent scope; no new ready backlog entry

Affected definition domains:
- product: true
- user_journeys: true
- ux: false
- architecture: true
- security: false

Potentially stale artifacts:
- [docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md), only R07 qualification/startup headless boundary requires resolution; no wholesale invalidation.
- The N2 milestone, N2 governing/slice03 plans, runtime source graph, readiness report and delivery state
  are derived planning/evidence records needing later reconciliation with that outcome, not Product
  Definition authority. They are not mutated or marked stale/passed here.

Implementation validation:
Once resolved, preserve both-compiler deterministic execution of the actual selected loaded scenario,
positive and malformed/missing data cases, same-process failure/retry, repeated owner teardown,
applicable sanitizer fault coverage and redacted retail integrity checks. Every previously verified
component remains tied to its exact source identity. No existing timeout or required check is waived.

### REC-002: Component progress is real and N2 acceptance is correctly withheld

Classification: EXPECTED_BEHAVIOR
Severity: LOW
Confidence: High within inspected scope
Reproduction: NOT_ATTEMPTED

Evidence:
- [DOCUMENTED] OBS-009, [evidence/qa/N2-original-core.md](../../../evidence/qa/N2-original-core.md) and [evidence/qa/N2-original-data.md](../../../evidence/qa/N2-original-data.md): actual core/data owners and scoped acceptance, not full simulation.
- [REPOSITORY_EVIDENCED] OBS-010, [cmake/OriginalRuntime.cmake](../../../cmake/OriginalRuntime.cmake): substantial original-source libraries and generated fixtures; [evidence/qa/N2-audio-admission-ownership.md](../../../evidence/qa/N2-audio-admission-ownership.md) records focused7/related72 passing all three configurations, explicit outstanding full509/startup gates, source fingerprints and fault/retry history.
- [REPOSITORY_EVIDENCED] OBS-011, [delivery/milestones/cnc-generals-zero-hour/linux-upstream-v2/status.yaml](../../../delivery/milestones/cnc-generals-zero-hour/linux-upstream-v2/status.yaml): N0/N1 completed, N2 in progress, N3–N7 pending, consistent with the broad acceptance limits.
- [REPOSITORY_EVIDENCED] OBS-012, starting worktree: 17 modified paths, 461 insertions/34 deletions; previous audio acceptance cannot validate these new changes.

Product Owner feedback: None canonical; correlated to OBS-001's request to reassess progress.

Current observed behavior:
There is meaningful implementation work despite no additional completed milestone. Narrow historical
component passes and absent integrated acceptance are explicitly distinguished. A percentage based
on two of eight milestones, QA report count or number of compiled units would be misleading.

Relevant implementation:
Original core/data source targets, original_logic and bootstrap archives, NativeHeadlessGameClient,
model/particle/audio fixtures. Ownership details were selectively inspected, not comprehensively audited.

Relevant UX rule:
R05/R09 prohibit treating placeholders or missing output as completed game experience. No violation
of that reporting boundary was established; actual visual parity remains unverified.

Relevant user journey:
N2's startup-to-loaded-simulation gate and N3–N7 user journeys remain unaccepted.

Relevant product requirement:
R02–R12 and Validation and completion in [docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md) require source fidelity and separate platform,
rendering, gameplay and release evidence. Partial component evidence is not whole-game acceptance.

Architecture/security constraint:
Keep the current upstream/public-API and read-only-asset boundaries. No newly evidenced conflict or
requested security/architecture change in this finding.

Root source of inconsistency:
No inconsistency in withholding milestone acceptance. The unresolved planning issue is REC-001;
this finding closes the narrower progress interpretation, not the entire user concern.

Authoritative desired state:
Preserve verified component work, label source/revision limits, and leave milestone acceptance open
until its real original-runtime criteria pass. Never use test count or renamed checkpoints as parity.

Authoritative artifact requiring change: None
Changed automatically: Not applicable
Decision required: No
Implementation remediation required: No new remediation from this finding

Affected definition domains:
- product: false
- user_journeys: false
- ux: false
- architecture: false
- security: false

Potentially stale artifacts: None from this finding.
Implementation validation: Existing incomplete milestone gates remain; this review reruns none.

## Evidence blockers and reproduction limits

No additional unclassified finding is manufactured. Total working time and a causal percentage
attributable to tests, scope or tooling are unavailable. The exact necessity of every remaining
provider requires REC-DEC-001's source trace. No fresh runtime or visual evidence is available;
NOT_ATTEMPTED must not be read as a test failure. Recovery-era claims are outside scope.
