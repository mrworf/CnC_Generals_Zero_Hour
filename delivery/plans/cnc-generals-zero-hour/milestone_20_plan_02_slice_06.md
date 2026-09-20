# M20 plan 02 slice 06: identity, retail gate, and milestone assurance

## Goal and observable outcome

Close M20 with durable proof that the production executable uses the actual original entry/lifecycle/providers and satisfies fixture plus separately gated read-only retail initialization without hardware acquisition or private-data disclosure.

## Scope and non-scope

Strengthen compile-command, contributing-object/live-symbol, conditional-branch, runtime-state, registry-freshness, ledger, classification, provider-removal, and no-reduced-class controls. Run cumulative four-preset, full-suite, sanitizer, arbitrary-CWD/XDG, lifecycle failure, and retail gates. Produce M20 QA evidence. Do not claim M21-M25 scenario/device/interaction/persistence/network acceptance.

## Dependencies and ordering

Requires slices 01-05 and is the final M20 slice.

## Entry point, state, and permissions

Tests execute the installed-form Linux entry from an arbitrary CWD. Owned fixtures always run. The retail gate receives explicitly configured read-only roots, records only logical resource names/counts and source-owned outcomes, and never records host paths, bytes, or hashes. No write permission is granted to retail roots.

## Validation and error handling

Positive and negative fixture controls are mandatory and never skip for missing retail. Retail absence fails only the explicit retail gate. Source/registry drift, ownerless dependencies, discarded providers, alternate classes, trace-only witnesses, NDEBUG-disabled checks, unexpected device acquisition, or retail/CWD writes fail closed.

## Expected surfaces

Assurance tools/tests, dependency ledger and classification finalization, CMake/CTest gates, evidence under `evidence/qa/cnc-generals-zero-hour/`, and governing plan completion index.

## Required validation

- Configure/build/run focused M20 targets under all four presets.
- Run canonical full asset-free CTest under all four presets after stabilization.
- Run focused Clang Debug ASan/UBSan with exact `ASAN_OPTIONS`/`UBSAN_OPTIONS`; separately report allocator/resource/worker live counts.
- Run identity, provider-removal, classification, ledger freshness/owner, arbitrary-CWD, XDG isolation, no-device, and all-stage failure controls.
- Run the explicitly provisioned read-only retail initialization gate without committing private details.

## Acceptance criteria

Every M20 milestone checkbox maps to command and runtime evidence. Original `GameMain -> GameEngine::init/postProcessLoadAll/execute/update/reset/destruction` runs with all RC-002 providers and genuine state changes; complete configuration/registries and nonempty cache behavior execute; failures preserve nonzero semantics and exact teardown; process services enclose engine/workers; all validation passes with no later milestone used as a code prerequisite.

## Commit boundary

Commit assurance, final ledger/classification updates, completed plan index, and QA evidence as `delivery: M20 slice 06 accept original runtime lifecycle`.

## Blocked checkpoint

The read-only retail gate advanced through the new Linux BIG adapter into original object-template parsing, where `W3DDefaultDraw` requires the authoritative W3D module registry. That 19-provider registry remains assigned to M22-M23 and its original translation units reach the excluded Direct3D 8 SDK at compile time. The accepted packet neither authorizes a separated original data-provider boundary nor permits M22-M23 as prerequisites. See [M20 plan 02 retail W3D registry blocker](../../../evidence/qa/cnc-generals-zero-hour/M20-plan02-retail-w3d-blocker.md). This slice must not be accepted by substituting a generic parser, placeholder registry, or no-op draw success path.
