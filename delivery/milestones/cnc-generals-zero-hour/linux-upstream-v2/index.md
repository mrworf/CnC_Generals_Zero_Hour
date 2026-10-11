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
| [N2](N2-original-engine-linux-foundation.md) | Original engine Linux foundation | N0 | completed |
| [N3](N3-original-world-rendering.md) | Original world rendering | N1, N2 | blocked: PRE-03 host Vulkan |
| [N4](N4-interactive-platform-and-media.md) | Interactive platform and media | N1, N2 | blocked: PRE-03 host Vulkan |
| [N5](N5-playable-single-player-and-persistence.md) | Playable single-player and persistence | N3, N4 | pending |
| [N6](N6-linux-lan-matches.md) | Linux LAN matches | N5 | pending |
| [N7](N7-release-and-dependency-maintenance.md) | Release and dependency maintenance | N6 | pending |

Default order N0→N1→N2→N3→N4→N5→N6→N7. Graph: N0→N1,N2;
N1+N2→N3,N4; N3+N4→N5→N6→N7. No reordering or added M0. No agents.
N0/N1 evidence is preserved historical completion, not freshly executed acceptance.
N2 completed its six-slice bounded original-runtime qualification; current
acceptance and limits are recorded in
[`N2-native-supplied-startup.md`](../../../../evidence/qa/N2-native-supplied-startup.md).

## Active boundary and handoff

N2 accepted HQ-01–07: actual original startup, H1 load/120 updates,
121×3 checkpoint records per configuration, negative/retry ownership, separate
read-only supplied-data initialization, and complete normal/GCC/Clang sanitizer
regression coverage. See its contract and traceability map.
N3/N4/N5 retain full rendering, interaction/media and gameplay/persistence obligations;
drawless H1 is not whole-game parity or authority to remove already-correct code.

N3/N4 are dependency-ready but currently blocked at their shared PRE-03
physical Vulkan/session prerequisite. The [current host readiness
record](../../../../evidence/qa/N3-N4-host-readiness.md) names the driver
mismatch and exact resumption check. Neither milestone has begun implementation
or accepted physical output. The historical old N2 plan01 slice03 remains
unaccepted history; plan02 is the completed bounded N2 transaction.

No unresolved definition item or unmapped requirement. Readiness owns prerequisite
classification; retail data and later physical/release facilities retain explicit
verification boundaries. See `workflow/delivery-planning/handoff.yaml` for the
committed readiness result and exact next workflow.
