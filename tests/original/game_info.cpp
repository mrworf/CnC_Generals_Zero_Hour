// SPDX-License-Identifier: GPL-3.0-or-later
// Actual GameInfo/slot/map owners; no replacement simulation or LAN provider.
#include "AllocationFault.h"
#include "Common/GlobalData.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/GameMemory.h"
#include "Common/Recorder.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameText.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/FileTransfer.h"
#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <dirent.h>

namespace {
void require(bool value,const char* label) { if (!value) throw std::runtime_error(label); }
unsigned descriptors() {
    DIR* directory=::opendir("/proc/self/fd"); require(directory,"descriptor census");
    unsigned count=0; while (::readdir(directory)) ++count; ::closedir(directory); return count;
}
template<class F> void rejects(F action) {
    bool rejected=false; try {action();} catch (ErrorCode) {rejected=true;}
    require(rejected,"actual source rejection");
}
struct Tree {
    std::filesystem::path root;
    Tree() { char path[]="/tmp/zh-game-info-XXXXXX"; const auto* selected=::mkdtemp(path);
        require(selected,"generated root"); root=selected; }
    ~Tree() { std::error_code ignored; std::filesystem::remove_all(root,ignored); }
    void put(const char* name,const char* content) {
        auto path=root/name; std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path,std::ios::binary); output<<content; require(bool(output),"generated input");
    }
};
struct Context {
    Tree tree;
    FileSystem files;
    MapCache maps;
    std::unique_ptr<GlobalData> data;
    std::unique_ptr<GameTextInterface> text;
    Context() {
        tree.put("data/Generals.str","GUI:Open\n\"Open\"\nEND\nGUI:Closed\n\"Closed\"\nEND\n"
            "GUI:EasyAI\n\"Easy AI\"\nEND\nGUI:MediumAI\n\"Medium AI\"\nEND\nGUI:HardAI\n\"Hard AI\"\nEND\n");
        for (auto leaf:{"Generated.tga","map.ini","map.str","solo.ini","assetusage.txt","readme.txt"}) {
            const auto path=std::string("Maps/Generated/")+leaf; tree.put(path.c_str(),"generated companion");
        }
        files.mountReadOnly({tree.root.string()}); TheFileSystem=&files;
        data=std::make_unique<GlobalData>(); TheWritableGlobalData=data.get();
        text.reset(CreateGameTextInterface()); text->init(); TheGameText=text.get();
        TheMapCache=&maps;
        MapMetaData metadata; metadata.m_CRC=0xabc; metadata.m_filesize=42; metadata.m_numPlayers=4;
        maps.publishMetadata("maps/generated/generated.map",std::move(metadata));
    }
    ~Context() {
        TheMapCache=nullptr; TheGameText=nullptr; text.reset();
        TheWritableGlobalData=nullptr; data.reset(); TheFileSystem=nullptr;
    }
};
struct Setup : GameInfo {
    std::array<GameSlot,MAX_SLOTS> slots;
    Setup() {
        for (Int i=0;i<MAX_SLOTS;++i) {
            setSlotPointer(i,&slots[i]);
            UnicodeString name; name.format(L"accepted player %d",i);
            slots[i].setState(SLOT_PLAYER,name,UnsignedInt(i+1));
            slots[i].setPort(UnsignedShort(200+i)); slots[i].setColor(i);
            slots[i].setStartPos(i); slots[i].setPlayerTemplate(i); slots[i].setTeamNumber(i%2);
            slots[i].saveOffOriginalInfo(); slots[i].setAccept(); slots[i].setLastFrameInGame(100+i);
            slots[i].mute(i%2); if (i%2) slots[i].markAsDisconnected();
        }
        setMap("Maps/Accepted/Accepted.map"); setMapCRC(1); setMapSize(2); setMapContentsMask(3);
        setSeed(4); setCRCInterval(5); setUseStats(6); setSuperweaponRestriction(7);
        setOldFactionsOnly(true); setLocalIP(1); markPlayerAsPreorder(3);
        setInGame();
    }
    std::array<UnsignedInt,17> scalars() const {
        return {UnsignedInt(m_preorderMask),UnsignedInt(m_crcInterval),UnsignedInt(m_inGame),
            UnsignedInt(m_inProgress),UnsignedInt(m_surrendered),UnsignedInt(m_gameID),
            m_localIP,m_mapCRC,m_mapSize,UnsignedInt(m_mapMask),UnsignedInt(m_seed),
            UnsignedInt(m_useStats),m_startingCash.countMoney(),m_superweaponRestriction,
            UnsignedInt(m_oldFactionsOnly),UnsignedInt(getNumPlayers()),UnsignedInt(getLocalSlotNum())};
    }
};
std::array<UnsignedInt,18> slotScalars(const GameSlot& slot) {
    return {UnsignedInt(slot.getState()),UnsignedInt(slot.isAccepted()),UnsignedInt(slot.hasMap()),
        UnsignedInt(slot.isMuted()),UnsignedInt(slot.getColor()),UnsignedInt(slot.getStartPos()),
        UnsignedInt(slot.getPlayerTemplate()),UnsignedInt(slot.getTeamNumber()),
        UnsignedInt(slot.getOriginalColor()),UnsignedInt(slot.getOriginalStartPos()),
        UnsignedInt(slot.getOriginalPlayerTemplate()),slot.getIP(),slot.getPort(),
        UnsignedInt(slot.getNATBehavior()),slot.lastFrameInGame(),UnsignedInt(slot.disconnected()),
        UnsignedInt(slot.isOccupied()),UnsignedInt(slot.isAI())};
}
struct Accepted {
    std::array<UnsignedInt,17> scalars;
    AsciiString map;
    std::array<GameSlot,MAX_SLOTS> slots;
    std::array<const GameSlot*,MAX_SLOTS> links;
    template<class OWNER> explicit Accepted(const OWNER& owner) : scalars(owner.scalars()),map(owner.getMap()) {
        for (Int i=0;i<MAX_SLOTS;++i) { links[i]=owner.getConstSlot(i); slots[i]=*links[i]; }
    }
    template<class OWNER> void verifyPayload(const OWNER& owner) const {
        require(owner.scalars()==scalars,"complete game scalar restoration");
        require(owner.getMap().str()==map.str(),"exact accepted map backing");
        for (Int i=0;i<MAX_SLOTS;++i) {
            const auto* slot=owner.getConstSlot(i);
            require(slotScalars(*slot)==slotScalars(slots[i]),"complete slot scalar restoration");
            require(slot->getName().str()==slots[i].getName().str(),"exact accepted name backing");
        }
    }
    template<class OWNER> void verify(const OWNER& owner) const {
        verifyPayload(owner);
        for (Int i=0;i<MAX_SLOTS;++i) require(owner.getConstSlot(i)==links[i],"actual parent slot identity retained");
    }
};
void change(Setup& owner) {
    for (Int i=0;i<MAX_SLOTS;++i) {
        GameSlot candidate; UnicodeString name; name.format(L"candidate player %d with new backing",i);
        candidate.setState(SLOT_PLAYER,name,UnsignedInt(i+1));
        candidate.setPlayerTemplate(-1); candidate.setTeamNumber(-1);
        owner.setSlot(i,candidate);
    }
    owner.setMap("Maps/Generated/Generated.map"); owner.setMapCRC(0xabc); owner.setMapSize(42);
    owner.setSeed(91); owner.setCRCInterval(92); owner.setUseStats(0);
    owner.setSuperweaponRestriction(93); owner.setOldFactionsOnly(false);
    owner.setLocalIP(2); owner.markPlayerAsPreorder(7); owner.markAsSurrendered();
    owner.setGameInProgress(true);
}
void transactions() {
    alignas(GameSlot) std::array<unsigned char,sizeof(GameSlot)> bytes;
    bytes.fill(0xa5); auto* fresh=::new(bytes.data()) GameSlot;
    require(fresh->getIP()==0 && fresh->getPort()==0,"nonzero fresh backing fully initialized");
    fresh->setIP(99); fresh->setName(UnicodeString(L"preserved reset name")); fresh->reset();
    require(fresh->getIP()==99 && fresh->getName().compare(L"preserved reset name")==0,"source reset preserves IP and name");
    fresh->~GameSlot();
    Context context; Setup owner; Accepted accepted(owner);
    {
        NativeGameInfoTransaction outer(owner);
        { NativeGameInfoTransaction inner(owner); change(owner); inner.commit(); }
        require(owner.getMapContentsMask()==127 && owner.getConstSlot(0)->hasMap(),"source seven-bit companion mask and availability");
    }
    accepted.verify(owner);
    {
        NativeGameInfoTransaction outer(owner); change(owner); Accepted candidate(owner);
        { NativeGameInfoTransaction inner(owner); owner.reset(); }
        candidate.verify(owner); outer.commit();
    }
    require(owner.getLocalSlotNum()==1,"actual source IP selection after publication");
    owner.setMapCRC(0xabd); require(!owner.getSlot(1)->hasMap(),"actual CRC mismatch marks local slot missing");
    owner.setMapSize(42); require(!owner.getSlot(1)->hasMap(),"source size setter intentionally compares CRC");
    owner.setMapCRC(0xabc); require(owner.getSlot(1)->hasMap(),"matching source CRC restores availability");
    Accepted changed(owner);
    auto* priorText=TheGameText; TheGameText=nullptr;
    rejects([&]{owner.getSlot(1)->setState(SLOT_OPEN);}); changed.verify(owner);
    TheGameText=priorText;
    owner.closeOpenSlots();
    require(owner.getConstSlot(0)==&owner.slots[0],"slot publication never copies parent pointers");
    require(GetINIFromMap("Maps/Generated/Generated.map")=="Maps/Generated\\map.ini"
        && GetStrFileFromMap("Maps\\Generated\\Generated.map")=="Maps\\Generated\\map.str"
        && GetReadmeFromMap("Generated.map")=="readme.txt","slash/backslash/root-level source path regression");
}
void faults() {
    for (Int life=0;life<3;++life) {
        const auto whole=AllocationFault::live(); const auto wholeFD=descriptors();
        {
            Context context; Setup owner; Accepted accepted(owner);
            const auto baseline=AllocationFault::live(); const auto fds=descriptors();
            std::size_t count=0;
            { NativeGameInfoTransaction transaction(owner); AllocationFault::arm(std::numeric_limits<std::size_t>::max());
              try {change(owner);} catch (...) {AllocationFault::disarm();throw;}
              count=AllocationFault::attempts(); AllocationFault::disarm(); }
            accepted.verify(owner); require(AllocationFault::live()==baseline,"discovery retires before faults");
            require(count>0,"actual complete operation census");
            for (std::size_t ordinal=0;ordinal<=count;++ordinal) {
                bool failed=false;
                {
                    NativeGameInfoTransaction transaction(owner); AllocationFault::arm(ordinal);
                    try {change(owner);} catch(const std::bad_alloc&) {failed=true;}
                    catch(...) {AllocationFault::disarm();throw;}
                    const auto attempts=AllocationFault::attempts(); const auto hit=AllocationFault::triggered();
                    AllocationFault::disarm();
                    require(failed==(ordinal<count) && hit==failed,"every failure and exact terminal");
                    if(!failed)require(attempts==count,"terminal proves full observed demand");
                }
                accepted.verify(owner);
                require(AllocationFault::live()==baseline && descriptors()==fds,"immediate exact resource residuals");
                { NativeGameInfoTransaction retry(owner); change(owner); }
                accepted.verify(owner);
                require(AllocationFault::live()==baseline && descriptors()==fds,"same owner corrected retry residuals");
            }
            std::printf("game-info life=%d allocations=%zu failures=%zu terminal=%zu\n",life,count,count,count);
        }
        require(AllocationFault::live()==whole && descriptors()==wholeFD,"whole owner retirement across lifetimes");
    }
}
void setterFaults() {
    for (Int life=0;life<3;++life) {
        const auto whole=AllocationFault::live(); const auto wholeFD=descriptors();
        {
            Context context; Setup owner;
            owner.setMap("Maps/Generated/Generated.map"); owner.setMapCRC(0xabc);
            owner.getSlot(MAX_SLOTS-1)->setState(SLOT_OPEN);
            Accepted accepted(owner);
            auto apply=[&](Int operation) {
                switch(operation) {
                case 0: owner.setMap("Maps/Generated/Generated.map"); break;
                case 1: owner.setMapCRC(0xabd); break;
                case 2: owner.setMapSize(123); break;
                case 3: owner.reset(); break;
                case 4: owner.clearSlotList(); break;
                case 5: owner.adjustSlotsForMap(); break;
                case 6: owner.startGame(42); break;
                default: throw std::runtime_error("bad fixture operation");
                }
            };
            const auto baseline=AllocationFault::live(); const auto fds=descriptors();
            for (Int operation=0;operation<7;++operation) {
                std::size_t count=0;
                { NativeGameInfoTransaction undo(owner); AllocationFault::arm(std::numeric_limits<std::size_t>::max());
                  try {apply(operation);} catch(...) {AllocationFault::disarm();throw;}
                  count=AllocationFault::attempts(); AllocationFault::disarm(); }
                accepted.verify(owner);
                for (std::size_t ordinal=0;ordinal<=count;++ordinal) {
                    {
                        NativeGameInfoTransaction undo(owner); AllocationFault::arm(ordinal);
                        bool failed=false;
                        try {apply(operation);} catch(const std::bad_alloc&) {failed=true;}
                        catch(...) {AllocationFault::disarm();throw;}
                        const auto hit=AllocationFault::triggered(); const auto attempts=AllocationFault::attempts();
                        AllocationFault::disarm();
                        require(failed==(ordinal<count) && hit==failed,"setter exact fault terminal");
                        if(failed) {
                            // Prove the production setter has already rolled back, BEFORE
                            // the test's successful-operation restoration guard retires.
                            accepted.verify(owner);
                            require(AllocationFault::live()==baseline && descriptors()==fds,"setter immediate rejection residuals");
                        } else require(attempts==count,"setter full demand terminal");
                    }
                    accepted.verify(owner);
                    { NativeGameInfoTransaction undo(owner); apply(operation); }
                    accepted.verify(owner);
                    require(AllocationFault::live()==baseline && descriptors()==fds,"setter same-owner retry residuals");
                }
                std::printf("game-info-setter life=%d operation=%d allocations=%zu terminal=%zu\n",life,operation,count,count);
            }
        }
        require(AllocationFault::live()==whole && descriptors()==wholeFD,"setter whole owner retirement");
    }
}
struct ReplayProbe : ReplayGameInfo {
    std::array<UnsignedInt,17> scalars() const {
        return {UnsignedInt(m_preorderMask),UnsignedInt(m_crcInterval),UnsignedInt(m_inGame),
            UnsignedInt(m_inProgress),UnsignedInt(m_surrendered),UnsignedInt(m_gameID),
            m_localIP,m_mapCRC,m_mapSize,UnsignedInt(m_mapMask),UnsignedInt(m_seed),
            UnsignedInt(m_useStats),m_startingCash.countMoney(),m_superweaponRestriction,
            UnsignedInt(m_oldFactionsOnly),UnsignedInt(getNumPlayers()),UnsignedInt(getLocalSlotNum())};
    }
};
void adoption() {
    for (Int life=0;life<3;++life) {
        const auto whole=AllocationFault::live(); const auto wholeFD=descriptors();
        {
            Context context; ReplayProbe owner,candidate;
            for (Int i=0;i<MAX_SLOTS;++i) {
                for (auto* setup:{&owner,&candidate}) {
                    GameSlot slot; UnicodeString name;
                    name.format(setup==&owner?L"accepted %d":L"candidate %d",i);
                    slot.setState(SLOT_PLAYER,name,UnsignedInt(i+1));
                    slot.setColor(setup==&owner?i:-1); slot.setStartPos(i);
                    slot.setPlayerTemplate(i); slot.saveOffOriginalInfo();
                    slot.setPort(UnsignedShort(200+i)); slot.setTeamNumber(i%2);
                    slot.setLastFrameInGame(123+i); slot.mute(i%2);
                    if(i%2)slot.markAsDisconnected(); setup->setSlot(i,slot);
                }
            }
            owner.setMap("Maps/Accepted/Accepted.map"); candidate.setMap("Maps/Generated/Generated.map");
            owner.setMapCRC(1); candidate.setMapCRC(2); owner.setMapSize(3); candidate.setMapSize(4);
            owner.setMapContentsMask(5); candidate.setMapContentsMask(127);
            owner.setSeed(10); candidate.setSeed(20); owner.setCRCInterval(30); candidate.setCRCInterval(40);
            owner.setUseStats(0); candidate.setUseStats(1); owner.setLocalIP(1); candidate.setLocalIP(2);
            owner.setSuperweaponRestriction(9); candidate.setSuperweaponRestriction(10);
            owner.setOldFactionsOnly(false); candidate.setOldFactionsOnly(true);
            owner.markPlayerAsPreorder(2); candidate.markPlayerAsPreorder(3);
            candidate.markAsSurrendered(); candidate.setGameInProgress(true);
            owner.setInGame(); candidate.setInGame();
            Accepted prior(owner),incoming(candidate);
            const auto baseline=AllocationFault::live();
            for (Int repeat=0;repeat<3;++repeat) {
                AllocationFault::arm(0); owner.swap(candidate);
                const auto count=AllocationFault::attempts(); const auto hit=AllocationFault::triggered();
                AllocationFault::disarm(); require(count==0 && !hit,"real replay setup adoption is allocation-free");
                incoming.verifyPayload(owner); prior.verifyPayload(candidate);
                for (Int i=0;i<MAX_SLOTS;++i) {
                    require(owner.getConstSlot(i)==prior.links[i] && candidate.getConstSlot(i)==incoming.links[i]
                        && prior.links[i]!=incoming.links[i],"both embedded slot arrays keep their parent identity");
                }
                owner.swap(candidate); prior.verify(owner); incoming.verify(candidate);
                { NativeGameInfoTransaction target(owner),source(candidate); owner.swap(candidate); }
                prior.verify(owner); incoming.verify(candidate);
                require(AllocationFault::live()==baseline,"adoption and nested rollback exact backing retirement");
            }
            std::printf("replay-setup life=%d adoptions=3 allocations=0\n",life);
        }
        require(AllocationFault::live()==whole && descriptors()==wholeFD,"both actual replay setup owners retire exactly");
    }
}
}
int main(int argc,char** argv) {
    bool initialized=false;
    try {
        if(argc!=2)return 2;
        const std::string_view family=argv[1];
        if(family!="transactions" && family!="faults" && family!="setter-faults" && family!="adoption")return 2;
        initMemoryManager(); initialized=true;
        { Context warm; Setup owner; }
        if(family=="transactions")transactions();else if(family=="faults")faults();
        else if(family=="setter-faults")setterFaults();else adoption();
        shutdownMemoryManager(); initialized=false;
        return 0;
    } catch(const std::exception& error) {AllocationFault::disarm();std::fprintf(stderr,"%s\n",error.what());}
      catch(...) {AllocationFault::disarm();std::fprintf(stderr,"source owner rejected unexpectedly\n");}
    if(initialized)shutdownMemoryManager();
    return 1;
}
