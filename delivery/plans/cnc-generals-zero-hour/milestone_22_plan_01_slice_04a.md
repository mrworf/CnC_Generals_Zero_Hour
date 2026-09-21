# M22 plan 01 slice 04A: original FVF and mesh CPU gate

## Outcome and dependencies

Requires accepted slices 01–03. The canonical original `dx8fvf.cpp/.h`
calculate vertex layout/offsets on Linux without Direct3D helper calls;
original `RenderInfoClass` constructs; original `MeshClass::Render` returns
for an authored hidden mesh and fails typed-unavailable for visible mesh at
the physical edge. This is a bounded CPU step, **not** original pass
scheduling, rendered output, or complete mesh behavior. 04B, 04C and later
full retail/Vulkan acceptance remain mandatory.

## Source and test boundary

Link original `rinfo.cpp` and `dx8fvf.cpp`, retaining the Windows/native
path. Preserve original FVF bit/layout meaning, UV sets, positions, normals,
colors, and strides without a fake D3D SDK. Drive original owned mesh state
through hidden/visible paths. Reject unsupported FVF position and UV count;
verify original provider removal, source identity, no duplicate schema/full
objects, GCC and Clang Debug suites, sanitizer-focused tests, teardown,
ledger freshness and worktree checks. The typed visible edge is a negative
control, not an acceptable final renderer result.

## Commit boundary

One independently validated commit: `delivery: M22 slice 04A preserve original FVF and mesh gate`.
