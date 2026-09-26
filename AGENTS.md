# Original Linux runtime integration

The original engine replaces ordinary global allocation/deallocation with its
memory-pool boundary. A library's temporary or nothrow allocation path can have
a different owner even when its result later reaches ordinary `delete`.
For new original-runtime algorithms, inspect both ownership paths and validate
with GCC and Clang sanitizers. When a transaction requires allocation-free
ordering, use an explicit bounded in-place algorithm; do not infer that property
from a standard-library algorithm. Keep allocator fixes scoped to their actual
owner, and preserve pre-publication rollback and accepted-state retry contracts.
