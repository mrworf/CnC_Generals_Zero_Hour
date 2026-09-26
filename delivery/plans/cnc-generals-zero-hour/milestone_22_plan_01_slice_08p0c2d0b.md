# M22 plan 01 slice 08P0C2D0B: public bgfx deferred frame transaction

## Goal, dependency and boundary

After D0A's bounded opt-in public transaction, implement that capability on
the Linux full-draw public bgfx route. C2C preparation and C2D source draw
depend on this real capability, not Recording-only success. SDL and other
devices remain explicitly unsupported/fail-closed unless source runtime
reachability later requires them. No private Vulkan/bgfx cancellation API,
tree advancement/factory, retail input or broad backend rewrite.

## Admission, deferred commands and native commit

Current bgfx begin touches/clears a native view and consumes order; draw
validates/allocates wrappers then immediately submits; end only destroys the
framebuffer. In the opt-in transaction, retain immutable bounded source-order
command/resource data without native view touch/clear/submit/present until
commit. Validate the entire generation/attachment/load/initialization,
shader/viewport/clear/index/layout/count/resource and finite-capacity contract
before irreversible native commands. Prepare every fallible native wrapper
and resource first; publication after the native commit point cannot fail.
An independently required prerequisite gets a plan-only split before edits,
never a partial-frame exception or weakened test.

Abort before commit removes candidates and restores accepted shadow bytes,
initialization, view order and identities; clean retry emits once. Stale/
removed providers and capacity fail before mutation. The admitted immutable
batch uses only pinned public bgfx operations and owned references, no source
registry/state access or fallible resource creation after submit begins.
Release candidate/retired native resources in bounded completion-safe order.
Ordinary nontransaction passes remain unchanged. A failed transaction cannot
mark targets accepted or present partial pixels; prior accepted contents stay
unchanged. Do not claim rollback after irreversible submission or disable
Khronos validation. Authorization is not applicable to generated tests.

## Surfaces, tests and acceptance

Expected surfaces: public bgfx transaction owner, narrow OriginalGpuEdge
capability admission/commit/abort, contract docs and generated renderer/
physical witnesses. Preserve the unrelated renderer edit by isolated paths/
hunks. Cover complete commit, partial command/resource and later staged-draw
faults, stale/removal, command/byte/view bounds+1, destruction/reset and retry
without duplicate draw/present/view consumption across two generations.
Physical Vulkan pixels prove accepted baseline survives rejected batches,
retry changes declared pixels once, exact clear/draw order and immediate
native/device resources. Use graphical host escalation and validation layers;
reject Validation Error/VUID categories, no emulation substitute. Persist
focused commands before edits. Run both toolchains/focused sanitizers, six
builds/canonical nonretail suites (sanitizer detect_leaks=0), exact serial host
LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), required physical
Vulkan and LAN 4/4 all six, ledger/diff. Commit one coherent slice:
`delivery: M22 08P0C2D0B defer bgfx frame commands atomically`.
