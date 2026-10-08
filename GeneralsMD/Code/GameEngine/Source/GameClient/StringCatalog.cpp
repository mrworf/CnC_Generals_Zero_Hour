// SPDX-License-Identifier: GPL-3.0-or-later
// Bounded native extraction of GameText's original .str/map.str grammar.
#include "GameClient/CSF.h"
#include "Common/file.h"
#include "Common/UTF16.h"
#include <cctype>
#include <set>
#include <strings.h>

namespace {
constexpr std::size_t limit=10*1024;
std::string trim(std::string text) {
    const auto first=text.find_first_not_of(" \t\r\n\v\f");
    if(first==std::string::npos)return {};
    return text.substr(first,text.find_last_not_of(" \t\r\n\v\f")-first+1);
}
class Reader {
    std::string bytes;std::size_t position=0;
public:
    explicit Reader(File& file) {
        const Int length=file.size();if(length<0 || file.seek(0,File::START)!=0)throw ERROR_BAD_ARG;
        bytes.resize(std::size_t(length));
        if(file.read(bytes.data(),length)!=length || bytes.find('\0')!=std::string::npos)throw ERROR_BAD_ARG;
    }
    bool eof()const{return position==bytes.size();}
    char get(){if(eof())throw ERROR_BAD_ARG;return bytes[position++];}
    std::string line() {
        std::string result;
        while(!eof()){char value=get();if(value=='\n')break;if(result.size()>=limit-1)throw ERROR_BAD_ARG;result+=value;}
        return trim(std::move(result));
    }
    std::pair<std::wstring,std::string> quoted(std::string opening) {
        // The source reader trims the first physical line and appends a newline
        // before continuing. Preserve that whitespace/escape contract.
        std::string raw;bool slash=false;std::size_t cursor=1;
        bool initial=true;
        auto next=[&]() {
            if(initial && cursor<opening.size())return opening[cursor++];
            if(initial){initial=false;return '\n';}
            return get();
        };
        for(;;) {
            char value=next();
            if(value=='"'&&!slash)break;
            if(value=='\n'){slash=false;value=' ';}
            else if(value=='\\')slash=!slash;
            else slash=false;
            if(std::isspace(static_cast<unsigned char>(value)))value=' ';
            if(raw.size()>=limit-1)throw ERROR_BAD_ARG;
            raw+=value;
        }
        std::string speech;bool started=false,done=false;
        while(initial || !eof()) {
            const char value=next();if(value=='\n')break;
            if(!started && (std::isspace(static_cast<unsigned char>(value))||value=='='))continue;
            started=true;
            const bool valid=(value>='a'&&value<='z')||(value>='A'&&value<='Z')||(value>='0'&&value<='9')||value=='_';
            if(!done&&valid){if(speech.size()>=limit-2)throw ERROR_BAD_ARG;speech+=value;}else done=true;
        }
        if(!speech.empty() && speech.back()>='0'&&speech.back()<='9')speech+='e';
        std::wstring text;slash=false;
        for(unsigned char value:raw) {
            if(slash){slash=false;text+=(value=='n'?L'\n':value=='t'?L'\t':WideChar(value));}
            else if(value=='\\')slash=true;
            else text+=WideChar(value);
        }
        return {normalizeOriginalText(text),speech};
    }
};
}
CSFCatalog decodeOriginalStringFile(File& file) {
    const Int prior=file.position();if(prior<0)throw ERROR_BAD_ARG;
    try {
        Reader reader(file);CSFCatalog catalog;std::set<std::string> labels;
        while(!reader.eof()) {
            auto label=reader.line();if(label.empty()||label.starts_with("//"))continue;
            std::string folded=label;for(char& c:folded)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
            if(!labels.insert(folded).second)throw ERROR_BAD_ARG;
            CSFRecord candidate;candidate.label=label.c_str();bool textRead=false,ended=false;
            while(!reader.eof()) {
                auto line=reader.line();if(line.empty()||line.starts_with("//"))continue;
                if(strcasecmp(line.c_str(),"END")==0){ended=true;break;}
                if(line.front()=='"') {
                    if(textRead)throw ERROR_BAD_ARG;
                    auto [text,speech]=reader.quoted(std::move(line));candidate.text=text.c_str();candidate.speech=speech.c_str();textRead=true;
                }
            }
            if(!ended)throw ERROR_BAD_ARG;
            catalog.records.push_back(std::move(candidate));
        }
        return catalog;
    }catch(...){file.seek(prior,File::START);throw;}
}
