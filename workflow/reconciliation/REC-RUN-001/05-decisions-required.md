# Decisions required

## REC-DEC-001 — Finite headless qualification contract

Type: technical definition resolution (product qualification, startup journey and architecture boundary).
Source finding: REC-001. Product Owner feedback: none canonical; raw OBS-001 is observational.
Authority/owner: delegated engineering decisions under the approved plan; Product Owner only if a
real scope/fidelity tradeoff emerges. No new product approval is required merely to investigate or
choose an equivalent internal implementation.

Question: **What finite original-engine startup/load/update/checkpoint/shutdown scenario and source-derived headless provider contract sufficiently qualify R07 while preserving R05 and leaving physical rendering/media acceptance in R08/R09?**

Reason: The approved intent fixes real source execution and fidelity, but neither the specification nor current plans settles the workload, observable checkpoint contract or required-versus-deferred provider closure; new prerequisites keep accumulating in slice03.

Required resolution record should identify the legal generated map/actor/module workload, actual
entry/factory and update path, deterministic seeds/ticks/checkpoint fields and comparison semantics,
repeat/failure/retry outcomes, necessary simulation-facing client queries, and the separate read-only
retail check. Every omitted output path needs a source argument; do not substitute a toy simulation,
zero terrain or fake successful media. Do not turn the first qualification scenario into permission
to omit required eventual game behavior.

No enumerated product options: this is an open technical closure question, not a request to choose
between fidelity and progress. The candidate text in [02-source-changes.md](02-source-changes.md)
is non-authoritative and deliberately does not select the scenario or provider design.

Affected users/behavior: developers qualifying native startup, and eventual players whose gameplay
must remain equivalent. Definition domains: product, user_journeys, architecture. Target authority:
[docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md) R07/Validation and completion. Derived impact: N2 definition/plans, runtime source graph,
readiness assumptions and current delivery resume metadata; N3/N4 boundaries remain constraints.

Consequence of deferral: no reliable finite integration stop condition; the current slice can continue
absorbing model/client/audio work while loaded simulation remains unproven. Dependent recompiled
remediation cannot truthfully be marked ready.

blocking: true
implementation_ready: false

No other product, UX redesign, security, renderer replacement or legacy-migration decisions are
established by this review. Delivery-unit sizing and coherent current status are downstream planning
responsibilities, not additional game-product choices manufactured here.
