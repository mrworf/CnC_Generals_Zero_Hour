# N2 native camera navigation checkpoint

Status: implementation verified; required focused/related matrix passed. N2 plan01/slice03 remains
in progress. No concrete frame/model, real-map constraints, shake/slave, UI/root
initialization or whole-game output acceptance is established here.

Parent: `f3a4f0cba0b459b659d301b05b9c8519fa78344f` (clean initial worktree). The existing slice plan was refined
before production edits. No vendor source, retail asset or recovery file changed.

Canonical644 SHA256:
`171ae7088b4690ecd7c4663023c434318a9264af410a844fea7b0b69459003de`.
Inclusive651 SHA256:
`9ed5db310044d259c558246dadf7361781d11c645fc4c27cf09cd6d867d7fd09`.
Same digest recipe as the preceding camera checkpoint. Configured suite367;
full367 is not run and remains a milestone completion gate.

## Behavior and evidence contract

Production NativeCameraView now consumes NativeCamera's actual unnormalized
view-plane displacement for source screen scrolling: scale250, retained integer
aspect, rotated camera right/up basis, XY-only position adoption. Candidate
position/camera is validated before publishing scroll state or canceling only
rotation. Shared ScriptedView implements the source lock/snap activity flags and
disabled-motion admission/withdrawal, distinct from movement-finished predicates.
No simulation callbacks, renderer internals or native resource ownership added.

Two new camera families cover production navigation, rotated/portrait viewport,
retained zoom/pitch/path, scripted-lock flags, disabled lock admission, missing
parent, malformed/overflow rejection with accepted-state preservation, corrected
same-owner retry and exact allocation terminal0. Existing camera families remain;
all9 repeat3 under actual memory manager/pools and exact whole-owner retirement.
GCC iteration build/focused9 PASS4.33s; no development failure observed so far.
Logs `/tmp/n2-camera-navigation-gcc-{build,focused}.log` retain that result.

## Required frozen matrix

```sh
cmake --build build/original-core-<configuration> -j 4
ctest --test-dir build/original-core-<configuration> -R '^original_script_camera_' --output-on-failure
ctest --test-dir build/original-core-<configuration> -R '^original_(native_font|text_ownership|script_camera|object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)' --output-on-failure
```

| Configuration | Exact live handle | Full build | Focused9 | Related177 |
|---|---:|---|---|---|
| GCC | 99342 terminal0 | PASS | PASS4.43s | PASS171.36s |
| GCC ASan/UBSan/LSan | 32125 terminal0 | PASS | PASS20.41s | PASS740.94s |
| Clang ASan/UBSan/LSan | 65854 terminal0 | PASS | PASS11.87s | PASS450.43s |

Logs `/tmp/n2-camera-navigation-{gcc,gcc-sanitize,clang-sanitize}-frozen-{build,focused,related}.log`.
All jobs returned terminal0. Checks ran on the normal host with timeout60 per
family and unsuppressed sanitizers/leak checks. No failure, restart or coverage
waiver occurred. Both frozen hashes were independently recomputed unchanged
after terminal acceptance. Commit this coherent source/test/documentation batch;
full367 and whole-root/milestone acceptance remain pending.

Durable source findings and pending lifetime/constraint contracts:
`docs/original-engine-formats.md`. Continue existing N2 plan01/slice03 afterward;
N3–N7 remain pending, not superseded or reordered.
