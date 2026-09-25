# M22 plan 01 slice 08L2: atomic ThingFactory object/drawable construction

## Goal and observable outcome

Make `ThingFactory::newObject` atomic across Object, Drawable, behavior/draw
modules, radar, game-logic/client registries, partition, binding, create callbacks
and `initObject`. Success preserves source order and IDs. Any failure leaves no
visible object, drawable, module, partition/radar/team entry or intrusive link;
retry produces exactly one coherent pair.

## Scope, ownership and rollback

Cover failures in object-module construction/resolution, 08L1 modeled draw
construction, drawable inter-module resolution, behavior `onCreate`, partition
registration, binding and initialization. Preserve construction-time drawable
ID availability and final bidirectional binding using explicit pending/owned
state or equivalent registry-aware rollback. Reject missing/null module providers
before dereference and prove provider removal.

The transaction begins at `ThingFactory::newObject`. Object construction publishes
radar/game-logic ownership before `sendObjectCreated`; Drawable construction
publishes its client ID/list before later draw modules. Track each publication,
unwind in reverse order, and invoke only hooks valid for the reached stage. Do
not special-case a retail template or bridge, suppress modules, bypass callbacks
or turn a required modeled object into a no-model object.

## Implementation and validation

Compose a generated template from accepted behavior modules and 08L1's modeled
drawable. Inject failure at every publication boundary and assert registry/list
counts, IDs, bindings, partition/radar/team/module owners, Recording resources
and retry. Include provider removal, no-model regression, two generations and
complete M22 validation.

## Acceptance and commit boundary

Depends on 08L1. One commit:
`delivery: M22 08L2 make ThingFactory construction atomic`.
