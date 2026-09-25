# M22 slice 08K: detached active-water pre-map display continuation

`W3DDisplay::draw` now recognizes the exact 08J reset-detached active-water
owner state without consulting a generated or retail route selector. During
the source client-before-logic update, repeated display draws issue no frame
and no device command. Missing, foreign, pending and malformed owners reject;
failed terrain construction returns to the same command-free state for retry;
successful load retains the ordinary mapped display path.

Generated coverage proves repeated no-op, zero command delta, predicate
rejection, failed-load rollback and retry, two device generations, the 08F0
parser stop, 08F construction completion and zero retained resources. No
retail input was used.

Acceptance on the final slice tree:

- Focused terrain-water, generated-boundary and generated-construction routes
  pass with GCC and Clang.
- All six canonical configurations build and their non-GPU/non-LAN/non-retail
  suites pass 261/261; sanitizer suites use `detect_leaks=0`.
- Strict host LeakSanitizer passes the three focused routes with GCC and Clang.
- Physical public Vulkan controls pass 2/2; serial host LAN passes 4/4 in all
  six configurations.
- All three dependency-ledger checks and `git diff --check` pass.
