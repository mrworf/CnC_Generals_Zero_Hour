# M22 slice 08H1: authored visual terrain metadata

The canonical Linux `WorldHeightMap` reader now accepts bounded version-8
authored terrain metadata with multiple base and edge texture classes, active
blend/extra-blend and cliff records, cell references and cliff-state bits.
It validates fixed capacities, complete non-overlapping class spans, record
flags/directions, finite cliff UVs and every referenced index before the map
is published. Constructor failure releases all arrays and any partially-owned
source tiles.

Project-owned generated fixtures exercise two base classes, one edge class,
an active blend, extra blend and cliff record over two generations. Negative
fixtures cover invalid spans, overlap, tile/blend/cliff indices, record flags,
nonfinite values and trailing payload, with allocation-baseline checks after
every rejection. The accepted flat-map and logical-only routes remain intact.
No retail input was used.

Acceptance on the final tree:

- Focused generated visual-map gates passed with GCC and Clang.
- All six GCC/Clang Debug, Release and ASan+UBSan builds completed. Canonical
  non-GPU/non-LAN/non-retail suites passed 261/261 in all six configurations;
  sanitizer suites used `detect_leaks=0` for the sandbox restriction.
- Strict host LeakSanitizer passed the focused gate 1/1 with GCC and 1/1 with
  Clang.
- The established physical public Vulkan validation-layer display/factory
  controls passed 2/2. Serial host-loopback LAN passed 4/4 in all six
  configurations.
- All three dependency-ledger gates and `git diff --check` pass.

Multi-tile image decoding, general atlas packing and active blend/cliff query
semantics remain explicitly ordered in 08H2, 08H3 and 08H4.
