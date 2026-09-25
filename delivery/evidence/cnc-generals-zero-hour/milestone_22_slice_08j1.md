# M22 slice 08J1: detached disabled-water map continuation

The previously accepted factory-map physical control had regressed after 08J:
display reset detached the disabled-water source owner, while the active-water
pre-map predicate correctly required enabled water and ready GPU buffers.
The disabled owner intentionally retains a pending sentinel with no GPU
buffers. The map-load preflight rejected it before visual-map binding.

The correction adds a distinct, selector-independent disabled-water state
check for the exact terrain, visual, water, scene and global configuration.
It requires zero extents, no cloud/grid/river state and absent water buffers;
the active-water predicate and its ready-buffer contract are unchanged.
After map binding, terrain attaches first and the same disabled-water owner
second. The native no-river override is an exact attached-owner no-op and
cannot allocate or draw. Any failed map attempt removes only its scene links,
releases partial terrain/map resources and leaves the detached owner retryable.

The generated witness covers two disabled-water generations, removed provider,
malformed extent/cloud rejection, injected map-buffer failure with immediate
zero additional Recording residual, same-owner retry, ordered attachment,
and no-water override rejection for removed provider, detached scene and
malformed configuration. Existing active-water and construction witnesses
remain separate. The generated physical factory-map control passes with
Vulkan validation under both GCC and Clang Debug; the established GCC physical
display/bootstrap/map controls pass 3/3, including the map control's
absent/missing/malformed/failure cases.

Final-source validation: all six GCC/Clang Debug, Release and ASan+UBSan
complete builds and canonical nonretail suites passed 267/267 each
(`-LE gpu|lan|retail`; sanitizer suites used `ASAN_OPTIONS=detect_leaks=0`).
Both focused strict host LSan groups passed 3/3 across terrain water,
generated scene boundary and construction. Serial host LAN passed 4/4 in
all six configurations. The original dependency ledger and `git diff --check`
passed. No private retail input, symlink content, raw output or renderer
diagnostic was changed.
