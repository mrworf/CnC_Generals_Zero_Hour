# N2 original environment ownership — validated checkpoint

Parent documentation checkpoint: `48f61848`. Existing N2 plan01/slice03; no new
milestone or acceptance boundary. The coherent source cohort is validated;
full N2 startup/simulation acceptance remains pending.

Actual water/weather parsers now stage pooled candidates and preserve owning
links while copying the root base payload. Time-of-day water is transactional.
GameEngine owns acquired configuration through world retirement and restores
borrowed configuration. Snow backing is guarded through virtual initialization;
unsafe numeric operations and duplicate initialization reject before replacement.
Assets and upstream dependencies are untouched.

Focused evidence stages:

- Initial five GCC families: PASS 0.13s.
- Expanded eight families: GCC PASS 0.22s, GCC sanitizer PASS 3.44s, Clang
  sanitizer PASS 1.45s. Leak detection and UBSan remain enabled, no suppressions.
- Source weather standard terminal 16, time-water terminal 11; every prefix and
  corrected same-owner retry retained. Snow array terminal 1. Fixed pool
  exhaustion and retry is separate from the standard allocation manifest.
- Actual GameEngine init prefix: GCC PASS 1.94s. Generated complete selected-file
  cohort and separate disposable user roots; stops deliberately at unavailable
  audio provider. Entire admitted prefix retires repeatedly.

Retained iteration failures:

- Build 89552 terminal 1: production OVERRIDE dereference yields pointer; fixed
  reference conversion with the source wrapper's double dereference.
- Build 86373 terminal 1: fixture lacked pure no-output Snow update override;
  corrected, including verified source opacity-field spelling.
- Root build 9717 terminal 1: explicit command-list declaration missing; initial
  wrong header retry terminal 1, actual MessageStream header correction passes.
- Root prefix test terminal 8: incomplete generated selected INI files, root
  stops before first registered owner commits. Redacted diagnostic confirms
  ERROR_BAD_INI, registered count zero, configuration mask zero. INICore prepFile
  requires every default/custom INI. Whole prefix fixture corrected together;
  retry 88463 terminal 0 and audio boundary reached.

Final frozen configured builds and related249 PASS on GCC (271.38s), GCC
ASan/UBSan/LSan (1116.02s) and Clang ASan/UBSan/LSan (708.31s). All three process
terminals are zero. Leak detection remains enabled without suppressions. Earlier
focused stages are retained separately. Full N2 remains incomplete: no native CreateGameEngine
factory, complete GameMain startup, loaded simulation/checkpoints, native ghost
and model providers or physical presentation accepted by this evidence.

Frozen cohort: canonical660
`7bb51cbf49cf61fbe452ffa5025ffb49bdf4c8322b232936aba0303677685df1`,
toolchain-inclusive667
`0943f9e7cdfd2777e0e342ab22e967ff8c6fd07a06e89975cdfdabd0e7d50651`.
Configured401; related249 per configuration. GCC6023 completed with configured
build and related249 PASS271.38s, exact process terminal0; do not re-poll it.
Clang sanitizer50901 completed with configured build and related249
PASS708.31s, exact process terminal0. GCCsan17049 completed with configured build
and related249 PASS1116.02s, exact process terminal0. No live sessions remain;
do not re-poll completed handles.
Logs: `/tmp/n2-environment-<configuration>-frozen-{build,related}.log`.
Both frozen source fingerprints were rechecked before commit. Full401 remains
unrun and is required at the milestone-completion boundary, not waived.

Both live sanitizer suites have passed all nine frozen environment families,
including actual root-prefix cleanup (GCC sanitizer 8.33s, Clang sanitizer 4.75s).
This proves that family on the frozen source, not full-game startup or simulation.
No failed case remains in the final frozen related suites.

Validated payload commit: `2b14ad41699323b5243b4ab061bf02aa27773826`.
