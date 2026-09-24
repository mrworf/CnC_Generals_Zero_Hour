# M22 08F3: original water map-override owner

Parent commit: `f40ad4b7d23de883385e8b01bbe9b2bb8b722dcd`.
The CPU `WaterRenderObjClass::updateMapOverrides` now returns only for the
exact published, scene-attached, resource-ready active-water owner with its
grid disabled, source configuration unchanged, and no river texture. This is
the native optional no-river branch: no water-transparency setting is read,
no asset provider is opened, and no Recording resource or alias is added.
Absent/foreign provider, detached scene, pre-init, pending buffer acquisition,
disabled or malformed active-water state, enabled grid, and a forced non-null
river texture reject. Unsupported active-river replacement remains pending.

The generated read-only terrain-water source probe calls the method twice on
a map-loaded owner and after a failed buffer acquisition/retry. It checks
provider removal and detachment. Two additional independent generated owner
generations exercise the malformed/unsupported states and end with zero
published water owners and Recording resources. The accepted 08F0 generated
scene-boundary test still passes. No 08F selector, terrain-load water
reattachment, retail provider/content traversal, private identifiers, paths,
bytes, hashes, images, or raw process output were introduced.

## Acceptance

- Six complete configured builds passed.
- GCC and Clang Debug focused terrain-water source probes passed **1/1** each.
- All six canonical non-GPU/non-LAN/non-retail suites passed **260/260** each
  with `-LE 'gpu|lan|retail'`. The sanitizer suites used
  `ASAN_OPTIONS=detect_leaks=0` for the established sandbox ptrace restriction.
  No CTest registration changed; count remains **260**.
- Strict host `ASAN_OPTIONS=detect_leaks=1` terrain-water and accepted 08F0
  generated-boundary focus passed **2/2** in GCC and **2/2** in Clang.
- Physical public Vulkan validation-layer display-owner/factory controls
  passed **2/2**; they are unchanged-device controls, not an 08F frame claim.
- Serial host-loopback LAN passed **4/4** in each of six configurations.
- GCC Debug terrain-water, accepted 08F0 boundary, presentation identity,
  provider-removal, and three dependency-ledger focused gates passed **7/7**.
  The water source digest and owner description were updated in the ledger.
  `git diff --check` passed.

The unrelated renderer diagnostic remains unstaged. 08F can resume at its
source terrain-load water reattachment/order boundary and probe later
construction; this slice does not claim that transition.
