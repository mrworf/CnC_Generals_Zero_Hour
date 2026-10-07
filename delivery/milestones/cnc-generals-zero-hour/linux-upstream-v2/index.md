# Linux upstream v2 delivery packet

Mode: CHANGE_PLAN from the user-approved docs/zero-hour-linux-port-plan.md,
source commit d0483ca911b029460fb46d878fe260e90ec8d071. Stable product ID:
cnc-generals-zero-hour. The old packet is explicitly replaced and archived.

| ID | Outcome | Dependencies |
| --- | --- | --- |
| [N0](N0-clean-baseline-and-specification.md) | Clean baseline and specification | None |
| [N1](N1-stock-renderer-suitability.md) | Stock renderer suitability | N0 |
| [N2](N2-original-engine-linux-foundation.md) | Original engine Linux foundation | N0 |
| [N3](N3-original-world-rendering.md) | Original world rendering | N1, N2 |
| [N4](N4-interactive-platform-and-media.md) | Interactive platform and media | N1, N2 |
| [N5](N5-playable-single-player-and-persistence.md) | Playable single-player and persistence | N3, N4 |
| [N6](N6-linux-lan-matches.md) | Linux LAN matches | N5 |
| [N7](N7-release-and-dependency-maintenance.md) | Release and dependency maintenance | N6 |

Default order N0→N1→N2→N3→N4→N5→N6→N7. Graph: N0→N1,N2;
N1+N2→N3,N4; N3+N4→N5→N6→N7. This permits independent work but does not
permit skipping acceptance. No agents. Source requirements are in traceability.md.

N1 is a go/no-go renderer qualification, not a promise that bgfx has already
passed. A real public-API gap triggers reassessment, not local dependency changes.
Readiness artifacts distinguish declared prerequisites from delivered outcomes.
