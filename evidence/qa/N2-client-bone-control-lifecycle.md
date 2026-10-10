# N2 client bone-control lifecycle

Parent618f7179227a454a11c3e65f06af9488a8dd2444, clean start. Same governing
N2 plan01/slice03; no new agent, plan, framework patch, asset or recovery change.

## Coupled behavior and limits

Real state admission owns/resets recoil rows; idle reselection preserves them.
Actual source fire callback admits recoil with null FX while returning false,
preserves repeated-fire displacement, and safely falls back at barrel==count.
Turret controls use original AI reads when a parent exists and art/default values
otherwise. Generated Drawable coverage exercises the objectless art path, not
real AI integration. Actual parsed master/slave particles sample the turret pose
before recoil; their sampled transform remains until the next particle callback.
START flash timing, damping/cap, tiny-rate settling, zero retirement, flash-only
barrels, idle retention and same-model state reset are covered together.

Captured controls and displayed pose publish atomically: whole index/matrix
validation, duplicate last-write order, local/world translations, late arithmetic
failure restoration, and warmed zero standard-allocation demand. Actual condition
root-copy tests retain each failure and same-owner retry including locally caught
bad_alloc with outer commit, exact pose/row/flash/cache identity and whole fixture
retirement. Final root control standard terminal62; pose admission terminal4.
Full model tick, real AI/physics integration, concrete19, physical FX/audio,
source transfer and actual startup/loaded simulation are still required.

## Retained exploratory evidence

Initial GCC focused4 terminal8: pose2 pass, control fixtures fail barrel census.
Direct generated names bypassed original lowercase normalization in both weapon
and turret INI parsers. Entire cohort corrected, no production lookup change.
Log `/tmp/n2-model-controls-gcc-tests.log` retained. Normalized GCC focused2
terminal8: actual empty FXList crashes because it always queries missing
PartitionManager/local PlayerList, even without nuggets. Clang diagnostic59552
terminal8 identifies the null PartitionManager in original FXList.cpp:810.
Logs `/tmp/n2-model-controls-normalized-gcc-tests.log` and
`/tmp/n2-model-controls-diagnostic-clangsan-tests.log` retained. Native admission
now rejects missing real FX parents before recoil/flash; fixture expects clean
rejection, not a fake effect sink. Corrected parent-focused4 PASS12.52s, then
expanded actual particle/idle/flash/tiny-rate cohort PASS12.50s. No weakened
ownership checks, leak suppression or timeout change.

## First freeze (superseded before acceptance)

Canonical735 SHA256
`79e71aca19450e2deac6d40b743876cbc26d5762870803cc0e1108f276a60f6b`;
inclusive742 SHA256
`8923d37ea4e53ad1cae1aafeeed06c60fbf8836f42b0bee32ee477d44282821a`.
All final target builds PASS, terminal0. Configured full500 **unrun**.
Focused selector `^original_(drawable_queries_producer-controls|w3d_animation_pose)`;
related selector `^original_(drawable_queries_|w3d_animation_|w3d_assets_)`,67 cases.

Final GCC focused4 PASS12.53s, terminal0,13 normalized manifest rows SHA256
`f599763ab13fc186370c55a3287eb53885c18084d8fd225c22f1fe448864491f`.
GCC sanitizer focused4 PASS50.09s and Clang sanitizer focused4 PASS29.32s;
both match those13 manifest rows and hash. GCC related67 PASS225.82s, terminal0,
job70611 retired. Originally live jobs GCC sanitizer62900 and Clang sanitizer85675
each ran focused4 then related67; their intentional retirement is recorded below.
Sanitizers run on normal host, ASan/UBSan/LSan all enabled. Logs
`/tmp/n2-model-controls-final-{gcc,gccsan,clangsan}-{build,tests}.log` and
`/tmp/n2-model-controls-related-{gcc,gccsan,clangsan}-tests.log`.

Production caller review found that Weapon.cpp invokes actual Drawable fire
during GameLogic update on the same owner thread. Client-only pose guards cannot
be applied to this notification. The two owned sanitizer CTest processes were
verified by their exact selectors, intentionally interrupted with SIGINT,
and sessions62900/85675 both retired130; no CTest/fixture children remained.
This is neither a timeout nor a transient observation failure. First-freeze
logs/results remain scoped historical evidence, not current acceptance.

## Caller-reconciled freeze (historical, superseded)

Fire now requires legitimate owner identity/thread but permits the original
logic phase. Turret/recoil draw-stage controls still reject logic phase. Actual
Drawable dispatch tests initialize both real modules and invoke null-FX fire
through the source module walk inside GameLogic phase. Local savepoints guard
whole multi-module publication: failure restores exact current/state owners,
while child commit promotes earliest retained Current/validated ownership into
the parent when needed. Root rollback uses moves, no cloned rollback owners or
double ownership. Tests combine child failure with outer commit and child success
with outer rollback. Source locomotion recoil rates also restore on walk
exceptions; full real Object/locomotion integration remains pending. External
FX side-effect rollback is not inferred from null-FX notification coverage.

Canonical735 SHA256
`131f85a74f82a11dde3ec39d32bf48b970a3811cc56e9c5b6bf8a37e8f420be9`;
inclusive742 SHA256
`0d48b6311962803bef5a299dccb6b713d63191facaa95ba2823da867f76af4d2`.
All dispatch builds67284/78884/29734 and focused4 jobs33021/49462/98952 retired0.
Focused4 PASS GCC12.52s/GCCsan50.15s/Clangsan29.54s. All3 have13 normalized
manifest rows SHA256 `64a22398d19b4194d1c73651186a8436d7def1282c36a7884ff73652fae0171f`.
Complete coupled root control/notification terminal62, every prefix/retry;
pose admission terminal4. Related67 jobs39597/64522/98395 subsequently retired0
for GCC/GCCsan/Clangsan; reconciled final historical results are below.
Logs `/tmp/n2-model-controls-dispatch-{gcc,gccsan,clangsan}-{build,tests}.log` and
`/tmp/n2-model-controls-dispatch-related-{gcc,gccsan,clangsan}-tests.log`.
Full500 remains unrun. This freeze was superseded before commit/acceptance.
GCC related67 PASS225.70s, terminal0, job39597 retired. Production/build footprint
excluding tests:659 files,
SHA256 `b77c3ac44655085b49f22549f0a445c8b5c6578955bf68683a57b309877feecd`.
This uses the same canonical changed/untracked source union from9e5c0e4f, omitting
paths beginning tests/. Recheck it across the pending test-only oracle addition;
do not present the old complete source hash as new fixture execution evidence.
Slice03 and N2 remain incomplete. After this cohort's
verified checkpoint continue real-parent ordered tick/attachment/construction/
power/movement duration, lazy clock/event consumption, source transfer and
concrete19, then actual original startup/loaded deterministic simulation.

The final test-only oracle refinement added
a direct Source Drawable late-module rejection inside a catch-and-commit outer
root, without the fixture's additional whole-controls local savepoint. Use the
second module's actual nonfinite initial recoil to reject after the first module
publishes, then restore that same borrowed config and prove corrected retry.
This distinguishes Drawable's own local savepoint from the enclosing test
savepoint. The test-only extension passed new focused assertions in all3;
unchanged production/build evidence was retained with distinct source provenance.
The subsequent production ownership correction below required a fresh matrix.

Historical job reconciliation: caller-freeze related67 jobs39597/64522/98395
all retired0: GCC225.70s/GCCsan907.70s/Clangsan538.43s. Each emitted122
normalized manifest rows SHA256
`b200b8e3b29e1f17f473d860a13c68d4fb8612a8366ab54267c57e57a45c646c`.
The subsequent test-only direct Drawable late-module oracle passed focused4
on all3 (GCC12.54s/GCCsan50.22s/Clangsan29.36s); all jobs retired0.
Canonical735 `8cdef60d82e1464a91461bdc2a9b9b2ac035075c7efeea656a0c3af34d6fdf16`,
inclusive742 `baff8ad2516af557fa271809f5eac43f0f181d3cd976d0c47b1dd99c6536a287`,
production659 remained b77c3ac4 above. Logs
`/tmp/n2-model-controls-oracle-{gcc,gccsan,clangsan}-{build,tests}.log`.
Earlier pending/live descriptions record observation history, not current jobs.

## Final ancestor-enrollment freeze

Lifecycle review found lazy particle admission enrolled only the child condition
scope. Register every owning ancestor first so successful child publication
cannot escape a later outer rollback. The actual generated master/slave fixture
commits the child, then rejects the outer root and verifies exact prior
Current/tracker identities, manager count, cursor and pooled used units.
The direct late-second-module fire oracle remains included, without an extra
fixture-local savepoint. No source transfer, full tick or whole-game claim.

Canonical735 SHA256
`af10a24d1de6621b8f9c821cb9ec874b1d33537fed2267be0eec3793cf67e9a9`;
inclusive742 SHA256
`59b496d97f989690f432409f705181d378a1bfed55d537314320279ff76cb62d`;
production/build659 SHA256
`720e4d520d54a50ec868ef6ef63aabfb872ca96892c6100c4b0cef07878dd74e`.
Builds51110/79378/40493 retired0. Focused4 jobs39912/26086/61661 retired0,
PASS GCC12.51s/GCCsan50.36s/Clangsan29.72s;13 matching normalized manifests
SHA256 `64a22398d19b4194d1c73651186a8436d7def1282c36a7884ff73652fae0171f`.
Root terminal62 and pose terminal4 retain every failure/retry pair.

Final GCC related67 PASS225.38s, job9005 retired0.
Normal-host sanitizer related67 PASS GCCsan910.02s/Clangsan534.90s. Their launch
tool output was truncated during context compaction; exact owned CTest selectors
were recovered from host process inspection. Processes301683/301711 both retired;
complete logs show100% PASS, with no duplicate runs. Every configuration emitted122
normalized manifest rows SHA256
`b200b8e3b29e1f17f473d860a13c68d4fb8612a8366ab54267c57e57a45c646c`.
Logs `/tmp/n2-model-controls-complete-{gcc,gccsan,clangsan}-{build,tests}.log`
and `/tmp/n2-model-controls-complete-related-{gcc,gccsan,clangsan}-tests.log`.
All final source fingerprints rechecked unchanged. ASan/UBSan/LSan enabled with
no suppressions or timeout changes. Official bgfx/bx/bimg pristine verification
passed; this CPU component evidence makes no physical GPU or whole-game claim.
Commit provenance: `git log -1 --format=%H -- evidence/qa/N2-client-bone-control-lifecycle.md`.
This is a verified supporting checkpoint, not slice03 or milestone acceptance.
Full configured500 remains unrun; slice03 and N2 remain incomplete.
