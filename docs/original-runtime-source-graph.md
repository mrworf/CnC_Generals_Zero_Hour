# Original Linux runtime source graph

Implementation reference for N2 slice03; none of this is runtime acceptance.
Reader foundation: commit `9e5c0e4f1a25d20e1d83843815ac39c4619fc08d`.
Use the existing `milestone_02_plan_01_slice_03.md`, not replacement tiny plans.

## Source census and build boundary

`GeneralsMD/Code/GameEngine/GameEngine.dsp` lists 582 C/C++ translation units,
including 259 GameLogic units. Two listed GameNetwork/GameSpy translation units
are absent at those old paths; their current ownership/replacement must be
resolved from source, not silently fabricated. The DSP is a census, not an
instruction to compile obsolete network/UI/platform owners into headless tests.
`GameEngineDevice.dsp` supplies native/renderer providers; no Windows SDK shim,
STLport patch or dependency-private adapter is authorized.

At slice03 entry the retained headers contained 55 distinct unfixed enum forward declarations.
GCC/Clang cannot treat those as MSVC did. Resolve definition/forward-declaration
consistency across the entire retained provider set. Dynamic IDs additionally
need defined raw representations, not synthetic UB before validation. Do not
assign one underlying width to every enum without inspecting its values and
reader/writer uses. Source-defined values and external encodings remain authority.

## Startup and frame ownership

| Owner | Source-established dependency | Native obligation |
| --- | --- | --- |
| GameEngine startup | FileSystem, NameKeyGenerator, GlobalData, subsystem list | explicit roots and XDG writes; guard partial publication/teardown |
| definition stores | Science, weapons, locomotors, armor, modules, ThingFactory | full original INI block table, not generic field tests alone |
| GameLogic init | partition/ghost/terrain/script actions/conditions/engine | preserve actual source instances and rollback all partial owners |
| GameLogic frame | script then terrain, recorder/commands, sleepy modules, AI/build/partition, cleanup/stores/victory | original ordering, frame/RNG/checkpoints; no replacement simulation |
| GameClient/Display/View | queried even during headless frames/load | omit physical output only; preserve simulation-facing captured state |
| map/startNewGame | sides/players/scripts/terrain/objects/recorder | generated legal original setup and supplied-data startup are separate gates |

Primary sources: `Source/Common/GameEngine.cpp`,
`Source/GameLogic/System/GameLogic.cpp::{init,startNewGame,update}` and
`Source/GameLogic/Object/Object.cpp`. The update path dereferences actual Script,
Terrain, Recorder, client/view and AI services; an empty GameLogic instance or
independent RNG loop cannot satisfy N2. Future rendering/media providers must use
the same simulation-facing seams rather than replacing accepted logic later.

## Coupled implementation findings

- `GameLogic.cpp::setFPMode`'s active code selects nearest rounding and 24-bit
  x87 precision after reset. Its introductory CHOP comment is stale: the CHOP
  assignment is commented out. Native float evaluation/rounding must follow
  active code and compiler-matching evidence, not that comment.
- `NameKeyGenerator.cpp` acquires a pooled Bucket, increments its ordinal, then
  assigns fallible string storage without a guard. Both exact and lowercase
  paths share that leak/ordinal-consumption risk. `StaticNameKey` caches only a
  numerical key and cannot distinguish namespace reset or a new process owner
  at the same address. Fix these coupled paths with generation-aware caches and
  offside publication; preserve source string hashing, case-selection semantics
  and assigned ordinal order on successful operations.
- `STLTypedefs.h` originally imported `<hash_map>` and assumed STLport string hashes.
  Standard unordered containers require source-consistent content hashes and
  equality; pointer hashing a content-equal AsciiString is not compatible.
  The traversal audit also found observable users: `ParticleSys.cpp::findParentTemplate`
  selects the nth matching parent; `GameAudio.cpp::isMusicAlreadyLoaded` keeps the
  last music entry; ScriptEngine particle-editor code enumerates templates.
  Do not claim every hash map is lookup-only or blindly change these orders.
  GameLogic version7/8 and Player/Team relation saves emit named/keyed records;
  their readers do not require the original hash bucket order. CRC methods for
  those three owners are empty, but deterministic native serialization still
  needs an explicit ordering contract before container migration acceptance.
- `Dict.cpp` originally embeds strings and scalar values in unconstructed
  `void*` storage and uses uint16 sharing. Native typed payloads and noexcept
  shared payload handles preserve key sorting without pointer punning. Guard
  payload admission and shared-graph clone before publication; unique-owner
  vector capacity reservation is the last fallible step before noexcept shifts.
  Maximum cardinality remains32767 and default name-key capacity is inclusive.
- PCH removal exposes implicit includes, while original shared subsystem owners
  refer to platform and presentation objects. Port source-owned boundaries and
  complete coupled include/type surfaces before broad compilation, not a new
  catch-all Windows-emulation header.

The native container batch now replaces all13 active typedefs with standard
unordered_map and content-compatible hashes, preserving reference stability.
Source list enumeration is independent of bucket order; four named/numeric save
loops collect canonical keys explicitly. Five shared-header/fault families pass
the67-case supporting matrix, but none accepts uncompiled owner classes. Audio
last-music selection is a retained startup readiness gate, not solely a CD check;
its valid/missing-media owner tests remain pending. Particle nth-parent selection
is only called through the excluded runtime ParticleEditor DLL gate, while
preloading remains a retained traversal. See the verified format reference.

## Current graph-port findings (not full compilation)

The GameEngine/Libraries public include-root census corrected112 unambiguous
casing mismatches in96 game-owned files, retaining dependency headers as-is.
The graph regression covers source files too: it caught one remaining ObjectID
opaque declaration in PropagandaTowerBehavior.cpp. Six dynamic ID definitions
now live in Common/EngineIDs.h with consistent UInt32 opaque representations;
their named values and representative layout checks pass the62-test supporting
cohort. Other unfixed enum domains and whole owner classes remain pending.

The original DSP also supplies WWVegas/WWLib, GameSpy and Compression include
roots; the expanded census corrected21 further caller includes in17 files. The
first five actual source probes initially stopped at WWMath/Matrix3D.h casing;
after correction, GameEngine reaches the dinput KeyDefs seam, GameLogic reaches
an absent GameSpy BuddyThread SDK include, and Object/factory/global probes expose
the remaining enum/type/class surface as a batch. The unmodified public header
WWMath/matrix3d.h compiles under GCC with the existing `_OPERATOR_NEW_DEFINED_`
guard (do not restore global allocator declarations). This is header evidence
only: matrix3d.cpp still includes D3dx8math.h and calls D3DXMatrixInverse, while
wwmath.cpp has VC6 loop-variable scope assumptions. Stock outline suitability
and a game-owned native boundary must be resolved without framework patches.

NOX source reachability and the unchanged candidate C API limitation are recorded
in docs/original-engine-formats.md. Preferred-compression loops do not define
decoder reachability. C++/other stock interfaces remain under investigation;
the public C probe alone is neither accepted NOX support nor a milestone blocker.

## Acceptance still required

Whole logic census checkpoints (source syntax only, no link/startup acceptance):
the 75-test support freeze exposed 121/260 compiling TUs and 139 failing TUs.
Removing the unused Xfer→ModelState include cycle and adding the five explicit
declaration dependencies raises this to 188/260, leaving72 failing TUs and365
diagnostic occurrences. Both inventories are retained separately in
evidence/qa/N2-logic-compile-census.json and N2-logic-compile-census-02.json.
The latter is historical before the excluded ScriptEngine tool detach batch.
Follow-up inventories03/04 now record214/260 and244/260 respectively. Inventory04
is the frozen79-case support checkpoint (normal/GCC/Clang sanitizers all PASS),
with16 failing TUs and57 remaining diagnostics. Original cubic math owners now
link in generated fixtures without D3DX or framework changes. Actual entry link,
owner lifecycle faults and full startup remain unaccepted.
The original_logic archive now compiles all259 physical original logic TUs
under normal GCC and both sanitizer compilers, with strict return diagnostics.
FPUControl is the260th source and stays uniquely in runtime_common. No executable
link or simulation acceptance is inferred. The physical145-TU Common inventory
(98/47 syntax outcomes) includes excluded/obsolete providers; classify its retained
startup/definition/transfer/preferences graph before implementing whole cohorts.
GlobalData.h's unchanged copy-assignment is an unimplemented stub, not a default
shallow copy. Corrected that source finding in original-engine-formats.md and
support evidence; complete configuration clone and actual owner rollback remain.
Original ScriptEngine tool helpers require DebugWindow/ParticleEditor DLLs;
their absent-provider path does not alter simulation. Particle name tables stay
owned by ScriptEngine; ordinary script freeze/counters/sequential updates remain.
Actual owner construction/reset and full startup acceptance remain pending.

The source-wide enum batch now covers49 locked domains:47 ordinary opaque
definitions, ObjectStatusTypes and tagged _TerrainLOD. Nine have negative
sentinels;40 are nonnegative indices/masks. Both optional-name branch configurations
and complete forward-declaration consistency pass the69-case supporting matrix.
Remaining raw admission, field-store aliasing and actual owner classes remain
pending even after representation fixes. HackerAttackMode had only two opaque
declarations with no definition/use; those are removed. ObjectStatusType was NOT
unused: RiderChangeContain::RiderInfo stores it, parseRiderInfo writes an index
from ObjectStatusMaskType, and onContaining/onRemoving consume that index. Its
member now uses the real ObjectStatusTypes (plural) domain. That aggregate lacks
explicit initialization for every fixed-capacity rider slot; inspect the whole
parser/default/copy/consume graph before changing defaults. Do not repeat the
earlier census inference that an absent enum definition means an unused field.

Post-enum actual source probes expose43/24 Object/factory diagnostics (previously
137/91). They are dominated by shared template/string/list/filter/time/header
seams, not independent runtime asserts. GlobalData.cpp omits its own header and
relies on the removed PCH, creating most of its cascading diagnostics. GameEngine
and GameLogic still stop at native input/absent Internet SDK boundaries. Port
game-owned seams as a coupled batch and keep framework sources unchanged.

The following original template/list/header batch reduces Object/ThingFactory
syntax diagnostics to14/7, with the complete72-family support matrix passing under
both sanitizer compilers. This is not graph build acceptance. The next shared
boundaries are filter definitions, calendar/save/replay fields, owner includes and
AttackPriorityInfo: its256 entries are embedded in ScriptEngine, yet pool glue
declares a protected destructor. Source census finds no retained pooled allocation;
do not repair this by globally exposing pool destructors or changing allocators.


Actual GameEngine/GameLogic entry and original object/module updates; legal
generated map/setup; matching GCC/Clang checkpoints; startup fault rollback and
same-process lifecycle repeats; XDG writers/cache fallback; rooted memory-profile
overrides; native timestamp/map-cache integration; text filter replacement width;
full read-only supplied startup/integrity. N3/N4 remain dependency-blocked on N2,
not independently delivered by these build/ownership support checks.

### Current native graph checkpoint (same N2 plan01/slice03)

Real259-TU original_logic archives build under normal GCC and both sanitizer
compilers. Source-only Common census02 is146 physical TUs with129 passing under
both compilers; the remaining17 include obsolete platform providers as well as
required GlobalData/Recorder/save-map/quoted-string owners. Actual original
UserPreferences and typed FunctionLexicon registration now have native ownership
support, with all91 support tests passing in the three configurations. Full
GameEngine/INI/global definition/object/module execution is still unaccepted.

The next broader source-only census includes all150 physical GameClient TUs:
66 syntax pass,84 fail,393 GCC diagnostics. See
evidence/qa/N2-client-compile-census-01.json. This is an inventory, not a retained
provider selection or physical UI/render acceptance. Separate removed Internet
SDK/old tool families from required startup definitions, Drawable, original
client update, options/LAN preferences, and GUI-owned callbacks. Resolve shared
header/clock/string/container seams and the complete actual startup ownership
graph as coupled batches; do not treat84 files as84 assert fixes or silently
revive excluded SDKs. Material changes stay in existing slice03 and are tested
against frozen source. Upstream libraries and read-only assets remain unchanged.

Verified distinction: obsolete Common/Audio/GameSpeech.cpp has an empty public
header, no retained caller and no source entry in the original DSP. Retained
GameAudio.cpp::AudioManager::isMusicAlreadyLoaded is a real startup asset gate.
Filename similarity is not evidence that these owners are interchangeable.

### Configuration checkpoint and complete-bootstrap continuation

The coupled configuration/native discovery batch has99/99 support tests passing
under normal GCC and both sanitizer compilers on frozen395-path code SHA
0c238633e87b3574fb4ea54ac2fd305d13b54befd8188a5e553abed1355b1129.
Actual GlobalData clone/parse/override retirement, explicit candidate preferences,
primitive INI admission and native IP chain rollback are covered. Full259-TU logic
archives and the real Xfer default-adapter archive build in all configurations.
The previous Common146/129 and client150/66 physical censuses remain historical
inventories; neither establishes current retained graph execution.

Resume the same plan01/slice03 at complete bootstrap ownership, not a new plan.
GameLogic Objects and GameClient Drawables call their own singleton during source
destruction. Preserve those live cleanup dependencies; blind early singleton
withdrawal is unsafe. Current NativeSubsystemInit failure ordering and complete
GameEngine/child-owner acquisition/registry/publication teardown remain unaccepted.
Detailed verified links are in docs/original-engine-formats.md; support results
and their limits are in evidence/qa/N2-runtime-support.md. Continue complete source
graph census/owner integration without importing excluded Internet/legacy tools,
adding dummy callback providers, editing frameworks or touching retail assets.

The subsequent coupled registry checkpoint is103/103 support PASS under all three
configurations on frozen398-path SHA
667e788770e25cfc78bfb2d8ef785ed48001eeda75a14994dd13400ee4864633.
Typed registry restoration now keeps parent publication live during legitimate
cleanup and restores prior slots immediately afterward. The guarded loader cannot
publish registry ownership; final adoption stays in the list transaction. Actual
GlobalData init/late decode/parallel-root rejection/override/reset/restart is covered.
Full GameEngine/child construction remains pending, including swallowed startup
errors and partial client-service cleanup. The refreshed Common census03 compiles
136/152 physical TUs in both compilers;16 fail (GCC197/Clang117 diagnostics).
Detailed public source hashes/results are in N2-common-compile-census-03.json.

Retained Common follow-through includes Recorder, GameState/GameStateMap and
QuotedPrintable; do not treat their physical compiler failures as independent
asserts. Recorder's header uses native sizeof(time_t) for fixed replay positions
and raw timestamps; this must be reconciled against original writer/reader widths
before Linux64 acceptance. QuotedPrintable is not only an Internet/chat owner:
INIMapCache/MapUtil and LAN/skirmish user preferences also use its encoding.
The source static buffer, unincremented decode counter, signed-byte hex shifts,
UTF16/native wchar representation and repeated odd-tail behavior form one coupled
codec admission/publication batch. Parent cleanup and native writer/read authority
must be preserved across the full remaining owner graph.

The quoted-codec batch is now implemented and directly exercised: all106 support
tests pass in normal GCC and GCC/Clang ASan/UBSan/LSan builds on frozen401-path
SHA0fc51f4c740e2f8008cf414c1bc8d78c43d25db417c4c98a3aeb29f465bf6510.
Actual map-cache/LAN/replay/save execution and complete engine startup remain
pending. Census03 above is historical; its quoted-codec failure is superseded,
not evidence that all retained Common owners now compile or execute.

The subsequent coupled parent batch builds all six actual bootstrap sources
(GameEngine/GameMain/GameClient/MessageStream/full INI/registry) and all259 logic
sources in normal GCC and both sanitizer compilers. Frozen406-path SHA
f1c73d75ba5fc4fd551175533c5dab840a34a21a1c110ec39ea4a304cfc27ccc passes
109/109 support tests in every configuration. Allocation-free member journals now
record direct parent child acquisitions separately from the subsystem list;
GameMain guards init/run/teardown and startup errors propagate before fatal
presentation. Translator node acquisition and client cookie publication are
guarded. See format/ownership findings and exact evidence in the linked documents.
Full real-provider execution and deterministic scenario checkpoints remain pending.

The complete30-unit Common INI directory is now represented by actual owners:
full dispatcher in bootstrap, core/value helpers in their prior targets, and
27 providers in original_definitions without duplicate ownership. Frozen408-path
SHA6078b4933f40a6f27ae4fa420fdb48e68cdbd03001f21bc13f89f272dc95a170 passes all
three complete builds and110/110 support tests in each configuration. Authored
WebpageURL fields are validated without the excluded ATL browser; source language
consumers share the configured catalog language. Full-table execution still needs
the real store/module/client owners, not fabricated definitions or callbacks.

Next retained integration cohort: native user-file namespace and its map/cache,
replay/save, mod and LAN transfer consumers. The Local/ArchiveFileSystem caller
census is in original-engine-formats.md. The current rooted asset interface cannot
replace physical user-file reads or mutable directory enumeration merely by
changing a singleton name. Keep user I/O protected by its XDG owner and preserve
read-only selected mod/asset precedence. Verified excluded GameSpy results/CD/DRM
providers are still in original bootstrap and must be removed with their caller
integration, not replaced by no-op service providers.

Protected user storage now belongs to original_data, alongside the actual File/
NativeDataBacking/VFS owners; original_runtime_common retains conversion/cache
and configuration consumers. This avoids a reverse VFS-to-runtime library cycle.
The413-path checkpoint passes storage/configuration target builds and19 focused
cases in normal GCC and both sanitizer configurations. User File views preserve
their own descriptor backing; attached VFS absolute reads/info/discovery enter only
the configured data root. Directory preparation, source FilenameList publication,
atomic seek/copy and exact retirement have complete fault/retry support coverage.
See original-engine-formats and N2-runtime-support for digest/manifests. Full
matrix/actual process/source folded-map identities/save/replay/mod/LAN execution
remain pending; this is not N2 acceptance or permission to start N3/N4.

Actual bootstrap now includes GameStateMap as its seventh source TU. Frozen414
passes those target builds in all three configurations and21 focused support
cases each. GameStateMap holds captured typed scratch records rather than a
retired GameState peer/CWD wildcard. GameEngine's member acquisition journal has
six slots and retires the XDG storage after subsystems and before asset backing.
Complete native root factory/entry, actual save/load transfer execution and folded
map identity lowering remain pending, not accepted by the shared scratch fixture.

Captured-map support now resolves folded/backslash source identities through the
configured Maps/Save owner without rewriting the physical root. The414-path
SHA9f87c3ae3e5ae2067b1d701d2e5840af4bae88da413d5f9ef777678c628fa872 builds
actual259-TU logic/seven-source bootstrap and the named support targets in all
three compilers;38 related cases pass in each. This covers actual cached stream
and four-companion preparation mechanisms, not whole MapCache/provider execution.
Source MapUtil needs its entire offside metadata query/cache graph and protected
output migration before its corrections can be accepted; the verified surrounding
findings and source links are durable in original-engine-formats.md.

The actual MapUtil TU is now the eighth bootstrap provider; shared unchanged
well-known keys live in NativeWellKnownKeys.cpp rather than W3D terrain, and real
MapCache global/storage/serialization/optional persistence live in
NativeMapMetadata.cpp. Actual selected INI metadata and temporary GameText catalog
fixtures execute these providers without substitute globals. Frozen424 code
SHAad0f024f7e0bd035f56c82a71237ee9a22c1ae169cb597b596d0b19e158d3bd8 builds all
targets and passes126/126 support cases in all three configurations. Whole MapUtil
query/ThingFactory, native entry/root factory, complete save/replay/mod/LAN and
actual startup/scenario remain pending. Optional malformed cache isolation/fallback
is the next connected correction, not a new milestone or acceptance waiver.

That optional-cache correction now executes real bounded INIMapCache callbacks
with whole-file publication; INIMapCache.cpp has moved to runtime_common (26 other
definition providers remain) without duplicated providers or archive cycles.
Actual cold-name acquisition is also coupled to readDict and metadata query/cache
scopes via NameKeyTransaction. Frozen424 SHA33b09711668dbc0b70b00a8dd2687627382885111fb933cda44e232028ca9e92
builds the named targets in all3 and passes31 related cases in each. Exact nested
key/Bucket rollback and ordinal/cached-generation retry are actual provider evidence;
complete process acceptance is still pending. All six Common/Thing sources pass
focused GCC syntax admission; their real archive/provider ownership and whole
template/module definition rollback are the next source cohort, not fake factories.

The actual six-TU original_templates archive is now integrated into bootstrap.
NativeModuleData.cpp owns the real base creator and decorated-key helper; the
production ModuleFactory guards returned ordinary data through tag/vector
admission and scopes failed key lookup. Source enum values/layout stay unchanged;
internal factory parameters use raw Int admission before the unfixed enum could
make rejection inputs undefined. New generated module_data tests execute actual
base/key providers and the unchanged shared parsing machinery, not a fake factory.
Frozen466 SHA777c496d4f4ba72b61f7cffb500f5a99d6660cd274a4aae09577497e5cfc340e
builds are pending, not acceptance. Whole definition publication must also cover
the factory ModuleData suffix, borrowed override linkage and resolveNames writes
to other accepted templates' build-facility flags before later fallible image
lookup. A local allocation guard alone cannot establish that owner transaction.

Current supporting freeze470 SHAbac5627b2d9505b7bd0faf119dadcf3dcfaf6288887e67f470c23246e450ed3a
builds selected actual providers and passes49/49 related cases all3. Actual
ThingTemplate data helpers, ProductionPrerequisite lifecycle and Geometry/KindOf
live in the runtime data graph, with no duplicate source providers. Clone3 and
data-vector6 independent manifests prove exact failure/retry/terminal ownership.
NativeModuleDataTransaction restores the accepted pointer-vector backing, retires
new suffix units and covers nested commits; its Object-parser adoption is pending
until template publication/retirement and cross-template effects are transactional.
The real constructor's FileSystem/storage parents are supplied by generated rooted
fixtures; a missing-parent fixture error did not weaken actual startup admission.


Definition-publication cohort: frozen472 code paths, SHA
`949cd4331c22af754f3e0c47a90937069d4ca7a44048f475d3e1941b866b82b0`.
All configured targets build; CTest148/148 passes normal GCC14.46s/GCC
ASan-UBSan-LSan68.79s/Clang sanitizers55.73s. Matrices ran concurrently on the
normal host with leak checks enabled, no suppressions; logs:
`/tmp/zh-definition-publication-<variant>-{build,test}.log`.
Object parsing now scopes original index/list/ID/override visibility and accepted
build-facility flags around namespace/module-data ownership. Rejection withdraws
links/index/flags before deleting the candidate, then retires the module-data
suffix, then speculative names. IDs are consumed after index admission; zero is
exhausted, never an admitted ID. Duplicate non-override definitions were source-
diagnosed invalid inputs and now reject without accepted-payload overwrite.
Template copies retain both destination override link and marker: importing a
base source's false marker would prevent map-override retirement. Base module
registration scopes its complete map/namespace; the W3D extension adds an outer
scope owning even a committed base pass. Release TestArt auto-discovery is disabled
so missing-template lookup cannot create modules behind map-query transactions.
See Common/Thing/ThingFactory.cpp, Common/Thing/ModuleFactory.cpp,
Common/System/NativeThingTemplateData.cpp, Common/FileSystem.h and
EngineDevice/Source/W3DDevice/Common/W3DModuleFactory.cpp.
This matrix is support evidence, NOT whole Object parser/factory reset/registration/
startup acceptance. W3D-derived registration remains source-only: it is not compiled
by original_templates. Continue the same incomplete N2 slice with whole-runtime
link dependency tracing, protected root integration and excluded CD/DRM/online
bootstrap retirement, preserving audio/mod/LAN semantics. No new slices/commits,
framework/asset edits, retail/GPU execution, or acceptance waivers.


Whole-runtime Common cohort: canonical frozen472 SHA
`87a51815814957d238630d9006d66349a8271998d1f1dd72e0526c14f3a10f9e`;
including the four toolchain-test paths gives476-path SHA
`f6b62483a913f2883fa0212b26fc63a634d9a995c318bd955dfa0be60ecf5bbc`.
All targets build all3; CTest148/148 passes normal GCC14.45s/GCC sanitizers68.45s/
Clang sanitizers54.99s, concurrent normal-host matrices, all leak checks enabled.
Logs `/tmp/zh-gameplay-common-<variant>-{build,test}.log`.
The actual32-TU Common archive adds player/team/state-machine, definition stores
and audio metadata/requests to the root graph. A generated direct GameEngine::init
link-only probe (never executed) reduces unresolved symbols680→380:310 prior
symbols resolved or excluded,10 newly exposed. Full symbol inventories are durable
in evidence/qa/N2-runtime-link-census.json; these are implementation dependencies,
not retail assertions or missing-user-input blockers. GameSpeech.cpp was erroneously
selected initially: unlike GameAudio/GameMusic/GameSounds it is absent from the
original GameEngine.dsp and relies on dormant WPAudio. The complete -k0 compile
census identified that sole failure; removing the unretained provider preserves
actual speech events through retained AudioEventRTS/GameAudio, not an SDK shim.
GameEngine no longer creates/updates CD, starts/stops online results threads or
requires DRM fingerprint archives. Retained music availability validation and
GameLogic/network update order remain. Required official MapCache loads now use
selected MapCache-only admission and fail on rejected required input; foreign
blocks cannot mutate unrelated definition stores. Factory reset now unlinks
map-only roots from both the name index and any retained list predecessor before
retirement; the old code only corrected a deleted head, leaving non-head dangling
ownership. IDs retain the original no-reset cursor policy. These latter whole
source lifecycle paths compile but await actual factory/startup execution tests;
source contracts and support fixtures are not acceptance substitutes. Next inspect
and compile the retained logical GameClient owners as a complete batch, with device/
UI callbacks and native roots explicit; no library/framework patches, fake globals,
retail/GPU execution, new slices/support commits or milestone acceptance.


Logical-client support cohort: canonical frozen485 SHA
`73d2d250a1b6894be18ec0e40e3e3b569a206680b225d07571439148c860b8a8`;
including four toolchain-test paths gives489 SHA
`cff489dcff9183a2cf37b7f094ca91a42e1f031e655ea85a85608afaa13a4102`.
All configured targets build all3; CTest150/150 passes normal GCC14.99s/GCC
ASan-UBSan-LSan69.07s/Clang sanitizers57.11s, concurrent normal-host matrices,
all leak checks enabled. Logs `/tmp/zh-logical-client-<variant>-{build,test}.log`.
The actual28-TU logical client archive includes drawable/client updates, particles,
images/animation/campaign/credits/weather/terrain-road metadata, presentation
interfaces and abstract input. Common now owns33 TUs including actual OurLanguage.
Root link-only inventory falls380→215 (166 prior symbols resolved/excluded, one
newly exposed: TheProjectedShadowManager); full inventories remain in the census
artifact. No probe executable was run and no fake singleton was supplied.

Client portability corrections use native32-bit clocks, direct authoritative
headers, scoped loop indices and explicit bounded field offsets. Keyboard source
CP1252 was converted to UTF-8 preserving code points, with the Euro literal
explicitU+20AC and defined zero for invalid printable-key admission. Source French
layout override uses SDL's unchanged current-keymap API when video owns a keyboard;
headless uses the existing source OurLanguage value. Physical SDL text/IME input
remains later acceptance, not inferred from compilation. Native mapped-image
listing retains the top-level user gate and user→size→hand-created load order;
whole collection and per-definition candidate/index ownership are still pending.

Actual GlobalLanguage uses selected Language-only parsing into a complete offside
configuration and allocation-free field/list swap. Namespace and borrowed global
restore on every rejection. Font descriptors and supplied font-file identities
remain intact; existence is checked through configured read-only mounts, never
CWD/host font installation. N3 must consume those assets/catalog for physical text
output; the generated existence fixture is explicitly NOT a rendered font.
Configuration locale/locale-faults execute the actual provider and pass22 allocation
rejection/retry pairs plus exact terminal through three lifetimes. Tests include
late/missing-asset/foreign-block rejection and exact accepted descriptor/list
backing, registry identity, descriptor/pool/ordinary-allocation residuals.

Smudge is created/used by the W3D renderer, not original headless simulation. Its
WW3D intrusive-list/allocator header is not portable under these compilers; do not
patch that library or import a Windows SDK. Adapt that owner in game-owned N3
rendering code while preserving heat effects. Stock WWMath matrix3d.cpp also
includes D3DX: the remaining non-inline yaw/rotation/transform/static-identity
consumers need a coherent game-owned public-header adaptation, not library member
redefinitions or a framework fork. All full factory/reset/registration/native-root/
save/replay/mod/LAN/scenario acceptance remains pending in this N2 slice. No
support-only commit, new slice, asset/framework edit, retail or GPU execution.


Math/helper cohort: canonical frozen491 SHA
`1feb4732d5ac956e78d6115d0e92acabd476d557807da355e7a7503755c841e4`;
including toolchain tests gives495 SHA
`2ed260029a186425611776d15ee4b81061589c6b688d5c7786e3ca8d1a55dfcc`.
All targets build all3; CTest151/151 passes normal GCC15.09s/GCC sanitizers69.10s/
Clang sanitizers57.42s, concurrent normal-host matrices with full leak detection.
Logs `/tmp/zh-source-math-<variant>-{build,test}.log`.
NativeSourceMath.h owns free functions for all retained GameEngine non-inline
Matrix3D yaw/rotate/transform callers; unchanged public headers supply indexing,
identity construction and in-place rotation. Original arithmetic order, XY-zero
fallback, yaw-before-pitch, translation and double atan2 boundary are retained.
No WWMath member definitions, D3DX shim, implementation patches or library fork.
The remaining static Matrix3D::Rotate_Vector call is a supported public inline
operation, not unresolved backing. Forty source-derived transform cases from
nonzero backing plus160 vectors/known vertical/degenerate directions are exact and
allocation-free; support checkpoint `3cd0795892e99389` matches GCC and Clang through
three lifetimes. This is a math support checksum, NOT a gameplay checkpoint.
The fixture uses the same public `_OPERATOR_NEW_DEFINED_` build boundary as every
actual WWMath-header consumer; it does not import legacy global allocation owners.
Actual DisabledTypes/GameCommon and Line2D/Statistics source owners are linked;
whole-root unresolved inventory215→197 with18 resolved and no new dependencies.
The projected-shadow interface singleton now lives in logical RadiusDecal.cpp,
removed once from W3DProjectedShadow.cpp; actual renderer publication is still
pending, and no fake provider is introduced. Radius-decal name bounds/null provider
handling and large/nonfinite normalizeAngle progress remain observed pending
admission obligations, not accepted runtime behavior. Collection/image candidates,
full factory/reset/registration/native roots/save/replay/mod/LAN/scenario execution
remain pending. Stock bgfx/bx/bimg verification passes; no library/asset edits,
retail/GPU runs, support commits, new slices or acceptance waiver.

## Mapped-image support cohort (N2 slice03; whole runtime pending)

Canonical frozen493 SHA
`3245ad677bb5d8216ddd1a90566bb0fae0f0cd43190e559352384cfb1215b605`;
including toolchain tests gives497 SHA
`db986c20455b6816790574fcf836fe579399da9fca6cf612a808ec971b2f35a8`.
All configured targets build all3; full CTest156/156 PASS normal GCC16.46s/GCC
ASan-UBSan-LSan74.81s/Clang sanitizers61.30s, concurrent normal-host matrices,
full leak detection. Logs `/tmp/zh-image-owner-<variant>-full.log`; final builds
`/tmp/zh-image-owner-<variant>-build-v3.log`, focused cohort
`/tmp/zh-image-owner-<variant>-focused-v3.log`.

Actual Image/collection/callback execution proves stable accepted Image addresses,
offside whole-graph replacement, exact accepted nodes/string backing on rejection,
case-folded lookup without miss-key consumption, duplicate rejection, borrowed raw
texture retention, widened coordinate maxima/minima and bound+1 rejection.
Discovery tests cover the user top-level gate, user→size→HandCreated precedence,
root-before-child sorting, normalized separators and nonrecursive admission.
The complete-registry loader shares the same selected-directory helper and actual
Xfer line callback; no parallel unchecked relative offset remains there.

Three independent CTest allocation manifests are new13, replacement10 and
collection54. Each executes every ordinal, rejection/rollback/corrected retry,
exact terminal, and three same-process lifetimes; ordinary allocations, pooled
Images/name buckets/files and descriptors return exactly. Fixture correction
preserved production contracts: attach the actual user store, withdraw attachment
before generated remounts, and establish resource baselines after local parser
construction. No sanitizer suppression, test weakening or ownership substitution.

Mapped-image support replaces the earlier pending image-graph note; map-preview
copy/writer ownership, full Object/module registration, native root/client,
mod/LAN/save/replay/NOX and actual compiler-matching gameplay checkpoints remain
pending. Latest root link inventory remains197 (no new link census claimed).
Stock bgfx/bx/bimg verification passes; source libraries and assets unchanged.
No retail/GPU execution, agents, support-only commit, replacement plan or genuine
blocker; continue the existing incomplete N2 plan01/slice03 transaction.

## Startup UI/message source cohort (N2 slice03; runtime acceptance pending)

Canonical frozen510 SHA
`a8a84c0bc15a4256508ee4f4035ef9efc6b9d442d5e65f012f603599d015607a`;
including toolchain tests gives514 SHA
`0a6b69b5e57cd3251a01e8f22b424dc997c9f63abeea2c69ad24c9010dd90bee`.
All configured targets build all3; full CTest158/158 PASS normal GCC16.40s/GCC
ASan-UBSan-LSan74.79s/Clang sanitizers61.21s, concurrent normal-host matrices.
Logs `/tmp/zh-startup-ui-<variant>-{final-build,focused,full}.log`.
Failed earlier compile inventories were not followed by tests. The new fixture
uses the actual UnicodeString comparison API (allocation-free wcscmp), not an
invented implicit comparison/conversion or a weakened resource assertion.

The actual logical client archive now owns70 original-project TUs plus the real
mapped-image INI callback; Common35. Complete translators/window/layout/transition/
font/header/Shell/menu/ControlBar/Eva/InGameUI/load-screen owners compile. The
source graph test confirms every selected owner against GameEngine.dsp.
Native clocks preserve UInt32 wrapping/delay widths; complete source descriptor
types/direct includes replace PCH dependencies. Standard scoped loops, native-width
command-slot validation, owned logical window paths and explicit ASCII admission
replace VC/platform assumptions without library/header patches. Win9x header-font
selection and excluded GameSpy overlays/load screen are retired; LAN/multiplayer
and map-transfer load screens remain. Dormant profiling is included only under its
actual _PROFILE guard, not repaired inside the source library.

NativeMessageText forwards typed arguments into the existing UnicodeString
formatter using its plain WideChar pointer anchor; the UI no longer anchors C
varargs on nontrivial string objects. Generated actual formatting tests prove
wide/narrow/numeric/empty inputs, oversized rejection, complete two-allocation
fault census, exact terminal, corrected retry and three lifetimes. This is text
preparation support, NOT UI callback, translated asset-format, font, or pixel
acceptance. Original external message-format interpretation still requires content
parity evidence before presentation acceptance.

The link-only whole-root inventory197→179 resolves/excludes95 previous symbols
and exposes77 deeper dependencies, primarily real widget callbacks. Full current
and newly exposed symbol inventories are in N2-runtime-link-census.json.
No probe executable, fake singleton/provider or substitute simulation was run.

Surrounding source ownership still needs coupled remediation before startup:
Shell constructor acquires two independent raw managers; HeaderTemplate parsers
publish before complete fields and list admission; font loading can throw after
pooled candidate acquisition and borrowed font publication is callback-dependent;
window script parsing owns raw files/partial windows/global defaults and manually
memoized IDs; video/control/transition caches need complete graph rollback and
repeat lifetimes. GeneralPersona/ChallengeGenerals retain incomplete scalar/pointer
defaults. Font/UI physical publication belongs N3, while safe metadata/runtime
ownership remains N2. GameState/Xfer/Recorder encoding and native streams,
MapObject/terrain CPU providers, original factory/module execution, mod/LAN/native
root/scenario/checkpoints remain pending. No milestone acceptance or support-only
commit is inferred; continue the same N2 transaction with the complete widget and
owner cohort, not per-assert fixes. Assets/frameworks unchanged; no retail/GPU runs,
agents, new plans or genuine blocker.
# Widget source/support checkpoint (N2 plan01/slice03 remains incomplete)

Actual logical-client81 project TUs plus mapped-image callback compile all3.
Eleven Gadget owners now use native clock/input policy, character-based IME edit
completion, defined source string constants and native typed selection messages.
Shared game-owned NativeListBoxSelection operates on original ListboxData:
complete bounded selection admission, capacity+1 terminator, matching array
ownership, overlap-safe shifts and allocation-free ordered row insertion.
NativeListBoxBacking acquires replacement arrays offside; callback publication
and full window lifecycle remain pending, not inferred from metadata fixtures.

Canonical516 SHA b4d36c32d2ec7249d7e157be8278ba68e6c8bb8e37b3171179938b5f0da2318a;
with toolchain520 SHA7672caf5bdd775c7c7bb3ef2859de90024cea10592f6ab011deb634911662043.
Focused8/8 and full161/161 all3 PASS16.36s/74.47s/61.09s; frozen logs
/tmp/zh-widget-final-<variant>-{build,focused,full}.log. Root link179→113,
66 prior dependencies resolved, zero new; full durable inventory in
evidence/qa/N2-runtime-link-census.json. Generated CPU support only: no root
probe execution, native scenario or retail/GPU acceptance. Libraries/assets
unchanged, upstream provenance verified. Continue current transaction.
# Persistence and replay-value continuation (N2, same incomplete slice03)

Actual save/load/CRC/deep-CRC transports and GameState/Recorder source providers
are retained. Native transfer services route contextual map/science/upgrade/
postprocess operations through the actual GameState registration, without fake
globals or sanitizer weakening. Canonical521 support full171/171 PASS all3;
see N2-runtime-support evidence and original-engine-formats source contracts.
Root link-only inventory113 to110 resolves23 earlier symbols and exposes20
actual deeper dependencies; it is not executed or accepted gameplay.

The current source graph adds actual GameInfo and GameMessageParser from their
original GameEngine.dsp entries. Original GameMessage/GameMessageList value
implementations now reside once in GameMessageValues.cpp, separate from parent
MessageStream/CommandList dispatch. Explicit admitted player context, initialized
links/union backing, Byte budgets, pooled constructor rollback and legitimate
detach/traverse/transfer semantics are source-derived, not replacement simulation.
Retained generic player-name utilities use source UTF8 and CR/LF-to-space decode;
do not retire a required generic codec merely because its old path says GameSpy.
Actual obsolete online endpoint dependencies remain excluded.

Canonical525 focused13/13 and full174/174 PASS normal/GCC/Clang sanitizers
(17.31s/79.35s/64.55s), with all checks enabled.
Root link-only inventory110 to75:40 earlier symbols resolved,5 deeper metadata/
network interval helpers exposed. Complete unresolved symbols are in
evidence/qa/N2-runtime-link-census.json. The link-only probe was never executed.
Whole GameInfo slot/map publication, Recorder streams/header/command transaction,
real native root/map/terrain/module/scenario/checkpoints remain pending; neither
archive compilation nor value/dispatch tests accept those parent owners.

Game setup continuation: original GameInfo.cpp now owns setup/slot values and the
native value journal; GameInfoPresentation.cpp retains apparent-player/skirmish
methods; GameInfoSerialization.cpp retains source replay/snapshot methods.
All three remain in original_bootstrap. MapCompanionPaths.cpp owns the actual
shared FileTransfer lexical path definitions once; NativeMapMetadata.cpp owns
actual MapCache::findMap. No fake singleton or replacement simulation is linked.
Normal/GCC/Clang focused3 and full181 PASS on canonical533
c207ad5ad97eea1cf017a27aa6eefd99930822d5d58418cd990b6ff2f5ad6f58.
Exact setup setter/combined fault manifests are linked from support QA. This is
actual setup-owner support, not accepted GameEngine startup or Recorder playback.
The prior75 root symbols are historical inventory, not a refreshed current count.
Complete native root/providers, supplied data/scenario/compiler checkpoints and
Recorder candidate publication across the real engine-reset callback remain
required in the same incomplete slice03 transaction.

Replay preparation now compiles in actual Recorder.cpp using a separate actual
owner across GameEngine reset. Shared startup wire admission executes through the
transfer fixture; actual ReplayGameInfo payload adoption executes through the
setup fixture. Neither substitutes for executing Recorder startup, its registered
reset callback, pooled messages and RNG in the full native graph.

Native provider continuation: actual BuildAssistant/FunctionLexicon compile in
original_gameplay_common; TerrainVisual/VideoPlayer compile in original_logical_client.
NativeWindowCallback constant initialization removes accidental unconditional GUI
table startup dependencies from lookup-only linking. A durable explicit diagnostic
target original_runtime_link_probe retains actual GameEngine::init, is excluded
from default builds and is never a CTest/runtime fixture. Current normal GCC
root-link unresolved count36 supersedes historical75 for this precise configuration;
the initial provider count247 is retained as the pre-constexpr comparison.
Sanitizer whole-root linkage/execution remains pending; normal linkage alone is
not acceptance. Current source/code/build/test provenance is in support QA and
the native_root_provider_cohort entry of N2-runtime-link-census.json.

CPU map continuation: the actual MapObject values/reference/name methods and
team-validation methods are explicit extracted providers in original_gameplay_common.
The complete validate entry point lives with team validation. Linker archive
admission precedes section GC: leaving that call in the value provider pulls
SidesList RTTI, AI and GameLogic even for an uncalled validate method. The trace
selected NativeThingTemplateData correctly; template-provider ordering was not
the cause. Both GCC/Clang map fixtures now link with all sanitizers intact.
Actual ThingTemplate stack owners, borrowed publications and live override chains
execute in the seventh map family, alongside six complete value/ownership families.
The refreshed never-executed normal GameEngine::init root fails31 providers;
earlier36 is historical. No whole map/team/native-entry acceptance is inferred.

Startup callback/shared-data cohort: actual Common/BitFlags.cpp, four source
control-bar/tooltip/chat/quit callback TUs, extracted score-screen fixup and six
actual borrowed publications now compile in production archives. Shared online
chat colors retain source enum/default/field hashes without importing the absent
GameSpy SDK into basic INI/UI parsing. Three new configuration families execute
actual source metadata/admission; all configured targets build all3. Initial
attempts to compile complete Diplomacy/ScoreScreen exposed GameSpy Peer/GP header
absence; those full menus remain uncompiled, with their source behavior preserved.
Current link-only init diagnostic fails14 providers on normal GCC,227 on GCC
sanitizers and237 on Clang sanitizers. Counts differ by retained instrumentation
graphs, not acceptance or a whole-runtime reachability guarantee. Probe never runs.
Keep archive/local filesystem, protected StatsCollector, command-line/native
capability providers, diplomacy/start-position callbacks and complete sanitizer
callback/metadata roots pending. See current census/QA for exact source identities.

Native asset/mod namespace continuation: the existing production FileSystem owns
both BIG and loose providers. GameEngine now initializes this actual captured
service instead of acquiring duplicate Local/Archive platform services. Its
asset-writing authoring converter and corresponding factory requirements are
removed; a requested legacy image update fails explicitly. Native mod admission
preserves source archive precedence and publishes the complete candidate only
after attached storage admission. Full leaf write checks and captured output
leases protect mod files/directories within user storage, including scratch
destructors after owner retirement. Three new actual namespace families cover
these paths and all44 allocation failure/retry pairs plus terminal44. Normal
root-link diagnostic now fails12 symbols, GCC sanitizer224 and Clang235; all
three are link-only and never run. CommandLine binding, StatsCollector, native
capabilities, diplomacy/start spots and actual process/scenario still require
implementation. The full198 supporting matrix is recorded separately in QA;
neither the namespace fixtures nor reduced unresolved counts accept N2.

Actual CommandLine.cpp now compiles in original_runtime_common. Its original
option table is source-locked; the native mod provider replaces obsolete archive
singleton loading, and its captured mod publications roll back together on failed
admission. Three original configuration families exercise source release options,
malformed inputs and60 complete allocation failure/retry pairs across repeated
GlobalData/storage lifetimes. All configured targets build all3, related71 pass
all3; full201 remains unrun. Current never-executed root link fails11/223/234.
Protected StatsCollector and native capability/diplomacy/start-position providers
remain the normal root cohort; sanitizer callback metadata remains wider.

The actual StatsCollector now compiles in original_runtime_common. Its real
sampler/default constructor is a separate original_gameplay_common translation
unit, avoiding fixture admission of GameLogic merely to test report ownership.
Generated inputs test the actual protected logging owner, not a replacement
GameLogic. The never-executed root-link diagnostic now has 7 normal GCC,
219 GCC sanitizer and 223 Clang sanitizer unresolved symbols. Normal pending
providers are diplomacy, start spots, native capability admission and ReleaseCrash;
sanitizer-retained callback roots remain wider. This is compilation evidence,
not startup acceptance. See `evidence/qa/N2-statistics-checkpoint.md`.

Both original fatal APIs now bind the native original_core provider. It exits
failure with a fixed redacted report independently of GameLogic/GlobalData/text
and storage lifetimes, including exhausted allocation and failed stderr output.
Original Windows Debug.cpp is not admitted by the native graph; do not compile
both fatal providers. This supplies the actual release-crash path, not a returning
stub or an assertion suppression. Current link-only diagnostics remain failures:
6 normal GCC,218 GCC sanitizer,222 Clang sanitizer unresolved symbols. The
remaining normal set is capability admission, diplomacy and map start spots.
None of these probe binaries is run. See `evidence/qa/N2-fatal-checkpoint.md`.

FunctionLexicon now separates generic owner/lookup from original GUI table/default
constructor admission. Seven original table payloads are source-locked, not stubbed
or reduced. Captured metadata supports actual native/headless owner execution;
borrowed singleton publication does not load menus. Default GUI still requires
its real providers. Startup link diagnostics fail6/8/15 and default-GUI212/214/222
(normal/GCCsan/Clangsan), all never executed. These distinct scopes are not startup
acceptance. See `evidence/qa/N2-lexicon-checkpoint.md`.

Original map placement/update now binds explicit logical-client providers outside
the GameSpy-dependent menu. Transactional CPU geometry/marker state and protected
preview copying share source behavior with the original GUI entry points; their
generated fixtures execute actual MapMetaData/GameInfo/scalar visibility and
File/NativeUserStorage owners, not a replacement simulation. Scalar GameSlot
visibility/alliance is separate from translated template-name metadata; borrowed
MultiplayerSettings publication no longer loads its metadata as a side effect.
All original translated-name and GUI providers remain required. Sanitizers are
unchanged. Startup probes still fail4/6/13 (normal/GCCsan/Clangsan); the GCCsan
closure explicitly exposes missing DisconnectMenu/LANAPI RTTI. Counts are not
startup acceptance. Physical/non-null GUI bindings and complete factories remain
pending. See `evidence/qa/N2-map-preview-checkpoint.md`.

Actual Diplomacy.cpp, independent retained briefing ownership and genuine
DisconnectMenu lifetime/RTTI now compile in original_logical_client. The excluded
GameSpy buddy provider is not admitted; LAN/skirmish paths retain source behavior.
TheLAN borrows the existing abstract interface, avoiding artificial concrete
Windows RTTI admission without supplying a native transport. All configured
targets build with unchanged instrumentation. Root link-only diagnostics now
fail1/1/7 (normal/GCCsan/Clangsan): capability admission on all, plus Clang's
IME/warning/terrain/skybox/hosting providers. None is executed. A pending actual
animation fixture is EXCLUDE_FROM_ALL, not CTest or startup evidence: constructor
fault/retry terminal8 passes normal/GCCsan, but Clang's native closure does not
link yet. Registered windows and nonnull diplomacy GUI retirement remain pending.
See `evidence/qa/N2-diplomacy-checkpoint.md`.

Shared script visual state now has explicit common entry points and a required
TerrainVisual operation rather than dependencies on obsolete W3D free symbols.
GameClient uses its required device IME factory; source W3D forwarding remains,
not native input acceptance. Native warnings bind actual GameText/owned UTF-8 to
public SDL3, with explicit failure and affirmative-only source music retry.
The excluded GameSpy-only in-game hosting diagnostic is removed rather than
replaced with fake online status. All configured targets build all3. Current
root and pending animation link diagnostics fail the same one native capability
symbol on all3; none is executed. Actual skybox state and warning ownership pass
focused checks all3, but dialogs, nonnull terrain/IME/GUI and full startup remain
pending. See `evidence/qa/N2-native-boundaries-checkpoint.md`.
