# M21 original map and simulation acceptance

## Accepted outcome

The production Linux original-engine entry now dispatches an initial `.map`
through the original chunk readers, starts original mission and skirmish state,
advances commands, objects, AI, scripts, players, and victory conditions, resets,
and enters a second scenario without stale state. The acceptance chain is
original-source integration; no modern simulation proxy or label is used.

The owned mission-to-skirmish re-entry witness requires the first scenario's
objects, map objects, props, preload records, and recorder controls all to reach
zero before the second map is entered. The second scenario then establishes
three fresh objects, one fresh prop, and one recorder-control initialization.
Independent client and audio random burns produce the same source-owned
simulation and lifecycle checkpoints.

## Reached original providers and device boundary

Retail campaign and skirmish setup reached ten original concrete draw classes:
`W3DDefaultDraw`, `W3DModelDraw`, `W3DTankDraw`, `W3DTankTruckDraw`,
`W3DTruckDraw`, `W3DSupplyDraw`, `W3DOverlordTankDraw`,
`W3DDependencyModelDraw`, `W3DOverlordAircraftDraw`, and
`W3DOverlordTruckDraw`. Their original concrete type and parsed `ModuleData`
identity are retained. Their reached CPU constructors, state, preload paths, and
destructors execute; model, bone, shadow, and render-device operations remain
explicit unavailable-device edges. Removing the original `W3DModelDraw`
provider makes the production link fail.

The Linux display records original preload requests, `TerrainVisual` owns prop
state, and recorder controls are initialized. Both retail scenario types report
zero physical devices. This is not renderer hardware/session acceptance.

## Lifecycle and correctness corrections

- Original client and player teardown re-read intrusive-list heads after each
  dependency-owning deletion, so dependent drawables and objects cannot leave a
  stale next link.
- Player teardown destroys reset-surviving GameLogic objects before players;
  victory and radar callbacks no longer retain or dereference destroyed globals.
  The original reset ordering itself is unchanged.
- The original partition coverage lookup now searches the target cell's object
  links rather than every link owned by the object. This preserves the selected
  link and ordering semantics while removing quadratic retail-map startup.
- Original right-HUD preload checks its matching image before dereference.
- `StateReturnType` retains its original 32-bit values as an integer protocol;
  this matches the original design in which every positive value is a sleep
  frame count and avoids manufacturing out-of-range C++ enum values.
- The W3D schema's physical negative control uses an unreached device-only
  provider and still requires the exact `ERROR_INVALID_D3D` `ErrorCode`; reached
  concrete CPU providers are not treated as unavailable.

## Negative, identity, and read-only controls

The cumulative suite rejects a missing map, missing chunk/provider, malformed
chunk, invalid command, absent actor/owner/AI state, partial publication, stale
reset state, and removed map/setup/behavior/draw provider. Identity gates bind
map parsing, setup, simulation, and all ten reached draw instances to their
original translation units and concrete classes. The checked dependency ledger
passes its freshness audit.

Owned inputs run read-only from arbitrary working directories with isolated XDG
roots. The separately gated retail campaign and skirmish cases both reach
scenario-ready and clean teardown. The gate compares recursive file metadata
before and after; it records no private path, byte content, or hash and detected
no corpus write. Logical retail results were 1,151 objects/122 props for campaign
and 249 objects/no map props for skirmish; both initialized recorder controls and
created zero physical devices.

## Validation

- Four native configure/build presets passed: GCC Debug, Clang Debug, GCC
  Release, and Clang Release.
- Complete asset-free CTest coverage passed in every preset: 126 sandbox-safe
  tests plus the two local-UDP tests run outside the socket sandbox, 128/128 per
  preset. The release UDP tests were run sequentially because the test harness
  intentionally uses one fixed loopback port.
- Focused original-simulation suite passed 11/11 in GCC Debug, 11/11 under
  GCC Debug ASan+UBSan, and 11/11 under the required Clang Debug ASan+UBSan.
- Strict sanitized runtime cases passed 4/4 under both compilers with
  `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1` and
  `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`.
- UBSan's `vptr` instrumentation is excluded because the intentionally partial
  Linux runtime does not link RTTI key functions for retired platform/service
  providers. ASan and every other `undefined` sanitizer check remain enabled.
- Gated read-only retail campaign/skirmish acceptance passed 1/1 in 41.06s.
- Dependency-ledger freshness and `git diff --check` passed.

This evidence makes no renderer hardware/session, interactive UI/media, full
save/replay, or LAN-product acceptance claim.
