# M22 slice 08G2: deterministic audio event playback state

All five ordinary original audio-event constructors now initialize the next
playback portion to the attack phase, matching the class's established reset
state. Copy and assignment preserve that initialized state, while the public
explicit override still selects another valid phase. Generated coverage
constructs every overload before playback and exercises copy, assignment and
override without an audio device or retail input.

The private-safe sanitized source-init check reports neither of the two fixed
undefined-behavior categories. It reaches the accepted configuration/family
and setup boundary with clean Recording teardown, then remains stopped before
scene admission as required. Retail roots stayed read-only and no private
path, identifier, filename, byte, hash, image or raw output is retained.

Acceptance on the final tree:

- Focused original audio-owner, generated scenario and retained 08F
  construction gates passed 3/3 in all six configurations.
- All six builds completed, and canonical non-GPU/non-LAN/non-retail suites
  passed 261/261 in GCC Debug, Clang Debug, GCC Release, Clang Release, GCC
  Sanitized and Clang Sanitized. Broad sanitizer suites used
  `detect_leaks=0`.
- Strict host LeakSanitizer passed the focused audio-owner gate 1/1 with both
  GCC and Clang.
- Public bgfx Vulkan validation-layer controls passed 2/2 on the physical
  device. Serial host-loopback LAN passed 4/4 in all six configurations.
- Dependency-ledger validation and `git diff --check` pass.

The remaining slice 08 work is the explicitly planned retail scene
borrow/unwind state correction and complete campaign/skirmish Recording
transaction; this prerequisite does not admit that selector or claim a frame.
