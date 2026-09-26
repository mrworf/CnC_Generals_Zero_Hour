# M22 plan 01 slice 08P0C2B: exact shroud content and stage binding

## Goal, dependency and boundary

After accepted C2A, D0's Recording/bgfx idle/frame capabilities and
[B0](milestone_22_plan_01_slice_08p0c2b0.md)'s complete delayed-source transaction,
restore native `W3DShaderManager::setShroudTex`
stage/resource/transform semantics and the existing W3DShroud content update on
the CPU shipping route. No tree draw,
C1 advancement, scene preparation, decal, physical factory or retail admission
is included. This generated local renderer operation has no authorization gate.

## Source binding and lifecycle

Resolve the exact active terrain/map/shroud texture, positive finite cell size,
texture dimensions and draw origin. For native stage 1, preserve camera-space
position coordinate selection, COUNT2 transform, TEXTURE/CURRENT MODULATE color
and SELECTARG2 alpha. Construct the canonical inverse-view × origin offset ×
scale transform, with one-cell border offset and source row/matrix orientation.
The tree vertex program uses its own unswayed world position and c32/c33;
preserve subsequent tree-loop UV-index/transform-disable state in source order
and do not double-apply the camera-space transform. Do not bind an unshrouded
pass or replace the exact source texture with a placeholder.

B remains exact shroud semantics only: use B0's complete binding transaction rather
than adding another ref/map/revision/sampler checkpoint here. Do not fold
frame-command rollback into shroud admission. Readiness must be side-effect free. Preflight every owner/resource and matrix
before delayed state mutation; preserve the existing asserting/query semantics
outside any narrow readiness peek. On provider/resource failure preserve prior
accepted source stage/transform/resource identity; binding failure unwinds
owned references and state, and retry succeeds without reset. Source owner
removal and active-generation mismatch reject before partial binding. A new
public capability owner, if independently needed, requires another explicit
architecture checkpoint before implementation; the scope freeze forbids an
autonomous split. No tree registry, RNG or effects may change.

## Approved pre-production audit refinement

The explicit architecture checkpoint approves folding bounded content update
into C2B, not adding a prerequisite: the existing W3DShroud update/render owner
must supply current fog pixels before exact binding. C2C already assumes that
camera/shroud update is accepted. The previous CPU render applied a resident
texture without uploading its cells; remove that incomplete readiness claim.
Accepted B1/D0 idle transactions provide regular-texture COW upload rollback;
B0 remains a later, nonoverlapping selected-stage binding transaction. If these
capabilities prove insufficient, stop at another architecture checkpoint.

## Exact bounded source content update

Retain native full-cell copying (the shipping render overrides visible bounds
to the whole map): origin zero, logical cell (x,y) at texture (x+1,y+1), and a
one-cell border on all four sides. Clamp cell and border levels to shroudAlpha.
Normal source pixels truncate each channel of level*(colorByte/255.0f), then
quantize R/B to five bits and G to six. Level 255 overrides all RGB to white.
The existing A8R8G8B8/BGRA8 transport expands an n-bit channel q to
floor((q*255 + floor((2^n-1)/2))/(2^n-1)); alpha is 255. This preserves normalized
RGB565 semantics without inventing an unsupported RGB565 transport format.
For the native debug/internal fog mode, cast the normalized Real RGBColor
members directly to Int as the native source does (do not use getAsInt or
multiply these debug RGB members by 255), then take their high nibbles. Valid
normalized [0,1] input therefore produces zero debug RGB. Quantize (255-level)
alpha to its high nibble and expand each nibble by multiplication by 17, preserving
ARGB4444 semantics (the normal-mode white override does not apply here).
Physical assertions use these exact transport bytes, not unquantized colors.
Do not introduce time/RNG interpolation: no DO_FOG_INTERPOLATION definition is
enabled in the accepted CPU profile; logical current/final levels remain the
existing immediate update. Validate color/alpha inputs before numeric conversion.

Before allocation, references, Edge publication or any device call, preflight
finite positive cell sizes, coherent cell/texture dimensions, current/final
providers, filter/profile/global services, active idle owner generation, native
extent/row-pitch limits and checked complete BGRA payload size <=64 MiB. Reject
overflow/bound+1 and malformed dimensions without mutation. Build the complete
bounded pixel payload before opening the idle journal. Content upload must not
apply/select stage 0 or consume a source marker/revision, LastAccessed, C1 or FX.

Use owner-local consistent Linux-only desired/accepted content epochs plus
accepted color/mode/generation, initialized explicitly and reset to the declared
baseline. Changed cell/fill/border inputs mark dirty before mutation; unchanged
inputs need not advance. Guard epoch overflow before mutation. Global color/mode
changes require a new upload even without cell changes. ReAcquireResources may
retain its explicit allocation semantics but leaves content dirty; binding must
reject until render accepts that exact resident generation/content. Release,
reset and removal clear accepted readiness. Clean repeated render is a no-op.
Windows declarations/layout and serialized/virtual boundaries remain unchanged.

Existing resident upload uses B1/D0 idle COW with the same logical TextureClass
and handle: checkpoint/create/upload/publication/commit failures preserve exact
accepted pixels, native ownership, source identity, epoch, dirty intent and
unselected state for retry. First render stages a private source/native candidate,
uploads and completes all fallible source-map publication before commit, then
publishes m_pDstTexture/accepted epoch with no-throw assignments only. A narrow
exact candidate-publication withdrawal on the existing Edge owner removes only
that candidate's map/ref units before device abort, without source revision,
callbacks or ordinary destroy; the journal owns native rollback/retirement.
Never release an accepted resident texture on upload failure. Commit/abort must
match the exact token and generation; accepted B1 ordinary-boundary drain/wait/
shutdown retires COW candidates, never during commit/abort. No new public
independent resource owner or broad allocator/format repair is introduced.

Prove first-allocation and resident update faults, nonuniform cells, exact border
and both quantization modes, color/alpha changes, stale/dirty rejection, repeated
clean render, failed update then identical retry, release/reacquire, removal and
two complete generations. Recording checks every byte and unchanged accepted
pixels on every injected failure. Physical Vulkan samples at least two unequal
interior cells plus the border from the exact bound resident texture and proves
fault rollback/retry; a uniform/empty texture is not acceptable evidence.
Include an explicit valid normalized-debug-RGB zero-byte control together with
nonzero nibble-expanded inverted alpha, so this source direct-cast distinction
cannot be silently replaced by normal-mode byte extraction.

B0 aggregate is accepted at `7dfb6bb496af997fbae83c5cc790866d711eeaa4`,
with B0B executable payload `bbe0b93a9842fbdc080eae346ccf645e044344cc`.
Native `setShroudTex` is absent from the Linux shader-manager branch, but the
accepted W3DShroud already owns resident regular TextureClass reacquisition.
Do not create/reacquire/render/init a shroud inside binding readiness.

Add only a narrow static Linux published-shroud peek on the existing
`s_emptyTerrainVisual` owner. Compare singleton/publication addresses before
dereferencing a global terrain/visual pointer; require exact terrain/map/scene
relationships and resident source texture ownership in the active generation.
Return borrowed state only: no ref increment, map/registry allocation, new owner,
loader access or serialized/virtual ABI change. Null/stale/removed publication
rejects before resource/provider dereference or any stage mutation.

The native authored Set_Transform transposes the view before `_Get_DX8_Transform`
reads D3D row-vector bytes. CPU transport retains authored WW column-vector
matrices: transpose-equivalent shroud transport is `scale * offset * inverse(view)`.
Translation stays in the WW final column. Prove the equivalence against an
independently formed native row formula with nontrivial rotation/translation,
shifted origin, unequal cells and border offset; do not silently reverse only
one factor. The tree c32/c33 constants continue using unswayed world coordinates,
and its later UV-index/transform-disable writes remain source-ordered.

Matrix4x4::Inverse is explicitly unfinished. Use a private bounded local 4x4
partial-pivot inverse solely in this binding helper, rejecting singular/nonfinite
input, overflow/nonfinite/unrepresentable results and invalid cell/extents before
opening B0. No general math repair or new owner is admitted. Stage 1 is the exact
tree profile; other stages and active frame modes fail closed.

After all readiness and matrix checks, use B0 to stage the native texture,
coordinate/transform/color/alpha sequence and selected-stage application. Abort
the exact owner and cancel/drain A pins at the ordinary inactive boundary on
failure, restoring prior identity and LastAccessed for clean retry. Successful
B0 commit has no fallible cleanup afterward: its acquired units transfer to A,
and the caller drains at the ordinary inactive boundary before another binding
attempt/frame/preparation. An unsuccessful terminal drain keeps A's accepted
queue/native/source publication for exact ordinary retry; do not rebind or replay
the committed source sequence. No C1/RNG/FX state is consumed here.

Exact focused commands, persisted before production:

`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_tree_program_tests original_w3d_source_reference_tests original_w3d_stage_transaction_tests original_w3d_gpu_edge_tests original_w3d_texture_decision_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_terrain_shroud_projection|original_w3d_shroud_data|original_w3d_tree_program|original_w3d_source_reference|original_w3d_stage_transaction|original_w3d_gpu_edge_failure|original_w3d_texture_decisions|renderer_recording_transaction|renderer_bgfx_transaction_resource)$' --output-on-failure`

This selects nine CPU controls, including the expanded real generated shroud
projection fixture and unchanged stage/lifetime regressions, on GCC/Clang native
and both sanitizer configurations. Strict host LSan selects these same nine with
exactly `ASAN_OPTIONS=detect_leaks=1`, no UBSan override. Retain every existing
fixture assertion; add exact content/stage/matrix/borrowed lifetime, all negative/fault
rollback and ordinary drain/retry across two generations. Six complete builds
and serial canonical suites, established physical Vulkan, six serial LAN and
ledger/header/diff remain the required final acceptance. No additional split
without the explicit architecture checkpoint required by the scope freeze.

Exact generated physical command (GCC/Clang native and both sanitizer builds,
host escalation and established graphical environment) is
`python3 tools/run_validation_clean.py python3 tests/original_rendering/test_w3d_terrain_shroud_projection.py --executable build/<preset>/zh_original_w3d_full_probe --source-root . --gpu`.
Extend this existing fixture with the physical selector and shipping-service
initialization before any worker, preserving all CPU assertions. Audit full
validation output, not exit status alone; keep generated output separate from
shipping shader/install closure.

## Planned implementation and acceptance surfaces

Expected surfaces: canonical shader manager CPU branch/types, original shroud
access/readiness if required, existing DX8 delayed state/transform transport,
existing Edge candidate-publication withdrawal, source identity ledger and
generated content/binding witness. Native Windows behavior/layout and serialized
state remain unchanged; consistent Linux-only owner fields are initialized and
audited across all archive consumers.

Generated positives exercise nontrivial camera/inverse view, shifted origin,
unequal cell dimensions, actual shroud texture/generation and the tree-program
constant/stage sequence. Assert exact color/alpha operations and matrices,
not only texture presence. Negatives cover null/stale/removed terrain or
shroud, singular/nonfinite matrix, invalid dimensions, missing/stale texture,
binding failure, unchanged prior state and clean retry across two generations.
Identify/persist exact focused commands before production. Run GCC/Clang and
sanitizer focused witnesses, six complete builds/canonical nonretail suites
(`-LE 'gpu|lan|retail'`, sanitizer `ASAN_OPTIONS=detect_leaks=0`), exact serial
host LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), established physical
Vulkan, serial LAN 4/4 all six, ledger and diff on final source. Commit one slice:
`delivery: M22 08P0C2B bind exact tree shroud stage`.
