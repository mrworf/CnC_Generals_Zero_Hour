# M22 plan 01 slice 02: complete original retail scenes on recording

## Goal and observable outcome

The production original-engine campaign and skirmish paths load representative user-owned retail maps/resources and record complete real scene families through the slice 01 adapter, with original camera, terrain, objects, lighting, fog/shroud, shadows, particles, water, and effects visibly represented in the deterministic command stream.

## Scope

- Extend the existing M21 read-only campaign/skirmish gate into original presentation traversal using the same original scene consumers later used on hardware.
- Port every newly reached original CPU/resource operation immediately and update the checked dependency ledger.
- Validate retail W3D/HLOD/animation/texture and terrain loading, original material/WWShade state, original object transform/animation selection, producer ordering, reset/re-entry, and teardown.
- Add project-owned failure fixtures and retail-private-safe missing/malformed/unsupported controls.
- Add source identity and provider-removal gates for terrain, camera/view/display, representative draw classes, WWShade/state translation, and retail asset loaders.

## Explicit non-scope

- Vulkan/device acceptance, screenshot judgment, UI/media interaction, or gameplay completion.
- Accepting aggregate object/preload counts without recorded original draw output.

## Dependencies and ordering

- Requires slice 01's shared producer/adapter semantics.
- Requires PRE-008 user-owned retail roots only for the separate gated tests. Default builds and tests remain retail independent.

## Entry point and end-to-end behavior

The installed production executable starts the same original campaign and skirmish scenarios used by M21, completes original setup, advances a bounded presentation frame, traverses the original GameClient/view/terrain/draw modules, records all required scene families, resets, optionally re-enters, and tears down. Recording output exposes only permitted logical family/aggregate data.

## Data and state transitions

- Scenario-ready original map/object state -> resource resolution/load -> camera and scene producer update -> complete recorded frame -> reset -> zero scene/resource state -> optional fresh second scenario -> reverse teardown.
- A missing/malformed required asset or unsupported material/state aborts before a complete-frame witness and releases partially created resources.

## Authorization and permission behavior

Retail roots and symlinks are read-only. Tests compare corpus metadata before/after, isolate all writable XDG locations, and redact private paths/content/hashes. No network or external service is used.

## Validation and error handling

- Positive: one shipped campaign and one shipped skirmish scene each cover the required original scene-family matrix and nonzero original draw/resource/state outputs.
- Negative: missing asset, malformed owned analog, unsupported material/state, omitted original provider, incomplete family, and failed reset/re-entry are rejected.
- Boundary: legitimately absent optional content in a selected scene is distinguished from a missing required family through explicit selection/coverage rules; the gate may select another real scene but never generate a substitute.
- Lifecycle: repeated campaign/skirmish and mission-reset-skirmish runs leave zero adapter resources, original draw instances, preloads, scenes, and workers.

## Expected implementation surfaces

- `CMakeLists.txt`
- original GameClient/W3D/WWShade/terrain/asset sources and Linux binding under `src/original_runtime/`
- `tests/original_rendering/` retail and owned integration gates
- `tools/` identity/provider-removal/privacy checks where needed
- `docs/original-runtime-dependency-ledger.tsv`
- `evidence/qa/cnc-generals-zero-hour/`

## Required validation commands

- Build/run focused owned original-rendering recording tests under GCC Debug.
- Run separately enabled read-only retail campaign/skirmish recording tests from arbitrary CWD with isolated XDG roots and corpus metadata comparison.
- Run focused GCC and Clang ASan+UBSan on the owned equivalents with strict settings.
- Run identity, provider-removal, ledger freshness, original simulation regression, and `git diff --check`.

## Acceptance criteria

- Both retail scenario types execute actual original GameClient/W3D/WWShade producers and record every required scene family; aggregate-only/preload-only evidence is rejected.
- All newly reached operations are implemented or an explicit required-state failure stops acceptance; no reached RC-012 fail-closed placeholder remains.
- Missing/malformed/unsupported/provider-removal controls fail the production path.
- Retail inputs are unchanged and no private paths, content, hashes, or selected physical filenames enter committed evidence.
- Reset/re-entry and partial-failure teardown return all original and adapter ownership to zero.

## Commit boundary

Commit retail recording integration, negative/identity/lifecycle gates, ledger changes, slice status, and evidence together as `delivery: M22 slice 02 render original retail scenes`.
