# M22 A0B Buffer ownership evidence

GCC ASan+UBSan originally reported `alloc-dealloc-mismatch (operator new [] vs operator delete)` at `Buffer::Reset` while `WW3D::Init` destroyed an INI `CacheStraw`. `BufferPtr` is `void*`; the source's `delete [] BufferPtr` selected scalar deallocation. Both owned release sites now cast to the allocated `char[]` element type. Borrowed pointers retain `IsAllocated=false` and are not freed.

Validation: GCC and Clang ASan+UBSan `original_wwsupport_runtime` passed, including owned reset, owned reassignment, and borrowed-storage negative controls. GCC `original_w3d_cpu_graph` and `original_w3d_static_sort_failure` passed with `alloc_dealloc_mismatch` detection enabled and only leak detection disabled for the sandbox's ptrace restriction. The source-scene fixture's separate failed-draw ref leak remains a B3A issue and is not claimed by this slice; its renderer task cleanup is unstaged here.

No retail material was accessed or modified. This slice changes no class layout or renderer behavior.
