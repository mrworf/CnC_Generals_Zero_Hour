# M22 slice 08L2: atomic ThingFactory construction

`ThingFactory::newObject` is now one transaction across Object and Drawable
construction, behavior/draw/client modules, game-logic and client registries,
team/radar publication, source binding, create callbacks, partition registration
and final object initialization. Object and Drawable constructors initialize
their module arrays before provider calls and fail closed on missing providers.
Every reached owner is unwound in reverse order, including intrusive lists and
ID allocation, so a failed attempt is absent from source-visible state and a
clean retry produces one coherent object/drawable pair.

The generated construction witness injects failures at 16 publication and
resolution boundaries: object ID, team, behavior modules and resolution,
radar, logic registration, object return, create callback, partition,
drawable registry, draw modules, client modules and resolution, drawable
return, binding and initialization. Each failed attempt proves zero residual
transaction owners, zero graphics owners and zero Recording resources, then
runs a separate successful retry. The existing mission/skirmish, two-generation,
provider-removal and accepted pre-construction stop checks remain intact. No
retail input, private selector, source-tree mutation or bridge-specific bypass
was used.

Audit of the bound-drawable failure path found that normal client destruction
would run UI and drawable deletion behavior during rollback. The accepted path
uses the drawable's construction rollback and direct pool release, including
the injected post-creation failure before binding.

Acceptance on the final production tree:

- Focused GCC and Clang Release and sanitizer tests pass 5/5.
- All six canonical non-GPU/non-LAN/non-retail suites pass 261/261; sanitizer
  suites use `ASAN_OPTIONS=detect_leaks=0`.
- Strict host LeakSanitizer passes the three focused generated/source routes in
  GCC and Clang, 3/3 each.
- The unchanged public physical Vulkan display-owner and factory controls pass
  2/2 with the validation layer; serial host LAN passes 4/4 in all six
  configurations.
- All three dependency ledgers and `git diff --check` pass.

Bridge/terrain and walk-on-wall/pathfinder attachment remains the
dependency-ordered 08L3 owner.
