# M22 plan 01 slice 07E0D3: retire source WW3D static caches

## Outcome and dependency

Requires accepted original WW3D source-frame and display owner slices. Two WW3D process-static allocations survive an otherwise completed original display lifetime: a 10,248-byte `MultiListNodeClass` slab retained by its static AutoPool after all scene nodes are returned, and a nine-byte texture-statistics `StringClass` buffer created by `Record_Texture_End`. Source-address traces and device-only/display-only/full-factory contrasts identify them independently of four bgfx/driver process-scoped allocations. This slice retires only the source caches at CPU WW3D shutdown; it does not change device/driver allocation policy.

## State, re-entry and failure

Add an explicit empty-block retirement operation to the existing `ObjectPoolClass`/`AutoPoolClass` templates without changing class data layout. It frees slabs only when every node has returned, resets counters/free-list state, and leaves an active pool untouched (negative control). CPU `WW3D::Shutdown` invokes it after scene/asset teardown so the next original display generation can allocate normally. The original native `W3DDisplay` destructor calls `Debug_Statistics::Shutdown_Statistics` before asset/WW3D teardown; restore that same call and order in its CPU branch to release the texture-statistics buffer. Native Windows shutdown remains unchanged. No active list node, render object or GPU resource may be freed by the cache-retirement operation.

## Entry point, boundaries and implementation

Entry is original `W3DDisplay` teardown, then `WW3D::Shutdown`. Only the CPU-only branch changes. Empty shutdown is idempotent; active pool nodes make retirement return false without touching memory, and subsequent release permits retirement. This is internal lifecycle policy, so user authorization does not apply. Source surfaces: `WWLib/mempool.h`, `WW3D2/ww3d.cpp`, CPU `W3DDisplay.cpp`, focused original rendering tests, and dependency ledger. Unsupported map/effect/factory behavior stays pending. A retirement refusal must not turn ordinary teardown into an exception; the active-node negative is tested directly and original source teardown must return its nodes before pool retirement.

## Acceptance

Direct positive/negative pool tests cover empty retirement, active-node refusal, second allocation generation and zero-count re-entry. Original display/view-scene source and physical tests cover two display lifetimes and four device generations; a real GameClient production-order diagnostic verifies no source-owned delta after original teardown while the same GPU device is still alive. Five fully rebuilt non-GPU suites with leak-capable GCC/Clang sanitizer controls, host Vulkan repeated validation, source/provider/ledger checks and an independent evidence commit are required. The four device-only process-scoped allocations remain separately measured, not silently exempted from a source owner count.

## Result and commit boundary

The independent D3-only tree passes the direct active-node/empty-pool/re-entry tests, five full 194/194 non-GPU suites, and GCC Debug/Clang Release 30/30 fresh-process Vulkan display/view repetitions. The pending D1/E0E work and unrelated renderer diagnostic were isolated during acceptance. In a production-order diagnostic with the pending opt-in factory, source display teardown drops the live count from the previously observed 28 to 26, matching the separate device-only 26 control; the original non-GPU baseline remains 22 and its absolute assertion is not weakened. Evidence: [07E0D3](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07e0d3.md). This slice commits only the pool/statistics shutdown behavior, tests, ledger and plan/evidence; factory publication and the +4 device/driver policy remain pending.
