# M22 slice 08P0C2D0A: bounded Recording transactions

Parent: `1ff903ad04a313f2adfaff34daa80be6f39964d8`.
Status: independently accepted on the refrozen source below; physical device
transactions and engine source-stage wiring remain deferred.

## Ownership and generated controls

Optional public capability defaults fail closed. Recording admits only a
bounded, nonnested idle/preparation or frame journal from an idle device.
Tokens bind device/sequence/caller generation/mode. Admission copies the
bounded resource/payload baseline before publishing its owner; finish is
allocation-free, no-throw and exactly once. Partial commands remain private
until commit. Abort restores prior slot identities/payloads/initialization,
command prefix, indexed bytes, target/phase/view budget and normalized counters.
Candidate slots retain retired generations; faults stay consumed. Ordinary
nontransactional behavior remains covered without modification.

The generated isolated witness runs two complete device generations. It covers
all five candidate resource types; rollback of prior releases and identity;
old buffer/texture/index bytes; uninitialized and initialized target rollback;
prior nonzero view budget through candidate presentation; 11 ordered operation
fault boundaries through release; six existing create/upload faults; clean
retry with exactly one pass/draw/present; nested/cross-mode/malformed/stale
tokens and exactly-once finish; unsupported ordinary-only consumer; active
pass rejection; command/resource/byte/view capacity+1; checkpoint failure;
incomplete-pass commit rejection; active owner destruction.
Attached-target retirement rejection and an injected diagnostic-allocation
failure prove commit is poisoned before formatting, while abort restores exact
identity/resources. The outer operation guard also covers argument formatting
that could throw before reaching the common diagnostic function. This confirmed
owner correction supersedes all earlier build/focused/host results; complete
final acceptance was repeated on the refrozen source below.

Fixture correction: live-count eight was not a valid total-slot bound after a
prior abort had retained a tombstone. Exact resource boundary now uses a fresh
equivalent scene, preserving retirement semantics and all original assertions.

## Frozen hashes

- `include/zh/renderer/contract.h`: `64607ef12f36805d22efff49805125de0d690304163ab73986d376b18c4bb700`
- `include/zh/renderer/recording_device.h`: `87a0e1304ccdced85cba7728dba2b3f5284c9bddd3fd5f831398e015ba53361b`
- `src/renderer/recording_device.cpp`: `4474f287615b2f7b4b48d38ebb8b84ed528325e86922d7f9b3e083e6a3e19dbb`
- `tests/renderer/test_recording_transaction.cpp`: `030f63c68d0dadb75c825832dd2fe040822d24c10561acbd44c9175854f9dbbe`

## Acceptance

Earlier native/sanitizer focused and host checks passed, but are superseded by
the diagnostic-allocation correction, not credited as final gates. All six
complete builds pass on corrected source. Clang sanitizer's explicit final
build completed all 806 recovery work items after the known premature
dependency-log warning, with no actionable source error. No source/test edit
followed refreeze; the recorded hashes are unchanged after the final gates.

| Configuration | Canonical result | Total seconds | Terrain seconds |
| --- | --- | ---: | ---: |
| GCC Debug | 270/270 | 278.58 | 60.07 |
| Clang Debug | 270/270 | 263.09 | 59.24 |
| GCC Release | 270/270 | 147.89 | 24.86 |
| Clang Release | 270/270 | 99.96 | 23.80 |
| GCC ASan+UBSan | 270/270 | 875.83 | 266.04 |
| Clang ASan+UBSan | 270/270 | 690.30 | 222.48 |

Canonical command is `ctest --test-dir build/<preset> -LE 'gpu|lan|retail'
--output-on-failure`; sanitizer runs use `ASAN_OPTIONS=detect_leaks=0`.
All six ran serially after build load cleared. No failure, sanitizer finding
or timeout occurred.

Exact focused regex is
`^(renderer_recording_transaction|renderer_recording_device|original_w3d_first_gpu_edge)$`:
GCC/Clang native each 3/3 (0.08s each), GCC sanitizer 3/3 (0.33s), Clang
sanitizer 3/3 (0.26s). Exact strict host LSan uses only
`ASAN_OPTIONS=detect_leaks=1`, no UBSan override, selecting
`^(renderer_recording_transaction|renderer_recording_device)$`;
GCC 2/2 (0.11s), Clang 2/2 (0.10s). Both were host-escalated and serial.

Established host physical Vulkan controls pass with validation enabled:
GCC display/bootstrap/map 3/3 (5.36s), Clang display/map 2/2 (4.18s).
No error/VUID category occurs. These unchanged controls preserve prior
physical acceptance; D0B still owns actual native device transactions.
Exact serial host LAN command is `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir
build/<preset> -L lan -j1 --output-on-failure`: 4/4 all six, respectively
1.71s, 1.71s, 1.70s, 1.71s, 1.81s and 1.78s in the table order.
Public-header neutrality, original dependency ledger and diff checks pass.

## Closeout boundary

Review confirms bounded no-throw swap/retirement, fully initialized new
aggregate members, field-only comparisons and correct exception poisoning.
Exact owned staging excludes the unrelated renderer diagnostic. Delivered
against the recorded parent by
`delivery: M22 08P0C2D0A own Recording frame transactions`.
Next is D0B public bgfx idle/frame transactions; no engine stage/shroud, tree
frame advancement/draw, factory, retail admission or private input is added.
