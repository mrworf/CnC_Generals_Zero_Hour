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
