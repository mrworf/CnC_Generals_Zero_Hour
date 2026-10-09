# N2 native spatial camera/View checkpoint

Status: coherent implementation verified; required focused/related matrix passed. This does not
complete slice03/N2, concrete headless root adoption or physical visual parity.

Transaction parent `ce4c7e4bfece565f61e92599c986faaca6e81c55`; initial tree clean. Existing N2 plan01/slice03 was
refined before production changes. No upstream framework/library/tool source,
retail asset or recovery archive was modified.

Frozen canonical644 SHA256:
`a11d44acec26bd44003d6e7367d9e18076717ff992ebf28ef654f908b02f2e51`.
Frozen inclusive651 SHA256:
`4560e74d338410696e65fb10ecdef03b2c93987d9ac78aabc1b1d7f7fdcf5ef4`.
Same original-source digest recipe as the preceding terrain checkpoint. Configured
suite365; full365 remains the N2 completion gate, not yet run.

## Implemented behavior and scope

NativeCamera is game-owned staged CPU eye/basis/frustum backing for source normal
zoom, pitch-then-yaw/ground/FX pitch behavior, source pixel projection, near/far
versus offscreen classification, normalized far-depth rays and Z-plane queries.
Owner-local eye references replace no source static storage. Configuration fails
without replacing accepted state; queries allocate nothing. Defined arithmetic
and checked truncation/origin addition avoid malformed-input UB.

Production NativeCameraView consumes it in actual ScriptedView state-change hooks
and View eye/projection/plane/terrain/elevated-look-at entry points. Mesh picks
compose the actual TerrainVisual virtual query with original TerrainLogic bridge
selection; missing required parents preserve output and reject. Center ray uses
logical(0,0), including odd-sized viewports. Production width/FOV/height/bias
boundaries reject invalid input. Legacy screenToWorld remains explicitly rejected
(source implementation is a crash and the active source has no callers).

NativeCameraView remains abstract for actual frame, constraints/scroll/lock,
reachable shake/slave, scene picking and physical output. A test adapter rejects
those missing operations; it is not a production headless provider. Source phase,
nonempty bridge/model, UI/root adoption and whole-game output remain required.

## Generated evidence

Three added script-camera families spatial/spatial-negative/spatial-ownership:
actual shared original choreography drives real production spatial queries, which
reach an actual HeightMapData-v2 decoded visual grid and original empty bridge
list. Source eye/viewport center, yaw/pitch order, both FX pitch branches, near/far
boundaries and extended far depth, offscreen/invalid, odd viewport logical center,
pixel/plane inverse quantization, elevated terrain look-at, malformed/missing
parents and corrected retries are covered. All7 original camera families repeat3
with original memory manager, Waypoint typed pool and whole-owner retirement.
Native spatial state/query acquisition has an exact terminal0, not a made-up fault
prefix. No retail/private content is used or emitted by these generated fixtures.

Intermediate focused7 (before final FX/odd-viewport additions) PASS all3:
GCC3.37s/GCC sanitizer15.29s/Clang sanitizer9.11s. Final frozen evidence follows.

## Required frozen commands and exact handles

```sh
cmake --build build/original-core-<configuration> -j 4
ctest --test-dir build/original-core-<configuration> -R '^original_script_camera_' --output-on-failure
ctest --test-dir build/original-core-<configuration> -R '^original_(native_font|text_ownership|script_camera|object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

| Configuration | Exact handle | Full configured build | Focused7 | Related175 |
|---|---:|---|---|---|
| GCC | 62905 terminal0 | PASS | PASS3.38s | PASS170.01s |
| GCC sanitizer | 33437 retired; completed PASS log | PASS | PASS15.75s | PASS733.28s |
| Clang sanitizer | 46586 terminal0 | PASS | PASS9.28s | PASS446.08s |

Logs: `/tmp/n2-camera-view-{gcc,gcc-sanitize,clang-sanitize}-frozen-{build,focused,related}.log`.
All required jobs completed without restarting. The GCC sanitizer handle had
retired by the final poll; its complete log records 175/175 PASS and elapsed time.
Both frozen source hashes were independently recomputed and match. All individual
tests retain timeout60; ASan/UBSan/LSan are unsuppressed. This checkpoint is ready
for the coherent source/test/documentation commit, not milestone acceptance.

## Retained development failure/retry pairs

- POSIX finite helper-name collision: entire helper/call cohort renamed
  requireFinite; original and retry logs `/tmp/n2-camera-spatial-gcc-build*.log`.
- Initial spatial ownership warm-up omitted new parser/name cohort; warming the
  identical complete spatial/parser cohort before baselines restored exact
  retirement, not an allocation waiver. Original focused failure and corrected
  warm retry retained under `/tmp/n2-camera-spatial-gcc-focused*.log`.
- Default sandbox sanitizer runs failed for every family because LSan cannot
  run under ptrace. Same host commands with all checks enabled passed, retained
  in `/tmp/n2-camera-spatial-*-focused.log` and `*-focused-host-retry.log`.
- NativeCameraView initially used the Vector3 forward declaration without the
  original public math definition; added WWMath/vector3.h. Original build failure
  and corrected retry retained under `/tmp/n2-camera-view-gcc-build*.log`.

Verified source findings and remaining coupled ownership/format contracts:
`docs/original-engine-formats.md`. Continue the existing N2 plan01/slice03 concrete
camera/frame/model and staged UI/root work after this checkpoint; N3–N7 pending.
