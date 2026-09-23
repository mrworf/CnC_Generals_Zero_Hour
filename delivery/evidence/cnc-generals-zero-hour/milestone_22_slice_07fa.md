# M22 slice 07FA evidence: source map-frame traversal

The source display owns `W3DShaderManager` for its WW3D lifetime. A generated
read-only map now records the original order `W3DShroud::render →
W3DView::updateView → RTS3DScene terrain traversal → two HeightMap draws`.
Only the published primary-scene map terrain is admitted; detached terrain,
foreign siblings and active water reject.

`original_w3d_terrain_map_frame` proves initial full update, stationary
re-entry, injected draw rollback/retry, shroud-filter rejection/retry,
detach/re-attach, active-water rejection, two generations and zero
edge-teardown resources. Map-less terrain update/presentation remains an
explicit no-op regression; shroud material verifies that no stale edge remains
published after its fixture ends.

Acceptance completed: focused coupled GCC/Clang Debug controls are 10/10 each;
all six configured non-GPU/non-retail/non-LAN suites pass (GCC Debug 200/200,
GCC Release 200/200, Clang Debug 200/200, Clang Release 200/200, GCC
sanitized 200/200, Clang sanitized 200/200). The sandbox's ptrace wrapper
prevents LeakSanitizer, so those full sanitizer runs use `detect_leaks=0`;
host `detect_leaks=1` coupled map/owner suites pass 7/7 for both GCC and
Clang. Host serial LAN is 4/4 in every configured build.
