# N2 slice 03 — actual original headless runtime

## Goal and dependencies

Original GameEngine startup reaches original GameLogic and executes deterministic
simulation on Linux, then shuts down and retries in the same process. Depends on
core owners and rooted original consumers. This is the N2 acceptance boundary;
core-only/RNG-only tests are not enough. Full rendering/media/match completion
are later milestones and must not be claimed here.

## Runtime and planned surfaces

Derive explicit CMake source graph from original GameEngine/GameEngineDevice
projects and reachable startup/object/module dependencies. Port shared native
services and extract settled presentation/platform seams without changing
simulation: headless output services may omit output, never replace logic.
Retain original GameEngine ordering, original object/module updates, original
map/data interpretation, fixed simulation time and RNG streams. Audit startup
failure/teardown across every constructed owner before retail execution.

Generate a complete legal minimal simulation fixture whose source setup reaches
original startup, object creation/update, and reproducible state checkpoints.
Use source-defined actor behavior, not an independently written toy. A native
entry point accepts explicit roots, headless mode and bounded test ticks.
Checkpoint outputs contain no private input-derived identifiers. Read-only retail
startup validates supplied content and integrity, with redacted error/status only.

## Tests and completion gate

Carry forward the reader slice's explicit remaining startup services: XDG
user/cache writers separated from immutable asset mounts; rooted MemoryPools.ini
profile loading; native FileInfo timestamp semantics used by map-cache owners;
complete original INI block table; non-BMP filter replacement-width compatibility.
The slice02 stage4/mask63 retail probe is only container/basic-family/text evidence,
not acceptance of these services or deterministic GameLogic execution.

Implementation source map: `docs/original-runtime-source-graph.md`. Start with
the original startup namespace/container/ID seams as one coupled owner batch,
then complete the original definition/object/script graph and headless provider
seams. NameKey allocation failures must preserve both buckets and next ordinal;
static name caches must distinguish namespace resets and replacement at reused
addresses. These are supporting work within this slice, not acceptance of
startup or an added milestone/slice. Full source-engine ordering remains required.

Map/setup dictionaries are part of this same owner graph. Replace the original
unconstructed pointer-punned values and 16-bit shared refcount with typed,
standard-owned COW backing, retaining sorted keys, public getters/setters,
type replacement, missing-value conventions and MAX_LEN. Prepare mutations
offside so clone/growth/type replacement failures preserve every accepted alias;
cover self-copy, same-backing copy, all value types and repeated failure/retry.
This is internal representation work, not a change to map wire formats. Keep
chunk table IDs separate from runtime NameKey IDs. Container iteration review
must also cover particle-parent selection and audio last-match selection before
replacing legacy hash maps; those are not lookup-only consumers.

Port CachedFileInputStream as a real rooted consumer with complete offside
read/decode backing, defined cursor admission, failure preservation and retry.
Derive RefPack token lengths/distances from the original REF_decode source;
validate the entire compressed stream before allocating its declared output.
Use system zlib through its public API, not the bundled modified zlib. Keep
unported NOX/EAB/EAH tags explicitly rejected and recorded as pending; successful
RefPack/zlib fixtures alone do not satisfy full map/startup acceptance. The chunk
reader/writer and remaining codecs stay in this same slice, not new tiny plans.

Extract the actual chunk TOC/input owner separately from the still-unported
writer. Preserve registered-parser newest-match selection, nested parent byte
accounting and unknown-chunk skipping. Guard TOC append and borrowed stream
cursor on rejection, initialize every pooled record, validate packed dictionary
types/table IDs before runtime-name admission, decode UTF-16 units explicitly,
and cover direct-read failure/rollback/retry and recursive parser reachability.
Output publication/destructor cleanup remains pending until the writer is ported
with explicit fallible finalization; no reader tests imply writer acceptance.

Native output will buffer a candidate using game-owned standard storage, encode
all widths explicitly, and expose an explicit fallible finish operation. A
destructor only releases owners and never writes/calls the sink. Finish requires
balanced chunks and publishes one fully prepared blob; a failed write/validation
poisons that candidate, including an otherwise commit-ready writer. Port every
source construction site to explicit finish (the excluded editor remains
unaccepted). The sink is a borrowed candidate-output service; eventual XDG file
publication must supply offside/atomic file ownership, not infer atomicity from
the old generic write API. Test complete roundtrips and fixed wire bytes, every
type/length/packed-ID boundary, nested size patching, abandoned/poisoned writers,
sink throws/short writes and coupled whole-operation allocation sweeps. This
remains the same slice03 transaction and is not whole-runtime acceptance.

Continue remaining map codecs as a coupled decoder-admission batch. EAB uses
the original byte-pair graph; reject duplicate/cyclic/special-clue branches and
verify complete bounded expansion before declared-size allocation. EAH uses
canonical code counts and leapfrog symbol ordering, MSB-first encoded integers,
clue literal/run/end escapes and optional delta/acceleration; inspect both
original encoder and decoder before implementation. No original codec/library
is patched. NOX's wrapper references an absent LZHL provider: discover a legal
upstream source or exact prerequisite rather than inventing its format. Keep
fixtures generated and prove truncated/late-invalid admission, rooted owner
preservation, exact allocation terminals and same-owner retry together.

Next startup service batch retains compiled memory defaults and applies rooted
MemoryPools.ini to a complete offside profile after mounts exist, before
definition pools are created. Explicit positive class counts and already-live
pools retain their original ownership; profile publication never resizes them.
Reset profile overrides at process teardown/startup. Preserve source row order,
case-insensitive known-name selection and four-count rounding without signed
overflow. Include missing/unknown profiles, malformed/late-overflow preservation,
allocation rollback and repeat startup. FileInfo must derive a real Windows
100ns-since1601 timestamp from POSIX metadata; archived entries use the archive
timestamp and member size, like the original provider. This metadata alone does
not fix MapCache's source size-only reuse test or meet content-keyed cache
acceptance. Preserve output values when metadata acquisition fails.

LanguageFilter stores native scalars but source masks counted UTF-16 units:
non-BMP authored words must produce two stars per original surrogate pair.
Prepare replacement text offside, preserve unchanged tokens/delimiters and
original normalization/search semantics, and publish only on success. Cover
multiple mixed-width tokens, configured astral words and allocation rollback;
do not change external encoded word lists or assets.

XDG storage is a separate game-owned write authority, not write access added to
asset mounts. Resolve absolute data/cache bases (ignore relative XDG overrides)
with the standard home fallbacks, append the stable product identifier, and
reject every candidate inside a mounted asset root before any mkdir/write.
Resolve existing base aliases before admission and walk prepared components
using bounded no-follow directory descriptors. Guard created temporary files
from acquisition through explicit finish/atomic rename; destructors only clean
up unpublished candidates. Validation/I/O exceptions poison a commit-ready
candidate. Keep no fallible allocation after rename. A post-rename directory
fsync failure must report published-but-durability-unknown, never pretend the
old file remains. A game-owned POSIX I/O seam permits complete callback faults
without library changes. Persisted output candidates implement the actual
OutputStream boundary so DataChunk.finish and file commit remain explicit steps.

Disposable cache envelopes carry source digest, converter version, bounded
payload count and payload digest. Use stock system OpenSSL3 public EVP APIs for
SHA256 rather than writing or copying a crypto implementation; installed3.6.4
is available and remains unchanged. Record the new public build prerequisite.
Missing, malformed, mismatched or inaccessible cache data triggers the actual
converter; unwritable cache publication preserves the converted memory result.
Converter failure preserves accepted source/candidate state and never becomes
a cache hit. Independently test absent/corrupt/same-length changed source/version,
storage inside asset roots (including base aliases), candidate abandonment,
commit/write/sync/rename failures, allocation rollback/retry and repeated owners.
Current support matrix50/50 is the prior frozen checkpoint, not acceptance of
this new storage code or actual GameEngine/GameLogic.

Bind the storage service to the actual CachedFileInputStream through an optional
borrowed startup-owned provider. The no-provider path remains memory-only;
with a provider, cache misses invoke the actual bounded native map decoder, not
a toy converter. Read the source from immutable mounted assets, retain accepted
stream backing/cursor until complete cache/decode publication, and use an
explicit map-converter kind/version identity. Prove first miss, subsequent hit,
corruption rebuild, unwritable memory fallback, malformed reopen preservation
and corrected retry through this actual rooted owner. Original GameEngine must
still own/pass this service at its later startup integration gate.

Before freezing the combined cache owner, apply the decoder's admission rule to
cache reads as well: validate fixed header identity/count and stream the payload
digest into bounded scratch before allocating declared backing. Then rewind,
read the accepted candidate and recheck its digest before publication. This
avoids treating a bounded stat size as proof of a valid initialized payload.
Keep callback state scoped through rejection and both compiler sanitizers;
cache header/body rejection remains a miss, not successful original startup.

Qualify the discovered LZH-Light candidate unchanged at its exact upstream
revision, using only its published C API. Check valid generated roundtrips and
truncated-input status, including whether the API exposes consumed counts or
decoder rejection at all. Instrument the stock provider for diagnostics without
patches or sanitizer exceptions; distinguish an upstream/API limitation from an
adapter bug. Record findings and revision before adopting any provider. A probe
failure is not permission to fork a framework, waive malformed admission, or
claim NOX acceptance. This remains the existing slice03 prerequisite work.

Before compiling the complete runtime graph, census repository-header includes
across the game-owned source/header tree and correct all unambiguous casing
mismatches together, including retained legacy provider callers. Resolve only
unique canonical repository headers and preserve local-header resolution; do
not rename or patch dependency headers. Add a regression census so future
case-insensitive assumptions fail as one graph contract, not successive missing
header fixes. Compilation of excluded providers is not implied by this census.

Treat dynamic engine IDs as one representation boundary: Object, Drawable,
Formation, ParticleSystem, Production and Waypoint IDs all have source-defined
32-bit nonnegative enumerators but carry runtime-assigned values. Extract their
unchanged named values into a lightweight game-owned header with explicit
UnsignedInt representation, and correct every retained opaque declaration.
Compare widths/alignment/member offsets with the preceding unfixed declarations
under both compiler sanitizers; cover full raw-width values without constructing
undefined unfixed-enum rejection inputs. Defined representation is not proof of
owner-level ID admission or startup acceptance; those remain runtime gates.

Migrate all13 active STLport hash-map typedefs as one container boundary to
standard unordered_map with source content equality/hash consistency. Fix the
speech borrowed-C-string hash and obsolete null constructor, the native pointer
hash width, and ThingFactory's bucket resize operation together. Actual storage
types retain node/reference lifetime semantics; source linked template/speech
name lists retain creation/enumeration order. Explicitly sort named/numeric keys
when saving GameLogic overrides and Player/Team relations, since their readers
consume keyed records, not buckets. Container order is not simulation order.
Particle parent nth-selection is only reached through the excluded editor DLL.
Audio last-music selection is a retained GameEngine startup asset-readiness gate,
not excluded merely because it is adjacent to CD initialization. Preserve valid
media readiness and separately qualify missing/corrupt media diagnostics at the
actual audio/startup owner boundary. Other preload,
cleanup and video traversals still require their owner acceptance later.
Cover independently backed equal strings, case distinction, C-string ownership,
rehash reference stability, insert/erase/retry, deterministic key collection and
allocation faults under both sanitizers. Test the actual shared header contracts,
but do not confuse these with compiling/accepting every migrated owner class.

The complete enum census now finds47 defined opaque domains plus two unresolved
legacy names. Source bodies have been inspected: nine domains contain negative
sentinels (Attitude, EvaMessage, KindOf, LocomotorSet, ModelCondition, PhysicsTurning,
StaticGameLOD, WeaponBonusCondition and WeaponSetCondition); the remaining38
are nonnegative indices/masks. Resolve all definitions and forwards together
using their preceding compiler signedness/32-bit width, not a blanket signed-int
conversion. Freeze each preceding enumerator body hash, generate isolated proofs
from those source-verified bodies for both conditional enum configurations, and
compare widths/alignment/representative member offsets plus full defined raw
representations under both sanitizers. Whole owner classes remain pending.
HackerAttackMode has no definition or retained use beyond two opaque declarations.
ObjectStatusType is different: RiderInfo really stores it and the Rider parser
uses ObjectStatusMaskType's index list. Resolve that member to the actual plural
ObjectStatusTypes domain; never discard it as unused. Inspect all RiderInfo
initialization/parse/copy/status consumers before changing its defaults. Enum
representation tests do not accept original INI field-store aliasing or module
owner transaction behavior; qualify those at the actual definition graph.

For the next coupled header batch, preserve existing behavior while correcting
dependent template names, in-class extra qualifications, missing forward/header
dependencies, the partition loop's post-loop sentinel and native-width intrusive
list diagnostics/member-function pointers. Exercise actual SparseMatchFinder and
GameCommon list macros with generated owners: match/tie/extraneous-bit selection,
cache reuse and explicit invalidation, empty/reverse/remove/reinsert/drain order,
allocation-failure rollback and retry. Borrowed cache values do not acquire vector
ownership; callers must clear before backing mutation. This supporting batch does
not accept the actual object/module graph or repair its untested INI publication.
Extract the unchanged viewport FilterTypes/FilterModes declarations into their own
shared header used by View and CommandXlat, avoiding a header cycle. Lock source
bodies and numeric endpoints in tests. Restore explicit owner headers previously
provided by the Windows PCH and qualify ambiguous std::min/std::isnan calls without
changing their operand order or NaN-only policy. Batch these with the remaining
calendar and embedded-value ownership work before another whole matrix freeze.
Use a game-owned NativeCalendarTime of eight unsigned16 fields in original
SYSTEMTIME order (year,month,weekday,day,hour,minute,second,milliseconds). Native
localtime conversion publishes a complete validated value; date uses the host
time locale and time remains minute-resolution. No Windows SDK emulation. Replace
calendar consumers coherently, but keep broader replay time_t/wchar encodings and
unchecked reads as pending N5 owner work, not proven by this calendar fixture.
AttackPriorityInfo is an embedded256-element ScriptEngine value array; the
complete retained source census finds no pooled creation. Remove only this value
type's inappropriate MemoryPoolObject/glue boundary, declare its existing public
destructor and prohibit accidental owning-map copies. On first priority insertion,
guard the map offside through final-key selection and node insertion before
publication. Preserve existing-map node/value behavior. Actual ScriptEngine/xfer
and first/late-failure lifecycle tests remain required before slice acceptance;
header/syntax probes alone do not establish that ownership acceptance.
Remove KeyDefs' SDK include by resolving each original KEY_* to its verified
numeric protocol value; retain all names, modifier bits, KEY_NONE/KEY_LOST and
MetaEvent subset identities. Lock the complete key table to source and the primary
public Microsoft numeric definitions, without adopting the SDK or changing SDL.
Native physical event translation remains N4; the original updateKeys256-event
loop lacks a bound and remains a pending provider/stream-owner admission contract.
Detach excluded Internet SDK code from the retained GameLogic path, rejecting
unsupported mode admission before loading/publication while preserving LAN and
replay/skirmish/campaign paths. Cover mode rejection and prior-state preservation
at actual owners before final N2 acceptance; do not fabricate GameSpy service stubs.
Replace active Windows clock/yield services with a game-owned steady-clock boundary.
Retain unsigned32 millisecond wrapping and simulation update order; use elapsed
subtraction for load-progress/timeout boundaries instead of signed deadline overflow.
Define disabled/nonpositive and sub-millisecond frame caps without undefined float
conversion, preserving ordinary positive-cap behavior. Prove wrap/threshold/cap
boundaries under both sanitizers. Remove unused ATL application/module setup and
classify _PROFILE/DRM-only imports rather than modifying their library headers.
WindowMsgData is an in-process callback transport, not a file/network field.
Retain its name across all callbacks and use uintptr_t for borrowed pointer payloads;
do not truncate before admission or invent a32-bit handle registry. Source census
includes the legacy physical W3D callback. Cover full pointer and packed scalar
round trips at the shared transport; actual synchronous borrowed-string and callback
publication/rollback remain native GUI owner acceptance, not transport proof.
Use the complete260-TU census artifact to batch shared declaration dependencies.
Detach the excluded ScriptEngine DLL debugger/particle editor/VTune graph as one
cohort, not Win32 loader stubs. Retain actual script freeze, counters, fades and
sequential update order; public tool hooks preserve the original absent-DLL
behavior. Reject explicit excluded-tool startup options before initialization.
Keep particle name-table ownership and real particle simulation includes. Remove
only private editor helpers/device imports. Actual ScriptEngine lifecycle and
option-admission evidence remains required; syntax success is not acceptance.
Xfer.h does not use ModelState declarations; removing that heavy include breaks
the Xfer→ModelState→BitFlagsIO→incomplete-Xfer cycle. Upgrade owns an explicit
BitFlags include; ObjectTypes owns its ThingTemplate forward; Display/Radar own
CellShroudStatus through GameCommon. Verify affected original TUs as a cohort and
keep the prior failed census as historical evidence. Mask parse/load/CRC hazards
remain unaccepted until the actual Xfer/INI graph is executable and tested.
Normalize original source-local backslash include separators against real files,
not symlink aliases, across the complete retained source scan. Give actual logic
callers explicit GlobalData/climits/string/algorithm declarations instead of
restoring a Windows PCH. Qualify diagnosed min/max/isnan calls preserving operands
and NaN-only policy. Repair dependent-template syntax and explicit BitFlags name
specializations without changing protocol/name bodies. Keep VC6 iterator/scope
and pointer-cookie ownership fixes separate for source inspection and tests;
do not mechanically turn all old pointer uses into integer casts.
Finish the diagnosed scope/iterator cohort together, preserving source traversal
and last-loop values. Borrow matched vector elements only after iterator/end
checks, reject empty authored creation paths and stop missing-flare traversal
before decrementing begin. Actual module lifetime/selection acceptance remains
required. Replace fixed path/decimal/font buffers coherently: capture every map
companion path before first loader mutation, retaining saved pristine-map ownership
and map/solo/text/asset-preload order; parse cinematic font labels from the actual
editor's name + " - Size:" + decimal + optional " [Bold]" encoding. Validate
all fields and frame multiplication before display mutation. Test long names,
spaces/hyphens, all companion kinds, malformed/overflow values and fault retry.
Audit all eight PartitionManager scanline callbacks together: declare the original
friend functions consistently, pass source player indices by address through the
synchronous DiscreteCircle dispatch, and clip a complete span before forming
backing pointers. Test exact clipping/range boundaries separately from physical
backing ownership. Port the actual original DiscreteCircle's signed-shift/mirror
arithmetic to checked wide intermediates while retaining scanlines, duplicate
removal and callback order; test negative centers, zero radii, extent rejection,
constructor failure and retry. Cell mutation rollback remains actual graph work.
After the complete logic syntax cohort is portable, compile its real259 retained
TUs as an original_logic archive under normal GCC and both sanitizer compilers.
Share game-owned core/runtime boundaries and the stock legacy header roots;
exclude only FPUControl already owned by runtime_common. Disable the unrelated
legacy library global-new hook with its existing public configuration macro,
preserving N1 ordinary allocation. Archive success is object-build evidence,
not executable reachability or owner acceptance. Inventory remaining Common
startup/definition/transfer dependencies before choosing the next coupled repair.
Port the coupled original BezierSegment/BezFwdIterator owner without a D3DX
shim or library patch: retain the exact source cubic basis, scalar operation
ordering, forward differences, subdivision and extrapolation. Test independent
Bernstein/de Casteljau oracles, endpoints, constructors, sampling, length and
same-process allocation rollback. Reproduce the one-sample zero-point bug before
repairing it; publish sampled vectors offside and reject negative counts before
allocation/mutation. Invalid length-tolerance/alias contracts require explicit
investigation rather than silently altering otherwise accepted behavior.
For synchronous callback data, pass actual bounded source values by address
instead of encoding them as pointers. Inspect Player→Team and contained-list
dispatch for non-retention before borrowing a stack frame. AI condition tables
retain stable source-owned attack-type values. Preserve exact unsigned frame
wrapping and condition identities. Actual owner dispatch/lifecycle tests remain
required alongside the full native graph; representation support is insufficient.

Normal, GCC and Clang original executables run the same fixture with identical
checkpoints. Test independent presentation omission, multiple startup/run/shutdown
cycles, interrupted startup/late decode failure, missing roots/malformed input,
and corrected retry. All live owned resources settle; process-global initialized
services are measured separately. ASan/UBSan/leak detection remains enabled.
Record source→entry→GameLogic/object reachability and compiler/build identity.

Continue the retained Common census as a coupled definition/transfer batch:
make source header dependencies explicit, preserve bit-name specialization
ownership and source ordering, and replace encoded INI module/count userData
with stable typed source values. Compile the complete physical census after the
batch, distinguishing obsolete providers from retained runtime dependencies.
Do not infer startup acceptance from syntax or static archives. Keep complete
GlobalData clone/publication, preferences persistence, typed FunctionLexicon
callbacks and INI block-owner rollback together in this existing transaction.
For callback registration, retain actual prototype families rather than function
pointer/void* erasure. Carry bounded tables through lookup and prepare complete
key sets before table publication, including original W3D device table producers.
Cover wrong-family lookup, malformed indices/sentinels, late registration faults,
prior-table preservation and corrected retry; no GUI replacement callback stubs.
Keep the next configuration/bootstrap ownership correction coupled: separate
complete configuration values from SubsystemInterface identity, use compiler-
generated complete copy/move (with a proven no-throw adoption boundary), clone
WeaponBonusSet backing offside, and borrow explicit candidate defaults in the
actual options owner. A registered original GlobalData keeps its address/name;
override nodes publish only after parsing/preferences succeed. The root must own
override teardown independently of the transient singleton link, so constructor
and subsystem-init failure can withdraw links before retiring candidate backing.
Audit initSubsystem argument evaluation, early singleton publication, final
registry growth, shutdown withdrawal and parallel subsystem constructors before
freezing bootstrap faults. Native executable identity/XDG paths/input settings
must replace host Windows services without changing data encodings or libraries.
The source-only150-TU client inventory is supporting this same batch, not an
authorization to adopt excluded Internet/tool providers or accept the physical UI.
Source integration now separates actual options core from its presentation/
campaign service defaults, and bonus configuration/Money serialization from
weapon execution/player/audio services. The complete original INI registry uses
the same bounded loadBlocks dispatcher as generated real-owner fixtures; a
selected owner table is support evidence, never a substitute for that complete
registry. Move definition-only BodyDamageType/AIDebugOptions/TerrainLOD/bonus
headers without changing source-locked enum bodies or external values. Check
every field-table offset spelling against the configuration base (including
compact offsetof spellings), not the polymorphic subsystem object. Preserve
dynamic singleton lookup for long-lived default GUI preferences; only explicit
offside preferences borrow a candidate. Generated configuration families cover
late semantic rejection, deep bonus independence, exact fault/rollback/retry
resources, root teardown after singleton withdrawal and native identity/settings.
Complete subsystem startup/registry fault integration remains in this transaction.

Separate Xfer's typed abstract contract from its existing default adapter owner
when configuration snapshot virtual calls require RTTI under GCC UBSan. Xfer
retains its public method signatures and state initialization, but default
implementations live in XferBase; all three retained load/save/CRC providers
inherit those unchanged defaults. Configuration-only owners can consume the
actual abstract interface without linking unrelated GameState/science/upgrade
adapters. No RTTI suppression, linker workaround, replacement implementation or
dummy owner is allowed. Compile the real default adapter archive; complete runtime
reachability must still link and exercise the providers and their actual owners.

Calibrate configuration allocation manifests against the real native address
cardinality before arming faults (57 base operations + one per discovered IPv4
label). Retire the census before baselines; prove each fault and corrected retry,
the exact terminal, interface-pool retirement and three same-process lifetimes.
Do not substitute a mocked network provider or silently truncate host coverage.
Source teardown review must preserve GameLogic/Object and GameClient/Drawable
self-callback links through legitimate cleanup while ensuring no stale singleton
survives backing retirement. Registry entry removal alone does not withdraw all
borrowed globals. Complete engine/child-owner rollback remains required before
acceptance; the current helper's early singleton restoration is unaccepted for
those self-callback owners and must be corrected as part of the coupled batch.

Pair each accepted subsystem registry unit with a non-allocating, noexcept typed
borrowed-slot restoration record. Keep that slot live through source destructor
cleanup (Objects/Drawables call their parent), pop the registry entry first, and
restore the prior slot immediately after cleanup without callbacks or allocation.
On failed preparation, retire the guarded candidate while its temporary slot is
still valid, then restore the prior slot before rethrowing. Simulation singleton
publication/retirement is synchronous on its owning thread; it is not permission
for render/audio threads to dereference these slots. Separate actual list lifecycle
from complete INI initialization so generated bounded-owner fixtures can test the
same registration transaction without fake full-registry providers. Cover name,
init, decode and final registry growth faults, duplicate admission, reverse teardown,
self-callback validity, accepted prior ownership and same-owner retry. This is
support for the complete original bootstrap, not acceptance of that executable.

Finish the retained quoted-string representation as one codec owner batch, not
four unrelated guards. The actual INIMapCache/MapUtil and LAN/skirmish preferences
consume it, independently of excluded Internet chat. Preserve ASCII alphanumeric
literal bytes and uppercase underscore-hex escapes; encode Unicode as original
UTF16LE units rather than native wchar_t bytes. Prevalidate escape syntax, scalar/
surrogate boundaries and complete output capacity before output ownership; build
offside using standard storage with no static shared buffers. Decode an odd final
UTF16 low byte with a zero high byte, preserving the source's intended padding,
but explicitly terminate the complete result rather than exposing a stale prior
static-buffer tail. Reproduce that stale-tail error in a bounded source-algorithm
witness; cover repeated long/short conversions and the unincremented decode-bound
hazard without executing undefined writes. Cover fixed known encodings, signed
high bytes, surrogate pairs, escaped underscores, malformed/truncated escapes,
invalid UTF16/scalars, exact capacities, allocation residuals and corrected retry
for all four actual conversion entry points under both sanitizers. Complete save/
replay timestamp, XDG writer/read authority and parent bootstrap remain separate
retained owner work in this same slice; codec tests do not accept persistence.

Complete parent bootstrap ownership as one coupled batch. Add a bounded,
allocation-free typed service ownership journal as a member of the actual engine,
client and logic parents. Admit empty borrowed slots and journal capacity before
calling factories, publish only completely constructed children, and retain each
acquired child independently of mutable singleton values. Constructor failure
must retire member acquisitions; explicit source cleanup order must preserve
parent/peer callbacks and retire only this parent's acquired children. Optional
null factories acquire no unit; required null factories reject. Keep fallback
reverse member cleanup, reentrant retirement, capacity reuse and no-allocation
publication/withdrawal under generated failure/retry coverage. Parent-specific
FontLibrary reset runs only for an acquired child. Couple translator acquisition
guarding and client cookie publication, engine init error propagation and guarded
GameMain ownership; do not execute after partially failed startup. Compile actual
parent translation units in both compilers, retaining pending complete executable
acceptance rather than replacing their gameplay/presentation providers with mocks.
Make the actual GameClient parent executable-source ready as part of this batch:
use the shared wrap-safe native millisecond clock for its four-second legal-page
interval, preserve the 100ms sleep and exact preload order, and replace Windows
debug-only memory samples with accurately labeled Linux process peak-resident
diagnostics (never pretend these are available physical/virtual memory). Repair
the final preload loop's escaped VC iterator and immutable texture-name pointers.

Expand the actual definition graph as a cohort: compile every original Common INI
provider, excluding already-owned translation units from duplicate archives.
Recognize the authored WebpageURL block using its source URL field and offside
parse admission, but do not instantiate the excluded ATL browser, open URLs,
resolve CWD file links or fabricate a browser singleton. The native language
consumer uses the same explicitly configured catalog language rather than a
Windows registry/PCH shim. Validate positive/unknown-field/late malformed blocks
and allocation rollback using the actual selected provider; complete full-table
link/execution remains required and is not inferred from selected-owner tests.

Integrate the connected native user-file cohort before any full retail startup.
Validate configured XDG data/cache roots against supplied mounts before parent
publication, not only on protected writes. Supply bounded native user-file read,
directory enumeration, cache/save/replay output and exact scratch retirement under
the XDG owner; no raw fopen/delete/CWD fallback may bypass it. Preserve real source
File/INI/map/transfer consumers and immutable asset backing, source official-over-
user map identity, selected mod precedence, external metadata and source state.
Retire verified excluded CD/DRM/GameSpy bootstrap acquisitions and updates together
with retained Local/Archive consumer migration. Do not drop user mods/LAN file
transfer or fabricate legacy services. Preserve typed descriptor ownership across
constructor failure and backing lifetime after parent drain. Generated fixtures
must cover traversal/alias/symlink/special-file/oversize/ambiguous-case admission,
complete ordinal rollback/retry, old-file preservation during failed replacement,
missing optional user data, same-owner repeats and exact std/pool/fd residuals.

Capture scratch retirement as a typed descriptor/inode owner before publication.
A committed scratch candidate must retain a no-throw, allocation-free cleanup
record (parent directory descriptor, target spelling, device/inode), not reconstruct
paths or query GameState during teardown. Inactive failed candidates cannot remove
an accepted prior scratch file; replacement identities retire independently and
cleanup must not delete an unrelated file that subsequently occupies the name.
Prepare the actual GameStateMap journal entry before publishing extracted bytes,
guard actual source File and transfer buffers across both embed paths, and publish
only after the complete bounded transfer payload is admitted. Retain the captured
XDG storage owner through subsystem teardown. Exercise abandonment, successful
publication, replacement, external replacement, explicit/repeated drain and every
allocation/I/O failure/retry before broad validation on a new frozen batch.

Treat folded map keys as a source identity protocol, not arbitrary physical paths.
Only a captured configured data owner may resolve absolute map identities, and
only its Maps/Save subtree participates. Preserve the configured native root,
resolve case-insensitive components against complete bounded no-follow discovery,
reject ambiguous case, traversal and unrelated prefixes, and preserve optional
missing companion reads. Ordinary physical user-path admission stays exact-root.
Integrate that lowering with actual CachedFileInputStream and GameLogic companion
preparation, and inspect MapCache's leaf/suffix/metadata paths as one cohort so
native '/' discovery does not lose source '\\' leaf rules or lowercase the actual
physical metadata query. Cover both source separator spellings, uppercase native
roots/components, saved-map companions, missing paths, collisions and every
allocation rollback/retry; retain original serialized portable map names.

The complete map metadata query/cache owner must accompany final integration:
replace static partially published query state with an offside context, guard
pooled MapObject before collection growth, initialize every metadata field, admit
dimensions/border counts before resize, treat failed open/negative read as failure
rather than stale metadata/prefix CRC, and retire all query/text backing on every
failure with same-owner retry. ParseSizeOnly's post-return height-data tail is
unreachable; do not invent a resampling requirement from it. Preserve compressed-
file CRC, selected waypoints, metadata grammar and official-map precedence. Prepare
complete cache state and optional atomic metadata output before accepted publication;
remove raw writes and excluded official-content tool branches without losing user
maps. Actual INI callbacks must target that same offside owner, not fabricate a
store. A provider/UI source split is an implementation detail when required for
the actual headless source graph, never permission to replace query behavior.

Metadata dependency tracing permits eliminating its unnecessary physical
MapObject construction: preserve the unchanged parsed dictionary, source z-version
rule, waypoint-name key, template lookup for every record, final-override selection
and tech-before-supply classification directly in the offside query context.
Constructor-only angle/render/shadow/bridge state is not observed by metadata.
Verify this against WorldHeightMap's actual MapObject methods and cover generated
metadata before acceptance. Do not assume template lookup is globally pure:
LOAD_TEST_ASSETS can create a template for TEST_STRING names; classify/preserve
that path and its owner rollback before claiming complete cache atomicity.

Keep shared metadata storage/serialization and protected optional persistence in
the actual runtime provider so generated tests execute the same INI callbacks and
cache-file publication used by MapUtil. Move the unchanged shared well-known key
table out of the W3D terrain TU; instantiate it exactly once, without fake fixture
globals. Cover initialized eight-slot metadata from nonzero backing, late malformed
and range rejection, insertion/replacement rollback, source-format roundtrip,
temporary filtered map-label lookup without gameplay catalog mutation, and every
cache writer allocation/I/O rejection with accepted-file preservation and retry.
Optional user MapCache.ini loading must select the real MapCache block provider
into a complete offside cache, never the unrestricted full definition registry.
Missing/malformed derived cache returns a rescan fallback with the accepted graph
unchanged; allocation/internal-owner failures propagate. Required official data
continues to use the required original registry/load path. Prove late-block semantic
rejection, unknown blocks, malformed quoted identities, exact all-ordinal rollback,
corrected retry and three same-process lifetimes with actual provider callbacks.
Include cold-name registry acquisition in the same query/admission graph. Add a
typed bounded in-place NameKey owner transaction with no allocation in rollback,
precaptured unique rollback generation, exact acquired Bucket retirement and
ordinal restoration. Inner success remains owned by an outer transaction; inner
rejection preserves outer-acquired keys. Reset/init cannot retire an active owner.
Invalidate StaticNameKey lookup caches when rejected keys retire. Couple this to
actual readDict, optional whole-file cache loading and whole MapCache update;
ensure dependent offside graphs retire before namespace rollback. Cover exact/
folded name paths, nested commit/rollback, allocator faults, cold malformed dicts,
first-use static keys, same-owner retries, capacity reuse and three lifetimes.
The whole MapUtil query/ThingFactory/global owner remains a separate execution
obligation within this same slice; selected provider evidence cannot accept it.
Extend the actual Common/Thing provider graph as one template/module cohort.
Inspect default/override copying, ID/list/hash publication and factory-owned
ModuleData backing together, including failed constructors and late INI fields.
Guard ordinary ModuleData within the actual creator macro and after factory return;
hold a factory transaction until unpublished template graphs retire. ModuleInfo
borrows those data pointers, not ownership. Prepare decorated type/name keys
without fixed-buffer overflow or permanent failed-lookup acquisitions. Audit all
retained literal/static key consumers and manually memoized window IDs for reset/
rollback generation correctness, preserving first-acquisition order. Classify
development test-art lookup versus retained source behavior before retiring it.
Source helpers/archives and generated actual-provider fixtures remain supporting
steps within this same full-runtime slice, never substitute gameplay acceptance.
For literal raw-key caches, retain a StaticNameKey cache plus a same-type local
resolved value at the original declaration point, preserving first-acquisition
timing and consumers' value type. Exclude comments from mechanical rewrites;
inventory address-bearing and genuinely mutable uses before changing storage.
Do not mechanically rewrite manual NAMEKEY_INVALID lifecycle slots. Compile the
six actual Common/Thing providers as their own shared archive and audit all actual
creator paths, not only the standard macro. Ordinary ModuleData parse candidates
must be guarded by a matching ordinary delete owner; default tag state is explicit.
Before accepting template cloning, include inherited override-chain ownership,
AudioArray prefix/assignment retirement and SparseMatchFinder's borrowed cached
vector elements in the same owner graph. Never allow an unpublished copied
candidate to delete an accepted override chain. Stage an entire initialized audio
array before publication and preserve accepted backing on rejection. Reset derived
match caches when copying to a distinct vector owner; copied pointers into the
source vector are not valid clone backing. Keep resolveNames' cross-template
build-facility publication after every fallible local name/image preparation, or
provide exact rollback for those writes. The excluded development TestArt loader
must not remain an active side-effecting metadata lookup. Add actual source-owner
tests where separable without fake gameplay providers; whole definition execution
and coupled ModuleData suffix rollback remain required before N2 acceptance.
For the definition-wide ModuleData transaction, clone only the owning pointer
vector offside before mutation and preserve its prior backing. Existing pointer
values remain borrowed by that snapshot, not extra ownership units. On rejection,
retire each newly acquired suffix unit in reverse order, restore the original
vector/backing without allocation, then retire its dependent namespace scope.
Inner success remains owned by an outer transaction; nested rejection preserves
outer-acquired units. Retire unpublished template graphs before withdrawing module
data. Prove exact accepted pointer/backing restoration, allocation faults from a
commit-ready candidate, corrected retry, nested scopes and three lifetimes using
the actual ordinary ModuleData owner, without fake factory registration.
Move the actual ThingTemplate data constructor/copy/destructor helpers into one
runtime provider and include actual GeometryInfo backing, so the original clone
owner can execute without linking unrelated gameplay parser factories. Preserve
the source zeroed-pool defaults explicitly for the eight otherwise uninitialized
scalar fields, and reject a missing GlobalData parent before dereference. Preserve
the destination's owned override link across every fallible payload copy; copied
source linkage is borrowed, never new ownership. Verify actual generated template
clones, nonzero fresh backing, live source overrides, allocation-fault retirement
and same-source retries under both compilers. These are supporting source-owner
tests, not permission to substitute a template/factory or waive whole definitions.
Adopt the definition transaction as one coupled parser change: capture namespace,
module-data vector, complete prospective name index and accepted build-facility
flags before original parsing; preserve source self-lookup through scoped index/
override visibility. On rejection withdraw the candidate's list/override links,
restore the exact previous index backing and ID cursor, restore accepted flags,
then destroy the candidate before module-data suffix and namespace retirement.
No cleanup notification or allocation occurs during withdrawal. A duplicate
non-override definition is the original explicitly diagnosed invalid input, not
permission to mutate an accepted payload. Restore debug parse-name ownership on
every exit. Keep full Object/parser/factory execution pending until its actual
registration and runtime graph can link; mechanism-only tests do not accept it.
Registration rollback includes the complete module-template map and namespace,
not only each add call. Base and retained W3D-derived registration scopes nest;
an accepted inner base census remains owned by a rejected outer derived pass.
Preserve source procedure/interface values and original registration/key order.
The source template clone must preserve both its destination's owned override
link and override marker (including reskin copying), not import those from the
source payload. Disabled release TestArt guards are not a runtime compatibility
feature and cannot make metadata lookup create modules/templates behind the
query owner's back. Record source-only provider checks separately from runtime
registration execution. Refresh the broad graph matrix on the complete frozen
cohort, not after each guard or isolated source-header extraction.

Continue whole-runtime dependency tracing with a generated link-only root probe;
never execute an incomplete entry point or infer startup acceptance from archives.
Retire excluded CD creation/update, release DRM fingerprint checks and online
results thread bootstrap/shutdown together, preserving retained audio/music
validation and LAN. Audit factory reset root-list unlinking alongside override
publication. Required official MapCache loads use the same selected-block provider
as optional caches but propagate missing/malformed input as required-data failure;
never allow a foreign official-cache block to mutate unrelated definition owners.
Keep full parser/reset/registration/runtime tests as acceptance obligations.
Use the whole-root link census to admit the actual remaining Common gameplay
providers as a coherent archive: player/team/state machine, definition stores and
audio metadata/requests, without fake singleton definitions or device playback.
Keep standard runtime execution and platform/device callbacks as explicit later
linkage obligations within this slice. Excluded tools/DRM/online owners are not
pulled in to satisfy obsolete references. Preserve one provider per actual source
and build the complete Common cohort before evaluating diagnostics as a batch.
The next actual logical GameClient cohort includes source-owned drawable/client
updates, particle/image/animation metadata, presentation interfaces, abstract
input, weather/terrain-road data, campaign/credits/color/selection services. Audit
each against the original project; do not admit dormant files or excluded online
callbacks through directory globs. No renderer/device output is claimed from
archive compilation. Collect the complete compile diagnostics before changing
coupled source contracts; preserve one definition owner and continue full-root
linkage rather than supplying fake missing globals. For stock WWMath, audit all
retained non-inline consumers together: its matrix implementation includes D3DX;
game-owned equivalents may use unchanged public header accessors, never a
framework patch or emulated Windows SDK. Record source operation order and exact
generated math/checkpoint evidence before accepting such adaptations.
The client compile cohort exposed clocks, required direct headers, legacy for-loop
scope, locale/input encoding and raw font/image filesystem calls together. Use
the existing native wrap-width clock; retain source min/max expressions with
unambiguous std calls and explicit bounded field offsets. Convert original
CP1252 keyboard source to UTF-8 without changing code points; preserve the source
French host-layout override through SDL's public current-keymap query when a video
keyboard exists, otherwise retain the content locale. Invalid printable-key input
returns a defined zero, never an invalid empty wide-character literal.
Language configuration is an offside selected-block transaction with all font
descriptors/list backing retained and missing declared font assets rejected through
rooted read-only access. Do not install/remove fonts in the host process/system;
native text output must consume the original catalog/assets in N3, and remains an
explicit acceptance obligation. Image metadata discovery uses the rooted owner,
not Win32 handles/CWD; preserve user-before-size-before-hand-created loading and
the original top-level user-file gate. Audit image replacement/admission ownership
alongside that filesystem migration before claiming complete collection rollback.
Execute actual GlobalLanguage configuration in the existing configuration fixture:
selected locale/fonts, missing declared assets, late/foreign-block rejection,
preserved accepted name/list/descriptor backing, complete allocation-fault census,
exact terminal and corrected retry through three lifetimes. This is configuration
owner evidence, not physical font rendering. The Smudge provider is invoked by
the W3D rendering path, not headless simulation; its uncompilable WW3D intrusive
list/allocator dependency must be adapted in game-owned N3 rendering code, without
editing that framework or weakening the required heat effect. Do not pull it into
the headless archive as an SDK workaround. Retain the actual OurLanguage definition
once; assigning/changing the source locale or future SDL event routing is separate
from admission of the original logical keyboard table.
Couple remaining source math/admission helpers as one batch. For all retained
GameEngine non-inline WWMath yaw, rotate-vector, transform and identity consumers,
use game-owned free functions with unchanged public Matrix3D/Vector3 accessors and
the original operation order, signs, zero-projection fallback and double atan2
boundary. Do not define/replace library members or compile its D3DX implementation.
Prove nonzero-backing initialization, known transforms, source-derived bit-exact
results, vector/translation separation and allocation-free execution under both
compilers. Keep clipping/normalization/disabled-mask metadata in its actual source
owners. Move the existing projected-shadow singleton definition from the W3D
device TU to its logical radius-decal owner, without duplicate/fake globals;
actual headless/native renderer publication remains pending, not accepted by moving
a symbol. Inventory radius-decal admission/copy bounds and null provider handling
alongside that ownership move before runtime acceptance.
Adopt complete mapped-image ownership together with bounded discovery: selected
MappedImage-only directory loading preserves root-files-before-subdirectories and
user→size→hand-created precedence.
The complete-registry directory owner must reuse the same normalized, bounded
selection helper and real Xfer line callback, avoiding parallel unchecked path
offsets. Honor the caller's recursion flag (the old owner ignored it).
Whole collection parsing occurs in a cloned offside graph with name scope;
existing Image addresses stay stable at commit by
allocation-free payload swaps and pointer/index exchange. Raw texture pointers
are borrowed (the actual Image destructor does not release them); reject parsing
an existing raw image as already source-diagnosed invalid. Per-definition parsing
guards a pooled candidate through all fields/index admission, including late
failure and widened coordinate subtraction. Restore accepted index/backing,
objects, keys and resource counts exactly on rejection before same-owner retry.
Missing-name lookups roll back speculative keys, duplicate direct add admission
cannot overwrite an owned image, and map-preview creation gets its matching pool
guard before any fallible identity/preparation. Keep helpers in the actual source
providers and execute generated real image/collection callbacks under all3,
with full ordinal manifests, exact terminals, accepted-pointer preservation,
late second-file/foreign blocks, raw pointer rejection and repeated lifetimes.
Continue the complete startup-facing logical UI/message cohort, not individual
unresolved symbols: original message translators, window/layout/transition and
font/header managers, Shell/menu schemes, ControlBar command/data owners, Eva,
InGameUI and load screens. Inventory selected TUs against GameEngine.dsp, keep
device IME and actual rendering/video backends outside this headless archive,
and retire excluded online dependencies without losing LAN/skirmish semantics.
Collect the complete compile census before coupled portability changes. Before
actual startup execution, inspect constructors, parser/cache publication,
manual name-key lifetimes, callbacks and teardown together; archive compilation
is not gameplay/UI acceptance. Preserve actual providers and singleton ownership,
no replacement globals or no-op simulation. Xfer CRC/save/load/deep-CRC belong to
the next coupled persistence boundary; source wire/poison/rollback findings must
be durable before native stream adaptation, not rediscovered from runtime errors.
The startup UI cohort also requires defined message formatting: do not pass
nontrivial AsciiString/UnicodeString as a C varargs anchor. Use typed forwarding
into the existing UnicodeString formatter, preserving format lookup and final
message delivery. Keep source descriptor/table types complete, replace raw fixed
window path buffers with owned logical paths, and preserve callback/command
indices with native-width admission. Deprecated online overlay/load screens and
Win9x font selection are excluded; no framework header repair or SDK shim.
Follow the revealed callback graph with the complete original eleven-widget
cohort (button/check/radio, both sliders, list/combo, static/progress/tab/text).
Verified widget-owner batch: multi-selection returns borrowed Int* backing
(original callers cast an Int** through Int*), not copied indices. Introduce a
typed borrowed-output overload with an explicit message discriminator; scalar
output must never receive a pointer-sized write. Reserve listLength+1 selection
slots for the terminating -1, bound every walk, and validate the complete forced
selection before publication. Couple overlapping row/selection moves, matching
array deletion, row/column bounds, scroll selection remapping and preallocated
resize backing. Preserve valid selection behavior and diagnose original bounds/
ownership bugs; do not infer window/display callback acceptance from metadata
tests. Use the existing native clock/input policy and character carriage return
for IME edit completion, not Windows key codes or an SDK shim. Generated fixtures
exercise the real ListboxData representation and shared production selection
operations, including full capacity, late-invalid rejection, typed borrowed
output, shifts, exact allocation boundaries and repeated lifetimes under both
sanitizers. Widget constructors/parser/display callback rollback remains pending
until actual owner execution proves it.
Keep callback family typing and pointer-width message payloads intact. Collect
the whole source census before corrections; field/data/list/cache/callback
ownership and same-owner teardown remain part of the surrounding UI transaction,
not inferred from successful compilation or isolated formatter tests.

Persistence owner cohort is one coupled transaction: actual XferSave/XferLoad/
XferCRC (including XferDeepCRC), GameState and Recorder. Include every retained
provider in the source census before adaptation. Derive string/block/version/
CRC protocols from source; protect user reads and atomic save/diagnostic writes,
capture storage ownership, and abandon unpublished outputs without fallible
destructor close. Session poisoning must include typed semantic rejection and
snapshot/postprocess callback failure, not merely write errors. Save block stack
admission precedes placeholder writes; retirement follows successful backpatch.
Load must reject malformed/truncated/beyond-EOF blocks and decode complete offside
strings using fixed UTF16 wire units, not native wchar_t width. Ordinary ASCII
length is Byte<=255, deep-CRC ASCII length is UInt16<=16385; Unicode is Byte<=255
UTF16 units. Preserve CRC grouping, network order and source leftover arithmetic
while removing unaligned native UInt32 reads. Raw stdio/unchecked position/native
time_t/Unicode text replay methods are pending and cannot be executed against
supplied content as accepted runtime. No fake TheGameState or Recorder globals;
fixture postprocessing uses the actual retained owner or explicitly documented
public XO_NO_POST_PROCESSING, never a replacement symbol. Test commit-ready poison,
every callback family, all coupled allocation/I/O boundaries, exact resource
residuals and corrected same-owner repeated sessions under both sanitizers before
whole root/scenario/checkpoint acceptance. Archive compilation remains inventory,
not persistence or gameplay validation.
Generated transfer fixtures link the actual source transports and registries.
Use standard per-function/data sections on repository-owned static targets and
fixture-only linker garbage collection to avoid retaining unused presentation
methods through an unrelated source TU. This is ordinary compiled reachability,
not replacement globals or an acceptance waiver: full native entry/root still
must resolve and execute its complete live graph. Retain actual GameState
postprocess admission and explicitly use its public no-postprocess option for
standalone generated transfer fixtures; test actual postprocess ownership later
in whole-state integration. No dependency source/flags are patched.
The parallel typed owner audit includes aggregate, ID-list/vector, science-vector
and named-mask transfers. Validate UInt16 count narrowing, build complete load
containers offside before swap, and preserve original empty-destination rules.
Named masks load into a candidate rather than clearing accepted state before a
late name fails. Canonical mask CRC covers declared logical bits in explicit
32-bit source words, never sizeof(this), native std::bitset padding or enlarged
word backing. The original pointer-size CRC truncation is a verified source bug;
fixing it changes diagnostics, not gameplay or the named-mask save representation.
Scope failure poisoning around complete helper operations as well as transport
callbacks. Expand generated fault manifests and negative late-field evidence
for these parallel owners before broad validation; whole-state graph rollback,
replay stream/header publication and actual native scenario remain pending.
Follow the real replay dependencies GameInfo and GameMessageParser as part of
this same owner cohort: inventory their complete original providers before
native adaptation, preserve retained LAN/skirmish grammar, guard parser-node
construction and complete command/header candidates before publication. Replay
epoch/header offsets remain source fixed-width, and ordinary byte-mode UTF16
writers must pair the reader; do not execute raw native time_t/stdio writers as
accepted playback. The full root link-only persistence census is113 to110:
23 earlier symbols resolved,20 deeper GameInfo/parser/map/message-box providers
exposed. Archive reachability is inventory, not runtime acceptance.
Frozen persistence validation exposed a real layering constraint: both sanitizer
linkers retain unrelated parent-game diagnostics through Xfer's direct GameState
references. Do not suppress vptr/global checks or fake a singleton to link a
transport fixture. Introduce an explicit game-owned native transfer service
binding (portable map encode/decode and snapshot notification), registered only
by the real GameState after successful init and withdrawn by raw owner identity
before teardown. Low-level transports require the relevant bound service for
contextual operations; standalone fixtures retain the existing public
XO_NO_POST_PROCESSING and cannot claim whole-state acceptance. Registration is
allocation-free, rejects incomplete/replacement owners without effects, and
preserves prior binding on failure. Keep this coupled with service lifecycle,
typed transfer poisoning, missing-provider negatives and all3 validation in the
same incomplete slice. The real root graph still must compile/link/execute fully.
Audit every contextual adapter before the next freeze: ScienceStore and
UpgradeCenter direct lookup methods also retain their parent definition/runtime
graphs under sanitizer diagnostics. Route science-name and named-upgrade codecs
through the same complete native service contract, supplied by the actual
GameState-owned runtime binding. Preserve dynamic science identities, template
iteration order and upgrade aliases; complete named upgrade encoding is built
offside and its UInt16 count admitted before output. Generated service tests
prove only dispatch/admission/fault contracts, never real store semantics. This
is ordinary original-game dependency inversion, not a framework fork or runtime
stub, and full native-root acceptance remains mandatory.
Replay-provider source preflight found five GameInfo compile gaps (native clock,
direct GlobalData completeness, loop scope), plus coupled ownership/representation
issues: raw strdup buffers can leak on throws, tokenization mutates shared
AsciiString backing, NAT input is cast into an unfixed enum before validation,
duplicate start spots compare the current rather than preceding slot, and base
local-IP backing is never initialized. Correct these as one original-provider
batch without importing online endpoints. Keep whole slot/map publication
pending until offside state/setter callback execution proves rollback.
GameMessageParser must initialize both ends/counts, admit raw Int type/count and
complete Byte-sized argument/run budgets before allocation, count acquired runs
centrally, and unwind all pooled nodes when construction from a message fails.
Preserve valid recorded run grammar. Test real source pools, every acquired-node
boundary, exact residuals, same-owner retries and repeated lifetimes before whole
Recorder command/header publication; a source-only build is not that acceptance.
Player-name conversion is a retained generic UTF8 utility despite its online
directory location. Port it through game-owned native source strings, preserve
single-line CR/LF-to-space decode and valid UTF8 bytes, and use offside results
instead of Windows conversion buffers/mismatched deletion. Validate numeric
tokens with defined-width parsing before enum conversion and reject exhausted
option budgets; never truncate inside a UTF8 scalar. Cover generated multilingual,
supplementary/malformed/overlong/truncated/control/budget cases with the broader
parser ownership fixtures before frozen all3 validation.
Test/implement the actual message value owner with its run parser: split the
original GameMessage and GameMessageList implementations from parent translator/
dispatcher code, preserving real methods rather than duplicating them. Add an
explicit player-context constructor for fully admitted replay/generated messages;
ordinary messages still obtain the real local player. Initialize all message
links and argument aggregate backing, admit the Byte argument budget before pool
acquisition, and validate list ownership before publication/removal. This keeps
source value tests independent of unrelated parent sanitizer metadata while the
actual runtime dispatcher remains mandatory. Cover pooled failed constructors,
append rejection/rollback, type-run counts, maximum arguments, list transitions,
every pooled-node boundary/terminal/retry and three complete lifetimes under
both compilers. Do not publish incomplete Recorder messages merely to make tests
link, or claim a value fixture as GameLogic execution.

Recorder command cohort continuation: construct each complete source command
offside, with exact pooled message/run/argument ownership, before command-list
publication or the first output byte. Share one game-owned wire codec between
the actual Recorder and generated actual message fixtures. Preserve the source
frame/type/player/run-table/payload ordering, explicit little-endian 32-bit
scalars, one-byte booleans and two-byte WIDECHAR code units. WIDECHAR is one
source UTF16 unit (including individual surrogate units), not a Unicode string;
do not invent scalar validation or finite-float policy at that protocol boundary.
Admit raw command/type/count/bool representations before conversions, reject
truncated fields, and distinguish a clean inter-command EOF from partial frame
bytes. Analysis and suppressed commands retire exactly once. Test complete
roundtrips, every truncated byte, late-invalid run/bool fields, output failure,
pooled exhaustion and retry, exact residuals and repeated process lifetimes.
Whole protected session/header/GameInfo/CRC/RNG publication remains pending;
these command fixtures cannot establish Recorder or N2 acceptance. Keep this
cohort in the existing incomplete transaction, not a support-only commit.
Protected Recorder session continuation: replace the raw active replay FILE with
an explicit game-owned session over captured NativeUserStorage. Recording uses
the existing protected atomic output/backpatch protocol; playback acquires a
complete immutable protected input bounded by the original signed file width.
Destructor/reset abort unpublished output; only explicit successful stop commits,
reporting published-but-unknown durability truthfully. Every rejected stream or
parent operation poisons the preparing session, including callback/diagnostic
exceptions before dereference. Record commands directly through the real codec;
preserve byte-mode flush visibility without inventing buffering. Retain original
optional recording availability behavior. Convert every active-stream callsite,
fixed four-byte epochs/header offsets and UTF16 terminators as one coupled batch.
Native context indices admit -1..MAX_PLAYER_COUNT-1 before enum/message acquisition.
Test real protected file publication, seek/backpatch, immutable playback, clean
EOF/truncation, commit-ready rejection, every I/O/allocation boundary, exact
residuals and retries; source generation only. Whole header/GameInfo/CRC/RNG
publication still cannot be accepted from isolated session support. Debug-only
stats/archive paths require separate protected owner admission before activation;
do not run them or provided recordings while they remain raw.

Worktree cleanup checkpoint explicitly requested by the user: all accumulated
source/build/test changes match canonical527
42461aba003d4547c7e5541645a293a2c84ac24a23e69927432d8a9eb0a9be3b;
including toolchain531
40b39537079204eb279987e7e947c2ebf5fd58f639b9105f3b80cb0ef68b1fbd.
Existing full178/178 normal/GCC/Clang evidence is current; no unchanged expensive
matrix is rerun just to commit. Persist one coherent implementation checkpoint,
including plans/findings/evidence/status. Slice03 remains in_progress and its
completion commit/actual scenario evidence remains pending. This user-authorized
checkpoint supersedes the earlier uncommitted-support working convention only;
it does not change milestone scope, dependency order or acceptance.

Run the frozen complete applicable CTest matrix, required asset-integrity checks,
and artifact/authority freshness checks. Every N2 acceptance criterion maps to
direct evidence; caches/writes stay outside supplied roots. Commit the completed
slice and N2 delivery status. If required private data is absent, retain passing
generated evidence and precise PRE04 unblock condition rather than accepting N2.

Resume after user-requested checkpoint a62794b2 (clean worktree): complete the
coupled GameInfo publication owner before broader Recorder startup integration.
A bounded value journal captures every base option/flag and each actual slot
payload, never a copied GameInfo or alternate LAN parent. Rejection restores
original slot identities and string backing without callbacks/allocations;
nested success remains owned by the outer transaction. Guard reset, multi-slot
mutation, map/setter work, whole options publication and replay header startup.
Prepare localized slot names before mutation and initialize fresh slot IP while
preserving reset semantics. Extract the actual map-companion path definitions
and CRC default into their logical providers (one definition each), so generated
actual-owner fixtures do not acquire unrelated network transfer/RTTI graphs.
Validate source-derived map masks/availability and slot flags, nested rejection,
all allocation ordinals/exact terminal/retry, prior backing and three lifetimes
under GCC/Clang sanitizers. No fake GameInfo/GameState/LAN globals or mocked game
logic; isolated owner evidence cannot complete Recorder, simulation or N2.
Both sanitizer linkers retain unrelated parent/runtime metadata through the
combined GameInfo replay-serialization TU. Separate its actual setup/slot values,
apparent-player presentation and replay/snapshot serialization providers without
changing methods or adding replacement globals. All providers stay in the real
bootstrap archive; only genuinely unreferenced methods may retire from standalone
fixtures. This is dependency separation, not a vptr/global sanitizer waiver.

Recorder startup continuation must prepare using a distinct actual Recorder with
its own ReplayGameInfo slots/session/CRC, never move its borrowed slot pointer
array into the registered Recorder. Read the complete original tail and first
frame before world reset; admit difficulty/mode before downstream conversion.
Prepare pooled NEW_GAME and clean-EOF CLEAR messages offside, preserve the source
CLEAR-before-NEW ordering, then resolve their player context against the actual
post-reset PlayerList. Engine reset can reset the registered Recorder but cannot
retire this separately scoped candidate. Transfer setup payloads and owned
session/CRC/strings without allocation; retire prior resources through the candidate.
Preserve analysis omission and all three source RNG streams. An exception during
actual destructive world reset must mark engine shutdown and propagate, not claim
an intact prior world or successful playback. Test complete ReplayGameInfo adoption
with both embedded slot identities retained; keep actual whole Recorder/reset/
message/RNG execution pending until the full native graph links and executes.

User-requested worktree cleanup checkpoint: all configured targets build and
all183 tests pass normal GCC and normal-host GCC/Clang ASan/UBSan/LSan
(17.98/83.24/67.88s). Current source identities and retained sandbox LSan failures
are in N2-runtime-support.md. Setup/replay-wire/adoption tests are executed;
whole Recorder/runtime remains compile-only. Commit this coherent progress
checkpoint without completing this slice or milestone. Generated files, libraries,
retail assets and recovery storage are excluded.

Resume from clean a815cbef: refresh actual root link dependencies after setup and
replay providers, then admit source-owned BuildAssistant, FunctionLexicon,
TerrainVisual and VideoPlayer together into their actual archives. Inspect whole
construct/init/retire paths and batch Linux/type/include corrections before
validation; do not replace any gameplay or callback with stubs. StatsCollector
still writes through CWD and must not become executable until protected user
storage replaces those writers. Keep root linkage evidence separate from startup
execution. Use a durable, EXCLUDE_FROM_ALL link-only probe that retains actual
GameEngine::init and is never registered as a runtime test. This diagnostic target
is not the native entry point or accepted gameplay fixture. Complete all three
archive builds, inspect the refreshed unresolved cohort and continue actual
factories/map/GameState execution. No new slice or milestone is created.

Provider census exposes a source-layering issue: NativeWindowCallback's typed
constructors were not constexpr, so linking FunctionLexicon lookup methods
retains dynamic startup initializers for every GUI/Internet callback table,
even when the table-owning init function is unreferenced. Make these existing
pure value constructors constexpr so original static tables initialize as data;
do not suppress sanitizer metadata or remove live callbacks to reduce a count.
Validate all existing typed callback families and refresh the root census.

TerrainVisual's actual source filter has legacy for-scope/MIN compiler assumptions
and an unguarded temporary workspace across virtual sample/write callbacks.
Keep the exact dome/velocity cap/gravity behavior while using owned initialized
workspace and defined radius/coordinate/size arithmetic. Generated grid oracles
exercise the actual filter, zero/expired life, malformed boundaries, callback
exceptions and allocation failure/retry under both sanitizers; they are isolated
algorithm/ownership evidence, not substitute WorldHeightMap or terrain acceptance.

The sanitizer filter link exposes a header-layering dependency: TerrainVisual
imports MapObject merely for its four-method height-velocity interface and shared
map scales, pulling ThingTemplate/BehaviorModule and the whole unlinked parent
through sanitizer type metadata. Move the actual interface/scales to their common
declaration headers and keep MapObject consuming those same declarations. No fake
typeinfo, alternative interface or sanitizer waiver. Initial sanitizer binaries
were not produced; their missing-executable tests are not acceptance evidence.

CPU MapObject continuation in this same transaction: move the actual original
MapObject definitions out of WorldHeightMap.cpp into Common/System/MapObject.cpp,
retaining properties, naming, template override selection, waypoint/team and linked
object behavior. Retain render/shadow/bridge associations; their non-null render
ownership becomes explicit game-owned acquire/release callbacks captured per
reference, so CPU ownership does not import renderer/library private headers.
Every bridge slot is initialized before fallible construction. The retained W3D
bridge caller supplies its existing public reference operations explicitly; no
library changes or alternate gameplay providers. Missing render ownership rejects
before changing the accepted reference; equal numeric pointers in distinct slots
remain separate acquired units. Null withdrawal needs no service callback lookup.
Exercise actual pooled MapObject construction/defaults/copy/link retirement,
generated properties and failed construction/duplicate/ID publication, all render
reference units and source naming order through repeated GCC/Clang sanitizer
lifetimes. Whole WorldHeightMap input and gameplay still require native execution.

The map source retains source bridge enum values/hash while exposing its declaration
without TerrainRoads/BodyModule imports. Whole IDs/default construction prepare
properties and namespace offside, with complete failure-prefix retirement.
Both sanitizer links reveal that including SidesList solely for the team-validation
method retains the entire live parent through its type metadata. Separate that
actual method into MapObjectTeams.cpp; keep it in the production graph, not an
alternate provider or suppressed RTTI check. Newly extracted game-owned providers
are explicit target_sources, not falsely claimed as original DSP-listed Common
files. Verify one active definition and original device-definition retirement.

Resume from user checkpoint57c262a8. Linker archive-admission trace proves the
actual MapObject::validate reference admits MapObjectTeams before section GC;
SidesList RTTI then admits AI/GameLogic/GUI parents. ThingTemplate RTTI already
selects the correct NativeThingTemplateData provider. Move the complete actual
team-validation entry point to the team provider, preserving property-before-
namespace rollback and atomic validate semantics. Do not suppress checks or
replace types/globals. Rebuild normal/GCC/Clang and exercise the actual MapObject
families; keep whole team/world/runtime integration as separate pending gates.
Extend this MapObject cohort with actual nonnull ThingTemplate binding and a
two-step override chain, template-derived unique IDs and duplicate identity.
Borrow/restore the actual GlobalData publication through complete template/map
retirement. Generated templates are original owners, not replacement providers;
detach borrowed stack override links before their source destructors retire.

Continue from174a2946 with a coupled startup-provider graph batch. Compile the
actual original shared BitFlags definitions and the source callbacks for control
bar/tooltip, diplomacy, in-game chat, quit and score-screen fixes. Preserve their
behavior and typed callback contracts; batch native include/type/for-scope fixes
without Win32 shims or fake callbacks. Separate the original shared color enum,
defaults and OnlineChatColors parser from GameSpy transport, keeping its exact
field names/colors and atomic parse admission. Move genuine borrowed runtime
publications out of device/menu/transport implementation TUs where necessary,
removing their old definitions rather than adding substitute singleton owners.
Validate source tables and positive/malformed/failure/retry color input under
both sanitizers, refresh normal and sanitizer root diagnostics, and keep unrun
whole menu/network/entry/scenario paths explicitly pending. No WAN functionality
or N3/N4/N6 acceptance is introduced by compile-only source reachability.
The initial callback compile found unavailable GameSpy Peer/GP SDK headers in
Diplomacy and ScoreScreen, plus source timer/include/C++20 keyword assumptions.
Retain online implementation uncompiled; extract the actual shared color enum,
table/parser and score-screen fixup/state without transport. Whole diplomacy and
score-screen menus remain pending, not compiled/accepted. Compile the four actual
control-bar/tooltip/chat/quit providers using nativeMilliseconds, explicit
GlobalData imports, renamed local requirements text and defined pointer checks.
Six genuine borrowed network/menu/input publications relocate once into the
production startup provider; original owners still control lifetimes. Source
census locks exactly one definition each and original color enum/default hashes.
Add three configuration families for actual color/table mapping, malformed and
complete allocation/retry/terminal admission, retaining every sibling color.

Continue from7ecee6df with one coupled native asset/mod namespace batch. Preserve
the actual original loose-before-archive selection and mod BIG overwrite order;
directory mods admit their BIG contents, not a new loose-file precedence rule.
Prepare the complete replacement index offside, validate attached Data/Cache roots
against it, and publish without changing accepted backing on any failure. Direct
base remount remains forbidden while user storage is attached. Native output and
scratch owners capture a shared, callback-free namespace lease; reject any mount
change until every such owner retires, including committed scratch cleanup. Check
complete physical leaf targets for writes, copies and removal, not just parents.
Exercise generated archive overrides, loose precedence, missing/malformed mods,
root alias rejection, live output/scratch leases, same-process corrected retries
and every allocation failure prefix with exact resource retirement under GCC and
Clang sanitizers. Retire GameEngine's obsolete duplicate Local/Archive service
factories and asset-writing TGA converter; a requested legacy asset update fails
explicitly instead of writing content or claiming conversion success. Native
command-line binding, complete GameEngine startup and simulation remain pending;
this batch does not fabricate their acceptance. Commit implementation, tests,
source findings and honest supporting evidence together in this same slice.
Protection roots are deduplicated;100 repeated same-mod admissions must preserve
exact standard-backing and descriptor counts, in addition to complete owner
retirement across three lifetimes. Keep the pre-growth-review matrix historical
and validate the final frozen cohort after this coupled lifecycle check.

Native namespace support checkpoint: all198 final frozen tests PASS normal
GCC19.59s/GCCsan91.67s/Clangsan72.16s, all checks intact. Canonical555
SHAe95ae6eb90b4e9e9d74f93a7098a6ad5441e7752ce8095ff24a9071014ff5281;
toolchain560 SHAa92eb02aab978e50c22fe4e5e4b594c73f295ec22804aeeb1cffd61a78379b42.
Actual native mod/protected-output lifetimes pass all44 failure/retry pairs and
100 repeated admissions; full matrix includes the three coupled consumer
manifests repaired through independent discovery, not weakened coverage.
Final never-executed root link fails12/224/235. This is a coherent progress
checkpoint in the existing incomplete slice, not startup/scenario acceptance.
Continue native CommandLine mod binding and protected StatsCollector/capabilities,
then full callback/map/factory/GameLogic/Recorder/scenario gates. Evidence:
`evidence/qa/N2-runtime-support.md`, latest native namespace cohort.

Continue fromfa973fde with actual CommandLine provider integration. Inspect the
complete option table/handlers together, including obsolete DX floating-point
toggles, raw argv logging and authoring writes, before compiling the provider.
Retain active original option names and effects, case-insensitive matching and
unknown-option skipping. Validate argv shape and integer encodings before
dispatch (defined overflow/malformed rejection); consume value options correctly.
Native mod selection resolves relative names within configured user data, uses
the admitted native archive namespace, and restores both mod publications with
no allocation if admission fails. This does not claim rollback of unrelated
options or complete GameEngine startup. Reject unsupported asset/debug authoring
requests explicitly instead of writing CWD, assets or inventing DX state. Cover
the actual release table, malformed/missing/overflow/null inputs, supplied mod
selection and all allocation failure/retry/terminal paths through three repeated
original GlobalData/storage lifetimes under GCC/Clang sanitizers. Keep protected
StatsCollector and complete debug-authoring execution pending; its inspected
writer/update/end ordering findings must be retained durably. This remains the
existing N2 slice03, not a new tiny plan or a replacement simulation.

CommandLine supporting checkpoint: related71/71 PASS normal18.61s/GCCsan64.71s/
Clangsan63.92s, all checks intact; final added Version effect assertion PASS
all3 in0.15/0.62/0.58s. Three complete original owner lifetimes exercise every60
allocation failure/retry pair and exact terminal60. Final canonical555
SHA62f420b4d18d6fbc13cd6ea603f3a131b30e8c38c557f3bd77eacf4a5ad6784d;
toolchain560 SHA95f981d78922a4e8d49cb3334cce39e45564b488ea22e44c2983d9206a449938.
QA records the related-run pre-final-assert identities and initial fixture defects
honestly. Link-only root fails11/223/234, never executed. Full201 pending,
StatsCollector/capability/callback/CPU map and actual process/scenario pending.
Keep this slice and N2 in progress. Continue from this actual source integration,
not archived providers or acceptance. See QA's current CommandLine cohort.

Continue from8cab2148 with the whole actual StatsCollector lifecycle. Separate
its real GameLogic/player sampler from game-owned logging state rather than
inventing GameLogic or borrowing renderer/private headers. The default source
adapter still reads original objects, player money/index/side, configured interval
and map/LAN metadata. An explicit borrowed sampler/storage constructor permits
generated ownership tests of the actual collector, not simulation acceptance.
Capture dependencies before use; do not call simulation from asynchronous threads.
Replace every raw header/row/footer writer with protected atomic user-storage
publication and streamed append of accepted backing. Preserve TSV fields, row
order, command selection and normal sampling intervals. Prepare filename, header,
row/footer and counter/frame changes offside; publish state only after file
publication. Optional storage failures expose failed output status without
breaking simulation, while allocation/invalid-input failures unwind candidates.
Treat published-but-durability-unknown as published, never roll back accepted
state or append twice. Define elapsed/frame rollover arithmetic and prevent
duplicate footer/retry rows; reset establishes fresh scroll/unit baselines.
Encode unsafe filename bytes reversibly, keep original report metadata unchanged,
and admit requested directories only inside captured user storage. Test actual
message filtering, TSV rows/end ordering, repeated reset/teardown, rollover,
malformed naming/intervals, all storage phase failures and every allocation
failure/retry/terminal for reset/update/end under GCC/Clang sanitizers. Compile
the real sampler/default constructor into the original gameplay graph and refresh
root diagnostics. Generated sampler tests are logging/ownership evidence only;
whole original GameLogic/object/player sampling and scenario remain N2 gates.

Pre-commit lifecycle review: a rejected new-session reset withdraws row/footer
admission until a corrected reset publishes successfully. Accepted old backing
remains unchanged, but new-world samples must never enter the old session report.
Preserve absolute root directory rejection and explicit debug directory joining.

Statistics checkpoint: all configured targets build all3; related67/67 PASS
GCC16.63s/GCCsan66.19s/Clangsan63.04s with all checks intact. Exact repeated
reset/update/end allocation terminals22/32/37; six actual collector families
plus related storage/configuration/message/runtime tests. Canonical559
SHAbcc50fb3f0c0cc746c400170b9cfeb85b9efdcf47d81cadcc941fb050eafac0e;
toolchain564 SHAabf08c7c215ca0b6c3360776b6d62dd10a84d18c013db4fc05b63a285f1e7d5c.
Never-executed root-link diagnostics fail7/219/223. Full207 not run. Whole original
sampling and N2 process/scenario remain pending. See
`evidence/qa/N2-statistics-checkpoint.md`; this coherent commit does not close N2
or the governing slice03.

Continue from717a474b with the complete native fatal-error boundary used by
GameEngine exception handling and GameLogic invariant failures. Both original
ReleaseCrash and ReleaseCrashLocalized must terminate with failure even before
initialization or after owner withdrawal, without looking up simulation globals,
allocating, invoking teardown callbacks, or modifying assets/CWD. Replace the
Windows dialog/raw-stack/raw-reason report with a fixed redacted stderr message;
do not dereference or echo caller selectors/localized identifiers. No fake game
provider or returning fatal stub. Use bounded POSIX output and immediate failure
termination; failed stderr output still must terminate. Test both entry points
in generated child processes, null/hostile reason data, exhausted allocation,
closed/broken stderr, and no atexit callbacks or output files. Run focused and
related original-core/configuration/runtime tests with normal/GCC/Clang
sanitizers, refresh link-only diagnostics, and commit one coherent checkpoint.

Source findings for subsequent coupled work: original testMinimumRequirements
lives in device-owned W3DShaderManager, returns true and supplies legacy CPU/GPU
profile identities. RunBenchmark has only a header/project here, no implementation.
GameLOD also writes raw Benchmark.txt on force-benchmark and overwrites unknown
CPU identity with P3 defaults. Do not fabricate benchmark values or silently
interpret a modern GPU as an accepted old profile. Native capability/LOD and
its protected optional report must be implemented together. Diplomacy and
start-spot functions live in full GUI menus with GameSpy dependencies; extract
or decouple legitimate original behavior rather than installing no-op callbacks.

Native fatal checkpoint: both original entry points now terminate failure without
owner/global lookups, allocations, raw selectors, callback-driven cleanup or
report files.27 generated cases test complete error-output/exit behavior across
three repetitions. All configured targets build all3; related74/74 PASS
GCC16.78s/GCCsan67.13s/Clangsan63.74s with checks intact. Canonical561
SHA8638d4c6722e00c1979964ffff6704522d5e1acea9b4da19b88b5869effc481a;
toolchain566 SHAcef73d71e18665ba890676775d229e72166b8e003c1a318d08ed08f131917004.
Never-executed root link still fails6/218/222; full209 pending. Source-reviewed
LOD and whole diplomacy/start-spot lifecycle findings are durable in formats.
See `evidence/qa/N2-fatal-checkpoint.md`. N2/slice03 and whole original startup/
simulation/default stats sampler remain unaccepted.

Continue from45b61c12 with explicit original FunctionLexicon table ownership as
the first coupled GUI/startup foundation correction. Generic lookup/publication
currently shares a TU with the complete menu table metadata; GCC/Clang sanitizer
module initializers admit its GUI dependency closure even without invoking GUI
initialization. Do not disable sanitizer instrumentation or replace callbacks.
Move the original tables/default constructor to a dedicated compiled TU, preserve
all table/declaration bytes (SHAc5070a715576d4bd607502f9bbf987efd98bd8c0f81b6a08e683db5af53b4d3d),
and separate borrowed singleton publication. Capture bounded explicit table
descriptors in the actual FunctionLexicon owner for generated/native headless
callers. An explicitly empty set means no GUI callbacks; it does not claim GUI
initialization. Default construction still requires all original GUI providers.
Init/reset/typed lookup and device-first selection must execute in the actual
owner; failed descriptor/table/key admission must preserve accepted publication
and support corrected same-owner retry. Dependencies remain borrowed and valid
through service lifetime; no table/address serialization or ownership crossover.

Test actual FunctionLexicon with generated callbacks and empty headless metadata,
missing name service, malformed/sentinel/prototype/duplicate/device-table input,
every allocation failure/retry with exact terminal and repeated complete owner
lifetimes under GCC/Clang sanitizers. Source-lock the complete original GUI table
payload; retain a default-constructor link-only diagnostic that must still expose
missing GUI providers. Compare normal/sanitized startup diagnostics but never use
reduced counts as startup acceptance. Then continue complete map/diplomacy native
source admission using the already-recorded lifecycle and projection findings.

Retain the source name namespace's accepted monotonic interning on failed cold
registration: unpublished entry keys and table spans roll back, but legitimately
interned names retire with the whole NameKeyGenerator. Warm failures must retire
exactly; cold residuals must equal the counted interned-name ownership units.

Actual lexicon checkpoint: all configured targets build all3; related67/67 PASS
16.23s/65.25s/62.24s; final strengthened six-family fixture PASS0.23s/1.33s/0.96s.
Three complete owner lifetimes, exact warm1/cold9/device9 fault/retry terminals.
All original GUI tables source-locked; default constructor still requires genuine
providers. Startup link-only diagnostics fail6/8/15, default GUI212/214/222,
never executed. Final canonical563
SHA6944a8345a61aa61246ce149590bfdc8dcbe84807f4bf2a221261d98ffbb6871;
toolchain570 SHAf2be068e779cf985d6eaad2cf3b7c75edab676b1d896aec0f385bbe159a2446b.
QA retains the earlier related-run identities and their final fixture-only delta.
Full215, actual GameEngine startup/scenario/statistics sampler and physical GUI
remain pending. Continue map/diplomacy/capability/IME/warnings/terrain/hosting roots
as coherent providers from documented source, not no-op definitions.
See `evidence/qa/N2-lexicon-checkpoint.md`; N2/slice03 remain in progress.

Continue from adf48b9e (clean worktree) with the complete map-preview placement
cohort inside this same slice. Extract the original five placement/update entry
points and marker publication from the GameSpy-dependent skirmish menu, retaining
real window/gadget/text calls, not success stubs. Introduce a transactional CPU
layout owner used by those entry points: original aspect-fit float/truncation,
inverted map Y, half-marker offsets, reverse supply/tech list order, ordered
start-spot collision adjustment, sparse controls, and source actual/apparent
GameSlot selection. Validate dimensions/extents/counts/finite coordinate and
integer conversion/addition together. Use common local coordinates for collision
tests; reject malformed candidates before marker/position publication. An absent
map/window hides available controls and withdraws marker state. Look up fallback
images in the current collection on each call, never cache borrowed pointers.

Inspect preview-copy acquisitions and failures together. Guard source/destination
File handles and bounded buffers; validate preview prerequisites and .map suffix
before deriving names. Optional preview failure must fall back without publishing
an Image backed by an unsuccessful copy. No retail mutation or distributed cache.
GUI callbacks may still be fallible after CPU admission; do not claim atomic GUI
message rollback or complete window/image lifecycle acceptance from CPU tests.

Own only map placement/layout source/header, existing MapUtil/menu consumers,
explicit CMake source/test registration, generated fixtures, this plan, formats/
graph docs, QA and delivery state. Test actual layout/MapMetaData/GameInfo owners:
source-derived valid geometry, sparse/collision/slot selection, malformed/extreme
inputs, candidate failure/retry and exact terminal, repeated complete ownership.
Build configured targets and run focused then related matrix under normal GCC,
GCC and Clang sanitizers. Real GUI bindings remain compiled rather than falsely
accepted. The attempted null-view fixture retains the real GameWindow callback
closure under intact sanitizer instrumentation and cannot link before the
remaining original GUI providers; retain that as an explicit diagnostic limit,
not a reason to stub them or disable instrumentation. CPU/storage/slot tests
must remain separate from this still-incomplete GUI execution gate. Retain
link-only startup diagnostics; do not execute them. N2 still requires actual
native factories, startup and compiler-matching original simulation.

Link-map evidence refines the visibility dependency: calling the actual apparent
start-position method currently admits PlayerTemplate metadata from the combined
GameInfoPresentation TU, then Money/Player/audio/GUI. Separate the original
scalar visibility/alliance predicates from translated template-name presentation,
without suppressing any instrumentation or changing random/team/observer policy.
The shared original alliance helper must check borrowed game/local-slot identity
before dereferencing: it currently calls getConstSlot(-1) before rejecting an
absent local slot. Withdraw MultiplayerSettings publication from its metadata
TU independently. Add actual random-start visibility and missing/local/team/
observer tests; constructor fixtures must supply actual required FileSystem and
GlobalData services rather than bypass their preconditions.

Map-preview checkpoint: all configured targets build all3; focused7/7 PASS
1.09s/8.26s/4.82s, related89/89 PASS18.41s/79.78s/71.98s with sanitizer/leak
checks intact. Exact layout5/copy-allocation21/copy-storage7 terminals across
three repeated complete owners all3. Actual scalar visibility/local-alliance
tests preserve source random/team/observer behavior and validate borrowed
identity before slot lookup. No actual GUI lifecycle/render claim. Never-executed
startup probes still fail4/6/13; full222 not run. Canonical570
SHAfc26d016de1d840104ac96874c0c52eeb0031cf4ccfcd22d1e8352aac08ba9e8;
toolchain577 SHAb06a60d3ed6c94fd552914d84e6d78d1d45d61d39173005cc5b9bd323fbf05a7.
See `evidence/qa/N2-map-preview-checkpoint.md`. This coherent provider checkpoint
does not close N2/slice03; continue diplomacy/capability/native boundaries, then
actual factories and original startup/world/scenario acceptance.

Continue from55f23984 (clean worktree) with the original diplomacy lifecycle
cohort, not individual unresolved symbols. Keep actual source UI/row/mute/
animation bindings and preserve ordered, case-sensitive briefing deduplication,
clear-plus-add semantics, stable mutable GetBriefingTextList identity and briefing
retention across ResetDiplomacy. Separate the briefing owner/public entry points
from optional presentation. Stage list candidates before publication; capture a
borrowed synchronous view callback, reject conflicting/reentrant binding, detach
without callbacks, and permit corrected retry after callback or allocation failure.
If a view partially fails, retry must resynchronize the complete candidate, not
duplicate an append. No callbacks on destruction or from render/audio threads.

Compile the actual Diplomacy.cpp independent of excluded GameSpy/Internet buddy
service replacement (spec exclusions); keep unsupported buddy tabs hidden, no
returning fake provider. Initialize layout/animation candidates under guards,
validate required windows/services before publication, and capture legitimate
UI/manager parent identities for teardown. Reset detaches the briefing callback
and withdraws all borrowed controls/IDs before window destruction, with legitimate
parents published through cleanup and borrowed publication restored afterward.
Hide preserves live layout/parent identity and only withdraws row bindings.
Populate without a live view omits presentation, not gameplay. Real nonnull GUI
execution remains an explicit pending gate until native factories/providers link.

Inspect DisconnectMenu construction/destruction together and split its actual
empty destructor and initialized borrowed-manager/menu state lifetime from GUI
methods, supplying genuine RTTI rather than a synthetic typeinfo object. Constructor
must define pre-init visibility. No socket/LAN implementation or N4 acceptance
is implied; concrete LANAPI RTTI remains its own native-interface dependency.

Own only original diplomacy header/source, new shared briefing owner, disconnect
lifetime/source, explicit build registrations, generated fixtures and this plan/
formats/graph/QA/state. Test actual public briefing entry points and independent
owners, captured views, binding/update/reset/detach/reentrancy, stable list identity,
case/order/duplicates and every allocation/callback failure with retry and exact
terminal across complete repeated lifetimes. Test actual DisconnectMenu pre-init
visibility and borrowed pointer lifetime without GUI claims. Focused then related
normal/GCCsan/Clangsan builds/tests; preserve intact leak checks. Never execute
link probes. Continue all remaining native startup providers/factories and actual
original simulation; N2/slice03 remain incomplete.

Surrounding lifecycle review found AnimateWindowManager acquires eight raw helper
objects before constructor completion, and registerGameWindow acquires a pooled
AnimateWindow before fallible list insertion. Guard all helper candidates and
the pooled registration together. Destroy animation/rest-position borrowing
before retiring its windows. Drain the original window destroy queue through a
cleanup-only owner API while legitimate UI/manager parents remain published;
never run a simulation/input frame merely to finish cleanup. Actual animation/
nonnull GUI constructor fault execution remains required at the native-provider
gate if current missing GUI dependencies prevent its fixture from linking. Do
not infer that ownership acceptance from the separate briefing model tests.

The LAN RTTI dependency is avoidable concretization, not a missing packet reader:
all TheLAN consumers use the existing LANAPIInterface request/status contract,
but publication is typed as the Windows LANAPI implementation. Publish the
existing abstract interface, retaining its pure methods and adding only the
already-existing concrete LookupPlayer/GetLocalIP queries to that contract.
Native N4 transport remains required; do not instantiate a fake LAN, synthesize
RTTI, or claim network acceptance. Add compile-time contract checks. The native
implementation can satisfy the interface without preserving the Windows class.

Capture the actual viewport width before constructor helper acquisitions, with
an explicit metadata constructor for native/ownership callers; the default still
reads the real Display. ProcessAnimateWindowSpiral accepts that captured width,
preserving the original integer-half radius. This is not a fake Display or renderer.
Exercise real eight-helper constructor failure/retry under all3 compilers if its
actual vtable/provider graph now links; otherwise retain the precise diagnostic
and required pending ownership fixture, never accept it from model-only checks.

Diplomacy checkpoint from55f23984: all configured targets build all3. Related
96/96 PASS normal18.40s/GCCsan81.12s/Clangsan72.95s with intact leak checks;
briefing7 families pass within those runs. Complete repeated owner failure/retry
terminals append6/clear2/attach2 pass all3. Actual eight-helper animation constructor
terminal8 passes normal/GCCsan, but Clang's missing native closure prevents its
fixture link. Retain this pending EXCLUDE_FROM_ALL fixture without CTest or
cross-compiler ownership acceptance; registered windows and actual nonnull GUI
remain pending. Root link-only diagnostics fail1/1/7; never executed. Full229
not run. Canonical581 SHA2708266f4b85f6f4b064553838f6ea48bd8bf46842c84970246f8b4ed4ffb944;
toolchain588 SHAb4387b4ab456b68ea44f14e1dfe6704bcac10d44dfd5a2add340862984f621b4.
See `evidence/qa/N2-diplomacy-checkpoint.md` and the durable formats/source graph.
User requested sorting/committing this cohort; ignored builds/caches, retail link
and recovery archive remain untouched. All 18 original pending paths are owned
by the cohort, not unrelated user changes. N2/slice03 remains incomplete.
Continue native capability/IME/warning/terrain/hosting, then actual factories,
startup/world/compiler-matching scenario. No synthetic providers or vendor edits.

Resume froma549a581 with one coupled native-presentation boundary batch. Preserve
the source skybox GlobalData mutation in a common provider and route terrain
oversizing through a required TerrainVisual virtual operation, not a W3D free
symbol or success stub. Original W3D device forwarding stays behavior-equivalent;
future native/headless factories must implement their explicit visual contract.
Move IME acquisition into GameClient's required device factory so a headless
device can explicitly omit optional input, while an interactive device supplies
real text/IME behavior. Do not create a fake IME object to satisfy RTTI.

Implement the actual OS warning boundary through public SDL3 message-box APIs:
localized text is captured into owned UTF-8, supported buttons/results are checked,
backend failure is OSDBT_ERROR rather than invented confirmation. Test a captured
synchronous presenter as request observation, not physical GUI acceptance. Reject
invalid/missing prerequisites before callbacks. Preserve source audio cancellation,
but stop its obsolete CD retry loop also on unavailable/error output with a fixed
redacted diagnostic; never claim missing music became available. Remove obsolete
GameSpy-only /host diagnostic dependencies from the in-game chat source; the
excluded online menus remain excluded, LAN/game chat is not removed.

Test actual shared state and generated real text ownership, every warning-owner
allocation failure/retry and complete repeated retirement, plus compile-time
factory contracts. Run normal/GCC/Clang focused and related checks on frozen source.
Native dialog display, nonnull TerrainVisual and IME event transport remain N4/
native-factory gates, not claims from observers. The absent historical benchmark
implementation remains a separately documented capability/LOD admission task;
do not synthesize scores, modern CPU classes or GPU acceptance to clear its root.
Keep N2/slice03 incomplete and use the existing plan/evidence/state transaction.

Native-presentation checkpoint froma549a581: all configured targets build all3;
focused5 PASS1.79s/3.38s/3.16s and related100/100 PASS20.37s/84.56s/75.68s with
intact sanitizer/leak checks. Actual warning/catalog allocation terminal6 and
all failure/retry pairs pass three complete owners all3; whole memory-manager
destruction retires warmed registry capacity exactly. Actual skybox GlobalData
state and typed input/terrain contracts pass. Physical dialogs/localized button
labels, nonnull terrain/IME/GUI remain pending; the English dialog buttons do
not establish locale parity. Pending root/animation link diagnostics expose only
native capability admission all3, never executed. Full232 not run. Canonical591
SHA34365252fb700503f4c0f27b628dca7b109e4743afcea6cf0ca1bb5018827282;
toolchain598 SHA347362305fbaa19f115d0d0361510ea2081f72fa4fd0a1a0af4b75f1096d3b23.
See `evidence/qa/N2-native-boundaries-checkpoint.md`. Continue the coupled hardware
metadata/LOD/absent historical benchmark/protected report task, then real factories
and actual original startup/world/compiler-matching scenario. Do not fabricate
legacy scores or renderer capability. N2/slice03 remains incomplete.

Capability follow-on investigation from9b9eff8e must preserve more than graphics:
SlowDeathBehavior activation/update consult dynamic SlowDeathScale and may retire
original objects faster when they have no non-LOD effects. isReallyLowMHz also
affects source audio limits. Do not arbitrarily force a profile or change dynamic
LOD thresholds/counters to bypass hardware admission. The old CPUDetect decoder
does not prove a modern Intel is P4 simply from vendor/x86-64; keep unknown states
honest. Native RAM must use a checked wide internal quantity rather than unsigned
physical bytes cast into signed32. Inspect all LOD/member/format consumers and
the absent benchmark's original score-unit contract before selecting the native
admission policy. Keep source file/INI representations and original simulation
behavior. The required owner test must exercise real GameLOD selection/admission,
not only a stand-alone metadata query, with failure/retry and captured services.
These verified facts are recorded in `docs/original-engine-formats.md`; no new
capability policy or production change is authorized by an unresolved score guess.

Resolve the native capability bridge in the same transaction, preserving original
LOD tables/score comparison and dynamic gameplay rules, not synthesizing the
missing RunBenchmark implementation. Capture a synchronous hardware/calibration/
chipset probe and protected user storage in actual GameLODManager. Default native
hardware reports checked POSIX physical RAM in wide bytes; era-specific CPU
classification/frequency, calibration and renderer classification remain explicitly
unknown when not measured. Unknown frequency must not be misreported as a slow
CPU. Missing calibration/chipset yields no justified automatic recommendation;
retain existing GlobalData/user-selected quality and do not force LOW, manufacture
TNT2/P4 identities, or persist a false IdealStaticGameLOD. Explicit forced legacy
calibration without equivalent score units fails with a fixed actionable error.
The supplied profile formats remain unchanged. A valid equivalent-score provider
still runs original selection rules; synthetic generated providers test those
rules, not native performance acceptance. N4 must supply physical/native quality
and input/renderer evidence; this bridge alone does not accept automatic tuning.

Stage original table parsing, hardware admission and optional calibrated metadata
in a complete GameLOD candidate, with legitimate borrowed singleton publication
restored before commit. Restrict its file-loading entry point to the five original
LOD blocks; reject invalid preset domains/counts/nonpositive divisors, malformed
values and unsafe indices before storage access. Correct initial uninitialized
particle-priority backing from source default LOWEST data; retain all original
dynamic masks/scales and their non-LOD-effect guards. Do not change death behavior.
Protect optional valid benchmark reports through captured atomic user storage;
no CWD or asset writes. Report failure cannot claim missing calibration succeeded.
Actual presentation callback rollback remains a separate nonnull-device gate.

Run actual GameLOD init/selection/retry against generated original INI and original
GlobalData/FileSystem/Options owners. Cover hardware >2GB, memory/CPU boundaries,
missing/failed probes, unavailable/invalid/valid calibration, manual settings,
source dynamic masks/scale/priorities, preset limits, protected reports, every
allocation/storage failure and corrected retry across complete lifetimes. Native
physical metadata is checked without printing values/roots. Freeze then run
focused/related normal/GCCsan/Clangsan builds/tests; link diagnostics are never
startup acceptance. Once the actual animation fixture links all3, execute its
existing complete constructor fault coverage without weakening instrumentation.
Own only actual LOD/probe, build/tests, original format/source findings and existing
plan/QA/state. Continue native factories/startup/world after this coherent gate.

LOD checkpoint from6640b868: native hardware/calibration/storage capture and actual
candidate LOD initialization are implemented. Focused6 PASS all3 in5.46s/22.32s/
14.06s; all configured builds and related106 PASS all3 in15.06s/62.20s/50.37s.
Generated source tables validate retained quality/threshold/scale/mask behavior;
native probe reports no fabricated legacy equivalence. Complete init/report
allocation and report-I/O failure/retry sweeps pass repeated owners and whole pool
retirement. See `evidence/qa/N2-lod-checkpoint.md` for identities, earlier failures
and honest limits. Full238 was not run. Recommendation-specific allocation/I/O
sweeps, nonnull callbacks, animation ownership and actual factories/startup/world
remain pending. This checkpoint commits the coherent verified pending batch;
N2/slice03 remains in progress, not accepted.

Follow-on from900b561d continues this same plan: close actual recommendation
allocation and storage failure/retry coverage, including unavailable/failed/invalid
chipset probes and legitimate parent restoration. Discover each manifest with
the same retained preferences and initialized source owner used by every ordinal;
retire discovery before baselines. Required read/acquisition failure keeps the
prior owner/settings; optional publication failure may retain a valid measured
recommendation with explicit unavailable persistence, followed by corrected retry.
Never manufacture modern hardware equivalence. Recheck actual root and animation
link closure all3; execute only the real animation owner, never the link probe.
If actual constructor ownership passes all3, register that unchanged fixture in
CTest, keeping registered-window behavior and full startup explicitly pending.
Own LOD tests/CMake and directly required source/plan/evidence/state only. Focused
and related frozen normal/GCCsan/Clangsan checks precede a coherent checkpoint.

Follow-on gate passes all3: focused10 in7.77s/33.17s/20.36s; related110 in17.84s/
74.94s/57.64s; all configured builds. Recommendation allocation/storage manifest
prefixes/retries and exact terminals preserve original owner/settings; actual
animation constructor terminal8 passes and is registered in CTest. Root link
closure succeeds all3, never executed. Separate default GUI lexicon still needs205
callbacks all3; headless GUI tables may be explicitly omitted, never gameplay or
logical Drawable state. Current evidence and source identities are appended to
`evidence/qa/N2-lod-checkpoint.md`. Full242 not run; N2/slice03 still incomplete.
Continue native root/client factories and actual original startup/world cohort.

Native client integration from0a44f4be is one coupled source-owner task, not a
replacement plan or fake renderer. Add an explicit headless construction mode to
actual GameClient and a concrete native headless client factory. Physical device
factories must never execute in that mode; unsupported physical calls reject.
Retain actual drawable IDs/list/lookup, frame ticks, ghost/fog updates, script pause
and source drawable updates. Isolate the existing drawable-update loop so both
interactive/headless entry points use it. Per-client repeated-frame state must
not survive owner replacement as a function static. Retain real RayEffectSystem
ownership and reject update before required original simulation owners exist.
No toy GameLogic, synthesized module registry, camera/movie success or discarded
logical drawables. Nonnull output and camera/model-dependent behavior stay explicit
pending gates, not accepted from headless tests.

The source constructor census also exposes early Drawable publication before ID/
ambient pointers are initialized, nonterminated partial module arrays and fallible
caption/module construction without rollback. Resolve initialization, partial
owner retirement and client registry withdrawal together before admitting actual
drawables. Lookup growth must start from nonzero capacity and reject duplicate or
invalid IDs without dropping existing entries. Constructor failure must preserve
legitimate parent identity and accepted owners; callback/device side effects are
not inferred reversible. First generated fixtures exercise actual empty source
templates, not replacement modules, and preserve exact original construction and
update behavior. Extend through actual source modules and native root startup
within this same slice before N2 acceptance. Focused owner/failure/retry tests and
GCC/Clang sanitizers are required; do not compile/test/fix individual assert leaves.
Own actual client/Drawable/native boundary, associated tests/build, source findings
and existing plan/state/evidence. Retail assets/dependencies remain untouched.

User-requested worktree checkpoint from0a44f4be: actual headless-client/Drawable
ownership cohort passes all configured builds and related114/114 all3 in21.58s/
89.73s/66.51s. The retained-source inventory rejected the new adapter's initial
placement; moving it beside the other native adapters preserves that source-lock
contract, and the focused retry plus frozen related matrix pass. Complete empty-
template constructor failure/retry, exact terminals, reset rejection/retry and
owner-local source fade regression are covered. Durable findings and current
source hashes: `evidence/qa/N2-native-client-checkpoint.md`. This checkpoint is the
commit titled `delivery: verify native headless client ownership`; it is supporting
progress, not full slice03 or N2 acceptance. Full246 not run. Continue actual
native GameEngine startup, nonempty module/callback ownership and matching world/
simulation before completion; physical output remains a separate later gate.

Continuation from38375d10: close actual registered nonempty module integration
before wiring startup factories. Audit both draw/CU arrays and all source creator
return paths together: unknown providers return nullptr, which must reject the
candidate rather than silently terminating an array with acquired suffix owners.
Validate null data/provider admission before dereference, retain zero-terminated
partial backing, and keep actual parent identity through retirement. Exercise
the complete original ModuleFactory registration (no shortened registry), real
AnimatedParticleSysBoneClientUpdate and BeaconClientUpdate construction/callback/
retirement, a late unknown-provider rejection and corrected same-owner retry.
Discover complete allocation manifests and prove every failure/retry pair plus
exact terminal; split independent registry sweeps if runtime warrants without
losing ordinal coverage. Only actual empty draw-module tables are used until a
native render provider exists; no mock draw module or gameplay replacement.
Source GameLogic's camera freeze and startup's music readiness are legitimate
dependencies, not null-device success gates; real camera/audio integration remains
required before full startup acceptance. Own existing native-client fixture,
Drawable source checks and directly coupled build/evidence/state. Focused then
related GCC/both sanitizer validation on frozen source; no asset/library writes.

The nonempty Beacon action reaches shared hidden/selectable deselection, which
unconditionally called the absent UI owner. Audit both callers and source
InGameUI::deselectDrawable together. A shared boundary must keep the original
interactive delegation, clear actual Drawable selected state in explicitly
headless mode, and reject an unexpectedly absent interactive UI. No invented UI
list/callback. Retain the normal module visibility/shadow propagation loops;
verify selected-to-hidden and selected-to-unselectable transitions. Caption,
camera physics, script presentation and object-bound behavior remain distinct
pending paths, not silently accepted from this action test.

Native-root factory gate from verified shipped declarations: do not instantiate
common GameLogic and assume its default terrain/ghost providers match the game.
Win32GameEngine selects W3DGameLogic, overriding those factories; common ground
height is a zero stub. Port the original logical height/layer/cliff/extents/
line-of-sight and ghost-owner semantics outside the renderer, preserving original
map dimensions/boundaries and waypoint load order. Nonflat generated terrain and
source-matching samples must gate the actual world fixture. Likewise preserve
camera timing/freeze state, original audio definition/music readiness and actual
particle/radar logical owners while omitting only explicit physical output.
These are locally implementable source-foundation work within slice03, not a
replacement architecture, fake-provider permission or new milestone packet.

Registered nonempty-module cohort passes all configured builds all3; focused10
in13.80s/56.21s/36.42s and related136 in33.04s/134.64s/96.61s. Actual complete
common registry198 and discovered1036-operation failure/retry manifests execute;
four shards retain every pair and exact terminal. Three actual client module pool
prefixes and terminal3 retire exactly; hidden/selectable source actions preserve
logical state without an invented UI. Evidence/current hashes are appended to
`evidence/qa/N2-native-client-checkpoint.md`. Commit checkpoint title:
`delivery: integrate original registered client modules`. Native shipped terrain/
ghost and camera/audio/startup/world gates remain pending; full252 not run.
N2/slice03 stays incomplete; continue the factory gate above within this plan.

Continuation from b984fff1 (clean worktree): extract the shipped height-data
reader and ground/normal/LOS queries into a game-owned NativeTerrainHeightMap,
using the actual DataChunkInput transaction and original grid constants. This
is a supporting integration owner within this same slice, not an alternate
world loader or flat terrain provider. Expose a height-chunk reader for the
eventual complete world candidate; a height-only stream entry point must be
named explicitly and never claim world/object/script load acceptance. Stage
complete backing before replacing accepted state, reject invalid wire lengths,
dimensions/borders/boundaries and duplicate/missing height chunks, and retain
all failure/retry pairs plus exact acquisition terminals. No globals or retail
assets are modified by this owner.

Preserve both source version-1 height-chunk interpretations explicitly:
ParseSizeOnly halves dimensions after resampling, whereas ParseHeightMapData
keeps dimensions and resamples only the prefix. The coupled BlendTileData v1
reader subsequently halves dimensions and updates dataSize; this is a staged
format transition, not an established engine bug. Do not silently normalize
the height chunk before that reader. Name the full-reader intermediate purpose
HeightChunkBacking, not RuntimeGrid; version1 world-query admission remains
pending the complete blend transition. The current writer emits version4.
Runtime queries preserve the original 0–2
triangle diagonal, unusual normal stencil (X3 equals X1), clipping from an
explicit display grid, and endpoint-exclusive Bresenham LOS with 0.5 tolerance.
Guard nonfinite/out-of-range coordinates before integer conversion and use
wide intermediate traversal arithmetic. Same-cell LOS retains the source true
result without its unused division by zero. Query tests require nonplanar
generated data and independent source-derived golden values, both orientations,
edges, distinct logical/display owners, and repeated replacement.

Cliff flags are authored BlendTileData v7/v8, not inferred from modern heights;
v7 uses the original undersized row stride. Full BlendTileData/WorldInfo/objects/
triggers/sides, terrain layers/bridges, native TerrainLogic/root and ghost fog
ownership remain required follow-on integration, not accepted by this owner.
Own the new game header/source, generated terrain fixture/build wiring, durable
format findings and existing plan/state/evidence. Focused functional/negative/
allocation cohorts then related data/map/source-boundary matrix on frozen GCC
and both sanitizer configurations; no repeated full-suite/assert loop.

The initial extracted terrain cohort passed all configured builds, focused3 and related145
in all three configurations. Complete height acquisition census5 retained all
failure/retry pairs and exact terminal; nonflat source triangle/normal/LOS goldens,
distinct display clipping and malformed wire/query inputs pass repeated owner
retirement. Evidence and current source hashes:
`evidence/qa/N2-native-terrain-checkpoint.md`. Commit checkpoint title:
`delivery: extract original terrain height queries`. Full255 not run; complete
world/blend/cliff/layer/ghost/root startup still gates this slice and N2. Continue
the coupled original world integration above; do not infer v1 world completion
from its height-chunk intermediate or substitute a height-only world loader.

Final envelope/backing audit before this checkpoint's commit: a chunk's declared
remaining size is not proof its physical bytes exist. Do not eagerly resize
boundary/height arrays from a forged-but-internally-consistent envelope. Acquire
boundary pairs after their reads and grow height backing after bounded byte-block
reads; preserve the candidate transaction and exact failure/retry coverage across
both small and multi-block shapes. Add forged large boundary and height envelopes
without allocating their declared payload in the fixture. Re-freeze the coherent
change and validate focused plus related matrix; replace the pre-audit source
identity/evidence with the resulting verified current cohort before committing.

Final bounded-acquisition cohort: all configured builds PASS all3; focused3
PASS in0.10s/0.79s/0.53s and related145 PASS in66.64s/273.36s/195.33s.
Actual small/multi-block censuses6/10 retain every failure/distinct-data retry
and exact terminal; failed candidates preserve accepted backing identity.
Forged large-envelope physical truncation rejects without eager arrays. Current
canonical599/toolchain-inclusive606 identities are recorded in the terrain QA
checkpoint. This supersedes the preliminary census5/source snapshot above.
N2/slice03 remains incomplete; proceed with full original world/blend integration
and native root. No asset, dependency or product-authority edits occurred.

Continuation from8740ec65, clean worktree: extend the existing terrain owner
through complete HeightMapData/BlendTileData topology, not just the authored bit
prefix. Retain all fixed-width tile/blend/extra/cliff indices, texture and edge
class names/ranges, packed blend flags/custom edge/marker, and cliff UV records.
Apply source index repair, three-way option filtering, old slope-derived flags,
v7 undersized authored rows, v8 full rows and the coupled v1 blend transition.
Ground/normal/LOS and cliff queries then operate on the same actual accepted
terrain candidate. Explicit loadTerrainTopology is not a full world/visual loader:
lighting, world dictionary, objects, sides, triggers, assets and native root remain
required; no discarded gameplay chunk or texture readiness success is inferred.

Read candidate records before growing backing; decode Short arrays explicitly
little-endian, guard declared extents and do not introduce arbitrary legacy map
caps. Absent old-version extra/cliff index arrays are defined zero sentinels,
not the original uninitialized arrays later inspected by its repair postpass.
Never preserve undefined uninitialized reads as a compatibility contract.
Keep accepted owner/backing through any late field/marker/UV/allocation failure,
then publish a distinct corrected candidate without global callbacks. Legacy
cliff derivation and row stride must follow the source's pre-resize ordering.
Document those limits; do not silently resample authored flags differently.

Own existing native terrain header/source/fixture, CTest wiring and existing
plan/findings/QA/state. Positive cases cover all blend versions, nonflat and
authored-flat disagreement, legacy shape/stride, flags/names/UV preservation,
three-way filtering and repaired bad indices. Negative cases include complete
truncation sweeps, missing/duplicate/reordered chunks, bad lengths/counts/markers/
UVs and hostile queries. Discover small/multi-block manifests, retain every
failure/distinct-candidate retry and exact terminal under both sanitizers, and
run focused then related matrix on frozen source before the supporting commit.
N2/slice03 is still incomplete; this is not a replacement milestone or plan.

The complete topology cohort passes all configured builds, focused6 and related59
in all three configurations. Small/multi-block manifests67/201 retain all fault/
distinct-candidate retry pairs and exact terminals; original authored/derived
cliffs, all8 blend versions, v1 transition, v7 stride, complete metadata/UVs and
packed flags pass. Exact evidence/current identities are appended to
`evidence/qa/N2-native-terrain-checkpoint.md`. Commit checkpoint title:
`delivery: integrate original terrain blend topology`. Full258 not run.
N2/slice03 remains incomplete: continue actual full world/TerrainLogic and
objects/dictionary/sides/triggers/waypoint/ghost/partition integration plus native
root. Texture/lighting and audiovisual acceptance are not inferred from metadata.

Continuation from669d65f4 (clean worktree): audit the coupled world publication
before connecting NativeTerrainLogic. Source objects are appended in file order;
WorldInfo writes both the world Dict and weather; SidesList owns build lists,
teams and scripts and validates them after parsing; triggers own a static list
AND next-ID cursor. All must join a whole-world candidate before replacing an
accepted map. Original side/script parsing and actual waypoint height-provider
order remain required; do not copy archived parsers or replace them with mocks.

First coherent ownership prerequisite is the actual PolygonTrigger lifecycle,
not one assert fix: stage all four chunk versions, list and ID cursor together;
guard every constructor/record/point acquisition; retain accepted identity on
malformed or allocation failure; consume physical point records before growing
backing. Expose a game-owned nonallocating owner exchange so the later combined
world transaction can stage and retire this exact original owner. Preserve file
order, version defaults, water/river/layer fields, discarded short polygons and
the original v1 water rectangle/ID policy. No gameplay oracle is inferred from
common terrain stubs. Audit edit/query/snapshot paths for the same capacity and
overflow hazards; preserve defined source geometry and serialized field order.

Own PolygonTrigger header/source, new actual-source fixture and CTest wiring,
this plan, source findings, QA and delivery state. Exercise all versions and
writer round trips, real nonempty polygons, borrowed publication exchange,
point growth/insert/remove, truncations/hostile counts/IDs, late failures,
constructor/growth and full parse allocation manifests with every failure/retry
pair and exact terminal. Run GCC plus both sanitizers with all leak checks, then
affected map/terrain/chunk/core/source suites on frozen source. This support
checkpoint cannot complete slice03/N2: sides/scripts/object/world-weather and
full TerrainLogic/root integration still follow in this same governing plan.

Snapshot audit refinement before freezing: internal array capacity is not a file
format limit (the old parser reserved numPoints+1). Preserve serialized variable
point counts without depending on the new geometric growth strategy. Read each
physical snapshot point before staging it; after all fields succeed, acquire any
larger backing and publish it atomically. A forged large count with a short file
must fail after actual reads, not eagerly allocate from the count. Validate
growth/shrink/empty loads and complete snapshot failure/retry manifests too.

Final actual-trigger cohort: all configured builds, focused5 and affected
related77 PASS all3 on canonical601/toolchain-inclusive608 frozen source.
Version1/2/3/4 small/large allocation manifests15/25,14/24,14/24,20/30 and
snapshot6 retain every failure/distinct-candidate retry and exact terminal;
three complete lifetimes retire original pools/publications exactly. Current
evidence: `evidence/qa/N2-original-world-checkpoint.md`. Full263 not run.
Commit checkpoint: `delivery: integrate transactional original polygon worlds`.
Continue this same slice with the coupled actual SidesList/BuildListInfo/
TeamsInfoRec and ScriptList/ScriptGroup/script-condition/action ownership paths,
then the enclosing object/Dict/weather/namespace/world transaction and actual
waypoint/render-ground-provider ordering. Durable source findings specify those
dependencies; do not infer startup or gameplay acceptance from this checkpoint.

Continuation from8d705d9d, clean worktree: complete the coupled actual sides and
script admission/clone owners before enclosing a world candidate. Keep the
existing plan/slice and source gameplay conversions; do not replace script
templates or simulation with a fixture. Stage source side/build/team arrays,
ScriptList read publication and generated group-ID cursor, and the name namespace
together. All parser false/throw/late-field and allocation paths must retire
candidate trees and preserve accepted side/script identity. Provide nonallocating
content exchange for enclosing world transactions. Fix copy/constructor/setter
ownership as part of this same batch, not as independently accepted leaf fixes.

Actual ScriptEngine templates are required for conditions/actions. Validate raw
file numbers before converting non-fixed enums; modern name keys can rematch a
changed numeric ordinal as source does. Retain source false/no-op fallback,
parameter arity repairs and every old-format conversion, releasing overwritten
or discarded parameters exactly once. Guard Parameter physical reads, Script/
Group/Or/Condition/Action parse/duplicate/qualified duplicate trees, and typed
constructors before helper acquisitions. Capacity MAX_PARMS and MAX_PLAYER_COUNT
are actual owner array limits, not invented data caps. Do not silently shorten
the registry or skip nonempty scripts. Default group names consume a global ID;
rollback must restore it even if a constructor's later formatting fails.

Own SidesList/Scripts headers and sources, actual-source side/script fixture,
CTest wiring, current plan/findings/QA/state. Positive cases must exercise all
side versions, build metadata/defaults, neutral/default-team repair, real nested
groups/conditions/actions/false actions, name-key rematching, legacy repairs,
writer round trips, independent deep and qualified copies and repeated owners.
Negative cases include every physical prefix, bad counts/types/versions/arity,
late nested failure and constructor/setter/copy rejection. Discover complete
parse/copy/qualified-copy/constructor/setter manifests, retain every failure/
distinct-candidate retry and exact terminal, then validate focused and affected
matrix under GCC and both sanitizers on frozen source. Keep the full source
world/weather/object/waypoint/layers/ghost/root acceptance gates pending.

User-requested worktree housekeeping checkpoint: the side/script batch is
preserved as WIP, not accepted delivery. Normal GCC builds; focused5 completes
with functional/negative PASS and copies/faults/setters FAIL. Current findings,
unverified census causes and remaining coverage are recorded in
`evidence/qa/N2-sides-scripts-wip.md`. Resume this coherent ownership batch before
claiming sanitizer, broader-matrix or slice acceptance; prior trigger acceptance
is not evidence for the modified side/script source.

Resume from69ec4282: diagnose all three fault-family invariants as a coherent
batch. Standard-allocation census may legitimately be zero for warmed explicit
pools; prove its exact zero terminal instead of fabricating a heap acquisition.
Retain count-only diagnostics for mismatched manifests and compare cold/warm
candidate paths before changing production behavior. Add explicit pooled-owner
residual/boundary evidence so a standard-allocation sweep is never presented as
every pooled acquisition. All planned format and lifecycle coverage remains
required before this batch's acceptance.

Source-verified manifest instability: successful ScriptGroup constructors advance
the global ID; their formatted default names eventually exceed std::string's
small-buffer capacity in AsciiString::ensureUniqueBufferOfSize. Keep an outer
fixture IDTransaction around each measured/retried operation so its starting
state is identical, while asserting that real successful admission advances
the two group IDs before fixture restoration. Production ID behavior is unchanged.
Exercise every source legacy action repair and all seven surface-condition
repairs together, with typed order/defaults, raw-number/name rematching and
false/no-op retirement. Check original pool usage as well as standard heap units.
Parameter's empty KIND_OF_PARAM string path indexes the source kind-name table
using the file integer; reject out-of-range indices before access, retaining
valid original conversions. Include direct original Parameter reader/writer
cases, long object-prefix migration and upgrade/status/kind-name conversions.

Final side/script supporting checkpoint: all configured builds, focused9 and
affected96 PASS GCC/GCCsan/Clangsan on canonical605/toolchain-inclusive612 frozen
source. Four complete side parse manifests295/619/291/615; all-format repair634,
qualified clone30, copy/assignment1, long Parameter8 and legitimate warmed zero
terminals retain every failure/retry pair. Eight explicit source pool exhaustion/
retry cases supplement heap sweeps; all families repeat three source lifetimes.
Evidence: `evidence/qa/N2-sides-scripts-checkpoint.md`; full272 not run.
This resolves the69ec4282 WIP faults without altering production sequencing to
fit fixture counts. Commit checkpoint: `delivery: verify transactional original
sides and scripts`. N2/slice03 remains incomplete.

Continue enclosing actual world objects/Dict/weather/name namespace and accepted
side/script/trigger/terrain owners in one candidate publication before replacing
an accepted world. Source root trace now resolves waypoint phases: initial nodes
use the currently published visual logical-height provider; the original later
TerrainLogic::newMap re-snaps every waypoint after visual load (GameLogic1631),
after partition init/ghost reset. Preserve those source phases instead of moving
the first waypoint load ahead of/behind client preparation speculatively. Detailed
source references and remaining limits are in docs/original-engine-formats.md.

Carry forward one explicit coverage limit: warmed typed constructor/setter heap
terminals are zero, while the current pooled exhaustion cohort covers parsing.
Add partial pooled Parameter prefixes for typed constructors/setters in the
enclosing runtime ownership batch; do not present the zero heap census as that
missing evidence or mark slice03/N2 complete before it is supplied.

Continuation from3e36918a: partial typed Parameter pool prefixes now pass focused
pool-failures under GCC and both sanitizers, with immediate failure/retry pairs
and terminals3/2 for both factories/setters. The extension is fixture-only and
remains in this same slice. At the user's repository-housekeeping request, save
it as a verified support checkpoint, not a new plan or independent acceptance of
startup. Include it in the next whole-world batch's frozen matrix. Evidence appendix:
`evidence/qa/N2-sides-scripts-checkpoint.md`.

Next coherent implementation batch: extract the actual logical-world entry
callbacks from the device-bound WorldHeightMap into game-owned native source,
retaining actual MapObject/ThingFactory/Dict, side/script/trigger owners and
accepted height readers. Stage the map object list and world Dict, writable
weather, source side contents, pending reads/group cursor, trigger list/cursor,
height metadata and actual namespace together. Begin namespace ownership before
DataChunkInput TOC admission, not merely inside the side callback. Retire failed
candidate worlds while legitimate singleton identity remains published, then
restore borrowed state without allocations/callbacks. Publish complete candidate
content only after all physical chunk/field reads and source validation succeed.

Inspect original map writers/defaults and logical-only versus visual load
semantics before choosing mandatory/optional chunk and version policies; do not
silently impose new format caps or skip nonempty objects/scripts. Preserve object
file order, old z/default/discard rules, actual template binding, waypoint/light/
scorch classification, source weather semantics, and metadata versus visual
height-grid purpose. Test successful/rejected/retried full nonempty worlds and
late failures after each owner, complete allocation manifests, namespace rollback,
borrowed publication identity, source writer encodings and repeated lifetimes.
WorldInfo/ObjectsList/SidesList/PolygonTriggers/height metadata form one behavior
batch. Waypoint initial/final height phases, layers/ghost/partition and native
GameEngine startup remain required afterward; no headless gameplay substitutes.

Whole-world implementation refinement (parent3ef97998): original emptySides and
emptyTeams clear both ordinary and skirmish ownership. Logical source loading
preserves the prior world Dict/weather if WorldInfo is absent, but empties
objects/triggers/sides and validates the resulting neutral side. Preserve these
optional defaults; require usable height metadata and reject duplicate owner
chunks rather than permit source list overwrite/leaks. Unknown visual chunks
remain skipped by the logical path. ObjectsList/WorldInfo are schema-neutral
containers; Object versions1–3 select fields. Read complete records before z
discard, retain v1's empty Dict (not constructor defaults), and use actual
ThingFactory lookup. Reject nonfinite physical coordinates/angles and weather
integers outside the source Weather domain before enum construction.

Add a game-owned NativeLogicalWorld entry and nonallocating MapObject WorldState,
pending-script retirement and height swap. Its loader stages all source globals
under legitimate candidate SidesList publication; explicit cleanup precedes
borrowed pointer restoration. The loader owns height metadata, not arbitrary
borrowed process-global world lifetimes. Preserve normal normalizeAngle arithmetic;
repair only nonfinite/nonprogressing loops using bounded remainder reduction.
Generated full-world positive/rejection/retry tests must use actual owners and
DataChunkOutput, then extend complete fault/pooled/lifetime evidence across GCC
and both sanitizer configurations before accepting this batch.

Final source-call audit: the logical WorldHeightMap constructor calls emptySides,
not clear, so omitting SidesList retains previous ordinary/skirmish team records
through validateSides. The original SidesList callback itself begins with clear
for EVERY version, so a present v1 chunk does not inherit prior teams. Preserve
the omission-only carryover by strongly copying both team records into the
candidate before final validateSides; do not copy side/skirmish-side contents.
Add explicit omitted-side preservation/owner-repair and allocation-failure tests.
This is a coupled logical-world compatibility correction from direct source
review, not an authority change or a newly introduced slice.

Native terrain/provider continuation remains this same slice03. Enclose the
logical world transaction in an actual terrain/root candidate before later
waypoint/visual/provider work can fail. Extract W3DTerrainLogic extent/boundary/
minimum/maximum and ground/LOS/cliff/layer semantics through game-owned CPU
providers and actual AI pathfinder/Bridge logic. Temporary logical metadata is
not the retained full visual-owned simulation grid. Preserve initial absent/prior
ground queries, source waypoint prepend and duplicate-ID/link selection, then
visual load and final newMap snap. Query mode must not invoke visual loading.
Stage filename, dimensions/boundaries, waypoint list/links and borrowed provider
publication; retire failure before restoring legitimate singleton identities.
Guard the eight-link accessor and physical link count/payload together with
constructor/retirement, without changing valid graph admission/capacity policy.
Source findings: original-engine-formats.md Combined native logical-world owner
and Waypoint source cohort. Couple generated map/provider failure/retry and
absent/prior/replacement height phases with real layers/bridge/pathfinder and
repeated lifetimes. Do not promote basic TerrainLogic's default ground/layer/LOS
stubs or a mock provider to gameplay acceptance. Actual ghost/partition/root,
camera/audio/particle/radar and compiler-matching simulation remain required.

Combined logical-world batch verified: final canonical608
`b10129e0d32eb1a9b04064951305a23c8d422030413985f5e421077113286a01`,
inclusive615 `c089e91fb87e608103c21b98f577fe070113ba8839803c678bec62d1200056e9`.
Full configured builds and affected101 PASS GCC/GCC sanitizer/Clang sanitizer;
authored-side manifest150 and omitted-side manifest75 retain every failure/retry
and exact terminal; ten source pool exhaustion paths and borrowed pending/fresh
namespace behavior repeat across three whole owners. Evidence:
`evidence/qa/N2-logical-world-checkpoint.md`. Commit boundary is this complete
logical-world behavior, not slice03/N2 completion; configured277 full suite has
not run. Continue the native terrain/provider/root batch documented above.

Provider inspection afterf300fca5: TerrainVisual's pure methods include CPU water
grid state/queries and raw terrain mutation consumed by actual TerrainLogic.
Do not instantiate a universal no-op headless visual to satisfy the interface.
Extract source water/height/transform gameplay state along with terrain queries;
defer only presentation output. Trace original DO_SEISMIC_SIMULATIONS build
configuration before enabling/disabling source conditional behavior. Details and
source links are durable in original-engine-formats.md's provider continuation.

Water/terrain provider implementation refinement (parentf300fca5; prior dirty
plan/docs are owned carry-forward): extract complete CPU WaterRenderObjClass
grid into NativeWaterGrid, retaining source vertex/border layout, nearest/truncate
queries, attenuation box, motion/gravity/damping/rest thresholds, angle versus
matrix transform behavior and source version1 scalar snapshot sequence. Use
actual GlobalData profile0 and actual Xfer/Snapshot dispatch. Keep no renderer
or simulation callbacks in the grid; caller passes logic frame/gravity.
Initialize previously undefined constructor fields; repair the source Y-only
resolution typo and keep per-owner frame state rather than a function-static
cursor. Preserve preferredHeight's unsigned-byte representation, rejecting
nonfinite/unrepresentable float-to-byte input rather than introducing UB.
Stage enable/resize/restore backing; reject impossible dimensions, nonfinite
arithmetic inputs and singular attenuation before mutating accepted state.
All snapshot cells remain float,float,byte,byte (no raw native struct padding).
Restore derives motion from loaded status so resumed waves do not freeze.
Native terrain visual integration, dynamic terrain mutation, lighting/material,
waypoint/layer/root and actual simulation remain this same slice's next work.
Positive/negative source-derived motion/rotation/clamp/query/snapshot tests,
complete enable/resize/load allocation manifests and repeated whole-owner
retirement run under GCC and both unsuppressed sanitizers before this coherent
provider batch commits. No extra plan or standalone helper-only slice is created.

Profile default audit: GlobalData initializes every vertex-water profile to zero.
Preserve disabled/inert zero resolution/spacing/factors during init; require
usable positive dimensions/spacing only on enable/backed query, and validate
actual attenuation denominators before a height mutation rather than forbid all
zero profiles. Two-pass arithmetic admission preserves strong nonallocating
height/velocity/frame updates. Source matrix-only transform updates only the
presentation matrix, not the world-query origin/axes; preserve that distinction.

Worktree consolidation requested by the user: transaction parent
`f300fca5df1fe3763bfd547fbaa2969e52f9e854`; all six pending paths belong to
this coherent provider batch, with no unrelated edits or generated files to
discard. Commit boundary: `delivery: preserve original CPU water behavior`.
Include implementation, five fixture families, build wiring, verified format
notes, QA evidence and current delivery checkpoint together. Exact validation
and frozen source identity: `evidence/qa/N2-native-water-checkpoint.md`.
This checkpoint does not complete slice03/N2; continue the native terrain
provider/waypoint/root integration, without importing historical implementation.

Retained terrain-content continuation (parent029730a6; clean transaction start):
extend NativeTerrainHeightMap with the full source height/blend/GlobalLighting
admission path consumed by TerrainVisual. Parse lighting versions1–3 into an
offside copy of actual GlobalData lighting/time fields; retain omitted lighting
and shadow color, initialize absent old-version extra lights to source zero
ambient/diffuse and downward direction, and retain an optional authored shadow
color without a renderer callback. Commit map and scalar configuration only
after the complete stream succeeds. Never reparse logical world owners here.
Reject invalid versions, enum values, duplicates, truncated/trailing fields, and
late failures without changing accepted map/configuration. Preserve source
height/blend legacy resampling and optional lighting order. Add source terrain-
name lookup (floor, border, clipping, packed tile divide by4, first class match)
and construction's lower-only raw-height mutation with bounded coordinates and
defined byte admission; do not regenerate authored cliff flags on deformation.
OversizeTerrain is already a no-op in the original BaseHeightMap; don't invent
CPU resizing. Color still requires actual decoded source tile mip1 backing and
is not proven by class metadata. This is the same provider/root batch and slice,
not a new helper plan. Test all lighting versions, omitted/optional shadow state,
late rollback/each corrected retry, exact allocation terminal and repeated owners
under all3 configurations before a coherent checkpoint. Full N2/root remains
pending; attach only current, actually executed evidence.

Retained CPU terrain checkpoint: all3 configured builds and related72 pass on
canonical611 source79af2341 / inclusive618 source5483f992. Three new visual
families cover lighting1–3 × blend1–8, source tile-name selection and deformation,
all incomplete/late malformed prefixes and complete allocation prefixes[0,61)
with terminal61 and every corrected retry across three whole owners. Evidence:
`evidence/qa/N2-visual-terrain-checkpoint.md`. Commit boundary is
`delivery: admit retained terrain and lighting atomically`, not N2/slice completion.
Full configured285 is not run. Native TerrainVisual integration must use this
retained full grid and the accepted water CPU owner; an outer admission scope
must keep original logical world publication rollback live through waypoint,
provider and later root failure, rather than call NativeLogicalWorld::load and
assume an already committed logical parse can unwind a later terrain failure.

Waypoint/world continuation refinement (parent253c2e46; clean start): keep the
actual NativeLogicalWorld publication, name and script-ID transactions open
through a synchronous CPU admission continuation after side validation and before
any accepted-world swap. The caller stages its own terrain/root candidate; the
world owner restores objects/Dict/weather/triggers/sides/script publication and
names/IDs if that continuation fails. No render/audio-thread callbacks or generic
simulation substitutes. Standalone load delegates to the same admission path.
Extract the original Waypoint constructor and graph operations into a game-owned
NativeWaypointGraph and reuse them from actual TerrainLogic add/link/delete/read
methods, not a second incompatible graph. Retain original pooled nodes, reverse
file traversal, first-file duplicate-ID choice, duplicate/self/missing suppression,
source-node-only bidirectionality and capacity8 including independently admitted
reverse edges after forward saturation. Fix getLink index8 bounds and physically
validate counts/trailing records. Stage all nodes/links, preserve prior/absent
initial height and apply later nonallocating atomic resnap using the retained
terrain grid. Provide explicit head exchange for subsequent actual TerrainLogic
publication. Test source graphs, malformed/late replacement and every corrected
retry, standard allocation census and actual Waypoint pool exhaustion, repeated
owners and outer-world continuation failure across all3 configurations. This
continues the existing provider/root cohort; no new helper slice or plan. Provider
binding and original startup/simulation remain the next integration boundary.

World/waypoint checkpoint: frozen all3 configured builds and related77 pass on
canonical614 sourcecbbeff2d / inclusive621 sourceb2f07b54. Actual pooled graph
and shared TerrainLogic operations retain source graph semantics. Standard graph
prefixes[0,4), actual Waypoint-pool exhaustion and world-plus-waypoint prefixes
[0,175) each retain every corrected retry/exact terminal across three owners.
Later continuation failure also restores fresh world namespace, accepted parent
side identity and source height/ID/publication state. Evidence:
`evidence/qa/N2-waypoint-world-continuation.md`. Commit boundary:
`delivery: retain world rollback through original waypoint admission`.
Full configured290 is not run; this does not complete slice03/N2 or bind the
native terrain providers/root. Continue actual TerrainVisual/TerrainLogic using
the existing staged terrain/water/graph owners and synchronous world scope.

Native interface binding refinement (parent8b3cef21, clean worktree): introduce
NativeTerrainVisual as the actual TerrainVisual CPU adapter, shared by later
headless and graphical providers. Bind virtual water/profile/frame methods to
the retained NativeWaterGrid and raw deformation/tile selection to retained
NativeTerrainHeightMap plus actual TerrainTypeCollection. Keep presentation,
image-backed color, picking and complete device snapshot responsibilities
explicitly abstract; do not claim output readiness or silently serialize only
the CPU prefix of the original version3 snapshot. A noexcept dirty-height hook
allows a device to defer uploads without changing accepted gameplay mutation.
Stage filenames before map admission; use actual CachedFileInputStream for the
filename entry and a stream entry for the enclosing candidate. Preserve prior
terrain through reset, source water reset/frame behavior, absent-map defaults,
and nonallocating whole CPU-state exchange for outer rollback. Validate actual
base-interface dispatch, restrictions, deformation and water/frame behavior,
missing prerequisites, malformed replacement, every allocation failure/retry
and repeated owners under GCC and both sanitizers. This is existing slice03
integration, not a new plan, and does not replace the required original root,
TerrainLogic/layer/ghost/partition or whole simulation gates.

Native CPU virtual binding checkpoint: all3 configured builds, focused3 and
related80 PASS on canonical616 source2dabfa05 / inclusive623 source4b9433ec.
Actual TerrainVisual dispatch preserves retained height/water, construction type
restrictions, reset/state exchange and source frame0 publication. Provider
prefixes[0,61) retain every failure/retry and exact terminal61 across three
owners; virtual water enable/Y-resize retain terminal1 failure/retry pairs.
Image color/picking/presentation and complete device snapshot remain explicitly
abstract, not no-op production acceptance. Evidence:
`evidence/qa/N2-terrain-provider-checkpoint.md`. Commit boundary:
`delivery: bind original terrain CPU virtual interface`.
Full configured293 has not run; slice03/N2 remains incomplete. Continue actual
TerrainLogic queries/layers/Bridge/pathfinder and enclosing world/provider/root
candidate, then partition/ghost/startup and compiler-matching original simulation.
Source bridge loading derives BridgeInfo from original model geometry and
registers gameplay bridges: that dependency must survive headless execution.

TerrainLogic integration refinement (parent26da2238, clean start): implement the
actual TerrainLogic-derived native CPU map provider using NativeLogicalWorld's
synchronous transaction. Stage source logical dimensions/boundaries/min/max,
initial original graph using prior visual ground, then retained full terrain and
lighting; publish candidate CPU fields only within rollback guards. Keep world,
names/IDs, metadata, graph, retained terrain/name and lighting live through an
explicit synchronous late admission hook. Query mode skips retained visual load.
Reuse actual AI/pathfinder wall queries and actual Bridge layer/height methods,
not all-layer ground substitutions. Device-map admission and newMap's source
road/bridge stage remain mandatory abstract device responsibilities, so this CPU
base cannot accidentally become a complete ground-only runtime. Snapshot wrapper
retains original native logic version1 plus actual TerrainLogic transfer.
Test combined nonempty original worlds/terrain, query mode, absent/prior/new
height phases, source extents/cached extrema, late provider rollback/retry and
complete allocation manifests across three owners/all3 configurations. Keep
bridge geometry, nonempty wall/bridge acceptance and actual root/simulation
pending; no new slice/plan or milestone-complete claim.

Native TerrainLogic combined admission checkpoint: all3 configured builds,
focused3 and related83 PASS on canonical620 source26e0eb24 / inclusive627
sourceaf4054cb. Original nonempty world, actual graph and retained terrain/light
state now share late rollback through the CPU provider. Prefixes[0,259), exact
terminal259 and every corrected retry retire standard and source pool units
across three owners. Distinct authored/active light rows and shadow restore on
late failure; query mode preserves prior visual backing and source phase order.
Evidence: `evidence/qa/N2-native-terrain-logic-checkpoint.md`. Commit boundary:
`delivery: integrate original terrain and world admission`.
Full296 is not run; device/road-bridge hooks remain abstract, not accepted no-ops.
Continue one coupled actual AI/pathfinder/bridge/root ownership batch: source
Pathfinder resets an uninitialized backing pointer and acquires shared cell info
before fallible work; shared array release uses scalar delete and forcibly
retires prior live ownership. AI acquires TAiData before fallible Pathfinder;
Bridge caches a factory-owned template across restarts and creates registered
objects before fallible helpers. Resolve allocation/publication/retirement
together, preserving actual wall/layer and model-derived BridgeInfo semantics,
then bind concrete native root/partition/ghost/startup/simulation. Verified source
anchors are durable in original-engine-formats.md. Slice03/N2 remains incomplete.

AI/pathfinder ownership implementation refinement (parent7176f853, clean start):
guard actual TAiData/Pathfinder constructor acquisitions, initialize source reset
inputs before use, and reset all owned cells/layers/zones before shared-info
release. Shared cell-info backing is acquired per owner without evicting another
owner's live cells; acquired info units retain backing until both owner and cell
counts reach zero. Pair array ownership correctly and reject duplicate/foreign
info retirement before mutation. Batch AI group acquisition/ID rollback and
whole-owner group cleanup with strong override side/build-list cloning so later
fallible helpers cannot leave stale registered ownership. Preserve valid source
group sequencing, data defaults and override list order. Exercise real AI and
Pathfinder constructors, concurrent owners, all30000 info units/exhaustion/reuse,
last-owner/live-cell retirement, repeated reset/destruction, nonempty override
and group failure/retry using original pooled nodes. Bind actual wall-height and
clipped empty-wall queries to native terrain in the existing generated-world
fixture. Validate complete heap manifests and actual typed pool exhaustion under
all3 configurations. Full map/layer/zone allocation, nonempty wall/bridge model
geometry and concrete root admission remain the next coupled integration work;
do not infer those from this ownership checkpoint or create another plan.

Ownership fixture review also found AISideInfo's five optional skill sets are
uninitialized before whole-node override copying. Initialize each to an empty
defined representation (zero active skills); populated parsed skill sets remain
unchanged. The generated fixture retains source side-list prepend reversal and
BuildListInfo append order, checks nonempty independent build clones and empty
build lists, and exercises each actual typed pool separately from heap manifests.

Actual AI/pathfinder ownership checkpoint: all3 full configured builds, focused5
and related87 PASS on canonical622 source6f61efe1 / inclusive629 source5e82472b.
The real concurrent owner/cell-info lifetime, source capacity30000, actual cell
reset, override clone and group constructor/registration/reset/destruction paths
now retire exactly. Standard-allocation terminals are coldAI3, warmAI2,
override1 and group1; every failure has a corrected retry. Actual side/build/
build-node/group typed-pool exhaustion has separate failure/retry proof. Native
TerrainLogic's wall query is now tested with actual AI/pathfinder wall height and
clipped empty-wall behavior. Evidence:
`evidence/qa/N2-ai-pathfinder-ownership-checkpoint.md`. Commit boundary:
`delivery: preserve original AI and pathfinder ownership`.
Full300 not run; slice03/N2 incomplete. Continue coupled classified map/zone/layer
admission with source two-pass reuse, nonempty walls/bridge model-derived bounds
and registered-object rollback, then concrete root/partition/ghost/startup and
compiler-matching original simulation. Original source allocation hazards for
the next batch are durable in original-engine-formats.md, not accepted behavior.

Classified-map backing refinement (parentb1f32299, clean start): batch both
ZoneBlock's four arrays and PathfindZoneManager's six arrays, zone-block backing
and index table, bridge/wall layer backing and index table, and the ground grid.
Acquire each complete backing group before publishing its capacity/pointers;
retain previous complete backing on individual acquisition failure. Validate
dimension products, finite geometry conversion and zone capacity before unsafe
arithmetic. Initialize layer identity before isUnused, and treat a wall-ID set
with no surviving objects as empty rather than consuming uninitialized bounds.
For actual newMap, preserve source same-size two-pass ground backing reuse and
source terrain/footprint/zone ordering. An allocation/classification failure
retires incomplete grid backing and withdraws readiness/open/closed pointers,
while preserving borrowed bridge descriptors and wall registrations so corrected
same-process admission can rebuild. This is candidate-owner recovery, not a
claim that a previously accepted root can remain published after its grid fails;
the outer root candidate/rollback transaction remains a mandatory later gate.
Exercise actual generated terrain through the published original AI/GameLogic
and NativeTerrainLogic/Visual providers, initial and repeated map admission,
source-classified mixed zone blocks/tables and exact allocation failure/retry
manifests under all3 configurations. Keep nonempty model-derived bridge geometry,
registered wall/object footprints and concrete root acceptance pending until
their actual source participants execute; do not substitute isolated cells for
whole-game or nonempty layer evidence. No new slice or replacement plan.

Packed-zone review refinement: PathfindCell retains a 14-bit zone attribute even
though zoneStorageType and equivalence arrays use 16-bit values. Derive the
admissible cell/layer IDs from that source packing, reject overflow before
assignment and preserve the packing. Source UNINITIALIZED_ZONE zero is not a
valid completed ZoneBlock input. Use representable 14-bit values and zero in
the malformed fixture; the earlier 65535 setter value was silently truncated by
the bitfield, invalidating that test oracle. Cover source classification overflow
and direct setter/layer rejection together rather than widening source fields
or allowing malformed data to corrupt zone equivalence indexing.

Classified-map backing checkpoint: all3 full configured builds, focused8 and
related91 PASS on canonical622 source653415f3 / inclusive629 source8864c904.
Actual generated terrain runs source Pathfinder newMap classification and zones
under legitimate AI/GameLogic/native terrain publications; same-size second pass
retains ground cell identity with exact zero standard allocations. Complete heap
manifests cover layer backing2, block growth4, whole mixed-zone owner46 and actual
map admission14, with all prefixes and corrected retries. Layer/zone growth
preserves accepted backing; failed map candidate admission retires incomplete
grid state and directly retries without reset hiding partial acquisition. Packed
zone limits and dimensions reject before truncation/overflow. Evidence:
`evidence/qa/N2-classified-map-backing-checkpoint.md`. Commit boundary:
`delivery: stage original classified map backing`.
Full304 not run; slice03/N2 incomplete. Next execute nonempty registered wall/
object footprint and model-derived bridge participants, bridge/tower construction
rollback and concrete root candidate publication, followed by partition/ghost/
startup and compiler-matching original simulation. Backing-helper evidence does
not substitute for those source participants or whole-game acceptance.

Actor prerequisite refinement (parent7737aea1, clean start): actual Object
construction requires a controlling neutral player/default team, rank data,
module factory, UI/radar, partition and script services. Player currently acquires
two pooled relation maps before initializing its raw owned fields, then creates
eleven squads and runs fallible rank/name/relation initialization. PlayerList
acquires the full MAX_PLAYER_COUNT cohort without an unwind guard and its
destructor calls allocating init before deletion. Batch constructor rollback,
complete pooled squad replacement and non-allocating whole player/cohort
retirement, including ownership already retired by explicit Player::init but
omitted from its destructor. Preserve source neutral defaults, rank/science
callbacks, local-player selection, build-list ordering and explicit reset
semantics. Missing rank authority rejects before dereference/acquisition, rather
than silently inventing rank data. Exercise real players/cohorts and source
selection/relationship/build-list behavior, every standard failure/retry prefix,
separate actual relation-map/squad pool exhaustion, repeated reset/destruction
and whole memory-manager retirement under all3 configurations. This prerequisite
is part of the existing actor/root integration, not a replacement milestone or
acceptance of empty actors. Team construction/publication, Object lookup and
modules, model-derived bridge geometry, concrete root/startup and simulation
remain pending. Record the source findings durably; do not create another plan.

Real player/cohort checkpoint: all3 configured builds, focused4 and related118
PASS on canonical624 sourceb2574cec / inclusive631 sourcebb07b9f2. Actual neutral
players/cohorts, nonempty two-node builds/production changes/selection, explicit
reset and non-allocating destruction retire exactly. Warmed heap manifests are
player4/cohort81/reset-whole-owner4/production-registration1, all prefixes and
corrected retries; actual relation-map/squad exhaustion and all pooled squad
replacement prefixes[0,11)/terminal11 separately pass. Evidence:
`evidence/qa/N2-player-ownership-checkpoint.md`. Commit boundary:
`delivery: preserve original player cohort ownership`.
Full308 not run; actual team/default-team and nonempty rank/science/optional
owners are not established by this fixture. Next batch guards TeamFactory/
TeamPrototype/Team relation-map and dual registration ownership/cursors together,
preserving source clear phase order without a fallible map copy; then actual
Object lookup/ID/modules/creation, local-player partition iterator callbacks,
registered wall/model-derived bridge/root admission and original simulation.
Source GameEngine initializes partition before the cohort; actual callback
publication order remains mandatory and is durable in engine-format notes.
Slice03/N2 remains incomplete; no new plan or upstream library changes.

Actual-team integration refinement (parent872cdd97, clean start): preserve source
neutral default-team naming, singleton reuse, activation and prototype/member
order. Batch factory clear's whole-map retirement, prototype factory/player dual
registration, Team's paired pooled relation maps and intrusive membership, and
both factory ID cursors across constructor/late creation failures. Keep source
clear semantics (withdraw the entire factory map before callbacks) via a map swap,
not a fallible copy. Reject missing definitions/parents, duplicate prototypes and
ID exhaustion before unsafe publication; retain fixed-width serialized IDs.
Exercise actual factory/prototype/team with real player/name/rank/script parents,
neutral setDefaultTeam, alliance/relationship cleanup and singleton versus
multi-instance behavior. Prove every heap failure/retry and typed-pool exhaustion,
accepted participant identity and corrected ID advancement, three complete owner
lifetimes and allocation-free clear/destruction under all3 configurations. Script
creation actions remain source callbacks; any gameplay side effects require the
later whole candidate-root rollback, not an invented per-team simulation rewind.
Actual Object/partition/wall/bridge and whole startup remain pending gates.

Team fixture baseline correction: the first normal focused4 run passed3 and
failed functional whole-factory heap equality. Player::setTeamRelationship uses
its persistent unordered_map; removing the entry retires the node but legitimately
retains bucket capacity in the still-live player owner. Warm the actual nonempty
relationship/clear/default-team sequence before repeated-factory baselines, then
still require whole player/context/memory-manager destruction to retire that
capacity exactly. No production workaround or weakened lifetime oracle.

Actual default-team ownership checkpoint: all3 full configured builds, focused4
and related122 PASS on canonical625 source21b32f18 / inclusive632 source0da9af4a.
Real source neutral default-team activation, singleton reuse, non-singleton
active/inactive instances, relationship notification and intrusive retirement
execute. Prototype/member39 and existing-prototype inactive5/active5 heap
manifests preserve accepted participants and prove cursor rollback/advance before
each corrected retry. Four actual typed pools exhaust/retry separately. Source
whole-factory clear is allocation-free; whole player/context/manager ownership
retires warmed relationship capacity exactly. Evidence:
`evidence/qa/N2-team-ownership-checkpoint.md`. Commit boundary:
`delivery: preserve original team admission ownership`.
Full312 not run; actual actors/root and authored script creation actions remain
pending. Next couple Object constructor/module/team/radar/early lookup ownership
with GameLogic scheduler registration and destruction-queue admission (including
the fixed256 cleanup assumption), plus legitimate UI/partition/script parents.
Execute actual registered wall/object footprints, model-derived bridge/root
rollback, then complete partition/ghost/startup and compiler-matching simulation.
Slice03/N2 incomplete; no new plan or upstream modifications.

Registered-actor/root refinement (parenta16d24fc, clean start): next outcome is
actual registered actors and nonempty pathfinder footprints under legitimate
CPU root participants, not another isolated backing acceptance. Inspect and
batch Object's five sightings, initialized/terminated module backing and source
helpers, team/early ID lookup/radar/public creation, GameLogic scheduler and
destruction queue as one admission/retirement chain. Preserve callback/destruction
order, fixed-width IDs and packed frame phases; remove zero-size lookup growth
and cursor overflow hazards before unsafe operations. Never invoke complete
Object destructor notifications on partial construction. Constructor and late
ThingFactory creation failure must withdraw every acquired registration before
guarded module/resource retirement and permit corrected retry. Whole-root
candidate rollback still encloses script/gameplay side effects and borrowed
global publication. Bind actual UI logical callbacks, player/default-team,
rank/script, module factory, radar, original partition, native terrain/visual and
AI/GameLogic participants; inspect borrowed control-bar ownership and local-player
partition iterators before construction. Common ghost manager's empty methods
are not accepted ghost behavior. Execute real actor/module cardinality, failure/
retry, repeated teardown, wall/footprint classification and nonempty lifecycle;
then integrate model-derived bridge/ghost/full GameEngine startup and actual
GameLogic::update compiler-matching checkpoints. Headless draw/audio omission is
permitted; replacing simulation or logical callbacks is not. Required source
anchors, queue/scheduler/UI/ghost hazards and acceptance limits are durable in
original-engine-formats.md. All3 ownership matrices remain mandatory; full suite
only when slice/milestone completion is actually established.

Working-tree housekeeping checkpoint: the player and team ownership batches
are committed as 872cdd97 and a16d24fc respectively. The source-review notes
and this refinement are documentation only. The remaining working changes in
GameLogic.h and GameLogic.cpp are an unfinished registered-actor batch: lookup
admission, scheduler withdrawal and destruction-queue staging. They have not
been compiled or tested and are not accepted behavior. Finish the coupled
Object constructor/ID/module/radar rollback and actual actor fixtures before
freezing source for the required validation matrix; review registration
preconditions before publication and scheduler retirement cardinality. Do not
commit these helpers as a completed independent delivery slice. N2 and slice03
remain in progress; the last verified runtime checkpoint is the team evidence.
No non-ignored untracked files were present during this housekeeping check.
Ignored build outputs, user asset links and recovery storage are retained.

Resume parent90f99df8: the two pre-existing dirty GameLogic files belong to this
same unfinished actor batch and are preserved. Couple the Object partial-owner
cleanup and strong ID replacement with actual original helper/module ownership,
source team/energy callbacks and queue retirement. Use real constructor-level
UI, radar, partition, scripts, rank, players/teams and AI parents for ownership
fixtures; presentation-only subclasses may omit drawing, never logical
callbacks. InGameUI allocates placement backing before subsequent fallible
string construction; move that acquisition to the final constructor phase and
reject invalid capacity first. Radar's acquired node needs a guard through
fallible apparent-controller helpers. Exercise many actual stock updates above
256 and survivor scheduling, duplicate registration, ID collision/growth,
destruction queue allocation failure and all constructor failure/retry prefixes.
This does not replace the required native full-root/startup, nonempty partition,
wall/bridge/ghost and actual GameLogic::update evidence.

First actor validation retained: initial GCC build rejected fixture-only enum
conversion and protected setID calls. Correct the fixture with explicit ObjectID
construction and an actually constructed, same-sized InspectObject subtype that
only exposes that protected operation; never downcast a base actor or change
production visibility to bypass the contract. The corrected build passed, but
all9 actor families stopped in shared many-module warm-up before individual
sweeps. Redacted stack identified rebalanceChildSleepyUpdate forming
&m_sleepyUpdates[319] for a159 parent after heap shrink. Review the full heap
family before retry: both child-loop versions form/compute children before
bounds, the optimized version also uses operator[] at size(), debug validation
multiplies signed indices, and remakeSleepyUpdate calls child rebalance on an
empty heap. Replace only the unsafe representation with bounded size_t indices,
retaining source comparison/tie/swap/index-update semantics. Exercise many
stock DeletionUpdate wake changes both earlier and later, then full and survivor
retirement. No full build/test-after-single-assert loop and no coverage waiver.

Second focused retry retained: scheduler warm-up proceeds, but every family
fails during fixture whole-context teardown. The redacted stack identifies
Radar::deleteListResources unconditionally consulting TheGameLogic after the
fixture's automatic reverse-member destruction retired it. This is a fixture
parent-order defect, not permission to null-guard away source callbacks. Review
UI/radar/partition/team/player/AI/script/template/module cleanup together; use
explicit two-phase actor then parent retirement with GameLogic kept alive
through radar cleanup, and the same order on constructor failure. Borrowed
globals are restored only after their actual acquired owner retires. Production
radar's source callback remains intact.

Parent-graph retry retained: normal/GCCsan/Clangsan all reach UI retirement and
report the same stopCameoMovie call into absent GameWindowManager. Reviewing
both movie acquire/stop pairs and all UI destructor helpers confirms the idle
constructor owns neither a cameo stream nor a buffer, but the original stop
still looks up and clears a GUI window. Empty cameo retirement should be an
ownership no-op, not an invented window-manager mock or swallowed callback for
an acquired movie. Return only when both owned pointers are absent; retain the
source window unbinding and media retirement when either exists. Actual media
acquisition/unbinding faults and view/control-bar startup remain pending, not
accepted from constructor-only headless UI. No new full build or acceptance
claim follows this focused root-parent correction alone.

Eight focused families now pass all3. Remaining ID/cold/scheduler families have
fixture-contract failures, not sanitizer errors. Redacted pool counters prove
nested real cohorts grow Squad capacity256 ->384 while used176 returns unchanged;
warm the actual two-root lifetime before baselines and retain exact whole-memory-
manager retirement. Entry-point template lookup constructs AsciiString: the
short generated name adds one standard acquisition, the long name adds two.
Thus cold array/lookup/scheduler demand3+1 and late array/scheduler demand2+2
both have exact terminal4; do not omit caller allocations from the manifest.

Fidelity correction before acceptance: direct behavior-array-order heap erasure
would change survivor layout for equal priorities relative to the source's
reverse heap-discovery order. Preserve the original 256-entry stack batch and
its reverse erasure order, but repeat discovery until no owned entry remains.
Ordinary <=256-module retirement retains the original sequence exactly; larger
cardinality completes formerly truncated/unsafe ownership. Stack batching is a
bounded workspace, not a cardinality cap. Existing many-module wake/survivor,
exhaustion and complete-retirement evidence still applies after frozen rerun.

Registered-actor ownership checkpoint verified on canonical626
fc86107a3c7b7a4b15bdb20eb25f5c563024bde45fa941390a07e345c20b3dc5 and
toolchain-inclusive633
ff6cfcd98fdf25a4eeb72a18e266018b91c45d5eef40d4ae774e972d9f596813:
all3 full configured builds, focused11 and related133 PASS. Related elapsed
times are GCC116.58s, GCCsan504.84s and Clangsan318.88s. Exact warm/cold/late
scheduler/UI/ID manifests2/4/4/10/1, eight typed actor pool exhaustion/retry
pairs and every stock update acquisition prefix through terminal270 are
retained. See evidence/qa/N2-actor-ownership-checkpoint.md for all failure/retry
history and scope limits. Full323 not run; this is a coherent supporting
ownership checkpoint, not completed slice03 or N2. Continue with nonempty
partition/ghost, full ThingFactory creation and candidate-root/startup/actual
simulation under legitimate parents. Commit boundary remains
`delivery: preserve original registered actor ownership`.

Next coupled partition lifecycle batch (parent ce047060): review grid init/reset,
radius backing, actor and orphan-ghost attachment, COI geometry sizing, dirty
lists and master-list withdrawal together before implementation. Grid admission
must validate finite source extents/cell size and signed index/product limits,
retain valid source minimum-size behavior, and retire all partially initialized
cells/radius backing on failure so the same owner can retry. Attachment failure
must withdraw acquired ghost/dirty/master-list ownership without invoking normal
fog-retention logic for a candidate that never became accepted. Preserve normal
orphan/snapshot semantics and source publication order. Both object and ghost
paths need the same acquisition guard; COI sizing must preserve the actual source
shape calculations while rejecting undefined conversion/multiplication inputs.
Start executable evidence with the existing actual NativeTerrainLogic generated
world: nonempty grid, repeated reset/init, every cold allocation prefix/terminal
and corrected retry on the same PartitionManager. Then integrate real actors,
dirty-cell contacts and shipped ghost semantics under legitimate parents. Empty
common GhostObjectManager is never fog acceptance. Focused normal/GCC/Clang
sanitizer tests precede a frozen related matrix; do not declare the whole batch
complete from grid-only tests or create a replacement slice plan.

Partition batch preliminary evidence: grid2 passes normal/GCCsan/Clangsan on
actual generated NativeTerrainLogic, including complete cold allocation demand
and every same-owner retry. These runs precede actor integration and are not a
frozen complete-batch acceptance. The first actor fixture build needs the full
Player declaration (not only PlayerList's forward declaration); its inadvertently
started CTest used the old executable and rejects the new family, so neither
that invocation nor the failed compilation is runtime defect evidence. Add the
required source header, rebuild successfully, then run the new family. Retain
both logs; subsequent builds/tests must be conditioned on successful compilation.

Actual actor attachment failure reached the intended allocation prefix but
cleanup stopped in removeAllTouchedCells: both source COI acquisition paths
publish count before new[] backing exists. Review all assignments to count,
backing and used count together; publish the array count only after new[]
completes, on actor and ghost paths. This couples the missing rollback
representation with the planned list/ghost guards, rather than masking the
destructor traversal. Retain the failed run and redacted stack; focused retry
follows the whole-family correction, not a broad acceptance rerun.

Nonempty partition ownership checkpoint verified on canonical626
f0dcee42be5fa88b88ba5c45b9ae6369ece3bd706f627cbe938e7463f9ae90ec,
inclusive633 88679126a0a6e2a3b38753556d8e66955e45980108cb4e42543b4c84946c650d:
all3 full configured builds, focused4 and related137 PASS. Related elapsed
GCC125.09s/GCCsan557.03s/Clangsan337.72s; configured327, full327 not run.
Actual generated NativeTerrainLogic supplies nonempty cells/radius backing;
real actors exercise touched-cell callbacks, survivor preservation, every COI
failure/retry, all three shapes, malformed candidate geometry and actual fixed
partition pool exhaustion/retry. Grid manifest31, warmed actor1 and parallel
ghost attachment1 have exact terminals and all failure/retry pairs. Ghost
attachment test uses source contract geometry/pointer publication only, with
snapshot calls explicitly rejected; shipped fog/snapshots are NOT accepted.
Evidence: evidence/qa/N2-partition-ownership-checkpoint.md. Supporting commit:
`delivery: preserve original partition admission ownership`. Slice03/N2 remain
incomplete. Continue full ThingFactory creation and legitimate native root/UI/
client/freeze composition, shipped ghost/snapshot/border/load ownership,
accepted-actor geometry replacement, walls/model bridges, full startup and real
compiler-matching simulation. Source findings are durable in
docs/original-engine-formats.md; no new replacement plan or milestone is created.

Next full-creation ownership batch (parent587b1234): actual ThingFactory::newObject
must retain candidate ownership across create interfaces, partition admission,
Drawable creation/binding and initObject. Guard the actor after complete
construction, withdraw its source registrations and retire that candidate on
later failure while legitimate parents remain live. This is candidate resource
ownership, not arbitrary gameplay-side-effect rollback: callback-created children,
scripts, RNG and accepted-world state still require the whole native root journal.
Do not rewind over IDs consumed by other actors. Review Drawable binding callbacks
and reciprocal pointers together; no half-bound candidate may outlive its owner.
Use the real NativeHeadlessGameClient initialized before logical UI publication,
with actual MessageStream, rank/module/script/team/radar/terrain/partition parents,
not temporarily nulled globals or fixture-written simulation. Generated source
definitions should exercise an actual body and create callback, then full
Object/Drawable/partition identity, retirement and every complete late allocation
failure/retry pair preserving an accepted survivor. Execute both compiler
sanitizers; batch surrounding ownership/cleanup paths before broad frozen tests.
The UI/headless update contradiction, camera freeze behavior, authored child/
script/RNG side effects, real fog and actual GameLogic::update remain explicit
integration requirements; creation fixtures cannot accept them by omission.

Binding review refinement before code: Drawable::friend_bindToObject publishes
its actor pointer before draw callbacks, then Object::friend_bindToDrawable
publishes the reciprocal pointer before behavior callbacks. A callback failure
can therefore leave a half-bound pair. Guard the newly created Drawable in
sendObjectCreated and withdraw only matching candidate pointers without callbacks
or allocation on rollback; retire its registry while both real owners live.
Preserve normal callback order and complete destruction. Reject a duplicate
sendObjectCreated before acquiring another Drawable. Scope actor-ID restoration
to an otherwise unconsumed candidate ID; no rewind over child/other acquisitions.

First creation build retained: the owner guard called Object's protected pooled
delete endpoint and passed unsigned storage to the typed ObjectID setter. Use
the existing public friend_deleteInstance owner endpoint after registry withdrawal
and retain ObjectID as the candidate identity; do not change pooled deletion
visibility or serialized types. No creation test was launched after that failed
build. Retry only after the source API corrections, then inspect runtime lifecycle
families rather than introducing fixture-written create behavior.

Worktree review and unfinished handoff (parent
`587b12343a51164c592433c6cb033271eedc9f5a`): the corrected GCC target build
passed, but both `original_logical_world_factory-creation` and
`original_logical_world_factory-creation-faults` failed during fixture setup
with `ERROR_BAD_INI`, before creation ownership was exercised. Context mounts
its initially empty generated tree before factoryCreation writes creation.ini;
NativeFileSystem::mountReadOnly indexes files at mount time. INI::prepFile rejects
the undiscoverable generated file. Correct generated-file preparation/discovery
using the existing mount contract, not a production filesystem bypass. Then
exercise the real creation families and both partial-binding callback-failure
phases before freezing source for the GCC/Clang sanitizer and related matrix.
The callback-failure branches are not yet specifically tested. No current batch
ownership, sanitizer, or milestone acceptance is claimed.

Six executable paths remain deliberately uncommitted as one coupled batch:
Drawable.h and Object.h candidate-binding withdrawal; ThingFactory.cpp candidate
actor ownership; GameLogic.cpp candidate Drawable ownership;
cmake/OriginalRuntime.cmake new test registration; and
tests/original/logical_world.cpp real body/create/binding fixtures. Commit them
together with eventual truthful evidence only after required validation passes.
Do not split header/source/test changes into unrelated commits or commit failing
tests as a completed delivery checkpoint. No untracked files, vendor modifications,
retail modifications, or recovery-archive modifications were found in the tracked
worktree review. Ignored build outputs are not deliverables and were not removed.

Retained local logs (ephemeral; this account is the durable failure handoff):
`/tmp/n2-factory-creation-gcc-build.log` (first compile failure),
`/tmp/n2-factory-creation-gcc-build-retry.log` (corrected build),
`/tmp/n2-factory-creation-gcc-build-current.log` (current build), and
`/tmp/n2-factory-creation-gcc-focused.log` (both setup failures). Previous partition
checkpoint acceptance applies to its committed source, not this unfinished batch.

Creation fixture correction and binding coverage refinement before changes:
refresh the generated-only VFS mount after closing creation.ini. Its index is a
snapshot, not a live directory watch. The first corrected run reaches real
creation and the expected duplicate-notification rejection, then a second warm
call fails because the same factory retains an already-defined template after
its ModuleFactory data owner has retired. Warm two complete fresh roots instead;
do not bypass duplicate-definition validation or retain dangling module data.
Both corrected creation families pass on GCC (6.33s); no sanitizer acceptance yet.

Add a separately named binding-failure family with explicitly test-only source
ModuleFactory registrations. One DrawModule fault callback observes the first
one-way publication and one BehaviorModule fault callback observes the later
reciprocal publication. Inject a typed error at each phase and prove both parent
registries, source IDs, heap/pool ownership and an accepted survivor are preserved;
then retry full real creation. Record callback order and ensure pointer withdrawal
adds no callbacks. These probes test original binding/ownership only: they must reject
physical rendering and must not stand in for shipped modules, visuals or gameplay.
Keep the existing InactiveBody/VeterancyGainCreate allocation families probe-free.

Binding-probe failures retained: first builds required the complete ThingTemplate
header, explicit public constructors after pooled macros and the actual mutable
generated template for ModuleInfo admission. No tests ran after compile failures.
The corrected callback family then exposed a fixture assumption: the original
complete Object destructor calls sendObjectDestroyed even without an accepted
Drawable, and that notifies behaviors of a null binding. Preserve this legitimate
source unbind callback; probes fault acquisition only, record unbind separately,
and require exactly one destructor unbind on candidate failure. Candidate pointer
withdrawal itself adds no callback. Do not suppress source destruction callbacks
or interpret this fixture failure as an ownership implementation defect.

Full creation ownership supporting checkpoint: all3 full configured builds,
focused3 and related140 PASS on canonical626
97bfe4fa32b00f1e6a8629352c2f03ead5471990fa98ff802f3479a878476677 and
inclusive633 3ca742ad1ed229dce339005c9ca2dbc790b052d83b7ee30ccb296163332d095c.
Matching manifests cover every failure prefix [0,11), exact terminal11 and every
corrected creation retry; actual InactiveBody/VeterancyGainCreate/Drawable pool
exhaustion; both original partial-binding phases and legitimate destructor unbind;
three whole-context lifetimes and memory-manager retirement. No test timeout or
sanitizer suppression. Focused3: GCC9.51s/GCCsan38.46s/Clangsan22.06s;
related140: GCC135.26s/GCCsan593.13s/Clangsan359.44s. Full330 not run.
Evidence: evidence/qa/N2-full-creation-ownership-checkpoint.md. Supporting commit:
`delivery: preserve original complete creation ownership`. All six previously
uncommitted executable paths belong to this verified coupled batch, not a WIP
delivery commit. Slice03 and N2 remain incomplete.

Continue native logical UI/client/camera-freeze and whole-root journal integration,
with real original GameEngine/GameLogic init/update. Preserve source callbacks,
script/RNG/child state and legitimate parent identities; no global-null alternation
or constant camera-finished substitute. Shipped fog snapshot/orphan/border/load,
accepted geometry replacement, walls/model bridges and compiler-matching actual
simulation remain mandatory. No replacement plan or milestone is created.

Native-root composition batch planning refinement (parent05d4f891, before new
production changes): the existing link-only original_runtime_link_probe now
builds successfully against the full current original source group. It retains
GameEngine::init; it does not execute initialization or any simulation and is
not a CTest acceptance case. Log: /tmp/n2-native-root-link-census.log. Do not run
it as a game, treat linking as startup acceptance, or replace unresolved runtime
provider behavior with success stubs. The next gap is runtime provider/lifecycle
composition, not another unchanged compilation census.

Keep one coupled root integration outcome inside this existing slice:

- Construct a real native GameEngine/actual GameLogic owner graph through the
  original factory and subsystem transactions. Explicit headless selection omits
  physical output only. Preserve original data/config loading, message propagation,
  script execution, scheduler phases, terrain/AI/partition updates and source CRC
  points. Generated tests must advance actual GameEngine::update/GameLogic::update
  with nonempty source actors/modules, not set frame counters as a simulation.
- Give the client one legitimate logical InGameUI and TacticalView ownership path.
  Existing headless init/update reject any UI/view; do not bypass this by publishing
  then nulling globals. Reject foreign/accepted publications before acquisition.
  Retire objects/drawables while logical UI, camera and their parent identities
  remain valid, then retire acquired services and restore outer borrowed leases.
- Review logical UI init/reset/transfer, superweapon and named-timer acquisition
  together. Current init assumes Display and creates physical controls; reset
  dereferences ControlBar and TacticalView and frees retained timer strings through
  DisplayStringManager. Explicitly separate absent physical output from retained
  bookkeeping, paired string/font ownership and source timer behavior. Do not
  discard timers/superweapon callbacks or fabricate font/render measurements.
- Preserve shipped camera state, not View's no-op motion/default-finished methods.
  Source order is zoom, pitch, then rotate OR waypoint-path; scripted-lock state
  also updates. Motion completion examines all four activity flags, with the six
  source motion-blur exceptions. Rotation/zoom/pitch use source frame quantization;
  waypoint movement uses elapsed milliseconds. Source default frame length is
  1000/LOGICFRAMES_PER_SECOND, but its configured value may change. Camera can
  continue on client updates while logic is frozen. Retain tracking, hold, ease,
  shutter, path arrival (consume-on-query), time multiplier and freeze retirement.
  Use explicit timing input, not an invented always-finished camera or logic ticks
  to advance a camera that is itself freezing logic. Common View transfer remains
  version1, angle and XYZ Reals, without a new serialized camera layout.
- The common198 module registry does not include the shipped W3D draw providers.
  Original W3DModuleFactory extends it with 19 names/data creators. Retail Object
  definitions must remain parseable and source model queries must retain their
  actual CPU meaning even with no GPU output. Weapon launch offsets and Object
  pristine logical bones use Drawable/ObjectDrawInterface, so empty draw tables or
  universal identity transforms are not whole-game parity. Inspect decoder and
  state-selection providers as a cohort; distinguish simulation-required metadata
  from N3 physical submission. Do not skip unknown draw definitions or patch bgfx.

Expected surfaces: GameEngine/GameMain native factories and entry wiring,
GameClient/NativeHeadlessGameClient, InGameUI/View logical lifecycle and native
camera provider, required logical text/font/data services, native GameLogic
terrain/ghost composition and actual module-definition/model-query providers;
root/frame generated fixtures and CMake. Use supported dependency APIs only.

Required evidence: generated full-root init/load/update/retire with source-linked
nonempty actor/module events and compiler-matching checkpoints; frozen/script/
camera/pause and clear-game transitions; malformed/missing roots without writes;
repeated root lifetimes preserving accepted/borrowed identities; complete native
failure/retry manifests and actual typed exhaustion across coupled parents;
GCC/Clang unsuppressed sanitizers, proportional focused tests and related matrix.
Split independent sweeps if needed but retain every failure/retry pair and exact
terminal. Shipped fog/geometry/wall/bridge and real read-only retail evidence remain
mandatory follow-through; no model-only fixture accepts them. Unknown implementation
details are source investigation, not authority to invent or weaken behavior.

Additional paired-path findings before native provider implementation:

- W3DView's waypoint storage pads all other arrays to MAX_WAYPOINTS+2, but
  groundHeight has only MAX_WAYPOINTS+1 while setupWaypointPath writes through
  numWaypoints+1. The maximum source path reaches that out-of-bounds final slot.
  Native camera staging must size/check every padded array together and initialize
  active paths, including empty/near-coincident paths; do not transplant the UB.
  rotateCameraTowardObject sets a target ID without initially caching position;
  first-frame lookup failure must not read an uninitialized/stale union target.
- Preserve the real source completion details: pitch/zoom clear their activity
  but do not clear the freeze flag; rotation/path completion clears freeze.
  MoveCameraTo with one millisecond deliberately keeps a pending path/arrival
  state for a later update. A segment-end arrival is consumed on query and final
  path retirement occurs on the next update. Do not simplify these into a single
  duration check. Sample ground via the source center/four corners at offset40.
- AudioManager::isMusicAlreadyLoaded checks an actual original music event's
  generated VFS file, not hardware availability. Native headless mode cannot
  return fabricated music readiness. AudioManager::update uses TacticalView eye,
  orientation and terrain for microphone placement/zoom volume before the client's
  update in GameEngine's source ordering. This is a coupled camera/audio dependency.
- FontLibrary::getFont owns a pooled GameFont before fallible loadFontData; failure
  cleanup and list admission need pairing. Base FontLibrary destruction calls
  deleteAllFonts, whose releaseFontData virtual dispatch is then the base method.
  A native derived font backend must retire its resources while its own vtable is
  still live and before borrowed display strings lose their font parent. Inspect
  this with SuperweaponInfo's two strings and named-timer replacement, not after
  an isolated font test. No new provider production changes have begun yet.

Shared camera implementation checkpoint (parent3b2f6d78, same native-root batch):
ScriptedView now carries the original W3DView choreography in game-owned shared
code, with concrete projection/camera backing still abstract. Original snapshot
layout stays in View. The four activity modes, easing, tracked actual source
actors/hold/destruction, motion-blur predicate exceptions, explicit path clock,
shutter/interpolation/arrival/instant retirement, padded maximum path and source
disabled-motion rules are exercised. Initial target position and every padded
array are initialized; discarded near-coincident cycles reject before replacing
accepted motion; near-coincident facing does not silently mutate live tracking.
Counter/time-multiplier arithmetic has defined full-width intermediates.

Current frozen canonical630 9fc886d515135df304cda898654a2746e7721275820680926a9be22497bab3fd,
inclusive637 d5f4c929c1937c06df41d07f764d793590b50e4aa7e38f128039fb3d89534630.
All3 full configured builds and focused5 PASS. Camera owner manifests [0,1),
terminal1/every retry and zero-allocation choreography terminal0 match all3.
Repeated actual Waypoint/whole-manager retirement passes. Related145 PASS all3:
GCC140.06s/GCCsan627.19s/Clangsan377.31s. Full335 has not run. Evidence:
evidence/qa/N2-script-camera-checkpoint.md retains initial build/fixture failures,
complete corrected validation and limitations. This is supporting implementation
progress, not a finished slice, native root or N2 acceptance.

Carry-forward source ordering is now pinned in docs/original-engine-formats.md:
sample camera/script/debug/pause freeze and update Drawables before the source
display/view phase advances camera. Script freeze allows camera movement; debug
freeze/game pause do not. GameLogic later sees the resulting camera predicate;
audio samples the earlier eye before client update. Do not reorder motion to
the start of client update. Source scripted lock, concrete native eye/projection,
legitimate logical UI/text/font owner lifecycle and NativeHeadlessGameClient
integration are still required. The client continues rejecting UI/view until the
whole legitimate ownership path exists. No constant-finished or fabricated
spatial adapter is accepted. Continue this existing plan toward actual original
GameEngine/GameLogic startup/frame, source root journal, fog/geometry/walls/bridges
and CPU model-query providers; do not generate replacement tiny plans.

UI/text ownership implementation refinement (parent f26a7b4c; same slice03):
Before native font/string providers are admitted into the root, correct the
coupled source admission/retirement contract. Guard the pooled font through
backend false/throw and list admission, initializing all backing fields. Guard
both SuperweaponInfo strings through constructor failure; acquire a valid font
before assigning either string. Use one guarded superweapon map/list admission
for runtime and snapshot reconstruction, removing only newly inserted empty
entries on list failure. Named-timer replacement must stage its complete candidate
before a nonfallible pointer exchange; insertion failure retains existing entries.
Retire timer/superweapon maps through one helper in reset and destruction, with
the real string manager still published. Retire client-owned strings before fonts
after other string consumers. Preserve source serialized fields and UI timing.

Validate actual source operations with an explicitly test-only font/backing and
display-string adapter that rejects rendering/metric operations; it is not a
native provider or visual oracle. Cover partial backend failure/throw, two-string
constructor stages, new/existing-key admissions, successful replacement/removal,
reset-helper and destruction without reset, all standard-allocation prefixes and
corrected retries, actual pooled units and repeated whole-manager retirement.
Use focused tests while implementing, then frozen related matrix under GCC,
GCC sanitizers and Clang sanitizers. Native FreeType/Fontconfig resources, source
spacing/wrapping/alternate-Unicode metrics, concrete camera/UI/client ownership
and actual root/frame remain mandatory subsequent work inside this same plan.
No mock adapter establishes native or whole-game acceptance.

Shared text/UI ownership checkpoint: the coupled source cohort above is
implemented and verified on canonical633
3d78803c12684934d0e6f1d83b084df388b91775cb332b7cf5d14b155a491ee6,
inclusive640 cf08696bfd511a952b332c62ba6c219d00197ac286554376f1e2e4ed98f44724.
All3 full configured builds/focused8/related153 PASS (related GCC145.51s,
GCCsan647.20s, Clangsan389.52s). Source-client cohort prefixes[0,18), terminal18,
each retry; font1/new-timer1/replacement0/new-power2/existing-power1 allocation
terminals; timer4/two-string8 callback terminals; four actual fixed-capacity pools
including both string-constructor acquisition failures; whole-manager retirement.
Evidence: evidence/qa/N2-text-ownership-checkpoint.md. Retained fixture compile/
configuration-baseline corrections did not change production to satisfy a test.
Full343 and native game text/UI/root/startup/frame/output remain pending.

Stock FreeType2.14.3/Fontconfig2.18.3 public-API preflight PASS under both compiler
sanitizer stacks: actual memory-backed glyph, supplied font-set matching, acquired
aliases and repeated teardown, without global-config replacement/FcFini/library
edits. Durable reproducible control and scope limits are in the same evidence.
Next implement the real native font/string provider and concrete camera/UI/client
ownership, not adopt the test-only adapters. docs/original-engine-formats.md pins
local-font/alternate/Generals-width/96dpi/per-character spacing requirements,
remaining actual snapshot-version/count issues and popup acquisition/side effects.
It also records that actual root reverse registration order retires PlayerList
before Team cleanup needs it; implement dependency-aware actor/team/UI retirement
while legitimate logic/player/script/client parents remain published. Do not infer
safe loaded-root shutdown from an empty registry or fixture-managed teardown.
This continues the original root journal/frame/model/fog/geometry scope; it does
not complete slice03 or add a new milestone or tiny plan.

Native font backing refinement (parent37ae7acf, same slice03/root batch):
Implement a real derived FontLibrary with an explicit, game-owned FreeType
library and Fontconfig configuration/font set. Stage initialization offside;
read LocalFontFile only through original read-only VFS, retain bytes through
face retirement and use public metadata/matching APIs, without host installation,
global-config replacement or FcFini. Preserve requested name/point/bold cache keys,
96dpi source sizing, the Generals->Arial requested-width rule, and primary below256
versus alternate Unicode selection. Native face resources must remain alive until
all pooled GameFont records retire. Failed init permits same-owner retry; font
candidate and glyph-copy failure cannot leak backing or publish partial state.

Expose copied CPU glyph coverage/advance/bearings and real font ascent/height for
logical strings and later rendering. Never return raw borrowed glyph-slot pixels
whose next FreeType call can replace them. Check defined codepoint/size/stride
arithmetic and paired backing; no pair kerning is introduced. Source W's extra
pixel belongs to spacing. Treat exact native metric/raster visual comparison,
full wrapping/hotkey/string manager and interactive rendering as outstanding
integration gates, not proven by a successful glyph. Source-controlled sizing
must not be discarded merely to make a test pass.

Use deterministic generated BDF fonts/configuration to prove real VFS font bytes,
local matching, alternate selection and mono coverage without proprietary
fixtures; separately prove host matching/Generals outline sizing. Exercise source
getFont/list/reset/destructor, null/missing/malformed local data, corrected init
retry, actual pooled exhaustion and repeat whole-owner retirement. Derive exact
standard/native acquisition terminals for each complete operation; native
checkpoints are game-owned post-acquisition observers, never vendor interposition.
Each acquired unit is guarded before a fallible observer. Retain every failure/
retry pair, including zero demand for warmed APIs, under GCC and Clang sanitizers.
Use focused builds/tests during implementation, then frozen related matrix.
Native string/UI/client/root/camera/snapshot/popup/journal integration remains
mandatory follow-through in this same governing plan; no fake adapter is adopted.

Native CPU backing is implemented and focused6 passes GCC/GCC sanitizer/Clang
sanitizer on canonical636, with actual generated glyph coverage and complete
native/standard acquisition pairs and typed pooled exhaustion. Evidence:
evidence/qa/N2-native-font-checkpoint.md. Frozen related159/full configured
builds passed all3; evidence records terminal execution handles and logs.
This is a supporting implementation checkpoint, not slice03 or N2 completion.
Continue actual native string/UI/root behavior after this coherent validation
batch; do not replace the governing plan. Point0 and Generals GDI metric/raster
comparison remain explicit integration limits, not silently settled behavior.

Native string follow-through, same plan/slice: use the durable full string-path
census in docs/original-engine-formats.md. Implement actual original manager
contracts with staged text/font/wrap/hotkey state and owned glyph/layout backing,
normal/bold borrowed fonts and localized group/formation caches. Preserve the
distinction between raw charPos width and formatted extents including source
overlap, >= word/hard wrapping and newline/centering behavior. Do not accept
stubbed width/output or replace that source contract with generic typesetting.
Test missing configuration/fonts/game-text and constructor/cache replacement
failures with exact admission/retirement and corrected retries. Retire caches and
every tracked string before font backing; integrate with actual UI and client
owners, not only a separate font fixture. Physical submission remains explicit
and must reject an absent renderer; N2 headless output cannot claim rendering.
Native-font related159 passed all3: GCC148.46s, GCC sanitizer642.98s, Clang
sanitizer393.10s; full configured builds/focused6 passed all3. All handles are
terminal0. Commit this coherent backing checkpoint, then continue the planned
string/UI/client/root follow-through. Full349 and N2 acceptance remain pending.

String implementation refinement (parent0d56ee9c, same coherent UI/root batch):
NativeDisplayString will stage copied glyphs, raw per-position advances and line
placements before a nonfallible text/font/layout swap. Override every inherited
mutation that otherwise publishes before fallible notification. Expose owned CPU
layout for a later explicit renderer; nonempty draw without that integration
rejects, never reports headless output success. NativeDisplayStringManager guards
default-font admission, keeps localized group/formation cache replacement offside,
and retires all tracked records before font backing. Extend real-font generated
tests with exact raw/formatted widths, overlap, soft/hard >= boundaries, trailing
newlines, ampersands, centered placements, mutation rollback and repeated manager
lifetimes; test cache initialization against original text/DrawGroupInfo parents.
Complete native/client integration and source-output parity are not inferred from
these CPU tests. Use actual UI consumers and coupled admission paths next.

Current implementation progress: real NativeDisplayString and manager compile;
focused8 passes all3 (GCC2.47s/GCC sanitizer7.79s/Clang sanitizer6.97s), with
actual generated CSF/GameText and DrawGroupInfo caches, complete text mutation7
and cache replacement76 standard-fault terminals and every retry, repeated3.
Details/limits are durable in docs/original-engine-formats.md. No live jobs,
implementation commit or broader acceptance for this batch yet. Continue actual
UI/client consumers, native hook failure cross-products and remaining layout
boundaries before freezing the related matrix; do not accept a separate manager
fixture as whole original-root adoption. N2 and slice03 remain incomplete.

Actual client integration ordering: GameClient::init constructs/initializes the
string manager before creating FontLibrary; localized cache postProcessLoad is
later, after fonts. The native manager must support this legitimate unbound
construction and resolve the published native font owner at admission, rather
than dereference a font that does not exist or reorder the original initialization
graph. Implement the original headless client's real font/string factories and
test their source registry composition/destructor with actual timer/superweapon
consumers, missing parents and paired native/standard admission failures. Keep
UI/view/display factories explicitly pending; this does not accept full init.

Current coupled composition evidence: production NativeHeadlessGameClient text
factories and source registry/destructor now exercise real manager/fonts plus
actual generated CSF caches, timer replacement, superweapon constructor/admission
and ordinary glyph-backed strings. The UI fixture rejects physical draw/view;
full source init and public player/science admission remain pending. Whole client
115 standard acquisitions are covered in early/late sweeps with every retry and
exact terminal, repeated3, under all3 focused11 (18.25s/59.73s/45.32s). All jobs
terminal0. Failed-development boundaries and paired source census are recorded
in docs/original-engine-formats.md and AGENTS.md. No implementation commit or
broader acceptance yet. Complete remaining native-hook/layout mutation boundary
coverage and truthful root adoption, then freeze this coherent batch for related
validation; preserve all N2 startup/frame/camera/popup/snapshot gates.

Coupled native text/client batch is now frozen canonical640/inclusive647. Full
configured builds/focused16/related169 run all3; exact jobs/logs and acquisition
manifests are in evidence/qa/N2-native-text-checkpoint.md. Added independent
standard/native sweeps for all seven mutations, native cache11, source-derived
space/marker/wrap/newline/centering/Unicode boundaries and negative manager
parents/default descriptors/foreign free. No full359, implementation commit,
slice03 or N2 acceptance yet. Poll exact live jobs; do not mutate the tested
runtime or restart based on a yield. Commit only after required terminal passes,
then continue original UI/view/root startup in this same governing transaction.

Actual-root follow-through reminder from source census during frozen validation:
initHeadless/updateHeadless still reject UI/view/window/display and do not invoke
the verified text factories or advance a camera. Actual native root adoption must
close that route deliberately with staged legitimate providers and source phase
ordering. Original InGameUI::init has early tactical-view/ControlBar publication
and fallible attachment/init; guard that whole cohort and pending popup/snapshot
paths rather than reuse the test subclass or nullable parents. Durable source
anchors and constraints: docs/original-engine-formats.md. This refines the
existing root/camera/UI scope, not another plan or a relaxed N2 acceptance gate.

Frozen coupled native text/client checkpoint complete: all3 full configured
builds/focused16/related169 PASS canonical640/inclusive647. Related times:
GCC166.19s, GCC sanitizer716.62s, Clang sanitizer434.11s. All validation handles
terminal0. Supporting evidence: evidence/qa/N2-native-text-checkpoint.md.
Commit the coherent provider/actual client factory/ownership/validation batch;
full359, slice03 and N2 acceptance remain pending. Continue the documented actual
headless UI/view/root adoption and original source phase/snapshot/popup/shutdown
gates, not another isolated text-only plan.

Camera/view spatial follow-through (parent10b43a84, same slice): source inspection
found NativeTerrainVisual still rejects intersectTerrain. Implement the real
display-height-mesh segment query before concrete view adoption; it must use the
visual grid, source 0-1-2 / 2-3-0 diagonal and clamped overhang vertices, not the
logical ground-height stencil or water/bridge shortcuts. Preserve nullable result,
unchanged output on a miss, two-sided segment intersections and endpoint exclusion
from the original Fraction=1 / t<Fraction admission. Validate nearest contact
across all candidates: original Cast_Ray's hit || Collide expression short-circuits
after its first hit despite its nearest-hit comment; correcting that original bug
is authorized, not permission to change ordinary terrain behavior. Keep terrain,
bridge and drawable queries separate; the view must compose bridge selection later.
Inspect replacement/lowering/reset, edge/diagonal/sloped/multiple-hit rays, extreme
defined inputs and allocation-free repeated queries together. Exercise the actual
TerrainVisual virtual entry point as well as backing, all3 sanitizer configurations,
and related terrain/world/camera/client families on frozen source before committing
this useful spatial behavior checkpoint. Then continue concrete camera transforms,
source clock ordering and staged original UI/root adoption. No ray fixture is
whole-root or physical-rendering acceptance; existing N2 completion gates remain.

Current displayed-terrain query implementation is frozen canonical640/inclusive647
at f52556b3/473cda14 (full digests in evidence/qa/N2-terrain-ray-checkpoint.md).
GCC full configured build/focused15/related172 PASS (21.55s/168.10s), handle44939
terminal0. Both sanitizer full configured builds/focused15 PASS (GCC112.07s,
Clang68.68s); related172 are live in exact handles22250/48724. No runtime mutation
or implementation commit until required terminal validation passes. Poll rather
than restart; full362, concrete camera/root and N2 acceptance remain pending.

Displayed-terrain query checkpoint now verified all3 full configured builds,
focused15 and related172. Related times168.10s/727.05s/442.76s; all jobs terminal0,
no required failures and no live jobs. Source digests remain identical to the
frozen evidence. Commit this real virtual spatial-query behavior with its tests,
source findings and provenance, then resume concrete camera/view and staged
original UI/headless root adoption in the same slice. This is supporting behavior,
not another slice/plan or whole-root/whole-game acceptance.

Concrete spatial backing refinement (parentce4c7e4b, same camera/UI/root batch):
implement game-owned NativeCamera value backing for the original normal camera
transform, orthonormal eye/right/up/forward basis, source near/far frustum,
absolute-pixel projection/unprojection and Z-plane rays. Build each candidate
offside and preserve the accepted camera on invalid/degenerate input; keep eye
references owner-local, avoid global static eye storage, and make queries
allocation-free. Validate source yaw/pitch rotation order, ground factor/FX pitch,
viewport aspect/origin, near/far versus offscreen classification, truncation and
defined extreme-coordinate arithmetic together. Use the actual ScriptedView
choreography plus generated real visual height maps to prove transformed movement
and ray contacts, not only isolated triangles or constant-eye adapters. Then use
this backing in the concrete native View: source constraints, actor follow/tether,
scroll/height settlement, reachable shake/slave and bridge/model selection plus
the original frame phase remain mandatory, not waived by backing tests. Physical
submission remains N3; no absent model query or rendering request may fake success.
Keep existing script/terrain owners' focused tests during iteration, then the
required all3 sanitizer/related matrix on a coherent frozen integration batch.

Spatial backing is now consumed by production NativeCameraView's real View
eye/projection/plane/terrain/elevated-look-at entry points, not just a fixture's
manual eye substitute. It remains abstract for actual frame/model/constraint/
scroll/shake/slave/output providers so unsupported source behavior cannot inherit
false no-op success. Focused7 passed all3 before latest negative additions;
development failure/retry boundaries are in docs/original-engine-formats.md.
The subsequent final frozen evidence below supersedes these intermediate results;
no completed slice or native root acceptance is claimed.

Coupled NativeCamera/NativeCameraView batch frozen canonical644/inclusive651.
All3 full configured builds and focused7 PASS3.38s/15.75s/9.28s, including final
FX pitch/odd viewport/near-far/negative/real generated mesh coverage. Related175
passed all3 in170.01s/733.28s/446.08s; all exact jobs completed and source digests
were recomputed unchanged. Evidence:
evidence/qa/N2-native-camera-view-checkpoint.md. Commit this coherent batch, then continue
the same source camera/frame/model/constraint and staged UI/headless root work.
No full365, completed slice03/N2 or whole-game rendering claim is established.

Next coupled navigation refinement (parent f3a4f0cb; existing slice03): supply
allocation-free unnormalized view-plane displacement in NativeCamera and consume
it in production NativeCameraView::scrollBy. Preserve source scale250, integer
aspect, XY-only adoption, recorded scroll amount and rotation-only cancellation.
Stage checked displacement, position and resulting camera before publication;
nonfinite/overflow input must preserve accepted eye, position, scroll state and
choreography. Complete actual lock/snap choreography flags in shared ScriptedView
with disabled-motion and malformed-distance boundaries. Positive/negative tests
must use production navigation, rotated/odd/non-square viewports, retained path/
zoom/pitch activities, allocation terminal0 and corrected retry. Keep concrete
frame/chase/model/shake/slave, real-map constraints and UI/root adoption pending;
this shared provider remains abstract and cannot fake a complete headless View.
Run all camera families all3 during iteration, then the related ownership/world
matrix on frozen source before commit. No new slice or milestone plan is created.

Navigation batch now frozen canonical644/inclusive651 with367 configured tests.
GCC iteration build/focused9 PASS4.33s; all3 full build/focused9/related177 jobs
started in exact sessions99342/32125/65854. Evidence:
evidence/qa/N2-native-camera-navigation-checkpoint.md. Poll those jobs and retain
source unchanged through validation; no completed slice/N2 or source commit yet.

All3 frozen full configured builds/focused9 now PASS4.43s/20.41s/11.87s;
related177 remain live in those same handles. No failure or retry observed in
this batch. Source/root/extent carry-forward census is durable in the engine
formats document; actual root adoption remains pending, not bypassed.

Navigation checkpoint terminal acceptance: all3 full configured builds/focused9/
related177 PASS; related times171.36s/740.94s/450.43s. All exact jobs terminal0,
canonical644/inclusive651 recomputed unchanged. Commit the coherent navigation
behavior/tests/source findings/evidence, then continue concrete camera frame/
model/constraints/shake/slave and staged actual UI/headless root admission and
cleanup in this same slice. Full367, slice03 and N2 remain incomplete.

Coupled camera lifetime batch refinement (parent e13b9605, clean tree): before
concrete chase/slave/frame adoption, withdraw raw Drawable camera borrows across
all live Views during client destruction, construction cancellation and direct
Drawable owner retirement. Keep Object lock IDs when only the Drawable retires;
never dereference a possibly stale pointer or invoke virtual callbacks/allocate
during withdrawal. Use a game-owned intrusive live-View chain, distinct from
Display ownership, whose construction/destruction registers/retires exactly.
Protect actual Display View admission against null/duplicate/cross-owner cycles;
link/withdraw owned Views directly without a fallible prepend callback. Retire
Display-owned Views one by one with legitimate tactical publication through
destruction, then withdraw only that exact publication. Preserve foreign Views
and borrowed parents; support direct owned-View retirement without stale list
links. This changes game-owned lifetime bookkeeping, not a framework or gameplay.
Generated tests must compose actual native spatial Views, original Drawable/
client lifecycle and source Object-survives-Drawable-replacement behavior, plus
duplicate/cross-owner attachment rejection, constructor-failure cancellation,
reset/direct/whole-owner destruction and zero-allocation withdrawal. Keep all
repeated source parent/registry/pool/whole-owner boundaries. Use focused all3
iteration checks before frozen broader ownership/world validation. Concrete
constraints/frame/model/shake/slave/UI/root acceptance remains pending; no new
milestone/slice plan or fake physical provider is introduced.

The live-View chain is a non-owning process registry; sanitizer reachability alone
cannot prove it retired. Expose only its count and require exact repeated and
constructor-failure registration retirement alongside standard allocation/pool/
whole-manager boundaries. No private addresses are diagnostic output.

Lifetime checkpoint closeout: canonical646/inclusive653 remained unchanged;
all3 full configured builds/focused13/related179 PASS with unsuppressed GCC and
Clang ASan/UBSan/LSan. Evidence: evidence/qa/N2-camera-lifetime-checkpoint.md.
Commit this coherent batch without marking slice03/N2 complete. Resume concrete
camera frame/constraints/model/shake/slave and actual UI/root integration; full369
is still unrun. No live validation jobs or failures remain.

Concrete camera cohort refinement (parent31ccae86, clean tree): build the actual
client-frame camera on the verified spatial/lifetime owners, not a fake fixed
eye or a component test substituted for root execution. First extend the native
spatial value to adopt validated explicit world camera poses for actor chase,
ordered local shake rotations and source model-bone slave transforms. All fallible
pose validation precedes adoption; nonfinite/degenerate transforms must not create
undefined projection or poison accepted backing. Queries remain callback/RNG/
clock/allocation-free. Derive normal-pose basis from the existing source contract;
do not import renderer internals or introduce a dependency patch.
Preserve finite authored scale/shear in an explicit pose: CameraClass::
Update_Frustum calls Transform.Get_Inverse, whose original Matrix3D implementation
uses a general D3DX inverse (not Get_Orthogonal_Inverse). Use inverse rows for
projection and raw world columns for unprojection. Ray direction alone is
normalized as in getPickRay. This was verified before frame/model adoption;
do not silently normalize bones or substitute transpose for scaled transforms.

Then compose source cached real-map constraints and explicit rebuild cardinality,
original Object follow/tether/snap/airborne and first-DrawModule chase queries,
freeze/pause/fast ordering, terrain-height settlement, legacy shake and reachable
C3 shake/slave resolution. Preserve Object/Drawable identity checks and actual
parent cleanup from the previous checkpoint. Inspect original RNG and model query
acquisition/output contracts together before adopting their providers. Unsupported
physical picking/drawing remains explicit, not successful empty output.

Validation: stage focused generated pose/negative/retry/zero-allocation tests,
then real NativeTerrainLogic and original actor/model/root composition; test
rebuild versus const-query effects, invalidation, inverted bounds, surviving
Objects and all callback/failure lifetimes. Freeze the complete coherent cohort
for all3 build/related ownership-world checks before its payload commit. Full369
and later added families remain milestone completion gates. The current stage
is implementation in progress, not concrete frame or full-root acceptance.

C3 ownership detail: use a root-owned shared NativeCameraShaker value, borrowed
by camera Views, rather than a process-global AutoPool/MultiList whose metadata
outlives a root. Preserve original libc rand() low12-bit visual draw contract,
six admission draws, newest-first accumulation and nine secondary draws per
in-range live record per rebuild. Keep legacy client RNG separate. Stage vector
capacity before random initialization/admission; allocation failure cannot consume
random values or mutate accepted records. Expire in place without a temporary
fallible deletion list. Advance exactly the caller-supplied rebuild step and
compute angles without hidden timestep; no RNG calls in const spatial queries.
Require finite positions/nonnegative power and positive radius/duration, retaining
original degree-to-radian conversion and original discarded-clamp behavior.
Generated tests need an independent original-formula oracle and standard-owner
admission failure/retry/whole-destruction, not a claim from descriptor counts.

Concrete View composition refinement before production frame changes: add
NativeFrameCameraView on the verified NativeCameraView with mandatory borrowed
root-owned C3 context. Keep frame/rebuild actions explicit and override spatial
synchronization to read accepted backing only; never invoke models, RNG or clock
from const eye/projection/ray queries. Preserve source two-build invalid-constraint
path, explicit recalculation without clipping/rebuild, viewport-only projection
refresh, and cached constraints across angle/position/scroll changes. Keep the
source shared follow ramp in root context rather than a process-static survivor.
The complete camera phase includes actor lookup/loss, follow/tether/immediate snap,
airborne orientation, actual first-DrawModule chase, debug/script freeze versus
pause/fast distinctions, legacy spring shake, C3 rotations and actual first
ObjectDrawInterface slave-bone selection/listener publication. Missing bone output
must never be read: source ignores the Bool with an uninitialized Matrix3D, an
original UB bug; preserve the valid normal pose when no bone was produced.

The native View still has explicit physical picking/drawing adapter obligations
for N3; no successful empty render path. Tests must compose actual ActorParents,
ScriptEngine, native terrain and original Drawable module selection, and measure
explicit rebuild counts and borrowed publications through shutdown. Missing root
parents reject before advancing the frame/effect context. Existing production
ScriptedView algorithms remain the frame choreography, not replacement movement.

Camera cohort checkpoint boundary: at the user's worktree cleanup request,
commit the coupled spatial-pose/constraints, shared C3 owner and production
camera-phase implementation with generated original actor/model/lifecycle tests,
source findings and truthful workflow recovery state. This is independently
testable camera behavior, not a helper-only commit or N2/root/render acceptance.
Source-defined coincident Look_At and coplanar-plane bug handling were reviewed
together; retained failure/retry evidence is in
evidence/qa/N2-camera-cohort-implementation.md. Required frozen validation is
all3 configured builds, focused17 and related184 on canonical651/inclusive658;
full374 remains the later milestone gate. Continue actual UI/Display/root/client
factory integration only after this clean committed checkpoint. No asset,
dependency or recovery archive changes are included.

Frozen camera checkpoint closeout: all3 full configured builds/focused17/
related184 PASS on unchanged canonical651/inclusive658. GCC and Clang sanitizers
remain unsuppressed; full374 is unrun. Evidence and every development failure/
retry are retained in evidence/qa/N2-camera-cohort-implementation.md. Commit this
coherent behavior without marking slice03 or N2 complete; resume actual native
UI/Display/root/client factory integration and production model providers.

Camera checkpoint payload: `e9821721595b958123b359b812a5a078aea730cc`, parent
`31ccae864d3b9cb0b77a1f2eacfcd6c61dd7d191`. This provenance closeout records the
verified checkpoint only; slice03 and N2 remain in progress.

Actual UI/root adoption refinement (parent0d18cdac, clean tree): original
GameEngine initializes GameClient before GameLogic, so InGameUI creates its
tactical View before TerrainLogic exists. W3DView::calcCameraConstraints simply
leaves constraints invalid without terrain, while setCameraTransform still does
the temporary and final builds. NativeFrameCameraView must support this real
startup phase, retaining invalid constraints and explicit build/effect cadence;
actual frame update and map-height initialization still require their real parents.
Do not manufacture a terrain provider or reverse original engine ordering.

Batch tactical admission with that correction: initialize/configure a guarded
View candidate under a scoped tactical publication, admit it to the actual
Display only after all fallible setup succeeds, and restore borrowed publication
on rejection. Duplicate initialization must not orphan a prior Display-owned
View. Configuration loading, logical UI state and physical control-bar/menu
creation remain separate integration surfaces; no empty physical success.
Inspect source constructor/init/reset/update/destructor together before selecting
the eventual headless UI route. Tests exercise pre-terrain camera creation,
later real-terrain adoption, explicit missing-frame-parent rejection, failed View
initialization/configuration and same-owner retry, exact live registration and
allocation retirement. Continue within this existing coherent root slice;
focused checks first, frozen broader GCC/Clang ownership checks before commit.

Complete the same UI adoption cohort with an explicit headless InGameUI mode
and native logical UI factory: retain actual INI/localization loading, tactical
admission, message/subtitle/timer state, floating text and camera input, while
omitting only physical control-bar/menu/money/power/idle-widget output. Preserve
interactive mode unchanged. Root-owned C3 context must outlive every View;
headless drawing/picking remains explicit rejection. Reset and destruction must
not call or delete absent/foreign physical controls. Review select/deselect with
the same lifecycle: stage selected-list allocation before flag/count publication,
retain actual source selection/nexus logic, skip only control-bar display callbacks
in explicit headless mode. Initialize previously uninitialized camera-input/
tracking/quit-menu flags to false, and reset them as one deterministic input
cohort. Test real generated UI configuration, initialized source flags, headless
update/reset, selection allocation rejection/retry, Drawable terminal withdrawal
and whole owner retirement. Native client/root factory wiring remains required;
these UI tests cannot accept full GameEngine startup.

Logical UI/tactical startup component checkpoint: final configured builds,
focused26 and related185 PASS in GCC, GCC sanitizer and Clang sanitizer on
canonical652/inclusive659. Exact hashes, timings and retained failure/retry pairs
are in evidence/qa/N2-headless-ui-implementation.md. Commit this coherent source,
test and documentation cohort; continue the same slice03 with logical command
metadata and actual client/root integration. Full375 and N2 acceptance remain
pending. Payload provenance is recorded by workflow/delivery/state.yaml.

Logical command metadata integration refinement (parent992b8b44, clean tree):
retain the actual ControlBar CommandButton/CommandSet parsers, command enum and
lookup semantics. Expose common metadata init/reset independently of physical
scheme/window setup. Require legitimate ControlBar publication for parsing;
reject duplicate init, and retire partial definitions on initialization failure
so that the same unpublished candidate can retry. Interactive init/reset invoke
the same common operations. Preserve image resolution when its real parent is
present; an absent image parent retains source-selected names for later binding.
Review new/override candidate acquisition and both intrusive lists together:
guard pooled candidates through fallible copying; reset removes every override-
only definition (including interior nodes), relinks survivors and retires chains.
No physical GUI success or changed command execution semantics are introduced.
Generated source INI tests cover actual logical init, duplicate rejection,
base/override lookup, interior override-only retirement, invalid data teardown,
allocation and typed-pool failure/retry, repeated whole-owner destruction under
GCC/Clang sanitizers. Freeze before related checks and commit with evidence.
This remains preparation for actual native client/root adoption within slice03,
not standalone N2 acceptance or a replacement implementation plan.

Whole native root continuation contract, following the command metadata cohort:
adopt native UI/camera/Display and metadata as actual GameClient-owned services,
with C3 lifetime surviving base-client cleanup. Preserve client-before-logic
initialization and the real terrain/camera/UI frame phases. Complete factory
selection and GameMain reachability; generated worlds must enter actual original
GameEngine init/reset/update and GameLogic simulation, not fixture counters.
Batch missing logical parent admission with lifecycle closure: real terrain/water,
ghost/partition, parsed command-science parents and diagnostic loading state.
Physical output remains explicit absence in headless mode, not fabricated success.

Before execution, inspect whole reset/shutdown dependencies: actor/client-only
Drawable cleanup needs live team/player/script/radar/UI parents despite registry
reverse order. Preserve source destruction callback ordering, guard fallible queue
admission and avoid using allocating GameClient::reset as shutdown cleanup.
Constrain UI preflights to the branch that actually uses a parent. Exercise live
actors plus client-only effects, repeated world reset and whole engine destruction,
and complete failure/retry manifests with identical initialized provider cohorts.
Current component matrices do not accept these whole-root paths; verified source
contracts and unresolved implementation limits are in docs/original-engine-formats.md.

Native client composition implementation refinement (parent2744b0dc; clean tree):
make native logical service composition the default production client mode.
Existing Drawables-only fixtures explicitly select that limited mode; it is not
a whole-root provider. Base GameClient owns the C3 context before its ownership
journal, keeping it alive through base cleanup and Display-owned View retirement.
Adopt real native strings/fonts, a logical Display with explicit physical-output
rejection, actual ControlBar metadata and NativeHeadlessInGameUI. Preserve source
client-before-terrain startup, configured virtual viewport and source camera/UI
phase order. Reject foreign/partial publications before acquisition, duplicate
initialization without replacement, and roll back the complete adopted cohort
on error so the same client can retry. Keep the pre-existing DrawGroupInfo owner.

Reset logical command overrides and Display Views even in composed headless
mode; no physical menu/video call. Retire metadata explicitly after UI and before
other parents. Restrict the GameLogic deselection preflight to selected Drawables,
the only branch that reads its frame. Add actual default-client generated INI
composition tests for startup before terrain, configured View/command binding,
duplicate rejection, reset, missing/bad input rollback and same-owner retry,
every standard-allocation prefix/terminal and whole-owner retirement. Existing
client/UI/text and command tests remain required. Native terrain/root factories,
whole engine actor shutdown ordering and GameMain execution remain pending;
this change must not claim those gates. Focused iteration precedes frozen related
GCC/Clang sanitizer validation; preserve all failure/retry pairs and commit the
coherent source/test/evidence cohort together under this same slice transaction.

Include source DrawGroupInfo INI/localized font overrides and the actual native
display-string postProcessLoad cache in this same composition, not deferred to
the root registry (GameClient invokes it itself in the original init). Generated
CSF and supplied local fonts must exercise nonempty numeral/formation strings.
Use actual unloaded NativeTerrainLogic, with explicit rejection of missing device
admission, as reset parent after pre-terrain startup. Its getExtent initializes
the full output without a loaded map. Common TerrainLogic::getExtent is an empty
stub and cannot be a valid fixture; discard composition evidence using that base.
This still proves neither loaded terrain nor composed simulation frames.

Next coupled root retirement implementation direction: reserve the actual
destruction queue node in each Object during guarded construction, rather than
allocate while destroying a root under failure. Preserve DestroyModule/status/
AI/splice/Object::onDestroy order, return the reserved node on a pre-mark callback
exception, retain marked late-rejected actors in the source queue, and keep
processDestroyList's child-append traversal. SpawnBehavior::onDelete is an actual
recursive child path, not a death callback. Batch this with actor/client-only
Drawable quiescence and explicit owned team-before-player/script retirement;
test complete root reset/shutdown and live children, not a node helper alone.
Inspect registry ownership and all source callback parents before execution.
Current source and verified limits are durable in original-engine-formats.md.

Native default-client composition checkpoint verified: canonical654/inclusive661
final identity in evidence/qa/N2-native-client-composition.md; configured builds
PASS all3. Unchanged-production unaffected173 plus corrected native-font21 PASS
all3; exact terminal205/every retry and four typed pools. The invalid common-base
terrain reset fixture was replaced with native initialized no-map bounds; its old
composition evidence is not accepted. No live jobs. Commit this coherent owned
source/test/documentation cohort; continue whole-root factory/composed frames,
science/model/terrain/ghost parents and coupled actor retirement in this same
slice03. Full384, slice completion and N2 acceptance remain pending.

Coupled world retirement batch (parent dfb3a3598446e4d36a49c607687eaeb7ce2c4a84;
clean worktree, no pre-existing edits): continue this slice, not a replacement
plan. Own Object/GameLogic queue admission, GameClient drawable quiescence,
GameEngine reset/destruction, registry publication/targeted retirement, and their
generated source ownership fixtures/build definitions/documentation. Reserve
one destruction node during guarded Object construction and splice at the original
post-AI phase. A pre-mark rejection returns the node; marked failures remain
queued. Recursive re-entry while callbacks are active must not acquire another
node or double-queue. Preserve processDestroyList's append traversal.

Registry cleanup must use acquired owners, not current borrowed globals. Provide
nonallocating publication activation and slot-based owned retirement, keeping
borrowed prefixes untouched until restoration after cleanup. Quiesce owned actors
and all owned client-only Drawables before root reverse reset/shutdown; retire
owned teams before players while scripts remain alive. Reuse the source drawable
cleanup loop, not allocating GameClient::reset during destruction. Headless root
reset explicitly omits the absent blank menu, never claims physical presentation.
Validate actual GameEngine cleanup with registered source parents/live actors,
recursive children and client-only effects; distinguish this lifecycle fixture
from still-pending full GameEngine initialization/simulation acceptance. Inspect
constructor rollback, callback rejection and all existing actor fault manifests;
new constructor demand changes their exact terminals. Focused GCC iteration then
frozen related GCC and both sanitizer matrices, unchanged timeout/leak policies.
Commit the complete coherent lifecycle cohort only after required evidence passes.
Source cleanup trace refinement: OpenContain removal currently re-registers an
already-marked destroyed rider into partition/pathfinding before Object::onDestroy
finishes. This is not live evacuation: the actor is terminal and will immediately
be deleted. Close that terminal re-admission across the shared enclosing helper,
retaining live evacuation and recursive non-enclosing child traversal. Keep
membership, removal callbacks and audio phases; do not suppress them to obtain
zero demand. Add source terminal/recursive coverage and preserve source live
re-entry coverage before committing this coupled batch.

Root init/reset continuation refinement: after the currently frozen related204
matrix finishes, add actual source GameLogic::init/reset plus default composed
native GameClient to a registry-owned GameEngine reset fixture, with initialized
name keys, AI, partition/script owners, actual native no-map terrain and source
water/weather base settings. This is empty reset/owned lifecycle coverage, not
full loaded simulation or fog acceptance. Close the inspected ScriptConditions
transport-cache null-call/global-owner defect and AudioManager's guarded
constructor cohort together with this provider adoption. Preserve cache predicate
ObjectID/frame/count semantics, source reset phase order and callbacks; use
owner-local cache retirement, not global suppression. Test independent source
conditions cache owners and real failure/retry boundaries, plus nonempty live
containment evacuation against loaded partition data. Keep all required parent
identities alive through source destructor callbacks and restore borrowed globals
only afterward. No production edit during the current frozen matrix; retain its
source-attributed results and validate subsequent deltas proportionately.

Coupled lifecycle checkpoint verified before commit: constructor-owned actor
queue admission and callback/retry ordering, acquired registry parent activation,
source actor/client-only effect quiescence, team-before-player retirement,
owner-local transport cache and guarded AudioManager constructor all have
positive/negative ownership coverage. Actual GameLogic::init plus default native
client enter original GameEngine::reset repeatedly: replacement-vector failure
ordinal0, exact terminal1, each with corrected same-owner reset. Loaded-partition
full ThingFactory actors and stock OpenContain prove live unload visibility/cell
restoration and recursive terminal retirement. Canonical656/inclusive663 frozen
configured builds/related208 PASS all3: GCC234.80s, Clangsan598.55s,
GCCsan960.93s; every process terminal0 and both hashes rechecked. Evidence:
evidence/qa/N2-root-retirement.md. Commit the complete owned lifecycle cohort;
this does not complete slice03/N2 or accept full392, GameMain, actual full engine
startup, loaded simulation/checkpoints, native ghost/model/science or physical
presentation. Continue those original-root integration gates under this plan.

Lifecycle payload commit: `850bfa4cea5f17962658371ddf43e3b98df580e9`.

Original startup configuration admission continuation (parent eab07f4a; clean
transaction start): retain this plan/slice03 and the complete native-root goal.
Own the environment cohort parsed by actual GameEngine::init, not test-only
base settings. Capture borrowed water/weather base identities and time-of-day
water payloads before parsing, isolate new source heads, capture acquired heads
on successful or failed loads, and activate them through legitimate root cleanup.
Retire only those acquired chains after registered world callbacks finish, then
restore borrowed configuration without allocation. No serialized layout changes.

Batch source water/weather admission: stage pooled candidates before publication,
preserve destination ownership links across fallible payload copies, parse before
linking an override, and withdraw publication if output notification rejects.
Keep the source root-base copy semantics and water skybox old/new argument order.
Water time-of-day updates must stage their payload before mutation. Fix the
coupled snow owner's uninitialized array cleanup/duplicate init and validate
finite, bounded box size and safe density/velocity before integer conversion,
modulo or reciprocals. Preserve valid source arithmetic and visual-only RNG;
do not replace simulation behavior or claim physical output.

Validate actual source parsers with base, multi-override, malformed, pooled and
standard failure/retry cases, exact terminal census, prior chain identity and
whole-owner retirement. Include a real GameEngine::init prefix using generated
mounted data and isolated user storage, ending truthfully at a deliberately
unavailable device-provider boundary; prove parsed configuration and partial-root
cleanup, not full startup or gameplay. Existing actual initialized root-reset
and source configuration/water/client tests remain required. Focused iteration,
then freeze the coherent cohort for GCC and GCC/Clang sanitizer related checks.
The concrete native process factory and loaded simulation remain mandatory
following this configuration admission work; do not accept a prefix as N2.

Environment continuation implementation: the coupled production owner/parsers
and generated environment_settings fixture are in progress. Initial eight GCC,
GCC sanitizer and Clang sanitizer families passed (0.22s/3.44s/1.45s); source
weather/time standard manifests are 16/11, snow array terminal 1, every prefix
has a same-owner retry, and fixed typed-pool exhaustion retires owning units.
The actual root-prefix fixture now reaches the explicit audio-provider stop and
retires its admitted services/configuration (GCC 1.94s). Its initial missing
custom INI failure was a fixture defect: source prepFile requires every selected
file. All selected prefix INIs now exist; empty generated tables are not accepted
as complete-game content. No physical output or simulation claim is added.
Freeze this whole cohort for configured builds and related suites including
configuration/LOD/environment under all three configurations before committing.
Current evidence and pending limits: evidence/qa/N2-environment-settings.md.

Environment cohort final validation: all configured builds and related249 PASS
GCC271.38s, GCC sanitizer1116.02s and Clang sanitizer708.31s; every process
terminal0, no suppressions or timeout changes. Frozen canonical660/inclusive667
fingerprints rechecked before commit. No live jobs remain. This closes the
coherent environment admission checkpoint, not slice03/N2: full401, concrete
native factory/full original startup and loaded simulation remain pending.

Environment payload commit: `2b14ad41699323b5243b4ab061bf02aa27773826`.

Concrete native factory continuation (parent d172c2fc, clean transaction start):
retain the entire original-root/model/loaded-simulation contract above. Before
production wiring, close the CPU asset/prototype/model-query prerequisite as one
coherent admission/lifecycle cohort: original W3D hierarchy, HLOD relationships,
mesh attributes/bounds and raw/compressed animation semantics; actual condition
selection, pristine/current bones and projectile offsets; immutable supplied
bytes, owned runtime metadata and exact candidate/cache retirement. Refer to the
new source anchors in original-engine-formats.md, not historical generated-only
model acceptance. Inspect all shipped draw-data extension registrations and
paired query callers; do not skip unrecognized retail draw definitions, substitute
identity transforms or replace the loaded terrain/fog providers with empty bases.

The concrete root must own CPU terrain visual metadata through world/logic/client
retirement. Its no-output mode remains explicit and does not accept physical
render/audio success. Existing AbsentAudioFixture is not a production provider;
actual source audio configuration/music readiness and camera/terrain listener
ordering remain part of full startup. Preserve actual original GameMain ownership,
GameEngine update ordering, source GameLogic/module behavior and serialized layouts.

Add coherent generated file/prototype/query admission, malformed nested extents,
versions/names/counts/parent/asset relationships, cold/cached failure/retry and
whole-owner tests while wiring their production consumers. Freeze the complete
model/provider cohort before the broader compiler/sanitizer matrix. A decoder or
factory link alone is supporting evidence only: full registered root, shipped
fog/geometry/bridge behavior and nonempty original simulation remain required.

CPU input-owner implementation phase: add NativeW3DDocument to the actual
original_data graph. Read from the original File boundary into owned immutable
bytes, bound nested chunk extents/depth and decode source hierarchy, HLOD,
mesh-header/channel backing and raw/compressed animation headers. Retain every
uninterpreted payload rather than making a lossy replacement for later rendering.
Stage replacement in a complete candidate and preserve accepted owner/view
identity on malformed input or allocation failure. Generated whole-document,
VFS, truncation/nesting/typed-count and exact failure/retry tests exercise this
production reader. Animation sampling, material decoding, asset cross-reference
resolution and original draw/query consumers remain part of the same model
cohort; this input-owner phase is not a usable-model or N2 acceptance boundary.

User-requested worktree checkpoint: this phase is independently exercisable
through the actual original File/VFS boundary and committed as supporting
behavior, without closing the encompassing model cohort or slice03. Frozen
W3D/core/data related23 PASS all3 (GCC0.98s/GCCsan5.93s/Clangsan4.15s), exact
allocation terminal51 and every retry, canonical663/inclusive670. Evidence:
`evidence/qa/N2-w3d-document.md`. Full405 unrun; continue model sampling/material
admission and original query consumers, then actual native factory/loaded world.

Animation/model continuation (parent e0789e5e, clean worktree): implement the
complete CPU motion admission cohort, not only raw samples. NativeW3DAnimation
resolves an explicit matching hierarchy and owns all admitted channel backing;
raw vector/bit, time-coded vector/bit and adaptive-delta streams share staged
replacement and exact failure/retry tests. Preserve pre3.0 raw pivot shifting,
source discard of out-of-range pivots, unknown payload retention in the document,
raw default/identity behavior, next-key binary movement, compressed endpoint
clamping, low/high nibble signed deltas and original filter-table construction.
Bound backing before expansion; reject malformed active channels and non-finite
sample requests without invalid native conversions. Keep raw Generals rounded
frame sampling distinct from the generic interpolated query. Do not reinterpret
enum names: compressed add_channel actually selects flags0/1/2/6.

Inspect complete admission/retirement and all interpolation paths before frozen
GCC/Clang sanitizer evidence. Tests must span multiple delta packets/components,
random-order seeks, raw last-to-first interpolation, sparse binary transitions,
visibility boundaries, borrowed-input retirement and whole-owner replacement.
Known original adaptive cache/reconstruction bugs must not be reproduced as
undefined behavior or order-dependent corrupt animation; document the source
defect and the deterministic corrected decoding. This remains supporting model
work under slice03: pose composition, materials, prototype/condition binding,
original production query consumers and complete game startup remain mandatory.

Freeze arithmetic as well as ownership: compile original_data with
`-ffp-contract=off` so the source interpolation/delta/composition operations are
not silently fused on a different compiler or CPU. Revalidate the whole related
cohort on that final configuration; retain the earlier freeze as prior evidence.

Coupled pose/query phase before cohort freeze: NativeW3DPose owns hierarchy
names, parent indices, authored quaternion/translation bases, capture controls
and two complete pose buffers. Compose external root, parent/base, local scaled
motion, then capture exactly in source order; root ignores authored root data.
Case-insensitive bone lookup must distinguish a found root from source miss0.
Pristine queries exclude animation/capture; captured local versus world-space
translation and forced visibility follow pivot.cpp. Source blend uses generic
interpolated motion and visibility OR, not Generals-rounded sampling. Evaluation
and query must allocate nothing after admission, remain deterministic across
random seeks, and preserve the previous whole pose on rejection. Generated
rotation/parent/scale/capture/blend oracles plus exact owner failure/retry and
allocation-free evaluation tests cover the coupled motion-to-bone boundary.

Before commitment, include a generated integrated original File/VFS -> decoded
hierarchy/document -> owned animation/pose -> named muzzle-query fixture. Retire
the file, VFS, document and borrowed hierarchy before querying the resulting
owners. Warm that identical complete source cohort before ownership baselines.
This integration is still not original draw-provider or GameEngine acceptance.

Frozen CPU motion/pose cohort: production owners and generated raw/sparse/delta,
negative/ownership, pose/capture/blend and original File-to-model tests pass related32 in all3
configurations (final GCC2.59s/GCCsan9.32s/Clangsan6.89s), terminals7/7/6 and pose3,
every retry, allocation-free evaluation. Canonical668/inclusive675; evidence
`evidence/qa/N2-w3d-motion.md`. Full413 unrun. This is a supporting admission/query
checkpoint, not closure of the whole model cohort, slice03 or N2. Continue
material/prototype/HLOD/cache/condition binding and original production query
consumers, then actual native factory/full original startup and loaded world.

Asset/prototype continuation (parent bd5b835f, clean tree): implement one coherent
NativeW3DAssets original FileSystem-to-resolved-model/animation cohort. Preserve
case-insensitive keys, model container-prefix filenames, animation suffix
filenames, hierarchy filenames, localized/base/user W3D precedence and first
accepted duplicate definitions. Release-only lookup stays inside configured
asset/user namespaces: do not revive legacy/test-art discovery or literal parent
traversal. Read-only input files and unknown document bytes remain untouched.

Use offside complete cache state and immutable document/definition handles;
commit a request only after every required hierarchy/HLOD child/aggregate and
skinned mesh bone relationship resolves. Empty HLOD hierarchy legitimately uses
the source one-root default; a missing named hierarchy/child rejects rather than
fabricating a usable model from source error-path omissions. Proxy names are
metadata, not recursively loaded render objects. Detect prototype/file cycles
and bounded depth, preserve LOD order/cardinality/thresholds and all authored
backing for later renderer use. Model, hierarchy and decoded animation handles
must survive cache clear and FileSystem/asset-owner retirement. Cache hits and
clear must allocate nothing; failed cold or warm requests preserve exact
accepted cache identities/counts and retire every candidate acquisition.

Generated whole-file loose/BIG/localized fixtures, nested/external HLOD assets,
source filename routing, collisions, missing/malformed/cyclic dependencies,
typed skin relationship rejection, retained-handle lifetimes and complete
standard-allocation failure/retry sweeps cover this admission cohort. Warm the
identical whole original FileSystem/provider cohort before each baseline and
derive exact cold/warm terminals. Reuse existing motion/pose tests and freeze
GCC plus both sanitizer related matrix only after the coupled cohort stabilizes.
This supports but does not complete condition binding, all shipped draw-data
registrations, actual original factory/startup and loaded simulation.

Animation cache handles retain decoded motion, their exact admitted hierarchy
identity and their immutable input document. Validate model/clip binding by that
identity, not only an equal hierarchy name/count: handles retained across clear
must not silently bind to another generation's reordered skeleton. Load definition
tables in authored root order across prototype kinds; mesh-first grouped loops
would violate first-winner collisions. Named null records are48 bytes including
two pad words before their32-byte name; preserve their backing and explicit
NULL builtin rather than treating arbitrary missing models as null objects.

Asset graph checkpoint verified before commitment: related42 PASS in GCC3.06s,
GCC sanitizer13.32s and Clang sanitizer9.81s, all terminal0. Sandboxed sanitizer
process-inspection errors were retained, then identical host reruns passed with
all checks enabled. Exact cold model/animation144/47 and NULL-seeded143/48
terminals/every retry; descriptor and pooled File retirement, complete-owner
standard retirement, allocation-free cache hits/clear and retained model/motion
queries passed. Evidence: `evidence/qa/N2-w3d-assets.md`; canonical671/inclusive678.
Full423 unrun; broader populated-cache fault shapes and remaining prototype,
material, condition/draw integration are not claimed. This is a supporting
checkpoint only: slice03 and N2 remain in progress under this same plan.

Condition-data continuation (parent2f6a09b0): extract the original
W3DAnimationInfo/ModelConditionInfo definitions and renderer-independent lifecycle
methods into a game-owned shared data header/source. The existing W3DModelDraw
header must consume these exact definitions, not a parallel replacement schema.
Retain flags, field order, source sparse matching and mode values; no serialized
encoding changes. Keep renderer-only operations explicitly pending rather than
linking legacy private renderer backends into the native graph.

Add original condition-to-native-owner admission: model, deterministic first
animation (never client random selection), exact hierarchy binding and owned
pose evaluated at frame0 or frames-1 for flag4, with source scale semantics.
Return a completely owned candidate without publishing partial original bone
caches; empty/no-model states remain legitimate and invalid required assets fail.
Cover source alias selection, night/snow mode matching, flags, scale, no-model,
incompatible trees, missing clip, retained owners and whole-operation allocation
failure/retry using generated original FileSystem inputs. Do not claim INI draw
registration, weapon/turret/subobject caches or actual Drawable consumers are
accepted until their real paths are wired and tested. Preserve the existing
N2 plan/transaction and broaden into those consumers after this shared boundary.

Admission must stage both model/clip graph and pose before publishing the asset
cache as well: use a coupled NativeW3DAssets binding transaction so a missing
clip, incompatible tree or late pose acquisition cannot retain a partial request.
Verified scale path: W3DAssetManager scales HTree only when abs(scale-1)>.01;
validateCachedBones then unconditionally scales the external root matrix. Preserve
both factors in these original condition queries, not a presumed single factor.

Frozen original condition-data checkpoint: related69 GCC4.59s/GCCsan26.60s/
Clangsan19.19s PASS, terminal0 all3, condition cold172/populated mesh-cache175
exact prefixes and every retry, three full repetitions. Shared original types,
alias matcher/copy/mode semantics, public-bone normalization and coupled
first/final owned poses are tested. Canonical675/inclusive682; full426 unrun.
Evidence: `evidence/qa/N2-w3d-conditions.md`. This does not complete draw INI
registration, weapon/turret/subobject caches, actual Drawable consumers or N2.
Continue those original paths under this same plan, then full native startup and
loaded deterministic simulation; no new milestone or acceptance waiver.

Final source after inherited-whitespace normalization: GCC4.54s/GCCsan26.03s/
Clangsan18.58s related69 PASS/terminal0. Hashes and evidence refer to this final
source; initial passing matrix logs are retained separately.

Original draw-definition continuation (parentbbcd6554): extract the complete
W3DModelDrawModuleData header, constructor/destructor, field tables, condition/
alias/default/transition parsing and selector/transfer methods to a common source
that the original draw header consumes. Do not replace original INI vocabulary
with generated test-only data. Preserve default inheritance, explicit animation
replacement/None, repeated animation weights, visibility lists, public weapon/
turret names, ignore flags and transition IDs. Replace pointer-to32bit parse-tag
casts with defined intptr_t conversions; encoded mode indices remain0..5.

Use a guarded original module-data creator and NameKey transaction, keeping key
rollback after candidate retirement. Reject invalid enum/tag/weapon slot, missing
model, ignored/duplicate/late default states and invalid transition rules before
publication. Keep full source particle-template lookup and its initialized parent
contract; unknown named templates cannot silently become None in release.
Generated original INI -> original module-data -> original selector -> owned
model/first-animation pose is the positive integration boundary. Whole-candidate
parse failures and allocation sweeps must retain prior data/name cursor and every
retry, with identical rooted VFS/provider warm-up before baselines. Native whole
draw factory, derived19 registrations, weapon/turret caches and Drawable queries
remain required next work, not acceptance inferred from a parsed base definition.

Verified parent follow-through: ParticleSystemManager is abstract only at three
presentation methods. A fixture subclass rejects render callbacks and otherwise
executes the real init/INI/template lookup/retirement contract. Its newTemplate
acquisition must be guarded through fallible map insertion; use the original
pooled deleter, not ordinary delete. Initialize the module's invalid attach-bone
cache vector before whole-data copies. These coupled source-owner corrections
do not claim a production particle renderer or whole native client factory.

Frozen base draw-definition checkpoint: related74 GCC5.20s/GCCsan30.64s/
Clangsan21.67s PASS, terminal0 all3. Exact cold104/populated prior-module101
parse terminals and particle-template insertion1, every failure/retry; namespace
cursor and pool used units proved before fixture rollback. Original INI ->
module selector -> owned final-frame pose passes; missing/invalid/parent inputs
preserve prior data. Canonical679/inclusive686, full431 unrun. Evidence:
evidence/qa/N2-w3d-draw-data.md. Same plan01/slice03 continues: derived19 data,
native factory/Drawable weapon/turret/subobject queries, condition transfer,
remaining formats/materials, then full startup/loaded original simulation.

Native pristine-query continuation (parent3fb53069, clean worktree): prepare an
owned original ModelConditionInfo candidate plus first/final model binding,
discover public/standard/extra bones with the original unadorned and contiguous
01..99 rules, then derive the original weapon/turret tables before publication.
Bone0 is not a named bone; subobjects are a fallback only when no bone matched.
Search full names before suffix names across all LODs and additional models,
using the actual attachment bone rather than the original unsafe global-index
into LOD0 shortcut. Keep source map/weapon/turret representations and algorithms,
not a competing schema. The candidate, NameKey cursor and asset-cache admission
must unwind together on late allocation rejection, preserving accepted state.
Test source INI through selector, original cache lookups, barrel transforms and
turret indices, with cold/seeded failure-retry cohorts and three compiler modes.
Actual Drawable dispatch, phase/FPU ownership, client current-pose queries and
all derived registrations remain required integration, not implied acceptance.

Frozen pristine-query checkpoint: related77 PASS GCC5.70s/GCCsan34.13s/
Clangsan24.22s, terminal0 all3; exact cold207/populated52 every failure/retry.
The generated original INI/selector now reaches the original pristine, weapon
and turret tables and consumers, including numbered barrels/FX reuse, second-LOD
and additional-model fallback and full-name-before-suffix precedence. Production
NameKey cursor, pool units and asset counts are preserved on late rejection.
Evidence: evidence/qa/N2-w3d-pristine-queries.md. Full434 and N2 remain pending;
next actual Drawable/phase/FPU integration, nested HLOD transform integration and
derived module registrations, alongside remaining transfer/material/startup work.

Serialized-prototype follow-through: HLodPrototypeClass::Create constructs from
HLodDef, retaining nested models. The flattening loop belongs to a separate
in-memory LOD-array constructor, not this load path. Preserve nested prototypes;
do not insert a flattening implementation prerequisite. Record the actual
Drawable adapter's logic/current-client/attachment distinctions and callbacks
before adding it; standard bones come from borrowed TheGlobalData identity,
pristine queries are admitted only in original logic-update/load-game phases,
and presentation callbacks cannot claim output from a headless fixture.

Actual Drawable query-adapter continuation (parent93abe2f4, clean worktree): add
an abstract native draw-module base implementing the original ObjectDrawInterface
pristine/projectile consumers, leaving presentation/current-animation callbacks
abstract. Generated fixtures register a presentation-rejecting concrete subclass
through actual ModuleFactory/ThingTemplate/Drawable construction, not a replacement
Drawable. Pristine methods reselect original conditions with ignored flags cleared,
keep scale, first/final pose and standard-public-bone identity, cap sweeps at64,
stop on the first miss and preserve original missing-transform fallback without
writing for zero capacity. Projectile methods retain barrel fallback, art-angle/
pitch rotation and CACHE_ATTACH_BONE turret-position offsets. Validate raw slot/
turret/index/name boundaries before accessing tables or publishing output.

Use a protected RAII logic-update phase shared by real GameLogic::update and the
fixture's phase driver. Require the module's construction thread and real logic/
load phase before FPU setup or query work. This fixture is phase/dispatch evidence,
not simulation acceptance. A root query transaction snapshots asset/name state
and every participating module cache, including recursive attachment queries;
fallible work completes before output publication. Preserve cached identities on
all standard failures and prove exact namespace/pool retirement before fixture
leases. Use uncached access to the original SparseMatchFinder algorithm so memo
insertion cannot leak partial shared state into a rejected native query.

Test generated INI -> actual registered module/Drawable -> real query adapter ->
pristine sweep/projectile/turret/attachment output, absent and malformed routes,
phase/thread/FPU boundaries, repeated ownership, cached allocation-free queries,
and cold/seeded standard failure-retry sweeps. Run focused tests, then related
native-client/query/logic ownership under GCC and both sanitizer configurations
on frozen source. Full native factory/derived registrations, current client pose,
renderer callbacks, transfer and loaded simulation remain the same N2 acceptance
gate. One coherent commit owns adapter, phase scope, consumers and fixtures.

Frozen actual Drawable checkpoint: related136 PASS GCC168.54s/GCCsan713.18s/
Clangsan427.49s, terminal0 all3. Exact projectile cold245/seeded0, wide
cold247/seeded2, constructor standard1 and pooled acquisition2; every failure
has a corrected retry and whole cohorts retire three times. Actual dispatch,
recursive attachment rollback, condition reselection, FPU and phase/thread
admission are exercised. Evidence: evidence/qa/N2-drawable-queries.md. Full441,
slice03 and N2 remain pending. Next concrete current-client/native draw providers
and derived19 registrations, condition transfer/material/prototype completion,
then full native factory/GameEngine startup and loaded deterministic simulation.

Derived-provider continuation (parent2bac36ab, clean worktree): extract all original
draw-provider ModuleData declarations, constructors and field tables into shared
original-source units. Preserve the exact19 registered names and their original
data inheritance; PoliceCar reuses Truck data, and default/debris/rope/tracer
use plain ModuleData. Register real parser creators through original factory
admission in generated fixtures, including inherited model pristine consumers.
Do not collapse independent beam/tree/prop data into base model data or claim
concrete presentation modules from data-only admission. Guard complete derived
data candidates and namespace allocation through late parser/factory rejection.
Inspect all13 unique derived tables as one cohort; test every registered type,
source defaults and conversions, inherited fields, malformed late input,
repeated owners and exact standard-allocation failure/retry terminals. Run focused
GCC first, then frozen relevant data/query/factory GCC and sanitizer coverage.
This batch supports the actual native factory; concrete19 runtime behaviors,
current-client pose, transfer/materials/startup and full441+ remain required.

Frozen provider-data checkpoint: focused8 PASS1.07s, related93 PASS GCC23.10s/
GCCsan111.39s/Clangsan68.16s, terminal0 all3. All19 exact names and source parser
inheritance are admitted through original ModuleFactory; model-derived data
reaches original pristine consumers. Every cold/seeded standard prefix and retry
is retained (38 manifests, three repetitions); namespace/pool/backing retirement
is exact. Evidence: evidence/qa/N2-draw-provider-data.md. Full449, concrete runtime
providers, current-client animation, condition transfer/materials/startup and
loaded simulation remain the same slice03/N2 gate. No live validation jobs remain.

Next current-model integration keeps original live-client pose separate from the
already-integrated pristine logic cache. Connect actual Drawable current bone
consumers to owned native pose publication/update, preserving exact missing-output
contracts, orthogonal inverse/scale semantics, bounded numbered sweeps and client
thread/phase ownership. Replace a complete model/clip candidate under asset/cache
rollback; ordinary frozen-pose updates and reads must have no standard-allocation
demand. Keep producer APIs protected until concrete providers perform source
condition/transition, weighted client RNG and sync-time animation choreography.
Explicit manual-pose fixtures accept consumers/ownership only, not these producers.
Source clock, all-LOD base bounds, special bounding primitive and distinct LOD/
aggregate/proxy/skin admission findings are recorded together in
docs/original-engine-formats.md. Complete those coupled prerequisites before
claiming concrete native model/camera or full factory acceptance; never substitute
a generic modulo clock, guessed sphere or universally relaxed attachment check.

Current-pose/attachment implementation batch (parent51fc1495, clean worktree):
distinguish source LOD stale-positive fallback0 from rejected negative encodings,
discard invalid aggregate attachments after their original child resolution,
and retain strict proxy/skin relationship checks. Generate positive and negative
parallel-path regressions, including retained accepted graph/pose state.
Add an abstract NativeModelDrawCurrentQueries base over the existing logic-query
base, owning the current model/clip/pose/world transform separately. Its protected
producer admits complete explicit model/clip/frame candidates and allocation-free
pose updates; actual concrete source animation/transition producers remain next.
Implement current world-space, client-only bone transform and model-space sweep
consumers through actual Drawable dispatch. Preserve index0 sentinel, bounded
start0/01..99 sweeps, all missing-output distinctions and source inverse/scale.
Reject worker/simulation-phase client access before globals or output publication.
Test actual two-module Drawable traversal, separate pristine/current frames,
rotated/scaled world transforms, missing/absent/static models, zero capacity,
invalid/nonfinite/late-overflow preservation, complete cold/seeded replacement
failure-retry manifests and repeated retirement with both sanitizer compilers.
Stage conversions and complete replacements under the existing namespace/asset
ownership scopes. Ordinary current-pose update and read must measure terminal0;
existing constructor/pool ownership remains included. Bound-info/OBBOX, native
sync-time animation, concrete19 callbacks and full startup remain explicit gates,
not fabricated headless output. Commit this coupled current-query behavior and
source attachment compatibility together after focused then frozen related tests.

Phase refinement from actual Drawable condition-notification callers: live client
reads and pose advancement reject simulation phase, but protected current-model
replacement accepts legitimate main-thread logic-originated condition changes.
It changes only client-owned state and never supplies live pose values to logic.
For native-only current-bone walks, preflight the same pure accepted-pose reads
before caller publication, preserving late second-module overflow without heap
staging. Do not double-call mixed legacy interfaces with lazy animation effects.

Frozen current-model checkpoint: focused25 PASS28.71s, related98 PASS GCC34.85s/
GCCsan155.79s/Clangsan96.25s, exit0 all3. Actual current Drawable consumers retain
all missing/scale/phase/range contracts; current replacement cold173/seeded176
retains every failure/retry and production cursor proof, three complete owners.
Explicit-frame updates and128-output numbered reads measure standard terminal0.
Source LOD/aggregate compatibility and strict negative/proxy/skin paths regress
together. Evidence: evidence/qa/N2-current-model-queries.md. Full454, source clock/
animation producers, bounding primitives, concrete19 runtime callbacks and whole
native startup/loaded simulation remain pending under this same slice03/N2.
No live validation jobs remain; this supporting checkpoint is not N2 acceptance.
Current-model query payload commit: `20140868832d6769c429ba5d746b09ce8c754ed1`.

Source single-animation timing continuation (parent abe51f11, clean worktree):
implement the original six mode values and Compute_Current_Frame/Single_Anim_Progress
rules as a game-owned, allocation-free value state, including unsigned32 sync-time
delta, backward direction, source one-step wraps/reflections, retained-frame
completion and duplicate-progress previous-frame preservation. Integrate timed
model admission and advancement with actual current-query pose ownership: stage
clock and pose together so late pose rejection cannot advance time/direction.
Keep explicit/manual producers working. Validate all modes and elapsed boundaries,
zero multiplier, invalid finite/overflow input and clock wrap, actual Drawable
outputs, worker/logic restrictions, cold/seeded candidate failure/retry sweeps and
repeated owners under both sanitizer compilers. Concrete condition selection,
client RNG/transitions, embedded sound dispatch, bounds and runtime19 remain next;
this does not replace them with a guessed producer. Same slice03/N2 transaction.

Frozen single-motion timing checkpoint: focused14 PASS32.25s; final prior-frame
focused2 PASS4.41s; related100 PASS GCC40.37s/GCCsan173.46s/Clangsan106.74s,
exit0 all3, no live jobs. All191 exact allocation manifest lines agree; timed
current cold173/seeded176 preserve every failure/retry and production cursor,
three complete owners. Advancement/actual reads measure terminal0. Evidence:
evidence/qa/N2-animation-clock.md. Source sync-time publication, concrete animation
selection/transition/pause/sound callbacks, bounds/runtime19 and full456/root/
loaded deterministic simulation remain pending under existing slice03/N2.
Single-motion timing payload commit: `f2921f1e5192887878091a59e582a371e8ff70ba`.

Fixed-step presentation owner continuation (parent b79590c4, clean worktree):
implement separate accumulated/published unsigned32 time and source last-client-
frame suppression, including fast-time deferred publication and freeze precedence.
Own it in NativeHeadlessDisplay, keeping physical draw APIs explicitly absent.
Actual GameClient headless logical-service updates call a logical timing entry
before camera View updates; the fast branch advances only source camera movements
and skips the ordinary View update. Read the actual script/game/camera parents,
not duplicated fixture freeze flags. Share the configured ScriptedView step.
Preserve monotonically accumulated display time through source reset; fresh
display construction starts a fresh owner, avoiding static state across lifetimes.
Reject workers/missing or wrong parents before time publication. Test actual
display/client/Drawable timing consumption, source script/logic/camera freezes,
same-frame suppression, deferred fast-path values and uint32 wrap; maintain
zero-allocation ticks and same-owner rejection/retry. Existing timing/pose and
client ownership cohorts run on frozen GCC/GCCsan/Clangsan source. Full startup,
real client selection/event callbacks and runtime19 remain the N2 gates; no
physical rendering, artificial simulation or dependency modification is admitted.

Frozen fixed-step display checkpoint: focused1 PASS2.22s; related120 PASS
GCC110.66s/GCCsan443.52s/Clangsan283.05s, exit0 all3, no live jobs. All218
prefix/composed-client manifest lines match; timed current cold173/seeded176,
advance/read0 and composed-client205 retain complete failure/retry ownership.
Actual logical display timing feeds current Drawable poses; source freeze and
same-frame/configured-step behavior, rejection/retry, reset and repeated owners
are verified. Evidence: evidence/qa/N2-display-time.md. Full457 and complete
GameClient update, bounding primitives, concrete19/condition/event callbacks,
whole native startup and loaded simulation remain pending under slice03/N2.
Fixed-step display payload commit: `425c341d81cf0f4385dfb527fe74abe1e9281bbb`.

Bounding-model continuation (parent f9abc2de, clean worktree): decode fixed-width
BOX0x740/version/flags/name/color/center/extent alongside existing typed metadata,
admit both source aligned/oriented prototype kinds in original root order, and
derive owned model bounds across every LOD, aggregate and retained nested HLOD.
Use source scaled-prototype Clone admission: clone recomputes base hierarchy
bounds after geometry/HTree scale, so do not retain the temporary pre-clone cache.
Meshes scale geometry bounds; boxes/null use inherited no-op Scale; null bounds
are sphere .1 and AABB extent .1. Preserve final-LOD reverse BOUNDINGBOX lookup,
current animated override, orthogonal inverse/root and distinct sphere/OBB rules.
Connect actual Drawable first-render-info and bound-box consumers, with client
phase/thread/parent rejection and staged output on finite/overflow failure.
Candidate model/bounds/clip/pose/cache ownership must commit together; accepted
reads allocate nothing. Test typed/unknown flags, malformed widths/names/extents,
duplicates, no special match, all-LOD/aggregate/nested/scale and animated bounds,
absent models, real Drawable dispatch, complete cold/seeded failure/retry manifests
and repeated owners under GCC and both sanitizer compilers. Explicitly retain
concrete19 animation/condition/event callbacks, complete startup/full loaded
simulation and physical presentation as unfinished gates, not inferred from
bounding metadata. Same governing transaction and slice03, no replacement plan.

Frozen bounds checkpoint: focused1 PASS2.22s; related121 PASS GCC113.13s/
GCCsan441.47s/Clangsan288.91s, exit0 all3, no live jobs. All218 manifest lines
agree. Current model cold182/seeded185 retain every failure/retry and source cursor
proof; accepted bounds reads0, independent constructor standard1/module-pool2.
Typed BOX and source scaled clone bounds feed actual Drawable info and current
OBB queries. Evidence: evidence/qa/N2-model-bounds.md. Full458, concrete19
condition/animation/event callbacks, ObjectScale doDraw publication, camera
actor-follow, complete native startup and loaded simulation remain pending;
slice03/N2 are not complete. User-requested worktree inventory found13 initial
owned paths and no unrelated nonignored files; retain assets/dependencies/recovery.
Model-bounds payload commit: 9dce617c38c16b71af10020de25cc2a6b686beeb.

Coupled condition-producer investigation (parent ca5afa8c; no unrelated dirty
paths): use docs/original-engine-formats.md's source contract before production
edits. Batch signed raw runtime sampling/defaults, source weighted client RNG and
rollback, state/pending transitions, frame precedence, duration and pause/reset
semantics together with actual current pose/bounds ownership. The strict decoded
sampler rejects raw source frames that maintain-state and oversized pingpong can
legitimately produce. Preserve signed/default raw behavior; compressed uint32
conversion paths must retain pre-cast rejection. Do not change clock reflection
or reparenthesize the original maintain-frame expression to avoid this gate.
Generic raw interpolation, optimized single-motion sampling and visibility have
distinct rounding/wrap rules; inspect all before changing their entry contracts.
Keep existing malformed-input checks where truly malformed, replace only
source-disproved frame oracles, and add independent raw/compressed boundary pairs.
Prove paused mode/rate/sync changes, duplicate notifications, cancel/restart versus
wait/transition precedence, first-record completion-time split, actual Drawable
dispatch and zero-demand accepted ticks. Stage all resource/condition/RNG changes
offside, test failures late in admission and corrected same-owner retries with
identical production RNG/cursor state, then freeze related GCC/GCCsan/Clangsan.
The concrete19 specialized callback cohort, particle/audio events, source
Object/Physics parents, same-model retained controls/materials, condition transfer
and full startup remain within this existing slice03; no new plan or milestone,
fake missing-parent effects, frame-domain waiver or dependency fork is authorized.

Frozen runtime raw-domain correction: final focused6 PASS2.34s; related121 PASS
GCC113.34s/GCCsan442.09s/Clangsan289.34s, all exits0/no live jobs. All218 manifest
lines agree; current cold182/seeded185 retain every failure/retry and production
cursor proof, accepted runtime raw reads0 and constructor standard1/module-pool2.
Actual Drawable supports signed raw identity/default poses from source clock
reflection and maintain-frame -1, without changing reflection/selection formulas;
explicit bounded/interpolated and compressed conversion checks remain intact.
Evidence: evidence/qa/N2-runtime-frame-domain.md. Continue shared condition/RNG/
transition/pause/duration and concrete19/event/resource-reuse producer work,
then native startup/loaded simulation. Full458, slice03 and N2 remain incomplete.
Runtime raw-domain payload commit: a7f1f9672dad077e62179083433a4175feb9038b.

Shared producer continuation (parent f44b668a, clean worktree): implement the
original condition/animation state owner as a shared abstract native module,
keeping specialized effects/physical callbacks explicit rather than successful
stubs. Actual DrawModule condition/pause/frame/duration callbacks consume the
registered Display presentation-time service. Stage a complete model/pose/bounds/
clock candidate and source selected/next/index/duration/pause metadata together
with NameKey/asset admission and the original six-word client RNG lease.
Reuse preserves captured pose controls, root and ObjectScale; recreation resets
pause false per nukeCurrentRender and invokes the appropriate later effects
contract. Keep real effect parents and unsupported condition effects explicit;
do not report concrete19 or N2 acceptance from empty-effect generated cases.
Share persisted animation flag numbers with original parsing; no new bit values.
Test source index rejection-sampling/weights, all start precedences and raw -1,
duplicate/pending/cancel/wait/explicit transitions, pause mode/sync/rate resets,
completion-time partition/deferred target duration, source movement-rate
preconditions, retained controls/scale, no-model/non-HLOD completion, absent/wrong
time owner, and whole transaction failure/retry under both compiler sanitizers.
Do not commit isolated helper definitions; this remains one coherent producer
batch within the existing plan/slice, with frozen related validation after the
coupled implementation settles.

User-requested worktree review at f44b668a: inventory19 initial nonignored files,
all belonging to this batch. Source findings/plan and review state are eligible
for a documentation checkpoint only. Retain14 modified and3 new implementation
files together, uncommitted: GCC coupled build passed, functional/transitions
passed, weighted-selection failed its retained-completion fixture precondition;
whole-owner failure/retry and current-batch sanitizer coverage remain pending.
See evidence/qa/N2-worktree-review.md for exact inventory, evidence and resume.
This review does not supersede the accepted runtime raw checkpoint or declare
producer, slice03, N2 or the configured full suite accepted.

Resume from documentation checkpoint5daba00f: preserve source completion-before-
reselection ordering. Weighted-selection fixture must only advance a selected
idle clip when it is not already complete; RANDOMSTART can select its endpoint.
Add independent cold initialization and seeded same-model replacement sweeps,
deriving each exact terminal before fault arming and retaining every failed
prefix/corrected retry. Snapshot client cursor via nested public-draw probes
without exposing seed words, and prove failed namespace/RNG/pose/pending/clock/
standard-live and pooled-used identity before outer fixture leases unwind.
Warm the identical generated ScriptEngine/Display/camera/producer cohort before
whole-owner baselines. Preserve accepted zero-demand ticks and add recreation,
empty/no-animation and late-rejection tests; no concrete19 effects claim.

Frozen shared producer checkpoint: focused producer/core12 PASS35.54s;
related128 PASS GCC149.73s/GCCsan584.05s/Clangsan372.56s, all terminal0/no live
jobs. Unchanged canonical726 f609dff2 and inclusive733 631a391a; all278 normalized
allocation manifests agree, cold223/seeded56 and every failure/retry retained.
Sanitizer sandbox LSan thread-inspection failures were isolated by unchanged
normal-host controls; full unchanged related matrix passed there with all leak
checks enabled. Evidence: evidence/qa/N2-condition-animation-producer.md.
Source record selection, pending transitions, pause/duration callbacks, retained
same-model controls/ObjectScale and no-model/leaf RNG paths are implemented in
the shared abstract producer. Whole Drawable dirty/forced/immediate condition
walks, first-leaf malformed-root admission, concrete19/effects/movement parents,
camera actor-follow, actual startup/loaded simulation and full465 remain pending.
These are documented coupled integration work, not a new plan or N2 acceptance.
Condition-animation producer payload commit: 43fc6b1eced6f85c26e5f2e5b11c955eea5fec9d.

Whole-condition-walk continuation (parent926c41d9, clean worktree): preserve
original dirty/forced notification order while enrolling initialized native
producer owners from the raw non-dirty module list. A root candidate scope owns
namespace/assets/client RNG and preallocated per-module previous selected/
pending/pause/duration metadata. On first publication, retain the original
current/validated owners by move, not cloned replacements, so late failure
restores accepted binding identity without allocation/callbacks. Nested getter
walks enroll in the same root; save requested flags only for immediate mutation,
and dirty status before dirty getters clear it. Deferred requested flags remain
pending on failure, while forced mutation restores prior flags. Pristine query
dispatch must open this scope before dirty notification and keep it until its
existing whole-query cache/admission result is known. No new guard allocation
for old uninitialized/non-native fixture paths. Stage finite world conversion on
every existing-model admission, including first leaf initialization.
Test two real initialized generated modules, const/nonconst dirty getters,
forced replacement, late second-module semantic rejection and corrected retry,
namespace/client cursor and exact binding/clock/pending/flag/dirty restoration,
finite first-leaf malformed/corrected admission, and independent complete
allocation failure/retry sweeps. Retain all original lifecycle/constructor/pool
checks and frozen GCC/GCCsan/Clangsan validation. Concrete19/effect owners remain
pending; extend owner enrollment for their additional mutable resources before
claiming full runtime. This remains the same N2 plan01/slice03 transaction.

Worktree checkpoint requested by the user: retain the coupled root, Drawable
callers, pristine-query dispatch, first-leaf conversion and generated fixtures in
one commit, without declaring this slice complete. Parallel-path review also
identified in-place pause/frame/duration/ordinary advancement inside a root;
these now stage a copy before first publication, retaining the exact accepted
owner for allocation-free rollback. Their dedicated nested callback and locally
caught rejection cases still need validation before component acceptance. Run
the existing eight whole-walk families on the current source in GCC and both
compiler sanitizer configurations, record the results separately from earlier
producer evidence, and persist the remaining frozen related-matrix requirement.
This user-requested implementation checkpoint is not a completed-slice commit
and does not waive that requirement or create a replacement plan.

Resume b9edefc4: exercise each copy-on-first-mutation route independently inside
the actual enrolled root, then trigger late failure through the original forced
Drawable walk. Preserve exact binding/source/pending/pause/frame/mode/rate and
duration identity, namespace/assets/client cursor, pooled-used and standard-live
ownership. Separately catch invalid frame/root locally and commit the enclosing
root to prove rejection did not publish a clone. Derive exact standard terminals
for pause/frame/duration/advancement and completion partition, preserving every
failed prefix and corrected same-owner retry. Keep this callback cohort coupled
with the existing whole-walk/query cohort and frozen related matrix.

Whole-walk/mutation frozen checkpoint: focused2 PASS16.86s; related138 PASS
GCC234.16s/GCCsan924.10s/Clangsan571.86s, all terminal0/no live owned jobs.
Unchanged canonical728 2cc04ea0/inclusive735 7ed7748b; all302 normalized
allocation rows agree. Whole-walk terminals119/119/164 and five mutation14
sweeps preserve every failure/retry across three complete lifetimes. Root
publication, metadata-only wait, finite first-leaf admission and staged pause/
frame/duration/advancement/completion partition are now component-validated.
Evidence: evidence/qa/N2-condition-walk.md. Source review also records concrete
tick ordering, visibility cadence, lazy bone evaluation and complete particle
master/slave/deferred-retirement boundaries in original-engine-formats.md.
Resume concrete19 runtime creators and complete logical tick/effect admission,
with real physics/AI/geometry/particle parents and original client-only event
semantics. Constructor/new-owner/effect resources need their complete admission
contract before enrollment; do not infer them from the initialized producer
guard. Actual startup/loaded simulation, full475, slice03 and N2 remain pending.
