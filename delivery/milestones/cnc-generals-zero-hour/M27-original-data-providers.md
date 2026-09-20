# Milestone M27: original data codecs and map metadata providers

## Objective

Exercise original file/configuration, serialization primitives and map metadata through native read/write boundaries before integrated initialization.

## User/System Outcome

Owned fixtures produce source-owned data and deterministic metadata through actual original consumers, with retail-style read roots remaining immutable and generated state isolated under XDG.

## Scope

Adapt original FileSystem/LocalFileSystem/ArchiveFileSystem to existing BIG/VFS/XDG components; original INI/CSF/GlobalData/name-key data access, Xfer fixed-width/UTF-16 primitives, chunk parsers and RandomValue streams. Port independently testable original MapUtil metadata/cache operations: cached streams, reusable WorldInfo/ObjectsList/height parsing, map enumeration, CRC and cache-path/output primitives. Full coupled registry callbacks/post-load relationships are M20, not required to link these small consumer targets.

## Explicit Exclusions

No full match, complete registry/engine startup, Windows import, full snapshots/replay or physical devices/private data prerequisite. M21 owns full scenario behavior; M24 owns full state/persistence acceptance. Metadata must not be deferred to those milestones merely because it reads maps.

## Source Requirements

[Runtime reconciliation](../../../docs/zero-hour-runtime-closure-reconciliation.md) RC-002 data/codec providers, RC-004/006 identity/resources, RC-007/008 source data semantics, RC-009 map metadata and writes; SE-002/003/007/010 and base plan §§5–6/10. Provenance is in status.yaml.

## Preconditions

M26 original process/allocation/ABI and M4 existing VFS/archive/corpus component. Owned fixtures must suffice for independent provider acceptance. Read-only retail initialization remains M20; no later original runtime build or successful scenario is an entry requirement.

- `PRE-035` — accepted M26 original allocation/ABI and shared floating-mode provider; `PRE-009` — accepted M4 native VFS component. This milestone produces `PRE-036`, not PRE-029 full runtime/cache acceptance.

## Readiness checks

- Run `cmake --list-presets`; inspect status.yaml and linked M26/M4 acceptance for PRE-035/009 with their evidence grades. The [prerequisite manifest](../../readiness/cnc-generals-zero-hour/prerequisite-manifest.md) records provider boundaries.
- Inspect the ledger's parser extraction boundary: the full production INI dispatch table, ThingFactory/MapObject and localized map cache remain M20-owned. Require one consumed shared `setFPMode`, not successful later GameLogic initialization. Owned fixtures must not depend on private assets or fake production callbacks.

## Functional Requirements

Prove actual original normalization, case/separator handling, override/enumeration precedence and distinct logical read roots versus XDG writes. Original INI/CSF callbacks produce source-owned values and report malformed/missing inputs. Scope small consumer targets to genuine independent operations; registry/module callbacks requiring runtime are explicitly owned by M20 in the ledger.

INI.cpp's static theTypeTable references callbacks across UI/audio/AI regardless of the fixture fields used. Preserve the complete production table for M20. Use narrow shared-method extraction of the original parser core/typed field routines and real selected callbacks in an owned fixture harness for independent evidence; never fake missing callbacks, trim the production registry or rely on dead stripping to hide required providers. Prove the extracted parser used by production is the same code under test. Preserve INI::loadDirectory's sorted root-files-first then subdirectories two-pass ordering, not a single global lexical ordering. Original INI/map metadata callers consume M26's single shared setFPMode definition without pulling in GameLogic.

Apply source-defined Bool/Int/WideChar encodings in actual Xfer/chunk/INI consumers. Characterize RandomValue's six UnsignedInt CRC values and independent logic/client streams with owned fixtures across presets; additional client/audio randomness must not alter logic checkpoints. Preserve rules and do not infer Windows compatibility.

Handle absent/stale caches, original metadata parsing, standard/user-map precedence and Debug rebuild branches. Route direct fopen cache writes to isolated XDG cache storage; never CWD or read roots. Preserve cold/warm metadata equivalence and bound malformed maps/write failures. Retail-derived metadata/checksums remain private runtime state, never committed evidence.

At this independent boundary, exercise only extracted original stream/chunk/parser/cache-path operations and state the evidence scope precisely. MapUtil::ParseObjectDataChunk uses ThingFactory and creates MapObject; addMap and INIMapCache use GameText/name keys/map.str. Their full registry-dependent object classification/localized cache refresh belongs to M20, including nonempty objects, complete cold/warm cache equivalence and all side-effect tests. Empty maps cannot prove full cache acceptance. Keep production callbacks intact and verify the shared methods used by production are those exercised here; no surrogate registry is allowed.

## Architecture / Security Constraints

Original parsers and state are authoritative; adapters preserve native VFS and source semantics. No toy codec fallback, unconditional success, reduced class or discarded-provider identity proof. All writes are XDG-isolated, reads bounded, retail unmodified. Maintain M26's source/edge/configuration ledger and evidence-grade/freshness gates for every new provider and consumer.

## Interfaces and Compatibility

Preserve original data interfaces, fixed-width UTF-16LE and baseline precedence; never -fshort-wchar. Existing native tests/APIs coexist without parallel authoritative state. Narrow parser portability corrections require characterization. No Windows import or full snapshot compatibility claim.

## Acceptance Criteria

- [ ] Actual original file/INI/CSF/Xfer/chunk/random consumers run on owned fixtures in all four presets without full engine startup.
- [ ] Width/encoding boundary fixtures are cross-preset consistent; client/random activity does not perturb logic stream/CRC.
- [ ] Independent original parser/cache-path consumers pass applicable cold/warm, absent/stale, malformed map, standard/user precedence and write-denial fixtures in Debug and Release; ledger explicitly reserves full registry-dependent cache acceptance to M20.
- [ ] Arbitrary CWD and read-only data roots remain unchanged; every generated cache uses isolated XDG storage.
- [ ] Ledger assigns remaining coupled registry/startup edges to M20; provider removal and source/registry drift fail real validation gates.
- [ ] Active tests, full asset-free regressions and source-consumer sanitizers pass; missing retail never skips fixture failures.

## Required Validation

Owned synthetic BIG/INI/CSF/map/chunk fixtures through original consumers; positive, malformed, truncated, boundary/width, override enumeration, warm/cold cache and permission-denial cases. All four GCC/Clang Debug/Release presets, full asset-free CTest and focused ASan/UBSan with options and allocator/resource ownership counts recorded. Prove target-specific compile/live-symbol identity and runtime outputs separately. Source-inspected ledger edges are not reported as runtime proof.

## Known Risks / Deferred Work

Some INI/map handlers require coupled engine registries. Record their exact M20 ownership instead of introducing fake globals; independently accepted parser/cache provider methods must still run real source. M20 integrates complete initialization; M21/M24/M25 extend scenario/save/network behavior without supplying missing accepted provider code.
