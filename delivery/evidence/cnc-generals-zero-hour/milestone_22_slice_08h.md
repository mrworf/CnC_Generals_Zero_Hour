# M22 slice 08H: authored visual-terrain aggregate

The accepted 08H1–08H5 owners compose the original authored visual-terrain
metadata, multi-tile image sets, atlas packing, blend/cliff queries, static
terrain payload and ordered extra-blend material/geometry submission through
the generated multi-class scene. The composed route covers base and edge
classes, active primary and extra blends, a cliff override, exact draw order,
failure rollback and two device generations without retail input.

The read-only redacted retail continuation was then rerun in GCC Debug and
Clang ASan+UBSan. Both configurations reported the same fixed boundary:
factory reachability=1, setup=1, scene=0, init=0, logic=0, map-INI=0,
stages=0, recording=0 and teardown=1; sanitizer categories=0. This is the
deliberately retained slice-08 scene-admission boundary, before the first
scene stage, rather than a new authored-terrain source owner. No private path,
logical identifier, filename, content, hash, image or raw output is retained.

Acceptance on the final production tree:

- Focused authored-terrain gates pass with GCC and Clang.
- All six builds and canonical non-GPU/non-LAN/non-retail suites pass 261/261;
  sanitizer suites use `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passes with GCC and Clang.
- Physical public Vulkan controls pass 2/2; serial host LAN passes 4/4 in all
  six configurations; all three dependency ledgers and `git diff --check`
  pass.

Slice 08 now owns the explicit retail scene admission transaction and its
borrowed display/scene-owner unwind contract.
