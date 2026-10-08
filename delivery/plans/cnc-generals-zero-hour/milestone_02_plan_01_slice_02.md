# N2 slice 02 — rooted original-data access

## Goal and scope

Original data consumers open explicit user-supplied roots independently of CWD,
honor verified loose/archive precedence, and read BIG/INI/CSF without writes to
assets. Depends on core-owner slice 01; supplies actual startup in slice 03.
No renderer, proprietary distribution or fabricated gameplay content.

## Entry points, ownership and surfaces

Port original File/LocalFile/ArchiveFile/FileSystem/INI/GameText consumer paths
using standard/POSIX services. Replace Windows installation discovery with
explicit root configuration and read-only mounts. Inspect full reader/writer
graphs, record byte orders, lengths, encoding and precedence before changes.
Validate archive entry paths/offsets, arithmetic, counts and terminators before
publishing indexes; reject traversal/ambiguous collisions without partial state.
Separate UTF-16 CSF file units from native character storage. Preserve original
INI semantic dispatch, rather than accepting a generic unrelated parser as parity.

Use XDG user/cache locations for writes. If conversion is needed here, key caches
by content plus converter version and support missing/corrupt/unwritable storage
with in-memory fallback. Do not introduce caches where no conversion exists.
Remove the obsolete startup asset-deletion path before any retail execution.
Report actionable redacted errors; preserve all asset paths/bytes. Generated
fixtures are public; private-input inspection emits only bounded public status.

## Validation and acceptance

Generated archives/INI/CSF prove original-consumer reachability, explicit roots,
precedence, non-ASCII text, boundary lengths/offsets, truncated tables/payloads,
duplicate/path errors, candidate rollback and corrected retries. Repeated
mount/read/unmount must settle owners under normal/GCC/Clang sanitizers.
Verify any cache fallback separately and never count it as successful decode
when required content is absent. CMake/Ninja/CTest commands are canonical.

Retail completeness probe at PRE04 is read-only, integrity checked and redacted;
no selector/root/raw input output is retained. Missing required supplied content
is recorded as an external prerequisite, not disguised as parser success.
Commit this complete rooted-access slice with source findings and evidence.
