# M22 slice 07F9A evidence: source shroud projection prerequisite

## Delivered boundary

For the generated, read-only map route, the original `W3DShroud` now owns one
default-filter projected texture through the public `OriginalGpuEdge` and the
existing source `TextureClass` lifetime. `render(CameraClass*)` rejects absent
camera, inactive edge, uninitialized state and a nondefault filter before
publication. A texture-create failure leaves no projection owner or Recording
resource; a retry publishes one texture. The existing
`W3DShroudMaterialPassClass` installs and uninstalls precisely that source
texture, rejecting a missing or released projection.

The focused probe proves release/reacquire, material retry, two independent
owned-source generations, visual teardown, and zero Recording resources. It
does not begin a frame, submit terrain, activate tracks/water/shadows/
particles/smudges, consume retail input, or claim physical pixels.

F9A also repaired a stale F9-era test expectation: a missing map stream has
the F9 contract of `FALSE` with no map publication, not an exception. The
coupled stack-published display/visual/view globals now restore during assertion
unwinding, preventing the test fixture from leaking a stack display into the
GameClient destructor. This is test exception safety, not a product behavior
change.

## Validation

- Focused F9A and coupled view-scene controls pass GCC Debug, Clang Debug,
  GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan. Sanitizers used the canonical
  host settings `ASAN_OPTIONS=detect_leaks=1` and
  `UBSAN_OPTIONS=print_stacktrace=1`.
- Dependency ledger, full-draw identity, full-draw provider-removal,
  presentation identity, and presentation provider-removal controls pass on
  GCC and Clang Debug.
- Exact non-GPU/non-LAN suites pass 200/200 in GCC Debug, GCC Release, Clang
  Debug, Clang Release, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan.
- Serial local-loopback LAN suites pass 4/4 in those six presets. The restricted
  sandbox denies UDP socket creation; canonical host-loopback acceptance passes.

## Remaining route

07FA remains responsible for bounded map view updates and the source display
ordering `shroud → views → scene`. This slice does not establish a map frame,
terrain draw, pixels, resize/recreation, active sibling effects, factory
publication, or retail recording.
