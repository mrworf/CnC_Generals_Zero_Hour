# M22 plan 01 slice 08Q0: source-owned house-color texture recoloring

Status: ready after 08L2R1; one coherent texture/remap/cache lifecycle owner.
Parent checkpoint: `72751dcb55bbdddc914b8187ceedaba2c5ccd42c`.

## Outcome, dependency order and scope

Depends on 08L2R1, accepted 08P0, 05B1/05B2A/05B2B1 canonical image/bitmap/
texture-task providers, and accepted 08P0C2D0B2R2 defined filter-tuple admission.
Accepted 08P0 transitively supplies resource transaction and source-pin lifetime
contracts. Before slice08 resumes, original W3DAssetManager house-color cloning
must own exact bounded CPU pixels, native remap decisions, mip generation,
candidate GPU upload and texture/prototype cache lifetime through failure/retry.
No general SurfaceClass/D3D emulation, new image parser, shader substitution,
GPU readback, retail selector-specific behavior, shadow expansion or frame
acceptance. Generated CPU/Recording/physical data has no authorization gate;
retail inputs remain runtime-only and read-only under the fixed-category policy.

## Read-only owner audit and implementation-ready disposition

Native `Recolor_Texture_One_Time` copies level-zero SurfaceClass pixels, selects
palette-only for source name character3 D/d and alpha-mask/hue remap for A/a,
then constructs a new surface-backed TextureClass. Other selectors copy unchanged;
do not invent the general palette branch here. Procedural `!` names return null.
The native surface-backed constructor is procedural, initialized, nonreducible,
uses the source requested mip count and native surface-to-texture mip generation.
Native min/mag/mip/U/V filter values are copied; W keeps the constructor default.
The color-keyed lower-case cache owns one reference and the caller another.

Existing Linux TextureLoadTask Begin/Load methods already select canonical
TGA/DDS, original reduction/format/orientation and BitmapHandler pixel decisions;
staging bytes are discarded after normal upload, so an accepted GPU handle is
not a CPU surface. Add a narrowly scoped CPU detached staging operation to that
same task: invoke its source Begin/Load decisions without GPU creation, source
initialization, missing-texture fallback, upload or cache publication. Retain only
owned bounded output needed by this recolor operation. Check all metadata, pitches,
products, mip counts and the existing 64MiB decoded budget before allocation.
An already initialized source must match its accepted dimensions/format/reduction;
source LastAccessed, filter, handle and initialized state stay unchanged on
rejection. Required missing/malformed inputs fail closed, never recolor fallback.
Compressed/non-2-or-4-byte surface formats unsupported by native recoloring
remain explicit negatives rather than a new decoder or guessed conversion.

Use a private bounded CPU surface value for the recolor operation, not a public
SurfaceClass API or class-layout fork. Preserve canonical PixelSize/Convert_Pixel,
the native sixteen-entry house-color scale, palette matching, inverse-alpha mask,
RGB/HSV hue/saturation decisions and REAL_TO_INT packing. Expose native remap
loops through one shared source body for native and CPU configurations; do not
reimplement color mathematics in the device edge. Generate requested lower mips
from recolored level zero with the authored surface constructor's BOX filter
(`DX8Wrapper::_Create_DX8_Texture(surface)` uses D3DX_FILTER_BOX for the copy and
lower-mip generation). Use existing BitmapHandler primitives only where their
packing and box arithmetic match that rule; pin exact integer/edge behavior in
generated reference controls, not separately recolored source lower mips.
Reject short/overlong names, unsupported format, malformed
palette extents, nonfinite values and overflowing dimensions before effects.
Checked color-key name construction replaces unchecked fixed-buffer formatting
on the reached path while preserving native cache keys for valid names.

## Publication, errors, cancellation and cache lifetime

Validate the entire copied filter tuple using R2 before GPU work. Build all CPU
bytes and candidate source TextureClass state locally; create/upload all mips
through accepted OriginalGpuEdge methods; publish exact texture mapping and
color-key cache only after complete upload. Every acquired source/device/cache
unit has one reverse release. On create/upload/map/hash failure restore the
pre-attempt cache/resource baseline and permit identical retry. Cache hit returns
the exact accepted identity without decode, upload or duplicate reference owner.
Same source/different colors remain isolated; alias multiplicity, unused texture
release, Free_Assets, display reset/re-entry and device generation retirement use
existing owners, not a new deferred-destruction queue.

The enclosing custom `Create_Render_Obj` transaction must unwind cloned render
refs/material/texture replacements, newly inserted color textures and prototype
publication if any recolor or allocation fails. Preserve accepted sibling caches
and original source prototype/texture bytes. Restore load-on-demand state exactly
on every exit. Pre-admit prototype vector growth before hash publication (or
equivalent exact rollback); no half-published prototype. Manager-local bounded
transaction bookkeeping may track only entries created by this attempt, with
no serialized field/vtable/Windows ABI changes. Active-frame mutation and stale/
foreign Edge generation reject before resource/cache mutation; idle retry works.
Any independently required owner not supplied by these accepted contracts stops
at an architecture checkpoint, not an autonomous split.

## Generated controls and reproducible validation

Add a registered generated house-color witness `original_w3d_house_color` using
the actual W3DAssetManager mesh/HLOD/custom-create route and exact source texture
providers. Add identity/provider-removal controls named
`original_w3d_house_color_identity` and `original_w3d_house_color_provider_removal`.
No retail bytes or private names are fixture inputs. Cover 16/32-bit native remap
packing where supported, D/d, A/a, unchanged selector, inverse-alpha threshold,
palette matches/nonmatches, orientation, level-zero and generated-mip bytes,
all filter/default semantics, zero-color native behavior, procedural rejection,
short/long/unterminated names, required missing/malformed and unsupported formats,
dimension/pitch/count/byte maxima and bound+1. Cover every allocation/upload/cache/
prototype publication fault with immediate exact residuals and clean retry;
cache hits, multiple colors, shared aliases, sibling isolation, reset/removal and
two generations. Actual model-color notification must compose with R1 rollback.
Physical witness samples nonuniform recolored texture and lower mip content
through original source draw; require unchanged prior pixels on candidate fault,
same successful retry pixels and validation-clean zero teardown.

Configure the six established presets and build the registered witness target
plus `zh_original_w3d_full_probe` and minimal/headless consumers. Exact focus on
GCC/Clang native and sanitizers:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_house_color|original_w3d_house_color_identity|original_w3d_house_color_provider_removal|original_w3d_texture_decisions|original_w3d_texture_identity|original_w3d_texture_provider_removal|original_w3d_generated_construction|original_w3d_modeled_volume_ready|original_w3d_tree_module)$' --output-on-failure -j1`.
Verify every registered ID before claiming a gate. Both strict host LSan runs use
`ASAN_OPTIONS=detect_leaks=1` and that exact focus, no UBSan override. Generated
physical command, after its registered wrapper is persisted:
`ASAN_OPTIONS=detect_leaks=0 python3 tools/run_validation_clean.py build/<preset>/original_w3d_house_color_tests --gpu` on both native and sanitizer toolchains.
Freeze final source, run six complete builds/six serial canonical nonretail suites
`-LE 'gpu|lan|retail'`, established native Vulkan, serial LAN4/4 all six, ledger/
header/link ABI/diff audits. Inspect complete sanitizer logs, not exit alone.

## Acceptance and exact commit

Only original authored remap/mip/cache behavior is opened; every fault withdraws
the candidate with accepted sibling/source identity intact and exact residuals.
One Q0 implementation/tests/evidence commit:
`delivery: M22 08Q0 close house-color texture recoloring`.
Preserve all unstaged active08 edits and the renderer diagnostic. Then mentally
rebase that unchanged trial onto accepted R1/Q0 and rerun the bounded redacted
retail probe before claiming any scene/frame success.
