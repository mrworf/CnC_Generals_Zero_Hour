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
