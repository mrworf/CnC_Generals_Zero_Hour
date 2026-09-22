# M22 plan 01 slice 07E0D2: original tactical camera-position accessor

## Outcome and dependency

Requires accepted 07D original W3DView camera ownership. The native `W3DView::get3DCameraPosition` converts its owned 3D camera's position to the `Coord3D` reference used during GameClient audio initialization. This is a narrow source-equivalent accessor prerequisite to production-order original view factory startup; it does not implement audio behavior or camera movement.

## State and failure

An uninitialized original view has no camera and rejects the accessor without publishing or changing an owner. An initialized view returns the current 3D camera coordinates, preserving native static-reference semantics and reflecting subsequent camera position changes. Reset/re-entry retains the same camera owner. Native Windows branch and class layout remain unchanged.

## Acceptance

Add positive/negative source controls to the original W3DView probe and retain its empty/one-rigid physical oracle. Rebuild and pass five full non-GPU suites including leak-capable GCC/Clang sanitizer controls, repeat GCC Debug/Clang Release host Vulkan validation, synchronize the source ledger, and commit this slice independently with evidence. The pending no-bib teardown and factory-bootstrap edits are isolated until afterward.
