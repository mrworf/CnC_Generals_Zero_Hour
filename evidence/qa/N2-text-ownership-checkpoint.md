# N2 shared text/UI ownership checkpoint

Implementation progress in existing N2 plan01/slice03, not slice or milestone
acceptance. Parent: `f26a7b4c419cc05176468baf100b2ce46b5068b5`.

## Frozen source and scope

Canonical633:
`3d78803c12684934d0e6f1d83b084df388b91775cb332b7cf5d14b155a491ee6`.
Toolchain-inclusive640:
`cf08696bfd511a952b332c62ba6c219d00197ac286554376f1e2e4ed98f44724`.
Identity uses sorted changed/new source paths relative to `9e5c0e4f`, pathname,
NUL and binary file SHA256; documentation/state are excluded.

Actual shared source FontLibrary, SuperweaponInfo, NamedTimerInfo, InGameUI
admissions/retirement and GameClient destructor execute. Pooled candidates and
partially acquired backing retire before publication/admission. New superweapon
keys stage their list before insertion; existing lists retain their entries on
failure. Named timer replacement stages a complete candidate before pointer
exchange. Reset and shutdown share paired map retirement; client-owned strings
retire before font backing with legitimate parent publications.

The new adapters are explicitly test-only: display metrics, rendering, group/
formation output and view creation reject. Font backing is an instrumented
game-owned allocation, not FreeType. This is not native text, physical output,
full UI initialization, public player/science admission or complete snapshot
restoration acceptance. Framework/vendor sources and assets were not modified.

## Required validation

All three full configured builds PASS. Debug GCC, GCC ASan/UBSan/LSan and Clang
ASan/UBSan/LSan use the normal host, existing leak detection/halt options and
TIMEOUT60, with no suppressions or coverage changes. Each family repeats three
whole text/UI/client lifetimes and checks actual typed used units, standard live
allocations and whole memory-manager retirement.

| Configuration | Focused8 | Related153 |
| --- | --- | --- |
| GCC | PASS, 3.96s | PASS, 145.51s |
| GCC ASan/UBSan/LSan | PASS, 18.04s | PASS, 647.20s |
| Clang ASan/UBSan/LSan | PASS, 11.25s | PASS, 389.52s |

Configured suite:343 tests. Full343 is **not run**; it remains a completion gate.
The related matrix includes all eight new families plus existing camera, actor,
team/player, pathfinder, waypoint, terrain, logical world, water/client, diplomacy,
statistics, path, map/chunk, runtime/core, enum and source-boundary families.

Commands: `cmake --build build/original-core-<configuration> -j <jobs>`;
`ctest --test-dir build/original-core-<configuration> -R '^original_text_ownership_' --output-on-failure`;
related regex:
`^original_(text_ownership|script_camera|object_ownership|team_ownership|player_ownership|ai_pathfinder|waypoint_graph|native_terrain|logical_world|native_water|native_client|diplomacy|statistics|header_paths|map|chunk|runtime|core|enum|source_boundaries)`.
Focused iteration builds only the affected target/dependencies; full configured
builds and related matrix run after source freezes. Independent sanitizer matrices
use separate build/test directories and generated resources.

## Complete failure and retry evidence

All3 focused runs independently prove matching exact terminals and retain each
failure prefix/corrected retry in `tests/original/text_ownership.cpp`:

- Font standard allocations: prefixes[0,1), terminal1.
- New timer map admission: prefixes[0,1), terminal1.
- Existing timer replacement: terminal0, with the original font/cache and pools
  warmed; no artificial allocator demand is introduced.
- New superweapon key/list admission: prefixes[0,2), terminal2.
- Existing-key superweapon list admission: prefixes[0,1), terminal1.
- Timer string operation callbacks: prefixes[0,4), terminal4, every retry.
- Two-string superweapon constructor callbacks: prefixes[0,8), terminal8,
  every retry, including failure after acquiring the first or both strings.
- Whole source-client text cohort: prefixes[0,18), terminal18, every retry.
- Actual GameFont, NamedTimerInfo, SuperweaponInfo and OwnershipText capacity8
  pools with no overflow: each exhaustion/retry, including both first- and
  second-string acquisition exhaustion. Earlier candidate units retire exactly.
- Partial font backend FALSE/throw, including both UI candidate callers; null
  display-string result; invalid player index; accepted timer identity survives.
- Shared reset helper is idempotent; shutdown without reset retires both maps.
  Manager-owned live strings in the actual GameClient destructor still observe
  valid backing, then all font resources retire.

## Retained development failures and corrections

1. Initial fixture compile used the pooled macro's private class-pool accessor.
   Corrected all three sites to inspect the actual public factory registry.
   Preflight also corrected parent construction order so GlobalData has its
   FileSystem parent before construction, and moved test labels before baselines.
   Corrected focused7 PASS, 3.44s.
2. Extended whole-client checks omitted the declaration header for DrawGroupInfo.
   Added the actual original header; no production stub or behavior change.
3. Extended backend FALSE/throw fixtures failed all eight families during their
   common warm-up: the test replaced the UI constructor's independently owned
   font-name literal with an equal borrowed configuration value after capturing
   the standard-allocation baseline. That legitimately retired one string owner.
   Normalize the same accepted configuration identity before the baseline.
   Production was unchanged; all exact ownership/pool checks remain enabled.
   Corrected focused8 PASS all3 as above. This repeatable baseline lesson is in
   AGENTS.md. No assert-driven production patch, timeout or sanitizer waiver.

Local logs: `/tmp/n2-text-build-*.log`, `/tmp/n2-text-full-build-*.log`,
`/tmp/n2-text-focused-*.log`, `/tmp/n2-text-related-*.log`; complete CTest output
includes each repeated manifest. This durable artifact preserves results and
corrections without requiring those disposable logs or private retail inputs.

## Remaining outcome gates

N2/slice03 remain incomplete. Native stock FreeType/Fontconfig backing, original
local-font precedence/Unicode selection/source metrics, concrete camera eye/
projection/lock, legitimate UI/client update order and actual GameEngine/GameLogic
startup/frame are still required. Source snapshot version1/2/3 traversal and
negative timer counts need explicit actual-wire tests and corrections documented
in `docs/original-engine-formats.md`; safe per-record admission alone does not
establish whole-snapshot restoration. Continue the root child/script/RNG journal,
CPU model-query providers, shipped fog/geometry/walls/bridges and compiler-matching
simulation in the same existing plan. Prior creation/camera checkpoints remain
supporting evidence, not whole-game acceptance. No new tiny plan was created.

## Stock font public-API preflight (separate diagnostic)

GCC and Clang ASan/UBSan/LSan both PASS, three lifetimes, FreeType runtime2.14.3
and Fontconfig21803 (2.18.3). Real memory-backed glyph and supplied font-set
matching, explicit configuration ownership and acquired aliases retire cleanly.
No FcFini, suppression, current-config replacement or vendor patch. This is not
the native game backend or source metric/retail-font parity acceptance.

Control SHA256: `3f1947faf2d9c19ee4b94cda64354ac3c069505a4f539f728f9ee9d4883ffd80`.
Copy this self-contained diagnostic to a disposable path and compile with
`c++` and `clang++`, `-std=c++20 -g -fsanitize=address,undefined -fno-omit-frame-pointer`
and `pkg-config --cflags --libs freetype2 fontconfig`. Run each executable with
`ASAN_OPTIONS=detect_leaks=1:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`. It reads only stock host-font
bytes, emits no private selectors, and does not depend on retail content.

```cpp
// Disposable stock-public-API preflight; not game/native text acceptance.
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
void require(bool ok) { if(!ok) throw std::runtime_error("stock font control rejected"); }
struct Library {
  FT_Library value=nullptr;
  Library() { require(!FT_Init_FreeType(&value)); }
  ~Library() { if(value)FT_Done_FreeType(value); }
};
struct Face {
  FT_Face value=nullptr;
  ~Face() { if(value)FT_Done_Face(value); }
};
using Pattern=std::unique_ptr<FcPattern,decltype(&FcPatternDestroy)>;
int main() {
  try {
    int major=0,minor=0,patch=0;
    for(unsigned repeat=0;repeat<3;++repeat) {
      Library library;FT_Library_Version(library.value,&major,&minor,&patch);
      std::unique_ptr<FcConfig,decltype(&FcConfigDestroy)> config(FcInitLoadConfigAndFonts(),FcConfigDestroy);
      require(bool(config));
      Pattern query(FcPatternCreate(),FcPatternDestroy);require(bool(query));
      require(FcPatternAddString(query.get(),FC_FAMILY,reinterpret_cast<const FcChar8*>("sans-serif")));
      require(FcPatternAddInteger(query.get(),FC_WEIGHT,FC_WEIGHT_NORMAL));
      require(FcConfigSubstitute(config.get(),query.get(),FcMatchPattern));FcDefaultSubstitute(query.get());
      FcResult result=FcResultNoMatch;
      Pattern match(FcFontMatch(config.get(),query.get(),&result),FcPatternDestroy);
      require(bool(match) && result==FcResultMatch);
      FcChar8* filename=nullptr;int index=0;
      require(FcPatternGetString(match.get(),FC_FILE,0,&filename)==FcResultMatch);
      require(FcPatternGetInteger(match.get(),FC_INDEX,0,&index)==FcResultMatch && index>=0);
      std::ifstream input(reinterpret_cast<const char*>(filename),std::ios::binary|std::ios::ate);
      require(bool(input));const auto length=input.tellg();require(length>0 && length<64*1024*1024);
      std::vector<FT_Byte> bytes(static_cast<std::size_t>(length));input.seekg(0);
      require(bool(input.read(reinterpret_cast<char*>(bytes.data()),bytes.size())));
      Face face;require(!FT_New_Memory_Face(library.value,bytes.data(),bytes.size(),index,&face.value));
      require(!FT_Set_Char_Size(face.value,0,10*64,96,96));
      require(!FT_Load_Char(face.value,'W',FT_LOAD_RENDER));
      require(face.value->glyph->bitmap.width>0 && face.value->glyph->bitmap.rows>0 && face.value->glyph->advance.x>0);
      require(!FT_Reference_Face(face.value));Face alias;alias.value=face.value;
      std::unique_ptr<FcFontSet,decltype(&FcFontSetDestroy)> fonts(FcFontSetCreate(),FcFontSetDestroy);
      require(bool(fonts));
      Pattern local(FcFreeTypeQueryFace(face.value,reinterpret_cast<const FcChar8*>("generated-memory-font"),index,nullptr),FcPatternDestroy);
      require(bool(local));require(FcFontSetAdd(fonts.get(),local.get()));local.release();
      FcFontSet* sets[]{fonts.get()};
      Pattern memoryMatch(FcFontSetMatch(config.get(),sets,1,query.get(),&result),FcPatternDestroy);
      require(bool(memoryMatch) && result==FcResultMatch);
      FcPatternReference(memoryMatch.get());Pattern patternAlias(memoryMatch.get(),FcPatternDestroy);
    }
    std::cout<<"PASS stock font public API control; FreeType "<<major<<'.'<<minor<<'.'<<patch
      <<"; Fontconfig "<<FcGetVersion()<<"; repeated3; memory face, real glyph, supplied set and acquired aliases\n";
    return 0;
  } catch(...) { std::cerr<<"FAIL stock font public API control\n";return 1; }
}
```
