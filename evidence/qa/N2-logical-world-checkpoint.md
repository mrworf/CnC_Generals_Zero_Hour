# N2 combined logical-world checkpoint

Parent: `3ef97998bf8d7ce4f190fd584cf3d447ad11f8ce`.
Packet: `linux-upstream-v2`; existing N2 plan01/slice03, still incomplete.
Production and fixture source frozen before the affected matrix below.

## Behavior and source identity

`NativeLogicalWorld::load` connects actual DataChunkInput to original
MapObject/ThingFactory, side/build/team/script/parameter, PolygonTrigger and
logical height readers. It commits or restores the entire logical owner group:
object list/world Dict, writable weather, side contents, trigger list/ID, pending
script publication/group ID, height metadata and actual NameKey namespace.
Detached failed/old owners retire before borrowed SidesList restoration.

Source semantics and the next integration boundary are documented in
`docs/original-engine-formats.md`, sections Combined native logical-world owner
and Source root terrain/waypoint provider phases. No framework, asset or recovery
content changed. Only generated in-memory worlds and disposable fixture roots
are used; no retail audit or physical GPU acceptance is claimed here.

Final canonical608 source SHA256:
`b10129e0d32eb1a9b04064951305a23c8d422030413985f5e421077113286a01`.
Final toolchain-inclusive615 SHA256:
`c089e91fb87e608103c21b98f577fe070113ba8839803c678bec62d1200056e9`.
Use the existing checkpoint recipe: changed paths since9e5c0e4f plus nonignored
untracked paths; sorted path bytes, path/NUL/binary content SHA256 stream;
canonical CMakeLists.txt and GeneralsMD/Code/, tests/original/, cmake/ prefixes;
inclusive additionally tests/toolchain/. Evidence/docs/state are not source hash
inputs. These identities include the committed typed-pool extension3ef97998.

## Generated original-owner coverage

Five independent families, each with three complete provider lifetimes:

- Functional: Object1–3 × Height1–4; file order, z forcing/discard, actual known/
  unknown template lookup, waypoint/light/scorch typed classification, nonempty
  source groups/scripts/parameters/builds/teams/triggers, Dict/weather, large
  finite angle termination and optional-chunk neutral/default behavior.
- Negative: every prefix with required height last, all five duplicate owner
  chunks, missing height, invalid field-selected versions, raw invalid weather
  integers and defined nonfinite angle encodings. Accepted pointer identities,
  group cursor, weather, metadata and standard ownership survive; each rejection
  receives an immediate corrected same-owner retry.
- Faults: complete standard allocation prefixes[0,150), exact success terminal150,
  each followed by corrected replacement and restoration. Stable initial group
  cursor, actual committed advancement, original used pool units and standard
  owners are proved, not inferred from descriptor counts.
  Omitted-SidesList replacement separately covers every prefix[0,75) and exact
  terminal75, including strong copying of both ordinary/skirmish teams, neutral
  owner repair and preservation of accepted script/team identities after failure.
  Every optional failure receives a corrected retry and restores the full baseline.
- Ownership: separately pending nonempty original script tree is retained across
  both rejected and successful world replacement. A newly admitted WorldInfo
  name is removed after later height rejection; retry commits the identical next
  ordinal and world dictionary/weather.
- Pool failures: actual MapObject, PolygonTrigger, BuildListInfo, ScriptList,
  ScriptGroup, Script, OrCondition, Condition, ScriptAction and Parameter pools
  are independently exhausted. Every late candidate failure preserves all old
  owners/cursors and retires its partial units, then succeeds after releasing
  held capacity. Whole-manager destruction retires warmed metadata exactly.

The affected matrix also includes the committed partial typed Parameter factory/
setter prefixes and immediate retries documented in the sides/scripts evidence.
This is neither action execution nor original simulation parity.

## Validation

Debug configurations: build/original-core-gcc (ZH_SANITIZE=OFF),
build/original-core-gcc-sanitize and build/original-core-clang-sanitize (ON).
All three configured full builds PASS. CTest sanitizer environments retain
detect_leaks=1, halt_on_error=1 and UBSan stack traces; no suppression or isolated
bus fixture is involved in these generated CPU-owner tests.

Commands:

```sh
cmake --build build/<configuration> -j 3
ctest --test-dir build/<configuration> -R '^original_logical_world_' --output-on-failure
ctest --test-dir build/<configuration> -R 'original_(logical_world|sides_scripts|polygon_world|native_terrain|native_client|header_paths|map|chunk|runtime|core|enum|source_boundaries|transfer)' --output-on-failure
```

Final frozen GCC focused5 PASS (10.48 seconds); affected101 PASS (80.32 seconds).
Final frozen Clang sanitizer affected101 PASS (240.90 seconds); final frozen GCC
sanitizer affected101 PASS (383.73 seconds). All five world families and the
committed typed-pool extension are included in each101-test matrix. No complete
277-test run has been performed; no milestone/slice completion is claimed.

Prior frozen base batch (before omitted-team carryover) passed affected101 under
GCC (80.20 seconds), GCC sanitizers (379.89 seconds) and Clang sanitizers
(240.81 seconds). Its canonical608 identity was
`c0aad73b227deb57b231c19ac1d08a9b4cf18e3136188183e089c5a43d6f4d4f`,
inclusive615 `d5faa4b5f59d7ea2ce23e8fe1599e4fd0c46de49b84441d704ac0d6ba607099d`.
Final direct source-call review found missing-SidesList team carryover; the final
source adds that compatibility correction and its complete optional manifest.
Do not relabel the base matrix as the final revision's execution.

Retained validation history (logs in /tmp, no private input-derived data):
`zh-world-*-focused.log`, `*-focused-retry.log`, `*-focused-final.log`,
`*-expanded-focused-retry.log`, `*-ownership-retry.log`, `*-frozen-focused.log`,
`*-frozen-related.log` and the corresponding build logs. Initial truncation
failure was a fixture oracle: complete optional-chunk boundaries can be valid.
The next negative fixture tried the legal writer on nonfinite values and was
rejected by the writer before loading; corrected fixtures write defined raw IEEE
words. Expanded builds caught the actual getAction API and fixed-width NameKey
casts. A test invocation against the older executable during that failed build
is not acceptance. Expanded ownership initially omitted the caller-owned
ScriptList envelope around WriteScriptListDataChunk's payload; restoring the
source writer contract corrected it. No production acceptance was weakened for
these fixture defects. Normal ownership retry PASS1/1 (1.85 seconds).
Final omitted-team compatibility build/validation logs are
`zh-world-*-teams-build.log`, `*-teams-final-build.log`,
`*-teams-focused.log`, and `*-teams-related.log`; CTest LastTest.log retains
both exact150/75 terminals and all ten pool failure/retry paths in each compiler.

## Remaining acceptance

This checkpoint does not complete N2 or slice03. Native TerrainLogic must retain
source extents/boundaries/layers, initial waypoint admission/linking and final
newMap snap using the retained full visual-owned grid. Later waypoint/provider
failure must be enclosed by terrain/root publication. Actual ghost/partition,
camera/audio/particle/radar, lighting/texture loading, GameEngine startup and
compiler-matching original simulation remain required. Metadata or owner tests
alone do not establish any of those outcomes.
