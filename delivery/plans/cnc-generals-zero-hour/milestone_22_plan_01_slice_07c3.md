# M22 plan 01 slice 07C3: original W3DDisplay owner bootstrap

## Goal, scope and ordering

Requires accepted 07C2. Instantiate the original `W3DDisplay` class in the full-draw source probe and give its Linux CPU branch a bounded, explicit lifecycle for the three original scene slots, original W3D asset-manager slot and WW3D session under a supplied active `OriginalGpuEdge`. `init()` publishes owners only after successful setup, repeated `init()` is idempotent, `reset()` detaches objects in the source scene, and destruction retires owned refs/assets and shuts WW3D down before the edge ends. No production factory switch, `draw()`, terrain, shroud, tracks, shadows, particles, or retail-scene acceptance. Preserve original header/class layout and native branch.

## Entry, state, errors and surfaces

An initialized GameClient scenario creates a local original display while the existing host services and edge are active. Before `init`, no display owners are published. After `init`, all three scene pointers and the asset-manager pointer are non-null and stable; re-entry does not duplicate or replace them. `reset` removes owned rigid objects, and teardown restores null slots and zero source/device resources before the edge is destroyed. Missing edge and already-owned static slots reject without leaving partially published owners; a later valid retry succeeds. Internal allocation or `WW3D::Init` failure must unwind locally built owners, but this slice introduces no allocator fault hook. There is no new authorization surface: this is an in-process source fixture with no user input or file mutation. Expected surfaces are `W3DDisplay.cpp` CPU branch, minimal source-only source-probe wiring, tests, ledger, and evidence. Investigate and resolve original class virtual closure and base `Display` dependencies before implementing, without inventing no-op successful display behavior.

## Acceptance and commit

Positive: source class identity, owner pointer/refcount lifecycle, repeated init/reset/destruction and two fresh generations. Negative: no edge with same-object retry, occupied global slots, reset before init and unsupported draw rejection. Verify GCC/Clang source-only ASan/UBSan with leak-capable controls, explicit host Vulkan where the owner lifecycle physically touches an edge, full mandated asset-free suites, provider/ABI identity and clean ledger. Commit plan, source, tests and evidence as one independent slice only after gates pass. Later 07 children own production factory/draw and terrain/effect families.
