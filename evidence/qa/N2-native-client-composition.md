# N2 native logical client composition — component checkpoint

Parent: `2744b0dc1c4da3adcf293a4686e21001cd582d8e`; clean starting tree.
Same N2 plan01/slice03 transaction; no milestone or slice acceptance claimed.

## Delivered behavior under validation

The default NativeHeadlessGameClient adopts actual logical Display/UI/View,
ControlBar commands and native font/display-string owners. DrawGroupInfo INI
and localized overrides precede actual native text-cache postprocessing. The
base-client C3 member survives Display-owned Views during base cleanup. Physical
output/device queries reject. Limited fixtures select DrawablesOnly explicitly.

Initialization rejects foreign publications and duplicate composition. Failure
retires adopted services, preserving constructor-owned DrawGroupInfo. Reset
retires command overrides and resets the owned Display; missing terrain rejects
before mutation. Unselected Drawable deselection needs no unused GameLogic parent.

Generated tests exercise original CSF/INI and local fonts, nonempty text cache,
pre-terrain camera startup, command binding, repeat reset, client-only Drawable
reset/destructor with live UI but no logic, foreign-publication rejection, bad
data teardown/same-owner retry, four fixed typed pools and complete allocation
failure/retry pairs. Unloaded NativeTerrainLogic supplies only a reset parent;
its extent output is fully initialized, without accepting device/map admission.
Composed frames, nonempty science/model parents, terrain/ghost/root factories,
GameMain execution and whole-root actor/team/player retirement remain pending.

## Initial matrix (superseded composition reset fixture)

- Canonical654: `811ddea1a284c47349620de20f839bae0e0ffc6f7a765f5a0c7b2ea67569aa3c`.
- Inclusive661: `11212530bdf950e097f94fc60f68aa54ea7b0b59e9584a9eee89523398a7aac8`.
- GCC focused5 PASS29.71s, terminal0, session50064; allocation terminal205 on
  superseded fixture. These composition results are not current reset acceptance.
- Every early/late failure/retry retained, repeated3 with exact whole-owner retirement.
- Configured384/full384 unrun. Frozen configured builds + related194 running:
  GCC75308, GCC sanitizer52980, Clang sanitizer76169. Results pending.
- Normal host, unsuppressed ASan/UBSan/LSan, unchanged60s bounds, build-j2,
  related-j1. No GPU or physical-output acceptance.

Logs `/tmp/n2-native-composition-<configuration>-frozen-{build,related}.log`,
configuration gcc, gcc-sanitize, clang-sanitize; final GCC focused:
`/tmp/n2-native-composition-gcc-final-focused.log`.
Regex includes command_metadata, native_font, text_ownership, script_camera,
object/team/player ownership, ai_pathfinder, waypoint_graph, native_terrain,
logical_world, native_water/client, diplomacy/statistics, header_paths,
map/chunk/runtime/core/enum/source_boundaries, anchored `^original_`.

## Retained iteration record

Focused builds, not full-build/assert loops:

-52619 compile1: dumpModelAssets override outside source debug conditional.
-10139 compile1: missing TerrainLogic include in GameClient.
-33087 compile1: fixture missing RayEffects include and used a fixture-only
  accessor; inspect public production build count/startup parents instead.
-58362 build0;29908 focused4 PASS16.33s before text-cache/Drawable expansion.
-61238 compile1: missing GameText include after source cache adoption.
-9487 corrected cache build0.
-27986/54198 compile1: fixture missing GameLogic include for absent-parent check.
  Final50064 compile0/focused5 PASS29.71s.

Source review found common TerrainLogic::getExtent is an empty stub and cannot
provide defined reset bounds. Replace only the composition fixture's parent with
actual NativeTerrainLogic (absent device admission explicitly rejects). Production
source is unchanged. Old matrices retain unaffected-family evidence; corrected
native-font families require new builds/runs and source identity before commit.

Corrected frozen canonical654:
`37e9d2b761a539358fd59b3225478dae727b739a8342e3bdf330832fbed5f936`;
inclusive661 `bbf2510c68691a29630f9368291a28c5e12d864be67c29de26749865019f89f1`.
Corrected GCC42273 build/focused5 PASS29.69s/terminal0, allocation terminal205.
GCC broad75308 terminal0/PASS194 in213.08s; composition families using the old
parent are discarded. Broader unchanged-production matrices52980/76169 still
running; retain only unaffected173. Corrected configured builds/native-font21
running GCC83799, GCCsan64650, Clangsan4308. These cover every affected family.
Logs `/tmp/n2-native-composition-<configuration>-defined-frozen-{build,font}.log`;
focused `/tmp/n2-native-composition-gcc-defined-reset-{build,focused}.log`.

Corrected configured builds/native-font21: GCC83799 PASS48.71s/terminal0;
Clangsan4308 PASS119.00s/terminal0; GCCsan64650 pending. Verified by reconstructing
the old test-only delta in memory: the aggregate reproduces the old canonical
hash exactly, proving every other source byte is unchanged. Therefore unaffected
173-family broad evidence remains applicable; all21 native-font families are
rerun on the corrected fixture, rather than accepting the invalid five families.

No vendor, recovery archive, retail link or target changed. Inputs are disposable
test-owned content. Exact commit provenance follows matrix review.

## Final consolidated component evidence

All configured builds PASS. All3 unchanged-production broad194 sweeps completed
terminal0: GCC213.08s, Clangsan551.88s, GCCsan872.16s. Only unaffected173 are retained
from those broad sweeps. The complete corrected native-font21 family matrix
passed terminal0 on final canonical654/inclusive661: GCC48.71s (83799),
Clangsan119.00s (4308), GCCsan154.43s (64650). Together these prove all194 relevant
families per configuration; they are not represented as one same-hash194 run.
The inverse test-only delta reproduced the initial hash exactly, and the final
source rehash matches after every terminal. There are no live processes.

Every corrected allocation family independently reports terminal205 on all3,
three repeats, every failure/retry. Four typed-pool failures and same-owner
retries pass on all3. No sanitizer suppression, timeout extension or physical
output waiver. Required whole-root/reset/frame acceptance and full384 remain
unrun. This is a coherent default-client composition checkpoint, not completed
slice03 or N2. Payload SHA is recorded in workflow/delivery/state.yaml.
