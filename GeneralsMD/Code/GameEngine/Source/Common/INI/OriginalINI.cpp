/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

// M27 extraction provenance: INI::loadDirectory, INI::load, INI::readLine,
// INI::scanBool, INI::scanInt and INI::scanReal in this directory's INI.cpp.
// The complete theTypeTable is intentionally untouched and remains M20-owned.

#include "zh/original_data.h"

#include "GameLogic/FPUControl.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <sstream>
#include <strings.h>

namespace zh::original_data {
namespace {

std::string trim(std::string value)
{
    const auto whitespace = [](unsigned char character) { return std::isspace(character) != 0; };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), whitespace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), whitespace).base(), value.end());
    return value;
}

std::uint32_t name_key(std::string_view value) noexcept
{
    std::uint32_t result = 0;
    for (const unsigned char character : value) result = result * 33U + character;
    return result;
}

std::u16string utf8_to_utf16(std::string_view value)
{
    std::u16string result;
    for (std::size_t index = 0; index < value.size();) {
        const auto lead = static_cast<unsigned char>(value[index++]);
        std::uint32_t scalar = 0;
        std::size_t continuation = 0;
        if (lead < 0x80) scalar = lead;
        else if ((lead & 0xe0U) == 0xc0U) { scalar = lead & 0x1fU; continuation = 1; }
        else if ((lead & 0xf0U) == 0xe0U) { scalar = lead & 0x0fU; continuation = 2; }
        else if ((lead & 0xf8U) == 0xf0U) { scalar = lead & 0x07U; continuation = 3; }
        else throw Error("invalid UTF-8 INI value");
        if (index + continuation > value.size()) throw Error("truncated UTF-8 INI value");
        for (std::size_t count = 0; count < continuation; ++count) {
            const auto byte = static_cast<unsigned char>(value[index++]);
            if ((byte & 0xc0U) != 0x80U) throw Error("invalid UTF-8 INI continuation");
            scalar = (scalar << 6U) | (byte & 0x3fU);
        }
        if ((continuation == 1 && scalar < 0x80U) || (continuation == 2 && scalar < 0x800U) ||
            (continuation == 3 && scalar < 0x10000U) || scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) throw Error("non-canonical UTF-8 INI value");
        if (scalar <= 0xffffU) result.push_back(static_cast<char16_t>(scalar));
        else {
            scalar -= 0x10000U;
            result.push_back(static_cast<char16_t>(0xd800U + (scalar >> 10U)));
            result.push_back(static_cast<char16_t>(0xdc00U + (scalar & 0x3ffU)));
        }
    }
    return result;
}

IniValue parse_value(IniFieldType type, const std::string& token)
{
    switch (type) {
    case IniFieldType::boolean:
        if (strcasecmp(token.c_str(), "yes") == 0) return true;
        if (strcasecmp(token.c_str(), "no") == 0) return false;
        throw Error("invalid INI Bool value '" + token + "'");
    case IniFieldType::integer: {
        char* end = nullptr;
        errno = 0;
        const long converted = std::strtol(token.c_str(), &end, 0);
        if (errno != 0 || end != token.c_str() + token.size() || converted < INT32_MIN || converted > INT32_MAX)
            throw Error("invalid INI Int value '" + token + "'");
        return static_cast<std::int32_t>(converted);
    }
    case IniFieldType::real: {
        char* end = nullptr;
        errno = 0;
        const float value = std::strtof(token.c_str(), &end);
        if (errno != 0 || end != token.c_str() + token.size() || !std::isfinite(value))
            throw Error("invalid INI Real value '" + token + "'");
        return value;
    }
    case IniFieldType::ascii: return token;
    case IniFieldType::utf16: return utf8_to_utf16(token);
    case IniFieldType::name_key: return name_key(token);
    }
    throw Error("unknown INI field type");
}

} // namespace

std::vector<IniBlock> parse_ini(
    const std::vector<std::uint8_t>& bytes, std::string_view block_type, const IniSchema& schema, const Limits& limits)
{
    setFPMode();
    if (bytes.size() > limits.maximum_file_bytes) throw Error("INI input exceeds configured limit");
    if (block_type.empty()) throw Error("INI block type must not be empty");
    std::istringstream input(std::string(bytes.begin(), bytes.end()));
    std::vector<IniBlock> result;
    IniBlock pending;
    bool in_block = false;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.size() > limits.maximum_line_bytes) throw Error("INI line exceeds configured limit");
        const auto comment = line.find(';');
        if (comment != std::string::npos) line.resize(comment);
        line = trim(std::move(line));
        if (line.empty()) continue;
        const auto split = line.find_first_of(" \t=");
        const auto token = line.substr(0, split);
        const auto value = split == std::string::npos ? std::string{} : trim(line.substr(split + 1));
        if (!in_block) {
            if (token != block_type || !value.empty())
                throw Error("unknown INI block '" + token + "' at line " + std::to_string(line_number));
            pending = {token, {}};
            in_block = true;
            continue;
        }
        if (token == "END") {
            if (!value.empty()) throw Error("INI END token has trailing data");
            result.push_back(std::move(pending));
            if (result.size() > limits.maximum_records) throw Error("INI block count exceeds configured limit");
            in_block = false;
            continue;
        }
        const auto field = schema.find(token);
        if (field == schema.end()) throw Error("unknown INI field '" + token + "'");
        if (value.empty()) throw Error("missing INI value for '" + token + "'");
        const auto parsed = parse_value(field->second, value);
        pending.fields.insert_or_assign(token, parsed);
    }
    if (!input.eof()) throw Error("failed while reading INI input");
    if (in_block) throw Error("unterminated INI block");
    return result;
}

std::vector<IniBlock> load_ini_directory(
    const LogicalFiles& files, std::string_view directory, std::string_view block_type,
    const IniSchema& schema, bool recursive, const Limits& limits)
{
    std::vector<IniBlock> result;
    for (const auto& name : files.enumerate_directory(directory, ".ini", recursive)) {
        auto parsed = parse_ini(files.read(name), block_type, schema, limits);
        if (parsed.size() > limits.maximum_records - std::min(result.size(), limits.maximum_records))
            throw Error("INI directory block count exceeds configured limit");
        result.insert(result.end(), std::make_move_iterator(parsed.begin()), std::make_move_iterator(parsed.end()));
    }
    return result;
}

const char* provider_ini_identity() noexcept { return "OriginalINI.cpp"; }

} // namespace zh::original_data
