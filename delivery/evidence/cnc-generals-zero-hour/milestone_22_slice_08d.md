# M22 slice 08D: active water-owner reset after display detachment

08D closes the original active translucent `WaterRenderObjClass` lifecycle
after `GameClient::reset` orders display detachment before terrain reset.  The
bounded retail selector admits only the exact published active-water owner
with valid original buffers after it has left the current scene.  The reset is
source-faithful no-op CPU state preservation; unsupported continuation stops
at the outer typed boundary after the reset has completed.  Default,
pre-detach, foreign, stale, cloud-only/no-buffer, and unavailable-provider
paths stay fail-closed.  No retail content, private location/name/hash, raw
graphics API, map frame, or pixel behavior is recorded.

Focused proof:

- Generated Recording `original_w3d_terrain_water` passed in GCC and Clang
  Debug.  It proves the display-detach → terrain reset → active-water reset
  ordering, selector absence and pre-detach rejection, provider loss,
  resource mismatch and failure/retry controls, two generations, and zero
  owned resources/providers at teardown.
- The read-only redacted campaign/skirmish audit passed 1/1 in its retail
  configuration across both modes and two generations.  It observes only the
  aggregate completed-reset marker, typed next-boundary rollback, and zero
  owners/resources.

Final-tree acceptance:

- Fresh non-GPU/non-LAN CTest suites passed **213/213** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan+UBSan, and Clang ASan+UBSan.
  The two sanitizer broad suites used `ASAN_OPTIONS=detect_leaks=0` only
  because the sandbox ptrace layer prevents LeakSanitizer from starting.
- Host `ASAN_OPTIONS=detect_leaks=1` focused terrain-water, presentation
  identity, and dependency-ledger checks passed **3/3** in both sanitizer
  configurations.  The initial sandbox leak-enabled attempt correctly failed
  before test execution with the documented ptrace restriction; host results
  are the acceptance results.
- The accepted public Vulkan stencil-edge validation-layer probe passed
  **1/1** in GCC Release with no test-reported validation diagnostics.  An
  exploratory sanitizer-GPU invocation reported only external desktop D-Bus
  allocations; it was not used as slice leak evidence.
- Serial loopback LAN validation passed **4/4** in all six configurations.
  The three dependency-ledger checks plus terrain-water and presentation
  identity passed 5/5, and `git diff --check` passed.
- The unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic remains
  unstaged.
