# M22 slice 06C3C2B3A0B: original Buffer array ownership

## Outcome and boundary

Original WWLib `Buffer` releases only its self-allocated `char[]` through array deletion; caller-owned borrowed memory remains untouched. This is a prerequisite to allocation-policy and physical source-scene sanitizer acceptance, not a renderer or scene acceptance claim. Keep the Windows/source public Buffer interface and data layout unchanged.

## Entry, state, and errors

`Buffer(size)` and null-pointer/positive-size constructors allocate owned arrays; `Reset`, destruction, and reassignment release exactly once. Borrowed-pointer constructors retain no deletion authority. Empty/zero-size cases remain safe. No authorization or persistence applies. Allocation failure retains normal C++ failure behavior.

## Evidence and tests

GCC ASan+UBSan observed `new[]` versus scalar `delete` at `Buffer::Reset` during original `WW3D::Init` (INI `CacheStraw` destruction). Source inspection shows `BufferPtr` is `void*`; `delete [] BufferPtr` therefore resolves the wrong deallocation family on GCC despite its spelling. Cast only the owned pointer to `char*` at both release sites. Positive tests: owned reset/destructor/reassignment under GCC and Clang ASan, plus original WW3D initialization. Negative control: borrowed storage is not released or changed by Buffer reset/destruction. Run focused source tests and related WWLib suite, then record sanitizer results. Do not suppress alloc/dealloc mismatch.

## Acceptance and commit

The mismatch disappears with ASan mismatch detection enabled, owned/borrowed behavior passes, and no unrelated source-scene changes are included. Commit the narrow Buffer fix, its own test, this plan, and evidence as one independently revertible slice before A1.
