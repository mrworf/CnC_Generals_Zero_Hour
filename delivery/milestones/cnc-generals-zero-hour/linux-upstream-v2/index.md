# Linux upstream v2 delivery packet

Mode: PRODUCT_BUILD from `workflow/definition/definition-package.yaml`, definition
transaction `e0a32f1c92b113733bbe588cfc00aeb7b019e4d7`. Exact revisions: `status.yaml::compiled_from`.
Product: `cnc-generals-zero-hour`. This adopts the bounded R07/HQ amendment into the
existing packet; inherited R01–R12 and the user-approved graph remain unchanged.
The package's six passes are amendment-only; later game scope is not recertified here.

| ID | Outcome | Dependencies | Preserved status |
| --- | --- | --- | --- |
| [N0](N0-clean-baseline-and-specification.md) | Clean baseline and specification | None | completed |
| [N1](N1-stock-renderer-suitability.md) | Stock renderer suitability | N0 | completed |
| [N2](N2-original-engine-linux-foundation.md) | Original engine Linux foundation | N0 | in_progress |
| [N3](N3-original-world-rendering.md) | Original world rendering | N1, N2 | pending |
| [N4](N4-interactive-platform-and-media.md) | Interactive platform and media | N1, N2 | pending |
| [N5](N5-playable-single-player-and-persistence.md) | Playable single-player and persistence | N3, N4 | pending |
| [N6](N6-linux-lan-matches.md) | Linux LAN matches | N5 | pending |
| [N7](N7-release-and-dependency-maintenance.md) | Release and dependency maintenance | N6 | pending |

Default order N0→N1→N2→N3→N4→N5→N6→N7. Graph: N0→N1,N2;
N1+N2→N3,N4; N3+N4→N5→N6→N7. No reordering or added M0. No agents.
N0/N1 evidence is preserved historical completion, not freshly executed acceptance.
N2 remains incomplete; its accepted core/data slices and useful component work survive.

## Active boundary and handoff

N2 now has finite HQ-01–07 acceptance: actual original startup, H1 load/120 updates,
121×3 checkpoint records per configuration, negative/retry ownership, and separate
read-only original retail initialization. See its contract and traceability map.
N3/N4/N5 retain full rendering, interaction/media and gameplay/persistence obligations;
drawless H1 is not whole-game parity or authority to remove already-correct code.

Before further N2 code, `milestone-delivery-orchestrator` must adopt this packet and
invoke fresh minimal-slice planning for remaining N2. Preserve accepted slices and
uncommitted work; explicitly supersede old slice03 as the remaining-work directive,
reconcile its open work against HQ-01–07, and replace conflicting resume pointers
atomically in delivery-owned state. Do not blindly continue Full475/Full509 or mark
slice03 complete from documentation. No slices or delivery-state mutations occur here.

No unresolved definition item or unmapped requirement. Readiness owns prerequisite
classification; retail data and later physical/release facilities retain explicit
verification boundaries. See `workflow/delivery-planning/handoff.yaml` for the
committed readiness result and exact next workflow.
