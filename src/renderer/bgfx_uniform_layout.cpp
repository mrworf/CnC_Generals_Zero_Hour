#include "zh/renderer/bgfx_uniform_layout.h"

#include <cstring>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace zh::renderer {
namespace {
[[noreturn]] void invalid() { throw std::runtime_error("malformed bounded bgfx uniform layout"); }
struct Type {
    unsigned opcode = 0;
    std::vector<UInt32> operands;
};
struct Layout {
    std::map<UInt32, Type> types;
    std::map<UInt32, UInt32> constants, strides;
    std::map<std::pair<UInt32, UInt32>, UInt32> offsets, matrix_strides;
    UInt32 alignment(UInt32 id) const {
        const auto found = types.find(id);
        if (found == types.end())
            invalid();
        const auto &type = found->second;
        if (type.opcode >= 20 && type.opcode <= 22)
            return 4;
        if (type.opcode == 23 && type.operands.size() == 2)
            return type.operands[1] == 2 ? 8 : 16;
        return 16;
    }
    UInt32 kind(UInt32 id, unsigned depth = 0) const {
        if (depth > 32)
            invalid();
        const auto t = types.find(id);
        if (t == types.end())
            invalid();
        if (t->second.opcode == 28 && t->second.operands.size() == 2)
            return kind(t->second.operands[0], depth + 1);
        if (t->second.opcode == 24 && t->second.operands.size() == 2) {
            const auto vector = types.find(t->second.operands[0]);
            if (vector == types.end() || vector->second.opcode != 23 ||
                vector->second.operands.size() != 2 ||
                vector->second.operands[1] != t->second.operands[1])
                invalid();
            if (t->second.operands[1] == 3)
                return 3;
            if (t->second.operands[1] == 4)
                return 4;
            invalid();
        }
        if (t->second.opcode >= 20 && t->second.opcode <= 23)
            return 2;
        invalid();
    }
    UInt32 size(UInt32 id, UInt32 matrix_stride = 0, unsigned depth = 0) const {
        if (depth > 32)
            invalid();
        const auto found = types.find(id);
        if (found == types.end())
            invalid();
        const auto &t = found->second;
        const auto &a = t.operands;
        switch (t.opcode) {
        case 20:
            if (!a.empty())
                invalid();
            return 4; // OpTypeBool, std140 scalar storage.
        case 21:
            if (a.size() != 2 || a[0] != 32 || a[1] > 1)
                invalid();
            return 4;
        case 22:
            if (a.size() != 1 || a[0] != 32)
                invalid();
            return 4;
        case 23: {
            if (a.size() != 2 || a[1] < 2 || a[1] > 4)
                invalid();
            const auto scalar = types.find(a[0]);
            if (scalar == types.end() || scalar->second.opcode < 20 || scalar->second.opcode > 22)
                invalid();
            return size(a[0], 0, depth + 1) * a[1];
        }
        case 24:
            if (a.size() != 2 || a[1] < 2 || a[1] > 4 || matrix_stride != 16 ||
                size(a[0], 0, depth + 1) > matrix_stride)
                invalid();
            return matrix_stride * a[1];
        case 28: {
            if (a.size() != 2)
                invalid();
            const auto count = constants.find(a[1]), stride = strides.find(id);
            if (count == constants.end() || stride == strides.end() || !count->second ||
                count->second > 4096 || stride->second % 16 ||
                stride->second < size(a[0], matrix_stride, depth + 1) ||
                UInt64(stride->second) * count->second > 65536)
                invalid();
            return stride->second * count->second;
        }
        case 30: {
            if (a.empty() || a.size() > 256)
                invalid();
            UInt32 end = 0;
            for (UInt32 member = 0; member < a.size(); ++member) {
                const auto key = std::pair{id, member};
                const auto offset = offsets.find(key), stride = matrix_strides.find(key);
                if (offset == offsets.end() || offset->second < end ||
                    offset->second % alignment(a[member]))
                    invalid();
                const auto extent =
                    size(a[member], stride == matrix_strides.end() ? 0 : stride->second, depth + 1);
                if (UInt64(offset->second) + extent > 65536)
                    invalid();
                end = offset->second + extent;
            }
            return (end + 15U) & ~15U;
        }
        default:
            invalid();
        }
    }
};
UInt32 u32(const std::vector<char> &data, std::size_t at) {
    if (at > data.size() || data.size() - at < 4)
        invalid();
    UInt32 result = 0;
    for (unsigned i = 0; i < 4; ++i)
        result |= UInt32(static_cast<unsigned char>(data[at + i])) << (i * 8);
    return result;
}
std::string name(const std::vector<UInt32> &words, std::size_t start, std::size_t end) {
    std::string result;
    for (auto i = start; i < end; ++i)
        for (unsigned byte = 0; byte < 4; ++byte) {
            const char c = static_cast<char>((words[i] >> (byte * 8)) & 255);
            if (!c) {
                if (result.size() > 255)
                    invalid();
                return result;
            }
            result.push_back(c);
        }
    invalid();
}
} // namespace

ValidationResult decode_bgfx_uniform_layout(const std::vector<char> &binary, ShaderStage stage,
                                            std::vector<BgfxUniformBlockLayout> &output) {
    try {
        if (binary.size() < 28 || binary.size() > 4 * 1024 * 1024 ||
            std::memcmp(binary.data(), stage == ShaderStage::vertex ? "VSH" : "FSH", 3) ||
            static_cast<unsigned char>(binary[3]) != 12)
            invalid();
        std::size_t cursor = 22;
        struct Reflection {
            std::string path;
            UInt32 kind = 0, count = 0, offset = 0, registers = 0;
        };
        std::vector<Reflection> reflected;
        std::set<std::string> reflected_names;
        const auto count = static_cast<unsigned char>(binary[20]) |
                           (UInt32(static_cast<unsigned char>(binary[21])) << 8);
        if (count > 1024)
            invalid();
        for (UInt32 i = 0; i < count; ++i) {
            if (cursor >= binary.size())
                invalid();
            const auto length = static_cast<unsigned char>(binary[cursor++]);
            if (!length || cursor + length + 10 > binary.size())
                invalid();
            std::string path(binary.data() + cursor, length);
            if (!reflected_names.emplace(path).second)
                invalid();
            cursor += length;
            const auto byte = [&](std::size_t at) {
                return UInt32(static_cast<unsigned char>(binary[at]));
            };
            reflected.push_back({std::move(path), byte(cursor), byte(cursor + 1),
                                 byte(cursor + 2) | (byte(cursor + 3) << 8),
                                 byte(cursor + 4) | (byte(cursor + 5) << 8)});
            const auto kind = reflected.back().kind;
            if ((kind & 32U) ? ((kind & ~48U) != 0 || reflected.back().count != 0)
                             : ((kind & ~31U) != 0 || (kind & 15U) < 2 || (kind & 15U) > 4 ||
                                reflected.back().count == 0))
                invalid();
            cursor += 10;
        }
        const auto bytes = u32(binary, cursor);
        cursor += 4;
        if (bytes < 20 || bytes % 4 || bytes > binary.size() - cursor)
            invalid();
        std::vector<UInt32> words;
        words.reserve(bytes / 4);
        for (UInt32 at = 0; at < bytes; at += 4)
            words.push_back(u32(binary, cursor + at));
        cursor += bytes;
        if (cursor + 4 > binary.size() || binary[cursor++] != 0)
            invalid();
        const auto attributes = static_cast<unsigned char>(binary[cursor++]);
        if (cursor + attributes * 2U + 2U != binary.size() ||
            (stage == ShaderStage::fragment && attributes) ||
            (stage == ShaderStage::vertex && !attributes))
            invalid();
        std::set<UInt32> live_attributes;
        for (unsigned i = 0; i < attributes; ++i) {
            const auto attr = UInt32(static_cast<unsigned char>(binary[cursor + i * 2])) |
                              (UInt32(static_cast<unsigned char>(binary[cursor + i * 2 + 1])) << 8);
            if (attr != 65535 && !live_attributes.emplace(attr).second)
                invalid();
        }
        if (stage == ShaderStage::vertex && live_attributes.empty())
            invalid();
        if (words[0] != 0x07230203 || words[1] != 0x00010000 || words[4] != 0 || !words[3] ||
            words[3] > 65536)
            invalid();
        Layout layout;
        std::map<UInt32, std::string> names;
        std::map<std::pair<UInt32, UInt32>, std::string> members;
        std::map<UInt32, UInt32> pointers, variables, sets, bindings;
        std::set<UInt32> block_types;
        unsigned entry_points = 0;
        for (std::size_t i = 5; i < words.size();) {
            const auto opcode = words[i] & 65535, count_words = words[i] >> 16;
            if (!count_words || count_words > words.size() - i)
                invalid();
            const auto end = i + count_words;
            const auto put = [&](auto &map, const auto &key, const auto &value) {
                UInt32 id = 0;
                if constexpr (std::is_integral_v<std::decay_t<decltype(key)>>)
                    id = key;
                else {
                    id = key.first;
                    if (key.second >= 256)
                        invalid();
                }
                if (!id || id >= words[3])
                    invalid();
                if (!map.emplace(key, value).second)
                    invalid();
            };
            if (opcode == 15 && count_words >= 4) {
                if (++entry_points != 1 || words[i + 1] != (stage == ShaderStage::vertex ? 0U : 4U))
                    invalid();
            }
            if (opcode == 5 && count_words >= 3)
                put(names, words[i + 1], name(words, i + 2, end));
            else if (opcode == 6 && count_words >= 4)
                put(members, std::pair{words[i + 1], words[i + 2]}, name(words, i + 3, end));
            else if (opcode >= 20 && opcode <= 32) {
                if (count_words < 2)
                    invalid();
                Type type;
                type.opcode = opcode;
                type.operands.assign(words.begin() + i + 2, words.begin() + end);
                put(layout.types, words[i + 1], type);
                if (opcode == 32 && count_words == 4 && words[i + 2] == 2)
                    put(pointers, words[i + 1], words[i + 3]);
            } else if (opcode == 43 && count_words == 4)
                put(layout.constants, words[i + 2], words[i + 3]);
            else if (opcode == 59 && count_words >= 4 && words[i + 3] == 2)
                put(variables, words[i + 2], words[i + 1]);
            else if (opcode == 71 && count_words >= 3) {
                if (words[i + 2] == 2 && (count_words != 3 || !block_types.emplace(words[i + 1]).second))
                    invalid();
                if ((words[i + 2] == 6 || words[i + 2] == 33 || words[i + 2] == 34) && count_words != 4)
                    invalid();
                if (words[i + 2] == 6)
                    put(layout.strides, words[i + 1], words[i + 3]);
                if (words[i + 2] == 33)
                    put(bindings, words[i + 1], words[i + 3]);
                if (words[i + 2] == 34)
                    put(sets, words[i + 1], words[i + 3]);
            } else if (opcode == 72 && count_words >= 5) {
                if (words[i + 3] == 35)
                    put(layout.offsets, std::pair{words[i + 1], words[i + 2]}, words[i + 4]);
                if (words[i + 3] == 7)
                    put(layout.matrix_strides, std::pair{words[i + 1], words[i + 2]}, words[i + 4]);
            }
            i = end;
        }
        if (entry_points != 1)
            invalid();
        const auto valid_id = [&](UInt32 id) { return id != 0 && id < words[3]; };
        for (const auto &item : names)
            if (!valid_id(item.first))
                invalid();
        for (const auto &item : layout.types)
            if (!valid_id(item.first))
                invalid();
        for (const auto &item : variables)
            if (!valid_id(item.first) || !valid_id(item.second))
                invalid();
        std::vector<BgfxUniformBlockLayout> next;
        for (const auto &variable : variables) {
            const auto n = names.find(variable.first);
            if (n == names.end() || n->second != "zh_uniforms")
                invalid(); // A present foreign provider is not an optimized-out UBO.
            if (!next.empty() || !sets.count(variable.first) || !bindings.count(variable.first) ||
                sets.at(variable.first) != 0 ||
                bindings.at(variable.first) != (stage == ShaderStage::vertex ? 0U : 1U) ||
                !pointers.count(variable.second))
                invalid();
            const auto root = pointers.at(variable.second);
            const auto t = layout.types.find(root);
            if (!block_types.count(root) || t == layout.types.end() || t->second.opcode != 30 ||
                t->second.operands.empty() || t->second.operands.size() > 4)
                invalid();
            (void)layout.size(root);
            std::set<std::string> unique;
            for (UInt32 member = 0; member < t->second.operands.size(); ++member) {
                const auto key = std::pair{root, member};
                if (!members.count(key) || members.at(key).empty() || !layout.offsets.count(key) ||
                    !unique.emplace(members.at(key)).second)
                    invalid();
                const auto source_type = t->second.operands[member];
                const auto source = layout.types.find(source_type);
                if (source == layout.types.end() || source->second.opcode != 30)
                    invalid();
                BgfxUniformBlockLayout block{
                    members.at(key), layout.offsets.at(key), layout.size(source_type), {}};
                std::set<std::string> field_names;
                for (UInt32 field = 0; field < source->second.operands.size(); ++field) {
                    const auto field_key = std::pair{source_type, field};
                    if (!members.count(field_key) || members.at(field_key).empty() ||
                        !layout.offsets.count(field_key) ||
                        !field_names.emplace(members.at(field_key)).second)
                        invalid();
                    const auto stride = layout.matrix_strides.find(field_key);
                    const auto field_type = source->second.operands[field];
                    const auto field_desc = layout.types.find(field_type);
                    if (field_desc == layout.types.end())
                        invalid();
                    UInt32 field_count = 1;
                    if (field_desc->second.opcode == 28) {
                        const auto &args = field_desc->second.operands;
                        if (args.size() != 2 || !layout.constants.count(args[1]))
                            invalid();
                        field_count = layout.constants.at(args[1]);
                    }
                    block.fields.push_back(
                        {members.at(field_key), block.origin + layout.offsets.at(field_key),
                         layout.size(field_type,
                                     stride == layout.matrix_strides.end() ? 0 : stride->second),
                         field_count, layout.kind(field_type)});
                }
                next.push_back(std::move(block));
            }
        }
        // Header offsets are independently checked against declared SPIR-V
        // members. Optimized-out entire UBOs have no runtime fields to bind.
        for (const auto &item : reflected) {
            if (item.path.rfind("ZhStageUniforms.", 0) != 0 || next.empty())
                continue;
            bool admitted = false;
            for (const auto &block : next)
                for (const auto &field : block.fields) {
                    if (item.path != "ZhStageUniforms." + block.instance + "." + field.name)
                        continue;
                    if (item.offset != field.offset || (item.kind & 15U) != field.kind ||
                        bool(item.kind & 16U) != (stage == ShaderStage::fragment) ||
                        (item.kind & ~31U) || item.count != field.count ||
                        item.registers != ((field.extent + 15U) & ~15U) / 16U ||
                        UInt64(field.offset) + ((field.extent + 15U) & ~15U) >
                            UInt64(block.origin) + block.extent)
                        invalid();
                    admitted = true;
                }
            if (!admitted)
                invalid();
        }
        output.swap(next);
        return {};
    } catch (const std::exception &) {
        return {false, "malformed bounded bgfx uniform layout"};
    }
}

std::string encode_bgfx_uniform_layout(ShaderStage stage,
                                       const std::vector<BgfxUniformBlockLayout> &blocks) {
    std::ostringstream out;
    out << "zh-bgfx-uniform-layout 1 " << (stage == ShaderStage::vertex ? "vertex" : "fragment") << ' '
        << blocks.size() << '\n';
    for (const auto &block : blocks)
        out << "block " << block.instance << ' ' << block.source_binding << ' ' << block.origin << ' '
            << block.extent << '\n';
    return out.str();
}

ValidationResult associate_bgfx_uniform_bindings(std::vector<BgfxUniformBlockLayout> &blocks,
                                                 const std::map<std::string, UInt32> &source_bindings) {
    std::set<UInt32> unique;
    for (const auto &binding : source_bindings)
        if (binding.second >= 4 || !unique.emplace(binding.second).second)
            return {false, "duplicate or oversized source uniform binding"};
    UInt32 previous = 0;
    bool first = true;
    for (const auto &block : blocks) {
        const auto found = source_bindings.find(block.instance);
        if (found == source_bindings.end() || (!first && found->second <= previous))
            return {false, "compiled block/source uniform binding mismatch"};
        first = false;
        previous = found->second;
    }
    for (auto &block : blocks)
        block.source_binding = source_bindings.at(block.instance);
    return {};
}

ValidationResult read_bgfx_uniform_bindings(std::string_view manifest,
                                            std::map<std::string, UInt32> &source_bindings) {
    if (manifest.empty() || manifest.size() > 65536)
        return {false, "missing or oversized source uniform manifest"};
    const std::string text(manifest);
    const std::regex entry("\\\"instance\\\":\\\"([^\\\"]*)\\\",\\\"source_binding\\\":([0-9]+)");
    const std::regex identifier("[A-Za-z_][A-Za-z_0-9]*");
    std::map<std::string, UInt32> candidate;
    std::size_t entries = 0, declared = 0;
    for (std::size_t at = 0; (at = text.find("\"instance\"", at)) != std::string::npos; at += 10)
        ++declared;
    for (std::sregex_iterator it(text.begin(), text.end(), entry), end; it != end; ++it) {
        ++entries;
        const std::string instance = (*it)[1], binding = (*it)[2];
        if (instance.size() > 255 || !std::regex_match(instance, identifier) || binding.size() != 1 ||
            binding[0] > '3')
            return {false, "malformed source uniform manifest block"};
        if (!candidate.emplace(instance, static_cast<UInt32>(binding[0] - '0')).second)
            return {false, "duplicate source uniform manifest block"};
    }
    if (candidate.size() > 4 || entries != declared)
        return {false, "malformed or oversized source uniform manifest"};
    source_bindings.swap(candidate);
    return {};
}

ValidationResult
validate_bgfx_uniform_layout_sidecar(std::string_view sidecar, ShaderStage stage,
                                     const std::vector<BgfxUniformBlockLayout> &blocks) {
    if (sidecar.empty() || sidecar.size() > 4096 || sidecar != encode_bgfx_uniform_layout(stage, blocks))
        return {false, "missing, malformed or stale bgfx uniform layout metadata"};
    return {};
}
} // namespace zh::renderer
