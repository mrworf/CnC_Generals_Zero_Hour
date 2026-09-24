# M22 08F0B: original display smart-purge handoff

The Linux `W3DDisplay` source owner now performs the original optional
`AssetUsage.txt` token scan and native exclusion-list purge. A missing optional
file passes an empty list and purges; null/empty names retain the source early
return. A broken published display/asset owner or missing file-system provider
fails closed before mutating assets. The scan closes an opened file on both
normal and exceptional exit. This closes the prerequisite to 08F0 only: no
scene selector, terrain transition, retail input, or pixel claim is admitted.

The generated two-root W3D fixture witnesses exact listed preservation,
unlisted removal, absent-file purge, comment-only purge, reload after purge,
missing-provider/owner rejection, reset, and two clean generations. Its
project-owned read-only source tree is byte-identical after the run. The
existing source owner probe still checks independent mesh refs, alias teardown,
and zero device resources. The unrelated renderer diagnostic remains unstaged.

## Acceptance

- Complete registered-target builds passed in GCC Debug, GCC Release, Clang
  Debug, Clang Release, GCC ASan/UBSan, and Clang ASan/UBSan.
- Focused generated display-owner tests passed in GCC/Clang Debug and both
  canonical sanitizers. Strict host `ASAN_OPTIONS=detect_leaks=1` focus on
  `original_w3d_display_owner` plus the `loadMapINI` parser aggregate passed
  **2/2** in GCC and **2/2** in Clang. The sandbox canonical sanitizer runs use
  `detect_leaks=0` only for the established ptrace restriction.
- Host physical bgfx Vulkan display-owner test passed **1/1** with
  `VK_LAYER_KHRONOS_validation` enabled. This is a generated owner/device
  lifecycle witness, not a scene or pixel claim.
- Serial local-socket LAN passed **4/4** in each of all six configurations.
  A sandbox-only attempt failed to create UDP sockets (`Operation not
  permitted`); host rerun with no code or test change supplied acceptance.
- Source presentation identity and original resources/data/process dependency
  ledger gates passed in GCC Debug. Final `git diff --check` passed.
- Full six-configuration non-GPU/non-LAN suite results are recorded below.

| Configuration | Result | Time |
|---|---:|---:|
| GCC Debug | 259/259 | 60.59s |
| GCC Release | 259/259 | 35.19s |
| Clang Debug | 259/259 | 55.96s |
| Clang Release | 259/259 | 21.55s |
| GCC ASan/UBSan | 259/259 | 159.39s |
| Clang ASan/UBSan | 259/259 | 125.09s |

The prior 08F1/08F0A evidence reported 260 non-GPU/non-LAN tests. The
committed `CMakeLists.txt` adds one parser-aggregate registration in 08F1 and
has no further registration change through this slice. The 08F0 discovery
records removal of an incomplete, uncommitted selector test before this
prerequisite slice. All six freshly configured lists contain 259 tests. Thus
the one-test difference is attributable to that transient exploratory
registration, not removal of an accepted test; its exact CTest name is not
recoverable from committed artifacts. No 08F0B CMake registration is added.

The dependency ledger includes the new source `FileSystem`/`File` edge and
corrects the pre-existing accepted 08F0A game-engine source hash. No private
retail identifiers, paths, content, or hashes were used or recorded.
