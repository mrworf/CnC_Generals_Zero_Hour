# M22 plan 01 slice 08L1: general modeled-drawable construction

## Goal and observable outcome

Extend the accepted original W3D model owner from bounded generated model
fixtures to normal source `ThingTemplate` draw-module construction. A generated
authored modeled drawable must run original time/weather, animation, pristine-
bone, state-selection and render-object acquisition behavior without a private
selector or substitute geometry.

## Scope, ownership and rollback

Use only project-generated W3D, animation and template inputs. Preserve original
condition selection, asset-manager identity, scale, public-bone caching,
animation ownership and `setModelState` order. Missing, malformed, wrong-class,
unregistered-provider and mid-validation failures fail closed, release every
temporary animation/render owner and permit deterministic retry. Provider-
removal coverage proves that schema-only or null providers cannot satisfy the
route.

`Drawable` asks `ModuleFactory` for the source provider. `W3DModelDraw` validates
matching conditions, creates and releases validation render objects, selects the
empty-condition state and acquires its live render object. Constructor failure
unwinds only owners acquired by that module. Drawable/object registry rollback
is 08L2; bridge/world attachment is 08L3. Do not special-case bridges, weaken
required-model semantics, synthesize model state or change the accepted no-model
prop route.

## Implementation and validation

Extend generated CPU/Recording coverage for ordinary and alternate time/weather
state, animation/public bones, missing/malformed assets, injected acquisition
failures, provider removal, retry, two device generations and teardown. Run
focused GCC/Clang and sanitizer tests, the canonical six nonretail suites, strict
host leak gates, physical Vulkan, serial LAN, ledgers and diff checks.

## Acceptance and commit boundary

One commit: `delivery: M22 08L1 construct general modeled drawables`.
