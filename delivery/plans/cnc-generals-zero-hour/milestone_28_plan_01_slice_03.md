# M28 plan 01 slice 03: audio definitions, music, and assurance

## Goal and observable outcome

Original audio settings/event/music tables parse owned definitions and resolve a valid owned music resource through the native VFS/audio provider without opening an audio endpoint. Missing music fails promptly with its logical name and never enters CD search, modal wait, or fabricated-loaded state.

## Scope

- Extract bounded source-owned audio settings, AudioEvent, DialogEvent, and MusicTrack parsing/lookup behavior.
- Resolve logical music through the accepted M12 VFS/audio seams in deterministic null-output mode.
- Preserve layered definition override and duplicate-name semantics required by source behavior.
- Complete M28 source classification, provenance, dependency ledger, provider-removal, lifecycle evidence, sanitizer checks, and QA evidence.

## Non-scope

Audible playback, physical audio-device acquisition, complete GameAudio/GameClient startup, gameplay-triggered spatial audio, cinematics, menu flow, and retail corpus acceptance.

## Dependencies and ordering

Requires slices 01-02 and accepted M12/M13 media components. It is the final M28 slice and owns milestone-wide regression evidence.

## Entry point and end-to-end behavior

The consumer loads layered owned AudioSettings/Music/Sound/Speech definitions, validates references, looks up a music track, probes/queues its owned audio bytes against a null sink, observes the logical event, shuts workers/resources down, and reports counts. Missing, malformed, and unsupported definitions return promptly with actionable diagnostics.

## Data and state transitions

Definition layers parse into a temporary registry, references resolve against the final registry and VFS, then the registry becomes active. Shutdown stops media work before clearing definitions/owners. Failed parses and missing resources never replace a previously valid registry.

## Authorization and permissions

No authorization surface. Owned fixture reads only; no CD enumeration, modal interaction, external process, network, or persistent write.

## Validation and error handling

Positive tests cover valid settings, event kinds, music lookup, layer override, null-output scheduling, prompt shutdown, and repeated lifecycle. Negative tests cover malformed fields/ranges, duplicate incompatible definitions, missing references/music, unsupported types, failure injection, and removal/drift of required providers.

## Expected implementation surfaces

Source-owned audio extraction unit; provider/CMake additions; owned definitions/audio fixtures and tests; checked dependency ledger, classification/provenance/removal tools; durable M28 QA evidence and governing-plan results.

## Required commands

- Focused `original-resources` CTest label in GCC/Clang Debug/Release.
- Complete asset-free CTest in all four presets after stabilization.
- Focused Clang Debug ASan/UBSan with explicit options.
- Source classification, dependency ledger, extraction provenance, identity/live-link, and provider-removal controls.

## Acceptance criteria

- Valid owned music succeeds through a null output; no hardware is opened.
- Missing music returns promptly, names the logical resource, and records no CD/modal/fabricated success.
- Registry/resources/workers/callback counts return to baseline for normal, repeated, and injected-failure shutdown.
- All M28 criteria map to durable evidence; coupled M20 and later owners are named without overclaiming.

## Commit boundary

One commit containing audio provider behavior, tests, assurance gates, plan results, and milestone QA evidence. Outer milestone/delivery status remains orchestrator-owned.

## Result

Complete. `AudioDefinitions` parses the source field vocabulary for AudioSettings, MusicTrack, AudioEvent, and DialogEvent, applies same-kind later-layer overrides, rejects incompatible duplicates, validates every logical resource, and publishes only a complete registry. Valid owned PCM music is decoded/scheduled through the accepted M12 `AudioManager` with a deterministic null sink. Unknown/non-music lookups fail, and missing music terminates in under one second with its logical path and explicit disabled CD/modal policy; observed CD-search and modal-wait counts remain zero.

Cumulative focused runtime, live-symbol, provenance, provider-removal, dependency-ledger/drift, and classification gates pass 10/10 in all four presets. Four audio acquisition failure stages and repeated shutdown restore definition/voice/worker/device counts exactly. Focused Clang ASan/UBSan passes 10/10 with leak detection disabled, and the canonical asset-free suite passes 92/92 in every preset. Full evidence is in `evidence/qa/cnc-generals-zero-hour/m28-original-cpu-resources-2026-09-20.md`; the resulting commit is recorded in the governing plan after creation.
