// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "GameClient/NativeFontLibrary.h"
#include "GameClient/NativeDisplayString.h"
#include "GameClient/NativeDisplayStringManager.h"
#include "GameClient/GameText.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameClient/InGameUI.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/NativeHeadlessDisplay.h"
#include "GameClient/NativeFrameCameraView.h"
#include "GameClient/ControlBar.h"
#include "GameClient/RayEffect.h"
#include "Common/MessageStream.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/NativeTerrainLogic.h"
#include "NativeTerrainDeviceFixture.h"
#include "GameLogic/GameLogic.h"
#include "Common/GlobalData.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/FileSystem.h"
#include "Common/GameMemory.h"
#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "GameLogic/AI.h"
#include "GameLogic/GhostObject.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/ScriptActions.h"
#include "GameLogic/ScriptConditions.h"
#include "GameClient/Water.h"
#include "GameClient/Snow.h"
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
    "ENDPROPERTIES\nCHARS 4\n";
  for(int code:{87,937,38})out<<"STARTCHAR generated"<<code<<"\nENCODING "<<code<<
    "\nSWIDTH 500 0\nDWIDTH "<<advance<<" 0\nBBX 3 5 0 0\nBITMAP\nA0\nA0\nE0\nA0\nA0\nENDCHAR\n";
  out<<"STARTCHAR space\nENCODING 32\nSWIDTH 300 0\nDWIDTH 3 0\nBBX 0 0 0 0\nBITMAP\nENDCHAR\nENDFONT\n";
  require(bool(out),"generated font written");
}
void writeCatalog(const std::filesystem::path& path) {
  std::ofstream out(path,std::ios::binary);
  auto word=[&](std::uint32_t value) { for(unsigned n=0;n<4;++n)out.put(char(value>>(n*8))); };
  const auto count=NativeDisplayStringManager::groupCount+1;
  for(auto value:{0x43534620u,3u,std::uint32_t(count),std::uint32_t(count),0u,0u})word(value);
  for(Int n=0;n<count;++n) {
    const auto label=n==count-1?std::string("LABEL:FORMATION"):"NUMBER:"+std::to_string(n);
    word(0x4c424c20u);word(1);word(label.size());out<<label;word(0x53545220u);word(1);
    const auto encoded=std::uint16_t(~std::uint16_t('W'));out.put(char(encoded));out.put(char(encoded>>8));
  }
  require(bool(out),"generated localized catalog written");
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
  AsciiString timer="GeneratedNativeTimer",power="GeneratedNativePower";
  Context() {
    writeFont(tree.path/"primary.bdf","GeneratedPrimary",5);
    writeFont(tree.path/"unicode.bdf","GeneratedUnicode",7);
    {std::ofstream invalid(tree.path/"malformed.bdf");invalid<<"not a font";require(bool(invalid),"generated malformed font written");}
    writeCatalog(tree.path/"generated.csf");
    files.mountReadOnly({tree.path.string()});
    // Backend configuration fixture; full original Language.ini init is not
    // claimed by this test. Both files go through the real read-only VFS.
    language.m_localFonts.push_back("primary.bdf");language.m_localFonts.push_back("unicode.bdf");
    language.m_unicodeFontName="GeneratedUnicode";
    language.m_defaultDisplayStringFont.name=primary;
    language.m_defaultDisplayStringFont.size=10;language.m_defaultDisplayStringFont.bold=FALSE;
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
void strings(Context& context) {
  NativeFontLibrary fonts;fonts.init();auto* font=fonts.getFont(context.primary,10,FALSE);
  auto* text=newInstance(NativeDisplayString)(fonts);
  struct Retire { NativeDisplayString* text;~Retire() { text->deleteInstance(); } } retire{text};
  text->setFont(font);UnicodeString content;content.set(L"WW\nW");text->setText(content);
  Int width=0,height=0;text->getSize(&width,&height);
  require(width==13 && height==10 && text->getWidth()==18 && text->getWidth(3)==12,"raw width counts newline position; formatted maximum includes overlap");
  content.set(L"WW");text->setText(content);text->setWordWrap(12);
  context.language.m_useHardWrap=TRUE;text->notifyTextChanged();text->getSize(&width,&height);
  require(width==7 && height==10,"source hard >= wrap boundary");
  context.language.m_useHardWrap=FALSE;text->setWordWrap(0);text->setWordWrapCentered(TRUE);
  content.set(L"WW\nW");text->setText(content);
  require(text->placements().size()==3 && text->placements().back().x==3,"center shorter line against formatted extent");
  content.set(L"&W");text->setText(content);text->setUseHotkey(TRUE,0);text->getSize(&width,&height);
  require(width==7 && height==5 && text->placements().size()==1 && text->placements().front().hotkey,"hotkey preserves primary spacing with bold overlay");
  rejects([&]{text->draw(0,0,0,0);});rejects([&]{text->setWordWrap(-1);});
  const auto accepted=text->getText();content.set(L"WWW");
  fonts.getFont(context.primary,10,TRUE); // Warm the same borrowed normal/bold cache cohort.
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());text->setText(content);
  const auto census=AllocationFault::attempts();AllocationFault::disarm();text->setText(accepted);
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    const auto live=AllocationFault::live();AllocationFault::arm(ordinal);bool failed=false;
    try { text->setText(content); }catch(std::bad_alloc&) { failed=true; }
    catch(...) { AllocationFault::disarm();throw; }
    const bool hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<census),"all staged string allocations and exact terminal");
    if(failed)require(text->getText()==accepted && AllocationFault::live()==live,"failed string mutation preserves text and backing");
    text->setText(content);text->setText(accepted);
    require(AllocationFault::live()==live,"every string failure has corrected retry and exact retirement");
  }
  text->reset();text->getSize(&width,&height);require(!width && !height && !text->getWidth() && text->placements().empty(),"reset retires CPU layout");text->draw(0,0,0,0);
  std::cout<<"native string mutation terminal "<<census<<"; every failure/retry\n";
}
void stringBoundaries(Context& context) {
  NativeFontLibrary fonts;fonts.init();auto* font=fonts.getFont(context.primary,10,FALSE);
  auto* string=newInstance(NativeDisplayString)(fonts);
  struct Retire { NativeDisplayString* text;~Retire() { text->deleteInstance(); } } retire{string};
  string->setFont(font);
  auto size=[&](const WideChar* text,Int width,Int height) {
    string->setText(UnicodeString(text));Int w=-1,h=-1;string->getSize(&w,&h);
    require(w==width && h==height,"defined source formatted boundary");
  };
  const auto space=fonts.readGlyph(font,L' ');require(space.advance==3 && space.coverage.empty(),"actual zero-bitmap supplied space");
  size(L"",0,0);size(L"\n",1,10);size(L"W\n",7,10);size(L"WW\nW",13,10);
  require(string->getWidth(0)==0 && string->getWidth(2)==12 && string->getWidth(3)==12 && string->getWidth(100)==18,"raw positions across newlines and beyond text");
  string->setWordWrap(15);size(L"WW W",13,10);
  require(string->placements().size()==4 && string->placements()[2].x==0 && string->placements()[2].y==5 && string->placements()[3].x==3,"source space retained on soft-wrapped next line");
  size(L"W W",10,10); // Equality (6+3+6) wraps, not only >width.
  string->setWordWrap(16);size(L"W W",16,5);
  string->setWordWrap(6);size(L"WW",13,5); // Soft wrapping does not split an unbroken word.
  context.language.m_useHardWrap=TRUE;string->notifyTextChanged();size(L"W",7,10);
  context.language.m_useHardWrap=FALSE;string->setWordWrap(0);
  string->setWordWrapCentered(TRUE);size(L"WWW\nW",19,10);
  require(string->placements().back().x==6,"stable centered short-line placement");
  string->setWordWrapCentered(FALSE);string->setUseHotkey(TRUE,0);
  size(L"&W",7,5);require(string->getWidth()==11,"raw width does not parse markers");
  size(L"W&",12,5);require(!string->placements().back().hotkey,"trailing marker remains ordinary glyph");
  size(L"& W",15,5);require(string->placements().size()==3,"marker before whitespace is ordinary text");
  size(L"&W&W",13,5);require(string->placements().size()==1 && string->placements().front().hotkey && string->placements().front().x==6,"last marker location and first marker glyph source rule");
  string->setUseHotkey(FALSE,0);size(L"W\u03a9",14,5);
  require(string->placements().back().glyph.alternate && string->placements().back().glyph.local,"real alternate Unicode placement");
  const auto accepted=string->getText();const auto width=string->getWidth();
  for(WideChar code:{WideChar(-1),WideChar(0xd800),WideChar(0x110000)}) {
    rejects([&]{string->appendChar(code);});require(string->getText()==accepted && string->getWidth()==width,"invalid append preserves accepted layout");
  }
  auto* foreign=newInstance(GameFont);rejects([&]{string->setFont(foreign);});foreign->deleteInstance();
  require(string->getFont()==font,"foreign font never admitted");
  rejects([&]{string->getWidth(-2);});rejects([&]{string->setClipRegion(nullptr);});
  IRegion2D invalid{{2,2},{1,1}};rejects([&]{string->setClipRegion(&invalid);});
  IRegion2D valid{{0,0},{20,20}};string->setClipRegion(&valid);
  string->removeLastChar();require(string->getText().compare(L"W")==0,"actual remove mutation");
  string->appendChar(WideChar(937));require(string->getText()==accepted,"actual append Unicode mutation");
  string->reset();string->removeLastChar();string->getSize(nullptr,nullptr);string->setFont(nullptr);
}
void stringMutations(Context& context,bool native) {
  Hooks hooks;NativeFontLibrary fonts(Hooks::step,&hooks);fonts.init();
  auto* normal=fonts.getFont(context.primary,10,FALSE);auto* bold=fonts.getFont(context.primary,10,TRUE);
  auto* string=newInstance(NativeDisplayString)(fonts);
  struct Retire { NativeDisplayString* text;~Retire() { text->deleteInstance(); } } retire{string};
  UnicodeString before(L"WW\nW"),after(L"W\u03a9W");
  auto baseline=[&] {
    string->setWordWrap(0);string->setWordWrapCentered(FALSE);string->setUseHotkey(FALSE,0);
    string->setFont(normal);string->setText(before);
  };
  for(unsigned kind=0;kind<7;++kind) {
    baseline();
    auto operation=[&] {
      if(kind==0)string->setText(after);
      else if(kind==1)string->setFont(bold);
      else if(kind==2)string->appendChar(WideChar(937));
      else if(kind==3)string->removeLastChar();
      else if(kind==4)string->setWordWrap(12);
      else if(kind==5)string->setWordWrapCentered(TRUE);
      else string->setUseHotkey(TRUE,0);
    };
    hooks.arm(SIZE_MAX);if(!native)AllocationFault::arm(SIZE_MAX);
    try { operation(); }catch(...) { hooks.disarm();AllocationFault::disarm();throw; }
    const auto census=native?hooks.count:AllocationFault::attempts();hooks.disarm();AllocationFault::disarm();baseline();
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
      const auto live=AllocationFault::live();const auto* pixels=string->placements().data();
      const auto raw=string->getWidth();Int w=0,h=0;string->getSize(&w,&h);
      hooks.arm(native?ordinal:SIZE_MAX);if(!native)AllocationFault::arm(ordinal);bool failed=false;
      try { operation(); }catch(std::bad_alloc&) { failed=true; }
      catch(...) { hooks.disarm();AllocationFault::disarm();throw; }
      const bool hit=native?hooks.hit:AllocationFault::triggered();hooks.disarm();AllocationFault::disarm();
      require(hit==failed && failed==(ordinal<census),"every mutation prefix and exact native/standard terminal");
      if(failed) {
        Int width=0,height=0;string->getSize(&width,&height);
        require(string->getText()==before && string->getFont()==normal && string->getWidth()==raw && width==w && height==h && string->placements().data()==pixels && AllocationFault::live()==live,"failed mutation preserves entire accepted cohort");
      }
      operation();baseline();require(AllocationFault::live()==live,"all corrected mutations retire staged storage");
    }
    std::cout<<"string "<<(native?"native":"standard")<<" mutation "<<kind<<" terminal "<<census<<"; every failure/retry\n";
  }
}
void manager(Context& context,bool native=false) {
  Hooks hooks;NativeFontLibrary fonts(Hooks::step,&hooks);fonts.init();Publication<FontLibrary> fontLease{TheFontLibrary,fonts};
  std::unique_ptr<GameTextInterface> catalog(CreateGameTextInterface());require(catalog->loadCSF("generated.csf"),"actual GameText CSF consumer");
  Publication<GameTextInterface> textLease{TheGameText,*catalog};
  DrawGroupInfo groups;groups.m_fontName=context.primary;groups.m_fontSize=10;groups.m_fontIsBold=FALSE;
  Publication<DrawGroupInfo> groupLease{TheDrawGroupInfo,groups};
  NativeDisplayStringManager strings(fonts);Publication<DisplayStringManager> stringLease{TheDisplayStringManager,strings};
  auto* ordinary=strings.newDisplayString();UnicodeString content;content.set(L"WW");ordinary->setText(content);
  strings.postProcessLoad();strings.postProcessLoad();
  const auto expected=std::size_t(NativeDisplayStringManager::groupCount)+2;
  require(strings.count()==expected && strings.getGroupNumeralString(0)->getText().compare(L"W")==0,"source localized cache with ordinary string preserved");
  rejects([&]{strings.getGroupNumeralString(-1);});rejects([&]{strings.getGroupNumeralString(NativeDisplayStringManager::groupCount);});
  hooks.arm(SIZE_MAX);if(!native)AllocationFault::arm(SIZE_MAX);strings.postProcessLoad();
  const auto census=native?hooks.count:AllocationFault::attempts();AllocationFault::disarm();hooks.disarm();
  for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
    auto* accepted=strings.getGroupNumeralString(0);auto* formation=strings.getFormationLetterString();
    const auto live=AllocationFault::live();const auto units=TheMemoryPoolFactory->findMemoryPool("NativeDisplayString")->getUsedBlockCount();
    hooks.arm(native?ordinal:SIZE_MAX);if(!native)AllocationFault::arm(ordinal);bool failed=false;
    try { strings.postProcessLoad(); }catch(std::bad_alloc&) { failed=true; }
    catch(...) { AllocationFault::disarm();hooks.disarm();throw; }
    const bool hit=native?hooks.hit:AllocationFault::triggered();AllocationFault::disarm();hooks.disarm();
    require(hit==failed && failed==(ordinal<census),"every localized cache allocation and exact terminal");
    if(failed)require(strings.getGroupNumeralString(0)==accepted && strings.getFormationLetterString()==formation && strings.count()==expected && AllocationFault::live()==live,"failed cache replacement preserves accepted pointers and storage");
    strings.postProcessLoad();
    require(strings.count()==expected && ordinary->getText()==content && AllocationFault::live()==live && TheMemoryPoolFactory->findMemoryPool("NativeDisplayString")->getUsedBlockCount()==units,"every cache failure has corrected retry and exact retirement");
  }
  auto* cached=strings.getGroupNumeralString(0);strings.freeDisplayString(cached);
  require(!strings.getGroupNumeralString(0),"free withdraws cache alias");strings.postProcessLoad();
  strings.reset();require(!strings.count() && !strings.getFormationLetterString(),"whole manager reset retires all records before fonts");
  strings.postProcessLoad(); // Destructor must also retire caches without reset.
  std::cout<<"string cache "<<(native?"native":"standard")<<" terminal "<<census<<"; every failure/retry\n";
}
void managerNegative(Context& context) {
  NativeDisplayStringManager manager;manager.init();rejects([&]{manager.newDisplayString();});
  NativeFontLibrary fonts;Publication<FontLibrary> fontLease{TheFontLibrary,fonts};
  rejects([&]{manager.newDisplayString();});fonts.init();
  rejects([&]{manager.postProcessLoad();});
  DrawGroupInfo group;group.m_fontName=context.primary;group.m_fontSize=10;
  Publication<DrawGroupInfo> groupLease{TheDrawGroupInfo,group};rejects([&]{manager.postProcessLoad();});
  const auto live=AllocationFault::live();const auto used=TheMemoryPoolFactory->findMemoryPool("NativeDisplayString")->getUsedBlockCount();
  context.language.m_defaultDisplayStringFont.size=101;
  rejects([&]{manager.newDisplayString();});context.language.m_defaultDisplayStringFont.size=10;
  require(!manager.count() && AllocationFault::live()==live && TheMemoryPoolFactory->findMemoryPool("NativeDisplayString")->getUsedBlockCount()==used,"invalid default descriptor retires unadmitted string/font candidates");
  auto* accepted=manager.newDisplayString();manager.freeDisplayString(nullptr);
  auto* foreign=newInstance(NativeDisplayString)(fonts);rejects([&]{manager.freeDisplayString(foreign);});foreign->deleteInstance();
  require(manager.count()==1 && accepted->getFont(),"foreign free preserves accepted owner list");
  manager.freeDisplayString(accepted);manager.reset();manager.reset();
}
struct UI:InGameUI {
  void draw() override { throw ERROR_BAD_ARG; }
  View* createView() override { throw ERROR_BAD_ARG; }
  using InGameUI::admitSuperweapon;
  void timerFont(const AsciiString& name) { m_namedTimerNormalFont=name;m_namedTimerNormalPointSize=10;m_namedTimerNormalBold=FALSE; }
};
struct Client:NativeHeadlessGameClient {
  Client():NativeHeadlessGameClient(Composition::DrawablesOnly){}
  void compose(Context& context) {
    // Actual production factories in original source order. The manager's init
    // is valid before any font owner exists, but string admission is not.
    m_serviceOwners.create(TheDisplayStringManager,[&]{return createDisplayStringManager();});
    TheDisplayStringManager->init();rejects([&]{TheDisplayStringManager->newDisplayString();});
    m_serviceOwners.create(TheFontLibrary,[&]{return createFontLibrary();},false);TheFontLibrary->init();
    TheDrawGroupInfo->m_fontName=context.primary;TheDrawGroupInfo->m_fontSize=10;TheDrawGroupInfo->m_fontIsBold=FALSE;
    TheDisplayStringManager->postProcessLoad();
    auto* ui=static_cast<UI*>(m_serviceOwners.create(TheInGameUI,[]{return new UI;}));ui->timerFont(context.primary);
    ui->addNamedTimer(context.timer,UnicodeString::TheEmptyString,FALSE);
    ui->addNamedTimer(context.timer,UnicodeString::TheEmptyString,TRUE);
    ui->admitSuperweapon(0,context.power,newInstance(SuperweaponInfo)(ObjectID(1),0,FALSE,FALSE,FALSE,FALSE,context.primary,10,FALSE,0,nullptr));
    auto* text=TheDisplayStringManager->newDisplayString();UnicodeString content;content.set(L"WW");text->setText(content);
    Int width=0,height=0;text->getSize(&width,&height);require(width==13 && height==5,"actual client cohort uses real glyph metrics");
    require(static_cast<NativeDisplayStringManager*>(TheDisplayStringManager)->count()==15,"cache plus timer, two power strings and ordinary text");
  }
};
void clientCase(Context& context) {
  require(!TheFontLibrary && !TheDisplayStringManager && !TheInGameUI && !TheDrawGroupInfo,"client requires unowned original slots");
  struct Restore { GameClient* prior;~Restore() { TheGameClient=prior; } } restore{TheGameClient};
  {Client client;TheGameClient=&client;client.compose(context);}
  require(!TheFontLibrary && !TheDisplayStringManager && !TheInGameUI && !TheDrawGroupInfo,"actual client destructor retires UI then strings then native font backing");
}
void client(Context& context,bool faults,bool late) {
  GlobalData data;Publication<GlobalData> dataLease{TheWritableGlobalData,data};data.m_xResolution=800;data.m_maxLineBuildObjects=0;
  std::unique_ptr<GameTextInterface> catalog(CreateGameTextInterface());require(catalog->loadCSF("generated.csf"),"original catalog for client composition");
  Publication<GameTextInterface> textLease{TheGameText,*catalog};
  clientCase(context);clientCase(context);
  if(!faults)return;
  AllocationFault::arm(std::numeric_limits<std::size_t>::max());
  try { clientCase(context); }catch(...) { AllocationFault::disarm();throw; }
  const auto census=AllocationFault::attempts();AllocationFault::disarm();
  const auto midpoint=census/2;
  for(std::size_t ordinal=late?midpoint:0;ordinal<=(late?census:midpoint-1);++ordinal) {
    const auto live=AllocationFault::live();
    const auto fonts=TheMemoryPoolFactory->findMemoryPool("GameFont")->getUsedBlockCount();
    const auto strings=TheMemoryPoolFactory->findMemoryPool("NativeDisplayString")->getUsedBlockCount();
    AllocationFault::arm(ordinal);bool failed=false;
    try { clientCase(context); }catch(std::bad_alloc&) { failed=true; }
    catch(...) { AllocationFault::disarm();throw; }
    const auto hit=AllocationFault::triggered();AllocationFault::disarm();
    require(hit==failed && failed==(ordinal<census),"source client native text composition fault prefixes and exact terminal");
    require(!TheFontLibrary && !TheDisplayStringManager && !TheInGameUI && !TheDrawGroupInfo && AllocationFault::live()==live,"failed client composition withdraws publications and retires native/game owners");
    clientCase(context);
    const auto retired=AllocationFault::live();
    if(retired!=live)std::cerr<<"client standard retirement ordinal "<<ordinal<<" expected "<<live<<" actual "<<retired<<'\n';
    require(retired==live && TheMemoryPoolFactory->findMemoryPool("GameFont")->getUsedBlockCount()==fonts && TheMemoryPoolFactory->findMemoryPool("NativeDisplayString")->getUsedBlockCount()==strings && TheMemoryPoolFactory->findMemoryPool("NamedTimerInfo")->getUsedBlockCount()==0 && TheMemoryPoolFactory->findMemoryPool("SuperweaponInfo")->getUsedBlockCount()==0,"every client failure has corrected retry and typed unit retirement");
  }
  std::cout<<"native client text terminal "<<census<<"; "<<(late?"late":"early")<<" prefixes, every failure/retry\n";
}

struct CommandUsage {
  Int buttons=TheMemoryPoolFactory->findMemoryPool("CommandButton")->getUsedBlockCount();
  Int sets=TheMemoryPoolFactory->findMemoryPool("CommandSet")->getUsedBlockCount();
  bool matches()const{return buttons==TheMemoryPoolFactory->findMemoryPool("CommandButton")->getUsedBlockCount() &&
    sets==TheMemoryPoolFactory->findMemoryPool("CommandSet")->getUsedBlockCount();}
};
struct ResetSourceLogic:GameLogic {
  DeviceFixture& visual;
  explicit ResetSourceLogic(DeviceFixture& value):visual(value){}
  TerrainLogic* createTerrainLogic()override {
    struct Terrain:NativeTerrainLogic {
      using NativeTerrainLogic::NativeTerrainLogic;
      void admitDeviceMap()override{throw ERROR_BAD_ARG;}
      void loadRoadsAndBridges(Bool)override{throw ERROR_BAD_ARG;}
    };
    return new Terrain(visual);
  }
};
struct ResetSourceRoot:GameEngine {
  ResetSourceRoot(){m_serviceOwners.create(TheSubsystemList,[]{return new SubsystemInterfaceList;});}
  template<class Base,class Concrete>void admit(std::unique_ptr<Concrete>& owner,Base*& slot){
    Base* acquired=owner.get();Base* previous=slot;
    const SubsystemPublication publication{&slot,previous,
      [](void* target,void* prior)noexcept{*static_cast<Base**>(target)=static_cast<Base*>(prior);},acquired};
    TheSubsystemList->adoptInitializedSubsystem(acquired,publication);
    slot=acquired;owner.release();
  }
  // Prepared owners exercise source reset/destruction, not full init factories.
  GameLogic* createGameLogic()override{throw ERROR_BAD_ARG;}
  GameClient* createGameClient()override{throw ERROR_BAD_ARG;}
  ModuleFactory* createModuleFactory()override{throw ERROR_BAD_ARG;}
  ThingFactory* createThingFactory()override{throw ERROR_BAD_ARG;}
  FunctionLexicon* createFunctionLexicon()override{throw ERROR_BAD_ARG;}
  Radar* createRadar()override{throw ERROR_BAD_ARG;}
  WebBrowser* createWebBrowser()override{throw ERROR_BAD_ARG;}
  ParticleSystemManager* createParticleSystemManager()override{throw ERROR_BAD_ARG;}
  AudioManager* createAudioManager()override{throw ERROR_BAD_ARG;}
};
template<class T>struct ResetSetting {
  OVERRIDE<T>& slot;const T* previous;T* owner;
  template<class Factory>ResetSetting(OVERRIDE<T>& target,Factory create):slot(target),previous(target.getNonOverloadedPointer()),owner(create()){slot=owner;}
  ~ResetSetting(){slot=owner;owner->deleteInstance();slot=const_cast<T*>(previous);}
};
void composition(Context& context,std::string_view family) {
  auto put=[&](const char* path,const char* contents){auto target=context.tree.path/path;
    std::filesystem::create_directories(target.parent_path());std::ofstream out(target);out<<contents;
    out.close();require(bool(out),"generated native client input");};
  put("Data/INI/InGameUI.ini","InGameUI\n MessageDelayMS = 4000\nEnd\n");
  put("Data/INI/DrawGroupInfo.ini","DrawGroupInfo\n FontName = GeneratedPrimary\n FontSize = 10\nEnd\n");
  put("Data/INI/Default/CommandButton.ini","CommandButton GeneratedStop\n Command = STOP\nEnd\n");
  put("Data/INI/CommandButton.ini","CommandButton GeneratedMove\n Command = ATTACK_MOVE\nEnd\n");
  put("Data/INI/CommandSet.ini","CommandSet GeneratedSet\n 1 = GeneratedStop\n 2 = GeneratedMove\nEnd\n");
  put("override.ini","CommandButton Temporary\n Command = STOP\nEnd\nCommandSet TemporarySet\n 1 = Temporary\nEnd\n");
  context.files.mountReadOnly({context.tree.path.string()});
  GlobalData data;Publication<GlobalData> dataLease{TheWritableGlobalData,data};
  data.m_xResolution=800;data.m_yResolution=600;data.m_maxLineBuildObjects=0;
  data.m_cameraHeight=300;data.m_cameraPitch=45;data.m_cameraYaw=0;data.m_maxCameraHeight=600;
  MessageStream messages;Publication<MessageStream> messagesLease{TheMessageStream,messages};
  std::unique_ptr<GameTextInterface> catalog(CreateGameTextInterface());
  require(catalog->loadCSF("generated.csf"),"original catalog for default client composition");
  Publication<GameTextInterface> textLease{TheGameText,*catalog};
  const auto views=View::liveViewCount();
  // Actual native no-map extent is defined. The common terrain base's empty
  // getExtent stub is not a valid reset fixture. No device/map success claimed.
  DeviceFixture visual;
  struct ResetTerrain:NativeTerrainLogic {
    using NativeTerrainLogic::NativeTerrainLogic;
    void admitDeviceMap()override{throw ERROR_BAD_ARG;}
    void loadRoadsAndBridges(Bool)override{throw ERROR_BAD_ARG;}
  } resetParent(visual);
  struct Definition:ThingTemplate{~Definition()override=default;} definition;
  auto clean=[&]{return !TheGameClient && !TheInGameUI && !TheDisplay && !TheTacticalView &&
    !TheControlBar && !TheRayEffects && !TheFontLibrary && !TheDisplayStringManager && !TheDrawGroupInfo &&
    View::liveViewCount()==views;};
  auto operation=[&]{
    struct Restore{GameClient* previous=TheGameClient;~Restore(){TheGameClient=previous;}} restore;
    NativeHeadlessGameClient owner;TheGameClient=&owner;owner.init();
    auto* camera=dynamic_cast<NativeFrameCameraView*>(TheTacticalView);
    auto* display=dynamic_cast<NativeHeadlessDisplay*>(TheDisplay);
    require(camera && display && display->getFirstView()==camera && camera->getWidth()==800 &&
      camera->getHeight()==462 && camera->transformBuildCount()==2 && !TheTerrainLogic &&
      static_cast<NativeFontLibrary*>(TheFontLibrary)->localFaceCount()==2,
      "default native client adopts real source UI/View/font services before terrain initialization");
    require(static_cast<NativeDisplayStringManager*>(TheDisplayStringManager)->count()==NativeDisplayStringManager::groupCount+1 &&
      TheDisplayStringManager->getGroupNumeralString(0),"actual client postprocess admits nonempty source numeral/formation strings");
    auto* set=TheControlBar->findCommandSet("GeneratedSet");
    require(set && set->getCommandButton(0)==TheControlBar->findCommandButton("GeneratedStop") &&
      set->getCommandButton(1)==TheControlBar->findCommandButton("GeneratedMove"),"default client owns parsed command metadata");
    bool rejected=false;try{owner.init();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
    require(rejected && TheTacticalView==camera && TheControlBar->findCommandSet("GeneratedSet")==set,
      "duplicate composed initialization preserves accepted source owners");
    rejects([&]{display->draw();});rejects([&]{display->setDisplayMode(1024,768,32,FALSE);});
    // Missing simulation parents reject before publishing any FRAME_TICK.
    rejects([&]{owner.update();});
    INI overrides;overrides.load("override.ini",INI_LOAD_CREATE_OVERRIDES,nullptr);
    require(TheControlBar->findCommandSet("TemporarySet"),"composed client admits actual map command overrides");
    auto* draw=owner.friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);const auto id=draw->getID();
    TheInGameUI->deselectDrawable(draw);
    require(!TheGameLogic && !draw->isSelected(),"unselected client-only Drawable cleanup requires no unused logic parent");
    rejects([&]{owner.reset();});
    require(TheControlBar->findCommandSet("TemporarySet"),"missing reset parent rejects before metadata retirement");
    {Publication<TerrainLogic> terrainLease{TheTerrainLogic,resetParent};owner.reset();owner.reset();}
    require(TheTacticalView==camera && display->getFirstView()==camera && !TheControlBar->findCommandSet("TemporarySet") &&
      !TheControlBar->findCommandButton("Temporary") && TheControlBar->findCommandSet("GeneratedSet")==set,
      "repeated client reset preserves owned View/base metadata and retires temporary definitions");
    require(!owner.findDrawableByID(id),"actual reset withdraws its client-only Drawable");
    owner.friend_createDrawable(&definition,DRAWABLE_STATUS_NONE); // Base cleanup with live UI, no logic parent.
  };
  operation();operation();require(clean(),"whole default client retires complete composed publication cohort");
  if(family=="composition-root-reset"){
    NameKeyGenerator names;Publication<NameKeyGenerator> namesLease{TheNameKeyGenerator,names};names.init();
    AI ai;Publication<AI> aiLease{TheAI,ai};ai.init();
    ResetSetting<WaterTransparencySetting> water{TheWaterTransparency,[]{return newInstance(WaterTransparencySetting);}};
    ResetSetting<WeatherSetting> weather{TheWeatherSetting,[]{return newInstance(WeatherSetting);}};
    data.m_partitionCellSize=10;data.m_scriptDebug=FALSE;data.m_particleEdit=FALSE;
    const auto* borrowedLogic=TheGameLogic;const auto* borrowedClient=TheGameClient;
    {
      ResetSourceRoot root;
      auto client=std::make_unique<NativeHeadlessGameClient>();
      {Publication<GameClient> clientLease{TheGameClient,*client};client->init();}
      root.admit(client,TheGameClient);
      auto logic=std::make_unique<ResetSourceLogic>(visual);
      {Publication<GameLogic> logicLease{TheGameLogic,*logic};logic->init();}
      root.admit(logic,TheGameLogic);
      auto* camera=TheTacticalView;auto* metadata=TheControlBar->findCommandSet("GeneratedSet");
      require(ThePartitionManager && TheTerrainLogic && TheGhostObjectManager && TheScriptActions && TheScriptConditions && TheScriptEngine,"actual GameLogic init admits complete source parent cohort");
      require(TheGameLogic->getFrame()==0 && TheGameLogic->getObjectIDCounter()==1,"actual source initialized frame and actor cursor");
      for(unsigned repeat=0;repeat<3;++repeat)for(std::size_t ordinal=0;ordinal<=1;++ordinal){
        INI overrides;overrides.load("override.ini",INI_LOAD_CREATE_OVERRIDES,nullptr);
        auto* drawable=TheGameClient->friend_createDrawable(&definition,DRAWABLE_STATUS_NONE);const auto id=drawable->getID();
        require(TheControlBar->findCommandSet("TemporarySet") && TheGameClient->findDrawableByID(id),"root reset starts with actual source overrides and client-only effect");
        bool failed=false;AllocationFault::arm(ordinal);
        try{root.reset();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
        const auto resetAttempts=AllocationFault::attempts();const auto resetHit=AllocationFault::triggered();AllocationFault::disarm();
        require(resetHit==failed && failed==(ordinal==0) && resetAttempts==1,"initialized source root reset replacement-vector failure and exact terminal one");
        if(failed)require(TheControlBar->findCommandSet("TemporarySet") && TheTacticalView==camera && !TheGameClient->findDrawableByID(id),"failed source reset preserves parents and metadata after legitimate root quiescence");
        root.reset(); // Every failure/terminal has a corrected same-owner retry.
        require(TheGameLogic->getFrame()==0 && TheGameLogic->getObjectIDCounter()==1 && !TheGameLogic->getFirstObject() && !TheGameClient->findDrawableByID(id),"actual root reset retires effect and resets original simulation state");
        require(TheTacticalView==camera && TheControlBar->findCommandSet("GeneratedSet")==metadata && !TheControlBar->findCommandSet("TemporarySet") && TheWaterTransparency.getNonOverloadedPointer()==water.owner && TheWeatherSetting.getNonOverloadedPointer()==weather.owner,"source root reset preserves complete logical client and base setting identities");
      }
    }
    require(clean() && TheGameLogic==borrowedLogic && TheGameClient==borrowedClient && !ThePartitionManager && !TheTerrainLogic && !TheGhostObjectManager && !TheScriptActions && !TheScriptConditions && !TheScriptEngine,"actual root destruction retires complete initialized logic/client cohort");
    return;
  }
  if(family=="composition-pools"){
    for(const char* name:{"CommandButton","CommandSet","GameFont","NativeDisplayString"}){
      auto* pool=TheMemoryPoolFactory->findMemoryPool(name);void* held[32]{};unsigned count=0;
      struct Free{MemoryPool* pool;void** held;unsigned& count;~Free(){while(count)pool->freeBlock(held[--count]);}}free{pool,held,count};
      require(pool->getFreeBlockCount()<=32,"fixed generated composition pool capacity");
      while(pool->getFreeBlockCount())held[count++]=pool->allocateBlock("generated client composition exhaustion");
      {struct Restore{GameClient* previous=TheGameClient;~Restore(){TheGameClient=previous;}} restore;
        NativeHeadlessGameClient owner;TheGameClient=&owner;CommandUsage used;bool failed=false;
        try{owner.init();}catch(ErrorCode code){failed=code==ERROR_OUT_OF_MEMORY;}
        require(failed && used.matches() && !TheRayEffects && !TheControlBar && !TheDisplay && !TheInGameUI &&
          !TheFontLibrary && !TheDisplayStringManager && View::liveViewCount()==views,
          "typed exhaustion retires whole partial client composition including admitted UI/View and font strings");
        while(count)pool->freeBlock(held[--count]);owner.init();
        require(TheInGameUI && TheControlBar && TheTacticalView,"same client retries after typed capacity is restored");}
      require(clean(),"typed failure/retry whole composed owner retires exact publications");
    }
    return;
  }
  if(family=="composition-negative"){
    struct Restore{GameClient* previous=TheGameClient;~Restore(){TheGameClient=previous;}} restore;
    NativeHeadlessGameClient owner;TheGameClient=&owner;
    NativeHeadlessDisplay foreign;TheDisplay=&foreign;bool rejected=false;
    try{owner.init();}catch(ErrorCode code){rejected=code==ERROR_BAD_ARG;}
    require(rejected && TheDisplay==&foreign && !TheRayEffects && !TheControlBar && !TheInGameUI,
      "foreign publication rejects before acquiring logical services");TheDisplay=nullptr;
    put("Data/INI/CommandSet.ini","CommandSet Invalid\n 1 = Unknown\nEnd\n");
    context.files.mountReadOnly({context.tree.path.string()});rejected=false;
    try{owner.init();}catch(const INIException&){rejected=true;}catch(ErrorCode){rejected=true;}
    require(rejected && !TheRayEffects && !TheDisplay && !TheControlBar && !TheInGameUI && !TheFontLibrary &&
      !TheDisplayStringManager && View::liveViewCount()==views,"bad command reference retires whole partial native composition");
    put("Data/INI/CommandSet.ini","CommandSet GeneratedSet\n 1 = GeneratedStop\nEnd\n");
    context.files.mountReadOnly({context.tree.path.string()});owner.init();
    {Publication<TerrainLogic> terrainLease{TheTerrainLogic,resetParent};owner.reset();}
    require(TheControlBar->findCommandSet("GeneratedSet") && TheTacticalView,"same client retries corrected source input");
    return;
  }
  if(family=="composition")return;
  AllocationFault::arm(SIZE_MAX);try{operation();}catch(...){AllocationFault::disarm();throw;}
  const auto terminal=AllocationFault::attempts();AllocationFault::disarm();require(terminal>0 && terminal<1024,"bounded complete default client manifest");
  const auto midpoint=terminal/2;const bool late=family=="composition-faults-late";
  for(std::size_t ordinal=late?midpoint:0;ordinal<=(late?terminal:midpoint-1);++ordinal){
    const auto live=AllocationFault::live();CommandUsage used;AllocationFault::arm(ordinal);bool failed=false;
    try{operation();}catch(const std::bad_alloc&){failed=true;}catch(...){AllocationFault::disarm();throw;}
    const auto attempts=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
    require(failed==(ordinal<terminal) && hit==failed && (failed || attempts==terminal),"complete composed client failure prefixes and exact terminal");
    require(clean() && used.matches() && AllocationFault::live()==live,"failed whole composed client retires all acquired units");
    operation();require(clean() && used.matches() && AllocationFault::live()==live,"every composed client failure retains corrected retry");
  }
  std::cout<<"composed client terminal "<<terminal<<"; "<<(late?"late":"early")<<" prefixes; every retry\n";
}
}
int main(int argc,char** argv) {
  const auto initial=AllocationFault::live();bool initialized=false;
  try { require(argc==2,"native font family");initMemoryManager();initialized=true;
    TheMemoryPoolFactory->createMemoryPool("GameFont",sizeof(GameFont),8,0);
    TheMemoryPoolFactory->createMemoryPool("NativeDisplayString",sizeof(NativeDisplayString),32,0);
    TheMemoryPoolFactory->createMemoryPool("NamedTimerInfo",sizeof(NamedTimerInfo),8,0);
    TheMemoryPoolFactory->createMemoryPool("SuperweaponInfo",sizeof(SuperweaponInfo),8,0);
    TheMemoryPoolFactory->createMemoryPool("CommandButton",sizeof(CommandButton),16,0);
    TheMemoryPoolFactory->createMemoryPool("CommandSet",sizeof(CommandSet),16,0);
    {Context context;
    for(unsigned warm=0;warm<2;++warm)functional(context);
    if(std::string_view(argv[1]).starts_with("client"))for(unsigned warm=0;warm<2;++warm)client(context,false,false);
    if(std::string_view(argv[1]).starts_with("composition"))for(unsigned warm=0;warm<2;++warm)composition(context,std::string_view(argv[1])=="composition-root-reset"?"composition-root-reset":"composition");
    const auto baseline=AllocationFault::live();
    for(unsigned repeat=0;repeat<3;++repeat) {
      std::string_view family=argv[1];if(family=="functional")functional(context);
      else if(family=="negative")negative(context);else if(family=="native-faults")nativeFaults(context);
      else if(family=="allocation-faults")allocations(context);
      else if(family=="host")host(context);else if(family=="pool-failures")pools(context);
      else if(family=="strings")strings(context);
      else if(family=="string-boundaries")stringBoundaries(context);
      else if(family=="string-mutation-faults")stringMutations(context,false);
      else if(family=="string-native-faults")stringMutations(context,true);
      else if(family=="manager")manager(context);
      else if(family=="manager-native-faults")manager(context,true);
      else if(family=="manager-negative")managerNegative(context);
      else if(family=="client")client(context,false,false);
      else if(family=="client-faults-early")client(context,true,false);
      else if(family=="client-faults-late")client(context,true,true);
      else if(family.starts_with("composition"))composition(context,family);
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
