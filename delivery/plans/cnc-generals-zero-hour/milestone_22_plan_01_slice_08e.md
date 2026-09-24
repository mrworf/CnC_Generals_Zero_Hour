# M22 slice 08E: original load-screen progress lifecycle

## Goal

Close the first original consumer reached after 08E1: the optional source
progress dispatch after reset bookkeeping and before map-INI/map loading.
On this Linux build the exact source factory is deliberately compiled to the
no-owner branch before any platform-only owner selection; this slice proves
that legitimate absence and bounded source dispatch without fabricating a
Windows-only route.

## Scope and guards

- Retain the exact `GameLogic::getLoadScreen(Bool)` non-Windows branch:
  `(void)loadingSaveGame; return NULL;`. Do not compile, invoke, enumerate, or
  expose the platform-only owner cases, retail layouts/assets, rendering,
  map loading, or permissive fallback.
- A test-only source probe uses two independent generations and every public
  game mode plus both save flags. Each selection must return no owner, every
  bounded progress call must be a no-op, and deletion must leave the private
  owner slot null. No GUI, display, provider, network, device, or pixel
  ownership may be published.
- A separate source audit rejects a missing no-owner guard, any owner creation
  in the Linux branch, a non-null runtime result, a nonzero owner witness, or
  a post-progress advancement placed after the first map-INI call. It names
  no retail route, content, path, or hash.

## Dependencies and acceptance

08E depends on 08E1 and is a prerequisite of 08. It requires a runtime
generated no-owner witness, source guard/order audit, provider-removal
negative, two-generation zero-ownership proof, proportional matrix,
leak/Vulkan/LAN checks, ledger/evidence/index, and one independent commit.
Subsequent map-INI or map-load consumers remain separate dependency work.

## Commit boundary

One test-only, independently reversible slice: Linux source-factory/progress
probe, source audit and removal gate, CTest registration, ledger/governing-row
update, and generated-only evidence. The pre-existing renderer diagnostic is
explicitly excluded.
