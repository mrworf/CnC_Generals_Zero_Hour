# Requirement coverage

Authority: docs/zero-hour-linux-port-plan.md, user-approved replacement plan.

| Requirement | Class | Milestones |
| --- | --- | --- |
| R01 | Implementation required | N0 |
| R02 | Implementation required; shared governing constraint | N1, N7 |
| R03 | Implementation required; shared governing constraint | N2, N7 |
| R04 | Implementation required; shared governing constraint | N2, N7 |
| R05 | Implementation required; shared governing constraint | N3, N4, N5 |
| R06 | Implementation required | N1 |
| R07 | Implementation required | N2 |
| R08 | Implementation required | N3 |
| R09 | Implementation required | N4 |
| R10 | Implementation required | N5 |
| R11 | Implementation required | N6 |
| R12 | Implementation required | N7 |

R02–R05 constrain every applicable milestone as well as the named providers.
N0 is the explicitly approved recovery outcome, not a synthesized setup bucket.
N1 owns API suitability decisions under the specified gate; downstream integration
is conditional on success, not unresolved product behavior hidden in planning.

Excluded: dependency forks, private APIs, asset distribution/modification, ARM64,
legacy MSVC/internal Windows ABI, original Generals executable, content tools,
DRM and online-service replacement. Deferred: optional Windows persistence and
mixed-OS LAN. Context only: old tests, old milestones and upstream examples.
Unmapped required items: none. No historical acceptance is inherited.
