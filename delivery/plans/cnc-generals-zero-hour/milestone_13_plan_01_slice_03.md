# M13 slice 03: localized fallback and retail variant acceptance

## Goal and observable outcome

Resolve selected-locale movie names with explicit fallback, then prove each observed legal Bink variant decodes headlessly through the production VFS path without storing retail content or identifying host paths.

## Scope

- Add deterministic locale-first/fallback path selection with actionable absent-fallback diagnostics.
- Add asset-free fallback/lifecycle acceptance tests.
- Add an opt-in retail test that mounts configured roots, verifies only declared logical representative movies, decodes bounded leading frames, and compares codec/dimensions/audio-rate/frame-order metadata.
- Record the derived variant table and operator invocation without retail bytes, hashes, or physical paths.
- Run all four preset builds and `audio|video|renderer-contract` suites.

## Non-scope

Committing media, hashes or private paths; decoding every duplicate movie; physical presentation or A/V quality; and M14 GPU acceptance are excluded.

## Dependencies and ordering constraints

Depends on slices 01 and 02. This is the M13 acceptance slice.

## Entry point and end-to-end behavior

The resolver receives a logical movie stem and locale preference, opens the first existing authorized candidate, then the production decoder/player runs it. The retail test chooses one logical representative for every observed dimension/audio-rate combination and checks bounded decoded leading frames.

## Data or state transitions

Candidate selection is locale -> neutral fallback -> explicit error. Corpus cases independently transition unopened -> metadata validated -> bounded leading frames decoded -> closed.

## Authorization and permissions

Retail execution is opt-in, read-only, and separately configured through existing `ZH_ENABLE_RETAIL_TESTS` roots/language. Test output contains logical names and derived media properties only.

## Validation, errors, and recovery

Asset-free tests cover locale hit, fallback hit, missing both, case-insensitive VFS resolution, decoder-unavailable injection, truncation, and shutdown. Retail cases must match Bink/YUV420P dimensions and optional stereo Bink audio sample rates; failures name only the logical resource.

## Expected implementation surfaces

Video resolver/corpus APIs, `tests/video/test_video_acceptance.cpp`, `tests/video/test_video_corpus.cpp`, `data/corpus/video-variants.tsv`, CMake retail gating, and `docs/building-linux.md`.

## Positive tests

- Locale-first and neutral fallback selection work.
- Each declared silent/44.1 kHz/48 kHz Bink dimension variant decodes ordered leading frames.
- All canonical preset suites remain green.

## Negative tests

- Missing fallback, corrupt/truncated content, decoder-unavailable injection, excessive dimensions, and early shutdown fail safely and clearly.

## Required validation commands

```sh
cmake --build --preset <preset>
ctest --preset <preset> -L 'audio|video|renderer-contract' --output-on-failure
```

Run for all four canonical presets, plus a separate read-only retail build and its `video` corpus test.

## Acceptance criteria

Fallback behavior and every observed variant class have headless evidence; errors remain actionable and private media/path data is absent from commits.

## Commit boundary

Commit resolver, corpus manifest/tests, docs, acceptance evidence in the plan, and final validation changes as `delivery: M13 slice 03 prove Bink variants`.
