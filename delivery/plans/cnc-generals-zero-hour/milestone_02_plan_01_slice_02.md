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

## Native graph implementation detail

Resume after core commit e7abf390. Separate FileSystem's filename ordering and
SubsystemInterface's base lifetime from the global gameplay INI/STL include
graph. A native rooted FileSystem retains the original File consumer API,
loose-before-archive resolution and deterministic archive ordering. Native file
handles are immutable shared backing; pooled File views own independent cursor
and declared ranges, so closing/remounting a registry cannot retire a live view.
Original RAMFile snapshot/scanning and INI semantic dispatch remain actual
consumers, not substitute parsers. Compile complete semantic owners as their
dependent original graph is ported; no stubbed block parse functions or claims
that container decoding proves GameLogic admission. Extract CSF decoding from
GameText's native-wide assumption into a fixed-width bounded owner used by the
real manager. Generated fixture acceptance must link these actual paths.

## Coupled ownership review and validation boundary

The same slice includes bounded original `.str`/map text extraction, actual
GameText and LanguageFilter publication, and native-wide token support required
by those consumers. Six manifest-registered allocation sweeps cover complete
mount, RAM, CSF, map/filter callbacks, filter-map and INI candidates; each retains
its failure/residual/retry pair and exact terminal proof, with three same-process
repeats. No framework source is changed to recover library directory-walk faults.
The read-only supplied-data audit proves container admission/basic source-family
presence and actual text initialization; it does not satisfy slice03 simulation.

No conversion/cache/write operation exists in this reader slice. XDG writers,
rooted memory-profile overrides and FileInfo timestamp/map-cache consumers are
implemented with the actual startup owners in slice03, not silently accepted by
read-only reader tests. Complete gameplay INI callbacks also remain slice03.
Evidence: `evidence/qa/N2-original-data.md`.
