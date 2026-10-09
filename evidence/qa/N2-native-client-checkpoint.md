# N2 native client ownership checkpoint

Parent: `0a44f4be7af018fc90ca88d4316b2ad0be4a248b`. This is supporting
progress in existing plan01/slice03, not milestone or full-slice acceptance.

## Behavior and scope

The concrete headless adapter uses actual common GameClient, Drawable and
RayEffectSystem owners, not replacement simulation or device objects. It rejects
premature whole update and physical device creation. Actual shared drawable
updates preserve source freeze/repeated-frame behavior; a regression proves that
client replacement must not inherit function-static last-frame suppression.

Construction initializes acquired pointers before registration, guards partial
ownership and exact registry withdrawal, and keeps module arrays terminated.
ID collisions preserve both accepted mappings; full-width ID lookup never narrows
to a negative index. Failed reset backing preparation leaves accepted drawables
and lookup intact, with corrected same-owner retry. Fixtures use real empty
source templates, actual rays and source fades over repeated complete lifetimes.
Allocation discovery retires before baselines; every prefix retains its retry,
with exact terminal and pool/heap/publication retirement checks.

## Validation and failure history

Initial compilation exposed an accidental mechanical edit to four inline getters;
all four were restored. Fixture compilation then exposed the protected original
ThingTemplate destructor and missing DrawGroupInfo declaration; the fixture uses
only a destructor-visibility subclass and the real declaration. Template lifetime
outlives client drawables. No original behavior was replaced to satisfy the fixture.

The first related matrix passed all runtime tests but failed its retained-source
inventory test: the new native adapter was incorrectly listed as original DSP
source. It now resides alongside existing native adapters in target_sources,
preserving the exact original-source inventory contract. Failure logs remain
`/tmp/zh-client-sort-{gcc,gccsan,clangsan}-tests.log`; focused inventory retry is
`/tmp/zh-client-sort-header-retry.log`. No timeout or leak check was weakened.

Final configured builds pass GCC, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan.
Frozen related114/114 passes each configuration in21.58s/89.73s/66.51s,
including all four native-client families. Sanitizers execute on the normal host
with instrumentation and leak checks intact. Final build and test logs:
`/tmp/zh-client-sort-{gcc,gccsan,clangsan}-build-final.log` and
`/tmp/zh-client-sort-{gcc,gccsan,clangsan}-tests-final.log`. Full246 not run.

Canonical cumulative596 path/content SHA256 since `9e5c0e4f`:
`8aa5b1a2c8ce0745ad82c2ffd728e63ea799d05ce10ac9311f0d93cdbed809c5`.
Toolchain-inclusive603:
`6c49e5e9f661881d588d0830454d438153fce1eab6026fb70d9b270d91b80d68`.
Procedure: sorted path bytes, NUL, binary SHA256 of file bytes. Specification
SHA256 remains `0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`.

## Pending limits

Nonempty module callbacks, object-bound fog/camera/model behavior, caption/audio/
static-image rollback, positive whole-client update, concrete GameEngine startup
and compiler-matching simulation remain pending. Physical team-color/texture-LOD/
scorch/terrain-decoration omissions do not establish interactive compatibility.
No assets, dependencies, or recovery contents were read or modified here.
N2 and slice03 remain in progress; configured full246 acceptance is not claimed.

## Follow-on: registered nonempty client modules

From38375d10, the unshortened actual common ModuleFactory registers198 distinct
providers in the executed normal conditional configuration. Source registration
contains205 calls with conditional entries and a duplicate-name replacement.
All three actual retained client-update providers attach in source order to real
Drawables. Source Beacon hidden action and parallel selectable transition retain
logical selection/visibility/shadows with explicit absent physical UI. Sway is
explicitly stopped through its real method; breeze/world behavior is not claimed.
Animated-particle and Beacon callbacks execute with empty draw tables and no
bound simulation object. This supersedes the empty-template-only limit only for
these named constructors/actions, not arbitrary gameplay or draw callbacks.

Both module arrays reject missing data/provider instead of accepting an interior
null sentinel. A generated late unknown entry followed by a real source provider
rejects, retires its acquired prefix, preserves accepted ID/list/lookup and permits
corrected retry. Actual pool capacity exhaustion covers each of the three module
acquisitions and exact terminal3; complete standard allocation discovery covers
nonempty constructor backing. Full original registry census is1036 operations in
each executed configuration. Four round-robin shards cover every ordinal with
its corrected retry and independently prove the exact discovered terminal, over
three lifetimes. No ordinal/retry pair is omitted to split runtime.

Initial registry checks counted the fixture's live long test-name string at final
whole-owner baseline; that observation now retires before memory-manager teardown.
A source Beacon action then exposed UI dereference in shared hidden status.
Both hidden/selectable source paths were audited and corrected together; original
interactive delegation remains. The sanitizer failure logs have addresses redacted
and remain `/tmp/zh-client-modules-{gccsan,clangsan}-focused-final.log`.
No leak suppression, timeout expansion or weakened coverage was used.

All configured builds pass all3. Focused10/10 passes normal/GCCsan/Clangsan in
13.80s/56.21s/36.42s. Frozen related136/136, including module-data/template/enum
checks, passes in33.04s/134.64s/96.61s. Sanitizers execute normal-host with all leak
checks intact. Logs `/tmp/zh-client-modules-<variant>-visibility-focused.log`,
`/tmp/zh-client-modules-<variant>-final-build.log` and
`/tmp/zh-client-modules-<variant>-related.log`. Full252 not run.

Current canonical596 SHA256:
`88d9cbad253a633973be78fc6192cbc92b506ac57bc10569ede912ae942eca7c`;
toolchain-inclusive603:
`f461cda8f7bfc65ca68fed0de4d185bf645f40b487ca34b027ff79d7090185d0`.
Same cumulative procedure and unchanged specification as above. Source startup
census qualifies common factory descriptions: shipped W3DGameLogic overrides
terrain/ghost factories and common terrain height is zero. Native root integration
must preserve real shipped height/layer and camera/audio dependencies, not accept
these base-owner fixtures as world parity. Details in original-engine-formats.md.
Assets, adopted dependencies and recovery remain untouched; N2/slice03 incomplete.
