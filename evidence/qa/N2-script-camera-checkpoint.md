# N2 shared script-camera checkpoint

Existing N2 plan01/slice03 remains incomplete. This is shared game-owned camera
choreography and ownership evidence, not native UI/root startup, actual simulation
frame advancement, audio, physical rendering or whole-game acceptance.
Parent: `3b2f6d78e81efe909db52b72d00b0a9c84d82564`.

## Frozen source

- Canonical630: `9fc886d515135df304cda898654a2746e7721275820680926a9be22497bab3fd`.
- Toolchain-inclusive637: `d5f4c929c1937c06df41d07f764d793590b50e4aa7e38f128039fb3d89534630`.

ScriptedView adapts original W3DView choreography into original game common/client
code. It remains abstract: physical output, projection and transformed camera
backing must come from a concrete adapter. It imports no adopted-framework private
headers or internals. Original View snapshot version1/angle/XYZ remains unchanged.
No retail inputs or assets were used or modified.

## Behavior exercised

Four activity flags, update order zoom/pitch then rotate OR waypoint movement;
frame-quantized rotation/zoom/pitch versus explicit elapsed milliseconds for paths;
source parabolic easing, reverse rotation exact terminal angle, hold/tracking,
freeze retirement, time multipliers and disabled-camera behavior. Zoom alone does
not clear freeze; rotation/path retirement does. The six original motion-blur
predicate exceptions remain distinct from physical filter implementation.

Source path interpolation, shutter delay, rolling average, capped linked path,
consume-on-query arrival and next-update final retirement are exercised. Instant
move retains pending path state. Empty and coincident paths are initialized;
all padded arrays use MAX_WAYPOINTS+2. A near-coincident-link cycle that otherwise
never advances the source point count rejects using constant-storage cycle
detection; a noncoincident cycle retains the original capped path behavior.
Finite coordinate bounds protect original squared-length math. Full-width frame
and time-multiplier arithmetic avoids signed overflow/undefined floating casts.

Tracking uses actual ThingFactory-created actors with original InactiveBody,
VeterancyGainCreate, nonempty partition and reciprocal Drawable binding. Actual
GameLogic lookup observes changed positions; real destroyObject/processDestroyList
withdraws the actor and tracking retains its last valid position through hold.
Missing initial target rejects before activation; corrected creation/tracking
works. Near-coincident facing must not cancel accepted tracking/hold implicitly.

NativeServiceOwners acquires/publishes/retires actual shared camera objects:
complete allocation prefixes [0,1), terminal1, every corrected retry. Duplicate
publication preserves the accepted owner. Choreography has exact standard-
allocation terminal0. Each family repeats three whole lifetimes and checks actual
Waypoint pooled units and whole memory-manager retirement. The test adapter
explicitly rejects all physical operations; it does not fabricate camera backing.

## Validation

All3 full configured builds PASS (`cmake --build <configuration> -j3`).
Focused5 PASS under unchanged TIMEOUT60 and unsuppressed ASan/UBSan/LSan:

| Configuration | Focused5 | Related145 |
| --- | --- | --- |
| GCC | PASS, 5.11s | PASS, 140.06s |
| GCC ASan/UBSan/LSan | PASS, 22.11s | PASS, 627.19s |
| Clang ASan/UBSan/LSan | PASS, 12.48s | PASS, 377.31s |

Focused regex:
`^(original_script_camera_|original_logical_world_factory-creation-camera)`.
Related regex:
`^original_(script_camera|object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.
All3 related145 PASS; the matrix ran sequentially on frozen source. Configured full suite335
has not run; it remains an N2 completion gate.

## Retained failures and corrections

Initial focused build failed on fixture WW3D collision include visibility, missing
GameCommon frame constant and device-local minf/maxf helpers. A second focused
build exposed fixture unsigned-to-WaypointID conversion. These were interface/
build wiring errors; no tests ran against failed/stale builds. Source public
includes, std::min/max and typed WaypointID corrected the cohort.

The first camera-mode/ownership families passed, but actual-actor fixture camera
initialization rejected source default zero camera pitch/height. Supplying a
generated GameData block exposed its initialized-root precondition; initializing
that root exposed the required explicit user-storage parent. A separate generated
user tree and NativeServiceOwners publication supply that parent without routing
preferences through asset mounts. These were fixture omissions, not permission to
invent production defaults or bypass original parsing.

The completed tracking fixture then failed repeated-owner baseline because its
warm-up still exercised only the earlier creation cohort. Warming two complete
GameData/storage/camera/actor roots, rather than skipping ownership checks,
corrected the baseline. Final focused5 pass all3, with exact whole-owner cleanup.
Local logs `/tmp/n2-script-camera-*.log` retain each failed build/fixture result and
corrected retry. No failed validation is counted as success, and no timeout,
sanitizer suppression, pool accounting or acceptance criterion was weakened.

## Remaining requirements

Concrete native logical camera/eye/spatial queries, scripted camera lock, client
clock/UI/font/display-string ownership, legitimate TacticalView integration and
actual GameEngine/GameLogic init/update remain pending. The shared abstraction
does not yet own those services or change headless client's rejection of UI/view.
Root child/script/RNG journals, shipped fog snapshots/borders/transfers, accepted
geometry replacement, walls/model bridges, CPU draw/model metadata and compiler-
matching actual simulation remain mandatory. Do not accept N2 from these tests.
