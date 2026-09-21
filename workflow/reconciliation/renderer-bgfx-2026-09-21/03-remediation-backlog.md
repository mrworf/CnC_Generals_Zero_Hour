# Product Reconciliation Remediation Backlog

> Mode: CHANGE_PLAN
> Implementation authority: `docs/zero-hour-linux-port-plan.md` §9; accepted `docs/zero-hour-renderer-backend-migration.md` RB-01–RB-04 at `64c7632`
> Preservation baseline: completed milestone histories and backend-neutral code at `64c7632`; no retail content copied
> Canonical result: [reconciliation-result.yaml](reconciliation-result.yaml)
> Delivery planning ready: true

| Priority | Remediation | Desired outcome | Findings | Requirements | Dependencies | Required validation |
|---|---|---|---|---|---|---|
| BLOCKING | REC-RM-001 | Public bgfx backend preserves original ordered camera clear and prior renderer behavior | REC-001 | Port plan §9; RB-01–RB-04 | Gate before M22 06C3C; M24 remains after M22 | Probe, M2/M7–M10/M14 rerun, retail scene acceptance |

### REC-RM-001: Migrate and revalidate renderer device edge

Priority: BLOCKING
Source findings: REC-001
Product Owner feedback: None
Authoritative basis: accepted `docs/zero-hour-renderer-backend-migration.md` RB-01–RB-04 and port plan §9.
Desired outcome: original source-scoped camera clear and existing renderer categories work on the public bgfx Vulkan backend.
Current behavior: SDL_GPU implementation cannot do active-pass viewport color/depth/stencil clear; synthetic bgfx probe passes, full migration absent.
Required behavior: backend-neutral command/recorder and bgfx device preserve target generations, draw/clear order, exact viewport clipping and selected attachment values, then present source scenes.
Affected users/roles: offline player viewing menu, mission and skirmish frames; no new role or UI behavior.
Affected interfaces and compatibility: `GpuDevice`, recorder snapshots, shader build, public bgfx device edge, SDL3 platform/input; preserve existing save/network/gameplay and engine-owned handles.
UX constraints: original visual appearance, ordering and alpha/depth/stencil behavior.
Architecture/security constraints: no private Vulkan call under SDL_GPU, no raw Vulkan ownership, read-only retail source, backend-neutral public headers.

Acceptance criteria:
1. Exact 160×120 probe passes on the RTX Vulkan host; original camera clears and interleaved W3D draws retain outer attachment content.
2. Illegal clear timing, invalid dimensions/formats/targets, generation misuse and view exhaustion fail explicitly before submission; device loss/resize paths recover or fail cleanly.
3. Previously accepted backend-neutral behavior and SDL3 platform/input remain intact; no wholesale restart of completed CPU/source providers.
4. M2 inventory and M7–M10 affected contracts/shaders/producers plus M14 hardware/visual/validation are rerun on bgfx; M22 retail scene acceptance follows before M24 dependent first-tick checks.

Dependencies: migration/revalidation gate precedes remaining M22 work; M24 remains after completed M22 provider.
Explicit exclusions: ARM64, base Generals, retail redistribution, gameplay rewrite, private Vulkan escape, synthetic fixture as retail-scene proof.
Required implementation validation: positive/negative unit and recorder tests, all required GCC/Clang build suites, real RTX Vulkan validation, original scene pixels and lifecycle, source dependency ledger/evidence grades.
