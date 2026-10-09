# Original engine findings for the clean restart

This is a source reference, not a claim that the new implementation passes tests.
The baseline is `0a05454d8574207440a5fb15241b98ad0b435590`. Prior implementation
and evidence are archived; no old milestone acceptance transfers automatically.

## Rendering requirements verified during restart assessment

- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` declares viewport
  clear, transforms, packed layouts, texture stages, copy rectangles, 2D/cube/3D
  textures and render targets. Translate actual call-site semantics; this is not
  a mandate to emulate every D3D8 API.
- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp`
  writes only stencil bit `0x80` for potential occluders while preserving player
  color bits. Stencil read masks and write masks are different requirements.
- `.../GameClient/Shadow/W3DVolumetricShadow.cpp` shares stencil meaning with
  marker/occlusion rendering. Test their combination, not independent shadows.
- `.../GameClient/W3DSmudge.cpp` acquires/copies background scene content and
  resets WORLD/VIEW transforms for its screen-space effect. The retired port's
  simplified atlas/black-fan fixture is not a visual parity reference.
- `.../GameClient/Water/W3DWater.cpp` selects multiple textures, texture
  transforms, shader variants and reflection behavior. Preserve the selected
  source path rather than substituting one generic transparent surface.
- `.../WW3D2/texturefilter.cpp` owns DEFAULT/BEST filter resolution. Initialize
  actual profile tables in fixtures; zeroed tables can invent no-mip behavior.

## Reusable ownership cautions

Original global allocation is replaced by memory-pool services. Pair ownership
at allocation and deletion, including library temporary/nothrow paths. A failed
constructor does not invoke its destructor; guard acquired references before
later throws. Native resource handles may deduplicate while acquiring additional
reference units. None of these facts require a private backend inventory API.

Descriptor mip count is logical metadata, not proof of initialized GPU backing.
Packed source vertex slots and source-selected coordinate transforms are shader
protocols; transport names are not semantic authority. New stock shader layouts
may differ when explicit translation preserves those semantics.

## Pending evidence

N1 must verify all renderer claims on a pristine stock dependency build. N2 must
reinspect native persistence layouts and original allocator service boundaries
before implementing them. Do not copy historical ABI assumptions into file
formats; derive fixed-width encodings from the original reader/writer pair.

## Fresh stock-bgfx source findings (N1 semantics; whole milestone pending)

Official bgfx `cca91681c953d2de9531197b0f580c866ffaa775` was acquired directly
from upstream into a fresh build tree. Public `setTexture` supports a mip/layer
view and quarter-mip `lodMin/lodMax` limits. This replaces the retired custom
base-level flag mechanism. `tests/renderer/qualification.cpp::mips` and
`spatial_filters` prove authored ranges and distinct min/mag spatial selection
on the RTX Vulkan device. This is not yet full original filter-table integration.

Upstream merged stencil write masks in PR3821 (merge
`7b3834644276012ab6643c45965cdfcf9b6ee457`, API148). The second public
`setStencil` argument carries the write mask in its RMASK field; the first
argument carries the compare mask. The public signature's prose is terse;
the official merged change establishes the intended calling convention:
https://github.com/bkaradzic/bgfx/pull/3821
The generated test uses public headers/macros only and tests 0x80/0xff/0x00.
No copied private helper or dependency modification is used.

The new upstream revision uses `TextureRegion` for blit/read and no longer
exports the old texture-blit/readback capability bits. Validate the required
formats/usage with public `isTextureValid` and actual readback tests. This is an
ordinary game-side API adaptation, not a reason to modify dependency sources.

## Projector and material representation

`GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/matrixmapper.cpp`,
`Compute_Texture_Coordinate` computes S/T from ViewToPixel rows 0/1, Q from
row 3. `Apply` installs rows 0/1/3 with PROJECTED|COUNT3 for perspective;
orthographic mapping uses COUNT2 without division. `texproject.cpp`,
`Pre_Render_Update` composes projector projection, inverse projector transform
and camera transform. `Init_Multiplicative` uses ZERO/SRC_COLOR blending,
LEQUAL and disabled depth writes. `Configure_Camera` insets the capture viewport
by one texel. These are game shader/state semantics, not a required raw D3D ABI.

The generated `projected_shadows` case uses explicit stock Mat4/STQ transport,
fragment division, a generated target and multiplicative blend. Its varying-Q
control distinguishes division from an affine substitute. Full original camera,
depth-gradient and intensity integration remains N3 work. Commented ZBIAS lines
in `W3DVolumetricShadow.cpp` are not an active depth-bias requirement; do not
promote them into a mandatory public API gap without a reachable caller.

`GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shaders/terrain.nvp`
mixes texture1/texture0 with diffuse alpha then multiplies by diffuse.
`fterrain.nvp` multiplies both textures and diffuse. Owned `fs_terrain.sc` and
`fs_flat.sc` implement those equations with explicit stock inputs; generated
pixels cover nontrivial color/alpha, without copying a prior shader ABI.

`WW3D2/textureloader.cpp` has explicit DXT1/3/5 and A8R8G8B8 paths. N1's
`source_formats` covers generated BC1 and BGRA8 uploads, not a complete format
decoder or all asset formats. Engine decoding and format dispatch remain N2/N3.

See `evidence/qa/N1-stock-renderer-semantics.md` for current configuration and
acceptance boundaries. None of these tests load proprietary assets.

## Renderer loading and capacity findings

Fresh source census and formulas are in `docs/renderer-workload-census.md`.
Flat terrain represents independent 16-cell regions; grouping their GPU storage
does not require changing source data or simulation. Stock bgfx's 4096 handle
tables are shared; 32-layer texture arrays and grouped vertex/index storage
preserve tile-selected layers and ranges while reducing 4096 handles to 128.
The generated mixed-frame test physically samples every terrain layer.

Public `createTexture2D` with initial memory creates immutable storage. Providers
that update must create with null memory then initialize all declared layers.
`tests/renderer/lifecycle.cpp` covers initialization, changed texture content,
dynamic vertex/index updates, canceled candidates and corrected retry. The
upload holder ledger owns plain C++ allocation in these generated fixtures; the
original pooled process is not linked and needs its own allocation boundary in N2.

`bgfx::touch` is a public dummy submit and consumes a render item, including a
clear-only view. Draw-budget admission must include touches alongside visible
draws; copies have their separate budget. Public `numDrawCallsPeak` is requested
demand before drops. Use distinct completed pixel witnesses to prove accepted
draws instead of treating the peak alone as completion. Rotating per-frame
statistics must not be attributed to an unrelated queued owner.

`bgfx::SwapChain` now needs an explicit color format for the native-window path;
its default Count is neutral, not an inferred host surface format. Fresh SDL3
fixtures select BGRA8/D24S8, Vulkan-capable windows and public native properties.
Three contexts/four resize sizes physically ran on the host Wayland environment.
Keep one balanced public SDL Vulkan-loader acquisition around every window/device
generation, then unload before SDL_Quit. Implicit per-window loader ownership
produced a 224-byte residual in both compiler sanitizers, including a native
initialization-only control. Loader diagnostics mapped return addresses into
unloaded NVIDIA driver mappings. Explicit SDL_Vulkan_LoadLibrary/UnloadLibrary
service ownership passes both controls with all leak checks enabled, without
pinning driver libraries or modifying SDL/bgfx. The source contract is documented
in installed `SDL3/SDL_vulkan.h`; the exercised owner is `lifecycle.cpp::Video`.
Final normal-host, GCC sanitizer and Clang sanitizer matrices passed 18/18 each.
See `evidence/qa/N1-stock-renderer-capacity-lifecycle.md` for attributable
qualification acceptance and explicit original-integration limits.

## N2 source investigation (implementation and acceptance pending)

- GameMemory.cpp replaces global new/delete, has MEM_BOUND_ALIGNMENT=4 and
  suppresses shutdown after pre-main allocation. GameMemory.h class-pool glue
  caches factory-owned pointers in function statics, which cannot survive an
  actual factory retirement. N2 removes global interposition and audits retained
  explicit pools; standard library allocations are not game pool allocations.
- AsciiString.h uses InterlockedIncrement/Decrement through a `long*` cast into
  a 16-bit refcount next to capacity. This is not a portable atomic protocol on
  Linux LP64. Inspect string backing and reference ownership together.
- GameEngine.cpp::init deletes a patch-era data archive. This is incompatible
  with supplied read-only assets and must be removed before runtime audits.
- Win32BIGFileSystem.cpp::openArchiveFile reads count, offset and entry size as
  32-bit big-endian via ntohl; names are NUL terminated. Header-size endianness
  is not established by its unconverted diagnostic-only read. Original bounds
  checks are insufficient; use actual file length and bounded index admission.
- GameText.cpp CSFHeader has six Windows 32-bit Int fields; labels and string
  lengths are Int. parseCSF reads inverted Windows WideChar units and then
  complements them, optionally reading STRW wave names. Decode file UTF-16
  independently of Linux wchar_t; exact validation/writer pairing is pending.
- RandomValue.cpp has separate six-word logic/client/audio seeds and unsigned
  additive carry state. Each integer-range family calculates `hi-lo+1` in signed
  arithmetic before conversion; inspect all three together for full-width ranges.
  CRC's release header uses x86 assembly; its debug implementation documents the
  byte-wise left-shift/high-bit carry algorithm. Keep the byte protocol unchanged.

These findings are source inspection, not N2 runtime acceptance. Plans and tests
will link verified portable implementations here as each coherent slice passes.

### Portable core representation and ownership (N2 slice 01)

The retained original RNG carry algorithm has three independent six-word seeds.
Seed `0x12345678`, inclusive integer range 0..1000, yields
`823,209,704,299,632,971,682,948,750,591,960,331`. The original full signed-width
range produces zero unsigned delta, returns the high endpoint and does not
advance the seed; this convention is retained. Range arithmetic now uses
defined unsigned/widened arithmetic in all three families. CRC bytes
`ff 00 80 7f 01 aa` produce 9864, independently of chunking. See
`RandomValue.cpp`, `Common/crc.h` and `tests/original/core.cpp`.

The published Trig.cpp unconditionally selected DEFAULT_TRIG; its active
sin/cos/tan/acos/asin path is retained with standard float overloads, not the
inactive integer tables. BaseType fast floor/ceil now implement their documented
mathematical operation without the legacy epsilon, which misrounded sufficiently
close fractional inputs. Float-to-Int conversion checks finite signed 32-bit
representability before casting. Internal Linux WideChar is native wchar_t;
this is **not** a CSF wire-format decision. File UTF-16 decoding remains slice 02.

Ordinary, array, nothrow and aligned C++ allocation is standard-owned, not
interposed by GameMemory. Explicit game pools use max_align_t-aligned blobs,
preallocated slot metadata and raw-address admission. Each DMA has its own
allocation ledger even when named subpools are shared. Factory teardown releases
DMA units before pools. Class glue reacquires its pool from the current factory;
constructor failure returns its acquired slot. Pool/factory/DMA and holder owners
are noncopyable. The obsolete Win32 custom-new/checkpoint declarations are
retired, so Debug and Release share the same native class layout. No adopted
framework source is changed. See GameMemory.h/.cpp and the pools/repeat families.

ASCII and Unicode backing uses standard allocation and atomic 32-bit reference
units, independent of pool shutdown. Mutation snapshots borrowed source data
before replacing backing, including self/interior aliases; all capacity slots
are initialized. Assignment retains its incoming unit before releasing the old
one. Format forwarding does not use a non-POD variadic last parameter, and
capacity/null/truncation rejection preserves accepted values. The strings family
exercises 70,000 real aliases and concurrent independent copies. See
StringStorage.h, AsciiString.cpp, UnicodeString.cpp and tests/original/core.cpp.

These are core-fixture claims only; GameEngine/GameLogic startup, rooted data,
CSF decoding and retail integrity/completeness remain unaccepted N2 work.

### Rooted data owner investigation (N2 slice 02, acceptance pending)

`FileSystem.cpp::openFile` tries local access before archive access.
`Win32BIGFileSystem.cpp::loadBigFilesFromDirectory` iterates the case-insensitive
FilenameList in ascending order and calls loadIntoDirectoryTree with overwrite
false by default; Zero Hour archives are loaded before Generals archives.
Native mounts preserve loose-before-archive and first ordered archive/root wins.
Original GameEngine patch startup removed `Data/INI/INIZH.big`, an obsolete
duplicate shipped alongside the proper root archive. Native selection excludes
that one source-established obsolete archive from indexing, while retaining its
physical bytes for the user. Simply removing DeleteFile while indexing the old
duplicate would silently change intended definition precedence. The roots
fixture puts conflicting generated definitions in both archives and proves the
proper one wins without deleting the duplicate. Other nested archives retain
the original recursive discovery contract.
BIGF count/entry offset/size are big-endian 32-bit; header size remains unused as
in the original reader. Native indexing bounds records against actual length and
the earliest payload, not diagnostic header-size assumptions. BIG4 has not been
established as an original retained-provider contract.

`LocalFile.cpp::convertToRAMFile` is **consuming** on successful conversion: it
transfers delete-on-close policy to the RAMFile, then retires the original File.
On a failed boolean snapshot it returns the unchanged original File. RAMFile's
own conversion returns itself. NativeDataFile must honor that owner protocol;
callers cannot close a successful conversion's retired source. Snapshot failure
restores the borrowed cursor; candidate array ownership is guarded before reads.

GameText CSF integer tags are loaded in Windows little-endian order: numeric
CSF_ID 0x43534620 appears as bytes `20 46 53 43`. LBL/STR/STRW tags follow the same
rule. A header has six four-byte words; string text is **inverted UTF-16LE units**,
not native wchar_t. Only each label's first alternative text/speech is retained,
but all alternatives must be decoded/validated. GameText stripSpaces collapses
spaces and trims at text/newline/tab boundaries, preserving newline/tab bytes.
Lookup is case-insensitive; getStringsWithLabelPrefix is deliberately
case-sensitive (`strstr`). Generated tests must not demand a case-insensitive
prefix API that the source never provided.

LanguageFilter's langdata.dat uses little-endian 16-bit units XOR 0x5555, with a
raw 0x0020 delimiter outside the XOR payload. Native wchar_t and WEOF must not
be read/cast as this file representation. Decode bounded words before publishing
the replacement map; preserve the accepted filter on truncation. Linked native
tests and complete slice acceptance are recorded separately below; these source
findings do not establish full original startup or scenario completeness.

### Rooted data representations and owner proofs (N2 slice 02)

Zero-length BIG members can use offset zero. The source reader accepts the range
as metadata; no bytes are read for that member. Empty members must not lower the
index/nonempty-payload boundary. All offsets still lie within actual archive
length; every nonempty range remains bounded after a preceding empty member.
`NativeFileSystem.cpp::readBig` and `data.cpp::archives` cover zero/interior
empty offsets, late nonempty rejection and accepted-owner preservation. The
read-only supplied-data audit found this convention; initial admission rejected
it before any consumer ran. Corrected admission reached mask63/stage4, and the
wrapper's complete before/after SHA-256 and metadata snapshots were unchanged.
This establishes indexing, basic required-family presence and actual GameText
initialization, **not** GameLogic startup, every scenario or visual/audio parity.

Native root traversal uses guarded POSIX directory handles and `openat`/
`fstatat` with no-follow admission. The fault sweep exposed a libstdc++ recursive
directory-iterator allocation terminating inside its implementation rather than
unwinding. No library is patched: the game-owned walker keeps all fallible
allocation outside library no-throw directory helpers, guards each directory
before container growth, and publishes only a complete replacement index.
Canonicalizing an explicit supplied root may resolve the user's root symlink;
internal symlink files/directories are not traversed. See `fault_mount` and exact
descriptor/standard-allocation/pool residual checks in `tests/original/data.cpp`.

`.str` and `map.str` use labels followed by a quoted text and `END` (case
insensitive), with outside-line `//` comments. Physical line breaks in a quote
become spaces; source `\\n` and `\\t` escapes produce newline/tab, and other
escaped bytes retain their low eight bits. Raw bytes are not reinterpreted as
UTF-8. Speech identifiers retain alphanumeric/underscore characters; a final
digit gains the source's `e` suffix. Source `stripSpaces` is shared with CSF.
`StringCatalog.cpp` is the bounded extraction of original `readToEndOfQuote`,
`translateCopy` and `parseStringFile`/`parseMapStringFile`; `data.cpp::textManager`
tests escapes, multiline values, speech, EOF, map filtering and rejected retry.
The original `getStringCount` added 500 reserve rows to the logical lookup count;
the native owner indexes only parsed rows, fixing empty-label lookup pollution.
Catalog arrays and their pointer lookup are prepared together before publication,
including every language-filter call. Missing-label teardown is iterative.

Native `UnicodeString::nextToken` cannot copy its remainder using the source's
hardcoded `len*2` bytes. That bug was exposed by actual LanguageFilter calls,
not slice01's null-token test. Both ASCII and Unicode token owners now prepare
token and remainder before either publication; core tests include odd-length,
non-ASCII and shared-backing transitions. `fault_map` covers allocation failures
inside filter callbacks as well as all catalog/lookup preparation. Native scalar
storage is separate from fixed UTF-16 file units; non-BMP filter replacement
width remains an explicit source-semantics question for full text integration.

`INICore.cpp` extracts original line/token/field dispatch; `INI.cpp` retains the
complete gameplay block table for slice03, without fake parse callbacks. Native
numeric scanners deliberately retain source `sscanf` prefix acceptance (`12x`
parses as 12, `25%` as .25, unsigned `-1` as UINT32_MAX), while rejecting overflow
and nonfinite reals before publication. Each INI owns its token/transfer state.
The caller owns offside semantic candidates. Rejection retires the parser's File
before reuse, and allocation exceptions propagate without another diagnostic
allocation. `INIException` owns independent diagnostic storage across copy/move.
`data.cpp::iniFields` and `fault_ini` prove real FieldParse dispatch and retry,
not complete GameLogic definition admission. Dynamic ScienceType IDs and File
seek admission use consistent fixed signed-32-bit enums across retained headers;
width/alignment checks do not assert legacy full-class MSVC ABI compatibility.

`RAMFile::open`/`openFromArchive` guard arrays before virtual reads and restore a
borrowed cursor on boolean rejection and thrown reads. The generated throwing
provider exercises both paths plus same-candidate retry. All retained native File
providers use defined invalid-seek admission without changing accepted cursors.
Each independent allocation sweep is a separate CTest batch, exhausts contiguous
ordinals through its exact first successful terminal boundary, and repeats three
times in one process. The fixture implements every ordinary/array/nothrow/aligned
allocation/deallocation pairing; production never uses this interposition.

See `evidence/qa/N2-original-data.md` for acceptance configuration and boundaries.
No conversion cache or write is introduced here. XDG user/cache writers, rooted
MemoryPools.ini overrides, original FileInfo timestamp/cache integration, complete
INI block dispatch and actual GameEngine/GameLogic reachability remain slice03
work; zero diagnostic timestamps are not map-cache compatibility acceptance.

### Original startup namespace and FP services (N2 slice03 support, pending acceptance)

`NameKeyGenerator` has separate exact-case and case-insensitive hash/lookup paths.
Both originally acquired a pooled Bucket and consumed its ordinal before fallible
string assignment. Native candidate guards cover both paths; strings and pool
units settle on failure, and successful insertion alone advances the ordinal.
The source NAMEKEY_MAX bounds the default namespace; smaller explicit capacities
exercise the same admission owner without allocating millions of fixture keys.
The generation token is process-monotonic, separate from assigned key IDs; it
does not change external names or accepted ordinal order. StaticNameKey resolves
again after namespace reset/replacement, including at an identical raw address.
All NameKeyType definitions/opaque declarations use consistent signed32 storage.
`tests/original/runtime.cpp` covers independent owners, bounded admission, reset,
same-address replacement and exact/lowercase allocation sweeps [0,2), terminal2,
each repeated three times. Four support cases pass normal (.09s), GCC sanitizer
(.89s) and Clang sanitizer (.59s); this does not prove GameLogic initialization.

The active original `GameLogic.cpp::setFPMode` resets FP state, selects nearest
rounding and 24-bit x87 precision. Its CHOP comment is stale; the actual CHOP
assignment is commented out. `FPUControl.cpp` uses public C floating-point reset/
rounding services (including the native SSE environment) and installed glibc's
x86-64 `fpu_control.h` for source x87 precision. The public header explicitly
warns that its control-word macros alone do not affect SSE; they are not a
replacement for `fesetenv`/`fesetround`. Generated tests enter all four rounding
modes with pending exceptions, then prove reset, actual control bits and nearest
ties-to-even integer conversion. Full original simulation/checkpoints remain
pending; FP-control tests alone do not establish whole-game numerical parity.

New map/chunk inspection (pending port/test evidence): `DataChunk.cpp` writes
`CkMp`, a signed32 table count, one-byte symbol lengths, symbol bytes and UInt32
table IDs. Chunk payload dictionaries pack a table ID in the upper24 bits and a
data-kind byte below; that table ID is **not** a raw runtime NameKey ordinal.
Unicode output writes a UInt16 **UTF-16-unit** count and Windows two-byte units;
native wchar_t output would corrupt files. CachedFileInputStream replaces backing
without a guard and may fall back to compressed bytes after decoder failure;
its buffer/cursor/decompression owner and parallel read/write/TOC paths must be
ported together before map acceptance. `docs/original-runtime-source-graph.md`
records full startup/frame dependencies and pending build/type surfaces.

`Dict.cpp` is an in-memory owner, **not** the map wire representation. Native
typed values replace the original unconstructed string/scalar objects inside
`void*` storage and uint16 sharing. Sorted NameKey enumeration, all five value
types, COW aliases, type replacement, missing defaults and MAX_LEN32767 remain.
Immutable typed payloads are independently shared; their noexcept handles make
unique-owner shifts/removal allocation-free after guarded capacity reservation.
Shared mutation clones the vector offside. No global allocation interposition
or adopted-library modification is used. `runtime.cpp::dictionary` exercises
70000 aliases, Unicode scalars, self/borrowed copy and exact cardinality maximum,
maximum+1 rejection and retry. Six independent fault families cover shared
insert/replace/remove/copy and unique insert/replace: contiguous ordinals through
terminals4/3/2/3/2/1, each repeated three times with exact immediate standard
allocation residuals and accepted-value checks. Seven cases pass normal(.18s),
GCC ASan/UBSan/leaks(1.93s) and Clang ASan/UBSan/leaks(1.12s). This is supporting
owner evidence; original GameLogic execution and map reader/writer acceptance
remain pending.

`CachedFileInputStream.cpp` now owns complete rooted read/decode candidates before
replacing backing/cursor. Missing/empty/malformed reopen preserves the accepted
stream; read range checks use subtraction rather than overflowing count+cursor,
and beyond-end seeks retain the original explicit clamp. Native map codec
framing is four-byte `EAR\0`/`ZL1\0`..`ZL9\0` followed by UInt32LE output size.
RefPack then has a big-endian 10fb/11fb/90fb/91fb header, 3/4-byte size, optional
preceding size, and short/int/very-int/literal/terminal tokens. Distances include
one, overlaps copy forward, and terminal tokens append0..3 bytes. The original
decoder ignores bytes after the terminal. The bounded game-owned decoder derives
these forms from `Compression/EAC/refdecode.cpp`; whole token/count validation
precedes declared output allocation. Zlib likewise validates complete output
count into bounded scratch before allocation; both passes guard public inflate
state. Public zalloc/zfree callbacks pair nothrow byte-array owners without
throwing through C callbacks. System zlib1.3.2 is unchanged, and its header stays
isolated from the original global signed `Byte` typedef rather than macro-renamed.
Original NOX/EAB/EAH codecs remain explicitly unsupported pending native ports;
they are never silently treated as raw maps or acceptance of full map support.

`tests/original/maps.cpp` proves all four RefPack header forms, literal boundaries,
all backreference forms/maxima, overlaps, truncated prefixes, bogus huge output
and zlib levels1..9/checksum/size rejection, plus source trailing-byte acceptance.
Three actual rooted stream allocation sweeps have contiguous ordinals and exact
terminals4/5/8 (raw/RefPack/zlib), each repeated three times. Immediate native File
pool units, POSIX descriptors, standard allocations and accepted bytes/cursor
settle before same-owner retry. Five cases pass normal(.18s), GCC full sanitizer
checks(1.37s) and Clang full sanitizer checks(.87s). This does not establish the
chunk reader/writer, remaining compression codecs, scenarios or GameLogic entry.

The native TOC/input extraction is `System/DataChunkInput.cpp`. A chunk header
is UInt32LE ID, UInt16LE version and Int32LE payload size: **10 bytes**, not the
old CHUNK_HEADER_BYTES4 comment/constant. Check complete extent against the
borrowed stream and every active parent before stack publication. Registered
parsers select the newest matching label/parent, and source dispatch passes the
parse-call userData (its stored registration field was unused). Unknown chunks
skip their bounded payload; valid zero-byte chunks must not disappear merely
because opening their header reaches EOF. All nested reads decrement each
parent. Compound-read guards restore that complete ledger and borrowed cursor;
failed cursor repair poisons reads until a verified reset. Array reads publish
outward bytes only after a successful offside physical read.

TOC identity remains full UInt32 metadata; packed dictionary identities expose
only24 bits, validated separately from the low kind byte. TOC max ID does not
permit wrapping nextID. Mapping/InputChunk/UserParser pooled fields are explicitly
initialized and constructor/helper acquisitions guarded. TOC append is offside,
rejecting duplicate names/IDs/invalid lengths without retiring the accepted table.
`tests/original/chunks.cpp` covers recursive dispatch/repeat, parser false/throw
and reset/retry, truncated records, malformed packed types/strings, real UTF-16
surrogates, throwing reads and failed cursor repair. TOC and nested value-read
sweeps exhaust ordinals[0,7)/[0,12), exact terminals7/12, three repeats, with
immediate graph/pool/cursor/all-parent checks. No writer finalization or actual
ScriptEngine/GameLogic acceptance is inferred. The entire current supporting
cohort passed38/38 normal and both sanitizer builds; configuration/digest and
remaining acceptance gates are in `evidence/qa/N2-runtime-support.md`.

The native writer in `System/DataChunk.cpp` now buffers a complete candidate and
requires explicit fallible `finish()`; its destructor only releases ownership.
All three excluded WorldBuilder callers now explicitly finish, but the editor
is not built or accepted. The old shared `_tmpChunk.dat` and destructor callback
are gone. Byte/word/string/dictionary encodings retain the reader's fixed widths,
and nested size placeholders count complete child headers. Full TOC IDs and
packed24-bit IDs have separate admission; packed maximum+1 rejects before a
Mapping acquisition. Any fallible admission or output failure poisons an
otherwise commit-ready writer. One borrowed sink call does not establish atomic
filesystem publication. `tests/original/output.cpp` proves independent bytes,
roundtrip, native capacity boundaries, depth2000, throw/short sink and complete
fault ordinals[0,18), terminal18, three repeats. The superseding supporting
matrix is42/42 normal/GCC/Clang in `evidence/qa/N2-runtime-support.md`; actual
GameEngine/GameLogic remains unaccepted.

EAB uses46fb/47fb, BE24 decoded length (47fb skips another BE24 field), a clue
byte, byte branch count, and node/left/right triples. Branches expand left then
right; clue/nonzero is an escaped literal and clue/zero terminates. Bounded
native decoding validates the complete acyclic graph, rejects duplicate/clue
branches and clue children, and caps expansion counts before output allocation.
Unused large acyclic expansions need no storage; selected expansions must fit.
This follows `Compression/EAC/btreedecode.cpp` and `btreeencode.cpp`; no source
codec is modified. The original recursive helper would chase uninitialized
children if a clue appeared as a child, so that malformed graph is not accepted.

EAH uses MSB-first bits,30fb..35fb/b0fb..b5fb, BE24/32 lengths, optional skipped
preceding length, clue byte, encoded canonical length counts, and leapfrog symbol
ordinals among unused bytes. Encoded integers use unary leading zeros followed
by width2+zero-count payload and base(2^width-4). Complete trees have at most256
symbols and16-bit codes. The original decoder's arrays had16 entries while
indexing length16; the native game owner explicitly covers1..16. Clue escapes
encode repeat-previous runs, explicit bytes and EOF. Variants32/33 and34/35 apply
byte-modular first/second integration, including their b-prefixed32-bit forms.
`huffdecode.cpp`/`huffencode.cpp` establish these semantics; native two-pass
validation allocates no declared output until complete EOF/count acceptance.

`maps.cpp::legacyCodecs` covers both pair headers, maximum-depth graphs, all12
Huffman headers, full256-symbol and16-bit tables, wrapped leapfrog order,
run-number boundaries, malformed graphs/tables, prefix truncation and exact
output counts. Rooted pair/Huffman fault sweeps exhaust ordinals[0,5), terminal5,
three repeats with accepted backing/cursor, descriptor and pool preservation.
The expanded eight-case map cohort passes normal(.29s), GCC ASan/UBSan/leaks
(2.18s) and Clang ASan/UBSan/leaks(1.42s). This supersedes EAB/EAH pending status
above, not full startup/map acceptance. NOX still lacks an adopted provider;
EA's published tree intentionally omits LZH-Light, and a discovered public
implementation is only an investigation candidate, not qualified acceptance.

Rooted MemoryPools.ini retains the source plain three-field row format, first
case-insensitive compiled name match and last valid row wins. Explicit positive
class counts bypass profiles; default counts round up to multiples of4 with
minimum4, including source zero/negative inputs. Unlike overflowing scanf/rounding,
native known-name integer overflow rejects the complete offside profile. Missing
profiles restore immutable compiled defaults, startup/shutdown release profile
backing, and applying a new profile never changes an already-live named pool.
See `MemoryInit.cpp`, `NativeMemoryProfiles.cpp` and
`startup_services.cpp::{profiles,profileFaults}`. The original pre-main executable
directory scan is not restored; future actual startup must call this rooted
service after mounting and before constructing definition pools.

LanguageFilter's original mask loop counted UTF-16 units before unHaxor. Native
scalar strings now emit two stars per astral scalar in a filtered token, retaining
delimiters, repeated-token search and original normalization. A complete offside
wstring is published only after every token/replacement succeeds; invalid native
scalar or allocation failure leaves the caller's accepted line intact. Authored
XOR16 word bytes are unchanged. Source/test: `LanguageFilter.cpp::filterLine`,
`startup_services.cpp::filtering`; exact fault terminal71, three process repeats.

`Win32LocalFileSystem.cpp::getFileInfo` uses FILETIME100ns since1601;
`Win32BIGFile.cpp::getFileInfo` inherits that archive timestamp and substitutes
member size. Native FileInfo derives the epoch with standard chrono, validates
POSIX nanoseconds and UInt64 multiplication, and bit-preserves high/low32 words.
Loose open/info use current physical size, not a stale mount-size snapshot.
`MapUtil.cpp::MapCache::addMap` currently reuses same-size/nonzero-CRC metadata
without even comparing timestamp; timestamps alone cannot prove cache freshness.
That whole cache owner remains pending content/version-keyed integration.
`startup_services.cpp::timestamps` proves exact generated provider values and
missing/retired metadata preservation. These service additions and codecs have
50/50 normal/GCC/Clang supporting evidence in `evidence/qa/N2-runtime-support.md`,
not actual GameEngine/GameLogic acceptance.

### Native user storage and conversion envelope (supporting acceptance)

Game-owned `NativeUserStorage.cpp` follows the
[XDG absolute-path/fallback rules](https://specifications.freedesktop.org/basedir/latest/):
relative or empty overrides are ignored, data/cache fall back to
HOME/.local/share and HOME/.cache, and the application leaf is
`cnc-generals-zero-hour`. The mounted FileSystem owns canonical input roots;
storage rejects those roots and descendants, including prospective-path aliases,
before creating directories. Descriptor-relative no-follow traversal creates
missing directories0700 without changing existing permissions. These services
do not yet establish the actual GlobalData startup path.

`NativeAtomicOutput` owns a0600 exclusive candidate in the target directory.
Its destructor removes only unpublished candidates, without callbacks. Explicit
commit syncs/closes candidate backing, runs all fallible pre-publication hooks,
then performs direct POSIX rename; successful rename cannot be rolled back by
pretending the old target remains accepted. Directory-sync failure reports
`PublishedDurabilityUnknown`. DataChunkOutput::finish authors the candidate;
only a separate commit publishes it. Source/test:
`NativeUserStorage.{h,cpp}`, `tests/original/storage.cpp::{atomic,ioFaults}`.

`NativeConversionCache.cpp` uses public system OpenSSL3
[EVP digest APIs](https://docs.openssl.org/3.0/man3/EVP_DigestInit/), not a local
crypto implementation or dependency patch. Filename identity is SHA256 of
LE32 converter kind, LE32 converter version, and SHA256 of unchanged source
bytes. The84-byte envelope is `ZHC1`, kindLE32, versionLE32, source digest32,
payload lengthLE64, payload digest32, then payload. Map decoding uses kind1,
version1. No asset is converted in place or distributed as required modified
content. Complete header/count/source and streamed payload digest validation
precede declared backing allocation; the loaded candidate is rechecked before
publication. Missing, malformed or unavailable storage is an ordinary miss.
The converter runs exactly once outside cache-error handling, so semantic
conversion failure propagates rather than being mistaken for optional storage
failure. Cache write failure preserves the accepted in-memory conversion.

Actual CachedFileInputStream accepts a borrowed storage service and uses the
bounded original-map decoder on misses, with offside backing/cursor publication.
`maps.cpp::{cachedStream,faultCached}` and seven `storage.cpp` families cover
corruption, version and same-length source changes, unwritable fallback,
abandonment, poisoning from commit-ready owners, partial writes, every I/O hook,
and retry. Allocation terminals are9/11/21 for atomic/cache hit/cache miss and
16/31 for rooted cached map hit/miss, with three same-process repeats. Optional
fallback successes do not terminate ordinal coverage; the terminal must be an
untriggered run. C++ injection does not claim OpenSSL-internal C malloc coverage.
The frozen60-test normal/GCC/Clang sanitizer cohort passes with leaks enabled;
see `evidence/qa/N2-runtime-support.md`. Whole MapCache metadata integration,
actual startup ownership and original scenario execution are still pending.

### Dynamic engine ID representation (supporting acceptance)

`Common/EngineIDs.h` now owns Object/Drawable/Formation/ParticleSystem/Production/
Waypoint definitions. All preserve source named values: three generic invalids,
particle/production invalid0, three force-size values0x07ffffff, waypoint invalid
0x7fffffff. Each now uses explicit UnsignedInt rather than an unfixed enum:
runtime-assigned raw32 values are defined before any owner admission check.
Every retained header and source opaque declaration uses the same representation;
`test_original_header_paths.py` checks unique definitions and complete declaration
consistency, not just the fixture's included subset. `ids.cpp` compares six enum
widths/alignment and representative nested member offsets against preceding
declarations, and exercises0,1,named maximum,bound+1,0x80000000 and UInt32 maxima
under both sanitizers. It does not prove full uncompiled owner-class ABI or
logical ID range/ownership checks. Actual original object/replay admission remains
pending. Supporting cohort62/62 passes all three configurations; see N2 evidence.

### NOX provider investigation (pending, not acceptance)

`Libraries/Source/Compression/Compression.h` sets COMPRESSION_MAX to RefPack,
excluding later codecs from preferred-encoding/performance loops. However,
`CompressionManager::getCompressionType/decompressData` explicitly recognizes
NOX; it is not unreachable just because RefPack is preferred. Original
`LZHCompress/NoxCompress.cpp` encodes500000-byte blocks using one retained LZHL
owner and concatenates them. Its decoder depends on provider success and consumed
source/produced-output counts; its final unconditional TRUE/raw-size reporting
cannot serve as a validation contract. The published original tree omits that
provider, so exact decoding still requires qualification.

The unchanged [public candidate](https://github.com/ryandrake08/lzhl) at
`ba3ac10ac76c3cf9a875ef477f45563865e2161a` is downloaded only for investigation,
not adopted or packaged. Its published `LZHL.h` C API discards the internal bool
decoder result and consumed count. `tests/toolchain/lzhl_probe.cpp` proves a
generated roundtrip passes both compiler sanitizers, but a rejected empty stream
reports full destination capacity with untouched backing (diagnostic exit2).
This public C API alone cannot establish rejection or original block traversal.
Its public C++ declaration exposes bool/counts and remains a separate unqualified
path; do not misreport the C API result as proving every interface impossible.
Source inspection also finds adaptive group-width reconstruction with unchecked
unary length/shifts and assert-only width bounds; malformed table admission must
be assessed before calling that provider, not patched inside it. Original J2K
releases are available from their [primary archive](https://sourceforge.net/projects/j2k/files/J2K_All/)
but have not been downloaded/qualified. No NOX support or external blocker is
accepted merely from this investigation, and no framework fork is authorized.

### Native container identity and keyed snapshots (supporting acceptance)

All13 active STLport map typedefs now use standard unordered_map; template and
speech source lists retain their independent creation/enumeration ordering.
`STLTypedefs.h` hashes AsciiString/borrowed C-string content through public
standard string_view hashing, matching their content equality. Pointer identity
hashing remains a native-width standard pointer hash, never truncated UInt32.
`ThingFactory.cpp` requests bucket backing through rehash; GameSpeech's obsolete
null map constructor is removed. Borrowed speech name backing must remain alive
until map-node withdrawal; the container does not own it.

GameLogic version7/8 overrides and Player/Team relation snapshots are keyed
records: their readers do not depend on bucket order. Four save loops now use
`rts::keysInOrder`, lexically for AsciiString and numerically for relation IDs.
The helper is fallible preparation, not allocation-free sorting; pointer keys
are compile-time rejected as snapshot-order identities. Named fields, values,
counts and empty-name terminators are unchanged. Actual owner save/load and
scenario acceptance remain pending; shared-header fixtures do not accept those
as-yet-uncompiled classes.

`containers.cpp` proves independent equal backing, distinct case, borrowed-key
lookup/erase, rehash node/reference stability, canonical order across insertion
orders/bucket sizes, all raw unsigned boundaries and signed indices. Insert,
copy and ordered-key allocation sweeps have exact terminals2/3/1 with three
memory-manager lifetimes, immediate standard-allocation and every DMA-pool used
count preservation, accepted node identity/value checks, and same-owner retry.
The frozen67-test cohort passes normal/GCC/Clang with leaks enabled; see N2
support evidence. Standard containers and all other dependency sources remain
unchanged.

Traversal review must not classify all maps as lookup-only. Particle template
nth-parent selection is called by the ParticleEditor DLL path; its source gate
is runtime DLL presence, not a compile-time exclusion. Preloading still traverses
templates during retained runtime and requires renderer-owner qualification.
`GameEngine.cpp` calls `AudioManager::isMusicAlreadyLoaded` as a retained startup
asset-readiness test, even though CDManager initialization precedes it. That
function selects the last music entry and checks its file. It is not a purely
excluded CD behavior. Native valid-media and missing/corrupt-media readiness
semantics still need actual startup/audio tests; do not silently drop the gate or
claim it accepted by container tests. Source links:
`Common/Audio/GameAudio.cpp`, `Common/GameEngine.cpp`,
`GameClient/System/ParticleSys.cpp`, `GameLogic/ScriptEngine/ScriptEngine.cpp`.

### Source-locked enum domains and Rider status identity

`tests/original/enum_domains.json` locks49 normalized enumerator bodies against
commit9e5c0e4f, including47 ordinary opaque domains, actual ObjectStatusTypes,
and tagged _TerrainLOD. Nine negative-sentinel domains retain signed32 Int:
AttitudeType, EvaMessage, KindOfType, LocomotorSetType, ModelConditionFlagType,
PhysicsTurningType, StaticGameLODLevel, WeaponBonusConditionType and
WeaponSetConditionType. The other40 retain unsigned32 representation. All named
values/order/counts and optional ALLOW_SURRENDER/ALLOW_DEMORALIZE branches remain
unchanged. `_TerrainLOD` keeps its TerrainLOD typedef alias; GlobalData declares
the complete-width tag, not the obsolete empty typedef declaration.

`test_original_enum_domains.py` verifies body hashes, unique definition ownership,
and every retained opaque declaration, then compiles source-derived prior/current
declarations in both branch configurations. Both compilers prove prior signedness,
size/alignment/representative member offsets and current full-width raw values
under sanitizers. These are isolated representation proofs, not whole owner ABI,
field-store aliasing, semantic range admission or simulation acceptance. The
frozen69-test cohort passes with leak checks enabled; see N2 support evidence.

HackerAttackMode has only two declarations and no definition/use; those are
removed. Singular ObjectStatusType is NOT unused: RiderChangeContain::RiderInfo
stores it, parseRiderInfo selects from ObjectStatusMaskType names, and
onContaining/onRemoving construct status masks from the index. The real domain
is ObjectStatusTypes (plural), now used by that member. No new index or status
meaning is introduced. The eight-slot aggregate still requires full initialization
and actual parse/copy/consume acceptance before module startup is accepted; its
source constructor currently does not initialize every enum slot. Source/test:
`Common/ObjectStatusTypes.h`, `GameLogic/Module/RiderChangeContain.h`,
`GameLogic/Object/Contain/RiderChangeContain.cpp`, enum-domain source lock.

### Original sparse selection and intrusive list protocols

`Common/SparseMatchFinder.h` selects maximum condition intersection, then minimum
extraneous yes bits; exact ties retain the first vector entry. Its map stores
borrowed vector element addresses, not owned copies. `clear()` must precede backing
relocation or definition replacement; a failed map insertion retains prior entries.
The generated supporting fixture proves explicit clear/relocate/retry, not every
actual model-state owner's invalidation contract. Owner integration remains pending.

`Common/GameCommon.h` list macros prepend once, make same-owner duplicate prepend
and absent removal no-ops, repair both neighbors on removal and withdraw each node
before its drain callback. Reverse swaps both links and the head; the iterator uses
the original next-member function. Modern C++ requires its explicit address; the
low-bit diagnostic needs uintptr_t, not a truncated unsigned int. Neither diagnostic
proves list ownership or makes cross-owner misuse valid. See
`tests/original/graph_headers.cpp` and the frozen72-test support evidence; actual
Object/partition/module lifecycle acceptance is still pending.

### Save/replay calendar and remaining native encoding boundaries

`Common/Recorder.h::ReplayHeader::timeVal`, `Source/Common/Recorder.cpp` and
`System/SaveGame/GameState.cpp` share the original calendar: unsigned16 year,
month,weekday,day,hour,minute,second,milliseconds, in that order. NativeCalendarTime
keeps16 bytes/alignment2/offsets0..14; SaveDate stores day before weekday but its
xfer writes explicit fields, not this aggregate. NativeCalendar.cpp uses public
localtime_r and guarded host locale formatting, without changing process locale.
All fields are validated before formatting; date is locale short-date and time
is24-hour minute-resolution. Source Windows code had a platform-specific24-hour
policy on Win9x and locale policy on NT; native display policy is not a claim of
pixel/locale-string identity with every obsolete Windows version.
Source/test: `Common/NativeCalendar.h`, `System/NativeCalendar.cpp`,
`tests/original/calendar.cpp`; complete74-family support matrix passes both
sanitizers with leak checks. Neither actual GameState nor Recorder is linked yet.

Recorder's surrounding format still writes/reads raw sizeof(time_t) and uses
fwprintf/fputwc for source two-byte-wide strings. Native Linux time_t/wchar_t
cannot be used as that wire schema; fread results are also unchecked. Calendar
compatibility does NOT establish replay compatibility or accepted input ownership.
Resolve the entire original replay header/message transaction in N5, preserving
source widths/encoding, rejection and rollback; do not copy host ReplayHeader
layout or silently generate a new replay format. No retail replay was inspected
for these findings; source evidence and generated calendar bytes are separate.

### Native input identities, callback transport and millisecond arithmetic

KeyDefs' KEY_* values are an original byte protocol, not SDL scancode values.
All selected numeric values were independently checked against
[Microsoft's primary DirectInput definitions](https://raw.githubusercontent.com/microsoft/win32metadata/main/generation/WinSDK/RecompiledIdlHeaders/um/dinput.h),
reference SHA256 `d532789ae49c3bdecefa7c8a00a7561a6161453cfebace0805da73ff0c6cf3b8`.
`tests/original/key_protocol.json` locks values and the unchanged modifier body;
KeyDefs now contains game-owned numeric declarations without SDK includes.
KEY_NONE0 and KEY_LOST255 are stream sentinels; KeyboardIO is8 bytes, with key at0,
state at2, sequence at4. MetaEvent mappable keys remain a subset of these identities.
Physical SDL translation is still pending. Keyboard::updateKeys currently reads
an unbounded number of events into256 slots; full-batch/focus-loss/sentinel/retry
admission must be established for every retained provider before input acceptance.

WindowMsgData is an in-process synchronous callback payload and carries both
borrowed addresses and packed scalars. It is not a file/network encoding. The
native alias is uintptr_t across the316 GameEngine source uses and the retained
legacy physical W3DMOTD callback. Shared transport proofs preserve full addresses
and packed16-bit scalar fields; actual callback lifetimes/publication remain pending.

NativeClock preserves unsigned32 millisecond wrapping. Source load progress uses
strict elapsed>500; network-start timeout remains elapsed>=30000. Signed deadline
addition had overflow risk; elapsed subtraction is defined across both32-bit
boundaries. Normal positive FPS caps retain the source float delay calculation;
nonpositive/submillisecond caps now produce zero delay rather than undefined
float-to-integer conversion. Simulation update order is not changed. See
NativeClock.h/.cpp, calendar clock family and the frozen75-test support evidence.

### Pending coupled original definition-owner findings

GlobalData currently publishes m_theOriginal at constructor entry, before later
fallible allocations. Its WeaponBonusSet is a pooled owned pointer. Correction
after inspecting GlobalData.h against the committed source: newOverride calls an
explicit private copy-assignment stub that DEBUG_CRASHes and otherwise returns
without copying. It does NOT currently shallow-copy/leak/alias that bonus; the
earlier inference from the call site alone was wrong. Overrides therefore fail
in diagnostic builds or retain constructor defaults instead of inherited values.
A replacement clone must own a distinct bonus and copy complete configuration
without introducing the shallow-copy hazards previously inferred incorrectly.
Override publication also precedes INI parse/user-preference work; reset deletes
override nodes through a destructor that assumes the singleton is original.
These are source-established hazards, not yet repaired/accepted. Audit and repair
the complete constructor/clone/parse/rollback/reset graph with actual owner faults.
Sources: Common/GlobalData.cpp::{GlobalData,newOverride,parseGameDataDefinition,reset},
GameLogic/Object/Weapon.cpp::WeaponBonusSet::parseWeaponBonusSetPtr.

BitFlagsIO's XFER_CRC path uses sizeof(this), a pointer width, not the bitset width.
Its parser/load paths mutate accepted masks incrementally on late rejection. These
are pending source findings; canonical full-mask CRC and transactional named-mask
parse/load require original-bug reproducers and actual Xfer/INI owner tests. The
Xfer/ModelState include cycle affects74 logic translation units in the complete
census; fix header ownership and these coupled mask paths together, not by
disabling template diagnostics or merely adding constants before an incomplete Xfer.

### Native cubic curves and synchronous callback payloads

BezierSegment owns four Coord3D control points (12 scalar values, despite its
old array-parameter spelling16). Its cubic basis is symmetric, row-major:
[-1,3,-3,1], [3,-6,3,0], [-3,3,0,0], [1,0,0,0]. Both evaluation and forward
differences use row-vector transformation; this is game math, not a renderer
protocol. The native scalar implementation retains source multiply/add ordering,
extrapolation, recursive length and subdivision without adopting a D3DX shim.
The original one-step iterator leaves its zero-initialized current point because
start returns before assigning P0. The native path assigns P0 and resets all
differences before that branch; generated nonzero-P0 sampling tests guard the
correction. Sample vectors publish offside; negative counts reject without
mutation and failed allocation retains prior storage through corrected retry.
Length tolerance/overlapping split-output hazards remain pending, not accepted
by finite ordinary-curve tests. Sources: Common/Bezier/BezierSegment.cpp and
BezFwdIterator.cpp; generated tests/original/bezier.cpp; N2-runtime-support.md.

SabotageInternetCenterCrateCollide encodes an unsigned32 wrapping frame as a
void* and casts it back. Actual Player→TeamPrototype→Team iteration and
OpenContain's forward/reverse loops call synchronously without retaining data.
The native owner now borrows the actual frame address for those calls, avoiding
integer-as-pointer transport. AI attack-condition arrays instead retain addresses
of two stable source-owned AbleToAttackType values, preserving normal/forced
condition identity. This source-established lifetime is not yet actual owner
dispatch acceptance. Sources: Common/RTS/{Player,Team}.cpp::iterateObjects,
Object/Contain/OpenContain.cpp::iterateContained, AI/AIStates.cpp and
Object/Collide/CrateCollide/SabotageInternetCenterCrateCollide.cpp.

All eight PartitionManager scanline callback families now use game-owned
NativeCellSpan to clip indices before forming a pointer. The prior source forms
an out-of-range pointer for negative x1 and then skips invalid loop cells; skipping
does not make that pointer formation defined. The bounded helper prepares a full
span offside, preserves prior output on rejection and uses size_t row offsets.
It proves index arithmetic only, not physical m_cells backing or cell rollback.
The four player-index callbacks borrow actual Int addresses through synchronous
DiscreteCircle::drawCircle; threat/value callbacks already borrow scoped structs.
Source: Object/PartitionManager.cpp::{hLineAdd/RemoveLooker,Shrouder,Threat,Value}.

Original DiscreteCircle performs signed left shifts of negative decision values
for ordinary radii, and of negative center coordinates. Native wide intermediates
retain the original generated scanlines, duplicate removal and top/mirror callback
order; callback coordinates stay Int. Extents reject before allocation if not
representable. The original radius2 golden spans are [-1,1] at ±2, [-2,2] at ±1
and [-2,2] at0, in source top/mirror order. Tests cover negative and near-limit
centers, zero radius, null callbacks, constructor faults and same-owner retry.
Sources: Common/DiscreteCircle.cpp/.h; tests/original/source_boundaries.cpp.

The editor's cinematic-font label is name + " - Size:" + decimal point size +
optional " [Bold]", emitted by WorldBuilder/src/EditParameter.cpp:1293-1300.
The old ScriptActions parser's fixed256-byte buffer, pointer-versus-character
termination check, double increments and unbounded colon search are incompatible
with that encoding. NativeSourceStrings parses the complete label (including
spaces/hyphens in names), checks defined Int sizes and checked frame multiplication
before display mutation. Display/font-cache publication and physical font ranges
remain actual presentation-owner acceptance; parser success is not that proof.
GameLogic::loadMapINI prepares map.ini, solo.ini, map.str and AssetUsage.txt paths
offside from the saved pristine source when needed; loader order is unchanged.
It preserves lexical parent rules and uses rooted relative companions for a
root-level map, not fixed _MAX_PATH buffers/CWD/leading absolute separators.
Rooted filesystem admission still owns access checks. Sources: NativeSourceStrings,
GameLogic::loadMapINI, ScriptActions::doDisplayCinematicText; source_boundaries tests.

ScriptEngine's optional Windows debugger, particle-editor and VTune helpers are
not simulation providers. Native public hooks preserve absent-DLL behavior;
explicit excluded-tool options reject before init. Particle name-table ownership
remains here and actual script freeze, fade/counter/sequential ordering remains.
The W3D asset reload import was exclusively a private editor helper. Both compilers
syntax-check the retained ScriptEngine now; construction/reset and option
admission tests remain pending until the complete owner graph is linked.

### Original preferences representation and native publication (N2 pending integration)

UserPreferences stores trimmed ASCII key/value pairs split at the first equals;
duplicate keys take the last value, and loads overlay existing entries. Empty
keys/values are ignored. The old parser forms line.str()+1 even for malformed
rows, splits long fgets rows, mutates the live map incrementally, and leaves FILE
cleanup unguarded on allocation failure. The native owner parses complete rows
offside, rejects NUL/overlong rows without publishing partial state, and swaps
both map and target leaf non-throwingly. Missing first-boot files retain the
writable leaf and prior values. Source: Common/UserPreferences.cpp::load.

Persistence is sorted map order with "key = value\n", through protected XDG
NativeUserStorage and atomic output, never the asset VFS. Newline/equals injection
in keys and newline injection in values reject before opening an output candidate.
Native readFile admits only regular, no-follow files and complete bounded reads;
missing files are distinct from path/I/O/size failures. FIFO admission is
nonblocking. Allocation manifests cover load19 and write9 ordinals, each paired
with unchanged accepted state, corrected retry and three same-process teardowns.
Sources: NativeUserStorage.cpp; tests/original/preferences.cpp. Normal targeted
tests and the frozen87-test normal/GCC/Clang sanitizer matrix pass; full graph
validation and engine service lifetime remain pending, not accepted startup
evidence. Empty values still serialize because original option getters insert
unset fields; they remain ignored on reload, as in the original owner.

Numeric getters preserve ordinary decimal-prefix behavior and malformed numeric
zero, but define overflow/non-finite fallback instead of relying on atoi overflow
or allowing NaN/Inf settings. AsciiString now has allocation-free move/swap of
its exact COW backing; copied aliases keep their reference ownership. This permits
no-throw preference-target publication without raising refcount headroom. Tests
exercise self-move, existing aliases and swapped identities; all87 support tests
pass in the frozen normal/GCC/Clang sanitizer matrix.

Options methods are owned in GUI/GUICallbacks/Menus/OptionsMenu.cpp and LAN methods
in LanLobbyMenu.cpp. Several defaults read TheGlobalData; extracting these actual
owners must borrow the offside candidate explicitly rather than publish a partial
GlobalData merely to provide defaults. Audio defaults borrow TheAudio, LOD defaults
borrow TheGameLODManager, and LAN IP selection enumerates configured interfaces.
Those service boundaries are retained pending source integration. Obsolete
Internet-profile preference classes are excluded, not substitutes for LAN/Options.

### Native original callback table ownership (N2 support; physical GUI pending)

FunctionLexicon has four distinct source prototypes: System/Input share one,
Tooltip has another, Draw/DeviceDraw share one, and layout Init/DeviceInit/Update/
Shutdown share one. NativeWindowCallback retains these function-pointer types;
it neither casts through void* nor calls a wrong prototype. Lookup of another
family returns null. WindowMsgData retains uintptr_t payloads end-to-end. Table
indices keep source identities -1/0..8/9 on defined Int backing; invalid indices
reject or return no table before backing access.

NativeFunctionRegistry admits bounded table spans including their original final
sentinel. Duplicate table indices, wrong families, interior/missing sentinels and
bad terminal keys reject the whole batch before interning names or publishing
keys/table links. All keys prepare offside; source names can remain monotonically
interned after a later failure, but entry keys and accepted table links do not
change. Core7 and W3D core+device9 registration now each publish as one transaction.
W3D declaration headers no longer import a full renderer simply to name callbacks;
the actual original W3D table producer syntax-checks under GCC and Clang, without
modifying the library or substituting callbacks. Physical invocation/init remains
future owner acceptance, not proved by this source probe.

Generated fixtures exercise all four prototypes, full pointer payloads,
wrong-family lookup, subset replacement, aliases, bounded metadata, late namespace
capacity rejection, corrected retry and three same-process lifecycles. Fault
manifests are warm1, short-name cold4 and long-name cold7. Immediate cold residuals
are exactly the accepted namespace's string backing plus source pool blocks;
the transient long-name copy allocation retires. Pool initialization precedes
allocation baselines; final blocks and standard resources retire exactly. Sources:
Common/FunctionLexicon.h/.cpp, NativeFunctionRegistry.h/.cpp,
GameEngineDevice/W3DFunctionLexicon.cpp, tests/original/function_registry.cpp.
All91 support tests pass normal/GCC/Clang sanitizers; complete engine and GUI are
still unaccepted. Enum inventory recognizes C++20 using-enum imports without
waiving actual unfixed-forward, scoped-definition or source-locked ABI checks.

The physical Common/Audio/GameSpeech.cpp is an obsolete WSYS/wpaudio provider:
its GameSpeech.h is empty and the original DSP lists the header, not that source;
no retained caller references CreateSpeechInterface. Do not revive its removed
SDK to satisfy a physical file census. In contrast the actual AudioManager in
GameAudio.cpp owns isMusicAlreadyLoaded, used by AudioManager::init as an asset
gate; that owner remains retained. This distinction replaces filename-based
inferences, not audio behavior. QuotedPrintable.cpp remains a retained LAN-name
encoding dependency: its Unicode representation is UTF16LE bytes, not native
wchar_t bytes. Complete conversion/bounds/odd-tail ownership remains pending.

### Configuration/bootstrap owner graph (N2 implementation; validation pending)

GlobalData's baseline copy constructor AND assignment are explicitly unimplemented
stubs, not working shallow copies. Source newOverride default-constructs then
calls that assignment. Native GlobalDataConfiguration now carries the complete
value graph separately from SubsystemInterface identity. The actual field table
addresses that configuration base; every whitespace/compact offsetof spelling
must move together when the parser receives a base pointer. WeaponBonusSet is
the one raw owned configuration pointer: clone it offside and guard acquisition;
compiler-generated configuration copy alone must never own/release it. The root
owns override retirement independently of TheWritableGlobalData, so singleton
withdrawal before destruction does not lose the chain. Overwrite/multifile adopts
values non-throwingly without replacing registered root address/name.

Actual OptionPreferences methods were in OptionsMenu.cpp, whose unused Internet
SDK imports blocked simulation configuration. Common/OptionPreferences.cpp now
owns the startup/core methods; OptionServicePreferences.cpp retains actual audio/
LOD/campaign service-dependent defaults. Default GUI preferences resolve the
current singleton at each use (it can change after construction); explicit
candidate preferences borrow that complete offside configuration. Checked native
numeric parsing retains decimal-prefix/malformed-zero behavior while defining
overflow/non-finite boundaries. Resolution defaults must come from the candidate,
not the prior singleton. Retained integer gamma's Real return can round INT_MAX
above Int range; check in double before narrowing and interpolate without signed
overflow. Source: GlobalData.cpp, OptionPreferences.cpp, original OptionsMenu.cpp.

WeaponBonus condition/name identities and BodyDamageType/AIDebugOptions/TerrainLOD
definitions now live in independent game-owned headers, consumed by the original
execution/module/terrain headers. This avoids constructing dependency/vtable
edges simply to read a configuration enum; all49 locked domain bodies remain
unchanged. MoneyDefinition.cpp retains original Money snapshot/INI methods;
RTS/Money.cpp retains actual deposit/withdraw and audio/player behavior. No service
replacement or adopted-library edits are implied by these ownership separations.

INI::load delegates the complete original registry to bounded loadBlocks. The
dispatcher validates the entire source table before opening input, preserves FPU
setup, original exact block names/token/field dispatch and Xfer line order, and
always unprepares owned input on failure. Selected real-owner tables in generated
fixtures are support tests, not full-registry/startup acceptance. Raw file/line
contents are not copied into new exception diagnostics. GameData's whole candidate
publishes only after parsing and options; complete other block owners and complete
startup transaction remain pending. Source: INI.cpp/INICore.cpp/INIValueParsers.cpp.

Process CRC remains executable→version UInt32→skirmish script→multiplayer script.
Native /proc/self/exe reads bypass rooted asset admission only for the process
image; owned read descriptors and script FileCloseOwner guards retire on failure.
Missing optional scripts keep the source skip behavior. User path comes from the
native engine's protected XDG storage, with native trailing slash and no asset
writes. NativeInputSettings reads the public SDL double-click hint with defined
500ms fallback and bounded unsigned milliseconds; full physical input integration
remains N4. IPEnumeration now reads public POSIX interface/host services and
publishes a complete sorted owned IPv4 chain, not Winsock/DNS/CWD state. Generated
tests/original/configuration.cpp covers these owners; results are pending below
their implementation checkpoint, not an accepted GameEngine run.

The original initSubsystem helper accepts an allocating AsciiString alongside an
already acquired factory pointer: argument evaluation can leak before helper
entry. Native helper takes a literal name and installs ownership before name
conversion, borrowed singleton publication, init/INI callbacks and final registry
growth. A failed registration withdraws the singleton before owner destruction;
reverse shutdown withdraws each registry entry before destructor callbacks.
Whole engine startup/global-service withdrawal and retained sibling constructor
faults remain required. Sources: NativeSubsystemInit.h, GameEngine.cpp,
System/SubsystemInterface.cpp; no complete engine acceptance yet.

Original Byte is plain char, whereas UnsignedByte is unsigned char. A stored
eight-bit mask with bit7 set can therefore promote as a negative value. The
bit-string8 helper previously handed an uninitialized UInt32 temporary to the
incremental32-bit parser; native seed/publication must use the raw UnsignedByte
representation, not sign extension. Both widths now prepare complete expressions,
admit each index before unsigned shifting, and publish only after success. Preserve
normal reset, +/- modification, NONE and empty-expression source behavior; test
bit31, bit7 prior masks, mixed late-invalid and bound+1 expressions. Duration
milliseconds retain source Real multiply/ceil ordering, but reject an out-of-range
frame count before narrowing to UInt16/UInt32. Anonymous INI status values are
now actual ErrorCode aliases (same values) rather than a distinct exception enum
that escapes ErrorCode handlers. Source: INI.h, INIValueParsers.cpp, BaseType.h;
generated configuration values tests; sanitizer acceptance pending.

The first new fault-fixture residual included a fresh INI's allocated "None"
filename. Normal unPrepFile retires that constructor string and leaves a reusable
empty reader. Establish the actual reused/unprepared owner's baseline before
arming allocation faults; do not label intended retirement as a leak or change
the parser to retain that obsolete string. The generated manifest is57 for both
overwrite/override and22 for construction, with every fault paired with corrected
retry and three complete lifecycles. Native IPv4 chain replacement shares actual
append/publication ownership with discovery, preserves accepted pointers on a
late fault and retains distinct nodes for duplicate address values; its five-node
generated manifest is5. Warm process-global pool creation before lifecycle
baselines. Tests: tests/original/configuration.cpp; normal targeted evidence only
at this checkpoint, complete sanitizer/frozen support matrix pending.

Xfer originally mixes an abstract typed snapshot boundary with default adapters
that require GameState/science/upgrade owners. GCC UBSan emits those type-identity
dependencies even when a configuration fixture never calls Money::xfer. Making
INI's typed adapter independent was insufficient: Money has the same edge.
Xfer now retains the actual typed abstract contract and original initialized
state; XferBase owns the unchanged default method bodies. XferLoad/XferSave/
XferCRC (and inherited deep CRC) consume those defaults. Native internal class
layout is not a compatibility requirement; transferred field widths/order and
method dispatch remain requirements. The actual adapter archive builds under
all three configurations, and provider headers prove they remain concrete.
No RTTI/sanitizer suppression, dummy provider, library patch or linker section
workaround is used. Full transfer execution remains pending with its real owners.
Sources: Common/Xfer.h, System/Xfer.cpp, retained Xfer* headers/sources,
cmake/OriginalRuntime.cmake, tests/original/graph_headers.cpp.

SDL's non-Windows double-click fallback is source-established 500ms in the
[pristine 3.4.16 mouse implementation](https://github.com/libsdl-org/SDL/blob/release-3.4.16/src/events/SDL_mouse.c).
Runtime code consumes only public SDL_GetHint/SDL_HINT_MOUSE_DOUBLE_CLICK_TIME,
not private headers/backend state. This does not establish an OS-wide Linux
desktop preference or physical event acceptance; those remain N4.

Bootstrap teardown cannot blindly withdraw every singleton before deleting its
owner. GameLogic::~GameLogic drains Objects whose destructors call TheGameLogic
for trigger invalidation/destruction messages, plus AI/radar/script/partition
services. GameClient::~GameClient drains Drawables whose destructors call
TheGameClient::removeFromRayEffects while rendering/audio owners are still live.
Thus root registry removal, simulation cleanup, singleton restoration and backing
retirement are distinct lifecycle steps. Current NativeSubsystemInit's failure
path restores the prior singleton before candidate destruction; that remains
unaccepted for these self-callback owners and needs coupled correction, not a
null guard. GameClient also unconditionally resets TheFontLibrary during teardown;
partial startup must track actual acquired child owners. Sources: GameEngine.cpp,
GameLogic/System/GameLogic.cpp, GameLogic/Object/Object.cpp:605,
GameClient/GameClient.cpp, GameClient/Drawable.cpp:533, NativeSubsystemInit.h.
These are verified source dependencies, not tested full-bootstrap acceptance.

Configuration fault manifests depend on native interface cardinality: the actual
OptionPreferences::getLANIPAddress enumerates POSIX IPv4 addresses even for an
unset selected address. Each EnumeratedIP label adds one ordinary allocation.
The sandbox's zero-address view yielded57; normal-host discovery yielded63
(six addresses) in both compilers and even the unsanitized executable. Thus
this was not an ASan/UBSan ownership failure or permission to truncate coverage.
tests/original/configuration.cpp now captures only the native address count,
retires that census before transaction baselines, freezes the exact terminal
at57+count (construction remains22), and verifies every ordinal's discovery pool
retirement as well as bonus/std/descriptor resources and corrected retry. The
independent generated five-address interface owner sweep remains exactly5 and
retains duplicate-address ownership units. Never record discovered addresses or
host names in evidence. The corrected frozen99-case matrix passed in all three
configurations; see evidence/qa/N2-runtime-support.md.

The registry transaction now separates typed abstract ownership from complete
INI provider loading: initOwnedSubsystem guards factory acquisition, publishes a
temporary borrowed slot, and retains ownership through name/init/definition work
and final registry growth. The load callback cannot publish registry ownership;
the list performs the one final acquisition after it returns. Accepted records
carry typed noexcept slot restoration; reverse teardown pops the owned entry,
preserves the live singleton through legitimate source destructor callbacks,
then restores the prior slot without further callbacks/allocations. No simulation
singleton is authorized for render/audio thread access. List destruction drains
owned entries, recursive shutdown does not retire siblings during parent cleanup,
and empty publication metadata rejects a nonempty ignored prior pointer.
Source: NativeSubsystemInit.h, SubsystemInterface.h, SubsystemBase.cpp, complete
SubsystemInterface.cpp loading adapter. Generated tests/original/subsystems.cpp
exercise source ordering, semantic rejection and all five allocation ordinals,
with exact prior ownership/retry and three process lifetimes. A real GlobalData
configuration bootstrap family exercises initialized-owner late decode rejection,
parallel root admission, root/override reset/retirement and restart through the
same transaction. These are lifecycle support, not complete GameEngine acceptance.

The accepted registry vector retains its high-water capacity after shutdown,
like the original clear-based owner. Establish its legitimate warmed-empty
baseline before testing repeated actual GlobalData admission; do not call
shrink_to_fit or treat accepted registry backing as a leaked candidate. Whole
list destruction must retire that capacity exactly.

Additional complete-bootstrap hazards remain coupled: GameEngine::init catches
non-D3D ErrorCode then continues into TheGlobalData dereferences after partial
initialization; GameEngine::~GameEngine dereferences GameResultsQueue and the
subsystem list even when not acquired. GameClient construction publishes an owned
DrawGroupInfo before later derived construction can fail, and teardown assumes
FontLibrary exists. Reconcile actual acquired child/service/publication owners
and fail propagation together; tested registry ownership does not prove these
parents are safe. Sources: GameEngine.cpp:185,656, GameClient.cpp:98,193.

QuotedPrintable is a retained map/cache and LAN/skirmish preference protocol,
not only excluded Internet chat. Actual INIMapCache.cpp/MapUtil.cpp and both
LAN/skirmish preference owners call its four public converters. Original encoders
walk Windows UTF16 bytes little-endian and emit ASCII alphanumeric literals or
underscore plus two uppercase hex digits; Unicode output must not walk Linux
wchar_t bytes. Signed char shifts incorrectly reject high hex nibbles, and static
1024-byte buffers couple independent conversions. Both decode loops initialize
i but never increment it, so their apparent bound does not bound backing writes.
The Unicode odd-tail branch writes a zero high byte but omits the following
complete terminator: decoding a shorter odd-byte result after a longer one can
expose that prior static tail. A bounded witness can reproduce this without an
out-of-bounds write. These are source-established hazards, not accepted native
codec behavior; planned actual converter tests are in existing N2slice03.

Recorder.cpp's startTimeOffset is6, but end/frame offsets and raw header writes
use sizeof(time_t). The original Windows timestamp width must remain fixed;
native64 time_t cannot silently expand the replay header. NativeCalendarTime's
already-tested16-byte SYSTEMTIME representation does not prove these separate
timestamp fields. Derive/prove the complete writer/reader/header ledger before
accepting replay execution. Sources: Recorder.cpp:59,229,561 and its paired reader;
replay/supplied-content ownership remains pending, not inferred from archive builds.

The actual four quoted-string converters now encode the unchanged canonical
underscore/two-hex grammar, prevalidate complete output capacity and Unicode
scalar/UTF16 surrogate admission, then publish independently owned standard
storage. The decoder consumes explicit UTF16LE words, pads an odd final low byte,
and never reuses a mutable buffer. Malformed/truncated escapes and encoded NULs
reject rather than producing truncated aliases; original canonical encoder output
remains unchanged. tests/original/quoted.cpp fixes known ASCII/high-byte/BMP/astral
bytes, maximum32766-byte result backing, expanded bounds, repeated2048-byte/short
conversions, a bounded stale-tail/non-incremented-counter source witness, and
three allocation ordinals for each actual converter with retry and three process
lifetimes. Focused and complete106-case normal/GCC/Clang sanitizer matrices pass
on frozen401-path SHA0fc51f4c740e2f8008cf414c1bc8d78c43d25db417c4c98a3aeb29f465bf6510.
Sources: System/QuotedPrintable.cpp and Common/QuotedPrintable.h. No
framework code, external encoding authority or proprietary assets were changed;
actual map-cache/save/replay/LAN owner integration still needs acceptance.

Parent acquisition differs from subsystem registry ownership. GameEngine directly
owns the subsystem list, FileSystem, NameKeyGenerator, CommandList and GameLOD;
GameClient directly acquires its visual/UI services (including DrawGroupInfo in
its base constructor), and GameLogic directly acquires partition/ghost/terrain/
script owners. A failed derived constructor runs base/member destruction, while
a failed base constructor only runs already-constructed members. Record actual
acquisitions in fully initialized, allocation-free member journals before child
init/definition work. Teardown retires those exact acquired objects, never an
unrelated object's current global slot. Keep source peer cleanup order and live
self-publication; required null providers reject, optional nulls acquire no unit.
NativeServiceOwners now supplies that boundary to the three actual parents and
GameMain. Its three generated families pass normal/GCC/Clang sanitizers, including
constructor failure, occupied-slot/capacity pre-admission, exact two-allocation
fault/retry, explicit/reentrant/reverse retirement and three process lifetimes.
This verifies the shared mechanism, not complete real parent execution.

MessageStream owns attached translators through TranslatorData, whose destructor
deletes the translator. Its original node allocation precedes guarding the incoming
translator, so failure leaks an already constructed translator (including a
LookAtTranslator's singleton publication). Guard before node allocation, consume
the ID only after success, and publish client cookies only after attachment returns.
Client translator destructor census: Window/Place/Command/Meta/GUI/Selection have
no peer cleanup callbacks; LookAt withdraws its own matching publication. Actual
GameEngine/GameMain/MessageStream/full INI/registry sources build in GCC and both
sanitizer compilers; actual GameClient was subsequently added after its native
clock/preload port. Complete translator/parent fault execution is still pending.

GameEngine startup must propagate original typed failures through GameMain's
construction guard. Legacy fatal presentation before unwinding bypasses cleanup;
swallowing non-D3D ErrorCode continues into unacquired GlobalData. Native startup
now rethrows without those pre-unwind fatal dialogs. Presenting errors belongs
outside the retired construction owner. GameClient's legal-page interval uses
wrap-safe native elapsed milliseconds with the same4000ms/100ms policy. Native
preload diagnostics report process peak resident KiB, not fictitious Windows
available-memory fields; texture/model/control-bar/particle preload order is
unchanged. These compilation changes do not establish physical audiovisual parity.
Sources: Common/GameMain.cpp, Common/GameEngine.cpp, Common/MessageStream.cpp,
GameClient/GameClient.cpp, GameLogic/System/GameLogic.cpp and NativeServiceOwners.h;
fixture: tests/original/service_owners.cpp. Frameworks and retail assets untouched.

Bootstrap provider census must distinguish exclusions from retained callers.
GameResultsInterface's queue is used only by ScoreScreen's GameSpy ladder/results
submission and its GameSpy worker thread; it is not a LAN results provider. Its
constructor starts worker threads before returning, so importing it would also
import excluded Windows/GameSpy ownership. DRM/CD checks and Internet replacement
are explicitly excluded by the active specification. Source links:
GameNetwork/GameSpy/Thread/GameResultsThread.cpp:78,109;
GameClient/GUI/GUICallbacks/Menus/ScoreScreen.cpp:1772; Common/GameEngine.cpp.
These verified exclusions are not yet removed from current bootstrap code.

LocalFileSystem cannot simply be deleted because the native rooted FileSystem
already exists. Retained ConnectionManager file announce/send uses physical user
map paths; replay menu callbacks use physical replay paths; command-line mod
selection checks physical directories and finishes with ArchiveFileSystem::loadMods.
MapCache creates/enumerates/writes user maps and separately loads official maps.
Native FileSystem currently accepts logical read-only mounted asset names only,
rejects absolute paths and always rejects createDirectory. Its admitsUserStorage
is an asset-root exclusion check, not a user-root containment proof or physical
user-file reader. NativeUserStorage owns protected XDG I/O separately. Integrate
these retained consumers through the correct user/asset owner without fake legacy
filesystem providers, broad physical path access, changed precedence or writes
inside supplied roots. Preserve source mod and official/user map selection rather
than silently dropping it. Pending native integration, not accepted by existing
mount/storage fixtures. Source links: Common/CommandLine.cpp:1089,1297;
GameNetwork/ConnectionManager.cpp:2105,2143;
GameClient/GUI/GUICallbacks/Menus/PopupReplay.cpp:265,286;
GameClient/MapUtil.cpp:355,360,457 and System/NativeFileSystem.cpp:176,183,241.

Retained replay reading couples representation and publication: readReplayHeader
replaces m_file directly, publishes each header field while reading unchecked
fread results, mutates m_gameInfo before local-player admission and can overwrite
an existing file owner. Its ASCII/Unicode loops increment index before storing
the next value; a nonterminated1024-unit input can write slot1024. Unicode reads
use fgetwc and writes use fwprintf/fputwc on an already byte-oriented FILE stream,
which is not a portable UTF16 codec. Timestamp widths, UTF16 strings, command
WideChar representation, bounded complete reads, offside header/game info and
file/replay-state rollback must form one coupled retained owner batch. No native
import acceptance follows from the earlier calendar or quoted-codec fixtures.
Source links: Common/Recorder.cpp:816,1167,1195 and its paired writer/readers.

Full INI callback reachability is broader than compiling INI.cpp's dispatch table:
the Common INI directory contains30 translation units, with INICore/value helpers/
full dispatcher already owned elsewhere. The other27 actual providers now have
their own original_definitions archive, without duplicate source ownership.
GameLogic/client/store-defined callbacks still require their real owners at final
link and execution. Original WebpageURL metadata has exactly one URL field;
its old parser dereferences an uninitialized URL pointer when the optional ATL
browser is absent. Native parsing admits the same named block/URL field offside
and deliberately does not create that excluded browser, convert CWD file links
or launch a process. Generated late unknown-field/missing-tag rejection and
exact12-allocation failure/retry coverage exercise the actual provider; native
GetRegistryLanguage shares the configured text-catalog language, not a fabricated
Windows registry. Original broad archive compilation and these selected-owner
fixtures do not accept the full registry or supplied-content startup. Source links:
INI/INIWebpageURL.cpp, WOLBrowser/WebBrowser.cpp:137, Common/INI/INI.cpp:135,
GameClient/NativeTextPaths.cpp and tests/original/configuration.cpp::webpage.
Latest408-path code SHA6078b4933f40a6f27ae4fa420fdb48e68cdbd03001f21bc13f89f272dc95a170
is frozen for complete normal/GCC/Clang checks; results remain pending here.
Those checks now pass: all three complete builds and110/110 support cases in
normal GCC2.82s/GCC ASan/UBSan/LSan11.97s/Clang sanitizers10.89s on the normal
host. The exact12 metadata terminal and source file-pool/descriptor/std backing
retirement are covered in all three same-process lifetimes. No browser or process
was launched; complete original startup remains pending.

Do not launch full retail startup before the user-file cohort is integrated.
GlobalData publishes the configured XDG data path, but NativeUserStorage's
constructor only records its paths; asset-root exclusion is enforced when its
protected I/O methods run. Original raw fopen/DeleteFile writers bypass that
boundary. MapCache::writeCacheINI can target official Maps when buildMapCache is
set; GameStateMap::clearScratchPadMaps derives deletion targets from CWD; replay
rename/overwrite callbacks delete a destination before copying. These are not
safe native writer/deletion services, even when a separate storage fixture passes.
Guard configured user/cache roots before parent startup publication, route actual
cache/save/replay/scratch writes and retirement through that owner, and never
activate the excluded official-content build/write tool against supplied roots.
Preserve original metadata grammar, official-over-user map identity, actual save/
replay state and recoverable overwrite semantics using complete offside publication.
Source links: GlobalData.cpp:1039; NativeUserStorage.cpp constructor/openDirectory;
GameClient/MapUtil.cpp:372,457,525;
Common/System/SaveGame/GameStateMap.cpp:460;
GameClient/GUI/GUICallbacks/Menus/PopupReplay.cpp:276.

User-map identity also reaches the already-ported companion preparation helper:
nativeMapCompanionPath currently accepts only logical relative maps, while retained
getUserMapDir returns an absolute configured user path and source MapCache lowercases
keys. Do not broadly permit absolute paths in that helper or pretend every path is
an asset name. Couple map identity/companion lowering to its captured asset or XDG
owner, preserving official/user distinction and original companion/load order.
Root and component case-selection must be explicit: source Windows consumers
lowercase names, but arbitrary native physical path rewriting can lose valid user
roots. Asset indexing already rejects ambiguous case collisions; retained user
directory selection needs equivalent bounded admission rather than unsafe CWD
fallback. Sources: NativeSourceStrings.cpp::nativeMapCompanionPath,
GameClient/MapUtil.cpp::getUserMapDir/loadUserMaps/clearUnseenMaps and original
GameLogic start/load companion consumers. Current source-string fixtures only
prove their declared relative logical protocol, not user-map integration.

Root protection and cache availability are separate contracts. The existing
cache family deliberately constructs a storage owner whose cache path is a regular
file, and proves successful conversion with no persisted cache. Eager startup
validation must not convert optional unwritable-cache fallback into fatal startup.
Reject asset-root/alias ownership before publication without demanding that an
optional cache directory can be created; preserve per-operation containment and
truthful unavailable-storage errors. Sources: NativeUserStorage.cpp and
tests/original/storage.cpp::cache (unavailable storage fixture).

Persistence ownership ledger (pending native implementation):

| Source owner | Coupled read/write/retirement contract |
| --- | --- |
| XferSave | beginBlock writes a size placeholder then allocates its pooled stack node; endBlock pops that node before fallible size backpatch and seek restoration. Guard stack ownership before output mutation, poison rejected streams and preserve the accepted target until complete publication. XferFilePos is native long bookkeeping, not a serialized width; XferBlockSize is the wire field. |
| XferLoad | Raw fopen/fread and relative seek need the admitted asset/user provider, complete bounded reads and owned file lifetime. Failed load must not manufacture a successful partial field. |
| GameStateMap | Both pristine and in-use embedding acquire file + full buffer before fallible transfer; extraction opens/truncates the target before size/read validation. Guard both resources, validate complete payload and publish extracted backing offside. Scratch cleanup must use its captured user owner, not CWD or TheGameState. |
| Recorder | Header and command writes mix native-sized scalar/UTF16 representation with unchecked stream operations and a pooled GameMessageParser. Guard file/parser/metadata, preserve source command ordering and raw field protocols, and pair fixed header backpatches with bounded owned stream state. |

Source registry order creates GameStateMap before GameState. Reverse shutdown
retires GameState first, so GameStateMap's destructor cannot safely query
TheGameState->getSaveDirectory even though both registrations succeeded. Keeping
each owner's self-publication valid does not keep a retired peer alive. Capture
the stable protected user-storage/scratch ownership instead; it must outlive the
subsystem list and retire before asset backing. Sources: Common/GameEngine.cpp
GameStateMap/GameState registration order; GameStateMap.cpp:66,78,135,190,460;
XferSave.cpp:165,206; XferLoad.cpp:66,122; Recorder.cpp::writeToFile/writeArgument.
These source findings were collected as one surrounding owner graph, not by
triggering a retail assertion or accepting an isolated guard.

### Protected native user namespace support (N2 slice03; actual consumers pending)

`NativeUserStorage::openReadFile` passes a guarded no-follow descriptor to the
actual shared `NativeDataBacking` and pooled `NativeDataFile` owner. The new
descriptor constructor adopts only on successful construction, rejects non-read-
only/special/oversized backing, and leaves failed construction with the caller.
Returned original File views outlive storage/asset owner destruction because
their shared descriptor backing is independent. Read access flags and the source
requested filename are retained when entering through attached FileSystem.

FileSystem now supports an explicit borrowed user-storage attachment, admitted
only for that storage's own asset provider and non-aliasing configured roots.
Absolute reads/info/discovery enter only the configured data namespace; arbitrary
absolute paths and writes still reject. Asset-relative names keep their existing
immutable precedence. Withdraw the attachment before remounting assets; a mounted
ownership generation cannot change underneath attached user storage. User path
lowering preserves the configured native root spelling and normalizes source
backslashes only below that boundary. **Source MapCache lowercases whole identity
keys; physical root/component reconstruction for those keys remains pending.**
Do not infer complete user-map execution from exact-spelling generated reads.

Directory discovery prepares a complete replacement before output publication,
uses guarded no-follow directories rather than fallible library iterators, rejects
case-colliding names/special files/over-Int32 files, and counts every encountered
entry against the caller's bound. FileSystem's runtime discovery budget is
1,048,576 entries per complete transaction. Missing optional directories return
empty results; late rejection preserves the caller's original FilenameList.
User FileInfo shares the existing 100ns-since-1601 timestamp protocol through
NativeFileMetadata.h; this is not Recorder timestamp-width evidence.

NativeAtomicOutput now supports bounded non-sparse seek/backpatch on unpublished
bytes. Invalid/failed seek poisons an otherwise commit-ready owner. Native copy
keeps an actual File read owner and a typed atomic output, preserving the old
destination on every pre-publication failure, including self-copy. Removal uses
an admitted parent descriptor and an exact regular-file name, never CWD or a
recursive wildcard; missing files are idempotent. Captured scratch-name ownership
in GameStateMap and actual save/replay/menu integration remain pending. Moving
NativeUserStorage.cpp into original_data keeps the storage/VFS owner graph acyclic;
cache conversion stays in original_runtime_common with OpenSSL's public API.

Frozen413-path SHA18bb61ff35f9a39d0ee16ea16128f9ce95b90cccfe693026dd1335ce79fc1104:
storage/configuration targets build and19/19 focused cases pass normal GCC4.72s,
GCC ASan/UBSan/LSan16.46s and Clang sanitizers17.88s (sanitizers on normal host).
Six complete generated allocation censuses/sweeps have exact terminals
19/23/12/32/21/39 (read/discovery/root admission/copy/metadata/original discovery),
each with every failed ordinal, rollback/retry and three same-process lifetimes.
The actual GlobalData default constructor adds12 root-admission allocations to
the earlier22, giving34, with independent census plus exact terminal proof.
Ordinary native interface cardinality remains separately measured, never printed.
These are focused support results, not a refreshed full112-case matrix, full
parent execution, N2 acceptance, retail audit or physical graphics evidence.
Sources/tests: NativeUserStorage.h/.cpp, NativeDataFile.h/.cpp, NativeFileMetadata.h,
NativeFileSystem.cpp; tests/original/storage.cpp::namespaceFiles/atomic/ioFaults,
configuration.cpp::faults and AllocationFault.cpp::attempts.

### Captured scratch retirement and actual parent integration (support checkpoint)

NativeScratchOutput captures the candidate's device/inode and a duplicated parent
descriptor before any irreversible publish. Its destructor performs direct
no-throw POSIX retirement only for that exact published regular-file identity,
with no allocation, storage lookup, CWD, GameState callback or provider callback.
Inactive failed candidates preserve accepted prior files. Prior scratch owners
cannot delete a replacement inode; unrelated later replacements survive too.
Scratch retirement remains valid after both storage and asset-provider destruction.
NativeUserStorage withdraws only its matching VFS attachment on destruction;
the asset provider must outlive the storage owner, while committed scratch records
and admitted File views have independent backing lifetime.

The actual GameStateMap now retains typed scratch outputs prepared before publish.
Both original embed paths use guarded real File and full payload owners; extraction
admits the complete transfer block before constructing output or replacing bytes.
Its destructor/clearScratchPadMaps drains only its own journal, not every .map in
a CWD-selected Save directory or the already-retired TheGameState. This corrects
the original unrelated-file deletion and retired-peer hazards, not gameplay.
GameEngine journals a sixth direct owner, NativeUserStorage, before GlobalData,
and retains it through all subsystem teardown before asset FileSystem retirement.
Its native FileSystem factory must return already-admitted explicit asset mounts;
the base unconfigured factory now rejects user-root attachment rather than faking
successful startup. A real native entry/root factory remains pending.

Frozen414-path SHA9de68fbee6991d76cebb3c07b958653942a1917971102cff98312ef4da56d189:
storage/configuration/seven-source bootstrap target builds pass all three compilers;
focused21/21 cases pass normal GCC4.80s, GCC ASan/UBSan/LSan16.65s and Clang
sanitizers17.90s. Scratch's independent complete allocation census is14; all
ordinals, corrected retries, constructor failure after descriptor duplication,
I/O status/throw families, allocation-free retirement and three lifetimes pass.
This is actual-source compilation plus shared-owner execution, **not** execution
of GameStateMap's complete xfer or whole save/load metadata rollback. Root/map-key
case lowering, whole GameState/Xfer/Recorder consumers and full parent execution
remain pending. No retail audits or GPU tests were run at these checkpoints.

Folded-map integration source findings (latest batch acceptance pending):
GameLogic::loadMapINI already selects the saved map's pristine source before its
four companion preparations. NativeUserStorage::mapIdentityPath now treats the
folded/backslash key as a separate source protocol, restricted to the captured
configured Maps/Save owner. It resolves physical components against complete
bounded case admission, preserving the actual native root spelling; ordinary
relativeDataPath absolute admission remains exact-root. CachedFileInputStream
captures explicit or native parent storage and resolves absolute map keys before
its actual File/decompression/cache transaction. This does not grant generic
case-folded physical access outside map ownership.

MapUtil has multiple coupled representation/owner paths, not one leaf assertion:
loadUserMaps searches only backslashes, compares a backslash suffix and then
queries physical FileInfo using a whole-path-lowercased key. Display/preview leaf
selection also adds1 to a potentially null reverseFind result. addMap's map.str
builder prepends dirName to an already-prefixed fname, then appends map.str below
the map basename; actual map companions belong to the containing map directory.
The current source corrections use both separator spellings, query original
physical spelling and reuse the captured companion rule. calcCRC/loadMap contain
unused fixed260-byte filename copies; their removal affects no consumer value and
avoids an overflow on native paths. These MapUtil source edits are **not yet a
compiled/executed complete MapCache provider**.

Required whole MapCache metadata owner work (pending, do not accept these guards):
ParseSizeOnly writes static width/height/border/boundaries before complete admission,
resizes from unchecked signed counts and returns before the historical height-byte
allocation/resampling tail. That tail is unreachable here, not an active preview
height conversion contract. ParseObjectDataChunk acquires a pooled MapObject before
fallible waypoint/supply/tech insertion. loadMap publishes raw static waypoint
ownership before parser registration/execution; addMap ignores a false open result
and can consume stale static world/extent data. calcCRC can treat a negative read
as successful EOF and publish a prefix CRC. MapMetaData/WaypointMap ordinary default
construction leaves scalar/vector metadata uninitialized. The original resetMap
is called only after successful addMap, so fault cleanup/retry needs the entire
offside query graph, not another isolated assert guard. Preserve compressed-file
CRC bytes, waypoint selection/order, original portable map names, official-map
precedence and text-catalog ownership; split metadata providers from UI only if
needed without dropping real callbacks. Raw MapCache writer/content-tool paths
and actual source INI global publication remain pending integration.
Source links: GameClient/MapUtil.cpp::calcCRC/ParseObjectDataChunk/ParseSizeOnly/
loadMap/addMap/loadUserMaps, GameClient/MapUtil.h::MapMetaData/WaypointMap,
GameLogic/System/GameLogic.cpp::loadMapINI and Common/INI/INIMapCache.cpp.

Captured-map support checkpoint: frozen414-path
SHA9f87c3ae3e5ae2067b1d701d2e5840af4bae88da413d5f9ef777678c628fa872.
Actual259-TU logic archive, seven-source bootstrap, storage/configuration/map/
source-boundary targets build in all three configurations. Focused38/38 cases
pass normal GCC4.76s, GCC ASan/UBSan/LSan16.83s and Clang sanitizers18.02s.
The complete map identity + companion allocation census is190, covering every
failure/rollback/retry and the exact terminal in three lifetimes. Generated
uppercase-root/component and backslash identities resolve to actual original
CachedFileInputStream bytes; outside/traversal/non-map namespace/case ambiguity
reject without widening ordinary exact-root admission. Optional missing companion
and saved-map selection pass. Existing compressed/cached stream tests remain green.
MapUtil's source corrections and complete metadata owner work are still pending
compiled/direct execution evidence; full116-case matrix/whole process acceptance
has not been refreshed or inferred from these38 cases.

MapObject dependency tracing for the metadata batch: its constructor/destructor
and getWaypointName/getThingTemplate definitions live in
GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp, not a shared Common
provider. Constructor validateName currently returns its input unchanged; supplied
properties are copied unchanged, setIsWaypoint changes only a runtime flag,
getWaypointName reads TheKey_waypointName, and getThingTemplate selects the final
override. Metadata uses only those properties/template classifications and loc;
it does not observe normalized angle, render/shadow backing or bridge arrays.
The original constructor's render members are null. A direct guarded parsed-value
metadata query can remove this unnecessary physical presentation-owner dependency
while retaining exact source version/z/waypoint/kind selection and lookup order.
Keep template lookup for all records: ThingFactory::findTemplateInternal also has
a special LOAD_TEST_ASSETS path that can create a template for TEST_STRING names;
do not assume every lookup is side-effect-free or silently change retained order.
Classify that content-tool/test-asset path before cache rollback acceptance.
Source links: WorldHeightMap.cpp:68,92,127,348,364;
Common/Thing/ThingFactory.cpp::findTemplateInternal; MapObject.h::setIsWaypoint.

Shared metadata provider batch (support evidence, whole query acceptance pending):
MapMetaDataReader initializes all eight authored waypoint slots and its initial
camera, extent, timestamps and flags. The actual INI callback admits player count
0..8, signed-File size bounds and ordered finite extents before publication.
Source supply/tech ordering retains both push-front stages. MapMetaData's no-throw
swap exchanges every field, including WaypointMap's separate start-count scalar.
UnicodeString's pointer swap does not acquire refs or allocate. The actual store
global and source-format serializer now belong to NativeMapMetadata.cpp, independent
of presentation. The unchanged WellKnownKeys table was formerly instantiated in
WorldHeightMap.cpp; NativeWellKnownKeys.cpp owns its single production definition
for shared INI/logic queries, with no fixture substitute globals.

Map metadata label reads must not borrow or reset the active gameplay map catalog.
GameTextManager::fetchMapMetadataLabel prepares a temporary filtered map catalog,
retains base-before-map label precedence and returns source missing-label text
without acquiring missing-label memoization. Empty companion means base-only
official metadata; a missing companion never reuses another map's catalog.
Both semantic and allocation rejection retire the temporary graph, leaving active
gameplay label backing unchanged. The excluded Windows Autorun tool has a separate
GameText implementation and is not a retained runtime provider.

The source cache serializer preserves field/waypoint/list order, quoted-printable
map identity, signed timestamp-word decimal spelling, %.2f coordinate spelling
and omission of localized display names. It prepares all bytes before persistent
output. MapCache::persistCacheINI uses captured native Data/Maps/MapCache.ini and
the actual atomic output owner; unavailable persistence returns false, allocation
or unexpected callback failure propagates, and post-rename durability uncertainty
is still truthful publication. MapUtil::updateCache clones accepted state and
temporarily directs actual INI callbacks to that candidate, prepares user-cache
bytes before official metadata overrides, completes official parsing before any
cache-file publication, and only then swaps accepted state without allocation.
Excluded -buildMapCache/physical official-content writer branches are removed;
user map discovery remains. createDirectory admits only the explicitly attached
user Data namespace, with no CWD or supplied-root creation.

Frozen424-path SHA fcb71b19ecfc5054c66ed6ca78c2c151e66dc2e08cf000efb96acd9eef17b5fc:
actual eight-source bootstrap including MapUtil and selected metadata/text/storage
providers build in all three configurations. Focused11/11 cases pass normal
GCC0.12s, GCC ASan/UBSan/LSan0.85s and Clang sanitizers0.60s. Independently censused
metadata insertion46/replacement45/serialization34 and temporary label60 cover
all failures/retry/exact terminal with three lifetimes. Fresh/reset/copy metadata
from nonzero backing, all eight waypoint slots, malformed/late fields, localization
precedence and source-format roundtrip pass. Subsequent protected-cache publication
fixtures are newer than this freeze and pending validation. No whole MapCache
query/ThingFactory/full native process acceptance is inferred. In particular,
TEST_STRING lookup can still acquire content-tool template/module backing, and
mixed source/native map-key separator canonicalization needs complete consumer
proof before accepting the whole MapCache query transaction.
Source/test links: Common/System/NativeMapMetadata.cpp, NativeWellKnownKeys.cpp,
Common/INI/INIMapCache.cpp, GameClient/GameText.cpp::fetchMapMetadataLabel,
GameClient/MapUtil.cpp::updateCache, tests/original/metadata.cpp,
tests/original/data.cpp::textManager/fault_metadata, tests/original/storage.cpp.

Complete support checkpoint after actual optional writer integration: frozen424
SHA ad0f024f7e0bd035f56c82a71237ee9a22c1ae169cb597b596d0b19e158d3bd8.
Full target builds (including259 original logic TUs/eight bootstrap TUs/all27
definition providers) and126/126 configured tests pass normal GCC4.77s, GCC
ASan/UBSan/LSan16.82s and Clang sanitizers18.12s on the normal host. Actual optional
cache publication census11 covers all allocation failures/retry/exact terminal in
three lifetimes. Physical generated-file tests exercise open/write/partial-write/
file-sync/prepare-publish failure status/throws, post-rename directory-sync
uncertainty, unavailable output, supplied-root exclusion and exact backing drain.
Framework pins remain pristine. This is support validation, not whole native
startup/query acceptance. Required next owner correction: optional malformed
MapCache.ini must fall back to complete rescan without retaining a partially parsed
cache or mutating unrelated definition stores; required official data must still
fail truthfully. Existing loadUserMaps directly invokes the unrestricted full INI
registry and currently propagates optional cache parse errors. The new bounded
actual MapCache-only registration will preserve source cache grammar, isolate
publication and keep allocation failure distinguishable from corrupt-cache fallback.

Optional metadata cache support checkpoint: frozen424
SHA1719a97f37083824d2d11fbd157afacc3e6f96e1cc431bfcb706a9dd0ac8c916.
Actual bounded MapCache-only loader clones complete accepted metadata, temporarily
publishes the candidate to actual INI callbacks and swaps only after whole-file
admission. Missing/corrupt optional cache returns false; allocation and required
owner errors propagate. Quoted identity syntax errors are classified as bad INI
at the metadata boundary, not hidden by catching arbitrary exceptions. Required
official parsing remains separate. INIMapCache.cpp now belongs to runtime_common,
not the definitions archive (26 remaining providers); every source still has one
actual owner and the helper introduces no static-library dependency cycle.
Target builds pass all3; focused21/21 metadata/configuration cases pass normal
GCC4.83s, GCC ASan/UBSan/LSan16.74s and Clang sanitizers17.96s. Complete optional
load allocation census49 passes all ordinals/retries/terminal in three lifetimes.
Late block rejection, foreign block isolation, malformed underscore-hex identities,
missing file, owner mismatch and accepted gameplay/catalog preservation pass.
Full128-case matrix/whole native process remain pending; prior126 full matrix is
historical evidence, not coverage of this newer loader.

Remaining connected name ownership finding: DataChunkInput::readDict calls the
actual global NameKeyGenerator::nameToKey for TOC names before the remainder of a
field/dictionary/query is admitted. Per-entry insertion is strong, but later
rejection can retain newly registered Buckets and consume ordinals. Existing chunk
fault fixtures warm all those names, so they do not prove cold-name rollback.
The same lazy shared-key acquisition can occur in the first metadata callback.
Do not accept whole query atomicity using warmed registry fixtures. A bounded,
allocation-free owner transaction must retire only keys acquired since admission,
restore ordinal headroom, invalidate StaticNameKey's cached generation on rollback
and retain successful/nested acquisition order. Dict uses noexcept shared_ptr copy,
so successful return is not a second fallible source publication boundary.
Source links: DataChunkInput.cpp::readDict, NameKeyGenerator.cpp::nameToKey/
nameToLowercaseKey/StaticNameKey::key, tests/original/chunks.cpp::warmNames/faults,
INIMapCache.cpp::parseMapCacheDefinition, Common/Dict.h::Dict.

Cold-name ownership support checkpoint: frozen424
SHA33b09711668dbc0b70b00a8dd2687627382885111fb933cda44e232028ca9e92.
NameKeyTransaction captures an initialized owner and next ordinal, reserves a
unique rollback-generation token before mutation, and tracks nested owner depth.
Rollback removes only newly prepended Bucket units from fixed sockets, restores
ordinal capacity and invalidates lazy StaticNameKey caches without allocation or
callbacks. Inner commit remains covered by outer rollback; inner rollback keeps
outer acquisitions. Tokens are issued process-monotonically, but a reserved outer
rollback token can be published after a newer nested token; uniqueness, not token
ordering, is the cache-identity contract. Reset/init reject while a scope owns the
namespace. Actual readDict captures that owner before provider reads and retires
its candidate before key rollback. Whole optional cache/update scopes cover keys
committed by nested dictionaries; successful Dict return uses noexcept shared_ptr
copy. All31 related runtime/chunk/metadata cases pass normal GCC0.27s, GCC
ASan/UBSan/LSan2.14s and Clang sanitizers1.53s. Exact/folded name-scope census6 each,
cold dictionary17, complete prefix/type rejection, first-use metadata static key,
changed ordinal retry, nested lifetimes, allocation-free retirement and bounded
capacity reuse pass three lifetimes. Full133-case refresh/whole runtime pending.

Template/module source-cohort inspection (pending, not accepted implementation):
all six actual Common/Thing TUs pass focused GCC syntax checks using the real
bootstrap compile configuration. ThingFactory::newTemplate/newOverride acquires
pooled templates before fallible default/override assignment; addTemplate links the
candidate before fallible hash growth and consumes IDs before complete admission.
copyFrom overwrites name/id/list linkage before all member copies return. These
paths need a coupled definition-owner transaction, not isolated guard fixes.
ModuleData is ordinary C++ storage (not pooled). MAKE_STANDARD_MODULE_DATA_MACRO_ABC
allocates it before fallible INI parsing without an owner guard; ModuleFactory then
sets a fallible name tag and grows its global owning pointer vector without guarding
that returned data. ModuleInfo Nugget pointers borrow from that factory; copying
template vectors does not acquire ModuleData ownership. Whole definition rollback
must cover factory-acquired data until the unpublished template graph retires.
makeDecoratedNameKey uses an unbounded strcpy into256 bytes and creates namespace
keys even for failed lookups; preserve accepted type/name protocol with bounded
preparation and scoped owner admission.

Retained namespace consumers must also be audited: the standard module macro
caches a raw NameKeyType forever, and a census finds180 literal raw-key initializers
in61 engine/device files (34 const; no address-bearing uses). Apparent later
assignments in7 matches are same-name locals/globals/comments in other scopes,
not automatically proof that a particular cache is mutable. Every matched input
is a literal or macro stringification. Preserve first-lookup ordinal order while
making these caches generation-aware, and classify separately manually initialized
NAMEKEY_INVALID window IDs and excluded GameSpy/MOTD paths. Existing StaticNameKey
tests do not execute every source cache getter. The LOAD_TEST_ASSETS development
loader remains a separate source-classification/retirement obligation; do not
claim immutable metadata lookup while its template/module creation path is active.
Source links: Common/Thing/ThingFactory.cpp::newTemplate/newOverride/addTemplate,
ThingTemplate.cpp::copyFrom/ModuleInfo::addModuleInfo,
ModuleFactory.cpp::newModuleDataFromINI/makeDecoratedNameKey,
Common/Module.h::MAKE_STANDARD_MODULE_DATA_MACRO_ABC/MAKE_STANDARD_MODULE_MACRO.

### Template/module cohort: corrected admission under validation

The literal-key census contained five commented-out declarations: 175 live
initializers in 59 source/header files, not 180 live consumers. Those live caches
now keep StaticNameKey backing and resolve a same-type local at the original
declaration point. This preserves first-use acquisition order and invalidates
values after namespace reset/rollback; manually memoized invalid-ID slots remain a
separate obligation. Archive compilation is not execution of every getter.

The base ModuleData creator lives in Common/System/NativeModuleData.cpp, with its
ordinary allocation guarded through actual INI parsing. The shared creator macro
uses the same ordinary matching-delete boundary. Factory-returned data is guarded
through tag acquisition and owning-vector growth; missing template lookups roll
back speculative namespace entries. This per-acquisition guard does NOT yet own
the suffix acquired by an entire later-rejected object definition.

Decorated keys retain the exact original one-character bucket prefix followed by
the complete module name; dynamic preparation removes the old 256-byte strcpy
limit. ModuleFactory entry points admit Int buckets, including ignored/empty-name
cases, rather than synthetic invalid values of the unfixed ModuleType enum. The
enum, source bucket values and class layouts are unchanged. No enum ABI adjustment
or external format change is inferred from the internal method signature change.

Supporting evidence: frozen 466-path SHA
`777c496d4f4ba72b61f7cffb500f5a99d6660cd274a4aae09577497e5cfc340e`
builds the six actual Common/Thing providers, related bootstrap/logic/definitions,
and generated base/shared-macro ownership fixture. Tests in
tests/original/module_data.cpp distinguish actual base-provider execution from a
generated shared-macro data shape. They cover late semantic rejection, complete
allocation manifests, same-owner retry, three lifetimes, all valid bucket prefixes,
long names and defined raw invalid buckets. Actual targets built all3; after
relinking affected fixtures38/38 related cases pass normal GCC1.12s/GCC
ASan-UBSan-LSan9.57s/Clang sanitizers6.69s. Independent creator/key censuses9/12/3
include every rejection/retry and exact terminal through three lifetimes.
No whole factory/query/runtime
acceptance follows until its owner graph and execution are completed.

### Template payload ownership and definition-wide module-data candidates

Actual AudioArray copies formerly leaked completed slots if a later pooled event
copy failed, and assignment to an absent source slot dropped an existing owner.
The corrected owner builds an entirely initialized local array and publishes via
pointer-only swap; self-assignment remains a no-op. AudioEventRTS's data
constructors/copies/name identity are owned once by NativeAudioEventData.cpp;
playback remains in the original AudioEventRTS.cpp. Initialize inactive position,
owner-ID backing and playback-phase enum explicitly; generatePlayInfo still
selects its unchanged authored start phase before playback.

SparseMatchFinder entries borrow elements of one vector, not a clone's distinct
backing. Copy construction begins with an empty derived cache and non-self
assignment clears it allocation-free; self-copy retains its owner. Source query
selection/tiebreakers remain unchanged. Frozen468 SHA
`d7121e35c0a41259b289bb0086a89b5433e200024b0f194c1589146e78742f23`
builds selected actual providers all3 and passes45/45 related cases normal GCC
1.49s/GCC ASan-UBSan-LSan13.87s/Clang sanitizers9.45s. Audio prefix/terminal
proofs use a fixed nine-unit pool and retain all three failure/retry pairs in each
constructor/assignment family, through three lifetimes. See
tests/original/template_backing.cpp and tests/original/graph_headers.cpp.

NativeModuleDataTransaction copies only the owning pointer vector into a
prospective backing. Its prior vector borrows accepted pointer values. Rejection
deletes every new suffix unit in reverse order and restores the original vector
backing without allocation; committed nested acquisitions remain owned by an
outer rejection. The actual ModuleFactory list uses this exact alias, but the
whole Object parser has NOT yet adopted the transaction: it must retire its
unpublished template graph first, before withdrawing borrowed data. Never attach
suffix retirement to the existing early-published parser in isolation.

ThingTemplate payload assignment also copies Overridable's owning next link
before later fallible fields. Its data copy helper now guards/restores the
destination's owned link on success and failure, so an unpublished clone cannot
delete an accepted source override. Constructor/copy/destructor helpers live
once in NativeThingTemplateData.cpp, with actual GeometryInfo in the runtime
archive. Eight scalar defaults were missing from the original constructor:
asset scale, display color, bridge/forbidden flags, armor/weapon default-copy
flags, buildable and editor sorting. Their explicit zero defaults preserve the
original allocateBlockImplementation's zeroed physical backing; authored INI
values and subsequent constructor assignments still override them normally.
The parent GlobalData must exist before its occlusion-delay dereference.

Supporting validation at frozen470 SHA
`bac5627b2d9505b7bd0faf119dadcf3dcfaf6288887e67f470c23246e450ed3a`:
actual data-owner clones and coupled suffix transactions pass49/49 related cases
normal GCC1.81s/GCC ASan-UBSan-LSan16.19s/Clang sanitizers11.04s. Independent
clone/data-vector censuses3/6 retain all faults/retries and exact terminals through
three lifetimes. Actual templates preserve source overrides, destination identity
and exact residuals; nested data rollback restores accepted pointer-vector backing.
Native data extraction initially exposed missing authoritative
Radar/Shadow includes and the shared -999 skill-point sentinel, then the generated
fixture used the read-only TheGlobalData macro as a publication lvalue. Corrected
providers use the actual headers/shared constant and actual writable publication
owner TheWritableGlobalData; no replacement values, globals or gameplay providers
were introduced. The direct fixture also initially omitted GlobalData's required
FileSystem parent: a generated debugger throw stack identified that guard, not a
template assertion or timeout. Corrected fixtures provide an actual rooted
FileSystem and NativeUserStorage with generated-only roots; the production guard
was not weakened. The unchanged KindOf table and ProductionPrerequisite's actual
constructor/destructor/init are runtime data providers; its gameplay lookup methods
remain in their source TU. Whole factory registration, Object parser publication,
resolveNames cross-template effects, manual GUI IDs and development TestArt
retirement remain pending in the same N2 slice.


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

## Mapped-image graph representation and publication (N2; validation pending)

The baseline `GameClient/System/Image.cpp` initializes every texture/UV/image-size
slot. Coords are signed Int32 edges; UV divides each edge by its corresponding
nonzero texture dimension and retains raw coordinates for zero dimensions.
Status parsing swaps the current width/height when ROTATED_90_CLOCKWISE is set;
it is an ordered parser operation, not a declarative post-parse normalization.
Representable negative differences remain source behavior. Compute differences
in Int64 before Int32 admission to avoid signed overflow at extreme edges.

Image owns its name/filename values, but its raw texture pointer is borrowed:
the actual destructor does not release it. The original mapped-image parser
diagnoses replacing an image with non-null raw backing as invalid. Preserve that
rejection; do not invent ownership transfer or silently reinterpret the texture.
Collection map entries own pooled Images, while ThingTemplate and UI consumers
borrow their addresses. Successful reload therefore must preserve accepted Image
addresses, not merely equal names or a replacement map's cardinality.

The original load order is gated user INI discovery, selected texture-size
directory, then HandCreated. User recursion occurs only when a top-level user
INI exists. Directory selection loads sorted root files before sorted nested
files. The native rooted filesystem returns normalized slash identities: never
derive unchecked relative offsets from the caller's unnormalized spelling.
Validate complete listing identities before parsing the first selected block.

The current implementation clones complete accepted payloads into an offside
collection under a namespace transaction. Complete parse and commit-ledger
validation precede allocation-free payload/pointer/index exchange; old candidate
backing retires after publication. Per-definition parsing similarly holds a pool
guard through all fields and index admission. Misses do not consume keys and
direct duplicate adds cannot overwrite owned Images. Preview creation now holds
its Image pool guard, but preview copying/writer retirement remains pending.

Source references: `GameClient/System/Image.cpp`, `Common/INI/INIMappedImage.cpp`,
`Common/INI/INICore.cpp::loadDirectoryBlocks`, `GameClient/MapUtil.cpp`;
generated execution contracts are `tests/original/configuration.cpp::images*`.
These are source-verified representation/ownership findings, not yet validated
image acceptance or whole GameEngine startup evidence. All tests use generated
CPU metadata only; no proprietary image or physical renderer is involved.

Mapped-image validation is now PASS for canonical493 SHA
`3245ad677bb5d8216ddd1a90566bb0fae0f0cd43190e559352384cfb1215b605`:
full156/156 all3, GCC16.46s/GCC sanitizers74.81s/Clang sanitizers61.30s.
Actual `images`, `images-negative`, `images-fault-new`, `images-fault-replace`,
`images-fault-collection` pass all13/10/54 registered allocation ordinals, exact
terminals and three lifetimes. Full and focused logs are recorded in
`evidence/qa/N2-runtime-support.md`. The existing user attachment freezes asset
mount publication: generated fixture remounts must withdraw/rebind that actual
owner outside the tested transaction. A local INI owns filename backing, so its
construction precedes allocation residual baselines. Both fixture corrections
retain strict production/resource checks. Whole startup and preview writers
remain unaccepted; these generated tests do not read/render supplied images.

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

## List widget selection protocol (N2 widget batch; acceptance pending)

`GadgetListBox.cpp::GLM_GET_SELECTION` has two distinct output owners: single
selection writes Int, multi-selection returns borrowed Int* backing. Original
GameSpyChat/Chat/WOLLobby callers explicitly pass the address of an Int* through
an Int* cast. It is not an array-copy protocol. Native admission must distinguish
the typed outputs before writing and cannot widen a write into a scalar caller.
The borrowed array expires at selection backing replacement or window teardown.

Selection lists use -1 termination and allow every row to be selected. Original
allocation of exactly listLength Ints leaves no terminal slot at full capacity.
Original removeSelection copies overlapping storage and reads beyond that array;
row delete/scroll also use overlapping memcpy. Cell/row/selection arrays acquired
with NEW[] have several scalar-delete paths, though destroy already uses delete[].
Row deletion iterates <=columns; scrolling reverses the survivor test, and leaves
duplicated tail owners after movement. These are coupled original source defects,
not reasons to change the library allocator or gameplay. Resize stores capacity
in signed Short, so admission must precede narrowing, and both replacement arrays
must exist before accepted storage is retired. Display callback failure/whole UI
publication remains pending, separate from generated metadata ownership evidence.

Source: GameClient/Gadget.h::ListboxData, GUI/Gadget/GadgetListBox.cpp selection,
delete/scroll/reset/resize/destroy owners; GameNetwork/GameSpyChat.cpp::sendChat
borrowed-output caller. Existing NativeInputSettings provides SDL double-click
policy; IME completion transports character U+000D, not a keyboard scan code.

Generated selection/backing support now PASS all3 on canonical516 SHA
`b4d36c32d2ec7249d7e157be8278ba68e6c8bb8e37b3171179938b5f0da2318a`;
full161/16116.36s/74.47s/61.09s. `tests/original/list_selection.cpp` executes
actual ListboxData/shared production helpers, including exact two-allocation
backing manifest, every rejection/retry, full sentinel capacity, zero/max/max+1,
late-invalid inputs and allocation-free overlap-safe row insertion. The typed
multi-output API uses an explicit message tag, and scalar output never receives
pointer backing. Borrowed outputs do not acquire ownership. Complete widget
archive compilation resolves66 root-link symbols179→113, no new symbols exposed.
Window construction, row text/image publication, callback failure, empty visible
row queries, manual names and whole UI graphs remain pending; generated metadata
tests do not accept those owners or whole startup. Do not equate the complete
support matrix with N2 gameplay/scenario acceptance.

## Coupled transfer/recorder source boundary (N2; implementation pending)

`XferSave`/`XferLoad` ordinary strings prefix ASCII with UnsignedByte (255 bytes)
and Unicode with UnsignedByte (255 Windows UTF16 units), no trailing terminator.
`XferDeepCRC` uses UnsignedShort for ASCII and source cap16385, but Byte for
Unicode cap255. Deep-CRC and ordinary ASCII limits are different protocols.
`XferBase::xferUnicodeString` currently hashes native wchar_t backing, which is
four bytes on Linux versus two in the source executable. CRC word grouping is
per transfer call; chunking the same byte sequence differently can change it.
Complete words rotate/add after htonl; leftover bytes are assembled low-byte-first,
then htonl is applied before addCRC applies htonl again. Preserve both operations,
not a substitute hash or a generic byte-at-a-time update. Current complete-word
reads dereference potentially unaligned UnsignedInt pointers and need defined
fixed-width loads. These are source findings, not accepted checkpoint parity.

Save beginBlock writes an Int32 placeholder before allocating its pooled stack
node; endBlock withdraws that node before unchecked seek/backpatch succeeds.
Save/deep-CRC destructors call potentially throwing close. Raw fopen can truncate
accepted data before transaction success. Load beginBlock returns0 on a short
read, skip may seek past EOF, raw reads can partially overwrite output, and static
text buffers share process backing. `XferBase::xferVersion` validates only after
the implementation call, so output poisoning must cover typed rejection and
snapshot exceptions as well as I/O. Load snapshot notification is fallible until
actual GameState postprocess admission returns; XO_NO_POST_PROCESSING is a real
public option, not permission to invent a replacement global. Source backing,
snapshot ownership and namespace recovery need complete surrounding owner tests.

Sources: Common/System/Xfer.cpp, XferSave.cpp, XferLoad.cpp, XferCRC.cpp and
Common/System/SaveGame/GameState.cpp::addPostProcessSnapshot; Common/Recorder.cpp.
NativeUserStorage/NativeAtomicOutput already provide protected user streams,
bounded backpatch and atomic publication with truthful durability results.
Provider compilation/diagnostic census does not accept raw writer behavior.

The parallel default adapters serialize ID/science collection cardinality as
UnsignedShort but previously narrowed size() without checking, and publish each
loaded element before later reads/allocations finish. BitFlags named loading
clears accepted state first and lacks signed count bounds. Its CRC path hashes
sizeof(this), a pointer width; on native 64-bit that both differs from the original
32-bit executable and still truncates masks wider than one word. Canonical complete
logical masks need explicit initialized UInt32 words, excluding std::bitset's
platform word backing/padding. This is a source diagnostic bug; named save grammar
and actual flag/gameplay values remain unchanged. Sources: Xfer.cpp typed collection
adapters; Common/BitFlagsIO.h::BitFlags::xfer; Common/BitFlags.h backing. These
findings and the new protected-stream implementation remain pending acceptance;
only the first five generated transfer families have passed normal GCC so far.

Typed collection loading now constructs an unpublished candidate and swaps only
after every read/allocation succeeds. ObjectID vector/list and Int list retain
the original empty-destination requirement; science vectors deliberately replace
existing creation-time sciences only on complete success. Science names are
dynamic NameKey identities: ScienceStore::getScienceFromInternalName maps through
NameKeyGenerator rather than searching registered science definitions. Do not
invent a registration-only rejection rule. Scalar science publication waits for
the existing invalid-name check; whole namespace rollback still needs integration
evidence. Upgrade masks similarly retain accepted bits until all names resolve.

Aggregate transfers preserve their original per-field call grouping (including
CRC) while LOAD uses initialized offside backing. Matrix3D uses its public
identity initializer, not empty Vector constructors, then publishes after all
twelve fields load. Generated tests cover late/truncated fields and corrected
same-owner retries. Canonical mask CRC emits one complete initialized sequence
of UInt32 words after the existing version prefix, with bit i in word i/32 at
position i%32; unused trailing bits are zero. Both named BitFlags and the actual
upgrade-mask CRC adapter use this representation. These implementations and the
broader collection/mask/aggregate fixtures are pending GCC/Clang sanitizer and
full-suite validation; do not infer whole GameState/Recorder acceptance from them.

The next surrounding replay owner graph is source-verified, not accepted:
Recorder::readReplayHeader currently opens directly into m_file, writes caller
header fields incrementally, and resets/parses/publishes m_gameInfo before the
local-index field is admitted. Its constructor calls init before parent startup
and its destructor does not close a live stream or retire CRCInfo. Playback
publishes CRCInfo before the remaining difficulty/mode/rank/FPS reads finish.
appendNextCommand publishes the pooled GameMessage into TheCommandList before
reading player identity, argument-type runs and complete payload. Separate parser
and message guards are required until full parse/semantic admission succeeds;
analysis and suppressed control-message branches must retire a message exactly
once. GameMessageParser's copy-from-message constructor acquires linked pooled
run nodes through fallible addArgType calls; a failed constructor does not run
its destructor. The parser constructor and teardown must guard the entire run
graph, not only individual allocation calls.

Replay header prefix is six bytes GENREP followed by source Windows time_t values
(four-byte signed epochs), frame duration UInt32, byte booleans and MAX_SLOTS
disconnect bytes. Current native sizeof(time_t) changes every later offset and
must not become format authority. NUL-terminated Unicode strings are UTF16LE,
not fwprintf/fputwc on a byte-oriented stream or native Linux wchar_t. GameInfo's
option grammar is semicolon-delimited key/value text; its parser and retained
LAN/skirmish provider must be inspected together before replay publication. It
includes excluded online headers, resets seed through GetTickCount and relies on
actual global data/map services. These are dependencies to port, not permission
to fabricate globals or relax full original-runtime reachability.
Sources: Common/Recorder.cpp constructor/destructor/readReplayHeader/playbackFile/
appendNextCommand; GameNetwork/GameMessageParser.cpp constructors/addArgType;
GameNetwork/GameInfo.cpp reset/ParseAsciiStringToGameInfo. All whole-replay/session
ownership evidence remains pending.

Native transfer service layering: all contextual map, science-name, named-upgrade
and postprocess operations now use `NativeTransferServices`, a complete borrowed
callback binding registered by actual GameState after successful init. Withdrawal
compares raw owner identities before any cast/dereference. Incomplete/conflicting
registrations preserve the prior binding and require no allocation. Actual
GameState callbacks still call the original stores/codecs/postprocess list; no
fake singleton or alternate gameplay provider is introduced. Low-level CRC/raw
transports no longer link the entire parent game merely through sanitizer RTTI
metadata. Missing contextual providers reject and poison; public
XO_NO_POST_PROCESSING still permits deliberately standalone generated loading.

Generated dispatch tests prove service admission/withdrawal, failures inside map
encode/decode and snapshot notification, offside science/upgrade adapter loading,
the complete10-allocation generated science candidate manifest, exact terminal
and same-owner retry. Generated codecs in that service-contract fixture are not
evidence for actual ScienceStore/UpgradeCenter/GameState semantics. Their parent
initialization, namespace rollback and full replay/scenario integration remain
pending. No sanitizer check was disabled to repair link reachability. Sources:
NativeTransferServices.h/.cpp; GameState::init/destructor callbacks; Xfer.cpp;
XferLoad.cpp; tests/original/transfer.cpp::services.

Persistence support checkpoint now PASS all3: canonical521 SHA
`ef3674d727e43eb5208d8447ad775bf6a01055b4fc3784ba7818f3e80a924467`,
focused10/10 and full171/171 (17.11s/77.32s/63.30s). Exact generated science
manifest is10 under each compiler and all three same-process lifetimes; complete
ordinal/terminal/retry proofs are retained. `evidence/qa/N2-runtime-support.md`
records source/test/log provenance. This supersedes earlier pending support
validation notes above, not the pending whole GameState/store/replay/scenario
requirements. All sanitizer checks and upstream dependencies remain unchanged.

Retained LAN/replay player names use UTF8, not the host locale or Latin1:
GameInfo calls the game-owned ThreadUtils helpers; their original implementation
uses CP_UTF8 in both directions and replaces CR/LF with spaces on decode. That
generic text utility remains required despite its GameSpy directory name. The
original buffers can be unterminated after failed conversion and are released
with scalar delete despite NEW[]. Native replacement must build whole standard
string candidates, admit scalar/UTF8 validity and preserve single-line conversion.
GameInfo's byte-budget truncation must retain complete UTF8 scalars and reject an
already exhausted fixed option budget rather than looping forever on empty text.
Source: GameNetwork/GameSpy/Thread/ThreadUtils.cpp; GameInfoToAsciiString and
ParseAsciiStringToGameInfo. This finding is source-verified; native codec/whole
GameInfo acceptance is pending.

Message ownership has a legitimate detach/traverse protocol: Network.cpp keeps
using msg->next()/prev() after CommandList::removeMessage. Do not reset those
links when withdrawing ownership. Fresh message construction initializes them;
new append rewrites them for its new owner. MessageStream::propagateMessages
hands its complete list to CommandList::appendMessageList while entries still
identify the source stream. This is an explicit transfer, not ordinary admission
of a second owner. The game-owned metadata transfer withdraws the original list
and appends to the destination using qualified, allocation-free base operations,
without cleanup notifications. Ordinary duplicate/foreign append/removal rejects
without effects. Sources: MessageStream.cpp propagation/command-list handoff;
Network.cpp remove/traverse caller; GameMessageValues.cpp original value/list
provider; tests/original/message_parser.cpp transitions. Validation pending.

Replay-value support now PASS all3 on canonical525 SHA
`7a83f843ed89e20f1a279bd26cacbbe88b4dbf84984fbce9d69c05375cb836e0`;
focused13/13/full174/174 (17.31s/79.35s/64.55s). Prior native parser/node/enum
sizes/alignments and member offsets remain unchanged: admission signatures use
raw Int, then convert only admitted values; no extra instance fields are added.
The bounded existing node ledger establishes total Byte argument headroom before
pool acquisition. Actual constructor failures retire every partial run and the
construction owner; all three pooled boundaries/terminal/retries and repeated
lifetimes are covered. This supersedes pending value/text/metadata support notes,
not whole GameInfo/Recorder/GameState/native-scenario acceptance. See support QA
for source/test/log provenance and exact exclusions.

Recorder command source protocol: frame UInt32 precedes command type Int32,
player Int32, Byte run count, every (Byte type, Byte length) pair, then all run
payloads in original order. Argument widths are Int/Real/object/drawable/team/
timestamp four bytes, Bool one, location twelve, pixel eight, pixel-region
sixteen and WIDECHAR two. Unlike a UTF16 string, WIDECHAR is an individual source
code unit; preserve isolated surrogate units rather than introducing scalar-only
admission. The current Linux native wchar_t copy widens that field incorrectly.
No new finite-float rejection is justified by this source transport alone.
Source: Recorder.cpp::writeToFile/writeArgument/appendNextCommand/readArgument;
MessageStream.h argument union and command enum. The source reader publishes the
message before reading its run table/payload, and analysis can delete an already
suppressed null message. Complete offside ownership must precede publication;
checked whole-command bytes must precede the first recording write. Source
findings only; native command/session acceptance pending.
Command support now PASS all3 on canonical526
`f4c89360ac10a483c852f823b928d335bd89b01dec5be4ac04a361e76a9d5cc8`:
focused15/15/full176/176, source-authored98-byte oracle/all argument families,
all truncations, accepted-list no-effect negatives, analysis/suppression overlap,
zero-allocation complete output and source-pool manifests owner1/arguments11,
exact terminals/retries/three lifetimes. This supersedes pending command codec
support only. Session/header/context/whole native runtime remain pending;
see N2-runtime-support.md for logs and limitations. No private input executed.

Protected replay session support now PASS all3 on canonical527
`42461aba003d4547c7e5541645a293a2c84ac24a23e69927432d8a9eb0a9be3b`:
focused17/17/full178/178. Source four-byte signed epochs place end at10,
frame duration at14, desync at18, quit at19 and disconnect bytes at20.
Unicode header writers pair bounded UTF16LE readers, including supplementary
pairs; active output is a protected atomic candidate, never raw user/asset stdio.
Record/playback manifests14/13 prove complete ordinals/terminal/retry/three
lifetimes and exact resource residuals; see QA for source/test/log provenance.
This supersedes pending protected-session/epoch/string support, not complete
GameInfo/CRC/RNG/world publication or whole native runtime acceptance.

Next whole GameInfo owner findings (source verified, unaccepted): ordinary
copy/swap of GameInfo would copy its borrowed m_slot array into the wrong owner;
preserve each parent slot identity and transfer complete slot payloads instead.
Base setters called by ParseAsciiStringToGameInfo copy base GameSlot payloads,
then perform fallible map path/open/lowercase work and local map-availability
queries. LANGameInfo's local-slot virtual method uses each LAN slot's isLocalPlayer,
which compares its IP to actual TheLAN->GetLocalIP, not base m_localIP. Do not
replace those contexts with a generic base clone or fake LAN provider.
GameSlot::reset initializes all primitive fields except m_IP (and deliberately
does not reset its name); fresh construction must initialize IP separately,
while preserving accepted reset semantics. GameInfo reset does initialize
m_preorderMask, and preserves derived constructor-owned local IP intentionally.
Recorder playback currently publishes mode/world clear before header admission,
CRC/global map/interval before trailing startup settings and RNG/new-game message
around final publication. InitRandom owns three six-word client/audio/logic arrays
and a separate base logic seed: protecting only GetGameLogicRandomSeed is not a
complete RNG rollback. Sources: GameInfo.cpp reset/setSlot/setMap/setMapCRC/
setMapSize/ParseAsciiStringToGameInfo; LANGameInfo.cpp getLocalSlotNum/
LANGameSlot::isLocalPlayer; Recorder.cpp::playbackFile; RandomValue.cpp seed owners.
These are implementation obligations for the next coupled owner batch, not
accepted behavior or a replacement milestone plan.

Game setup publication now has actual-owner generated evidence on frozen533
SHA c207ad5ad97eea1cf017a27aa6eefd99930822d5d58418cd990b6ff2f5ad6f58
(537 including toolchain:63bb8fb6f6beae5e1801fea4ed86cb2c2947c7f955d8ebc95772d413a0531832).
NativeGameInfoTransaction captures base option/flag values and each real slot's
payload/borrowed identity, not a copied GameInfo with another parent's slot links.
Rejection restores exact name/map backing and every scalar without callbacks or
allocation; inner acceptance remains owned by an outer journal. Fresh GameSlot
initializes IP while accepted reset retains IP/name as the original intended.
Localized slot names are prepared before scalar effects. Whole reset/clear/start/
map/CRC/size/adjust publication and the options commit/replay-header owner use
the same journal; virtual local-slot queries retain their original parent.

GameInfo::setMapSize intentionally uses map CRC for availability, not filesize.
This source behavior is retained and tested. Seven companion bits are map1/
preview2/map.ini4/map.str8/solo.ini16/assetusage32/readme64. Original setMap's
mixed slash/backslash loop loses the directory, and FileTransfer helpers also
search backslashes only (root-level companions get an absolute-leading separator).
Actual shared path definitions now accept both separators and avoid inventing
an absolute root-level path; generated actual setMap reaches mask127. The unchanged
NET_CRC_INTERVAL default moved once into the shared setup provider; MapCache::findMap
moved once into its logical metadata provider. Presentation and replay/snapshot
methods remain actual separate providers in the bootstrap archive, not fixture
substitutes. This permits full sanitizer instrumentation without retaining
unexecuted parent methods through an unrelated setup-only source TU.

Generated source-owner manifests: combined mutation83; direct map63, CRC2, size2,
reset1, clear8, adjust3, start1. Every ordinal, exact terminal and failure/retry
pair retains accepted pointers/backing and exact allocation/descriptor residuals
through three lifetimes under all3. Focused3/3 and full181/181 PASS
(17.82s/81.90s/66.68s); source/log provenance is in N2-runtime-support.md.
Whole options/actual LAN/replay-header callback execution, Recorder startup,
Skirmish snapshot graph, native root and GameLogic scenario remain unaccepted.

Recorder startup surrounding lifecycle finding: GameLogic::clearGameData calls
GameEngine::reset, which dispatches SubsystemInterfaceList::resetAll including
the actual Recorder. Its reset/init retires file/CRC and resets game setup.
Do not merely move clearGameData after parsing into this same Recorder: that
would retire the admitted candidate. Prepare replay session/setup/tail settings,
CRC/message resources and filename outside the reset-owned Recorder, finish input
admission before destructive world reset, then adopt the complete candidate without
fallible copies into the original owner. Runtime world-reset failure is not
reversible via metadata/RNG snapshots; never claim accepted gameplay from a
partially reset world. Engine reset itself creates a blank-window layout and can
fail, so it needs truthful failure handling at the actual parent integration gate.
Full RNG publication owns all three six-word arrays and the base seed.
Source evidence: GameLogicDispatch.cpp::clearGameData; GameEngine.cpp::reset;
Recorder.cpp::reset/init/playbackFile. These are verified next-batch obligations,
not new accepted runtime or extra milestones.

Recorder preparation implementation now uses a separately scoped actual
RecorderClass and adopts ReplayGameInfo payloads without swapping embedded slot
pointers. Candidate file, CRC, complete startup settings, first frame and pooled
messages are acquired before destructive reset. Actual post-reset PlayerList
supplies message context; clean EOF retains source CLEAR-before-NEW ordering.
Reset exceptions mark the actual engine quitting and propagate. This code is
compiled, not yet accepted through whole Recorder/runtime execution.

NativeReplaySession::nativeReplayReadStartupSettings reads four little-endian
32-bit words (difficulty, original mode, rank points, max FPS). Reject difficulty
outside EASY..COUNT-1 and excluded/invalid native modes before enum conversion.
Rank/FPS remain source signed Int values; nonpositive FPS retains source policy.
The session guard poisons short/invalid reads. Generated fixture covers all 16
short prefixes, invalid raw encodings, every supported difficulty/mode pair,
corrected retries, prior immutable input, and wrong-mode output preservation.
Actual ReplayGameInfo adoption tests retain both owners' embedded slot identities,
complete payload/backing, allocation-free swap and whole-owner retirement.

## Native startup provider continuation (N2; whole runtime still pending)

Actual BuildAssistant/FunctionLexicon/TerrainVisual/VideoPlayer providers now
compile in the original archives. Typed NativeWindowCallback constructors are
constexpr and all seven actual tables enforce constinit: original callback tables
initialize as data rather than unconditional
dynamic startup initializers. A constinit fixture proves all four pointer families
and sentinel preservation. This does not omit any live callback; GUI table init
still requires its actual implementations when reached.

WorldHeightMapInterfaceClass's four source methods now live in a thin common
header, and original map scales live in Common/Terrain.h. MapObject and
TerrainVisual consume the same declarations. Importing MapObject merely for this
interface had pulled gameplay type metadata into the isolated filter fixture;
the fix removes that dependency, not sanitizer checks or parent type identities.

Actual DomeStyleSeismicFilter preserves cosine dome, source square write extent,
9.0 cap and 1.5 gravity subtraction. Its workspace is initialized RAII storage;
radius/coordinate/size calculations are defined and malformed active input rejects
before writes. Generated grid oracles cover radius1..3/life1..14, zero/expired
life, no input, nonfinite/overflow boundaries, all25 sample and36 write callback
failures, allocation ordinal0/terminal1 and corrected retries through three
lifetimes. Callback write failure can leave preceding writes: this is workspace
ownership evidence, not world-state rollback or actual WorldHeightMap acceptance.

BuildAssistant source follow-through remains required: init publishes size before
array acquisition; wall-position resize deletes accepted backing before replacement;
sell-list load clears pointers without retiring nodes and publishes each pooled
node before complete input/list admission; start-selling mutates construction
before pooled/list acquisition; destructor does not retire sell nodes. Correct
these coupled owner paths with actual game graph tests, not an isolated fake
GameLogic. Sources: BuildAssistant.cpp::init/destructor/xferTheSellList/
buildTiledLocations/sellObject.

MapObject constructor/destructor and bridge render references reside in actual
GameEngineDevice/WorldHeightMap.cpp, not a Common MapObject.cpp. Bridge render
references are used by W3DBridgeBuffer, so do not classify them as editor-only or
discard intended visual behavior. Its constructor calls reference replacement on
bridge slots without first initializing them: prepare every slot before any
fallible helper/reference operation when separating CPU map ownership. Source
WorldHeightMap parses maps and owns tile/height data; a generated velocity grid is
not a substitute for that owner. Next native CPU map/source-provider work must
preserve serialized properties, linked object ownership and bridge semantics.
### CPU MapObject extraction checkpoint (not whole-map acceptance)

The actual definitions formerly in device `WorldHeightMap.cpp` now live in
`Common/System/MapObject.cpp` and `MapObjectTeams.cpp`. The common header uses
the extracted original bridge enum and height-velocity interface, not renderer
implementation headers. Original bridge enum values and their recorded hash
are unchanged. The device bridge caller supplies public retain/release hooks.
Each map attachment captures one acquired reference unit; equal pointers in
five slots require five releases. All attachment members initialize before
fallible property construction. Default properties and all-ID assignment prepare
offside dictionaries and commit the name namespace only after admission.
Normal source fixtures cover defaults, reverse ID order, linked-node/pool
retirement and complete allocation failure/retry manifests. See
`evidence/qa/N2-runtime-support.md` for exact evidence and limitations.
The initial sanitizer links failed through team metadata, not incorrect template
provider selection. Linker archive admission occurs before section GC; the
uncalled validate entry point admitted the team translation unit and its entire
SidesList/AI/GameLogic graph. Moving that complete entry point to its team owner
resolved both links without suppressing checks. Seven actual MapObject families
now execute under both sanitizers, including actual nonnull ThingTemplate binding,
two-step live override resolution, duplicate borrowed identity and final-template
ID naming. Generated temporary data parents preserve borrowed publications through
retirement; borrowed stack override links detach before original destructors.
Whole team validation and CPU WorldHeightMap acceptance remain pending.

### Startup callback/shared metadata continuation

Actual `Common/BitFlags.cpp` defines both model-condition and eight-bit armor
names. The missing `BitFlags<8>::s_bitNameList` root symbol was the armor table,
not a missing weapon provider (weapon sets have seventeen flags). Keep source
domains separate and use their actual tables, not a generic replacement list.

`OnlineChatColors` is shared INI/UI metadata despite its former placement in
GameSpy/Chat.cpp. Exact enum, 27 defaults and field-to-slot table now live in the
thin `GameClient/OnlineChatColors.h` and `Common/INI/INIOnlineChatColors.cpp`.
Original transport includes the same declaration and no longer defines them.
Hashes lock all three source bodies to174a2946. Block parsing works on a complete
offside array and publishes all colors only on success. Configuration tests cover
mapping/untouched siblings, unknown/wide/missing fields and eight allocation
failure/retry pairs plus exact terminal8, over three lifetimes. This is block
admission, not a claim of transaction rollback across several unrelated INI blocks.

The four source control-bar, tooltip, chat and quit callback TUs compile natively
using explicit GlobalData imports, native monotonic milliseconds, a non-keyword
requirements-text local and defined pointer comparison. Their whole UI behavior
has not executed. Tooltip static borrowed-window/wait state and unsigned clock
deadline addition still need original UI lifecycle/rollover validation; compilation
is not acceptance of those paths.

`FixupScoreScreenMovieWindow` and its original blank-layout publication are
extracted once into `GUI/NativeScoreScreenState.cpp`. The original score screen
retains its shared declaration; no movie-state substitute was introduced.
Six borrowed network/setup/input publications move once to the production
`NativeStartupPublications.cpp`. Actual original owners still create/retire them;
no dummy owner or typeinfo was added. Source census proves single definitions.
Full Diplomacy/ScoreScreen TUs still import absent GameSpy Peer/GP headers and
remain uncompiled, while their independent shared data is reachable. Online
transport is outside the native LAN contract. Actual in-game diplomacy and score
behavior still require source separation from that transport, not removal/stubs.
Root14 normal and227/237 sanitizer symbol inventories are diagnostics only;
the exact frozen cohort and executed36-test support are recorded in QA.

### Native mod admission and write ownership

Source `ArchiveFileSystem.cpp::loadMods` loads an explicit mod BIG first with
overwrite enabled, then directory BIGs with overwrite enabled. Device
`Win32BIGFileSystem.cpp::loadBigFilesFromDirectory` requests recursive discovery
and iterates the sorted FilenameList. The original FileSystem checks local loose
files before archives. A directory mod is therefore an archive override source,
not authority to add a new loose-file precedence rule. Native
`FileSystem::mountReadOnlyMods` keeps that selection and publishes a complete
offside index only after the attached storage's prospective Data/Cache roots are
admitted against the candidate. Base remount still forbids an active attachment.
The explicit mod file and whole mod directory become read-only asset boundaries.

Parent-directory admission alone does not protect a mod BIG stored inside the
user data directory: atomic rename or unlink could mutate the protected leaf.
`NativeUserStorage::beginWrite/removeFile` now admit the full prospective physical
target; copy/scratch use the same writer. Output owners also retain a shared
namespace lease. Any mount mutation rejects until all output owners retire,
including already committed scratch owners whose destructor may unlink backing.
The token has no owner callback and can outlive FileSystem/storage; it introduces
no serialized data or allocation/deallocation crossover. Discovery/lease checks
are synchronous startup/storage operations, not render-thread callbacks.

GameEngine no longer acquires duplicate Local/Archive platform services: its
existing native FileSystem owns both providers. The legacy TGA-to-DDS authoring
path generated files in CWD and invoked a converter writing Art/Textures. That
path is removed, and a requested legacy asset update fails explicitly. This is
not native decoding/cache conversion acceptance. CommandLine still needs native
mod binding and broader argument/provider integration; complete startup remains
pending. Generated mod fixtures cover selection, malformed/root rejection, every
mutation path, leases and complete allocation/retry ownership. Current executed
results belong in `evidence/qa/N2-runtime-support.md`, not this source finding.

### Original CommandLine and pending statistics integration

Actual `Common/CommandLine.cpp` keeps the original option table byte-for-byte
(SHA2565c4bb5b78b547b75c4e2340693b7a4a685d1dd84cb3e681c6d4a4905d397f6cc).
Release handlers retain their original effects, including quickstart's sizzle/
shell/window-animation selection. Numeric values now use defined Int-width
decoding, not overflowing atoi. Active release value options are validated
before dispatch; unknown flags still skip. FullVersion consumes its value before
the following option. Whole unrelated-option rollback is not claimed. Relative
mods use the explicit user namespace; absolute user-selected mods remain read-only
inputs. Both mod metadata publications have a captured no-allocation rollback
guard through native archive admission. Raw argv/path logging is retired.

Native simulation owns floating-point setup; the former DX preservation toggle
has no backend variable. Its direct handler admits preserve1 and rejects changes
that would disable preservation. Debug asset reports still have downstream CWD
writers: the request now rejects explicitly rather than creating files or
claiming success. Complete debug-authoring/RunAhead and runtime semantic range
validation are pending; encoded integer representability is not proof of valid
whole-game configuration. Release command fixtures bind actual GlobalData and
native storage, not fake configuration or a replacement argument table.

`Common/StatsCollector.cpp` still is not compiled/admitted. Its constructor reads
the live GameLogic frame. Reset publishes a filename and writes a header before
zeroing counters; collectUnitCountStats adds to current counts. Update computes
the interval with signed frame addition/multiplication, then updates counters
before writing. End opens one append stream and invokes another append writer
before writing the footer. All three writers use raw fopen and ignore write/close
errors; debug path generation may include LAN player names and a caller-selected
base directory. Native integration must bind the whole logging lifecycle to
captured protected storage, preserve intended TSV field/row/footer order and
sampling behavior, and make elapsed-frame arithmetic defined. Do not just replace
one fopen or execute the provider before its ownership/metadata boundaries are
settled. Actual GameLogic/player sampling and debug/LAN naming are separate from
isolated output ownership evidence; no current StatsCollector acceptance exists.

### Protected original statistics lifecycle (current source)

The preceding raw-writer findings describe the pre-integration source. The actual
collector now compiles in `original_runtime_common`; its default constructor and
real GameLogic/object/player adapter compile separately in
`Common/System/NativeOriginalStatsSource.cpp`. The adapter preserves source
infantry/vehicle eligibility, neutral/Civilian exclusion, local/other partition,
and money/configuration inputs. Generated tests supply explicit synchronous
inputs to the actual collector; they do not execute or accept whole simulation.

Reset, row and footer publication use captured NativeUserStorage and streamed
atomic replacement, never retail writes or raw CWD writers. Counter/frame state
advances only after publication; directory-sync uncertainty is published state,
not rollback. A failed new-session reset withdraws row/footer admission until a
successful reset, preventing new-world samples from entering an old report.
Reset clears scroll/unit baselines; final samples precede one ordered footer.
Frame differences use defined unsigned arithmetic, interval multiplication uses
64-bit values, and unsafe filename bytes encode reversibly without changing
report metadata. Absolute directories must belong to captured user storage.

Borrowed sampler/storage dependencies are synchronous; destruction has no
simulation callback or I/O. Allocation failures unwind candidates; optional
storage failures expose output status. Actual default-adapter execution,
debug/LAN naming and whole GameLogic/scenario acceptance remain pending.
Current evidence: `evidence/qa/N2-statistics-checkpoint.md`.

### Fatal reporting and LOD capability source dependencies

`Common/System/Debug.cpp` implements both ReleaseCrash entry points using Win32
dialogs, raw reason/stack reports and user-path file rotation; localized reporting
also calls the text singleton. Ordinary ReleaseCrash could return during shutdown
if GlobalData was withdrawn, despite GameEngine's explicit nonreturning contract.
The native provider in `Common/System/NativeFatal.cpp` instead emits one fixed
redacted stderr report and immediately exits failure without allocations,
simulation globals, localization callbacks, filesystem mutations or atexit.
Closed/broken stderr does not prevent failure exit. Reasons/identifiers are not
read, because they can contain private roots or refer to retired owners. This
boundary is not restart/teardown or whole-runtime acceptance.

The remaining capability entry point is defined in device-owned
`W3DShaderManager.cpp::testMinimumRequirements`, not common GameLOD. It reports
legacy CPU classes from CPUDetect, RAM/speed and W3D chipset, then returns true;
when all three benchmark outputs are supplied, it calls RunBenchmark. This tree's
Benchmark directory contains its header and project, but none of the project-
listed nbench/misc/sysspec implementations. GameLOD invokes hardware discovery
before GameClient initialization; it additionally writes raw Benchmark.txt on
force-benchmark and substitutes P3/1000 when selecting unknown-CPU profiles.
Native capability results, optional report ownership and legacy-profile mapping
must be settled together. Do not manufacture benchmark acceptance or accepted
modern renderer capability from old profile ordinals. These dependencies remain
pending, alongside full diplomacy/start-spot menu decoupling and original startup.

Before admitting the remaining GUI roots, review their complete lifecycle:
`SkirmishGameOptionsMenu.cpp::positionStartSpots` dereferences mapWindow on its
missing-map path, caches UnknownMap Image pointers statically across collection
lifetimes, copies MapMetaData, and iterates player count into fixed MAX_SLOTS
button backing. `positionStartSpotControls` divides by map extents and reads
preceding buttons without sparse-array checks; screen-coordinate rectangles are
compared with locally computed gadget coordinates. `updateMapStartSpots` checks
the source-slot button but then writes the selected-position button, which can
be a different null entry. Admit counts, extents, positions and target pointers
together while preserving map aspect/projection and apparent-versus-real slot
selection. These are source findings, not executed defect reproduction.

`Diplomacy.cpp` keeps layout/window/animation/control pointers as static state.
Hide releases control pointers; Reset destroys layout windows through InGameUI
and clears only its layout/window/animation members directly. Preserve legitimate
parent identity through cleanup, withdraw all borrowed controls, and test reset/
hide/populate order across whole owner lifetimes. Full GameSpy-dependent menus
remain uncompiled; their callback definitions must not be replaced with returning
no-ops to make the startup link green.

### Explicit FunctionLexicon metadata and dependency ownership

The original seven GUI table/declaration payloads now live in
`Common/System/FunctionLexiconTables.cpp`, source-locked byte-for-byte by
`tests/toolchain/test_original_lexicon_tables.py` (SHA256
c5070a715576d4bd607502f9bbf987efd98bd8c0f81b6a08e683db5af53b4d3d).
Default construction still captures every original GUI table and requires their
real providers; no callback is replaced/deleted. Generic lookup/init/reset and
destruction remain in FunctionLexicon.cpp. Borrowed TheFunctionLexicon publication
is independent in NativeStartupPublications.cpp, not a menu-loader side effect.

The actual owner also accepts explicit bounded table descriptors, copied at
construction while entry backing remains borrowed. This supports native/headless
service ownership with explicitly empty metadata, not apparent GUI initialization.
Init validates complete candidate tables before key/span publication; optional
device draw/layout tables preserve paired admission and device-first lookup.
Rejected cold registration may retain source-monotonic interned names, but no
unpublished entry keys/table spans publish; whole name-owner destruction retires
those legitimate units. External mutation/retirement of admitted entry backing
is not an owner rollback guarantee. Valid backing must outlive its borrower.

Before separation, GCC/Clang sanitizer module constructors in the combined lookup/
table TU kept the full uninvoked GUI metadata closure in the startup diagnostic.
Instrumentation is unchanged. Current startup link still fails6/8/15 symbols;
default GUI constructor diagnostics still fail212/214/222, proving missing menu
providers were not silently replaced. All probes are link-only, never execution
or startup acceptance. Remaining sanitized roots include native IME/warning/
terrain/network-status and diplomacy briefing/Toggling paths; inspect them as
coupled providers rather than replacing them with constants.

### Original map-preview placement and slot visibility

The skirmish menu's placement/update entry points are now compiled separately in
`GameClient/GUI/NativeMapPreview.cpp`; LAN, online map selection and LoadScreen
still call the same source entry points. The old single-control helper was only
an internal caller of that removed block, not an independent external provider.
Actual window/image/gadget/text calls remain; no visual or GUI success stub is
introduced. Nonnull GUI execution and parent/window/image retirement still await
the complete native factory/GUI providers, not acceptance from CPU fixtures.

`NativeMapPreviewLayout` prepares an entire candidate before publishing arrays/
marker lists. It retains original Real aspect-fit intermediates/truncation,
letterboxing, inverted Y, integer half-marker offsets, push_front supply/tech
ordering and ordered start collision adjustment. Integer conversion/addition,
dimensions, extents, counts and used coordinates are checked together. Collision
coordinates are consistently local, and rectangles use each preceding control's
actual size rather than comparing local positions to screen positions. Sparse
controls and selected-position targets are checked; absent source-slot controls
do not suppress labels on an existing destination. Duplicate destinations retain
the original last-slot-wins order. Missing candidate waypoints reject, not assert
and continue. No metadata/window/image pointer is retained by the CPU owner.

The ineffective previous-metadata cache is removed. Missing maps/views withdraw
marker state; fallback UnknownMap is looked up in the current collection every
time, eliminating the old cross-collection static pointer. Actual window userdata
still borrows MapCache backing and must be withdrawn before its parent retires.
CPU admission is transactional, but GUI callbacks may be fallible after it: do
not claim complete message rollback or physical presentation acceptance.

The original copy helper had raw source/file/buffer acquisitions that leaked on
multiple exits, and native VFS correctly refuses its WRITE open. The replacement
`NativeMapPreviewStorage.cpp::nativeCopyMapPreview` guards the actual File view,
streams bounded blocks into captured NativeUserStorage, and commits one complete
file. Any prepublication allocation/I/O/input failure preserves the prior file
and removes temporary backing; failed directory sync after rename retains a
complete published file. Source assets are never write targets. Preview names
require a .map suffix; prerequisites are checked before deriving paths, and
portable separators encode into the local preview leaf. Failed optional copying
does not publish Image metadata for nonexistent backing. Generated opaque-byte
copy tests do not validate TGA decoding or physical preview drawing.

`GameSlotVisibility.cpp` retains the three original scalar apparent-value getters
and the shared source alliance predicate; translated names remain in
GameInfoPresentation.cpp. The alliance predicate checks borrowed game membership,
local index and local slot before dereferencing (the original fetched slot -1
before rejecting it). Local/team/observer visibility and hidden original opponent
random selections are unchanged. Missing publications/identity are not allies.
MultiplayerSettings singleton publication is independent of INI metadata; all
metadata/default providers remain compiled and instrumentation is unchanged.

Evidence: `tests/original/map_preview.cpp`, `evidence/qa/N2-map-preview-checkpoint.md`.
This is actual shared CPU/storage/slot ownership, not complete original startup,
world simulation, GUI rendering, or whole map-preview image lifecycle acceptance.

### Original diplomacy briefing and presentation ownership

`GUI/NativeDiplomacyBriefing.cpp` owns the actual GetBriefingTextList and
UpdateDiplomacyBriefingText entry points independently of optional presentation.
The list object retains its address, source order and case-sensitive deduplication;
clear-plus-add stages a complete candidate. ResetDiplomacy retains entries, while
the original InGameUI world-reset path explicitly clears them. The exposed mutable
list pointer remains the source save/load protocol, not a transaction guarantee
for arbitrary external mutations. Clear process-held entries before retiring the
original memory manager.

A borrowed main-thread view is captured only after successful initial replay.
Ordinary updates append one entry; clear or a previously failed callback requests
full replay. A partially failing callback does not publish model state. Corrected
retry (including a duplicate/empty update) repairs the presentation from accepted
state. Conflicting/reentrant capture or mutation rejects; detach cannot retire an
active callback and never calls the borrowed view. Destruction has no callbacks.

Actual `GUICallbacks/Diplomacy.cpp` remains the GUI provider. Its layout and eight
animation helpers are guarded during acquisition. Required controls are scoped
to the owned root; legitimate UI/manager identities are captured. Reset withdraws
briefing capture, controls and IDs, retires animation/rest-position borrowing
before windows, and drains original deferred window destruction with legitimate
parents published. Borrowed publication is then restored without allocation.
`GameWindowManager::retireDestroyedWindows` performs cleanup only, not an input
or simulation frame. Actual callbacks must satisfy their nonthrowing teardown
contract. Failed Show retires owned GUI state for retry; this is not an atomic
rollback guarantee for every gadget callback. Nonnull GUI execution is pending.

GameSpy buddy controls are hidden and their excluded Internet service is not
replaced; actual LAN/skirmish diplomacy remains. Observer color uses the original
observer palette selector rather than treating packed ARGB as a palette index;
reoccupied rows explicitly unhide source controls. These source bug corrections
have no physical-render acceptance yet.

`AnimateWindowManager` guards all eight actual ProcessAnimate acquisitions and
pooled registration/list insertion. Failed registration restores prior window
position and timed duration. Explicit captured viewport metadata preserves the
Spiral helper's original integer-half radius; default construction still requires
the actual Display. GCC constructor failure/retry terminal8 passes normally and
under sanitizers; Clang ownership and registered-window lifecycle remain pending
native providers. Do not infer their acceptance from briefing observer tests.

`DisconnectMenuLifetime.cpp` supplies the actual empty destructor, borrowed
manager attachment and initialized SCREENOFF state (formerly undefined before
init), including genuine RTTI. TheLAN now publishes the existing abstract
LANAPIInterface instead of the Windows concrete owner. Existing LookupPlayer and
GetLocalIP queries join its pure contract; no fake LAN, sockets or network
acceptance are supplied by this type correction.

Evidence: `tests/original/diplomacy.cpp`, pending `tests/original/animation.cpp`,
and `evidence/qa/N2-diplomacy-checkpoint.md`. N2/slice03 remains incomplete.

### Native presentation boundary cohort and remaining capability gate

`NativePresentationState.cpp` owns the actual source script entry points.
doSkyBoxSet retains the exact optional WritableGlobalData field mutation, now
outside the Windows water renderer. oversizeTheTerrain dispatches through a
required TerrainVisual::oversizeTerrain operation; the original W3D device
forwards to its original terrain object. No object means no visual operation,
as in the original free provider, not accepted output. The base renderer's
oversize method is intentionally empty for flat terrain; concrete HeightMap/
FlatHeightMap overrides remain source authority. Nonnull native dispatch and
physical terrain acceptance are pending, not inferred from type checks.

GameClient now obtains optional IME ownership through its device factory, using
the same NativeServiceOwners transaction. Interactive implementations must
supply real input; a headless device may explicitly omit presentation/input.
The original W3D device calls the original CreateIMEManagerInterface factory.
No fake IME object or fabricated composition/candidate state is introduced.
This removes the Windows input factory from the common startup closure without
accepting native text/IME event transport. Actual GameClient device startup
still remains required. The in-game /host command only reported excluded GameSpy
QR2/thread state; its obsolete diagnostic branch is removed, not implemented as
zero-valued pretend hosting. LAN/game chat paths remain source code.

`NativeWarningBox.cpp` resolves actual GameText labels and owns complete UTF-8
request backing until synchronous borrowed presentation returns or throws.
AsciiString::translate only truncates wchar_t to bytes in the original source;
it is not a Unicode encoder and is deliberately not used for SDL dialog text.
Invalid/missing prerequisites or labels reject before output. Defined source
buttons are checked both before presentation and on return. Public SDL3
SDL_ShowMessageBox is the actual backend, on SDL's main thread; unavailable
output returns OSDBT_ERROR, never invented OK. Modal scope is platform-managed;
original icon constants overlap, so error/stop precedence is explicit. Physical
dialogs, parent-window integration and non-English button labels remain N4 gates;
the current adapter uses English OK/Cancel labels, not locale acceptance.

The source audio loop previously retried indefinitely if the warning backend
returned ERROR instead of CANCEL. Retry now requires actual OK; cancellation
remains source behavior and unavailable output emits a fixed redacted diagnostic.
This does not claim missing music is loaded; GameEngine's separate actual music
readiness check remains. No raw localized text, selector or SDL error is logged.

Generated real CSF/GameText fixtures verify non-ASCII/supplementary characters,
captured request lifetimes, missing/invalid inputs, callback failure with retry,
all six allocation prefixes and exact terminal across three complete owners.
Original file pools warm during retired discovery; live owner baselines then
hold exactly and whole memory-manager shutdown retires warmed metadata exactly.
The observer does not display a dialog or substitute a gameplay service. Actual
skybox state executes against GlobalData, including withdrawn publication;
typed device contracts and original source forwarding are also checked.

Startup diagnostics now expose only testMinimumRequirements on all three
toolchains. The absent Benchmark implementation is not replaced by fabricated
scores. Its header defines int RunBenchmark(int,char*[],float* float,float* integer,
float* memory); the project lists nbench0.c, nbench1.c, emfloat.c,
misc.c and sysspec.c, absent here. Native metadata, source LOD preset comparisons,
wide RAM representation, explicit benchmark capability/error and protected
optional report output must be resolved together before actual startup.
`GameLODManager::init` ignores capability returns, writes Benchmark.txt into CWD
and maps unknown CPU via these legacy score units. `findStaticLODLevel` also
ignores capability failure and maps unknown chipset to TNT2; neither fallback
proves native renderer capability. N2's actual startup/scenario remains pending.

Evidence: `evidence/qa/N2-native-boundaries-checkpoint.md`,
`tests/original/native_warning.cpp`, actual configuration and source-contract tests.

Capability follow-on source audit: LOD is not exclusively renderer state.
`SlowDeathBehavior.cpp` reads getSlowDeathScale at activation and during update;
when no non-LOD effects are present, zero can immediately destroy the original
Object and nonzero values can alter retirement timing. Preserve original dynamic
LOD state/scale/guards while changing hardware discovery or recommendation.
Do not substitute an arbitrary quality profile to make a headless fixture pass.
GameLODManager::isReallyLowMHz also affects audio stream/sample limits through
applyStaticLODLevel. Native clock/memory/profile changes therefore need actual
source consumer coverage, not only query-helper tests.

The original CPUDetect Intel decoder only recognizes selected family6 models
through0xb and family0xf as Pentium4; newer CPUs are not proven P4 merely by
vendor or x86-64 support. Its AMD decoder is likewise era-specific. Preserve
unknown classification rather than asserting a modern chip is an old benchmark
equivalent. CPUDetect returns unsigned physical bytes while GameLOD stores Int;
native hardware discovery must not wrap RAM above signed32 capacity. No binary
save/network field for that hardware-derived member has been identified; verify
all consumers before widening. The native capability bridge must distinguish
measured facts, unavailable legacy calibration, and renderer suitability.

### Native LOD capability bridge (current, not full-startup acceptance)

Actual `GameLODManager` now captures a borrowed synchronous `NativeLODProbe`
and protected `NativeUserStorage`. `NativeLODProbe.cpp` measures POSIX physical
RAM with checked 64-bit multiplication and optionally reads the Linux CPU maximum
frequency in kHz, converting to MHz. Neither frequency nor vendor establishes a
legacy CPU class: native class stays XX; equivalent historical scores and chipset
classification stay unavailable. A normal unclassified machine retains current
game/user quality without persisting a made-up ideal profile. Explicit forced
legacy calibration fails with an actionable error if equivalent scores are absent.
Generated fixture scores validate the retained comparison algorithm, not native
performance or renderer capability.

The five original LOD block handlers and existing Int/Real INI representations
remain. Table loading, preferences and hardware/calibration acquisition occur in
a candidate under legitimate borrowed singleton publication; precommit failure
restores the original owner. Reports use atomic protected `Benchmark.txt` output,
not CWD. Required preference-read failure rejects initialization; optional report
publication failure is distinguished from complete publication with uncertain
durability. Real nonnull device callback rollback is still a separate pending gate.
Unknown preset names, invalid domains, count overflow, nonpositive divisors and
nonfinite score/scale values reject before unsafe indexing or division.

Only the hardware-derived internal RAM quantity is widened; no serialized member
consumer was found. Original dynamic thresholds, truncation, masks and death
scales are retained. Previously uninitialized owner particle priorities now use
the existing source LOWEST default; initial scale remains 1, not the configured
HIGH scale until a source transition occurs. Generation counters use defined
unsigned 32-bit wraparound, retaining mask bits without signed-overflow UB.
`applyStaticLODLevel` still compares requested texture reduction with the owner
cache before writing GlobalData: requesting the initial cached zero does not
overwrite an independently initialized GlobalData texture value. Tests follow
that existing source behavior rather than inventing a visual oracle.

`tests/original/lod.cpp` exercises real GameLOD/INI/GlobalData/FileSystem and
protected storage, repeated whole-owner destruction, complete allocation and I/O
failure/retry sweeps and exact terminals. I/O discovery establishes the protected
namespace first: an absent directory otherwise skips the preference-file open,
making a later warmed manifest one operation longer. Full startup, native automatic
tuning and nonnull renderer/audio behavior are not established by these tests.
See `evidence/qa/N2-lod-checkpoint.md`.
