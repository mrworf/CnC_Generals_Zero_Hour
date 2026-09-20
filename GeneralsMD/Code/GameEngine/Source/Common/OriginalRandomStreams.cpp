/*
** Command & Conquer Generals Zero Hour(tm)
** Copyright 2025 Electronic Arts Inc.
** GPL-3.0-or-later
*/

#include "zh/original_data.h"
#include "PreRTS.h"

#include "Common/AudioRandomValue.h"
#include "Common/RandomValue.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/LogicRandomValue.h"

namespace zh::original_data {

RandomCheckpoints characterize_random_streams(
    std::uint32_t seed, std::size_t logic_draws, std::size_t client_draws, std::size_t audio_draws)
{
    constexpr std::size_t maximum_draws = 1000000;
    if (logic_draws > maximum_draws || client_draws > maximum_draws || audio_draws > maximum_draws)
        throw Error("random characterization draw count exceeds limit");
    InitRandom(seed);
    RandomCheckpoints result;
    result.initial_logic_crc = GetGameLogicRandomSeedCRC();
    for (std::size_t index = 0; index < logic_draws; ++index)
        result.logic_values.push_back(GetGameLogicRandomValue(-1000000, 1000000, nullptr, 0));
    result.logic_crc_after_draws = GetGameLogicRandomSeedCRC();
    for (std::size_t index = 0; index < client_draws; ++index)
        result.client_values.push_back(GetGameClientRandomValue(-1000000, 1000000, nullptr, 0));
    for (std::size_t index = 0; index < audio_draws; ++index)
        result.audio_values.push_back(GetGameAudioRandomValue(-1000000, 1000000, nullptr, 0));
    return result;
}

const char* provider_random_identity() noexcept { return "RandomValue.cpp"; }

} // namespace zh::original_data
