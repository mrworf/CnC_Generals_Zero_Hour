# M22 plan 01 slice 08A: private-safe retail recording boundary discovery

## Goal and outcome

Require the real original factory to select a `RecordingGpuDevice` only when
the explicit host/test selector accompanies `ZH_M22_ORIGINAL_FACTORY_PROFILE`.
Keep the default factory on public bgfx.  On a separately provisioned,
read-only campaign or skirmish startup, report the exact *enumerated aggregate*
terrain-configuration mask at the existing fail-closed W3D terrain guard, then
reject before map/scene consumption.  This is a coherent prerequisite for
slice 08, not retail rendering acceptance.

The aggregate mask has no retail-derived string fields: bit 0 means existing
terrain ownership, bit 1 unsupported track count, bit 2 shadow volumes, bit 3
shadow decals without the already accepted profile, bit 4 cloud plane, bit 5
invalid enabled-water shape/type, and bit 6 invalid disabled-water residue.
Only the integer mask is observable.  No source/root path, logical filename,
archive member, byte, hash, model name, screenshot, or raw command label is
printed, committed, or retained.

## Scope and non-scope

Included: Recording-versus-bgfx factory selection, color/depth target setup
through the existing `OriginalGpuEdge`, aggregate Recording operation counts,
strict selector validation, source-owned factory identity/reverse teardown,
the read-only retail configuration audit and output-redacted Python fixture.

Excluded: admitting a newly observed configuration, map load, campaign or
skirmish draw, device pixels, retail model/material inventory, raw Recording
snapshots, resource labels, VFS changes, or private-data copying.  Those remain
for 08/09 after the reported mask has an explicitly implemented source route.

## Entry, state and failure contract

`ZH_M22_RECORDING_FACTORY_PROFILE=1` is valid only with
`ZH_M22_ORIGINAL_FACTORY_PROFILE=1`; it conflicts with device-only mode and
cannot alter an unselected startup.  The selector owns a temporary recording
device and targets; after `GameMain`, all original factory aliases must be
gone, targets are destroyed, the edge/device are released, and zero Recording
resources remain.  A forced factory-device failure or invalid selector must
leave no aliases/resources.  Two fresh recording generations prove recreation.

The retail audit requires both supplied roots, independent writable XDG state,
and the pre-existing authorized campaign/skirmish logical map selections.  It
snapshots read-only metadata before/after without printing it.  Both runs must
emit a nonzero aggregate mask and fail at the native guard before a scenario
success marker; the test emits only fixed sanitized outcome text.

## Implementation surfaces

- `src/original_runtime/linux_main.cpp`: private-safe Recording selection,
  operation-count/zero-resource marker and selector/rollback checks.
- `include/zh/renderer/recording_device.h` and `src/renderer/recording_device.cpp`:
  label-free aggregate operation-count API.
- `GeneralsMD/.../W3DTerrainVisual.cpp`: audit-only configuration-mask marker
  immediately before its unchanged fail-closed guard.
- `tests/original_rendering/test_w3d_retail_recording_audit.py` and
  `CMakeLists.txt`: read-only, output-redacted retail audit registration.
- Governing index, dependency ledger, and 08A evidence.

## Tests and acceptance

Positive: two Recording factory generations preserve one original
display/view/terrain publication, expose only label-free aggregate command
counts, and retire all Recording resources; absence preserves existing bgfx
factory behavior.  Negative: missing/invalid selector, forced device failure,
and provider removal reject without stale ownership.  Retail campaign and
skirmish audit runs retain metadata, yield the same nonzero enum mask, produce
no scenario success marker, and surface no private material.

Run focused Recording/factory and source identity/provider controls, the
private retail gate in the separate configured build, GCC/Clang and sanitizer
asset-free coverage, proportional six configured non-GPU/non-LAN suites,
host leak/Vulkan/LAN checks where applicable, ledger and whitespace-diff
checks.  Record only sanitized results, stage only owned paths, preserve the
unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic, and commit one
independent 08A slice.
