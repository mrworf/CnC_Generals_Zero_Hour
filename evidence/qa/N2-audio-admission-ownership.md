# N2 coupled audio admission and queue ownership

Status: frozen focused7/related72 PASS all3. Supporting N2 plan01/slice03
cohort, not slice03 completion, N2 acceptance, physical playback, or full-game
parity. Parent cleanup commit: c6d2fddbc8f0e2060b3efe1a0e713cf64c64d520. Governing contract:
delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01_slice_03.md.

## Scope and source behavior

Actual original AudioManager admission, MusicManager play/stop, SoundManager
accept/reject, pooled AudioRequest retirement/transfer and AudioManager constructor,
parsed initialization and destructor. Guard every candidate through fallible
helpers/append; preserve accepted policy rejection's handle/RNG effects. Nested
scopes retain ancestor snapshots of caller info/index, handles, both source RNG
streams, queue boundary and sample counters. Fix source rejection's dangling
pointer and localization's doubled separator. No dependency/vendor/asset change.

The fixture has real parsed generated GameData, explicit separate user storage,
original audio settings/events and Music/Sound managers. A zero-channel device
does not fabricate playback. Generated interrupt policy permits a pending Play
request according to original source; consumption/output is not exercised.
Music files are generated existence sentinels, not a decoding oracle.

External dynamic ambient metadata registration/collision and its Object-map/
Drawable-transfer callers are not exercised by this parsed-audio fixture.
Their distinct caller-owned failure cleanup is recorded in the format notes
and existing slice03 carry-forward contract; do not infer whole-frame metadata
rollback, saved ambient restoration or real-map acceptance from this cohort.

## Frozen source

- Canonical source 744 files: 5c67690c830b3ab7dedab82ad5aed71afc8b698e871ea053ce2cc6939b35586c.
- Toolchain-inclusive 751 files: c93ea696f7e5d235fbbcff817f0535fb9e1843bbd74fe75437bba3282adaf544.
- Production/build 667 files: 805fc796eb7acf2286eac409dc30edd93c282c2ad23c31bfddbf018d85fd0d79.
- Source union baseline and fingerprint procedure follow the existing shared
  attachment QA contract; documentation/checkpoint patch is not executable input.

## Validation contract and current results

Focused selector: `^original_audio_admission_` (7 tests).
Related selector:
`^original_(object_ownership_|drawable_queries_|native_warning_|core_random$)`
(72 tests). Build every selected fixture from frozen source before execution.
CTest is verbose, serial; existing 60-second family limits unchanged. GCC and
Clang sanitizer runs use the normal host with address/undefined/leak checks
enabled. Every fault prefix retains its same-owner corrected retry; discovery
and each prefix enter identical cursor/RNG/caller/configuration state.

- Final lifetime-aware frozen focused: GCC PASS7/7,14.69s,job25853 retired0;
  Clang sanitizer PASS7/7,38.58s,job30153 retired0;
  GCC sanitizer PASS7/7,60.35s,job34805 retired0.
- All3 normalized focused manifests match exactly: 12 rows,
  fa769ac96482ad3be8d15ad39b7c3236caf8960480921c5c47ea2b02bddb869f.
- Related72: GCC PASS72/72,289.04s,job24720 retired0;
  Clang sanitizer PASS72/72,681.91s,job49908 retired0;
  GCC sanitizer PASS72/72,1158.17s,job18245 retired0.
  Build jobs5350/35313/1314 retired0; no live validation jobs remain.
- GCC related normalized manifest: 153 rows,
  24337b36a4b59bd076ee82ceee00449d798e5448aec248b8e84fb6df820cbf30.
  Both sanitizer related manifests match exactly.
- Ordinary and logical admission terminals: 179 each.
- Full constructor/parsed-audio startup terminal: 231.
- Pooled AudioRequest exhaustion exercises music, sound, direct MusicManager
  admission and stop request callers; corrected retry and used-unit retirement.
- Full configured milestone suite: 509 tests, not run; actual simulation gate remains open.

Logs: `/tmp/n2-audio-admission-{gcc,gccsan,clangsan}-final-{build,tests}.log`
and `/tmp/n2-audio-admission-{gcc,gccsan,clangsan}-related-tests.log`.
GCC related rebuild: `/tmp/n2-audio-admission-gcc-related-build.log`.
Lifetime-aware final logs use the `lifetimes-{build,tests,related-tests}` suffix.
All three source fingerprints rechecked after final execution and match exactly.

## Retained failure/retry history

1. Initial GCC build retired1: test include used Version.h instead of original
   lowercase version.h. Corrected build exposed missing direct special-handle
   header; both are fixture compile defects, not production behavior changes.
2. First executable focused6 retired8 in shared original localization startup,
   before admission assertions. Redacted source backtrace proved the doubled
   separator join. Census of related audio path helpers preceded the common leaf
   correction; strict VFS validation was not relaxed. Corrected focused6 PASS.
3. Added path/policy oracle compile retired1 for unsupported string find and
   missing direct AudioAffect header. Corrected fixture headers/string inspection;
   final focused6 PASS as above. No test, sanitizer or timeout was weakened.
4. First freeze canonical1a7758e0/inclusived51a3534/productionc8f0ffd5 passed
   focused6 all3 (GCC12.81s/GCCsan55.20s/Clangsan33.59s; jobs35957/6163/21289
   retired0). Related jobs58997/41927 intentionally retired130 after original
   FX stack-local caller census identified expired ancestor snapshots. Logs are
   preserved; this is source-driven supersession, not a timeout/retry shortcut.
   Final cohort adds nonallocating caller retirement tracking, nested expired
   caller, explicit same-address lifetime reuse, and truly const caller tests.

All exploratory logs remain `/tmp/n2-audio-admission-gcc-*.log`.
Pristine upstream verification passes for bgfx
cca91681c953d2de9531197b0f580c866ffaa775, bx
d86e4ea9d9da6e832a3ff41398587d82b772c69b and bimg
101b5b5fd4670f82cfdec8e98aa1ab9ee93bb2a1. This CPU cohort makes no GPU claim.

Commit locator:
`git log -1 --format=%H -- evidence/qa/N2-audio-admission-ownership.md`.
