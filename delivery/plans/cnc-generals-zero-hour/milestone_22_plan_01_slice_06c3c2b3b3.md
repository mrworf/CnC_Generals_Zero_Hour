# M22 plan 01 slice 06C3C2B3B3: complete public bgfx readback before releasing caller storage

## Outcome and boundary

Correct the public bgfx diagnostic readback buffer lifetime independently of B3B2 fixture-lock bootstrap and pending B3B source categories. The pinned bgfx `Context::readTexture` queues the caller `_data` pointer and returns submit frame `m_submit->m_frameNum + 2`. `Context::frame` waits for the prior render frame, then submits the current frame and returns its number. Thus reaching the returned frame number submits the read command but does not prove its render worker has finished writing caller storage. Call one additional public `bgfx::frame()` before inspecting, transforming, destroying or returning the pixel vector. No backend, source-scene geometry, or original allocator behavior changes.

## Positive, negative and lifetime controls

Direct host Vulkan test: repeated immediate BGRA8 and RGBA8 clear/readback/target retirement across two device generations and extents, with exact per-pixel color/alpha checks and immediate caller-heap reuse. Negative controls reject uninitialized and retired targets, and every generation reaches zero public resources. Source static and pending mixed four-generation scenes retain their physical pixel oracles; this fix must not be credited for the separately proven B3B2 allocator-lock race.

## Validation and commit

Use pinned bgfx source to justify the completion boundary. Run GCC/Clang Debug/Release builds, direct repeated Vulkan readback tests with validation enabled, GCC/Clang ASan+UBSan, source-only leak-capable controls, full canonical suites, provider/ABI and dependency ledger. Physical host libdbus leak scanning can be separated from source-only LSan, with exact evidence. One independent commit; keep B3B mixed source/test edits unstaged and B3B acceptance pending.
