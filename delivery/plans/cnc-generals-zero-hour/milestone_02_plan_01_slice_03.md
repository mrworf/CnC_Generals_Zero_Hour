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
