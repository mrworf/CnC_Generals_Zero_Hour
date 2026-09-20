#include "zh/world/corpus.h"

#include <algorithm>
#include <array>
#include <set>
#include <sstream>
#include <string>
#include <utility>

namespace zh::world {
namespace {

constexpr std::array<std::string_view, 13> required_states{{
    "camera", "vertex-stream", "index-stream", "transform", "lighting", "fog", "alpha-test",
    "texture-stages", "addressing", "cull", "depth", "color-mask", "render-target-dependency",
}};

renderer::ValidationResult failure(std::string message) { return {false, std::move(message)}; }

bool parse_faction(std::string_view value, Faction& faction)
{
    if (value == "usa") faction = Faction::usa;
    else if (value == "china") faction = Faction::china;
    else if (value == "gla") faction = Faction::gla;
    else return false;
    return true;
}

bool parse_family(std::string_view value, GeometryFamily& family)
{
    for (unsigned index = 0; index <= static_cast<unsigned>(GeometryFamily::selection_marker); ++index) {
        const auto candidate = static_cast<GeometryFamily>(index);
        if (geometry_family_name(candidate) == value) { family = candidate; return true; }
    }
    return false;
}

template <typename Value>
bool contains(const std::vector<Value>& values, const Value& value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

renderer::ValidationResult parse_world_corpus(std::string_view text, WorldCorpusCoverage& output)
{
    WorldCorpusCoverage parsed;
    std::set<std::pair<std::string, std::string>> seen;
    std::istringstream stream{std::string(text)};
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;
        const auto first = line.find('\t');
        const auto second = first == std::string::npos ? std::string::npos : line.find('\t', first + 1);
        if (first == std::string::npos || second == std::string::npos || second + 1 >= line.size()
            || line.find('\t', second + 1) != std::string::npos)
            return failure("world corpus line " + std::to_string(line_number) + " must have kind, value, and evidence");
        const auto kind = line.substr(0, first);
        const auto value = line.substr(first + 1, second - first - 1);
        if (kind.empty() || value.empty()) return failure("world corpus line " + std::to_string(line_number) + " has an empty field");
        if (!seen.emplace(kind, value).second)
            return failure("world corpus line " + std::to_string(line_number) + " duplicates " + kind + "/" + value);

        if (kind == "map") parsed.maps.push_back(value);
        else if (kind == "faction") {
            Faction faction;
            if (!parse_faction(value, faction)) return failure("world corpus line " + std::to_string(line_number) + " has unmapped faction " + value);
            parsed.factions.push_back(faction);
        } else if (kind == "family") {
            GeometryFamily family;
            if (!parse_family(value, family)) return failure("world corpus line " + std::to_string(line_number) + " has unmapped family " + value);
            parsed.families.push_back(family);
        } else if (kind == "state") {
            if (std::find(required_states.begin(), required_states.end(), value) == required_states.end())
                return failure("world corpus line " + std::to_string(line_number) + " has unmapped state " + value);
            parsed.states.push_back(value);
        } else {
            return failure("world corpus line " + std::to_string(line_number) + " has unmapped kind " + kind);
        }
    }

    if (parsed.maps.empty()) return failure("world corpus is missing a representative map");
    for (auto faction : {Faction::usa, Faction::china, Faction::gla})
        if (!contains(parsed.factions, faction)) return failure("world corpus is missing faction " + std::string(faction_name(faction)));
    for (unsigned index = 0; index <= static_cast<unsigned>(GeometryFamily::selection_marker); ++index) {
        const auto family = static_cast<GeometryFamily>(index);
        if (!contains(parsed.families, family)) return failure("world corpus is missing family " + std::string(geometry_family_name(family)));
    }
    for (auto state : required_states)
        if (std::find(parsed.states.begin(), parsed.states.end(), state) == parsed.states.end())
            return failure("world corpus is missing state " + std::string(state));
    output = std::move(parsed);
    return {};
}

} // namespace zh::world
