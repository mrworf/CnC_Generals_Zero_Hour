# M22 plan 01 slice 03: reached original GameClient draw behavior

## Goal and observable outcome

The selected campaign/skirmish consumer set constructs and exercises all ten reached concrete original draw classes against slice 01's original render-object graph and slice 02's original CPU display/scene/shadow/track owners. Their source-defined overrides execute before any device boundary and retain distinct state/ordering semantics.

## Scope

- Remove the M21 headless substitutions only for reached operations in `W3DDefaultDraw`, `W3DModelDraw`, `W3DTankDraw`, `W3DTankTruckDraw`, `W3DTruckDraw`, `W3DSupplyDraw`, `W3DOverlordTankDraw`, `W3DDependencyModelDraw`, `W3DOverlordAircraftDraw`, and `W3DOverlordTruckDraw`.
- Preserve original condition/model selection, transforms/scaling, animations, recoil/turret/wheel/tread/bone work, supply-bone visibility, dependency gates, rider draw ordering/tint propagation, shroud/hidden state, color/tint, and shadow decisions.
- Add only newly reached CPU dependencies required by these overrides. Device calls remain fail-closed until slice 04.

## Validation and error handling

- An owned original-engine scenario creates representative instances through canonical `W3DModuleFactory`, drives state transitions, and records source-owned decisions immediately before the unavailable device edge.
- Every reached concrete override has a distinct assertion or a source-proven inherited-equivalence assertion; a base-only call cannot stand in for omitted behavior.
- Missing bones/riders/dependencies/models and invalid condition state fail according to original required/optional semantics without fabricated values.
- Provider-removal covers representative base, vehicle, dependency, and rider providers.

## Acceptance criteria

- Canonical 19-entry M20 registry and ten M21 concrete identities are unchanged.
- All reached subclass-specific CPU behavior is restored and tested before device translation.
- No generic draw module, alternate registry, no-op override, or adapter-owned scene state is accepted.

## Commit boundary

Commit reached concrete GameClient behavior, focused tests, ledger update, slice status, and evidence as `delivery: M22 slice 03 restore original draw behavior`.
