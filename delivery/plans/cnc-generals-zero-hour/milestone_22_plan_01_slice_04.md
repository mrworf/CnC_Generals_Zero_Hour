# M22 plan 01 slice 04: complete retail scenes on recording

## Goal and observable outcome

The accepted retail campaign and skirmish scenarios traverse the same production original rendering entry point and complete representative terrain, camera, object, lighting, fog/shroud, shadow, particle, water, and effects families on the recording device.

## Scope

- Add every newly reached original producer and ledger operation exposed by the two bounded real scenes; do not substitute component/generated scene implementations.
- Record aggregate/private-safe family, producer, command, ownership, and bounded timing evidence.
- Exercise reset/re-entry and device-resource invalidation between scenario families.

## Validation and error handling

- Required-family assertions reject omitted/no-op producers, placeholders, missing assets, malformed assets, unsupported material/effect state, partial frames, and incomplete teardown.
- A failed required scenario asset/reference load unwinds resources and newly published scene assets, then a corrected retry succeeds; the original `WW3DAssetManager` intermediate-publication contract is not globally changed.
- Runtime identity and provider-removal gates prove original terrain/object/effect producers and the slice 03 translation chain.
- Retail roots remain read-only; recursive metadata is compared before/after and no private path/name/byte/hash is committed.

## Acceptance criteria

- Both real scenes complete all required families through actual original producers on recording.
- Every newly reached dependency is implemented or explicitly classified as optional and not reached; required state cannot be ignored.
- Normal, failed, reset, and re-entry runs end with zero original and device resources.

## Commit boundary

Commit retail recording integration, private-safe gates, ledger update, slice status, and evidence as `delivery: M22 slice 04 record original retail scenes`.
