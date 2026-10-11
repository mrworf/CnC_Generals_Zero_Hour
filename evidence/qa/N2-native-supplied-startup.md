# N2 plan02 slice06 — supplied-data startup and frozen qualification

Source base: `40f69080187a` plus the slice06 source and fixture changes. The
renderer is disabled in three Debug configurations: normal GCC, GCC with
ASan/UBSan/LSan, and Clang with ASan/UBSan/LSan. This is the frozen N2
qualification record; the earlier five slice reports remain the detailed
source/input/semantic/failure witnesses.
The inclusive current worktree code fingerprint (3,468 code/build/test/tool
files, including this audit wrapper; sorted pathname and per-file SHA256)
is `9802cfb634282f4f8fdfd528d06a95c01226f7308df303bfedefd101f3c6e73f`.
It includes the 17 pre-existing dirty paths; the 16 outside the shared CMake
file remain byte-identical to the delivery transaction's starting snapshot.

The actual `zero_hour_headless --startup` entry ran original
`GameEngine::init` through post-init and complete teardown against a locally
supplied, read-only data link. The redacted wrapper hashes every supplied
regular file before and after, creates independent disposable user-data and
cache roots, and retains no private selector, path, content, or derived cache.
It verifies missing-root rejection, required user-data failure, successful
startup with a non-writable cache directory, and corrected startup. All three
configurations reported:

```text
NATIVE_STARTUP_AUDIT stage=post-init status=PASS
MISSING_INPUT=REJECTED
STORAGE_FAILURE=REJECTED
CACHE_FALLBACK=PASS
INPUT_INTEGRITY=UNCHANGED
```

The headless factory extends the original module registry with all 19
source-ordered W3D draw-data parsers already implemented in the shared CPU
source. Physical draw-instance requests still reject. This closes the
first actual supplied-data startup gap without asserting a rendered scene.
The supplied-data run additionally exposed a negative shift for regular
veterancy in original flags; the zero-based enum now maps to defined flag
bits, with invalid enum values guarded. GCC and Clang sanitizer startup
passed after that correction. A generated composed-client fixture now
supplies the campaign file read by original client initialization, warms its
whole input cohort, and checks campaign-owner retirement. The graph fixture
links its required original audio owner; the native audio extension is listed
in CMake's native-source section, preserving the original-project source
graph check and compiled source set.

## Reached-source and provider manifest

The call path is the CLI in `tools/zero_hour_headless.cpp` → original
`GameEngine::init` (`GameEngine.cpp`) → original `GameClient`/`GameLogic`
initialization → original `GameEngine::update` → `GameLogic::update` and
pending `startNewGame` → `NativeTerrainLogic::loadMap`/
`NativeLogicalWorld::load` → original sides, scripts, templates, objects and
modules → 120 steady source updates → reverse owner teardown. The generated
input and exact semantic checks are recorded in
`N2-h1-map-admission.md`, `N2-h1-original-updates.md` and
`N2-h1-checkpoints.md`; failure/retry and pool boundaries are in
`N2-h1-failure-retry.md`. This table classifies every source service cohort
on that path. Names are public engine identifiers, not supplied-data names.

| Reached cohort | Source-owned providers and boundary |
|---|---|
| Root/registration/storage | SubsystemList, FileSystem, NativeUserStorage, NameKeyGenerator, CommandList, GlobalData, GameLODManager, DeepCRCSanityCheck, GameText and GlobalLanguageData initialize in original order. Read-only mounts and separate user storage are mandatory. |
| Parsed startup definitions | ScienceStore, MultiplayerSettings, TerrainTypes, TerrainRoads, RankInfoStore, PlayerTemplateStore, FXListStore, WeaponStore, ObjectCreationListStore, LocomotorStore, SpecialPowerStore, DamageFXStore, ArmorStore, ThingFactory, UpgradeCenter, AI, CrateSystem and MetaMap consume the original INI registrations. ThingFactory admits actual object files and ModuleFactory supplies behavior/data schemas; the headless extension registers the 19 original W3D draw-data schemas for definition ingestion. |
| Other original engine services | FunctionLexicon, MessageStream, SidesList, CaveSystem, BuildAssistant, TeamFactory, PlayerList, Recorder, Radar, VictoryConditions, ActionManager, GameStateMap and GameState retain source initialization and cleanup. The single-player path publishes a null Network. |
| Logical client | Original GameClient owns RayEffects, DrawGroupInfo, DisplayStringManager, FontLibrary, Display, ControlBar, InGameUI, TerrainVisual and CampaignManager. Native implementations provide CPU/view/terrain backing and explicit rejection of physical display output. The composed fixture verifies the campaign owner and generated metadata cohort. |
| Logical world | Original GameLogic owns PartitionManager, GhostObjectManager, TerrainLogic, ScriptActions, ScriptConditions and ScriptEngine. H1 loads sides, players, teams, two actor definitions, `InactiveBody` and one `DeletionUpdate`, authored terrain and a script. Source frame, AI/build/partition and destruction paths run for 120 steady updates; the no-road fixture rejects an unsupported device road request. |
| Audio, particles and draw | Original audio metadata and logical state initialize through NativeHeadlessAudio; physical playback rejects. ParticleSystemManager retains the bounded logical path and rejects physical render requests. H1's actors request no draw instances or model/bone query. A requested physical W3D draw instance rejects; N3 owns its implementation. |
| Media/LAN | No window, decoder, video playback or LAN session is requested by H1. These outputs remain deferred to N4/N6 and are not represented as successful services. |

Validation commands (run from the repository root; the wrapper does not
print the data link):

```sh
python3 tools/run_native_startup_audit.py --binary build/original-core-gcc/zero_hour_headless --workspace-link
python3 tools/run_native_startup_audit.py --binary build/original-core-gcc-sanitize/zero_hour_headless --workspace-link
python3 tools/run_native_startup_audit.py --binary build/original-core-clang-sanitize/zero_hour_headless --workspace-link
python3 tools/compare_h1_checkpoints.py build/original-core-gcc/original_native_headless_root_fixture build/original-core-gcc-sanitize/original_native_headless_root_fixture build/original-core-clang-sanitize/original_native_headless_root_fixture
ctest --test-dir build/original-core-gcc --output-on-failure -j4
ctest --test-dir build/original-core-gcc-sanitize --output-on-failure -j4
ctest --test-dir build/original-core-clang-sanitize --output-on-failure -j4
```

The H1 comparator matched all 363 records of 224 bytes byte-for-byte across
all three configurations and rejected four mutations. Canonical SHA256:
`d9b52c40d02661238631a01d0b5a6977162904d27bd65c39ab90aee814211d1a`.
The final normal GCC CTest matrix passed 524/524. The six composed-client
regressions passed after adding the generated campaign input. Clang
ASan/UBSan/LSan passed 524/524. GCC ASan/UBSan/LSan passed a first segment
of 33 whole tests and a continuation excluding exactly those names passed
491/491. A comparison with CTest's full 524-name inventory proved the two
segments disjoint, complete and without failure or timeout. LeakSanitizer
tests and supplied-data sanitizer probes ran in a host execution context
where ptrace inspection is available; the sandbox's ptrace restriction was
not treated as an application result. The static source-graph and generated
campaign-fixture corrections were followed by a complete final normal matrix
and both complete sanitizer matrices on the code fingerprint above.

This probe establishes definition ingestion and teardown only. Generated H1
is the bounded world/update witness; physical rendering, media, full retail
gameplay and release acceptance remain with later milestones.
