# M22 slice 08P0C1A: atomic visible tree frame

The CPU-only terrain tree owner now stages the source breeze-version, pause,
sway interpolation and camera-sphere cull phase before publishing a visible
frame. A changed frame rebuilds visible-only XYZNDUV1/index geometry and
Recording buffers; a fully culled frame owns no geometry buffers. Camera
sort keys are recomputed on visibility re-entry. The GameClient RNG stream is
previewed only for a new, unpaused breeze version and committed after all
fallible geometry, atlas and Recording work. Failed preflight or publication
retains the prior frame, tree/type identity, source pointers, resource counts
and RNG. Move and removal invalidate the visible frame; last-user type
removal rebuilds the surviving atlas on clean retry. Physical tree factory,
terrain draw, unit interaction and optional decals remain closed.

The generated two-generation fixture covers two model types, visible/cull/
re-entry transitions, source sort-key distinction, exact previewed sway-slot
and RNG sequence, pause/resume, move, type removal, provider loss, atlas retry
and immediate owner-removal residuals. It injects edge-mismatch and nonfinite
sort-key rejection before RNG/GPU/frame consumption, cull and publication
rejection, geometry/vertex/index faults, and Recording buffer creation/upload
failures. The active GPU edge cannot be nested to fabricate a true stale
generation; the focused injected edge-mismatch branch reaches the exact
preflight rejection and checks unchanged accepted state. An audit found the
legacy `Vector3` default constructor does not initialize elements, so the
new frame explicitly zeros sway arrays. A paused first frame verifies zero
sampled sway and no GameClient RNG consumption. All temporary diagnostics
were removed before final gates.

Final-source validation: six complete GCC/Clang Debug, Release and
ASan+UBSan builds passed. Six canonical nonretail suites passed 267/267 each
(`-LE gpu|lan|retail`; sanitizer suites isolated with
`ASAN_OPTIONS=detect_leaks=0`). The focused generated terrain witness passed
1/1 on native GCC and Clang. Strict host LSan with
`ASAN_OPTIONS=detect_leaks=1` and no UBSan override passed the two focused
display-owner/terrain probes 2/2 on GCC and 2/2 on Clang. Physical Vulkan
display/bootstrap/map passed 3/3 on GCC; display/map passed 2/2 on Clang.
Serial host LAN passed 4/4 in all six configurations. The original
dependency ledger and `git diff --check` passed. The unrelated renderer
diagnostic remains unstaged.
