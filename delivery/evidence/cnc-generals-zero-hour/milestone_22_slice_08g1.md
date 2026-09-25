# M22 slice 08G1: zero-based veterancy flag safety

The original veterancy flag helpers now use the declared zero-based enum
directly: the four valid levels occupy bits zero through three. The named
invalid value returns false or leaves the incoming mask unchanged before any
shift. `ALL` and `NONE` retain their established meanings. Generated coverage
checks every set/get/clear operation and drives mixed modifiers through the
original INI/module parser, including exclusion of the first level.

Acceptance on the final production state:

- GCC and Clang Debug focused source-owner, parser and retained 08F
  construction gates passed 3/3.
- All six configured builds completed. Canonical non-GPU/non-LAN/non-retail
  suites passed 261/261 in GCC Debug, Clang Debug, GCC Release, Clang Release,
  GCC Sanitized and Clang Sanitized. Sanitizer broad suites used
  `detect_leaks=0`; after the final test-only invalid-value correction, focused
  final-tree gates passed 3/3 in every configuration.
- Strict host LeakSanitizer passed the focused owner gate 1/1 with both GCC
  and Clang. The named invalid enum control is sanitizer-clean.
- Public bgfx Vulkan validation-layer controls passed 2/2 on the physical
  device. Serial host-loopback LAN passed 4/4 in all six configurations.
- Dependency-ledger validation and `git diff --check` pass.

The initial redacted probe category that motivated this slice is resolved.
Retail roots remained read-only; no private path, identifier, filename, byte,
hash, image or raw output is retained. Audio-event initialization remains the
separate 08G2 prerequisite, and the scene borrow/unwind transaction remains
owned by slice 08.
