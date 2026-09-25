# M22 slice 08M0 prerequisite discovery: empty terrain bib cleanup

After accepted 08L, a corrected single-separator, read-only retail trial
crossed three engine-init, four map-local INI and five game-logic stages through
`map-loaded`, then stopped before scene construction. Its fixed failure
category is SIGABRT in `W3DTerrainVisual::removeAllBibs`, reached from the
client/UI cleanup path. The same category reproduced with the retail-configured
GCC probe. No retail path, logical name, content, raw output or stack was
retained. The temporary scene admission and redaction edits were removed
before this plan-only checkpoint.

Source ownership separates this reached empty-list operation from active bib
production. `InGameUI::destroyPlacementIcons` calls `removeAllBibs` when a
terrain visual exists, even with no placement icon or bib. The CPU-only
`BaseHeightMapRenderObjClass` initializes its bib-buffer pointer to null and
does not create a bib buffer on the accepted terrain-map route. In contrast,
placement feedback in `BuildAssistant`, AI and `InGameUI` can call
`addFactionBib` or `addFactionBibDrawable`; the CPU-only entry points remain
fail-closed. The legacy active storage/render path uses a separate
`W3DBibBuffer` with raw Direct3D operations and is not available in this
route. Thus an empty-list cleanup is independently testable, while active bib
storage, rendering, per-ID removal and highlighting require a separate owner
slice only if a subsequent redacted retail continuation actually reaches a
producer. No permissive active-bib no-op is authorized.
