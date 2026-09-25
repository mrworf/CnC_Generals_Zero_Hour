# M22 slice 08H3: authored terrain atlas packing

The canonical `WorldHeightMap` terrain atlas owner now packs every authored
base and edge texture class into independent bounded fixed-width grids. It
places larger square classes first, preserves source class/tile order within
each size, retains the four-pixel wrapped border contract and publishes only
after complete GPU create/upload success. Base and alpha continue to share one
GPU texture while the edge atlas has independent ownership and teardown.

Project-owned coverage supplies two differently sized base classes and one
edge class. It verifies exact source positions, atlas dimensions, representative
base and edge bytes, shared-alias ownership, create/upload rollback, retry and
two generations. Direct negative controls reject a missing tile owner and a
class set exceeding atlas capacity. No retail input was used.

Acceptance on the final tree:

- Focused atlas gates passed with GCC, Clang and both sanitizers.
- All six builds and all six canonical non-GPU/non-LAN/non-retail suites pass
  261/261; sanitizer suites used `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passed 1/1 with GCC and 1/1 with Clang.
- Physical public Vulkan validation-layer display/factory controls pass 2/2.
  Serial host-loopback LAN passes 4/4 in all six configurations.
- All three dependency-ledger gates and `git diff --check` pass.

Active blend, extra-blend, custom-edge and cliff query semantics remain 08H4.
