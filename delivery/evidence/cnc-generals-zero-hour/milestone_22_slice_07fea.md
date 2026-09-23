# M22 slice 07FEA: original particle-provider closure

The full-draw CPU target now compiles the canonical original
`W3DParticleSystemManager`.  The explicit `ZH_M22_PARTICLE_PROFILE` selects it
only for its generated-map probe; the ordinary factory remains the Linux
manager.  On a bounded map frame, the source scene queues exactly one original
particle request after terrain and water, drains it through the original empty
`W3DSmudgeManager`, and releases its ready latch.  A missing edge or smudge
owner, and an active generated particle system, fail closed before geometry;
the latter tears down and retries cleanly.  Point, streak, volume, retail and
smudge geometry remain deferred to 07FE.

Final-tree acceptance:

- Fresh exact non-GPU/non-LAN CTest: 206/206 in GCC Debug (99.97s), GCC
  Release (64.90s), Clang Debug (95.02s), Clang Release (46.77s), GCC
  ASan+UBSan (256.72s), and Clang ASan+UBSan (213.41s).  Broad sanitizer
  suites used `LSAN_OPTIONS=detect_leaks=0` for the established ptrace
  compatibility limitation.
- Host focused GCC ASan+UBSan with `detect_leaks=1`: coupled source
  view/tracks/water/shadow/provider suite 5/5; host Clang ASan+UBSan provider
  passed from an identical `/tmp` copy because the environment denied execve
  only from the Clang-sanitized build path.
- Host Vulkan `original_w3d_view_scene_bgfx`: pass.
- Serial loopback LAN: 4/4 in every configured build.
- GCC and Clang identity/provider/ledger groups: 4/4 each; direct dependency
  ledger validation and `git diff --check`: pass.
