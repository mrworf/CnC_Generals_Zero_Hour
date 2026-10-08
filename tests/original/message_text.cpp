// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/NativeMessageText.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <cwchar>
namespace {
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
bool equal(const UnicodeString& value,const wchar_t* expected){return std::wcscmp(value.str(),expected)==0;}
void functional() {
    UnicodeString format(L"%ls %d %hs");
    require(equal(formatOriginalMessageText(format,L"generated",-17,"narrow"),L"generated -17 narrow"),
        "typed forwarding reaches the actual native UnicodeString formatter");
    require(formatOriginalMessageText(UnicodeString::TheEmptyString).isEmpty(),"empty source message");
    std::wstring excessive(UnicodeString::MAX_FORMAT_BUF_LEN+1,L'x');
    const auto live=AllocationFault::live();
    bool rejected=false;
    try{(void)formatOriginalMessageText(UnicodeString(L"%ls"),excessive.c_str());}
    catch(ErrorCode){rejected=true;}
    require(rejected && AllocationFault::live()==live && equal(format,L"%ls %d %hs"),
        "oversized formatting rejects offside without changing format or live ownership");
    require(equal(formatOriginalMessageText(format,L"corrected",19,"retry"),L"corrected 19 retry"),
        "same-format corrected retry");
}
void faults() {
    const UnicodeString format(L"%ls %d");
    std::size_t census;
    {
        AllocationFault::arm(std::numeric_limits<std::size_t>::max());
        auto candidate=formatOriginalMessageText(format,L"generated candidate",37);
        census=AllocationFault::attempts();AllocationFault::disarm();
        require(equal(candidate,L"generated candidate 37") && census>0 && census<16,"complete bounded formatting census");
    }
    for(std::size_t ordinal=0;ordinal<=census;++ordinal) {
        const auto live=AllocationFault::live();
        bool failed=false;AllocationFault::arm(ordinal);
        try{auto candidate=formatOriginalMessageText(format,L"generated candidate",37);}
        catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}
        const auto attempts=AllocationFault::attempts();const auto triggered=AllocationFault::triggered();
        AllocationFault::disarm();
        require(equal(format,L"%ls %d") && AllocationFault::live()==live,"all formatting rejects retire complete candidate backing");
        if(ordinal<census)require(triggered && failed,"each formatting allocation ordinal rejects");
        else require(!triggered && !failed && attempts==census,"exact formatting terminal succeeds");
        require(equal(formatOriginalMessageText(format,L"generated candidate",37),L"generated candidate 37"),
            "every format fault has a corrected retry");
    }
    std::cout<<"message formatting allocation terminal "<<census<<'\n';
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"message text family");
        const std::string family(argv[1]);
        for(int repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if(family=="functional")functional();else if(family=="faults")faults();
            else throw std::runtime_error("unknown message family");
            require(AllocationFault::live()==live,"same-process complete message lifecycle teardown");
        }
        std::cout<<"PASS original message formatting support (UI delivery pending)\n";return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
    catch(ErrorCode){std::cerr<<"FAIL source error\n";}
    AllocationFault::disarm();return 1;
}
