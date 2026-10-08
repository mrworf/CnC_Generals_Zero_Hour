// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
#include <vector>
class File;
struct CSFRecord {AsciiString label;UnicodeString text;AsciiString speech;};
struct CSFCatalog {UnsignedInt version=0,language=0;std::vector<CSFRecord> records;};
// Borrowed File: decode a complete offside catalog, restore cursor on rejection.
CSFCatalog decodeOriginalCSF(File& file);
CSFCatalog decodeOriginalStringFile(File& file);
