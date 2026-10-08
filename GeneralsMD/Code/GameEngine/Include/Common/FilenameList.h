// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/AsciiString.h"
#include <set>
#include <memory>
#include <string>
struct FilenameLessNoCase {
    bool operator()(const AsciiString& a,const AsciiString& b) const {
        return a.compareNoCase(b)<0;
    }
};
using FilenameList=std::set<AsciiString,FilenameLessNoCase>;
using FilenameListIter=FilenameList::iterator;
