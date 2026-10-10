# PRD readiness — R07 qualification amendment

Source: `docs/zero-hour-linux-port-plan.md` with `product/headless-qualification.md`.
Mode: existing approved product; narrowly clarify REC-001/REC-DEC-001.
Implementation-ready: yes — definition of the R07 qualification amendment only.
No claim of whole-game implementation acceptance or a full new requirements baseline.

## Decision model

Purpose: make original-source headless foundation acceptance finite and executable.
Actor: local developer/runner; retail data owner supplies read-only input. No accounts,
new product roles, network trust or user-facing game design is added. The native executable,
loaded world, actor/module instances, script state, checkpoints and root lifetime have
explicit identity and lifecycle in HQ-01–03. Writes remain in user storage; private
retail data and identifiers never enter tracked evidence. H1 generated data is disposable.

Preserved: R01–R12, source simulation semantics, upstream dependencies, immutable assets,
source encodings, original UI direction and all later game acceptance. Changed: definition
of how the foundation is demonstrated. Deprecated: no product behavior. Migration: none;
existing component evidence retains its exact source scope. Delivery-plan updates are separate.

## Readiness gates

| Gate | Result and rationale |
|---|---|
| Purpose | PASS: R07's original-engine demonstration is fixed in HQ-01/02 |
| Existing baseline | PASS: approved source/packet and preserved/deferred outcomes explicit |
| Actors/authorization | PASS: ordinary local runner; OS access only; no new roles/tenants |
| Domain/lifecycle | PASS: setup/load/120 updates/121 checkpoints/retirement/three lifetimes |
| Security/privacy | PASS as product invariants: immutable inputs, separated writes, redacted evidence |
| Primary/negative workflows | PASS: HQ-01–06 define success, missing/malformed/late failure, retry and cancellation |
| Data/interfaces | PASS: fixed evidence fields/ordering; CLI spelling is documented implementation detail |
| Integrations/compatibility | PASS: original paths and source formats; no Windows save/network obligation added |
| Operations | PASS: GCC/GCCsan/Clangsan frozen evidence, supplied-data prerequisite, no privileged setup |
| UX | PASS for this amendment: headless stage/category and actionable failure; game UI preservation unchanged |
| Acceptance/traceability | PASS: HQ IDs, source mapping, semantic witnesses plus compiler equality |
| Decision closure | PASS: fixture sizes/seed/module choices are delegated test mechanics; no new product choice |

## Specialist-resolution adoption

The architecture resolution selects H1's fixed cohort and provider boundary under existing
internal engineering delegation. This is not a claim that each current provider already
works. A missing concrete entry is implementation work, not an open product decision.
The PRD adds only a normative reference and preservation boundary; it does not rewrite
architecture or select new game behavior. The technical contract owns the exact fixture.

Local correction verification: the appended section references existing HQ-01–07, changes
no R01–R12 table row or delivery dependency, and preserves original text byte-for-byte as a
prefix. No open material product assumption remains within the amendment.

Remaining architecture choices: executable/CLI names, concrete native factory organization,
evidence codec/accessors and generated fixture bytes; all must satisfy the fixed observable
contract. UX, coverage, journey, architecture and security definition review follow within
this same bounded scope. Release, GPU, gameplay and runtime test evidence remain unexecuted.
