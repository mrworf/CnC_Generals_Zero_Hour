# M22 08E: generated Linux load-screen progress evidence

The exact original `GameLogic` source boundary is intentionally narrower on
this OS than the prior mode-owner aggregate: its non-Windows factory consumes
the save flag and returns no owner before platform-only selection logic. This
slice does not expose, enumerate, construct, or infer those unavailable
platform routes, and it does not read retail content, map inputs, paths,
hashes, layouts, audio, network state, or pixels.

The project-owned runtime probe accesses the private factory only for test
observation. Across two independent `GameLogic` generations it sweeps all
public game modes and both save flag values. Every factory result remains
null; bounded optional progress dispatch and deletion preserve the null owner
slot, with no GUI, display, provider, device, or window ownership. The
separate negative source audit rejects a missing Linux null guard, any owner
creation in that branch, a nonzero witness, and a post-progress source advance
that is no longer before the first map-INI call. Provider removal verifies
that the actual source `GameLogic` object is required by the passing probe.

Final-tree acceptance passed with a reconciled 259 non-GPU/non-LAN CTest
registration in every configured build:

- focused GCC and Clang progress/contract/provider-removal plus all ledger
  gates passed 6/6 in each compiler;
- full suites passed GCC Debug 259/259 (224.31s), GCC Release 259/259
  (130.10s), Clang Debug 259/259 (207.13s), Clang Release 259/259 (79.25s),
  GCC ASAN/UBSAN 259/259 (582.10s), and Clang ASAN/UBSAN 259/259 (454.40s).
  The canonical sandbox sanitizer wrappers use `detect_leaks=0` only for the
  established ptrace restriction;
- strict host `detect_leaks=1` passed with no leak report: GCC 23/23
  (103.08s) and Clang 23/23 (75.72s), both including the new lifecycle;
- physical Vulkan validation passed 9/9 and serial local-socket LAN passed
  4/4 in each GCC Debug, GCC Release, Clang Debug, Clang Release, GCC
  sanitizer, and Clang sanitizer build.

Subsequent map-INI and map loading remain 08 work.
