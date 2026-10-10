# Definition decisions

## D-BASELINE-001 — Existing approved intent

Status: accepted; provenance: `docs/zero-hour-linux-port-plan.md`, user-approved status,
requirements R01–R12, exclusions/delegated decisions, and `AGENTS.md`.
Preserve native x86-64 Linux original gameplay/visual/audio/file semantics, upstream-clean
libraries and immutable retail assets. Original UI direction is preservation, not redesign.
Internal equivalent mechanisms and qualification-fixture choices are delegated engineering work.
No milestone order change, recovery reset or legacy acceptance import is authorized by this run.
This records existing authority, not a new user decision.

## REC-DEC-001 — Headless qualification

```yaml
id: REC-DEC-001
question_id: Q-R07-001
status: accepted
source_findings: [REC-001]
resolution_category: SPECIALIST_RESOLUTION
resolution_owner: [architecture, prd]
routing_status: resolved
depends_on: []
affected_requirements: [R03, R05, R07, R08, R09]
affected_domains: [product, user_journeys, architecture]
result_path: workflow/definition/results/headless-resolution.yaml
```

Canonical request: What finite original-engine startup/load/update/checkpoint/shutdown scenario and source-derived headless provider contract sufficiently qualify R07 while preserving R05 and leaving physical rendering/media acceptance in R08/R09?

Reason: The approved intent fixes real source execution and fidelity, but neither the specification nor current plans settles the workload, observable checkpoint contract or required-versus-deferred provider closure; new prerequisites keep accumulating in slice03.

Accepted delegated resolution: HQ-01–HQ-07 in `product/headless-qualification.md`,
linked normatively from the approved specification. H1 is a finite source-driven workload,
not a replacement simulation or reduced final scope. Technical choice authority:
D-BASELINE-001; evidence/result: `results/headless-resolution.yaml` and `results/prd.yaml`.
Canonical reconciliation REC-RUN-002 accepts this resolution and closes REC-001/REC-DEC-001.
