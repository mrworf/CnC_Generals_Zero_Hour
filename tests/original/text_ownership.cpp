// SPDX-License-Identifier: GPL-3.0-or-later
// Actual shared source owners. Adapters below are test-only and reject output
// and metrics; these tests do not establish native text or root acceptance.
#include "PreRTS.h"
#include "AllocationFault.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "GameClient/ControlBar.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/DrawGroupInfo.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/InGameUI.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include <array>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> void rejects(F action) {
  bool rejected=false;try { action(); } catch (ErrorCode e) { rejected=e==ERROR_BAD_ARG; }
  require(rejected,"source operation rejects invalid/failed backing");
}
struct Hooks {
  std::size_t attempts=0, failAt=SIZE_MAX;
  bool hit=false, invalidBorrow=false;
  void step() { if (attempts++==failAt) { hit=true;throw ERROR_BAD_ARG; } }
  void arm(std::size_t n) { attempts=0;failAt=n;hit=false; }
  void disarm() { failAt=SIZE_MAX; }
};
struct Backing { unsigned value=0x1357; };
struct Fonts:FontLibrary {
  enum Mode { normal, returnsFalse, throws } mode=normal;
  unsigned acquired=0, released=0;
  ~Fonts() override { deleteAllFonts(); }
  Bool loadFontData(GameFont* font) override {
    require(!font->fontData && !font->next && font->height==0,"initialized unpublished pooled font");
    font->fontData=new Backing;++acquired;
    if (mode==returnsFalse) return FALSE;
    if (mode==throws) throw ERROR_BAD_ARG;
    font->height=17;return TRUE;
  }
  void releaseFontData(GameFont* font) override {
    if (font->fontData) { delete static_cast<Backing*>(font->fontData);++released; }
    font->fontData=nullptr;
  }
};
class Text:public DisplayString {
  MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(Text,"OwnershipText",8,0)
public:
  Hooks& hooks;
  explicit Text(Hooks& value):hooks(value) {}
  void reset() override { hooks.step();DisplayString::reset(); }
  void setText(UnicodeString text) override { hooks.step();DisplayString::setText(text); }
  void setFont(GameFont* font) override { hooks.step();DisplayString::setFont(font); }
  void setWordWrap(Int) override { throw ERROR_BAD_ARG; }
  void setWordWrapCentered(Bool) override { throw ERROR_BAD_ARG; }
  void draw(Int,Int,Color,Color) override { throw ERROR_BAD_ARG; }
  void draw(Int,Int,Color,Color,Int,Int) override { throw ERROR_BAD_ARG; }
  void getSize(Int*,Int*) override { throw ERROR_BAD_ARG; }
  Int getWidth(Int=-1) override { throw ERROR_BAD_ARG; }
  void setUseHotkey(Bool,Color) override { throw ERROR_BAD_ARG; }
};
Text::~Text() {
  if (getFont() && (!getFont()->fontData || static_cast<Backing*>(getFont()->fontData)->value!=0x1357))
    hooks.invalidBorrow=true;
}
struct Strings:DisplayStringManager {
  Hooks& hooks;
  bool nullResult=false;
  explicit Strings(Hooks& value):hooks(value) {}
  ~Strings() override { while (m_stringList) freeDisplayString(m_stringList); }
  DisplayString* newDisplayString() override {
    hooks.step();if (nullResult) return nullptr;
    auto* text=newInstance(Text)(hooks);link(text);return text;
  }
  void freeDisplayString(DisplayString* text) override { unLink(text);text->deleteInstance(); }
  DisplayString* getGroupNumeralString(Int) override { throw ERROR_BAD_ARG; }
  DisplayString* getFormationLetterString() override { throw ERROR_BAD_ARG; }
  unsigned count() const { unsigned n=0;for (auto* text=m_stringList;text;text=text->next()) ++n;return n; }
};
struct UI:InGameUI {
  void draw() override { throw ERROR_BAD_ARG; }
  View* createView() override { throw ERROR_BAD_ARG; }
  using InGameUI::clearTimerResources;
  using InGameUI::admitSuperweapon;
  void timerFont(const AsciiString& face) { m_namedTimerNormalFont=face; }
  NamedTimerInfo* timer(const AsciiString& name) const {
    auto it=m_namedTimers.find(name);return it==m_namedTimers.end()?nullptr:it->second;
  }
  unsigned timers() const { return m_namedTimers.size(); }
  unsigned entries() const { unsigned n=0;for (const auto& map:m_superweapons) n+=map.size();return n; }
  unsigned powers() const {
    unsigned n=0;for (const auto& map:m_superweapons) for (const auto& entry:map) n+=entry.second.size();return n;
  }
};
template<class T> struct Publication {
  T*& slot;T* previous;
  Publication(T*& target,T& value):slot(target),previous(target) { slot=&value; }
  ~Publication() { slot=previous; }
};
struct Parents {
  FileSystem files;
  Publication<FileSystem> filesLease{TheFileSystem,files};
  GlobalData data;
  Publication<GlobalData> dataLease{TheWritableGlobalData,data};
  GlobalLanguage language;
  Publication<GlobalLanguage> languageLease{TheGlobalLanguageData,language};
  Parents() { data.m_xResolution=800;data.m_maxLineBuildObjects=0; }
};
struct Context {
  Hooks hooks;Fonts fonts;Strings strings{hooks};
  Publication<FontLibrary> fontsLease{TheFontLibrary,fonts};
  Publication<DisplayStringManager> stringsLease{TheDisplayStringManager,strings};
  AsciiString name="GeneratedTimer", power="GeneratedPower", face="Arial";
  UnicodeString oldText, newText;
  Context() { oldText.set(L"Original");newText.set(L"Replacement");fonts.getFont(face,10,FALSE); }
  SuperweaponInfo* make(unsigned id=1) {
    return newInstance(SuperweaponInfo)(ObjectID(id),0,FALSE,FALSE,FALSE,FALSE,face,10,FALSE,0,nullptr);
  }
};
constexpr std::array<const char*,3> poolNames{"GameFont","NamedTimerInfo","SuperweaponInfo"};
struct Usage {
  std::array<Int,4> units{};
  Usage() {
    for (unsigned i=0;i<3;++i) units[i]=TheMemoryPoolFactory->findMemoryPool(poolNames[i])->getUsedBlockCount();
    units[3]=TheMemoryPoolFactory->findMemoryPool("OwnershipText")->getUsedBlockCount();
  }
  bool matches() const {
    for (unsigned i=0;i<3;++i) if (units[i]!=TheMemoryPoolFactory->findMemoryPool(poolNames[i])->getUsedBlockCount()) return false;
    return units[3]==TheMemoryPoolFactory->findMemoryPool("OwnershipText")->getUsedBlockCount();
  }
};
void functional(Context& c) {
  { UI ui;
    ui.addNamedTimer(c.name,c.oldText,FALSE);auto* old=ui.timer(c.name);
    require(old && old->timerText==c.oldText && !old->isCountdown && old->timestamp==~UnsignedInt{0},"original timer fields");
    ui.addNamedTimer(c.name,c.newText,TRUE);
    require(ui.timers()==1 && ui.timer(c.name)!=old && ui.timer(c.name)->timerText==c.newText && ui.timer(c.name)->isCountdown,"complete replacement retains one timer");
    ui.admitSuperweapon(0,c.power,c.make());ui.admitSuperweapon(0,c.power,c.make(2));
    require(ui.entries()==1 && ui.powers()==2 && c.strings.count()==5,"actual map/list/string ownership");
    require(ui.removeSuperweapon(0,c.power,ObjectID(1),nullptr),"source removal retires one list member");
    ui.removeNamedTimer(c.name);ui.removeNamedTimer(c.name);
    ui.clearTimerResources();ui.clearTimerResources();
    require(!ui.timers() && !ui.entries() && !c.strings.count(),"shared reset/shutdown helper is idempotent");
    ui.addNamedTimer(c.name,c.oldText,FALSE);ui.admitSuperweapon(1,c.power,c.make());
  }
  require(!c.strings.count() && !c.hooks.invalidBorrow,"shutdown without reset retires all borrowed fonts first");
  require(c.fonts.getFont(c.face,10,FALSE)==c.fonts.firstFont() && c.fonts.getCount()==1,"source font cache identity");
}
void negative(Context& c) {
  UI ui;ui.addNamedTimer(c.name,c.oldText,FALSE);auto* accepted=ui.timer(c.name);
  c.strings.nullResult=true;
  rejects([&] { ui.addNamedTimer(c.name,c.newText,TRUE); });
  rejects([&] { auto* value=c.make();value->deleteInstance(); });
  c.strings.nullResult=false;
  require(ui.timer(c.name)==accepted && c.strings.count()==1,"null string backing preserves accepted timer");
  rejects([&] { ui.admitSuperweapon(MAX_PLAYER_COUNT,c.power,c.make()); });
  rejects([&] { ui.removeSuperweapon(-1,c.power,ObjectID(1),nullptr); });
  require(!ui.entries() && c.strings.count()==1,"invalid admission retires its entire candidate");
  // Normalize the borrowed configuration identity before measuring. Replacing
  // the UI constructor's independently owned literal legitimately retires it.
  ui.timerFont(c.face);
  for (auto mode:{Fonts::returnsFalse,Fonts::throws}) {
    AsciiString missing="GeneratedNewFont";c.fonts.mode=mode;const auto live=AllocationFault::live();Usage units;
    if (mode==Fonts::returnsFalse) require(!c.fonts.getFont(missing,11,TRUE),"normal backend rejection returns null");
    else rejects([&] { c.fonts.getFont(missing,11,TRUE); });
    rejects([&] { auto* value=newInstance(SuperweaponInfo)(ObjectID(2),0,FALSE,FALSE,FALSE,FALSE,missing,11,TRUE,0,nullptr);value->deleteInstance(); });
    ui.timerFont(missing);
    rejects([&] { ui.addNamedTimer(c.name,c.newText,TRUE); });
    ui.timerFont(c.face);
    require(c.fonts.getCount()==1 && c.fonts.acquired-c.fonts.released==1 && units.matches() && AllocationFault::live()==live,"partial failed font backing and pooled candidate retired");
    require(ui.timer(c.name)==accepted && c.strings.count()==1,"backend false/throw in both UI candidates preserves accepted state");
  }
  c.fonts.mode=Fonts::normal;
  require(c.fonts.getFont("GeneratedNewFont",11,TRUE),"corrected font retry");
  ui.addNamedTimer(c.name,c.newText,TRUE);ui.clearTimerResources();
  c.fonts.reset();c.fonts.getFont(c.face,10,FALSE);
}
void callbacks(Context& c, bool weapon) {
  UI ui;ui.addNamedTimer(c.name,c.oldText,FALSE);auto* accepted=ui.timer(c.name);
  auto action=[&] { if (weapon) ui.admitSuperweapon(0,c.power,c.make());else ui.addNamedTimer("GeneratedSecond",c.newText,TRUE); };
  c.hooks.arm(SIZE_MAX);action();const auto terminal=c.hooks.attempts;c.hooks.disarm();
  if (weapon) ui.removeSuperweapon(0,c.power,ObjectID(1),nullptr);else ui.removeNamedTimer("GeneratedSecond");
  require(terminal==(weapon?8u:4u),"exact complete string callback terminal");
  for (std::size_t n=0;n<=terminal;++n) {
    const auto live=AllocationFault::live();Usage units;
    c.hooks.arm(n);bool failed=false;try { action(); }catch(ErrorCode e) { if(e!=ERROR_BAD_ARG)throw;failed=true; }
    const bool hit=c.hooks.hit;c.hooks.disarm();
    require(failed==hit && failed==(n<terminal),"every constructor/operation callback failure and terminal");
    if (!failed) { if (weapon) ui.removeSuperweapon(0,c.power,ObjectID(1),nullptr);else ui.removeNamedTimer("GeneratedSecond"); }
    require(ui.timer(c.name)==accepted && units.matches() && AllocationFault::live()==live && c.strings.count()==1,"all failed prefixes preserve accepted owner and retire candidates");
    action();if (weapon) ui.removeSuperweapon(0,c.power,ObjectID(1),nullptr);else ui.removeNamedTimer("GeneratedSecond");
    require(units.matches() && AllocationFault::live()==live,"every corrected callback retry retires exactly");
  }
  std::cout<<(weapon?"two-string":"timer-string")<<" callbacks prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
std::size_t allocationCase(Context& c,unsigned kind,std::size_t ordinal,bool discovering=false) {
  UI ui;
  if (kind==1) ui.addNamedTimer(c.name,c.oldText,FALSE);
  if (kind==3) ui.admitSuperweapon(0,c.power,c.make(1));
  if (kind==4) c.fonts.reset();
  auto* accepted=ui.timer(c.name);
  Usage units;const auto live=AllocationFault::live();
  auto action=[&] {
    if (kind<2) ui.addNamedTimer(c.name,c.newText,TRUE);
    else if (kind<4) ui.admitSuperweapon(0,c.power,c.make(2));
    else require(c.fonts.getFont(c.face,10,FALSE),"complete font acquisition");
  };
  AllocationFault::arm(ordinal);bool failed=false;try { action(); }catch(const std::bad_alloc&) { failed=true; }
  const auto count=AllocationFault::attempts();const bool hit=AllocationFault::triggered();AllocationFault::disarm();
  require(hit==failed,"allocation fault corresponds to propagated failure");
  if (failed) {
    require(ui.timer(c.name)==accepted && ui.powers()==(kind==3?1u:0u) && ui.entries()==(kind==3?1u:0u) && units.matches() && AllocationFault::live()==live,"failed admission preserves old entry and no orphan empty key");
    action();
  } else if (!discovering) require(count==ordinal,"independently proved exact terminal");
  require(kind<2 ? ui.timers()==1 && ui.timer(c.name)->timerText==c.newText : kind<4 ? ui.powers()==(kind==3?2u:1u) : c.fonts.getCount()==1,"terminal/corrected retry supplies complete source state");
  ui.clearTimerResources();
  if(kind==4) require(c.fonts.acquired-c.fonts.released==1,"font retry has one admitted backing");
  return count;
}
void allocations(Context& c,unsigned first,unsigned last) {
  for (unsigned kind=first;kind<last;++kind) {
    const auto terminal=allocationCase(c,kind,SIZE_MAX,true);
    require(terminal==(kind==0?1u:kind==1?0u:kind==2?2u:1u),"source standard allocation demand exact for warmed pools/cache");
    for (std::size_t n=0;n<=terminal;++n) allocationCase(c,kind,n);
    std::cout<<"source admission family "<<kind<<" prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
  }
}
void pools(Context& c) {
  const std::array<MemoryPool*,4> pools{TheMemoryPoolFactory->findMemoryPool("GameFont"),TheMemoryPoolFactory->findMemoryPool("NamedTimerInfo"),TheMemoryPoolFactory->findMemoryPool("SuperweaponInfo"),TheMemoryPoolFactory->findMemoryPool("OwnershipText")};
  for(unsigned n=0;n<pools.size();++n) {
   for(unsigned free=(n==3?1u:0u);;--free) {
    UI ui;std::array<void*,8> held{};unsigned count=0;
    // Every fixture pool is explicitly capacity8 with no overflow.
    while(pools[n]->getUsedBlockCount()<8-free) held[count++]=pools[n]->allocateBlock("generated text ownership exhaustion");
    const auto live=AllocationFault::live();Usage units;
    bool failed=false;
    try { if(n==0)c.fonts.getFont("GeneratedUncached",11,TRUE);else if(n==1)ui.addNamedTimer(c.name,c.newText,TRUE);else { auto* info=c.make();info->deleteInstance(); } }
    catch(ErrorCode e) { if(e!=ERROR_OUT_OF_MEMORY)throw;failed=true; }
    require(failed && units.matches() && AllocationFault::live()==live && !c.strings.count(),"actual typed pool exhaustion retires preceding candidate prefix");
    for(unsigned i=0;i<count;++i) pools[n]->freeBlock(held[i]);
    if(n==0) { require(c.fonts.getFont("GeneratedUncached",11,TRUE),"font pool retry");c.fonts.reset();c.fonts.getFont(c.face,10,FALSE); }
    else if(n==1) { ui.addNamedTimer(c.name,c.newText,TRUE);ui.clearTimerResources(); }
    else { auto* info=c.make();info->deleteInstance(); }
    if(!free)break;
   }
  }
  std::cout<<"four actual typed pools; both string constructor acquisitions; every failure/retry pair\n";
}
struct Client:NativeHeadlessGameClient {
  Client():NativeHeadlessGameClient(Composition::DrawablesOnly){}
  void compose(Hooks& hooks) {
    m_serviceOwners.create(TheFontLibrary,[]{return new Fonts;},false);
    m_serviceOwners.create(TheDisplayStringManager,[&]{return new Strings(hooks);});
    m_serviceOwners.create(TheInGameUI,[]{return new UI;});
  }
};
void clientRetirement() {
  require(!TheFontLibrary && !TheDisplayStringManager && !TheInGameUI,"client test requires unowned slots");
  Hooks hooks;
  { Client client;client.compose(hooks);
    TheInGameUI->addNamedTimer("GeneratedClientTimer",UnicodeString::TheEmptyString,FALSE);
    auto* cached=TheDisplayStringManager->newDisplayString();cached->setFont(TheFontLibrary->firstFont());
  }
  require(!TheFontLibrary && !TheDisplayStringManager && !TheInGameUI && !hooks.invalidBorrow,"actual GameClient destructor retires UI, manager strings, then font backing");
}
void clientFaults() {
  auto action=[] { clientRetirement(); };
  AllocationFault::arm(SIZE_MAX);action();const auto terminal=AllocationFault::attempts();AllocationFault::disarm();
  require(terminal>0 && terminal<64,"complete bounded client text cohort acquisition census");
  for(std::size_t n=0;n<=terminal;++n) {
    const auto live=AllocationFault::live();Usage units;
    AllocationFault::arm(n);bool failed=false;try { action(); }catch(const std::bad_alloc&) { failed=true; }
    const bool hit=AllocationFault::triggered();const auto count=AllocationFault::attempts();AllocationFault::disarm();
    require(failed==hit && failed==(n<terminal) && (failed || count==terminal),"complete client text cohort failure prefixes and exact terminal");
    require(!TheFontLibrary && !TheDisplayStringManager && !TheInGameUI && !TheDrawGroupInfo && units.matches() && AllocationFault::live()==live,"failed whole owner restores publications, typed units and standard ownership");
    action();require(units.matches() && AllocationFault::live()==live,"every whole client corrected retry retires exactly");
  }
  std::cout<<"whole source-client text cohort prefixes [0,"<<terminal<<"); terminal "<<terminal<<"; every retry\n";
}
}
int main(int argc,char** argv) {
  const auto initial=AllocationFault::live();bool initialized=false;
  try {
    require(argc==2,"text ownership test family");initMemoryManager();initialized=true;
    for(auto [name,size]:{std::pair{"GameFont",sizeof(GameFont)},std::pair{"NamedTimerInfo",sizeof(NamedTimerInfo)},std::pair{"SuperweaponInfo",sizeof(SuperweaponInfo)}})
      TheMemoryPoolFactory->createMemoryPool(name,size,8,0);
    { Parents parents;
      // Warm the identical complete parent/string/font/UI/client cohort before
      // capturing registry capacity; never exempt individual retained allocations.
      for(unsigned n=0;n<2;++n) { { Context c;functional(c);negative(c); }clientRetirement(); }
      const auto live=AllocationFault::live();Usage units;
      for(unsigned repeat=0;repeat<3;++repeat) {
        { Context c;const std::string_view family=argv[1];
          if(family=="functional") functional(c);
          else if(family=="negative") negative(c);
          else if(family=="font-faults") allocations(c,4,5);
          else if(family=="timer-faults") { callbacks(c,false);allocations(c,0,2); }
          else if(family=="superweapon-faults") { callbacks(c,true);allocations(c,2,4); }
          else if(family=="pool-failures") pools(c);
          else if(family=="client-retirement" || family=="client-faults") {}else require(false,"unknown text family");
          require(!c.hooks.invalidBorrow,"strings never outlive their font backing");
        }
        if(std::string_view(argv[1])=="client-faults")clientFaults();else clientRetirement();
        require(units.matches() && AllocationFault::live()==live,"repeated whole text/UI/client cohorts retire exactly");
      }
    }
    shutdownMemoryManager();initialized=false;require(AllocationFault::live()==initial,"whole memory manager metadata retirement");
    std::cout<<"PASS original shared text ownership; native text/root/output remain pending\n";return 0;
  }catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; }
  catch(ErrorCode e) { std::cerr<<"FAIL text owner "<<Int(e)<<'\n'; }
  AllocationFault::disarm();if(initialized)shutdownMemoryManager();return 1;
}
