// SPDX-License-Identifier: GPL-3.0-or-later
#include "Common/FileSystem.h"
#include "Common/NativeDataFile.h"
#include "Common/RAMFile.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/FileOwner.h"
#include "AllocationFault.h"
#include "GameClient/CSF.h"
#include "GameClient/GameText.h"
#include "GameClient/LanguageFilter.h"
#include <array>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <dirent.h>

namespace {
struct Failure:std::runtime_error{using std::runtime_error::runtime_error;};
void require(bool value,const char* message){if(!value)throw Failure(message);}
template<class F> void rejects(F action,const char* message){bool failed=false;try{action();}catch(ErrorCode){failed=true;}catch(const std::exception&){failed=true;}require(failed,message);}
struct Tree {
    std::filesystem::path path;
    Tree(){std::array<char,32> pattern{};std::strcpy(pattern.data(),"/tmp/zh-native-data-XXXXXX");const auto* p=::mkdtemp(pattern.data());if(!p)throw std::runtime_error("generated temporary root");path=p;}
    ~Tree(){std::error_code ignored;std::filesystem::remove_all(path,ignored);}
    void write(const std::string& name,const std::string& bytes) const {
        const auto file=path/name;std::filesystem::create_directories(file.parent_path());
        std::ofstream out(file,std::ios::binary|std::ios::trunc);out.write(bytes.data(),std::streamsize(bytes.size()));if(!out)throw std::runtime_error("generated fixture write");
    }
    std::string root()const{return path.string();}
};
void word(std::string& output,std::uint32_t value){for(int shift:{24,16,8,0})output+=char(value>>shift);}
void little(std::string& output,std::uint32_t value){for(int shift:{0,8,16,24})output+=char(value>>shift);}
std::string csf() {
    std::string result;for(auto value:{0x43534620u,3u,1u,2u,0u,6u})little(result,value);
    little(result,0x4c424c20u);little(result,2);little(result,5);result+="LABEL";
    little(result,0x53545257u);
    const std::vector<std::uint16_t> text{u' ',u' ',u'Ω',u' ',u' ',0xd83d,0xde00,u' ',u'\n',u' ',u' ',u'B',u' '};
    little(result,std::uint32_t(text.size()));
    for(auto unit:text){const std::uint16_t encoded=std::uint16_t(~unit);result+=char(encoded);result+=char(encoded>>8);}
    little(result,4);result+="wave";
    little(result,0x53545220u);little(result,1);const auto encoded=std::uint16_t(~u'X');result+=char(encoded);result+=char(encoded>>8);
    return result;
}
std::string big(const std::vector<std::pair<std::string,std::string>>& entries) {
    std::uint32_t offset=16;for(const auto& [name,data]:entries){(void)data;offset+=std::uint32_t(9+name.size());}
    std::string result="BIGF";word(result,0);word(result,std::uint32_t(entries.size()));word(result,offset);
    for(const auto& [name,data]:entries){word(result,offset);word(result,std::uint32_t(data.size()));result+=name;result+=char(0);offset+=std::uint32_t(data.size());}
    for(const auto& [name,data]:entries){(void)name;result+=data;}return result;
}
std::string content(FileSystem& fs,const char* name) {
    File* file=fs.openFile(name);require(file,"generated file reachability");
    try{std::string result(std::size_t(file->size()),'\0');require(file->read(result.data(),Int(result.size()))==Int(result.size()),"exact generated read");file->close();return result;}
    catch(...){file->close();throw;}
}
void roots() {
    Tree zh,base;zh.write("Data/INI/Loose.ini","loose-zh");base.write("Data/INI/Loose.ini","loose-base");
    zh.write("a.big",big({{"Data\\INI\\Archive.ini","first"},{"Data/INI/Loose.ini","archived"}}));
    zh.write("b.big",big({{"Data/INI/Archive.ini","later"}}));base.write("base.big",big({{"Data/INI/Archive.ini","base"}}));
    zh.write("INIZH.big",big({{"Data/INI/Patched.ini","current"}}));
    zh.write("Data/INI/INIZH.big",big({{"Data/INI/Patched.ini","obsolete"}}));
    FileSystem fs;fs.mountReadOnly({zh.root(),base.root()});
    require(content(fs,"data\\ini\\LOOSE.INI")=="loose-zh","loose first and root priority");
    require(content(fs,"DATA/INI/ARCHIVE.INI")=="first","archive ordinal and root priority");
    require(content(fs,"Data/INI/Patched.ini")=="current","patch-era duplicate is excluded from archive selection without deletion");
    require(zh.path.empty()==false&&std::filesystem::exists(zh.path/"Data/INI/INIZH.big"),"obsolete supplied archive bytes remain in place");
    FilenameList list;fs.getFileListInDirectory("Data/INI","*.ini",list,FALSE);require(list.size()==3,"merged unique filename census");
    FileInfo info{};require(fs.getFileInfo("Data/INI/Archive.ini",&info)&&info.sizeLow==5,"source FileInfo width");
    File* retained=fs.openFile("Data/INI/Archive.ini");require(retained,"retained archive view");
    fs.mountReadOnly({base.root()});std::array<char,5> bytes{};require(retained->read(bytes.data(),5)==5&&std::string(bytes.data(),5)=="first","view survives registry replacement");retained->close();
    require(content(fs,"Data/INI/Archive.ini")=="base","replacement publication");
    rejects([&]{fs.mountReadOnly({""});},"empty root admission");require(content(fs,"Data/INI/Archive.ini")=="base","rejected replacement keeps accepted graph");
    for(const char* path:{"../escape","/absolute","a/../escape","a//escape","C:\\escape"})rejects([&]{fs.openFile(path);},"path admission");
    for(Int flag:std::array<Int,6>{File::WRITE,File::APPEND,File::TRUNCATE,File::CREATE,File::ONLYNEW,Int(0x200)})rejects([&]{fs.openFile("Data/INI/Archive.ini",flag);},"read-only access admission");
    require(!fs.createDirectory("created"),"asset directory writes unavailable");
    Tree collision;collision.write("same","a");collision.write("SAME","b");
    rejects([&]{fs.mountReadOnly({collision.root()});},"ambiguous physical case collision");
    require(content(fs,"Data/INI/Archive.ini")=="base","physical collision rolls back");
}
void archives() {
    Tree stable,candidate;stable.write("stable.big",big({{"accepted","accepted"}}));
    const auto complete=big({{"one","first"},{"nested/two","second"}});
    FileSystem fs;fs.mountReadOnly({stable.root()});
    for(std::size_t prefix=0;prefix<complete.size();++prefix){candidate.write("candidate.big",complete.substr(0,prefix));rejects([&]{fs.mountReadOnly({candidate.root()});},"every truncated prefix rejected");require(content(fs,"accepted")=="accepted","truncated candidate preserves owner");}
    for(const auto& invalid:{big({{"../outside","bad"}}),big({{"same","a"},{"SAME","b"}}),big({{"/absolute","bad"}}),big({{"","bad"}})}) {
        candidate.write("candidate.big",invalid);rejects([&]{fs.mountReadOnly({candidate.root()});},"archive name admission");require(content(fs,"accepted")=="accepted","name rejection preserves owner");
    }
    auto overflow=complete;for(std::size_t index=16;index<20;++index)overflow[index]=char(0xff);
    candidate.write("candidate.big",overflow);rejects([&]{fs.mountReadOnly({candidate.root()});},"archive offset range");
    candidate.write("candidate.big",complete);fs.mountReadOnly({candidate.root()});
    require(content(fs,"one")=="first"&&content(fs,"nested/two")=="second","same-owner corrected archive retry");
    File* first=fs.openFile("one"),*second=fs.openFile("one");char a=0,b=0;
    require(first->read(&a,1)==1&&second->read(&b,1)==1&&a==b,"independent cursors shared descriptor");first->close();second->close();
    for(std::uint32_t sentinel:{0u,15u,32u}) {
        auto empty=big({{"empty",""},{"value","payload"}});
        for(unsigned index=0;index<4;++index)empty[16+index]=char(sentinel>>(24-8*index));
        candidate.write("candidate.big",empty);fs.mountReadOnly({candidate.root()});
        require(content(fs,"empty").empty()&&content(fs,"value")=="payload","zero-length archive range has no payload boundary");
        auto late=empty;late[30]=char(0xff);late[31]=char(0xff);late[32]=char(0xff);late[33]=char(0xff);
        candidate.write("candidate.big",late);rejects([&]{fs.mountReadOnly({candidate.root()});},"late nonempty range remains bounded after empty entry");
        require(content(fs,"value")=="payload","empty/late-invalid archive preserves accepted graph");
    }
}
class ThrowingReadFile : public File {
    MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(ThrowingReadFile,"GeneratedThrowingReadFile",4,4)
    Int cursor=0;
public:
    bool reject=true;
    ThrowingReadFile(){File::open("generated",READ|BINARY);}
    Int size() override{return 4;}
    Int read(void* output,Int count) override {
        if(reject){++cursor;throw std::runtime_error("generated read failure");}
        count=std::min(count,4-cursor);if(output&&count)std::memcpy(output,"data"+cursor,std::size_t(count));cursor+=count;return count;
    }
    Int write(const void*,Int) override{return -1;}
    Int seek(Int offset,seekMode mode=CURRENT) override {
        Int64 next=offset;switch(mode){case START:break;case CURRENT:next+=cursor;break;case END:next+=4;break;default:return -1;}
        cursor=Int(std::clamp<Int64>(next,0,4));return cursor;
    }
    Bool scanInt(Int& v) override{return File::scanInt(v);}
    Bool scanReal(Real& v) override{return File::scanReal(v);}
    Bool scanString(AsciiString& v) override{return File::scanString(v);}
    void nextLine(Char* v,Int n) override{File::nextLine(v,n);}
    char* readEntireAndClose() override{throw ERROR_BAD_ARG;}
    File* convertToRAMFile() override{return this;}
};
ThrowingReadFile::~ThrowingReadFile()=default;
void ram() {
    Tree tree;tree.write("data","skip=-17 .5 token\nnext\n");tree.write("empty","");tree.write("bad","2147483648");
    FileSystem fs;fs.mountReadOnly({tree.root()});TheFileSystem=&fs;
    try {
        File* native=fs.openFile("data");File* snapshot=native->convertToRAMFile();native=nullptr;
        require(dynamic_cast<RAMFile*>(snapshot),"original RAMFile consumer");MemoryPoolObjectHolder hold(snapshot);
        Int number=0;Real real=0;AsciiString token;
        require(snapshot->scanInt(number)&&number==-17,"original skip-to-numeric grammar");
        require(snapshot->scanReal(real)&&real==.5f,"original decimal grammar");
        require(snapshot->scanString(token)&&token=="token","original token grammar");
        std::array<char,4> line{'x','x','x','Z'};snapshot->nextLine(line.data(),3);require(line[3]=='Z',"line guard remains untouched");
        const Int prior=snapshot->position();require(snapshot->read(nullptr,-1)==-1&&snapshot->position()==prior,"negative read has no cursor effects");
        require(snapshot->seek(INT32_MAX,File::CURRENT)==snapshot->size(),"widened seek addition clamps");
        auto* bad=newInstance(RAMFile);MemoryPoolObjectHolder badOwner(bad);require(bad->open("bad"),"bad numeric source reachability");
        number=23;require(!bad->scanInt(number)&&number==23&&bad->position()==0,"numeric overflow rolls back cursor and output");
        auto* empty=newInstance(RAMFile);MemoryPoolObjectHolder emptyOwner(empty);require(empty->open("empty")&&empty->size()==0,"empty original RAM backing");
        require(empty->read(nullptr,1)==0,"empty read");
        static_assert(sizeof(File::seekMode)==sizeof(Int)&&alignof(File::seekMode)==alignof(Int));
        for(File* provider:{snapshot,static_cast<File*>(empty)}) {
            const Int cursor=provider->position();require(provider->seek(7,static_cast<File::seekMode>(3))==-1&&provider->position()==cursor,"defined raw seek rejection without effects");
        }
        auto* throwing=newInstance(ThrowingReadFile);MemoryPoolObjectHolder borrowed(throwing);
        for(bool archive:{false,true}) {
            auto* candidate=newInstance(RAMFile);MemoryPoolObjectHolder owner(candidate);throwing->seek(2,File::START);throwing->reject=true;
            rejects([&]{if(archive)candidate->openFromArchive(throwing,"generated",0,4);else candidate->open(throwing);},"borrowed read throws during candidate construction");
            require(throwing->position()==2&&candidate->read(nullptr,0)==-1,"read throw restores borrowed cursor and unpublished owner");
            throwing->reject=false;require(archive?candidate->openFromArchive(throwing,"generated",0,4):candidate->open(throwing),"same RAM owner retry after throwing read");
        }
    }catch(...){TheFileSystem=nullptr;throw;}
    TheFileSystem=nullptr;
}
void catalogs() {
    Tree tree;const auto complete=csf();tree.write("catalog.csf",complete);
    FileSystem fs;fs.mountReadOnly({tree.root()});File* input=fs.openFile("catalog.csf");
    try {
        const auto accepted=decodeOriginalCSF(*input);
        require(accepted.version==3&&accepted.language==6&&accepted.records.size()==1,"fixed CSF header and count");
        require(accepted.records[0].label=="LABEL"&&accepted.records[0].speech=="wave","original first-string label/speech ownership");
        require(accepted.records[0].text.compare(L"Ω 😀\nB")==0,"inverted UTF16, surrogate pair and source whitespace rules");
        std::vector<std::string> invalidCatalogs;
        for(std::size_t offset:std::array<std::size_t,6>{4,8,12,20,28,complete.size()-10}) {
            auto bad=complete;for(std::size_t byte=0;byte<4;++byte)bad[offset+byte]=char(0xff);invalidCatalogs.push_back(std::move(bad));
        }
        auto surrogate=complete;const auto lone=std::uint16_t(~0xdc00u);surrogate[49]=char(lone);surrogate[50]=char(lone>>8);invalidCatalogs.push_back(surrogate);
        invalidCatalogs.push_back(complete+"extra");
        input->close();input=nullptr;
        for(const auto& bad:invalidCatalogs) {
            tree.write("catalog.csf",bad);fs.mountReadOnly({tree.root()});FileCloseOwner rejected(fs.openFile("catalog.csf"));
            rejected->seek(3,File::START);rejects([&]{(void)decodeOriginalCSF(*rejected);},"CSF encoded subfields, alternatives and Unicode admission");
            require(rejected->position()==3,"CSF late invalid input restores nonzero borrowed cursor");
        }
        tree.write("catalog.csf",complete);fs.mountReadOnly({tree.root()});input=fs.openFile("catalog.csf");
        (void)decodeOriginalCSF(*input);
        require(input->position()==input->size(),"complete catalog consumes declared file");input->close();input=nullptr;
        for(std::size_t prefix=0;prefix<complete.size();++prefix) {
            tree.write("catalog.csf",complete.substr(0,prefix));fs.mountReadOnly({tree.root()});input=fs.openFile("catalog.csf");
            rejects([&]{(void)decodeOriginalCSF(*input);},"every truncated CSF prefix rejected");
            require(input->position()==0,"CSF rejection restores borrowed cursor");input->close();input=nullptr;
        }
        auto invalid=complete;invalid[0]=0;tree.write("catalog.csf",invalid);fs.mountReadOnly({tree.root()});input=fs.openFile("catalog.csf");
        rejects([&]{(void)decodeOriginalCSF(*input);},"CSF magic admission");require(input->position()==0,"bad catalog cursor rollback");input->close();input=nullptr;
        tree.write("catalog.csf",complete);fs.mountReadOnly({tree.root()});input=fs.openFile("catalog.csf");
        require(decodeOriginalCSF(*input).records[0].text==accepted.records[0].text,"same-owner corrected catalog retry");input->close();input=nullptr;
    }catch(...){if(input)input->close();throw;}
}
void textManager() {
    Tree tree;const auto complete=csf();tree.write("Data/English/Generals.csf",complete);
    FileSystem fs;fs.mountReadOnly({tree.root()});TheFileSystem=&fs;
    try {
        std::unique_ptr<GameTextInterface> text(CreateGameTextInterface());text->init();Bool exists=FALSE;
        require(text->fetch("label",&exists).compare(L"Ω 😀\nB")==0&&exists,"actual GameText init and lookup");
        require(text->getStringsWithLabelPrefix("LAB").size()==1,"actual text prefix lookup");
        require(text->getStringsWithLabelPrefix("lab").empty(),"original prefix selection is case-sensitive");
        for(int cycle=0;cycle<3;++cycle) {
            tree.write("Data/English/Generals.csf",complete.substr(0,complete.size()-1));fs.mountReadOnly({tree.root()});
            rejects([&]{text->loadCSF("Data/English/Generals.csf");},"actual text replacement rejects malformed candidate");
            require(text->fetch("LABEL",&exists).compare(L"Ω 😀\nB")==0&&exists,"actual text links survive rejected replacement");
            tree.write("Data/English/Generals.csf",complete);fs.mountReadOnly({tree.root()});
            require(text->loadCSF("Data/English/Generals.csf"),"actual text same-owner retry");
        }
        require(!text->fetch("missing",&exists).isEmpty()&&!exists,"original missing-label behavior");
        rejects([]{configureGameTextLanguage("../bad");},"native language directory admission");
        require(getConfiguredGameTextLanguage()=="English","rejected language retains selection");
        std::string words;for(char c:std::string("bad")){const auto unit=std::uint16_t(std::uint16_t(c)^0x5555);words+=char(unit);words+=char(unit>>8);}words+=char(0x20);words+=char(0);
        tree.write("langdata.dat",words);fs.mountReadOnly({tree.root()});LanguageFilter filter;filter.init();
        UnicodeString sentence(L"a bad word");filter.filterLine(sentence);require(sentence.compare(L"a *** word")==0,"original XOR word/filter semantics");
        tree.write("langdata.dat",words.substr(0,words.size()-1));fs.mountReadOnly({tree.root()});rejects([&]{filter.init();},"partial UTF16 filter word rejected");
        sentence=L"bad";filter.filterLine(sentence);require(sentence.compare(L"***")==0,"filter rejection preserves accepted map");
        tree.write("langdata.dat",words);fs.mountReadOnly({tree.root()});filter.init();sentence=L"bad";filter.filterLine(sentence);require(sentence.compare(L"***")==0,"filter corrected retry");
        tree.write("map.str","MAP:Name\n\"a bad map\\n next\" = speech1\nEnd\n");fs.mountReadOnly({tree.root()});
        TheLanguageFilter=&filter;
        try {
            text->initMapStringFile("map.str");
            require(text->fetch("map:name",&exists).compare(L"a *** map\nnext")==0&&exists,"original map text/filter owner publication");
            tree.write("candidate.str","MAP:Name\n\"bad metadata\"\nEnd\nLABEL\n\"not the base\"\nEnd\n");
            fs.mountReadOnly({tree.root()});
            for(int repeat=0;repeat<3;++repeat) {
                require(text->fetchMapMetadataLabel("candidate.str","MAP:Name").compare(L"*** metadata")==0,
                    "metadata lookup uses temporary filtered catalog");
                require(text->fetchMapMetadataLabel("candidate.str","LABEL").compare(L"Ω 😀\nB")==0,
                    "metadata preserves base-before-map precedence");
                require(text->fetchMapMetadataLabel("missing.str","MAP:Name").compare(L"MISSING: 'MAP:Name'")==0,
                    "missing metadata companion cannot borrow active gameplay map");
                require(text->fetchMapMetadataLabel(AsciiString::TheEmptyString,"LABEL").compare(L"Ω 😀\nB")==0,
                    "official cached metadata uses base without physical file");
                require(text->fetch("MAP:Name").compare(L"a *** map\nnext")==0,
                    "metadata lookup preserves accepted gameplay catalog");
            }
            tree.write("candidate.str","MAP:Name\n\"unclosed\n");fs.mountReadOnly({tree.root()});
            rejects([&]{text->fetchMapMetadataLabel("candidate.str","MAP:Name");},"metadata malformed catalog rejection");
            require(text->fetch("MAP:Name").compare(L"a *** map\nnext")==0,"metadata rejection preserves gameplay links");
            for(const std::string bad:{"MAP:Name\n\"unclosed\n","MAP:Name\n\"first\"\n\"second\"\nEnd\n","MAP:Name\n\"first\"\nEnd\nmap:name\n\"duplicate\"\nEnd\n"}) {
                tree.write("map.str",bad);fs.mountReadOnly({tree.root()});rejects([&]{text->initMapStringFile("map.str");},"map-text malformed candidate");
                require(text->fetch("MAP:Name",&exists).compare(L"a *** map\nnext")==0&&exists,"map-text rejection preserves both lookup/backing links");
            }
            tree.write("map.str","MAP:Name\n\"retry\"\nEnd\n");fs.mountReadOnly({tree.root()});text->initMapStringFile("map.str");
            require(text->fetch("MAP:Name",&exists).compare(L"retry")==0&&exists,"map-text corrected retry");
            text->reset();(void)text->fetch("MAP:Name",&exists);require(!exists,"original map reset withdraws lookup/backing");
        }catch(...){TheLanguageFilter=nullptr;throw;}
        TheLanguageFilter=nullptr;
        tree.write("Data/Generals.str","GUI:Base\n\" base text \" = sound2\nEnd\n");fs.mountReadOnly({tree.root()});
        std::unique_ptr<GameTextInterface> sourceText(CreateGameTextInterface());sourceText->init();
        require(sourceText->fetch("gui:base",&exists).compare(L"base text")==0&&exists,"original source-string precedence over CSF");
        tree.write("grammar.str","// comment\nLABEL\n\" first \\\"quote\\\" \\\\ slash\\t tab\\nline\n continued \" = speech2.extra\nEnd");
        fs.mountReadOnly({tree.root()});FileCloseOwner grammar(fs.openFile("grammar.str"));
        const auto catalog=decodeOriginalStringFile(*grammar);
        require(catalog.records.size()==1&&catalog.records[0].speech=="speech2e","source speech suffix and final END without newline");
        require(catalog.records[0].text.compare(L"first \"quote\" \\ slash\ttab\nline continued")==0,"source escaped/multiline grammar and whitespace");
    }catch(...){TheFileSystem=nullptr;throw;}
    TheFileSystem=nullptr;
}
void iniFields() {
    INIException original("owned diagnostic");
    INIException copied(original),moved(std::move(copied)),assigned(nullptr);
    assigned=moved;original=INIException("replacement");
    require(std::strcmp(assigned.mFailureMessage,"owned diagnostic")==0 && copied.mFailureMessage==nullptr,
            "INI diagnostics copy/move retain independent storage");
    Tree tree;
    struct Fields {Int integer=7;UnsignedInt flags=0;Real real=1;Bool enabled=FALSE;AsciiString name;};
    const FieldParse table[]={
        {"Integer",INI::parseInt,nullptr,offsetof(Fields,integer)},
        {"Flags",INI::parseUnsignedInt,nullptr,offsetof(Fields,flags)},
        {"Real",INI::parseReal,nullptr,offsetof(Fields,real)},
        {"Enabled",INI::parseBool,nullptr,offsetof(Fields,enabled)},
        {"Name",INI::parseQuotedAsciiString,nullptr,offsetof(Fields,name)},
        {nullptr,nullptr,nullptr,0}
    };
    tree.write("fields.ini","Integer = -17 ; comment\r\nFlags = -1\nReal = .5\nEnabled = Yes\nName = \"two words\"\nEnd\n");
    FileSystem fs;fs.mountReadOnly({tree.root()});TheFileSystem=&fs;
    try {
        INI parser;Fields accepted;parser.loadFields("fields.ini",INI_LOAD_OVERWRITE,&accepted,table);
        require(accepted.integer==-17&&accepted.flags==UINT32_MAX&&accepted.real==.5f&&accepted.enabled&&accepted.name=="two words","original INI field dispatch and representation");
        for(const auto& malformed:std::vector<std::string>{
            "Integer = 2147483648\nEnd\n","Real = nan\nEnd\n","Real = 1e50\nEnd\n",
            "Enabled = Maybe\nEnd\n","Unknown = 1\nEnd\n","Integer = 1\n",
            "Integer = "+std::string(1030,'1')+"\nEnd\n"}) {
            tree.write("fields.ini",malformed);fs.mountReadOnly({tree.root()});Fields candidate(accepted);
            bool failed=false;try{parser.loadFields("fields.ini",INI_LOAD_OVERWRITE,&candidate,table);}catch(...){failed=true;}
            require(failed,"INI malformed candidate rejected");
            require(accepted.integer==-17&&accepted.name=="two words","caller offside accepted fields unchanged");
            tree.write("fields.ini","Integer = 23\nName = \"retry\"\nEnd\n");fs.mountReadOnly({tree.root()});Fields retry;
            parser.loadFields("fields.ini",INI_LOAD_OVERWRITE,&retry,table);
            require(retry.integer==23&&retry.name=="retry","same-parser corrected field retry after retirement");
        }
        require(INI::scanInt(" +12suffix")==12&&INI::scanUnsignedInt("-1")==UINT32_MAX,"retained original numeric-prefix rules");
        require(INI::scanPercentToReal("25%")==.25f,"original percentage token");
    }catch(...){TheFileSystem=nullptr;throw;}
    TheFileSystem=nullptr;
}
std::size_t descriptors() {
    DIR* directory=::opendir("/proc/self/fd");if(!directory)throw Failure("descriptor census unavailable");
    std::size_t count=0;while(auto* entry=::readdir(directory))if(entry->d_name[0]!='.')++count;
    ::closedir(directory);return count;
}
struct Residuals {
    std::size_t allocations=AllocationFault::live(),files=descriptors();
    Int native=TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount();
    Int ram=TheMemoryPoolFactory->findMemoryPool("RAMFile")->getUsedBlockCount();
    void verify() const {
        require(AllocationFault::live()==allocations,"failed candidate releases exact standard ownership");
        require(descriptors()==files,"failed candidate releases exact descriptors");
        require(TheMemoryPoolFactory->findMemoryPool("NativeDataFile")->getUsedBlockCount()==native &&
                TheMemoryPoolFactory->findMemoryPool("RAMFile")->getUsedBlockCount()==ram,
                "failed candidate releases exact file pool units");
    }
};
template<class Action,class Rollback,class Restore>
void sweep(Action action,Rollback rollback,Restore restore) {
    // Warm public owner backing before the baseline. Each ordinal keeps its
    // failure, immediate residual proof and same-owner retry in one batch.
    action();restore();
    constexpr std::size_t ceiling=2048;
    for(std::size_t ordinal=0;ordinal<ceiling;++ordinal) {
        const Residuals baseline;
        AllocationFault::arm(ordinal);bool failed=false;
        try{action();}catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        if(!failed){require(!AllocationFault::triggered(),"allocation failure cannot be silently accepted");
            require(ordinal>0,"fault census contains operations");restore();
            std::cout<<"fault ordinals [0,"<<ordinal<<") complete; terminal "<<ordinal<<'\n';return;}
        require(AllocationFault::triggered(),"expected injected allocation rejection");
        baseline.verify();rollback();action();restore();
    }
    throw Failure("fault census exceeded bounded manifest");
}
void faults(const std::string& family) {
    Tree accepted,candidate;
    accepted.write("accepted.big",big({{"accepted","accepted"}}));
    candidate.write("candidate.big",big({{"candidate","candidate"}}));
    candidate.write("nested/deeper/loose.ini","public nested candidate");
    accepted.write("Data/English/Generals.csf",csf());
    accepted.write("map.str","MAP:Name\n\"accepted\"\nEnd\n");
    accepted.write("candidate.str","MAP:Name\n\"bad candidate\" = speech1\nEnd\n");
    accepted.write("fields.ini","Integer = 17\nName = \"candidate\"\nEnd\n");
    std::string words;for(char c:std::string("bad")){auto unit=std::uint16_t(c^0x5555);words+=char(unit);words+=char(unit>>8);}words+=char(0x20);words+=char(0);
    accepted.write("langdata.dat",words);
    const std::vector<std::string> oldRoots{accepted.root()},newRoots{candidate.root()};
    FileSystem fs;fs.mountReadOnly(oldRoots);TheFileSystem=&fs;
    try {
        // Warm both explicitly pooled File providers (including their backing).
        {FileCloseOwner input(fs.openFile("fields.ini"));FileCloseOwner snapshot(input->convertToRAMFile());input.release();}
        if(family=="fault_mount") {
            sweep([&]{fs.mountReadOnly(newRoots);},[&]{require(content(fs,"accepted")=="accepted","mount graph rollback");},[&]{fs.mountReadOnly(oldRoots);});
        }else if(family=="fault_ram") {
            FileCloseOwner input(fs.openFile("fields.ini"));input->seek(3,File::START);
            sweep([&]{auto* file=newInstance(RAMFile);MemoryPoolObjectHolder snapshot(file);
                require(file->open(input.get()),"RAM candidate snapshot");},
                [&]{require(input->position()==3,"RAM candidate restores borrowed cursor");},[&]{input->seek(3,File::START);});
        }else if(family=="fault_csf") {
            FileCloseOwner input(fs.openFile("Data/English/Generals.csf"));input->seek(3,File::START);
            sweep([&]{auto catalog=decodeOriginalCSF(*input);require(catalog.records.size()==1,"CSF candidate decode");},
                [&]{require(input->position()==3,"CSF candidate restores borrowed cursor");},[&]{input->seek(3,File::START);});
        }else if(family=="fault_map") {
            LanguageFilter filter;filter.init();TheLanguageFilter=&filter;
            std::unique_ptr<GameTextInterface> text(CreateGameTextInterface());text->init();text->initMapStringFile("map.str");
            try{sweep([&]{text->initMapStringFile("candidate.str");},
                [&]{require(text->fetch("MAP:Name").compare(L"accepted")==0,"map graph and callback rollback");},
                [&]{text->initMapStringFile("map.str");});}catch(...){TheLanguageFilter=nullptr;throw;}
            TheLanguageFilter=nullptr;
        }else if(family=="fault_metadata") {
            LanguageFilter filter;filter.init();TheLanguageFilter=&filter;
            std::unique_ptr<GameTextInterface> text(CreateGameTextInterface());text->init();text->initMapStringFile("map.str");
            try{sweep([&]{require(text->fetchMapMetadataLabel("candidate.str","MAP:Name").compare(L"*** candidate")==0,
                    "temporary metadata callback result");},
                [&]{require(text->fetch("MAP:Name").compare(L"accepted")==0,"temporary metadata rollback preserves active owner");},[]{});
                require(text->fetch("MAP:Name").compare(L"accepted")==0,"successful temporary metadata retires backing");
            }catch(...){TheLanguageFilter=nullptr;throw;}
            TheLanguageFilter=nullptr;
        }else if(family=="fault_filter") {
            LanguageFilter filter;filter.init();
            sweep([&]{filter.init();},[&]{UnicodeString word(L"bad");filter.filterLine(word);require(word.compare(L"***")==0,"filter graph rollback");},[]{});
        }else if(family=="fault_ini") {
            struct Fields{Int integer=7;AsciiString name;};
            const FieldParse table[]={{"Integer",INI::parseInt,nullptr,offsetof(Fields,integer)},
                {"Name",INI::parseQuotedAsciiString,nullptr,offsetof(Fields,name)},{nullptr,nullptr,nullptr,0}};
            INI parser;Fields acceptedFields;acceptedFields.name="accepted";
            sweep([&]{Fields offside=acceptedFields;parser.loadFields("fields.ini",INI_LOAD_OVERWRITE,&offside,table);
                require(offside.integer==17&&offside.name=="candidate","INI candidate dispatch");},
                [&]{require(acceptedFields.integer==7&&acceptedFields.name=="accepted","INI caller graph unchanged");},[]{});
        }else throw Failure("unknown fault batch");
    }catch(...){TheFileSystem=nullptr;TheLanguageFilter=nullptr;throw;}
    TheFileSystem=nullptr;
}
}
int main(int argc,char** argv) {
    bool initialized=false;
    try {
        require(argc==2,"data family required");initMemoryManager();initialized=true;
        const std::string family=argv[1];
        if(family=="roots")roots();else if(family=="archives")archives();else if(family=="ram")ram();else if(family=="catalogs")catalogs();else if(family=="text")textManager();else if(family=="ini")iniFields();else if(family.starts_with("fault_")){for(int repeat=0;repeat<3;++repeat)faults(family);}else require(false,"unknown data family");
        shutdownMemoryManager();initialized=false;std::cout<<"PASS: original data "<<family<<" (INI/GameLogic acceptance pending)\n";return 0;
    }catch(ErrorCode error){std::cerr<<"FAIL: original data error "<<std::uint32_t(error)<<'\n';}
    catch(const Failure& failure){std::cerr<<"FAIL: "<<failure.what()<<'\n';}
    catch(const std::exception&){std::cerr<<"FAIL: original data fixture\n";}
    TheFileSystem=nullptr;if(initialized)shutdownMemoryManager();return 1;
}
