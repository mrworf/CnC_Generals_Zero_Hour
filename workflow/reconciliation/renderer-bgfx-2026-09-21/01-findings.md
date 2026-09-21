| Finding | Title | Classification | Severity | Reproduction | Confidence | Decision | Implementation | Definition revalidation |
|---|---|---|---|---|---|---|---|---|
| REC-001 | Original camera clear exceeds SDL_GPU public pass API | IMPLEMENTATION_DEFECT | BLOCKING | REPRODUCED | High | None | REC-RM-001 | No |

### REC-001: Original camera clear exceeds SDL_GPU public pass API

Classification: IMPLEMENTATION_DEFECT
Severity: BLOCKING
Confidence: High
Reproduction: REPRODUCED

Evidence:
- REPOSITORY_EVIDENCED: `delivery/evidence/cnc-generals-zero-hour/milestone_22_backend_gap_06c3c.md` identifies original `CameraClass::Apply` → `DX8Wrapper::Clear` inside one WW3D frame, with exact 160×120 camera inset and the SDL_GPU 3.4.14 public boundary.
- OBSERVED: `tools/renderer/bgfx_viewport_clear_probe.cpp` passed exit 0 on RTX 4070 / NVIDIA 610.57.04 / Vulkan 1.4.341 with bgfx `81d81fba72c42d348c589514c774bbfe01e110fa`; outer and inset color plus independent depth and stencil tests passed.
- DOCUMENTED: `docs/zero-hour-linux-port-plan.md` §9 selects a backend switch after this demonstrated gap; `docs/zero-hour-renderer-backend-migration.md` records the accepted bgfx decision and migration boundary at commit `64c7632`.

Product Owner feedback: None.

Current observed behavior: the SDL_GPU device cannot express the original scene's active-frame viewport clear; M22 is blocked at 06C3C and M24's retail-first-tick path is downstream. The bgfx capability test passes but the game has not been migrated.

Relevant implementation: `include/zh/renderer/contract.h`, `src/renderer/sdl_gpu_device.cpp`, `include/zh/renderer/recording_device.h`; exact missing operation, not a driver fault.

Relevant UX rule: Not applicable; existing original UI and presentation intent is preserved.

Relevant user journey: Not applicable; this is a renderer implementation boundary within existing offline gameplay.

Relevant product requirement: `docs/zero-hour-linux-port-plan.md` §§1, 6, 8, 9 preserve original Zero Hour behavior and require a public backend escalation for a proven abstraction gap.

Architecture/security constraint: accepted `docs/zero-hour-renderer-backend-migration.md` selects bgfx for only the device edge; SDL3 platform/input and no private Vulkan escape remain. Security is unaffected; retail files stay read-only.

Root source of inconsistency: the provisional SDL_GPU device/contract cannot realize a source-required clear. The architecture decision itself is now settled, so no unresolved architecture conflict remains.

Authoritative desired state: one ordered frame can clear a camera subrect's selected color/depth/stencil attachments and preserve outer attachments, then draw the scene and later scenes before presentation, through the selected public bgfx backend.

Authoritative artifact requiring change: None within this reconciliation; the decision was separately committed at `64c7632`.

Changed automatically: No
Decision required: No
Implementation remediation required: REC-RM-001.

Affected definition domains:
- product: false
- user_journeys: false
- ux: false
- architecture: false
- security: false

Potentially stale artifacts: None in Product Definition. SDL_GPU implementation mappings, shader packages and historical hardware tests require implementation revalidation under REC-RM-001.

Implementation validation: exact probe; positive and negative recorder/clear tests; M2/M7–M10/M14 affected coverage; original W3D/WWShade/retail scene acceptance; M24 dependent retail first-tick only after M22.
