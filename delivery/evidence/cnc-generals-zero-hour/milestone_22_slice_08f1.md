# M22 08F1: generated map-override parser/provider evidence

08F1 closes only the native `loadMapINI` parser/provider boundary with
project-owned generated inputs.  The witness statically requires the original
override load, optional text handoff, and display-preload order, then runs the
existing generated override/retry and provider-removal paths.  It does not
read, copy, name, hash, or record any private input, and it makes no scene,
pixel, audio, or retail-fidelity claim.

The source records fixed, selector-gated stage diagnostics only.  Default
startup and the accepted 08B/08D fail-closed boundaries are unchanged.  The
next dependency is 08F's post-parser source construction, not a permissive
continuation.

## Final-tree acceptance

- Focused GCC and Clang parser/provider, identity, and ledger controls passed.
  Strict host `detect_leaks=1` focus passed GCC **9/9** (33.34s) and Clang
  **9/9** (23.90s).
- After rebuilding every registered target in every configuration, the exact
  non-GPU/non-LAN suites passed GCC Debug **260/260** (234.27s), GCC Release
  **260/260** (136.23s), Clang Debug **260/260** (218.17s), Clang Release
  **260/260** (83.14s), GCC ASan/UBSan **260/260** (604.43s), and Clang
  ASan/UBSan **260/260** (467.59s).  Canonical sandbox sanitizer execution
  uses `detect_leaks=0` only for the established ptrace limitation.
- Registration is intentional: the five non-Clang-sanitizer configurations
  register 273 tests; Clang sanitizer registers 294 because it adds 21
  GPU-labelled physical tests.  Both sanitizer configurations register the
  same exact 260-test non-GPU/non-LAN acceptance set.
- A strict-UB diagnostic run was recorded only as non-acceptance information:
  it exposed pre-existing fixture/persistence/renderer-enum categories outside
  this slice.  It did not relax a check or alter source behavior.
- The host Vulkan validation set passed **9/9** under the Khronos validation
  layer.  The sandbox-only Vulkan initialization failure is environmental and
  was not counted as acceptance.
- Serial local-socket LAN passed **4/4** in GCC Debug, GCC Release, Clang
  Debug, Clang Release, GCC sanitizer, and Clang sanitizer.
- A stale full-probe artifact was found after the fixture-lifetime correction.
  Rebuilding the complete registered target set in all six configurations
  restored the preserved 08D boundary and invalidated every earlier suite;
  only the rebuilt final-tree results above are accepted.

The dependency ledger and whitespace diff check pass.  The unrelated renderer
diagnostic remains unstaged.
