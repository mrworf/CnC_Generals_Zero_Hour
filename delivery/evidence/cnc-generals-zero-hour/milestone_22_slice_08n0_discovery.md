# M22 slice 08N0 discovery: in-flight modeled volume shadow

After accepted 08M0, a corrected single-separator, read-only retail Recording
continuation crossed the fixed engine-init, map-local INI and game-logic
markers through `map-loaded`, then six of seven fixed post-map phases through
observer. An authored modeled object in the ordinary map-object loop stopped
at the fixed `bounded volume shadow owner or state unavailable` category.
The process performed clean Recording teardown. No active bib producer was
reached, so there is no 08M1 justification. No retail selector, path, logical
name, bytes, hash, raw output or stack is retained.

Source ordering explains the failure. `Drawable` registers itself before
constructing draw modules, but the new `W3DModelDraw` constructor calls
`setModelState` before its pointer is stored in the Drawable module array.
That method currently calls `W3DShadowManager::addShadow(render, info, draw)`
before adding the render object to the scene and binding its DrawableInfo
user-data. The current shadow guard accepts only a scene-linked render object
discoverable through a published W3DModelDraw module. Neither condition can
hold at this legitimate in-flight call. An unconditional relaxation would
also accept a foreign render object or stale Drawable.
`DrawableInfo` itself is default-constructed with a null back-pointer and
receives its `m_drawable=this` assignment only after draw-module creation;
safe early user-data binding therefore requires moving that assignment
before the constructor invokes module creation, without widening publication.
The fail-closed closure is to defer initial shadow admission until the
`onObjectCreated` callback sees the published module and scene-linked model,
and to move replacement admission after new scene publication. The existing
shadow-manager identity guard need not be weakened.

The two owners are separable. `BoundedVolumeShadow` construction publishes a
manager entry and stores the render-object pointer but creates no geometry;
`reacquire` allocates volume slots. Terrain-map resource reacquisition runs
before ordinary map objects are constructed, so it cannot make this later
caster render-ready. `W3DModelDraw::nukeCurrentRender` releases its shadow
before scene removal, while constructor failure before module publication
needs local unwind because Drawable's module-array rollback cannot delete an
unpublished module. Plan 08N0A closes exact in-flight scene/admission and
unwind; 08N0B closes deferred geometry readiness and resource failures.
