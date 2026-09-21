# M22 original GameClient CPU presentation closure (slice 02)

The full-behavior M22 target compiles the same ten original GameClient draw
translation units as the M20/M21 schema target in a separate CMake directory
scope. Its `NDEBUG`/`ZH_WW3D_CPU_ONLY` configuration matches the original WW3D
reference-count layout and excludes `ZH_W3D_SCHEMA_ONLY`,
`ZH_W3D_HEADLESS_INSTANCE`, and `BRUTAL_TIMING_HACK`. The original schema
target remains available for its existing tests. The link-map gate rejects a
schema archive member for any of the ten classes in the full executable;
the ten class size/alignment comparisons now match the actual schema/full
header configurations, including `W3DDefaultDraw`'s two physical-state
fields. No source copy or second geometry/material authority was introduced.

The reached CPU providers are the original `W3DDisplay` ownership slots,
`W3DAssetManager`, `RTS3DScene`, `W3DShadowManager`,
`TerrainTracksRenderObjClassSystem`, `W3DShroud` material-pass vtables, and
WW3D2 `scene.cpp`, `light.cpp`, and `matpass.cpp`. A test-only harness constructs
the original asset manager and 3D scene and publishes their original display
slots; it does **not** claim production `W3DDisplay::init()` has completed.
That original init path creates the 2D status-circle device object before
publishing the 3D scene/asset manager. The original 2D physical edge and
subsequent presentation need the slice-04 GPU translation before a production
switch is possible.

The witness attaches/removes an original `Null3DObjClass` using original
`SimpleSceneClass` list/reference methods; uses the original track system and
terrain logic for bind, edge, cap, unbind, reset, and reverse shutdown; and
observes original shadow light/color/scene state and distinct volume,
projection, and decal routing. GPU-requiring operations throw the typed
`OriginalW3DDeviceUnavailable` from the original source at the first reached
physical operation, including WW3D scene fog/light state, GameClient 2D
status-circle construction, physical mesh/shadow/track flush, material-pass
installation, and texture-surface recolor. These are deliberately incomplete
devices, not successful null draws. Slice 04 must replace every reached guard
with the actual original-derived CPU/WWShade state translated into `GpuDevice`
commands, including both derived shadow managers. Slice 05 must validate
required retail asset failure/reset/retry at the owning scenario transaction;
the original AssetManager's intermediate publication semantics are unchanged.

The original WWLib pooled list-node allocator used `uint32* + 1` to skip a
pointer-sized block link. On x86_64 this overlapped the link and crashed at
static destruction. The pointer-width offset fix preserves its 32-bit layout;
the owned witness now checks four allocations of seven nodes, 22 live nodes,
all returns, and teardown on GCC and Clang. No original-game symlink contents
are read or written for this CPU fixture.

The CPU `DX8Wrapper` color conversion preserves the original ARGB component
order and x87 channel truncation after clamping; a focused source-parity
witness covers half-channel and out-of-range values. This converts original
CPU-selected color state only and does not expose a fake device.
