// SPDX-License-Identifier: GPL-3.0-or-later
// Fixed-width extraction of GameText.cpp's original CSF reader protocol.
#include "GameClient/CSF.h"
#include "Common/file.h"
#include "Common/UTF16.h"
#include <array>
#include <set>
#include <string>

namespace {
class Reader {
    File& file;
    Int remaining;
public:
    explicit Reader(File& input):file(input),remaining(input.size()) {
        if(remaining<24 || file.seek(0,File::START)!=0)throw ERROR_BAD_ARG;
    }
    UnsignedInt word() {
        std::array<unsigned char,4> bytes{};
        if(remaining<4 || file.read(bytes.data(),4)!=4)throw ERROR_BAD_ARG;
        remaining-=4;
        return UnsignedInt(bytes[0])|(UnsignedInt(bytes[1])<<8)|(UnsignedInt(bytes[2])<<16)|(UnsignedInt(bytes[3])<<24);
    }
    std::string bytes(UnsignedInt count) {
        if(count>UnsignedInt(remaining) || count>65534)throw ERROR_BAD_ARG;
        std::string value(count,'\0');
        if(file.read(value.data(),Int(count))!=Int(count))throw ERROR_BAD_ARG;
        remaining-=Int(count);if(value.find('\0')!=std::string::npos)throw ERROR_BAD_ARG;return value;
    }
    std::wstring text(UnsignedInt count) {
        if(count>UnsignedInt(remaining)/2 || count>65534)throw ERROR_BAD_ARG;
        std::vector<UnsignedShort> units;units.reserve(count);
        for(UnsignedInt ordinal=0;ordinal<count;++ordinal) {
            std::array<unsigned char,2> bytes{};
            if(file.read(bytes.data(),2)!=2)throw ERROR_BAD_ARG;
            remaining-=2;units.push_back(UnsignedShort(~(UnsignedInt(bytes[0])|(UnsignedInt(bytes[1])<<8))));
        }
        const auto result=decodeOriginalUTF16(units);
        return normalizeOriginalText(result);
    }
    Int left()const{return remaining;}
};
}
CSFCatalog decodeOriginalCSF(File& file) {
    const Int prior=file.position();if(prior<0)throw ERROR_BAD_ARG;
    try {
        Reader reader(file);if(reader.word()!=0x43534620u)throw ERROR_BAD_ARG;
        CSFCatalog catalog;catalog.version=reader.word();const auto labels=reader.word(),strings=reader.word();
        (void)reader.word();const auto language=reader.word();
        if(catalog.version<1 || catalog.version>3 || language>9 || labels>UnsignedInt(reader.left())/12 || strings>UnsignedInt(reader.left())/8)throw ERROR_BAD_ARG;
        catalog.language=catalog.version>=2?language:0;
        catalog.records.reserve(labels);std::set<std::string> names;UnsignedInt consumed=0;
        for(UnsignedInt ordinal=0;ordinal<labels;++ordinal) {
            if(reader.word()!=0x4c424c20u)throw ERROR_BAD_ARG;
            const auto alternatives=reader.word();const auto name=reader.bytes(reader.word());
            if(name.empty() || alternatives>strings-consumed)throw ERROR_BAD_ARG;
            std::string folded=name;for(char& c:folded)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
            if(!names.insert(folded).second)throw ERROR_BAD_ARG;
            CSFRecord candidate;candidate.label=name.c_str();
            for(UnsignedInt alternative=0;alternative<alternatives;++alternative) {
                const auto kind=reader.word();if(kind!=0x53545220u && kind!=0x53545257u)throw ERROR_BAD_ARG;
                auto text=reader.text(reader.word());std::string speech;
                if(kind==0x53545257u)speech=reader.bytes(reader.word());
                if(alternative==0){candidate.text=text.c_str();candidate.speech=speech.c_str();}
                ++consumed;
            }
            catalog.records.push_back(std::move(candidate));
        }
        if(consumed!=strings || reader.left()!=0)throw ERROR_BAD_ARG;
        return catalog;
    }catch(...){file.seek(prior,File::START);throw;}
}
