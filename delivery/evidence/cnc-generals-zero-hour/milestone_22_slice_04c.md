# M22 slice 04C: original mesh entry, WW3D state, typed device edge

The canonical original `MeshClass::Render` now executes the authored hidden,
frustum, sort, alpha/override, base/additional, skin and decal decisions
before it queues polygon/category work. Original `ww3d.cpp` exclusively
defines WW3D static state; no copy of those defaults remains in the Linux
support TU. Original `static_sort_list.cpp` owns deferred replay and
`WW3D::Flush` retains mesh → configured SHD → static-sort → sorting order.
Physical sorting and material/device commands remain typed unavailable until
slices 05–06; no full pass or successful frame is asserted by this slice.

Owned W3D fixture checks hidden suppression, default-camera rigid frustum
rejection, skin's authored frustum exception, static-sort deferral/replay,
alpha override publication, additional-passes-only suppression and authored
alpha-shadow exception restoring base passes, original
no-camera/no-task flush, and source entry reaching a typed first physical
edge. `original_w3d_cpu_identity` and provider-removal control now require
the original WW3D, static-sort and sorting-renderer translation units at
compile and link boundaries. Source changes remain in mutually exclusive
CPU versus original native configurations; no dual object appears in the
test link map. `sortingrenderer.cpp`'s CPU physical edge throws typed and
must be replaced by authored sorting behavior before slice 06 acceptance.

WWShade configuration: original `shdlib.h` compiles `SHD_FLUSH` to a no-op
without `USE_WWSHADE`; neither the authored original project nor the Linux
configuration defines that symbol. Read-only aggregate classification of
20 retail BIG archives and 4,438 W3D entries found zero top-level
`W3D_CHUNK_SHDMESH` (0xB00) chunks and 3,386 ordinary mesh chunks. No
retail paths, selected filenames, bytes or hashes were retained. The
disabled branch does not eliminate original regular shader/material/effect
semantics, which must be accepted with full pass scheduling in slice 06.

Validation: GCC and Clang Debug complete builds; 136/136 asset-free non-UDP
CTest cases each, plus both preexisting local-UDP tests per compiler outside
the socket sandbox; GCC and Clang focused ASan+UBSan fixture runs; focused
graph, original source identity and provider-removal tests; dependency
ledger hash freshness and `git diff --check`. Retail frame, SDL_GPU Vulkan,
validation-layer and visuals remain slices 08–09.
