# M22 plan 01 slice 08P0A: source tree owner and atomic registry

## Goal and boundary

Close the original terrain-owned tree instance lifecycle without claiming a
physical tree frame. The current CPU-only `BaseHeightMapRenderObjClass` leaves
`m_treeBuffer` null. The source `W3DTreeDraw` attempts tree admission from its
first transform callback, sets `m_treeAdded` before checking admission, and
never removes the instance in its destructor. Merely enabling its factory
create proc would therefore silently omit or retain trees.

Implement a bounded CPU tree type/instance owner attached to the exact loaded
terrain/map identity. Preserve source `DrawableID`, position, scale, angle,
model/texture module data, type and area-partition semantics; no fabricated
retail asset or proxy model. Make registry capacity/readiness failure explicit
and atomic, with `m_treeAdded` committed only after successful admission.
Keep the original terrain class layout shared with non-CPU translation units:
the CPU registry is lazily owned by exact terrain identity and erased on
reset/teardown, so an empty graphics bootstrap allocates no tree state.
Correct relocation, removal, reset and terrain teardown so the source
`W3DTreeDraw`/terrain owner cannot leave a stale ID/type/partition entry.
Guard null/foreign/stale terrain or module data before mutation. Model and
texture asset readiness remains 08P0B, before physical factory admission.
Constructor and failed first transform must unwind without gameplay destroy hooks and
allow deterministic retry. Preserve existing map/bridge/bib/shroud owners.

The factory's `W3DTreeDraw` physical create proc stays null in this slice;
direct generated source-owner tests exercise the registry. Raw Direct3D,
texture atlas, vertex/index upload, tree frames, shadow decals and retail
admission belong to later slices.

## Validation and commit

Generated tests cover distinct type/instance IDs, repeated transform,
relocation, duplicate/stale ID, capacity/partial allocation failure,
immediate pre-teardown residual counts, reverse removal, two map generations,
provider removal and clean retry. Default/schema-only routes stay closed.
Run six complete GCC/Clang Debug, Release and ASan+UBSan builds and canonical
nonretail suites (`-LE gpu|lan|retail`, sanitizer `detect_leaks=0`), focused
strict host LSan, physical Vulkan 2/2, serial LAN 4/4 each six, ledger and
diff checks on final source. Commit one slice:
`delivery: M22 08P0A own tree registry atomically`.
