# M22 08F2: disabled source water-grid query

Parent commit: `5f81db3c6314364e4bafe4c457ff0074f2563e58`.
The original Linux `W3DTerrainVisual::getWaterGridHeight` now returns `FALSE`
for a published disabled-grid owner without reading coordinates or changing
the caller's height. It accepts the already-established scene-attached and
08D reset-detached water states, but rejects pre-init, missing or foreign
terrain/water providers, pending water resources, inconsistent scene/terrain
aliases, and an enabled grid. A failed `enableWaterGrid(TRUE)` leaves the
disabled flag unchanged; the accepted bounded water owner still rejects an
active grid. The getter does not admit 08F scenario construction, a frame, or
any retail provider.

The generated read-only terrain-water probe covers disabled and null-output
queries, failed active selection, forced inconsistent flag, provider removal,
failed resource acquire and retry, reset-detached query, repeated generations,
and zero Recording resources on teardown. The accepted 08F0 generated parser
boundary test still passes. No private identifiers, paths, bytes, hashes,
images, or captured process output are recorded.

## Acceptance

- Six complete configured builds passed.
- GCC and Clang Debug focused terrain-water plus accepted 08F0 selector tests:
  **2/2** each.
- Source identity, provider-removal, presentation, and dependency-ledger
  focus: **8/8** in GCC Debug; `git diff --check` passed.
- Strict host `ASAN_OPTIONS=detect_leaks=1` terrain-water and 08F0 selector
  focus: **2/2** in GCC and **2/2** in Clang. Canonical sandbox sanitizer suites
  used `detect_leaks=0` for the established ptrace restriction.
- Physical host Vulkan validation-layer display-owner/factory controls:
  **2/2**. These are unchanged-device controls, not a grid or 08F frame claim.
- Serial host-loopback LAN: **4/4** in each of all six configurations.

Canonical non-GPU/non-LAN/non-retail full suites:

| Configuration | Result | Time |
|---|---:|---:|
| GCC Debug | 260/260 | 38.14s |
| GCC Release | 260/260 | 24.32s |
| Clang Debug | 260/260 | 33.48s |
| Clang Release | 260/260 | 15.09s |
| GCC ASan/UBSan | 260/260 | 167.85s |
| Clang ASan/UBSan | 260/260 | 132.62s |

An initial noncanonical test filter omitted the `retail` exclusion and ran one
additional retail-labeled test. That run is not used as acceptance evidence;
the corrected six-suite matrix above explicitly excludes it. No output from
that test is included in tracked evidence. This slice adds no CTest registration,
so the canonical count remains the accepted 08F0 **260**.

08F may now resume from the generated post-parser boundary. The source
map-load water reattachment and later disabled setter call remain 08F
construction work, not a claim of this query-only prerequisite.
