| Change ID | Source findings | PO feedback | Target artifact | Change kind | Status | Decision | Semantic impact |
|---|---|---|---|---|---|---|---|
| REC-CHANGE-001 | REC-001 | None | None in this transaction | Prior architecture decision traced | NO_SOURCE_MUTATION | None | No remaining definition revalidation |

### REC-CHANGE-001

The previous provisional SDL_GPU rule in `docs/zero-hour-linux-port-plan.md` §9 became its already-authorized second branch after executable evidence. The separately committed `64c7632` decision changed the current device-edge rule to bgfx and preserved SDL3 platform/input. This reconciliation does not edit authority or claim automatic product/architecture mutation. The accepted rule is testable via `tools/renderer/bgfx_viewport_clear_probe.cpp` and the migration acceptance in `docs/zero-hour-renderer-backend-migration.md`. Product, journey, UX, architecture and security definition boundaries require no further semantic decision; implementation mappings and tests must be rerun.
