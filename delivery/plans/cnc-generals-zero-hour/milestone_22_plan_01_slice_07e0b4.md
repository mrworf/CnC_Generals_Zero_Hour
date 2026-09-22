# M22 plan 01 slice 07E0B4: original empty smudge owner

## Outcome and dependency

Requires accepted 07E0B3. Native `W3DTerrainVisual::init` always creates and initializes `W3DSmudgeManager` after the water object, even for an empty world. Bootstrap the canonical original derived manager and its original `SmudgeManager` base under the active original display/device edge, with zero smudge sets, zero frame count and no D3D index/back-buffer allocation. Keep the class layout and native Windows implementation unchanged. Smudge effects, active sets, hardware capability probing, render-to-texture and pixels stay typed pending.

## Entry, state and failure

One original derived owner initializes against the existing display/edge and publishes `TheSmudgeManager`; identical init and empty reset/resource release/reacquire are stable without drawing. The owner can coexist with original view, scenes, empty HeightMap, zero-track, disabled-shadow and no-water owners across source/physical frames; its destruction clears only its own publication. Reject no edge/display, duplicate owner, nonempty smudge render or active sets, stale/released device, without displacing a live owner. Preserve the original base's empty-set list semantics; do not synthesize an effect or silently succeed for an active smudge.

Surfaces: canonical `W3DSmudge.cpp` and `GameClient/System/Smudge.cpp` only as needed for the original base, full-draw target, source/physical fixture, source identity/provider-removal, ledger and evidence. No retail symlink edit, factory switch, terrain visual composition, tactical update or display draw.

## Acceptance and commit

Positive and negative owner/no-draw controls, BGRA8/RGBA8 two-extents four-generation physical source pixel consistency, GCC/Clang leak-capable ASan+UBSan, identity/provider-removal/ledger, and five rebuilt full non-GPU suites pass. One independent plan/source/tests/evidence commit. 07E0B aggregate visual composition, factory and 07E draw remain pending.
