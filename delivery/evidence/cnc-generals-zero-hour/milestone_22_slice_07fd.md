# M22 slice 07FD: active original bounded decal-shadow route

The CPU-only original path now admits exactly one `SHADOW_DECAL` issued by a
published generated `W3DModelDraw` while the explicit 07FD profile, a loaded
owned map, and the primary display shadow manager are active.  The manager
owns the actual original DX8 four-vertex/six-index decal buffers, source
material and texture state.  The decal is deliberately a source owner rather
than ordinary scene geometry; `RTS3DScene` skips that caster and queues the
manager render after map terrain and tracks, before the accepted translucent
water route.

The generated Recording probe covers the real source `allocateShadows` and
`releaseShadows` calls, exact buffer range and ordering, disabled decals,
volume/projection/none, duplicate/foreign/detached controls, injected
create/upload/draw failure with retry, two complete generations and resource
teardown.  Default production behavior is unchanged: the profile is opt-in,
and volume, projection, particles, physical decal pixels and retail shadow
families remain explicitly unavailable or deferred to later slices.

Acceptance passed on the final tree:

- Exact non-GPU/non-LAN CTest suite: 205/205 in GCC Debug, GCC Release, Clang
  Debug, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan.  The two sandbox
  sanitizer broad suites use `detect_leaks=0` because their ptrace wrapper
  cannot collect LeakSanitizer diagnostics.
- Host GCC ASan+UBSan with `detect_leaks=1`: coupled view/map/tracks/water,
  source-owner and active-decal suite 6/6.
- Host Vulkan validation-layer `original_w3d_view_scene_bgfx`: pass.
- Host serial loopback LAN suite: 4/4 in each of the six configured builds.
- GCC and Clang identity/provider/ledger controls: 5/5 each; direct ledger
  validation and `git diff --check`: pass.

The host checks are separate from the sandbox broad suite because sandboxed
processes cannot access the Vulkan device or UDP sockets.
