#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ArcDPS.h"

struct SquadTrackedPlayer
{
    std::string characterName;
    std::string accountName;

    uintptr_t agentID = 0;
    uintptr_t instanceID = 0;

    uint32_t profession = 0;
    uint32_t elite = 0;

    uint16_t subgroup = 0;
    uint16_t team = 0;

    bool isSelf = false;

    bool foodStateKnown = false;
    bool utilityStateKnown = false;

    bool hasFood = false;
    bool hasUtility = false;

    uint32_t foodSkillID = 0;
    uint32_t utilitySkillID = 0;

    int64_t foodRemainingMilliseconds = 0;
    int64_t utilityRemainingMilliseconds = 0;
};

namespace SquadTracker
{
    void ProcessEvent(
        const EvCombatData* combatData
    );

    std::vector<SquadTrackedPlayer>
        GetPlayers();

    void Reset();
}
