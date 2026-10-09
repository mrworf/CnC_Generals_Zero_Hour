# N2 original diplomacy checkpoint — not milestone acceptance

Parent `55f23984`; existing governing plan01/slice03. The user requested sorting
and committing the pending work. Its 18 original pending paths were all owned by
this cohort: original source, generated fixtures, build registration and plan.
No unrelated work was staged. Ignored build/cache outputs, the retail symlink
and recovery archive remain ignored and untouched. No agents or vendor changes.

## Frozen source and validation

All configured targets build normal GCC, GCC ASan/UBSan/LSan and Clang
ASan/UBSan/LSan. Seven briefing/disconnect families pass in the related matrix on
all three; the related matrix contains96 tests. Sanitizer execution uses the
normal host with leak detection and instrumentation intact. No suppressions,
timeout extensions, fake GUI or fake transport providers are admitted.

Canonical581 cumulative path/content SHA256 since9e5c0e4f:
`2708266f4b85f6f4b064553838f6ea48bd8bf46842c84970246f8b4ed4ffb944`.
Toolchain-inclusive588:
`b4387b4ab456b68ea44f14e1dfe6704bcac10d44dfd5a2add340862984f621b4`.
Digest encoding is sorted path bytes, NUL, binary SHA256 of each file's bytes.
Specification SHA256 remains
`0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`.
The configured full229 suite is not run; these are proportionate checkpoint
checks, not full N2 acceptance.

```sh
cmake --build build/original-core-<variant> -j4 -- -k0
ctest --test-dir build/original-core-<variant> --output-on-failure -R 'original_(diplomacy|map|game_info|lexicon|function_registry|startup|service|configuration|runtime|core)'
cmake --build build/original-core-<variant> --target original_runtime_link_probe -j4
```

The last command is a failing link-only diagnostic and is never executed.
Temporary logs: `/tmp/zh-diplomacy-<variant>-commit-{build,related,root-link}.log`.
Related96/96 PASS normal GCC18.40s, GCCsan81.12s and Clangsan72.95s. The seven
diplomacy cases within these runs account for0.16s/1.53s/0.98s respectively.

## Executed behavior and source findings

The actual GetBriefingTextList/UpdateDiplomacyBriefingText owner retains stable
container identity, order, case-sensitive duplicates and clear-plus-add behavior.
Tests exercise attach/detach/rebind, no destructor callbacks, rejection of
conflicting/reentrant calls, partial observer failure without model publication,
full repair on corrected retry and actual global entry points with explicit
world-state clearing. The observer is not a GUI acceptance substitute.
All allocation prefixes and corrected retries retire exactly across three
complete owner iterations, with terminals append6, clear2 and attach2 all3.
Actual DisconnectMenu initialized pre-init state and genuine source RTTI execute;
compile-time checks preserve the abstract LAN interface contract.

Actual Diplomacy.cpp is compiled without the explicitly excluded GameSpy buddy
service. Source GUI acquisition/reset captures legitimate parent identity, retires
animation before windows and drains deferred destroy callbacks. Observer color
index and reoccupied-row visibility correct original source bugs. Nonnull GUI,
fallible gadget callbacks and physical audiovisual behavior are not accepted.

Animation construction guards all eight actual helper acquisitions. Registered
window admission guards pooled ownership/list insertion and restores prior
position/timed duration on failure. The separate actual constructor fixture
passes terminal8 and every failure/retry pair across three lifetimes on normal
GCC and GCC sanitizers. Clang's fixture fails to link on missing native providers;
it remains EXCLUDE_FROM_ALL and is not registered with CTest. Neither cross-
compiler animation ownership nor registered-window lifecycle is accepted.
Initial fixture include-order/self-contained-header compilation defects were
corrected; their failed attempts remain in temporary reviewed-link logs. No
assertion was removed or acceptance criterion weakened to obtain a pass.

TheLAN now borrows existing LANAPIInterface rather than the Windows concrete
owner; LookupPlayer/GetLocalIP remain pure required queries. This supplies no
native sockets, packet handling, LAN implementation or network acceptance.

## Pending gates

Startup root probes fail1/1/7 unique symbols (normal/GCCsan/Clangsan). All need
native testMinimumRequirements; Clang additionally needs IME, warning UI,
terrain/skybox and hosting-status providers. Symbol counts describe differing
instrumentation closure, not proof of startup. Actual original factories,
GameEngine startup, world/scenario execution and compiler-matching gameplay
remain required. N2/slice03 remains in progress; N3–N7 remain pending.
