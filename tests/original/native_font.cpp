// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "GameClient/NativeFontLibrary.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace {
void require(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
template<class F> void rejects(F operation) { bool failed=false;try { operation(); }
  catch(ErrorCode) { failed=true; }catch(std::bad_alloc&) { failed=true; }
  require(failed,"invalid native font operation rejected"); }
template<class T> struct Publication {
  T*& slot;T* prior;Publication(T*& target,T& value):slot(target),prior(target) { slot=&value; }
  ~Publication() { slot=prior; }
};
struct Tree {
  std::filesystem::path path;
  Tree() { char pattern[]="/tmp/zh-native-font-XXXXXX";auto* result=::mkdtemp(pattern);
    require(result,"generated font fixture");path=result; }
  ~Tree() { std::error_code error;std::filesystem::remove_all(path,error); }
};
void writeFont(const std::filesystem::path& path,const char* family,int advance) {
  std::ofstream out(path);
  out<<"STARTFONT 2.1\nFONT -generated-"<<family<<"-medium-r-normal--13-100-96-96-p-50-iso10646-1\n"
    "SIZE 10 96 96\nFONTBOUNDINGBOX 3 5 0 0\nSTARTPROPERTIES 10\n"
    "FONT_ASCENT 5\nFONT_DESCENT 0\nFAMILY_NAME \""<<family<<"\"\n"
    "WEIGHT_NAME \"Medium\"\nSLANT \"R\"\nPIXEL_SIZE 13\nPOINT_SIZE 100\n"
    "RESOLUTION_X 96\nCHARSET_REGISTRY \"ISO10646\"\nCHARSET_ENCODING \"1\"\n"
    "ENDPROPERTIES\nCHARS 2\n";
  for(int code:{87,937})out<<"STARTCHAR generated"<<code<<"\nENCODING "<<code<<
    "\nSWIDTH 500 0\nDWIDTH "<<advance<<" 0\nBBX 3 5 0 0\nBITMAP\nA0\nA0\nE0\nA0\nA0\nENDCHAR\n";
  out<<"ENDFONT\n";require(bool(out),"generated font written");
}
struct Hooks {
  std::size_t count=0,fail=std::numeric_limits<std::size_t>::max();bool hit=false;
  static void step(void* context) { auto& self=*static_cast<Hooks*>(context);
    if(self.count++==self.fail) { self.hit=true;throw std::bad_alloc(); } }
  void arm(std::size_t ordinal) { count=0;fail=ordinal;hit=false; }
  void disarm() { fail=std::numeric_limits<std::size_t>::max(); }
};
struct Context {
  Tree tree;FileSystem files;Publication<FileSystem> filesLease{TheFileSystem,files};
  GlobalLanguage language;Publication<GlobalLanguage> languageLease{TheGlobalLanguageData,language};
  AsciiString primary="GeneratedPrimary";
  Context() {
    writeFont(tree.path/"primary.bdf","GeneratedPrimary",5);
    writeFont(tree.path/"unicode.bdf","GeneratedUnicode",7);
    {std::ofstream invalid(tree.path/"malformed.bdf");invalid<<"not a font";require(bool(invalid),"generated malformed font written");}
    files.mountReadOnly({tree.path.string()});
    // Backend configuration fixture; full original Language.ini init is not
    // claimed by this test. Both files go through the real read-only VFS.
    language.m_localFonts.push_back("primary.bdf");language.m_localFonts.push_back("unicode.bdf");
    language.m_unicodeFontName="GeneratedUnicode";
  }
};
void glyphs(NativeFontLibrary& fonts,Context& context) {
  auto* font=fonts.getFont(context.primary,10,FALSE);require(font,"original font admission");
  require(font==fonts.getFont(context.primary,10,FALSE),"original cache identity");
  const auto latin=fonts.readGlyph(font,L'W');const auto retained=latin.coverage;
  const auto unicode=fonts.readGlyph(font,WideChar(937));
  require(latin.local && !latin.alternate && latin.advance==6,"supplied primary and source W spacing");
  require(unicode.local && unicode.alternate && unicode.advance==7,"supplied alternate chosen above255");
  require(latin.width==3 && latin.height==5 && latin.coverage.size()==15 && latin.coverage==retained,"owned bitmap geometry");
  require(latin.coverage==std::vector<std::uint8_t>({255,0,255,255,0,255,255,255,255,255,0,255,255,0,255}),"actual supplied mono coverage");
  require(font->height>0 && latin.lineHeight==font->height,"real font metrics");
}
void functional(Context& context) {
  NativeFontLibrary fonts;fonts.init();require(fonts.localFaceCount()==2,"both local metadata faces");
  glyphs(fonts,context);fonts.init();glyphs(fonts,context);
  fonts.reset();require(fonts.initialized() && fonts.localFaceCount()==2,"reset retains library and local metadata");glyphs(fonts,context);
}
void negative(Context& context) {
  NativeFontLibrary fonts;rejects([&]{fonts.getFont(context.primary,10,FALSE);});fonts.init();glyphs(fonts,context);
  for(int point:{-1,0,101})rejects([&]{fonts.getFont(context.primary,point,FALSE);});
  rejects([&]{fonts.getFont("",10,FALSE);});
  auto* font=fonts.getFont(context.primary,10,FALSE);
  for(WideChar code:{WideChar(-1),WideChar(0xd800),WideChar(0x110000)})rejects([&]{fonts.readGlyph(font,code);});
  rejects([&]{fonts.readGlyph(nullptr,L'W');});
  GameFont* foreign=newInstance(GameFont);rejects([&]{fonts.readGlyph(foreign,L'W');});foreign->deleteInstance();
  context.language.m_localFonts.push_back("missing.bdf");NativeFontLibrary retry;
  rejects([&]{retry.init();});require(!retry.initialized(),"missing local data never publishes initialization");
  context.language.m_localFonts.pop_back();retry.init();glyphs(retry,context);
  context.language.m_localFonts.push_back("malformed.bdf");NativeFontLibrary malformed;
  rejects([&]{malformed.init();});require(!malformed.initialized(),"malformed local backing never publishes initialization");
  context.language.m_localFonts.pop_back();malformed.init();glyphs(malformed,context);
}
void host(Context& context) {
  NativeFontLibrary fonts;fonts.init();
  auto* regular=fonts.getFont("Arial",10,FALSE);auto* condensed=fonts.getFont("Generals",10,FALSE);
  auto* bold=fonts.getFont("Arial",10,TRUE);
  const auto plain=fonts.readGlyph(regular,L'M');const auto narrow=fonts.readGlyph(condensed,L'M');
  const auto heavy=fonts.readGlyph(bold,L'M');
  require(!plain.local && !narrow.local && !heavy.local && !plain.coverage.empty() && !narrow.coverage.empty() && !heavy.coverage.empty(),"stock host matching and real glyph backing");
  require(narrow.advance<plain.advance,"source Generals requested average width is applied");
  glyphs(fonts,context);
  // This proves the public API mapping, not Windows raster/metric parity.
}
void pools(Context& context) {
  Hooks hooks;NativeFontLibrary fonts(Hooks::step,&hooks);fonts.init();
  std::vector<GameFont*> held;
  struct Retire { std::vector<GameFont*>& records;~Retire() { for(auto* font:records)font->deleteInstance(); } } retire{held};
  held.reserve(8);for(unsigned n=0;n<8;++n)held.push_back(newInstance(GameFont));
  const auto live=AllocationFault::live();hooks.arm(std::numeric_limits<std::size_t>::max());
  rejects([&]{fonts.getFont(context.primary,10,FALSE);});
  require(hooks.count==0 && AllocationFault::live()==live,"exhaustion occurs before native acquisition");
  held.back()->deleteInstance();held.pop_back();glyphs(fonts,context);fonts.reset();
  require(TheMemoryPoolFactory->findMemoryPool("GameFont")->getUsedBlockCount()==7,"corrected pooled admission retires its record");
}
void nativeFaults(Context& context) {
  Hooks hooks;std::size_t census;
  {NativeFontLibrary discover(Hooks::step,&hooks);discover.init();census=hooks.count;}
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    NativeFontLibrary fonts(Hooks::step,&hooks);hooks.arm(ordinal);bool failed=false;
    try { fonts.init(); }catch(std::bad_alloc&) { failed=true; }
    const bool hit=hooks.hit;hooks.disarm();require(failed==hit && failed==(ordinal<census),"every init native acquisition and exact terminal");
    if(failed)require(!fonts.initialized(),"failed native init remains unpublished");
    fonts.init();glyphs(fonts,context);
  }
  std::cout<<"native font init terminal "<<census<<"; every failure/retry\n";
  hooks.arm(std::numeric_limits<std::size_t>::max());NativeFontLibrary fonts(Hooks::step,&hooks);fonts.init();
  hooks.arm(std::numeric_limits<std::size_t>::max());fonts.getFont(context.primary,10,FALSE);census=hooks.count;fonts.reset();
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    hooks.arm(ordinal);bool failed=false;try { fonts.getFont(context.primary,10,FALSE); }catch(std::bad_alloc&) { failed=true; }
    const bool hit=hooks.hit;hooks.disarm();require(failed==hit && failed==(ordinal<census),"every matched face acquisition and exact terminal");
    glyphs(fonts,context);fonts.reset();
  }
  std::cout<<"native font admission terminal "<<census<<"; every failure/retry\n";
  auto* font=fonts.getFont(context.primary,10,FALSE);auto accepted=fonts.readGlyph(font,L'W');auto pixels=accepted.coverage;
  hooks.arm(0);rejects([&]{fonts.readGlyph(font,WideChar(937));});require(hooks.hit,"glyph acquisition failure armed");hooks.disarm();
  require(accepted.coverage==pixels,"prior copied glyph survives failure");glyphs(fonts,context);
}
void allocations(Context& context) {
  auto* pool=TheMemoryPoolFactory->findMemoryPool("GameFont");
  for(unsigned kind=0;kind<3;++kind) {
    NativeFontLibrary fonts; if(kind)fonts.init();
    GameFont* font=kind==2?fonts.getFont(context.primary,10,FALSE):nullptr;
    auto operation=[&] {
      if(kind==0) { NativeFontLibrary candidate;candidate.init(); }
      else if(kind==1)fonts.getFont(context.primary,10,FALSE);
      else { auto glyph=fonts.readGlyph(font,L'W');require(!glyph.coverage.empty(),"real acquired glyph pixels"); }
    };
    auto restore=[&] { if(kind==1)fonts.reset(); };
    AllocationFault::arm(std::numeric_limits<std::size_t>::max());
    try { operation(); }catch(...) { AllocationFault::disarm();throw; }
    const auto census=AllocationFault::attempts();AllocationFault::disarm();restore();
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
      const auto live=AllocationFault::live();const auto units=pool->getUsedBlockCount();bool failed=false;
      AllocationFault::arm(ordinal);
      try { operation(); }catch(std::bad_alloc&) { failed=true; }
      catch(...) { AllocationFault::disarm();throw; }
      const bool hit=AllocationFault::triggered();AllocationFault::disarm();
      require(hit==failed && failed==(ordinal<census),"every standard acquisition and exact terminal");
      if(failed)require(AllocationFault::live()==live && pool->getUsedBlockCount()==units,"failed native font operation retires game storage");
      operation();restore();
      require(AllocationFault::live()==live && pool->getUsedBlockCount()==units,"every corrected retry retires game storage");
    }
    std::cout<<"font standard owner "<<kind<<" terminal "<<census<<"; every failure/retry\n";
  }
}
}
int main(int argc,char** argv) {
  const auto initial=AllocationFault::live();bool initialized=false;
  try { require(argc==2,"native font family");initMemoryManager();initialized=true;
    TheMemoryPoolFactory->createMemoryPool("GameFont",sizeof(GameFont),8,0);
    {Context context;
    for(unsigned warm=0;warm<2;++warm)functional(context);
    const auto baseline=AllocationFault::live();
    for(unsigned repeat=0;repeat<3;++repeat) {
      std::string_view family=argv[1];if(family=="functional")functional(context);
      else if(family=="negative")negative(context);else if(family=="native-faults")nativeFaults(context);
      else if(family=="allocation-faults")allocations(context);
      else if(family=="host")host(context);else if(family=="pool-failures")pools(context);
      else require(false,"unknown native font family");
      require(AllocationFault::live()==baseline,"whole native font backing retires game storage");
      require(TheMemoryPoolFactory->findMemoryPool("GameFont")->getUsedBlockCount()==0,"whole native font owner retires pooled records");
    }}shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole native font memory manager retirement");
    std::cout<<"PASS real CPU font backing; UI/root/parity pending\n";return 0;
  }catch(const std::exception& error) { std::cerr<<"FAIL "<<error.what()<<'\n'; }
  catch(ErrorCode code) { std::cerr<<"FAIL font status "<<Int(code)<<'\n'; }
  catch(...) { std::cerr<<"FAIL native font fixture\n"; }
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
