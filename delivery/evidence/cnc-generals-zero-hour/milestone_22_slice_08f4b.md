# M22 08F4B: display shroud refresh dispatch

Parent commit: `20f661a5bad94862e900de42e2fa9100f89a8f35`.
The CPU `W3DDisplay::clearShroud` now preserves the native no-op for an exact
initialized, map-owned display/terrain/shroud provider. The CPU per-cell
setter maps the three native logical categories to configured alpha, writes
the existing `W3DShroud` cell, then calls the accepted terrain notification
once. Invalid category, out-of-range cell, no map, detached/foreign display
or terrain, removed provider, or malformed prop owner reject before mutation.

To preserve that fail-before-mutation guarantee without an extra callback,
the terrain owner exposes a CPU-only side-effect-free readiness predicate.
`notifyShroudChanged` uses the same predicate; native class layout and the
single post-write notification order are unchanged. No active prop path,
border setter, frame, construction selector or retail traversal was added.

The generated read-only map/scene probe verifies repeated clear leaves the
grid untouched; shrouded/fogged/clear category values at first and final
cells; invalid cell/category; missing display, terrain, asset and scene
providers; forced malformed prop leaves a written cell unchanged; retry;
two generations and zero Recording resources after teardown. The dependency
ledger tracks both source owners and their CPU-only readiness declaration.
No private retail data or raw process output is present.

## Acceptance

- Six complete configured builds passed.
- All six canonical non-GPU/non-LAN/non-retail suites passed **260/260** each
  with `-LE 'gpu|lan|retail'`. The sanitizer suites used
  `ASAN_OPTIONS=detect_leaks=0` for the established sandbox ptrace
  restriction. No CTest registration changed.
- GCC and Clang Debug focused generated scene/ledger gates passed **4/4** each.
- Strict host `ASAN_OPTIONS=detect_leaks=1` generated display shroud and
  accepted scene-boundary focus passed **2/2** with GCC and **2/2** with Clang.
- Physical public Vulkan validation-layer display-owner/factory controls
  passed **2/2**; this is an unchanged-device control, not a shroud frame claim.
- Serial host-loopback LAN passed **4/4** in all six configurations.
- GCC Debug generated shroud, accepted scene boundary, presentation identity
  and provider-removal, and three dependency-ledger gates passed **7/7**.
  `git diff --check` passed.

The unrelated renderer diagnostic remains unstaged. This closes the 08F4
owner prerequisite with accepted 08F4A; it does not claim the 08F scenario
construction transition.
