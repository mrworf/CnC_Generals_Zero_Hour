# M22 slice 08N0B: render-ready modeled volume geometry

After 08N0A publishes a modeled render object and its bounded volume caster,
the CPU-only W3D draw path now readies that exact caster when an authored map
is present. Initial creation and model replacement use the same post-scene,
post-caster transition. The terrain visual proves the exact map, scene and
map-owned `W3DBufferManager` identity; missing or foreign providers, detached
renders, duplicate readiness and absent managers reject. Standalone pre-map
volume casters retain their existing deferred `ReAcquireResources` boundary.

The per-caster transition builds the original source vertex/index slots and
eagerly uploads both Recording buffers before accepting the modeled caster as
render-ready. Partial source-slot allocation, geometry, vertex/index upload
and finalization failures release the new slots and device buffers before the
Drawable constructor/replacement unwind removes the just-created caster and
scene object. Resource release remembers the exact source buffer provider,
independent of a later global pointer change. Earlier accepted casters remain
published, and a clean retry succeeds.

The generated map/frame test creates a modeled Drawable after map load,
verifies exact caster identity and immediate Recording residual after six
injected source/geometry/finalization edges plus device create/upload faults,
then replaces its model and exercises replacement failure/retry. It rejects
missing/foreign providers, duplicate finalization and detached renders. Two
generations produce ordered terrain, tracks, two volume stencil casters with
nonzero 24-index source ranges, and water Recording commands. Drawing failure,
resource reacquisition and removal end with zero caster/device residual. The
existing standalone volume, decal and generated bridge-constructor fixtures
remain independent and green.

All six GCC/Clang Debug, Release and ASan+UBSan complete builds and canonical
nonretail suites passed 267/267 each (`-LE gpu|lan|retail`; sanitizer suites
used `ASAN_OPTIONS=detect_leaks=0`). Both focused strict host LSan groups
passed 11/11, including the modeled-ready frame witness. Physical Vulkan
display/factory controls passed 2/2 with host escalation; serial host LAN
passed 4/4 in all six configurations. The original dependency ledger and
`git diff --check` passed on final source.

No retail source, selector, path, logical name, bytes, hash or raw output is
retained. The original symlink/content is untouched, and the unrelated
renderer diagnostic remains unstaged.
