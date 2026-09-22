# M22 plan 01 slice 07B2B: deterministic W3D dependency identity

## Outcome and boundary

The M20 W3D identity checker scans complete `ldd` output for `d3d`, so a randomized hexadecimal mapping address can falsely classify a clean Linux executable as Direct3D-linked. Inspect dependency names only. This is a test-gate reliability prerequisite for 07B2, not a change to product rendering or the production source inventory.

## Positive, negative and gate

A Linux DSO line with `d3d` only in its address must pass; `libd3d8.so`, `libvkd3d.so`, and `libDirect3D.so` dependency names must fail. An actual production `zh_original_main` identity check must pass in all four presets, with repeated invocations insensitive to ASLR. Run the new unit test, four-preset relevant identity/negative suites and the full asset-free suites, except any explicitly documented pending 07B2 source-ledger drift. Commit checker, tests, plan and evidence independently from 07B2 source work.
