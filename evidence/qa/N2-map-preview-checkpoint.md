# N2 original map-preview checkpoint — not milestone acceptance

Parent `adf48b9e`; same governing plan01/slice03. N2 remains in progress.
No agents, vendor edits, retail access/edits, or recovery imports. GUI entry
points now compile outside GameSpy-dependent menu code; their real calls remain.

## Frozen source and validation

All configured targets build normal GCC, GCC ASan/UBSan/LSan and Clang
ASan/UBSan/LSan. Sanitizer execution uses the normal host; no suppression or
instrumentation reduction. Seven focused families PASS all3 in
1.09s/8.26s/4.82s. Related89/89 PASS all3 in 18.41s/79.78s/71.98s.

Canonical570 cumulative path/content SHA256 since9e5c0e4f:
`fc26d016de1d840104ac96874c0c52eeb0031cf4ccfcd22d1e8352aac08ba9e8`.
Toolchain-inclusive577:
`b06a60d3ed6c94fd552914d84e6d78d1d45d61d39173005cc5b9bd323fbf05a7`.
Specification remains SHA256
`0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`;
source/planning commits remain ancestors of HEAD. Current full222 suite has
not been run; related checks are not full milestone acceptance.

```sh
cmake --build build/original-core-<variant> -j4 -- -k0
ctest --test-dir build/original-core-<variant> --output-on-failure -R '^original_map_preview_'
ctest --test-dir build/original-core-<variant> --output-on-failure -R 'original_(map|game_info|lexicon|function_registry|startup|service|configuration|runtime|core)'
```

`tests/original/map_preview.cpp` executes actual shared layout/metadata, original
GameInfo/GameSlot apparent-position/alliance and protected File/storage owners.
Independent source-derived Real geometry oracle, sparse/differently sized
controls, collision order, marker list order, malformed count/extent/coordinates/
dimensions, output overflow/alias rejection, destination-vs-source presence,
duplicates, observers, local/team/opponent random visibility and absent identity
are checked. Constructors receive actual required services; warmed source pool
metadata is admitted before live ownership baselines, not treated as a leak.
Three complete owner iterations follow warming; global publications restore.

All discovered allocation/I/O failure ordinals retain failure/retry pairs and an
exact success terminal across all3 configurations and three complete lifetimes:
layout5, copy allocation21, copy storage7. Source File, output/temp descriptors
and heap ownership retire exactly. Copying streams150000 generated opaque bytes
through multiple blocks, preserves prior output before publication and leaves
no temp files. Post-rename directory-sync failure is accepted complete published
state, not alleged rollback. Generated source bytes remain unchanged by copying.
This is not TGA decode or image/render acceptance.

Temporary logs `/tmp/zh-map-preview-<variant>-{final-build,frozen-tests,related-tests,root-link}.log`.
Initial include-order, missing fixture FileSystem, expected float-truncation,
over-restrictive maximum-dimension expectation and cold pool-baseline fixture
errors were corrected together with dependency findings; no product assertion
or acceptance criterion was suppressed. One prematurely invoked suite could not
run because its fixture had not built; it supplies no evidence. A rejected patch
context made no mutation. Earlier failed attempts are retained in temporary logs.

## Source dependency findings and pending limits

Actual scalar visibility previously shared GameInfoPresentation.cpp with
translated template-name metadata. A link-map diagnostic traced sanitizer
admission from that metadata through PlayerTemplate/Money/Player/audio/GUI.
The scalar getters/shared alliance predicate are now separate actual source
providers; translated names and their real providers remain compiled. The old
helper fetched a negative local slot before checking it; identity validation now
precedes lookup. MultiplayerSettings publication is separate from INI metadata.

An attempted null-view GUI fixture retains the real GameWindow callback closure
under sanitizers and cannot link before the pending GUI providers. It is not part
of the accepted CPU/storage fixture. Real nonnull GUI binding, window/MapCache/
ImageCollection lifetime retirement and fallible gadget publication are pending.
No fake manager, returning callback stub or sanitizer workaround was admitted.

Never-executed original-runtime root probes still fail4/6/13 unique symbols.
Normal: HideDiplomacy, PopulateInGameDiplomacyPopup, ResetDiplomacy and native
testMinimumRequirements. GCCsan additionally exposes real DisconnectMenu/LANAPI
RTTI. Clangsan additionally requires IME, warning UI, briefing/ToggleDiplomacy,
terrain/skybox and hosting status. Different instrumentation closure means counts
alone cannot establish progress or acceptance. The actual map providers are no
longer undefined, but startup still does not link or execute.

Continue the complete diplomacy/capability and native boundary cohort, then
actual headless factories, startup/world and compiler-matching scenario fixture.
Default GUI callback providers, physical rendering and whole-game acceptance
remain separate gates. N0/N1 are accepted; N2/slice03 and N3–N7 are not complete.
