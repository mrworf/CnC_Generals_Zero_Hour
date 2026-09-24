# M22 08F5: map-owned no-model terrain prop dispatch

Parent commit: `4aaf9c22ccd73d6ae99bc95431ba2bb00cf20043`.
The original CPU `W3DTerrainVisual::addProp` now follows native weather/time
model-condition and first-draw-module selection. It accepts only the exact
published display/visual/map owner when no model name is selected, leaving
the no-prop-buffer terrain and Recording device unchanged. A selected active
model remains typed pending before prop mutation; this slice neither
allocates a prop buffer nor admits the 08F construction selector or retail
input.

The generated read-only terrain-map fixture uses the existing no-draw prop
and an independent generated W3DModelDraw template with a model name. Its
probe proves repeated no-model success, active-model rejection, pre-map and
malformed-input rejection, removed display/visual/terrain/height-map/asset/
global-data/scene providers, provider retry, three failed map-load rollbacks,
two in-process generations and two independently provisioned source runs,
zero retained aliases and zero Recording resources. The source dependency
ledger digest and bounded owner description are synchronized. No private
retail identifiers, paths, bytes, hashes, images or raw process output were
used or recorded.

## Final-state acceptance

- Six complete configured GCC/Clang Debug, Release and Sanitized builds passed
  after the final global-data guard.
- All six canonical `-LE 'gpu|lan|retail'` suites passed **260/260** each.
  Sanitized suites used `ASAN_OPTIONS=detect_leaks=0` for the established
  sandbox ptrace restriction; no CTest registration changed.
- GCC and Clang Debug focused generated prop, accepted scene boundary,
  presentation identity/provider-removal and three ledger gates passed
  **7/7** each.
- Strict host `ASAN_OPTIONS=detect_leaks=1` generated terrain prop and accepted
  scene-boundary controls passed **2/2** with GCC and **2/2** with Clang.
- Physical public Vulkan validation-layer display/factory controls passed
  **2/2**. This is an unchanged-device control, not a prop-frame claim.
- Serial host-loopback LAN controls passed **4/4** in all six configurations.
- Source dependency ledger validation and `git diff --check` passed. The
  unrelated renderer diagnostic remains unstaged.

08F5 closes only the no-model prop prerequisite. 08F must still independently
admit and validate its source-ordered water/object construction selector.
