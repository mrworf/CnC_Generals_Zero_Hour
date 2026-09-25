# M22 slice 08H5: authored terrain consumer aggregate

The accepted 08H5A–08H5D owners now compose through one generated authored
terrain scene. The same original height-map instance consumes multi-class
base/edge atlases, primary and custom blend metadata, an exact row-major
extra-blend inventory and cliff-authored height data; it records two base
passes followed by the bounded road-alpha extra pass.

The aggregate generated route covers zero, one and multiple extra cells,
ordinary and cliff-overridden topology, exact draw order, failure at the
atlas/buffer/material/draw boundaries, clean retry and two device generations.
Every owner returns to baseline and no retail input was used.

Acceptance on the final production tree:

- Focused terrain/material gates pass 4/4 with GCC and Clang.
- All six builds and canonical non-GPU/non-LAN/non-retail suites pass 261/261;
  sanitizer suites use `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passes the focused test with GCC and Clang.
- Physical public Vulkan controls pass 2/2; serial host LAN passes 4/4 in all
  six configurations; all three dependency ledgers and `git diff --check`
  pass.

Aggregate authored terrain consumers are ready for 08H closure and redacted
retail boundary revalidation.
