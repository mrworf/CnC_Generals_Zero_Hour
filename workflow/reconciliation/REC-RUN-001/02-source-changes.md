# Source changes

| Change ID | Source findings | PO feedback | Target artifact | Change kind | Status | Decision | Semantic impact |
|---|---|---|---|---|---|---|---|
| REC-CHANGE-001 | REC-001 | None canonical; OBS-001 | [docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md) R07 / Validation and completion | Qualification-boundary clarification | PROPOSED_AWAITING_DECISION | REC-DEC-001 | product, user_journeys, architecture |
| REC-CHANGE-002 | REC-002 | None canonical | None | Acceptance/progress clarification | NO_SOURCE_MUTATION | None | All false |

## REC-CHANGE-001

Previous rule: execute actual original startup and headless simulation, preserving gameplay with
GCC/Clang matching checkpoints and safe repeated startup; physical rendering/media acceptance is later.

Candidate rule, **non-authoritative and incomplete until REC-DEC-001 is resolved**:
“R07 qualification names a finite generated loaded-world scenario and a source-derived required
provider set. Its contract identifies real initialization and module update entry points, seeded
inputs, tick/checkpoint fields and equality rules, failure/retry and repeat-shutdown expectations,
and a separate read-only supplied-data check. Each omitted physical-output service has evidence
that the omission preserves simulation-facing behavior. This qualification does not accept later
rendering, interactive/media, full-match, persistence or LAN requirements.”

The exact cohort, checkpoints and cross-layer boundary cannot be supplied by a safe one-sentence
editorial correction. Multiple feasible realizations exist, and the evidence does not choose among
them. The narrow-gap mutation gate therefore fails. No authority is changed; candidate text cannot
unlock implementation. Architecture authority is not edited by reconciliation.

Target definition: [docs/zero-hour-linux-port-plan.md](../../../docs/zero-hour-linux-port-plan.md); requirements R03/R05/R07, neighboring R08/R09 boundary preserved.
Affected domains: product=true, user_journeys=true, architecture=true, ux=false, security=false.
Derived records requiring consideration after resolution: N2 milestone, governing and slice03 plans,
[docs/original-runtime-source-graph.md](../../../docs/original-runtime-source-graph.md), readiness report, and [workflow/delivery/state.yaml](../../../workflow/delivery/state.yaml). This is an impact record, not a lifecycle transition.
Application evidence: none; proposed only. No applied changes or eligible-but-unwritten corrections.

## REC-CHANGE-002

Previous and required rule are identical: component evidence remains scoped; full original simulation
and subsequent milestone behavior require their own acceptance. No target change, product decision
or dependent implementation remediation. Application evidence: no mutation, source inspection and
historical QA corroborate the existing progress classification.

Only this run's seven new artifacts are transaction-owned. Implementation, tests, milestone/plan
files, workflow state, definition packages, authoritative docs and existing dirty paths are untouched.
