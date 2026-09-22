# M22 slice 07E evidence: bounded original display/view frame

## Aggregate source behavior

- 07E0D commit `c84988d` gives canonical `W3DDisplay::draw` a bounded no-map
  frame through the published original `W3DView`, camera and `RTS3DScene`.
  Recording controls cover empty, rigid, detached, missing/stale owners,
  unsupported modes, abort and retry.
- 07E0E commit `e5eb031` publishes the original display, view and empty terrain
  in production startup order behind an explicit test profile. Its paired
  device-only control proves the original owners add no retained allocation,
  while device failure and partial-factory failure leave no published owner.
- 07E0F1 commit `6ed26c9` retires only drained polygon-task pool blocks, and
  07E0F commit `f9f6243` attaches a generated rigid object through the
  factory-owned asset manager and scene. The real GameClient update changes
  physical pixels; absence, malformed/missing input and teardown controls pass.

## Validation reconciliation

- The 07E0D public-bgfx oracle covers BGRA8 and RGBA8 at two extents over four
  fresh device generations, with empty/visible/detached pixels and explicit
  Khronos diagnostic rejection. GCC Debug and Clang Release each passed 30/30.
- The production-order empty and rigid factory profiles each passed GCC Debug
  and Clang Release 30/30 fresh-process repetitions. The exact five non-GPU
  GCC/Clang Debug/Release and ASan+UBSan suites passed 194/194 for the final
  child; leak-capable source controls, provider/ABI checks and the dependency
  ledger remained green.
- A fresh read-only PRE-008 entry check ran the existing original retail
  recording control and compared corpus metadata before and after: the control
  passed and metadata was unchanged. No private path, filename, hash or payload
  is recorded here.

This aggregate accepts only the bounded no-map display/view frame. Full 07 is
still pending map terrain, active shroud/tracks/water/shadows/effects and the
production-default full-behavior factory. Consequently slice 08 is not yet
entry-ready, and slice 09 retains the later retail visual,
validation-layer, resize and device-recreation gates.
