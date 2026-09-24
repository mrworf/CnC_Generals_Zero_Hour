# M22 08F: redacted post-map scenario construction

Parent commit: `680d9e7ef9c5f4721cb919550249e41ce8e75424`.
The separate generated construction selector is valid only with the accepted
08F0 generated scene and bounded Recording/config/reset owners. Without it,
08F0 still stops after map-INI parsing and before terrain load. With it, native
source order publishes terrain, reattaches water, disables the grid, applies
map overrides, then proceeds through radar, shroud, terrain logic, radar
terrain refresh, pathfinder, permanent observer and authored object traversal.
The generated no-model prop reaches the accepted 08F5 source owner before the
deliberate post-object boundary.

The generated fixture contains an explicit playable boundary, flat visual
blend metadata, a generated terrain texture and a non-edge partition size.
Mission and skirmish each pass twice. Negative controls reject an absent or
standalone selector and a removed terrain-texture provider; retry succeeds.
Failure and normal boundary cleanup remove the water and terrain scene links,
release the map and leave zero published graphics owners and Recording
resources. Input remains read-only. No retail provider/content or private
path, identifier, filename, byte, hash, image or raw process output was used
or recorded.

## Final-state acceptance

- Six complete GCC/Clang Debug, Release and ASan+UBSan builds passed.
- All six canonical `-LE 'gpu|lan|retail'` suites passed **261/261** each.
  Sanitized suites used `ASAN_OPTIONS=detect_leaks=0` for the established
  sandbox ptrace restriction.
- GCC and Clang focused generated construction/parser-boundary, terrain-map,
  water, shroud, identity and provider-removal controls passed.
- Strict host `ASAN_OPTIONS=detect_leaks=1` generated parser and construction
  controls passed **2/2** with GCC and **2/2** with Clang.
- Physical public Vulkan validation-layer display/factory controls passed
  **2/2** in the non-sanitized GPU build. A diagnostic attempt in the Clang
  sanitizer GPU build exceeded the wrappers' fixed subprocess limits; it
  reported no device or validation error and was not used as acceptance.
- Serial host-loopback LAN controls passed **4/4** in all six configurations.
- All three source dependency-ledger gates and `git diff --check` passed. The
  unrelated renderer diagnostic remains unstaged.

08F closes generated post-map construction only. Slice 08 still owns the
read-only retail campaign/skirmish Recording gate.
