# Coverage Matrix: Zero Hour R07 qualification

> Source PRD: [zero-hour-linux-port-plan.md](zero-hour-linux-port-plan.md)
> Normative amendment: [headless-qualification.md](../product/headless-qualification.md)
> Generated: 2026-10-10T17:11:40.167953+00:00 (UTC)
> Evaluation scope: bounded amendment PRD plus inspected source; no runtime execution.

## Status key

Verified means inspected repository evidence for that cell, not a fresh runtime pass.
Specified is authoritative desired behavior. Gap (implementation) means inspected missing
integration; Unverified means unexecuted or insufficient evidence. Gap (spec) would prevent
definition readiness. N/A requires a stated reason. Nothing is implied by an empty cell.

## Coverage matrix

| Capability | User Role | UI or Entry Point | Backend/API | Data/State | Authorization | Error States | Tests |
|---|---|---|---|---|---|---|---|
| CAP-HQ-01 — native entry (HQ-01) | Local developer/runner | Specified: documented native executable, explicit roots/seed/ticks; Gap (implementation): GAP-HQ-01 | Verified: GameEngine.cpp::init/update; Gap (implementation): concrete native root, GAP-HQ-01 | Specified: prepared→initialized→loaded; real original root | Specified: normal OS user; read-only supplied roots, separate writable user storage | Specified: missing roots/required configuration fail with stage/category; corrected retry | Specified: actual entry call trace, no link-probe substitution; Unverified: integrated execution |
| CAP-HQ-02 — generated world (HQ-02) | Local developer/runner | Specified: H1 fixture selection at native entry | Verified: GameLogic.cpp::startNewGame; logical_world.cpp proves generated map chunks, not full startup | Specified: 65×65 varied terrain, original sides/scripts, persistent and DeletionUpdate actors | Specified: generated input only; no private asset-derived output | Specified: malformed INI/map/provider reject; no fake successful setup | Specified: both actors loaded, script executes, 120 updates; Unverified: integrated H1 |
| CAP-HQ-03 — deterministic observation (HQ-03) | Automated runner | Specified: after load and each steady update | Verified: GameLogic.cpp::update, DeletionUpdate.cpp, RandomValue.h; observer codec Unverified | Specified: 121 records/lifetime; frame/RNG/object/owner/scheduler/terrain state | Specified: observation cannot mutate simulation or expose private roots | Specified: differing bytes, missing semantic witness or early termination fail | Specified: semantic oracle plus GCC/GCCsan/Clangsan comparison and three lifetimes; Unverified: matrix |
| CAP-HQ-04 — logical provider boundary (HQ-04) | Port developer | Specified: native factories at original init/load/update | Verified: NativeHeadlessGameClient.cpp; runtime source graph and Weapon.cpp query coupling | Specified: required logical backing versus source-supported absence/deferred output | Specified: no vendor/private-API patch or silent provider substitution | Specified: unsupported requested behavior rejects, never successful placeholder | Specified: provider/call manifest; no GPU/playback claim; Unverified: whole factory closure |
| CAP-HQ-05 — failed lifetime and corrected retry (HQ-05) | Automated runner | Specified: fault/malformed input variants through same entry | Verified: component fault fixtures in tests/original; Unverified: complete root retry | Specified: failed owner/world retires; fresh corrected startup; no universal tick rollback invented | Specified: no weakening owner/thread/asset constraints | Specified: missing/corrupt/late failure; non-success plus complete cleanup; catchable interruption | Specified: exact terminal/failure-retry/pool exhaustion, both sanitizers; Unverified: composed root acceptance |
| CAP-HQ-06A — supply original data (HQ-06) | Asset owner | Specified: local explicit roots; no repository upload | Verified: tools/run_data_audit.py supplies redacted integrity boundary | Specified: immutable original files; independent generated fixture/user storage | Specified: owner-provided local files; never redistribute derivatives | Specified: missing content remains external acceptance blocker | Verified: historical N2-original-data.md indexing probe; Unverified: current root completeness |
| CAP-HQ-06B — retail startup and storage (HQ-06) | Local developer/runner | Specified: same native entry with supplied roots; separated user/cache storage | Verified: GameEngine.cpp plus NativeUserStorage boundary; Gap (implementation): GAP-HQ-01 | Specified: original initialization→post-init→retirement; cache fallback; original assets unchanged | Specified: no asset writes/private selectors or raw input digests in tracked output | Specified: cache miss/corruption/unwritable cache falls back; required user-write failure is explicit | Specified: redacted init result plus before/after integrity; Unverified: whole retail init |
| CAP-HQ-07 — evidence and qualification (HQ-07) | Maintainer | Specified: frozen acceptance record | Specified: actual original process, compiler matrices and existing applicable regression suite | Specified: revision/configuration IDs, generated input/call/provider manifest, no inherited stale pass | Specified: generated public evidence only; no raw private assets/identifiers | Specified: timeout/incomplete/mismatch is not PASS; no full-game claim | Specified: HQ-01–06 all pass plus existing N2 constraints; Unverified: acceptance execution |

## Explicit gaps

| Gap | Capability | Surface | Type | Evidence | Required resolution |
|---|---|---|---|---|---|
| GAP-HQ-01 | CAP-HQ-01,06B | Native concrete entry and composed startup | Implementation | cmake/OriginalRuntime.cmake::original_runtime_link_probe is EXCLUDE_FROM_ALL; tests/toolchain/original_runtime_link_probe.cpp never calls initialization; source subclass census finds Win32GameEngine only | Implement actual native root/entry and qualify HQ-01–07 in later delivery; no new product choice |

Search scope: GeneralsMD/Code/GameEngine and GameEngineDevice, cmake/OriginalRuntime.cmake,
tests/original and tests/toolchain; concepts GameEngine subclasses, native entry, startup
callers and composed world fixtures. Negative subclass search is corroborated by the explicit
link-only probe and current delivery state; it is not the sole proof of missing integration.
No specification gap was found in HQ-01–07 after the accepted technical resolution.

## Inherited baseline trace (outside this amendment's detailed review)

| Baseline requirement | Existing authoritative outcome and unchanged acceptance boundary |
|---|---|
| R01 | Recovery and fresh baseline: N0; no archive access or new reset |
| R02 | Upstream-clean dependencies: N1/N7 plus every relevant provider |
| R03 | x86-64 GCC/Clang Linux: HQ-03/07; distribution matrix remains N7 |
| R04 | User-provided immutable assets: HQ-06; no redistribution |
| R05 | Gameplay/audio/visual/format preservation: HQ-02–04 constraints; full parity remains later |
| R06 | Stock renderer physical gate: existing N1 acceptance, not repeated by H1 |
| R07 | Fully expanded by CAP-HQ-01–07 above |
| R08 | Original world rendering: N3, no parity assertion from headless qualification |
| R09 | Original input/UI/fonts/locale/audio/movies: N4, unchanged original experience direction |
| R10 | Campaign/skirmish/user maps/save/load/replay: N5, not replaced by H1 |
| R11 | Linux LAN lifecycle and synchronized play: N6 |
| R12 | Clean installation/build/package/upgrade: N7 |

## Unmapped or uncertain PRD statements

None within HQ-01–07: entry, workload, oracle, provider boundary, lifecycle negatives,
retail probe and evidence all map above. Broader baseline detailed feature decomposition
is outside this amendment; its absence here must not be interpreted as full implementation coverage.

## Coverage summary

8 capabilities/rows; 0 specification gaps; 1 implementation gap. Unverified-cell count:
9. Zero unmapped amendment requirements.
All 7 HQ requirement sections and all 12 baseline IDs have explicit trace. Implementation gaps
and unexecuted acceptance are delivery obligations; they do not invalidate a complete definition.
