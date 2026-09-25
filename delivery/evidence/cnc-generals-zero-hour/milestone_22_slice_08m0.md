# M22 slice 08M0: exact empty terrain-bib cleanup

The accepted CPU-only terrain-map route initializes `m_bibBuffer` to null and
does not construct `W3DBibBuffer`. `W3DTerrainVisual::removeAllBibs` now
permits idempotent empty cleanup only when its visual and height-map owners
are still the published owners, the loaded logic map is the exact height-map
map, and no bib buffer exists. This does not depend on a generated selector.
The same empty owner may be cleared after scene/device detachment during
teardown. Existing no-map cleanup remains unchanged. Active faction/drawable
bib creation, per-ID removal, highlighting, storage and raw Direct3D render
paths remain guarded; this slice makes no claim that a retail scene never
reaches those producers.

The two-generation generated terrain-map witness repeats cleanup and proves
unchanged map/ref identities, Recording resource counts and operation trace.
Missing published visual, height-map or terrain owner rejects without owner
or Recording changes; detached-scene empty cleanup remains safe. Both active
producer entry points reject. The present-buffer negative is an explicit
source guard; no `W3DBibBuffer` can be constructed on the accepted CPU-only
route without widening into the separate active-bib owner. Existing generated
construction, borrowed file, bridge/pathfinder/map-attempt and no-map scene
controls verify teardown, reset and retry interactions.

Validation on final source: all six GCC/Clang Debug, Release and ASan+UBSan
complete builds and canonical asset-free nonretail suites passed 265/265 each
(`-LE gpu|lan|retail`; sanitizer suites used `ASAN_OPTIONS=detect_leaks=0`).
Both strict host LSan focused original W3D owner groups passed 9/9. Physical Vulkan
display/factory controls passed 2/2 with validation available; serial host
LAN passed 4/4 in all six configurations. Original dependency ledger and
`git diff --check` passed.

No retail source, path, logical name, raw output or private identifier is
retained. The original symlink and content remain untouched. The unrelated
renderer diagnostic remains unstaged. Slice 08 may now resume a read-only
redacted Recording continuation; if active bib production is reached, split
08M1 before admission.
