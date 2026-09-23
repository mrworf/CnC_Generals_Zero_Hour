# M22 slice 08A: private-safe retail Recording boundary discovery

08A adds a host/test-only `RecordingGpuDevice` selection to the already
published original factory edge.  The default factory remains public bgfx;
the Recording selector is rejected unless the original-factory selector is
also present.  The Recording result exports aggregate operation counts only,
never labels or snapshots.

The read-only campaign and skirmish audit ran with isolated writable state and
returned the same sanitized configuration mask: **30**.  This means only bits
1--4 were reached: terrain tracks, shadow volumes, source shadow decals, and
cloud plane.  Each run stopped at the unchanged native terrain guard before a
scenario success marker.  Read-only input metadata was unchanged.  No private
root, filename, archive member, byte, hash, model, screenshot, or raw command
label is retained here.

Acceptance on the final tree:

- The separately configured registered retail Recording audit passed **1/1**;
  two generated Recording generations, invalid selector, forced factory
  failure/rollback, and zero-resource teardown are included in that gate.
- Fresh non-GPU/non-LAN suites passed **206/206** in each of GCC Debug,
  GCC Release, Clang Debug, Clang Release, GCC ASan+UBSan, and Clang
  ASan+UBSan.  Broad sanitizer CTest uses `LSAN_OPTIONS=detect_leaks=0` only
  for the established ptrace limitation.
- Host `ASAN_OPTIONS=detect_leaks=1` focused ownership/provider validation
  passed **6/6** in both sanitizer builds.  The source-owner focus includes
  terrain atlas, particle provider, shadow/decal route, presentation identity,
  provider removal, and the dependency ledger.
- Validation-layer physical factory bootstrap, rigid, and generated-map
  controls passed **3/3**.  They exercise the unchanged public bgfx factory;
  08A makes no physical-pixel or retail-rendering claim.
- Serial local LAN validation passed **4/4** in every one of the six builds.
  The final dependency-ledger validator, source identity/provider controls,
  and whitespace diff check passed.

The existing `tests/renderer/test_bgfx_device.cpp` diagnostic remains
unstaged and outside this slice.  The required next slice is 08B; it must
close only the mask-30 bits without relaxing the guard.
