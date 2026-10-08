// SPDX-License-Identifier: GPL-3.0-or-later
#include "AllocationFault.h"
#include "Common/QuotedPrintable.h"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F action) {
    bool rejected=false;try{action();}catch(ErrorCode){rejected=true;}
    require(rejected,"quoted protocol admission rejects");
}
void functional() {
    require(AsciiStringToQuotedPrintable("abc-_ ")==AsciiString("abc_2D_5F_20"),"source ASCII literals/escapes");
    require(QuotedPrintableToAsciiString("abc_2d_5f_20")==AsciiString("abc-_ "),"source mixed-case hex decode");
    const char high[]{char(0x80),char(0xff),0};
    require(AsciiStringToQuotedPrintable(high)==AsciiString("_80_FF") &&
            QuotedPrintableToAsciiString("_80_FF")==AsciiString(high),"unsigned source byte payloads");
    const UnicodeString text(L"A\u00e9\U0001f600");
    const AsciiString wire("A_00_E9_00_3D_D8_00_DE");
    require(UnicodeStringToQuotedPrintable(text)==wire && QuotedPrintableToUnicodeString(wire)==text,
            "fixed UTF16LE BMP/astral representation independent of wchar width");
    require(UnicodeStringToQuotedPrintable(UnicodeString(L"_"))==AsciiString("_5F_00"),"source Unicode underscore byte encoding");
    for(unsigned repeat=0;repeat<3;++repeat) {
        require(QuotedPrintableToUnicodeString("A_00B_00D_00")==UnicodeString(L"ABD"),"preceding complete value");
        require(QuotedPrintableToUnicodeString("B_00C")==UnicodeString(L"BC"),"odd low byte pads zero, no stale tail");
        const std::string longText(2048,'z');
        require(QuotedPrintableToAsciiString(longText.c_str())==AsciiString(longText.c_str()),
                "complete decode exceeds obsolete static backing safely");
        require(AsciiStringToQuotedPrintable(longText.c_str())==AsciiString(longText.c_str()),
                "complete encoding does not silently truncate");
        require(QuotedPrintableToAsciiString("a")==AsciiString("a"),"short conversion independent of previous backing");
    }
    require(AsciiStringToQuotedPrintable(AsciiString()).isEmpty() &&
            QuotedPrintableToAsciiString(AsciiString()).isEmpty() &&
            UnicodeStringToQuotedPrintable(UnicodeString()).isEmpty() &&
            QuotedPrintableToUnicodeString(AsciiString()).isEmpty(),"empty converters publish empty values");
}
void sourceWitness() {
    // Bounded exact legacy terminator branch, independent of the native codec.
    // The prior static bytes include a complete terminator, so this witness
    // demonstrates the original stale-tail bug without an undefined read/write.
    std::array<unsigned char,10> backing{'A',0,'B',0,'D',0,0,0,0,0};
    const std::array<unsigned char,3> next{'B',0,'C'};
    std::size_t position=0;
    for(auto byte:next)backing[position++]=byte;
    if(position%2) {} else {backing[position++]=0;}
    backing[position]=0;
    require(backing[0]=='B' && backing[2]=='C' && backing[4]=='D',
            "legacy odd-tail branch retains prior complete code unit");
    require(QuotedPrintableToUnicodeString("A_00B_00D_00")==UnicodeString(L"ABD") &&
            QuotedPrintableToUnicodeString("B_00C")==UnicodeString(L"BC"),
            "actual decoder fixes reproduced stale-tail behavior");
    std::size_t legacyCounter=0,advanced=0;
    for(unsigned n=0;n<1024 && legacyCounter<1023;++n)++advanced;
    require(advanced==1024 && legacyCounter==0,"source counter condition does not bound writes");
}
void negative() {
    for(const char* malformed:{"_","_A","_GG","A_0Z","abc_1"}) {
        const AsciiString source(malformed);const auto live=AllocationFault::live();
        rejects([&]{(void)QuotedPrintableToAsciiString(source);});
        rejects([&]{(void)QuotedPrintableToUnicodeString(source);});
        require(source.compare(malformed)==0 && AllocationFault::live()==live,
                "whole escape admission preserves source and backing");
    }
    for(const char* malformed:{"_00","_00_00","_00_D8","_00_DC","_00_D8A_00"})
        rejects([&]{(void)QuotedPrintableToUnicodeString(malformed);});
    rejects([&]{(void)QuotedPrintableToAsciiString("A_00B");});
    for(WideChar scalar:{WideChar(0xd800),WideChar(0xdfff),WideChar(0x110000),WideChar(-1)}) {
        const WideChar source[]{L'A',scalar,0};
        rejects([&]{(void)UnicodeStringToQuotedPrintable(UnicodeString(source));});
    }
    const std::string exact(AsciiString::MAX_LEN-1,'a');
    require(AsciiStringToQuotedPrintable(exact.c_str())==AsciiString(exact.c_str()) &&
            QuotedPrintableToAsciiString(exact.c_str())==AsciiString(exact.c_str()),"exact complete ASCII capacity");
    const std::string escaped((AsciiString::MAX_LEN-1)/3,'_');
    const auto accepted=AsciiStringToQuotedPrintable(escaped.c_str());
    require(accepted.getLength()==AsciiString::MAX_LEN-1,"exact expanded escape capacity");
    const std::string tooMany=escaped+'_';
    rejects([&]{(void)AsciiStringToQuotedPrintable(tooMany.c_str());});
    const std::wstring units((AsciiString::MAX_LEN-1)/4,L'a');
    require(UnicodeStringToQuotedPrintable(UnicodeString(units.c_str())).getLength()==Int(units.size()*4),
            "UTF16 zero-byte expansion counted independently of native bytes");
    const std::wstring tooWide=units+L'a';
    rejects([&]{(void)UnicodeStringToQuotedPrintable(UnicodeString(tooWide.c_str()));});
    require(QuotedPrintableToUnicodeString("A_00")==UnicodeString(L"A"),"corrected conversion after rejection");
    sourceWitness();
}
template<class F> void sweep(F convert) {
    constexpr std::size_t terminal=3;
    for(std::size_t ordinal=0;ordinal<=terminal;++ordinal) {
        const auto live=AllocationFault::live();bool failed=false;
        AllocationFault::arm(ordinal);
        try {auto result=convert();require(!result.isEmpty(),"complete conversion candidate");}
        catch(const std::bad_alloc&){failed=true;}
        catch(...){AllocationFault::disarm();throw;}
        AllocationFault::disarm();
        require(AllocationFault::live()==live,"each quoted candidate retires exact backing");
        if(ordinal==terminal) require(!failed && !AllocationFault::triggered(),"exact conversion terminal");
        else {
            require(failed && AllocationFault::triggered(),"complete converter allocation manifest");
            auto corrected=convert();require(!corrected.isEmpty(),"same-source conversion retry");
        }
    }
}
void faults() {
    const AsciiString ascii(std::string(40,'a').c_str());
    const UnicodeString unicode(std::wstring(40,L'a').c_str());
    const AsciiString encoded=UnicodeStringToQuotedPrintable(unicode);
    sweep([&]{return AsciiStringToQuotedPrintable(ascii);});
    sweep([&]{return QuotedPrintableToAsciiString(ascii);});
    sweep([&]{return UnicodeStringToQuotedPrintable(unicode);});
    sweep([&]{return QuotedPrintableToUnicodeString(encoded);});
    require(ascii==AsciiString(std::string(40,'a').c_str()) &&
            unicode==UnicodeString(std::wstring(40,L'a').c_str()),"all source aliases unchanged");
    std::cout<<"four converters allocations [0,3); terminal 3 each\n";
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"quoted family");const std::string family(argv[1]);
        for(unsigned repeat=0;repeat<3;++repeat) {
            const auto live=AllocationFault::live();
            if(family=="functional")functional();else if(family=="negative")negative();
            else if(family=="faults")faults();else throw std::runtime_error("unknown quoted family");
            require(AllocationFault::live()==live,"complete same-process quoted owner retirement");
        }
        std::cout<<"PASS original quoted format owners (full persistence pending)\n";return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL "<<error.what()<<'\n';}
     catch(ErrorCode){std::cerr<<"FAIL source error\n";}
    AllocationFault::disarm();return 1;
}
