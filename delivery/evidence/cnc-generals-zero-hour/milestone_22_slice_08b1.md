# M22 slice 08B1: bounded multi-track source pool

08B1 closes only aggregate configuration-mask bit 1. The CPU original terrain
track system now restores its existing general module pool for every positive
cardinality that fits the native 16-bit vertex-offset budget. It retains the
zero-module no-resource owner and rejects invalid or overflowing capacities.
The map scene admits only active modules owned by that exact system and no
more than the configured capacity. Cloud, volumetric shadow, and every other
advanced terrain option remain fail-closed for 08B2/08B3.

Generated Recording acceptance proves one- and two-module generations, shared
source buffer sizing, capacity exhaustion, foreign bootstrap rejection,
create/upload/draw failure rollback and retry, reset, re-entry, overflow
rejection, and zero Recording resources. The private retail corpus was not
opened by this slice.

Final-tree acceptance:

- Fresh non-GPU/non-LAN suites passed **206/206** in GCC Debug/Release,
  Clang Debug/Release, GCC ASan+UBSan, and Clang ASan+UBSan. Broad sanitizer
  CTest used `LSAN_OPTIONS=detect_leaks=0` only for the established ptrace
  limitation.
- Host `ASAN_OPTIONS=detect_leaks=1` focused terrain-track, identity,
  provider-removal, and dependency-ledger checks passed **4/4** in both
  sanitizer configurations.
- Validation-layer physical factory bootstrap, rigid, and generated-map
  regressions passed **3/3**. Serial LAN validation passed **4/4** in every
  one of the six configurations.
- The final original dependency-ledger validator, identity/provider checks,
  and whitespace diff check passed.

The existing `tests/renderer/test_bgfx_device.cpp` diagnostic remains
unstaged and is outside this slice.
