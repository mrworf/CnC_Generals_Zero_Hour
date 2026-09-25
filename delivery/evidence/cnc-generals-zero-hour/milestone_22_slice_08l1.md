# M22 slice 08L1: general modeled-drawable construction

The full original W3D draw provider now compiles a build-tree overlay of the
linked `W3DModelDraw` implementation, leaving the imported source tree
unchanged. Time/weather validation is transactional: a nonempty model state or
transition publishes its validation bit only after pristine-bone acquisition
succeeds. Missing managers and missing models fail closed and remain retryable;
temporary validation render objects use a local owner whose destructor releases
the reference on every exit. Intentionally empty states remain valid without
inventing a model.

The generated full-draw source witness exercises morning/normal, night/normal,
night/snow and morning/snow through the accepted game-logic latch. Existing
generated fixtures distinguish the ordinary and alternate HLOD models, inject
the first model acquisition failure, prove deterministic re-entry, enforce
full-provider identity and reject removed/schema-only providers. No retail
input, private selector, substitute geometry or original-source mutation was
used.

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

Atomic factory-wide Object/Drawable/module/registry/partition rollback remains
the dependency-ordered 08L2 owner. Bridge/world attachment remains 08L3.
