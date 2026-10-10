# N2 logical UI / tactical startup cohort — verified component checkpoint

Parent `0d18cdac5f7cef07aa8ae272ce353f985ffc70a2`; initial worktree clean.
Existing N2 plan01/slice03 refined before production changes. This is not N2,
root, full command registry, render or whole headless client acceptance.
No upstream dependency, retail asset or recovery archive changes.

## Coupled implemented behavior

- Preserve original client-before-logic startup: initialize the real native
  camera without terrain, retain invalid constraints, perform both explicit
  transform builds, and adopt later actual native terrain without hidden effects.
  An absent optional terrain query preserves an already-valid source cache.
- Guard actual InGameUI tactical candidate publication through init, viewport and
  default setup; Display admits only complete candidates. Failed setup restores
  the borrowed tactical publication after legitimate candidate destruction.
  Duplicate admission preserves the accepted View.
- Explicit NativeHeadlessInGameUI retains actual source configuration, logical
  UI/selection and camera state while separating physical menus/output. Its
  borrowed C3 owner must outlive all Display-owned cameras.
- Initialize/reset the complete camera input/tracking/quit flag cohort. Stage
  selection-list node acquisition before flag/count publication; validate
  deselection membership before mutation. Retain source nexus selection behavior.
- Generated real-world fixture loads actual InGameUI.ini, executes headless UI
  update/reset, selection failure/retry, actual Drawable terminal withdrawal,
  pre-terrain camera creation and later actual-terrain/cache admission. Repeated
  roots retain exact standard allocation/View/pool/whole-manager checks.

## Retained iteration evidence

- GCC33031 build failed because the new fixture lacked TerrainLogic.h declaring
  the singleton. Fixture-only include corrected; no test executed in that run.
- GCC95851 corrected focused16 PASS14.90s, terminal0.
- Clang sanitizer38382 focused16 PASS37.41s, terminal0.
- GCC sanitizer87684 focused16 PASS66.09s, terminal0.
  These are the initial admission cohort, before final accepted-cache refinement
  and explicit logical UI mode. They do not accept the later source.
- Complete cohort GCC71573 and Clang25936 failed fixture compilation: the new
  rebound used a nonexistent bindToObject name. Corrected to actual reciprocal
  Drawable::friend_bindToObject/Object::friend_bindToDrawable calls.
- GCC82072 and Clang44192 retries and GCC sanitizer89351 ran focused26, each
  failing only the same source-selection baseline oracle (terminal8;
  20.87/52.51/92.20s). The preceding malformed-membership probe legitimately
  acquired an owned flash envelope. Normalize the complete cohort before the
  baseline, not by exempting changed counts. The surrounding callback/pool census
  also identified late selected-flag publication and list-node rollback gaps;
  corrected together and added actual pooled exhaustion/retirement/retry coverage.
- Corrected complete-cohort focused26: GCC97800 PASS22.96s, Clang14594 PASS57.12s,
  GCC sanitizer53962 PASS103.00s; all terminal0, on the final frozen source below.
- Frozen configured builds and related185 PASS on canonical652
  `66a4dabb40a8eb4ca06d6a6b4cc1f0542156de1509002a6453be81f3ed15e609`,
  inclusive659 `faf797c161cec042b3d595982b92073aa12e4b756d17f102d6e1271f10f75ced`.
  Recipe matches the preceding camera checkpoint. 375 tests configured.
  GCC51010 PASS181.73s, Clang sanitizer94141 PASS480.59s, GCC sanitizer38442
  PASS806.51s; every run terminal0. Recomputed source hashes match after all runs.

Log prefixes `/tmp/n2-tactical-startup-<configuration>-{build,focused}.log`,
GCC retry suffix, and `/tmp/n2-headless-ui-<configuration>-{build,focused}.log`.
Unsuppressed ASan/UBSan/LSan and existing family timeouts remain unchanged.
Related185 acceptance applies only to this frozen component checkpoint. Full375
has not run; neither the complete slice nor N2 is accepted.

## Remaining root integration

Native headless client currently rejects Display/UI/Tactical state and is not yet
wired to this UI owner. Logical CommandSet/CommandButton metadata still needs its
owner: source ControlBar init/reset cannot be dropped wholesale as physical UI.
Native Display/terrain/model/ghost provider selection and complete original
GameEngine init/frame/scenario/cleanup remain pending. See durable verified source
findings in docs/original-engine-formats.md. Continue the same N2 root transaction;
do not replace the goal with support-only evidence or add microplans.
