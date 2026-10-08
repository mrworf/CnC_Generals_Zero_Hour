// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/GlobalData.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/FileSystem.h"
#include "Common/NativeUserStorage.h"
#include "Common/NativeInputSettings.h"
#include "Common/NativeSubsystemInit.h"
#include "Common/UserPreferences.h"
#include "Common/version.h"
#include "Common/crc.h"
#include "Common/Registry.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameText.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/Image.h"
#include "GameLogic/WeaponBonus.h"
#include "GameNetwork/IPEnumeration.h"
#include <SDL3/SDL.h>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <limits>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <array>
namespace {
void require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F action) {
    bool rejected=false; try{action();}catch(ErrorCode){rejected=true;}catch(const INIException&){rejected=true;}
    require(rejected,"source input rejected");
}
unsigned descriptors() {
    DIR* directory=::opendir("/proc/self/fd");require(directory,"descriptor census");unsigned count=0;
    while(::readdir(directory))++count;::closedir(directory);return count;
}
struct Tree {
    std::filesystem::path root;
    Tree() {char name[]="/tmp/zh-configuration-XXXXXX";char* path=::mkdtemp(name);require(path,"generated root");root=path;}
    ~Tree(){std::error_code ignored;std::filesystem::remove_all(root,ignored);}
};
const INIBlockDefinition blocks[]{{"GameData",GlobalData::parseGameDataDefinition}};
struct LineCensus {
    CRC crc;
    unsigned calls=0;
    unsigned fail=std::numeric_limits<unsigned>::max();
    static void line(void* owner,const char* bytes,Int count) {
        auto& census=*static_cast<LineCensus*>(owner);
        if(census.calls++==census.fail)throw ERROR_BAD_ARG;
        census.crc.computeCRC(bytes,count);
    }
};
struct Context {
    Tree assets,user;
    FileSystem files;
    NativeUserStorage storage;
    Version version;
    Context() : storage(NativeUserPaths::resolve(user.root.string(),(user.root/"data").string(),
                                               (user.root/"cache").string()),files) {
        put("Data/Scripts/SkirmishScripts.scb","generated skirmish");
        put("Data/Scripts/MultiplayerScripts.scb","generated multiplayer");
        put("accepted.ini","GameData\nXResolution = 640\nYResolution = 480\nUseTrees = No\n"
            "WeaponBonus = VETERAN DAMAGE 125%\nDefaultStartingCash = 12345\nEnd\n");
        put("candidate.ini","GameData\nXResolution = 1280\nYResolution = 720\nUseTrees = Yes\n"
            "MapName = candidate\nWeaponBonus = VETERAN DAMAGE 175%\nEnd\n");
        put("bad.ini","GameData\nXResolution = 999\nWeaponBonus = VETERAN DAMAGE 200%\nUnknown = bad\nEnd\n");
        put("values.ini","Mask = B31\nByteMask = +B7\nDuration = 2184500\nEnd\n");
        put("mixed.ini","Mask = +B0 B31\nEnd\n");
        put("wide-mask.ini","Mask = B0 B32\nEnd\n");
        put("wide-byte.ini","ByteMask = +B0 +B8\nEnd\n");
        put("wide-duration.ini","Duration = 2184534\nEnd\n");
        put("missing-values.ini","ByteMask =\nEnd\n");
        put("remove-byte.ini","ByteMask = -B7\nEnd\n");
        files.mountReadOnly({assets.root.string()});
        TheFileSystem=&files;TheNativeUserStorage=&storage;TheVersion=&version;
    }
    ~Context() {
        // Registered root must own override teardown even after withdrawal.
        GlobalData* root=original;
        TheWritableGlobalData=nullptr;delete root;
        TheVersion=nullptr;TheNativeUserStorage=nullptr;TheFileSystem=nullptr;
    }
    GlobalData* original=nullptr;
    void put(const char* name,const char* text) {
        const auto path=assets.root/name;std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path,std::ios::binary);file<<text;require(bool(file),"generated definition");
    }
    void load(const char* name,INILoadType type) {INI ini;ini.loadBlocks(name,type,blocks);}
    void start() {load("accepted.ini",INI_LOAD_OVERWRITE);original=TheWritableGlobalData;original->setName("registered original");}
    void preferences(const char* text) {
        auto output=storage.beginWrite(NativeUserArea::Data,"Options.ini");
        output->write(text,static_cast<Int>(std::strlen(text)));output->commit();
    }
};
Real damage(const GlobalData& data) {
    WeaponBonus bonus;data.m_weaponBonusSet->appendBonuses(UnsignedInt{1}<<WEAPONBONUSCONDITION_VETERAN,bonus);
    return bonus.getField(WeaponBonus::DAMAGE);
}
const char* acceptedLocale =
    "Language\nUnicodeFontName = GeneratedFace\nUseHardWordWrap = Yes\n"
    "CreditsTitleFont = \"Generated Credits\" 21 Yes\n"
    "LocalFontFile = Data\\Fonts\\generated.font\nEnd\n";
const char* candidateLocale =
    "Language\nUnicodeFontName = ChangedGeneratedFace\nUseHardWordWrap = No\n"
    "CreditsTitleFont = \"Changed Generated Credits\" 31 No\n"
    "LocalFontFile = Data\\Fonts\\generated.font\nEnd\n";
struct LocaleContext {
    Context context;
    NameKeyGenerator names{128};
    GlobalLanguage language;
    NameKeyGenerator* previousNames = TheNameKeyGenerator;
    GlobalLanguage* previousLanguage = TheGlobalLanguageData;
    LocaleContext() {
        context.put("Data/Fonts/generated.font", "generated existence fixture, not a rendered font");
        input(acceptedLocale);
        names.init();
        language.setName("registered-language-owner");
        // All fallible fixture preparation precedes borrowed publication.
        TheNameKeyGenerator = &names;
        TheGlobalLanguageData = &language;
    }
    ~LocaleContext() {
        TheGlobalLanguageData = previousLanguage;
        TheNameKeyGenerator = previousNames;
    }
    void input(const char* text) {
        AsciiString filename;
        filename.format("Data/%s/Language.ini", GetRegistryLanguage().str());
        context.put(filename.str(), text);
        context.files.mountReadOnly({context.assets.root.string()});
    }
};
struct LocaleSnapshot {
    const char* face;
    const char* title;
    const AsciiString* fontHead;
    std::size_t count;
    Int titleSize;
    Bool wrap;
    explicit LocaleSnapshot(const GlobalLanguage& value)
      : face(value.m_unicodeFontName.str()), title(value.m_creditsTitleFont.name.str()),
        fontHead(value.m_localFonts.empty() ? nullptr : &value.m_localFonts.front()),
        count(value.m_localFonts.size()), titleSize(value.m_creditsTitleFont.size),
        wrap(value.m_useHardWrap) { }
    void unchanged(const GlobalLanguage& value) const {
        require(value.m_unicodeFontName.str()==face && value.m_creditsTitleFont.name.str()==title &&
            value.m_localFonts.size()==count &&
            (value.m_localFonts.empty()?nullptr:&value.m_localFonts.front())==fontHead &&
            value.m_creditsTitleFont.size==titleSize && value.m_useHardWrap==wrap,
            "locale rejection preserves accepted descriptor and list backing");
    }
};
void locale() {
    LocaleContext owner;
    owner.language.init();
    require(owner.language.m_unicodeFontName=="GeneratedFace" && owner.language.m_useHardWrap &&
        owner.language.m_creditsTitleFont.name=="Generated Credits" &&
        owner.language.m_creditsTitleFont.size==21 && owner.language.m_creditsTitleFont.bold &&
        owner.language.m_localFonts.size()==1, "actual selected language/font configuration");
    for (const char* rejected : {
        "Language\nUnicodeFontName = RejectedFace\nUnknown = late\nEnd\n",
        "Language\nUnicodeFontName = RejectedFace\nLocalFontFile = Data\\Fonts\\missing.font\nEnd\n",
        "Language\nUnicodeFontName = RejectedFace\nEnd\nGameData\nXResolution = 999\nEnd\n"}) {
        owner.input(rejected);
        LocaleSnapshot accepted(owner.language);
        const auto live=AllocationFault::live();const auto fds=descriptors();
        for(int repeat=0;repeat<3;++repeat) {
            rejects([&]{owner.language.init();});accepted.unchanged(owner.language);
            require(TheGlobalLanguageData==&owner.language && !TheWritableGlobalData &&
                AllocationFault::live()==live && descriptors()==fds,
                "whole locale rejection restores globals and retires all backing");
        }
    }
    owner.input(candidateLocale);owner.language.init();
    require(owner.language.getName()=="registered-language-owner" &&
        owner.language.m_unicodeFontName=="ChangedGeneratedFace" &&
        owner.language.m_creditsTitleFont.size==31 && !owner.language.m_creditsTitleFont.bold &&
        !owner.language.m_useHardWrap && owner.language.m_localFonts.size()==2,
        "same-owner corrected locale retry retains registry identity and source append semantics");
}
void localeFaults() {
    std::size_t census;
    {
        LocaleContext owner;owner.language.init();owner.input(candidateLocale);
        AllocationFault::arm(std::numeric_limits<std::size_t>::max());
        owner.language.init();census=AllocationFault::attempts();AllocationFault::disarm();
        require(census>0 && census<256, "complete bounded actual locale allocation census");
    }
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
        LocaleContext owner;owner.language.init();owner.input(candidateLocale);
        LocaleSnapshot accepted(owner.language);
        const auto live=AllocationFault::live();const auto fds=descriptors();
        const auto files=TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount();
        bool failed=false;AllocationFault::arm(ordinal);
        try{owner.language.init();}catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}
        const auto attempts=AllocationFault::attempts();const auto triggered=AllocationFault::triggered();
        AllocationFault::disarm();
        if(ordinal<census) {
            require(triggered && failed, "every locale allocation ordinal rejects");
            accepted.unchanged(owner.language);
            require(AllocationFault::live()==live && descriptors()==fds &&
                TheGlobalLanguageData==&owner.language &&
                TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount()==files,
                "locale allocation failure restores owner and exact resource residuals");
            owner.language.init();
        } else require(!triggered && !failed && attempts==census,
                       "locale exact terminal succeeds with complete census");
        require(owner.language.m_unicodeFontName=="ChangedGeneratedFace" &&
            owner.language.m_creditsTitleFont.size==31 && owner.language.m_localFonts.size()==2 &&
            owner.language.getName()=="registered-language-owner", "locale fault corrected retry");
    }
    std::cout<<"locale allocation terminal "<<census<<'\n';
}
const INIBlockDefinition imageBlocks[]{{"MappedImage",INI::parseMappedImageDefinition}};
const char* acceptedImage =
    "MappedImage Stable\nTexture = accepted.tga\nTextureWidth = 256\nTextureHeight = 128\n"
    "Coords = Left:16 Top:8 Right:48 Bottom:24\nStatus = ROTATED_90_CLOCKWISE\nEnd\n";
const char* candidateImage =
    "MappedImage Stable\nTexture = changed.tga\nTextureWidth = 512\nTextureHeight = 256\n"
    "Coords = Left:32 Top:16 Right:96 Bottom:48\nStatus = NONE\nEnd\n";
class InspectedImages final : public ImageCollection {
public:
    struct Entry {
        const void* node;
        unsigned key;
        const Image* image;
        const char* name;
        const char* filename;
        ICoord2D texture,size;
        Region2D uv;
        const void* raw;
        UnsignedInt status;
    };
    struct Snapshot {std::array<Entry,8> entries{};std::size_t count=0;};
    Snapshot snapshot() const {
        Snapshot result;
        require(m_imageMap.size()<=result.entries.size(),"bounded generated image index");
        for (const auto& entry:m_imageMap) {
            const auto& value=*entry.second;
            result.entries[result.count++]={&entry,entry.first,&value,value.getName().str(),
                value.getFilename().str(),*value.getTextureSize(),*value.getImageSize(),
                *value.getUV(),value.getRawTextureData(),value.getStatus()};
        }
        return result;
    }
    void unchanged(const Snapshot& original) const {
        const auto current=snapshot();
        require(current.count==original.count,"rejected image index retains cardinality");
        for (std::size_t i=0;i<current.count;++i) {
            const auto& a=current.entries[i];const auto& b=original.entries[i];
            require(a.node==b.node && a.key==b.key && a.image==b.image && a.name==b.name &&
                a.filename==b.filename && a.texture.x==b.texture.x && a.texture.y==b.texture.y &&
                a.size.x==b.size.x && a.size.y==b.size.y && a.raw==b.raw && a.status==b.status &&
                a.uv.lo.x==b.uv.lo.x && a.uv.lo.y==b.uv.lo.y &&
                a.uv.hi.x==b.uv.hi.x && a.uv.hi.y==b.uv.hi.y,
                "rejected image graph preserves exact nodes, identities and payload backing");
        }
    }
};
struct ImageContext {
    Context context;
    NameKeyGenerator names{128};
    InspectedImages images;
    NameKeyGenerator* previousNames=TheNameKeyGenerator;
    ImageCollection* previousImages=TheMappedImageCollection;
    ImageContext() {
        names.init();images.setName("registered-image-owner");
        input(acceptedImage);
        context.files.attachUserStorage(&context.storage);
        TheNameKeyGenerator=&names;TheMappedImageCollection=&images;
    }
    ~ImageContext(){TheMappedImageCollection=previousImages;TheNameKeyGenerator=previousNames;}
    void remount() {
        // A mounted user owner freezes asset publication. Generated fixture
        // changes occur outside transactions, with the real binding withdrawn.
        context.files.attachUserStorage(nullptr);
        context.files.mountReadOnly({context.assets.root.string()});
        context.files.attachUserStorage(&context.storage);
    }
    void input(const char* text) {
        context.put("images.ini",text);remount();
    }
    void parse(){INI ini;ini.loadBlocks("images.ini",INI_LOAD_OVERWRITE,imageBlocks);}
    void collectionInput(const char* last=candidateImage) {
        context.put("Data/INI/MappedImages/TextureSize_64/z-root.ini",
            "MappedImage NewImage\nTexture = new.tga\nEnd\n");
        context.put("Data/INI/MappedImages/TextureSize_64/a-child/final.ini",last);
        remount();
    }
    NameKeyType nextKey() {
        NameKeyTransaction scope(names);return names.nameToKey("generated-unpublished-sentinel");
    }
};
struct ImageResources {
    std::size_t live=AllocationFault::live();
    unsigned fds=descriptors();
    Int images=TheMemoryPoolFactory->findMemoryPool("Image")->getUsedBlockCount();
    Int keys=TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount();
    Int files=TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount();
    void unchanged() const {
        require(AllocationFault::live()==live && descriptors()==fds &&
            TheMemoryPoolFactory->findMemoryPool("Image")->getUsedBlockCount()==images &&
            TheMemoryPoolFactory->findMemoryPool("NameKeyBucketPool")->getUsedBlockCount()==keys &&
            TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount()==files,
            "image rejection retires all standard, pooled, namespace and descriptor units");
    }
};
void imagesFunctional() {
    ImageContext owner;owner.parse();
    const auto* stable=owner.images.findImageByName("sTaBlE");
    require(stable && stable->getFilename()=="accepted.tga" && stable->getImageWidth()==16 &&
        stable->getImageHeight()==32 && stable->getUV()->lo.x==0.0625f &&
        stable->getUV()->hi.y==0.1875f,"actual source UV and clockwise size protocol");
    const auto key=owner.nextKey();
    require(!owner.images.findImageByName("absent-generated-image") && owner.nextKey()==key,
        "missing lookup does not consume a namespace ordinal");
    owner.input(candidateImage);owner.parse();
    require(owner.images.findImageByName("Stable")==stable && stable->getFilename()=="changed.tga" &&
        stable->getImageWidth()==64 && stable->getImageHeight()==32,
        "per-definition replacement preserves borrowed accepted Image address");
    owner.context.start();
    auto user=[&](const char* path,const char* text) {
        auto output=owner.context.storage.beginWrite(NativeUserArea::Data,path);
        output->write(text,static_cast<Int>(std::strlen(text)));output->commit();
    };
    user("INI/MappedImages/a-child/ignored.ini","MappedImage Gate\nTexture = gated-child.tga\nEnd\n");
    owner.images.load(64);
    require(!owner.images.findImageByName("Gate"),"user discovery requires a top-level INI");
    user("INI/MappedImages/z-root.ini","MappedImage Stable\nTexture = user.tga\nEnd\n");
    user("INI/MappedImages/a-child/ignored.ini","MappedImage Stable\nTexture = user-child.tga\nEnd\n"
        "MappedImage Gate\nTexture = gated-child.tga\nEnd\n");
    owner.context.put("Data/INI/MappedImages/TextureSize_64/z-root.ini",
        "MappedImage Stable\nTexture = size-root.tga\nEnd\n");
    owner.context.put("Data/INI/MappedImages/TextureSize_64/a-child/final.ini",
        "MappedImage Stable\nTexture = size-child.tga\nEnd\n");
    owner.remount();
    owner.images.load(64);
    require(owner.images.findImageByName("Stable")==stable && stable->getFilename()=="size-child.tga" &&
        owner.images.findImageByName("Gate"),"root-before-child and user-before-size precedence");
    owner.context.put("Data/INI/MappedImages/HandCreated/z-root.ini",
        "MappedImage Stable\nTexture = hand.tga\nEnd\n");
    owner.remount();owner.images.load(64);
    require(owner.images.findImageByName("Stable")==stable && stable->getFilename()=="hand.tga" &&
        owner.images.getName()=="registered-image-owner","whole collection commit preserves addresses and final hand-created precedence");
    INI ini;ini.loadDirectoryBlocks("DATA//INI\\MappedImages\\TextureSize_64///",FALSE,
        INI_LOAD_OVERWRITE,imageBlocks);
    require(stable->getFilename()=="size-root.tga","normalized repeated separators and nonrecursive selection");
    ini.loadDirectoryBlocks("Data/INI/MappedImages/TextureSize_64/",TRUE,INI_LOAD_OVERWRITE,imageBlocks);
    require(stable->getFilename()=="size-child.tga","selected recursive loader preserves root-before-child order");
}
void imagesNegative() {
    ImageContext owner;owner.parse();
    {
        INI ini;
        const auto accepted=owner.images.snapshot();ImageResources resources;
        const INIBlockDefinition duplicates[]{{"MappedImage",INI::parseMappedImageDefinition},
            {"MappedImage",INI::parseMappedImageDefinition}};
        rejects([&]{ini.loadDirectoryBlocks("missing-generated-directory",TRUE,INI_LOAD_OVERWRITE,duplicates);});
        rejects([&]{ini.loadDirectoryBlocks("",TRUE,INI_LOAD_OVERWRITE,imageBlocks);});
        rejects([&]{owner.images.addImage(nullptr);});
        owner.images.unchanged(accepted);resources.unchanged();
    }
    for (const char* bad:{
        "MappedImage Stable\nTexture = bad.tga\nUnknown = late\nEnd\n",
        "MappedImage ColdRejected\nTexture = bad.tga\nUnknown = late\nEnd\n",
        "MappedImage Stable\nCoords = Left:-2147483648 Top:0 Right:0 Bottom:1\nEnd\n",
        "MappedImage Stable\nCoords = Left:0 Top:-2147483648 Right:1 Bottom:0\nEnd\n",
        "MappedImage Stable\nCoords = Left:2147483647 Top:0 Right:-2 Bottom:1\nEnd\n",
        "MappedImage\nEnd\n"}) {
        owner.input(bad);const auto accepted=owner.images.snapshot();const auto next=owner.nextKey();
        ImageResources resources;
        for (int repeat=0;repeat<3;++repeat) {
            rejects([&]{owner.parse();});owner.images.unchanged(accepted);
            require(owner.nextKey()==next,"semantic image rejection restores complete namespace ordinal");
            resources.unchanged();
        }
    }
    owner.input("MappedImage Stable\nCoords = Left:-2147483648 Top:2147483647 Right:-1 Bottom:-1\nEnd\n");
    owner.parse();const auto* stable=owner.images.findImageByName("Stable");
    require(stable->getImageWidth()==2147483647 && stable->getImageHeight()==std::numeric_limits<Int>::min(),
        "exact signed coordinate-difference boundaries admitted without overflow");
    owner.input(acceptedImage);owner.parse();
    for (const char* last:{"MappedImage Stable\nTexture = bad.tga\nUnknown = late\nEnd\n",
        "MappedImage Stable\nTexture = bad.tga\nEnd\nLanguage\nEnd\n"}) {
        owner.collectionInput(last);const auto accepted=owner.images.snapshot();const auto next=owner.nextKey();
        ImageResources resources;
        for (int repeat=0;repeat<3;++repeat) {
            rejects([&]{owner.images.load(64);});owner.images.unchanged(accepted);
            require(TheMappedImageCollection==&owner.images && owner.nextKey()==next,
                "late second-file rejection restores borrowed collection and namespace");resources.unchanged();
        }
    }
    owner.collectionInput();owner.images.load(64);
    require(owner.images.findImageByName("Stable")==stable && owner.images.findImageByName("NewImage"),
        "corrected collection retry publishes full graph with stable prior identity");
    int borrowed=77;
    auto* raw=newInstance(Image);MemoryPoolObjectHolder rawOwner(raw);
    raw->setName("RawGenerated");raw->setRawTextureData(&borrowed);raw->setStatus(IMAGE_STATUS_RAW_TEXTURE);
    owner.images.addImage(raw);rawOwner.release();
    owner.input("MappedImage RawGenerated\nTexture = invalid.tga\nEnd\n");
    const auto accepted=owner.images.snapshot();ImageResources resources;
    rejects([&]{owner.parse();});owner.images.unchanged(accepted);resources.unchanged();
    auto* duplicate=newInstance(Image);MemoryPoolObjectHolder duplicateOwner(duplicate);
    duplicate->setName("sTaBlE");
    rejects([&]{owner.images.addImage(duplicate);});owner.images.unchanged(accepted);
    owner.collectionInput();owner.images.load(64);
    require(owner.images.findImageByName("RawGenerated")==raw && raw->getRawTextureData()==&borrowed && borrowed==77,
        "untouched borrowed raw texture survives cloned graph commit without transfer or release");
}
enum class ImageOperation {New,Replace,Collection};
void imagesFaults(ImageOperation operation) {
    auto prepare=[&](ImageContext& owner) {
        owner.parse();
        if (operation==ImageOperation::New) owner.input("MappedImage ColdGenerated\nTexture = cold.tga\nEnd\n");
        else if (operation==ImageOperation::Replace) owner.input(candidateImage);
        else owner.collectionInput();
    };
    auto execute=[&](ImageContext& owner) {
        if (operation==ImageOperation::Collection) owner.images.load(64);else owner.parse();
    };
    std::size_t census;
    {ImageContext owner;prepare(owner);
        AllocationFault::arm(std::numeric_limits<std::size_t>::max());
        execute(owner);census=AllocationFault::attempts();AllocationFault::disarm();}
    require(census>0 && census<1024,"bounded complete mapped-image allocation manifest");
    constexpr std::array<std::size_t,3> manifest{13,10,54};
    require(census==manifest[static_cast<std::size_t>(operation)],
        "current complete image allocation census matches registered independent-family manifest");
    for (std::size_t ordinal=0;ordinal<=census;++ordinal) {
        ImageContext owner;prepare(owner);
        const auto accepted=owner.images.snapshot();const auto next=owner.nextKey();ImageResources resources;
        const auto* stable=owner.images.findImageByName("Stable");
        bool failed=false;AllocationFault::arm(ordinal);
        try{execute(owner);}catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}
        const auto attempts=AllocationFault::attempts();const auto triggered=AllocationFault::triggered();
        AllocationFault::disarm();
        if (ordinal<census) {
            require(triggered && failed,"every mapped-image allocation ordinal rejects before publication");
            owner.images.unchanged(accepted);
            require(TheMappedImageCollection==&owner.images && owner.nextKey()==next,
                "allocation image rejection restores publication and namespace");resources.unchanged();
            execute(owner);
        } else require(!triggered && !failed && attempts==census,"mapped-image exact terminal succeeds");
        require(owner.images.findImageByName("Stable")==stable &&
            owner.images.getName()=="registered-image-owner","corrected image retry preserves accepted object/registry identity");
        if (operation==ImageOperation::New) require(owner.images.findImageByName("ColdGenerated"),"new-image corrected retry");
        else require(stable->getFilename()=="changed.tga","replacement/collection corrected retry payload");
        if (operation==ImageOperation::Collection)require(owner.images.findImageByName("NewImage"),"complete collection retry publishes new sibling");
    }
    std::cout<<"mapped-image operation "<<static_cast<int>(operation)<<" allocation terminal "<<census<<'\n';
}
void webpage() {
    Context context;
    context.put("web.ini","WebpageURL GeneratedMetadata\nURL = https://example.invalid/help\nEnd\n");
    context.put("bad-web.ini","WebpageURL GeneratedMetadata\nURL = file://read-only-help\nUnknown = bad\nEnd\n");
    context.put("missing-web.ini","WebpageURL\nURL = ignored\nEnd\n");
    context.files.mountReadOnly({context.assets.root.string()});
    const INIBlockDefinition selected[]{{"WebpageURL",INI::parseWebpageURLDefinition}};
    auto load=[&](const char* filename) {INI reader;reader.loadBlocks(filename,INI_LOAD_OVERWRITE,selected);};
    load("web.ini"); // Warm the real pooled file owner before fault baselines.
    const auto live=AllocationFault::live();const auto fds=descriptors();
    rejects([&]{load("bad-web.ini");});rejects([&]{load("missing-web.ini");});
    require(AllocationFault::live()==live&&descriptors()==fds,
            "excluded browser metadata admission retires complete parse backing");
    constexpr unsigned expectedTerminal=12;
    unsigned terminal=0;
    const auto pooled=TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount();
    for(unsigned ordinal=0;ordinal<=expectedTerminal;++ordinal) {
        bool failed=false;AllocationFault::arm(ordinal);
        try {load("web.ini");} catch(const std::bad_alloc&) {failed=true;}
        catch(...) {AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        require(AllocationFault::live()==live&&descriptors()==fds&&
                TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount()==pooled,
                "each actual metadata allocation retires backing and descriptors");
        if(!AllocationFault::triggered()) {require(!failed,"metadata terminal accepts");terminal=ordinal;break;}
        require(failed,"armed metadata allocation rejects");load("web.ini");
        require(AllocationFault::live()==live&&descriptors()==fds,"same owner corrected metadata retry");
    }
    std::cout<<"web metadata allocation terminal "<<terminal<<'\n';
    require(terminal==expectedTerminal,"complete calibrated metadata sweep and exact terminal");
    const AsciiString prior=getConfiguredGameTextLanguage();
    configureGameTextLanguage("German");
    require(GetRegistryLanguage()==AsciiString("German"),"all actual source language consumers share native configuration");
    rejects([]{configureGameTextLanguage("../invalid");});
    require(GetRegistryLanguage()==AsciiString("German"),"invalid language keeps accepted source identity");
    configureGameTextLanguage(prior);
}
void functional() {
    Context context;context.start();GlobalData* root=context.original;
    require(root->m_xResolution==640 && root->m_yResolution==480 && !root->m_useTrees &&
            root->m_defaultStartingCash.countMoney()==12345 && damage(*root)==1.25f,"original complete GameData dispatch");
    require(root->getPath_UserData()==AsciiString((context.storage.paths().data+"/").c_str()),"native XDG path and separator");
    require(root->m_doubleClickTimeMS==nativeDoubleClickMilliseconds(),"native input policy");
    CRC expected;std::ifstream image("/proc/self/exe",std::ios::binary);char bytes[65536];
    while(image){image.read(bytes,sizeof(bytes));expected.computeCRC(bytes,static_cast<Int>(image.gcount()));}
    UnsignedInt version=context.version.getVersionNumber();expected.computeCRC(&version,sizeof(version));
    for(const char* script:{"generated skirmish","generated multiplayer"})expected.computeCRC(script,std::strlen(script));
    require(root->m_exeCRC==expected.get(),"complete process/version/script CRC order");
    LineCensus lines;INI reader;
    reader.loadBlocks("accepted.ini",INI_LOAD_OVERWRITE,blocks,{&lines,LineCensus::line});
    CRC lineExpected;std::ifstream definitions(context.assets.root/"accepted.ini");std::string line;
    unsigned lineCount=0;
    while(std::getline(definitions,line)){++lineCount;lineExpected.computeCRC(line.data(),line.size());}
    require(lines.calls==lineCount+1 && lines.crc.get()==lineExpected.get(),"exact original stripped-line transfer ordering, including EOF");
    root->m_modDir="retained mod";root->m_vertexWaterAvailableMaps[3]="retained water";
    root->m_terrainObjectsLighting[TIME_OF_DAY_COUNT-1][MAX_GLOBAL_LIGHTS-1].lightPos={3,4,5};
    auto* priorBonus=root->m_weaponBonusSet;
    context.load("candidate.ini",INI_LOAD_CREATE_OVERRIDES);GlobalData* first=TheWritableGlobalData;
    require(first!=root && first->m_weaponBonusSet!=priorBonus && damage(*first)==1.75f && damage(*root)==1.25f,
            "deep independent bonus clone");
    require(first->m_modDir=="retained mod" && first->m_vertexWaterAvailableMaps[3]=="retained water" &&
            first->m_terrainObjectsLighting[TIME_OF_DAY_COUNT-1][MAX_GLOBAL_LIGHTS-1].lightPos.z==5,
            "complete strings and nested tail configuration copy");
    context.load("accepted.ini",INI_LOAD_CREATE_OVERRIDES);
    require(TheWritableGlobalData!=first && damage(*TheWritableGlobalData)==1.25f && damage(*first)==1.75f,"multiple override ownership units");
    root->reset();require(TheWritableGlobalData==root && damage(*root)==1.25f && root->getName()=="registered original","root identity/reset retained");
    context.preferences("UseAlternateMouse = yes\nScrollFactor = 90\nResolution = 1920 1080\nGamma = 75\n");
    context.load("candidate.ini",INI_LOAD_MULTIFILE);
    require(TheWritableGlobalData==root && root->getName()=="registered original" && root->m_xResolution==1920 &&
            root->m_yResolution==1080 && root->m_useAlternateMouse && root->m_keyboardScrollFactor==0.9f &&
            root->m_displayGamma==1.5f && damage(*root)==1.75f,"preferences apply offside and adoption preserves owner identity");
    // Default GUI preferences follow the current singleton, not a retired node.
    OptionPreferences live;
    context.load("accepted.ini",INI_LOAD_CREATE_OVERRIDES);context.original->reset();
    require(live.getTreesEnabled()==root->m_useTrees,"long-lived default preferences follow accepted graph");
    context.load("candidate.ini",INI_LOAD_CREATE_OVERRIDES); // Context withdraws singleton before teardown.
}
void negative() {
    Context context;context.start();auto* root=context.original;auto* bonus=root->m_weaponBonusSet;
    const auto live=AllocationFault::live();const auto fds=descriptors();
    for(int repeat=0;repeat<3;++repeat) {
        rejects([&]{context.load("bad.ini",INI_LOAD_CREATE_OVERRIDES);});
        rejects([&]{context.load("bad.ini",INI_LOAD_OVERWRITE);});
        require(TheWritableGlobalData==root && root->m_weaponBonusSet==bonus && root->m_xResolution==640 &&
                damage(*root)==1.25f && descriptors()==fds && AllocationFault::live()==live,"late rejection retains complete accepted graph");
    }
    const INIBlockDefinition invalid[]{{"GameData",GlobalData::parseGameDataDefinition},{"GameData",GlobalData::parseGameDataDefinition}};
    INI ini;rejects([&]{ini.loadBlocks("candidate.ini",INI_LOAD_OVERWRITE,invalid);});
    rejects([&]{ini.loadBlocks("candidate.ini",static_cast<INILoadType>(99),blocks);});
    LineCensus invalidTransfer;
    rejects([&]{ini.loadBlocks("candidate.ini",INI_LOAD_OVERWRITE,blocks,{&invalidTransfer,nullptr});});
    rejects([&]{ini.loadBlocks("candidate.ini",INI_LOAD_OVERWRITE,blocks,{nullptr,LineCensus::line});});
    require(invalidTransfer.calls==0 && TheWritableGlobalData==root,"invalid line-owner pairs reject before dispatch");
    // Every callback inside GameData's still-offside block, through its END
    // line, remains fallible. Whole-file/startup rollback is a separate gate.
    for(unsigned phase=0;phase<7;++phase) {
        LineCensus fault;fault.fail=phase;
        rejects([&]{ini.loadBlocks("candidate.ini",INI_LOAD_CREATE_OVERRIDES,blocks,{&fault,LineCensus::line});});
        require(TheWritableGlobalData==root && root->m_weaponBonusSet==bonus && damage(*root)==1.25f &&
                root->m_xResolution==640 && descriptors()==fds,"callback fault cannot publish a partial configuration block");
    }
    context.preferences("Resolution = 9999999999999999999 17\nGamma = 2147483647\nScrollFactor = -999\nFirewallPortOverride = 65536\n");
    context.load("candidate.ini",INI_LOAD_OVERWRITE);
    require(root->m_xResolution==1280 && root->m_yResolution==720 && root->m_keyboardScrollFactor==0 &&
            root->m_firewallPortOverride==0 && std::isfinite(root->m_displayGamma),"defined numeric boundaries and candidate defaults");
    const auto owned=TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount();
    require(owned==1,"one accepted bonus ownership unit");
}
void faults(INILoadType type,bool constructing) {
    Context context;context.start();context.preferences("UseAlternateMouse = yes\nResolution = 1600 900\nGamma = 25\n");
    // Warm process-global pool creation/high-water storage before exact baselines.
    context.load("candidate.ini",INI_LOAD_CREATE_OVERRIDES);context.original->reset();
    // Preferences discover real native IPv4 interfaces. The sandbox and normal
    // host expose different cardinalities; each source label acquires one unit.
    // Capture only the count, retire discovery, then freeze the exact manifest.
    std::size_t interfaceCount=0;
    if (!constructing) {
        IPEnumeration census;
        for (auto* node=census.getAddresses();node;node=node->getNext()) ++interfaceCount;
    }
    // Root ownership is now validated in the actual default constructor.
    // Count the complete read-only admission against these exact generated
    // paths; missing ancestor depth changes allocation demand, not coverage.
    std::size_t rootOwnershipAllocations=0;
    if(constructing) {
        AllocationFault::arm(SIZE_MAX);
        try {context.storage.validateRootOwnership();}
        catch(...) {AllocationFault::disarm();throw;}
        rootOwnershipAllocations=AllocationFault::attempts();
        AllocationFault::disarm();
    }
    const std::size_t expectedTerminal=constructing?22u+rootOwnershipAllocations:57u+interfaceCount;
    std::size_t terminal=0;
    for(std::size_t ordinal=0;ordinal<512;++ordinal) {
        const AsciiString filename("candidate.ini"); INI ini;
        // Establish a reusable unprepared reader baseline. A fresh reader owns
        // the constructor's "None" string, retired by normal unPrepFile.
        if (!constructing) ini.loadBlocks("accepted.ini",INI_LOAD_OVERWRITE,blocks);
        auto* root=context.original;auto* bonus=root->m_weaponBonusSet;
        const auto live=AllocationFault::live();const auto fds=descriptors();
        const auto pooled=TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount();
        auto* interfacePool=TheMemoryPoolFactory->findMemoryPool("EnumeratedIP");
        const auto interfacesBefore=interfacePool?interfacePool->getUsedBlockCount():0;
        const auto preparing=AllocationFault::live();bool failed=false;
        AllocationFault::arm(ordinal);
        try {
            if(constructing){std::unique_ptr<GlobalData> candidate(new GlobalData);}
            else ini.loadBlocks(filename,type,blocks);
        } catch(const std::bad_alloc&){failed=true;}
          catch(...){AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        require(descriptors()==fds,"all fault ordinals retire descriptors");
        require(!interfacePool || interfacePool->getUsedBlockCount()==interfacesBefore,
                "discovery backing retires after every configuration attempt");
        if(!AllocationFault::triggered()){require(!failed,"untriggered terminal accepts");terminal=ordinal;break;}
        if (!(failed && AllocationFault::live()==preparing && TheWritableGlobalData==root &&
              root->m_weaponBonusSet==bonus && damage(*root)==1.25f &&
              TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount()==pooled))
            std::cerr<<"ordinal "<<ordinal<<" standard "<<AllocationFault::live()<<" expected "<<preparing
                     <<" pooled "<<TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount()
                     <<" expected "<<pooled<<" link "<<(TheWritableGlobalData==root)<<'\n';
        require(failed && AllocationFault::live()==preparing && TheWritableGlobalData==root &&
                root->m_weaponBonusSet==bonus && damage(*root)==1.25f &&
                TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount()==pooled,
                "every allocation fault retains graph and exact resources");
        if(constructing){std::unique_ptr<GlobalData> corrected(new GlobalData);require(corrected->m_weaponBonusSet!=bonus,"constructor retry independent");}
        else {
            ini.loadBlocks(filename,type,blocks);
            require(TheWritableGlobalData->m_xResolution==1600 && damage(*TheWritableGlobalData)==1.75f,"same-owner corrected retry");
            context.original->reset();context.load("accepted.ini",INI_LOAD_OVERWRITE);
            context.original->m_mapName.clear(); // Restore the complete fault candidate's initial state.
        }
        (void)live;
    }
    std::cout << "configuration allocations [0," << terminal << "); terminal " << terminal << '\n';
    std::cout << "native interface count " << interfaceCount << "; expected " << expectedTerminal << '\n';
    require(terminal==expectedTerminal,"complete cardinality-calibrated allocation manifest and exact terminal");
}
void platform() {
    SDL_ResetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME);
    require(nativeDoubleClickMilliseconds()==500,"native default policy");
    for(const char* value:{"0","300","2147483647"}) {
        require(SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME,value),"public SDL hint");
        require(nativeDoubleClickMilliseconds()==static_cast<UnsignedInt>(std::strtoul(value,nullptr,10)),"native configured click time");
    }
    for(const char* value:{"-1","junk","2147483648","99999999999999999999"}) {
        SDL_SetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME,value);require(nativeDoubleClickMilliseconds()==500,"invalid native policy defaults defined");
    }
    SDL_ResetHint(SDL_HINT_MOUSE_DOUBLE_CLICK_TIME);SDL_Quit();
    IPEnumeration interfaces;const auto* head=interfaces.getAddresses();
    UnsignedInt previous=0;for(auto* entry=head;entry;entry=entry->getNext()) {
        require(entry->getIP()>=previous,"native interfaces source sorted");previous=entry->getIP();
    }
    require(interfaces.getAddresses()==head,"accepted interface chain cached");
    require(!interfaces.getMachineName().isEmpty(),"bounded native host identity");
}
void interfaces() {
    const UnsignedInt accepted[]{0x01020304u,0x7f000001u,0xffffffffu};
    const UnsignedInt proposed[]{0xffffffffu,0u,0x01020304u,0x01020304u,0x7f000001u};
    // Warm every retained pool before counting transaction-local ownership.
    {IPEnumeration warm;warm.replaceAddresses(proposed);}
    std::size_t terminal=0;
    for(std::size_t ordinal=0;ordinal<128;++ordinal) {
        IPEnumeration owner;owner.replaceAddresses(accepted);auto* prior=owner.getAddresses();
        auto* pool=TheMemoryPoolFactory->findMemoryPool("EnumeratedIP");
        const auto pooled=pool->getUsedBlockCount();const auto live=AllocationFault::live();bool failed=false;
        AllocationFault::arm(ordinal);
        try{owner.replaceAddresses(proposed);}catch(const std::bad_alloc&){failed=true;}
          catch(...){AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        if(!AllocationFault::triggered()){require(!failed,"interface terminal accepts");terminal=ordinal;break;}
        require(failed && owner.getAddresses()==prior && pool->getUsedBlockCount()==pooled &&
                AllocationFault::live()==live,"late interface replacement fault retains complete owned chain");
        owner.replaceAddresses(proposed);
        const UnsignedInt sorted[]{0u,0x01020304u,0x01020304u,0x7f000001u,0xffffffffu};
        auto* node=owner.getAddresses();
        for(UnsignedInt value:sorted){require(node && node->getIP()==value,"corrected exact sorted snapshot");node=node->getNext();}
        require(!node && pool->getUsedBlockCount()==5,"duplicate numerical addresses retain distinct ownership units");
    }
    require(terminal==5,"interface complete manifest and terminal");
    std::cout<<"interface allocations [0,"<<terminal<<"); terminal "<<terminal<<'\n';
}
void values() {
    Context context;
    const char* const names[]{"B0","B1","B2","B3","B4","B5","B6","B7","B8","B9",
        "B10","B11","B12","B13","B14","B15","B16","B17","B18","B19","B20",
        "B21","B22","B23","B24","B25","B26","B27","B28","B29","B30","B31","B32",nullptr};
    struct Values {UnsignedInt mask=7;Byte byte=3;UnsignedShort duration=17;} candidate;
    const FieldParse fields[]{{"Mask",INI::parseBitString32,names,offsetof(Values,mask)},
        {"ByteMask",INI::parseBitString8,names,offsetof(Values,byte)},
        {"Duration",INI::parseDurationUnsignedShort,nullptr,offsetof(Values,duration)},
        {nullptr,nullptr,nullptr,0}};
    INI ini;ini.loadFields("values.ini",INI_LOAD_OVERWRITE,&candidate,fields);
    require(candidate.mask==0x80000000u && static_cast<UnsignedByte>(candidate.byte)==131 && candidate.duration==65535,
            "exact maximum bit/index/millisecond-to-frame boundaries");
    const auto before=candidate;
    for(const char* file:{"mixed.ini","wide-mask.ini","wide-byte.ini","wide-duration.ini"}) {
        rejects([&]{ini.loadFields(file,INI_LOAD_OVERWRITE,&candidate,fields);});
        require(candidate.mask==before.mask && candidate.byte==before.byte && candidate.duration==before.duration,
                "late-invalid bit expression and unrepresentable duration cannot publish field prefixes");
    }
    ini.loadFields("missing-values.ini",INI_LOAD_OVERWRITE,&candidate,fields);
    require(candidate.byte==before.byte,"empty byte expression preserves initialized prior mask");
    ini.loadFields("remove-byte.ini",INI_LOAD_OVERWRITE,&candidate,fields);
    require(static_cast<UnsignedByte>(candidate.byte)==3,"high-bit byte prior mask is not sign extended during removal");
    ini.loadFields("values.ini",INI_LOAD_OVERWRITE,&candidate,fields);
    require(candidate.mask==0x80000000u && candidate.duration==65535,"same-owner corrected primitive retry");
}
void bootstrap() {
    Context context;
    struct Registry {
        SubsystemInterfaceList list;
        SubsystemInterfaceList* prior=TheSubsystemList;
        Registry(){TheSubsystemList=&list;}
        ~Registry(){list.shutdownAll();TheSubsystemList=prior;}
    } registry;
    auto initialize=[&](const char* filename) {
        initOwnedSubsystem(TheWritableGlobalData,"generated GameData root",new GlobalData,
            [&](GlobalData*) {
                INI ini;ini.loadBlocks(filename,INI_LOAD_OVERWRITE,blocks);
            });
    };
    // Keep the list's accepted vector capacity; it is registry-owned backing,
    // not a transaction leak or a reason to force shrinking during shutdown.
    initialize("accepted.ini");registry.list.shutdownAll();
    const auto live=AllocationFault::live();const auto fds=descriptors();
    const auto pooled=TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount();
    for(unsigned repeat=0;repeat<3;++repeat) {
        rejects([&]{initialize("bad.ini");});
        require(!TheWritableGlobalData && registry.list.ownedCount()==0 &&
                AllocationFault::live()==live && descriptors()==fds &&
                TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount()==pooled,
                "actual initialized GameData owner/decode rejection retires registration and backing");
    }
    initialize("accepted.ini");
    auto* root=TheWritableGlobalData;
    require(root && registry.list.ownedCount()==1 && root->m_xResolution==640 &&
            root->m_defaultStartingCash.countMoney()==12345 && damage(*root)==1.25f &&
            root->getName()==AsciiString("generated GameData root"),
            "same-list actual GlobalData init/definition/adoption retry");
    const auto acceptedLive=AllocationFault::live();
    rejects([&]{initialize("candidate.ini");});
    require(TheWritableGlobalData==root && registry.list.ownedCount()==1 &&
            root->m_xResolution==640 && damage(*root)==1.25f &&
            AllocationFault::live()==acceptedLive,
            "parallel root admission restores the accepted registry owner after candidate cleanup");
    context.load("candidate.ini",INI_LOAD_CREATE_OVERRIDES);
    require(TheWritableGlobalData!=root && damage(*TheWritableGlobalData)==1.75f,
            "registered root owns a separately published override head");
    registry.list.resetAll();
    require(TheWritableGlobalData==root && registry.list.ownedCount()==1,
            "actual reverse-reset restores registered original identity");
    context.load("candidate.ini",INI_LOAD_CREATE_OVERRIDES);
    registry.list.shutdownAll();
    require(!TheWritableGlobalData && registry.list.ownedCount()==0 &&
            AllocationFault::live()==live && descriptors()==fds &&
            TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount()==pooled,
            "actual registry shutdown retires root/override ownership and borrowed publication");
    initialize("accepted.ini");registry.list.shutdownAll();
    require(!TheWritableGlobalData && AllocationFault::live()==live,
            "same-registry real owner restarts after complete shutdown");
}
}
int main(int argc,char** argv) {
    bool initialized=false;
    try {
        require(argc==2,"configuration family");initMemoryManager();initialized=true;
        const std::string family(argv[1]);
        // Explicitly initialize source process-global pool services before baselines.
        {Context warm;warm.start();warm.load("candidate.ini",INI_LOAD_CREATE_OVERRIDES);}
        if (family.starts_with("images")) {
            ImageContext warm;warm.parse();warm.collectionInput();warm.images.load(64);
            warm.input("MappedImage ColdGenerated\nTexture = cold.tga\nEnd\n");warm.parse();
        }
        if (family=="interfaces") {
            IPEnumeration warm;const UnsignedInt addresses[]{0u,1u,2u,3u,0xffffffffu};
            warm.replaceAddresses(addresses);
        }
        for(int repeat=0;repeat<3;++repeat) {
            const auto baseline=AllocationFault::live();
            if(family=="functional")functional();else if(family=="negative")negative();
            else if(family=="fault-override")faults(INI_LOAD_CREATE_OVERRIDES,false);
            else if(family=="fault-overwrite")faults(INI_LOAD_OVERWRITE,false);
            else if(family=="fault-constructor")faults(INI_LOAD_OVERWRITE,true);
            else if(family=="platform")platform();else if(family=="interfaces")interfaces();
            else if(family=="values")values();
            else if(family=="bootstrap")bootstrap();
            else if(family=="webpage")webpage();
            else if(family=="locale")locale();
            else if(family=="locale-faults")localeFaults();
            else if(family=="images")imagesFunctional();
            else if(family=="images-negative")imagesNegative();
            else if(family=="images-fault-new")imagesFaults(ImageOperation::New);
            else if(family=="images-fault-replace")imagesFaults(ImageOperation::Replace);
            else if(family=="images-fault-collection")imagesFaults(ImageOperation::Collection);
            else throw std::runtime_error("unknown family");
            require(!TheWritableGlobalData && AllocationFault::live()==baseline,"complete same-process configuration teardown");
            require(TheMemoryPoolFactory->findMemoryPool("WeaponBonusSet")->getUsedBlockCount()==0,"all bonus refs retire");
        }
        shutdownMemoryManager();initialized=false;std::cout<<"PASS original configuration owners (GameEngine pending)\n";return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
     catch(ErrorCode){std::cerr<<"FAIL source error\n";}
    AllocationFault::disarm();SDL_Quit();if(initialized)shutdownMemoryManager();return 1;
}
