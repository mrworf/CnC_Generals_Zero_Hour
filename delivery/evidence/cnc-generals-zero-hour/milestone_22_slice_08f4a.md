# M22 08F4A: map-owned terrain shroud notification

Parent checkpoint: `1c782d1091fa371f810face2f6711928373332ca`.
The CPU `BaseHeightMapRenderObjClass::notifyShroudChanged` now accepts only
the published, map-loaded terrain owner attached to the primary display scene
under an active original GPU edge and intact display providers. It requires
an initialized shroud and no prop buffer. The native method only forwards to
an optional prop buffer; with no CPU prop producer, the bounded source result
is a no-op. A forced non-null prop pointer rejects without dereference.

The generated scene-attachment probe checks pre-init, detached and foreign
scene, missing published terrain/asset/scene provider, malformed prop, released
map, retry, repeated notification, two generations and unchanged Recording
resource counts. No display cell dispatch, construction selector, retail
provider/content traversal, private data or source outputs were added. The
source digest and behavior description were updated in the dependency ledger.

## Acceptance

- Six complete configured builds passed.
- All six canonical non-GPU/non-LAN/non-retail suites passed **260/260** each
  with `-LE 'gpu|lan|retail'`. The sanitizer suites used
  `ASAN_OPTIONS=detect_leaks=0` for the established sandbox ptrace
  restriction. No CTest registration changed.
- Physical public Vulkan validation-layer display-owner/factory controls
  passed **2/2**; these are unchanged-device controls, not a new frame claim.
- Serial host-loopback LAN passed **4/4** in all six configurations.
- Strict host `ASAN_OPTIONS=detect_leaks=1` terrain notification and accepted
  generated scene-boundary focus passed **2/2** with GCC and **2/2** with Clang.
- GCC Debug source terrain notification, accepted generated scene boundary,
  presentation identity/provider-removal and three dependency-ledger gates
  passed **7/7**. `git diff --check` passed.

The unrelated renderer diagnostic remains unstaged. 08F4B can implement the
separate display shroud-clear/per-cell caller; 08F construction remains
pending that prerequisite.
