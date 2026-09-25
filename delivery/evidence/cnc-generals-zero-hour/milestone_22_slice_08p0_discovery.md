# M22 slice 08P0 discovery: original tree draw provider

After accepted 08N0, a corrected read-only retail continuation crossed all
six fixed post-map phases and entered ordinary authored object construction.
Ten `W3DModelDraw::onObjectCreated` entries and exits balanced. The first
failing edge was `ModuleFactory::newModule`: draw-provider type 1, public
schema-only provider ordinal 8 of nine, `W3DTreeDraw`. The factory correctly
throws `ERROR_INVALID_D3D` for its null physical create proc. No Recording
scene frame was published; teardown completed. No active bib producer was
reached. Only this fixed source category is retained; no retail selector,
root, logical name, byte, hash, raw output, stack or label is retained.

The source graph is not a one-line registration change. Full-instance
`W3DModuleFactory` retains a schema-only `W3DTreeDrawModuleData` parser and
null physical proc. `W3DTreeDraw` itself has an empty constructor and draw
method; its first transform sets `m_treeAdded` before asking
`TheTerrainRenderObject->addTree`, and its destructor does not call
`removeTree`. The accepted CPU-only `BaseHeightMapRenderObjClass` initializes
`m_treeBuffer=NULL`, while the Windows tree buffer constructor allocates raw
Direct3D resources. Source tree type admission resolves a model mesh, bounds
and texture; the buffer owns bounded tree/type/partition arrays, atlas tiles,
vertex/index buffers and a projected-shadow pointer. Terrain pass drawing
performs cull, sway/topple updates, atlas/shroud/lighting binding, triangles
and optional decal queue/flush. Native `addTree` returns `void`; full type
capacity and missing/non-mesh asset branches return ambiguous `0`. Enabling
the create proc now would silently omit or retain trees and could cross raw
device calls.

The dependency closure is separated by real owners: 08P0A atomically owns
tree type/instance and terrain lifetime without a frame; 08P0B prepares
model/atlas/mesh GPU-edge resources without factory admission; 08P0C records
the source tree pass and only then admits the physical module. 08P0 is the
generated aggregate. Retail selector/borrow-unwind belongs to slice 08.
All trial production wiring and diagnostics were removed before this
plan-only checkpoint; unrelated renderer diagnostic remains unstaged.
