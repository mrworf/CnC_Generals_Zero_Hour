# M22 slice 08P0C2A0: exact reflected source-block origins

Implementation against parent `562364b51428d1632b9327cd5088174680d5df92`.
Acceptance is complete; this artifact does not admit the tree program,
scene draw, factory, retail or frame-command transaction.

## Independent correction and publication

The old public-bgfx loader inferred a source-block base from its first live
field. The existing original fragment has declared origin 0/extent 224, although
its first live field is byte 64. Stage integer arrays remain at 128/160, alpha at 96
and fog at 64/80; no caller/source UBO is compacted or repacked.

One bounded CPU-only decoder is shared by build emission, offline closure and
runtime admission. Compiler SPIR-V root/source-member offsets and std140 sizes
are authoritative. A canonical `zh-bgfx-uniform-layout 1` sidecar records stage,
source-block identity/binding/origin/extent and is generated/installed beside
every existing shader binary/JSON manifest. Runtime compares the sidecar with
the loaded compiler layout and source binding manifest before native creation.
Raw reflected names, exact offsets, kind/count/register extents, descriptor
stage/binding, std140 alignment and bounds are checked before normalization.
Entire compiler-eliminated UBOs add no runtime reads. Identifier normalization,
compiled shader bytes, descriptor slots and public caller payloads are unchanged.

Only actual source-block origins are subtracted. Post-native UniformInfo also
must match the admitted kind/count and declared extent; an owned candidate guard
retires the native shader on rejection or exception before opaque publication.
Prior accepted shader/program/buffer/target identities are not replaced.
Native bgfx create failure has no deterministic safe injection seam here; the
existing invalid-handle return and candidate cleanup paths remain explicit.
No allocator changes or original/retail object-layout/vtable changes occur.

## Generated controls and strict boundary audit

Two-generation CPU controls use actual compiler outputs plus modeled layout-only
boundary modules. Positive maxima are 4 blocks, 256 fields/block, 1024 reflection
records, 32 recursive layout levels and 4096 array entries/64KiB extent. Negatives
cover bound+1, recursive cycles, duplicate blocks/field names/member offsets,
overlapping extents, mis-sized registers/type/count, stage/root descriptor and
binding mismatch, unknown blocks, absolute underflow/overflow, malformed and
truncated envelopes, trailing bytes, missing/noncanonical/version/stale sidecars
and invalid/duplicate/oversized source-binding manifests. Rejection preserves
the prior output layout/map, not a partial candidate. New value members have
explicit initialization; fixture byte layouts have size/offsetof assertions.

The isolated physical witness uses two complete device generations. Its owned
shader proves dead prefix, interior holes, matrices, integer-selected arrays and
nonzero later source-block origins across bindings 0/2 and 0/3. Exact original
fixed-function shaders separately prove a nontrivial atlas quadrant, stage
integer controls, alpha-test rejection and fog values using unchanged 800-byte
vertex and 224-byte fragment records. Missing/truncated/oversized sidecars,
stale/duplicate binding/stage manifests and forged reflection leave accepted
resource counts and target pixels unchanged. Clean retry returns a distinct
shader while the accepted program still renders identical pixels. All resources
retire to zero before the next generation. This standalone renderer fixture
does not link original global allocation overrides; original-engine host
fixtures retain shipping service initialization before bgfx worker creation.

No temporary diagnostics or slice-08/C2A trial wiring remains. The unrelated
`tests/renderer/test_bgfx_device.cpp` diagnostic stays untouched and unstaged.
Existing original terrain/FX source hashes remain accepted and unchanged.

## Classified correction and refreeze

Late generated-only localization proved an admission defect: a present foreign
Uniform variable was skipped and therefore classified as an optimized-out UBO.
The decoder now distinguishes actual absence from a present unrecognized root
and rejects the latter before native publication. Genuine compiler-eliminated
`post_effect.vert` remains admitted with no runtime reads. Duplicate known roots
and mixed known/foreign roots in both traversal orders reject candidate-only.
A physical debug-name-only mutation leaves the live compiled UBO in place and
supplies an empty-root sidecar; it must reject without changing accepted
resources/pixels. No source shader, fixture expectation or assertion is weakened.

All pre-correction results, including completed native canonical runs, are
superseded and are not final acceptance evidence. The queued old Release run
was intentionally interrupted. Clang sanitizer recovered a premature-end
dependency-log category through a complete rebuild, without actionable source
errors or disk exhaustion; no mixed/stale objects are accepted. All six complete
builds and every final gate were restarted on the refrozen source.

Refrozen decoder hash:
`de99d3a50bf6b1b488dfecd83831e1ab5a41354bbb85fd792c382207f5a9c6fb`.
Loader hash: `4b6d26cf02e23fdfa9a0f1b88b03fcfb668b09248bc774139fc4b3fc1073a810`.
Generated witness hash:
`0becece351633721d1347d37c6e026097f974dd588508df79edfe077e6f573c5`.
Physical driver category is NVIDIA 615.71.09.

## Refrozen acceptance

All six complete builds pass. Focused origin/closure/source/headless controls
pass 4/4 in GCC and Clang native and both ASan+UBSan configurations; the final
Clang sanitizer focused run occurred after its complete rebuild. All four
native canonical nonretail suites pass 268/268: GCC Debug 281.86s, Clang Debug
262.82s, GCC Release 147.62s and Clang Release 99.90s. GCC ASan+UBSan canonical
passes 268/268 with `ASAN_OPTIONS=detect_leaks=0`, total 869.76s and terrain
attachment 263.60s, without finding or timeout. The serial Clang ASan+UBSan
canonical passes 268/268 with the same environment, total 690.27s and terrain
attachment 222.42s, without finding or timeout. All six final-source builds
and canonical suites therefore pass; no superseded result is credited.

Strict host LSan passes origin/display-owner 2/2 in each sanitizer configuration,
serial and host-escalated with exactly `ASAN_OPTIONS=detect_leaks=1`, no UBSan
override: GCC 10.69s/6.28s, Clang 7.53s/5.76s. New physical origin controls pass
in both native compilers, each with two device generations. Established host
Vulkan passes GCC display/bootstrap/map 3/3 and Clang display/map 2/2; validation
has no error or VUID category. Serial host LAN passes 4/4 in all six
configurations. Ledger and diff checks pass. Final source hashes above remain
unchanged; no production or test edits occurred after refreeze.

Read-only final source/diff review passes. The shared metadata/runtime decoder
and post-native candidate guard remain independently scoped; no tree program,
original terrain/FX behavior, scene/factory or retail admission is opened.
AGENTS records the general compiler-origin versus first-live-field lesson.
The unrelated renderer diagnostic is preserved and excluded from exact staging.
Delivered against parent `562364b51428d1632b9327cd5088174680d5df92` by
`delivery: M22 08P0C2A0 bind exact shader block origins`. C2A is dependency-next
and must revalidate its trial against this accepted owner.
