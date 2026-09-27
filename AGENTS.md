# Original Linux runtime integration

The original engine replaces ordinary global allocation/deallocation with its
memory-pool boundary. A library's temporary or nothrow allocation path can have
a different owner even when its result later reaches ordinary `delete`.
For new original-runtime algorithms, inspect both ownership paths and validate
with GCC and Clang sanitizers. When a transaction requires allocation-free
ordering, use an explicit bounded in-place algorithm; do not infer that property
from a standard-library algorithm. Keep allocator fixes scoped to their actual
owner, and preserve pre-publication rollback and accepted-state retry contracts.

Shader reflection may omit dead fields without changing their source block's
origin or padding. Bind unchanged source payloads using compiler-authoritative
block offsets, never the first live field; cross-check generated metadata and
cover dead prefixes, multiblock layouts and genuinely eliminated providers.

Treat packed vertex attributes as a source-shader protocol, not their transport
names. Preserve encoded slots through transitions when native load/update does;
prove exact source selection and physical payloads instead of inventing resets.

Native create APIs may deduplicate handles while incrementing reference counts.
Transaction journals must track each acquired ownership unit, not unique handle
values; cover canceled aliases, replacement, drain and teardown independently.

Known-bit masks do not bound encoded native table indices. Validate every packed
subfield (including ignored/default values) before reservation or replay; cover
exact maxima, representable bound+1 and mixed late-invalid inputs without effects.

An unfixed enum may make a synthetic rejection value undefined before validation
reads it. Use a defined raw representation at admission, keep platform class
definitions consistent, and verify enum/class size, alignment and member offsets
against the prior ABI before accepting range checks under both sanitizers.

Test transaction poisoning from an otherwise commit-ready candidate. A rejected
commit on an unapplied owner does not prove poisoning. Put scope guards before
provider dereference, phase diagnostics and diagnostic allocation, including
no-throw entry points that must poison and return without mutation.

Bidirectional publication stays fallible until notification callbacks return.
Exercise faults inside every callback family, not only after binding completes.
Restore both ownership links without cleanup notifications before construction
owners withdraw registries/modules and release backing storage.
