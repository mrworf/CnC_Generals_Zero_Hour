#pragma once

#include "zh/world/scene.h"

#include <string>
#include <string_view>
#include <vector>

namespace zh::world {

struct WorldCorpusCoverage {
    std::vector<std::string> maps;
    std::vector<Faction> factions;
    std::vector<GeometryFamily> families;
    std::vector<std::string> states;
};

renderer::ValidationResult parse_world_corpus(std::string_view text, WorldCorpusCoverage& output);

} // namespace zh::world
