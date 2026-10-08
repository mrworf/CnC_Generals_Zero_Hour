/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/LanguageFilter.h"
#include "Common/FileSystem.h"
#include "Common/file.h"
#include "Common/FileOwner.h"
#include "Common/UTF16.h"
#include <array>
#include <memory>

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


LanguageFilter *TheLanguageFilter = NULL;

LanguageFilter::LanguageFilter() 
{
	//Modified by Saad
	//Unnecessary
	//m_wordList.clear();
}

LanguageFilter::~LanguageFilter() {
	m_wordList.clear();
}

void LanguageFilter::init() {
    if(!TheFileSystem)throw ERROR_BAD_ARG;
    FileCloseOwner file(TheFileSystem->openFile(BadWordFileName,File::READ|File::BINARY));
    LangMap candidate;
    if(file) {
        wchar_t word[128]{};
        while(readWord(file.get(),word)) {
            if(!*word)continue;
            UnicodeString value(word);unHaxor(value);
            candidate[value]=true;
        }
    }
    m_wordList.swap(candidate);
}

void LanguageFilter::reset() {
	init();
}

void LanguageFilter::update() {
}

wchar_t ignoredChars[] = L"-_*'\"";

void LanguageFilter::filterLine(UnicodeString &line) 
{
	auto backing=std::make_unique<WideChar[]>(std::size_t(line.getLength())+1);
	WideChar* buf=backing.get();
	wcscpy(buf,line.str());

	UnicodeString newLine(line);
	UnicodeString token(L"");

	while (newLine.nextToken(&token, UnicodeString(L" ;,.!?:=\\/><`~()&^%#\n\t"))) {
		wchar_t *pos = wcsstr(buf, token.str());
		if (pos == NULL) {
			DEBUG_CRASH(("Couldn't find the token in its own string."));
			continue;
		}

		Int len = token.getLength(); // need to get the length of the original word, not the unhaxor'd word.

		unHaxor(token);
		LangMapIter iter = m_wordList.find(token);
		if (iter != m_wordList.end()) {
			DEBUG_LOG(("Found word %ls in bad word list. Token was %ls\n", (*iter).first.str(), token.str()));
			for (Int i = 0; i < len; ++i) {
				*pos = L'*';
				++pos;
			}
		}
	}

	line.set(buf);
}

void LanguageFilter::unHaxor(UnicodeString &word) {
	Int len = word.getLength();
	UnicodeString newWord(L"");
	for (Int i = 0; i < len; ++i) {
		wchar_t c = word.getCharAt(i);
		if ((c == L'p') || (c == L'P')) {
			if (((i + 1) < len) && ((word.getCharAt(i+1) == L'h') || (word.getCharAt(i+1) == L'H'))) {
				newWord.concat(L'f');
				++i; // skip the h
			} else {
				// not a problem at all.
				newWord.concat(c);
			}
		} else if (c == L'1') {
			newWord.concat(L'l');
		} else if (c == L'3') {
			newWord.concat(L'e');
		} else if (c == L'4') {
			newWord.concat(L'a');
		} else if (c == L'5') {
			newWord.concat(L's');
		} else if (c == L'6') {
			newWord.concat(L'b');
		} else if (c == L'7') {
			newWord.concat(L't');
		} else if (c == L'0') {
			newWord.concat(L'o');
		} else if (c == L'@') {
			newWord.concat(L'a');
		} else if (c == L'$') {
			newWord.concat(L's');
		} else if (c == L'+') {
			newWord.concat(L't');
		} else if (wcsrchr(ignoredChars, c) == NULL) {
			newWord.concat(c);
		}
	}
	word.set(newWord);
}

// returning true means that there are more words in the file.
Bool LanguageFilter::readWord(File* file,WideChar* output) {
    if(!file || !output)throw ERROR_BAD_ARG;
    std::array<UnsignedShort,127> units{};std::size_t count=0;
    for(;;) {
        std::array<unsigned char,2> bytes{};
        const Int read=file->read(bytes.data(),2);
        if(read==0 && count==0){output[0]=0;return FALSE;}
        if(read!=2)throw ERROR_BAD_ARG;
        const auto unit=UnsignedShort(UnsignedInt(bytes[0])|(UnsignedInt(bytes[1])<<8));
        if(unit==0x20)break; // original raw separator, outside XOR payload
        if(count==units.size())throw ERROR_BAD_ARG;
        units[count++]=UnsignedShort(unit^LANGUAGE_XOR_KEY);
    }
    const auto word=decodeOriginalUTF16(std::span<const UnsignedShort>(units.data(),count));
    std::copy(word.begin(),word.end(),output);output[word.size()]=0;return TRUE;
}

LanguageFilter * createLanguageFilter() 
{
	return NEW LanguageFilter;
}
