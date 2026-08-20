#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ArcDPS.h"

struct BuffEventDebug
{
    uint64_t eventID = 0;

    uint32_t skillID = 0;
    std::string skillName;

    int32_t value = 0;
    int32_t buffDamage = 0;
    uint32_t overstackValue = 0;

    uint8_t buff = 0;
    uint8_t buffRemove = 0;
    uint8_t stateChange = 0;

    bool sourceIsSelf = false;
    bool destinationIsSelf = false;
};

namespace BuffTracker
{
    void Reset();

    void ProcessEvent(const EvCombatData* combatData);

    uint64_t GetTotalEventCount();
    uint64_t GetBuffLikeEventCount();

    std::vector<BuffEventDebug> GetRecentBuffEvents();

    bool HasFood();
    bool HasUtility();

    bool HasMetabolicPrimer();
    bool HasUtilityPrimer();

    int64_t GetFoodRemainingMilliseconds();
    int64_t GetUtilityRemainingMilliseconds();

    int64_t GetMetabolicPrimerRemainingMilliseconds();
    int64_t GetUtilityPrimerRemainingMilliseconds();

    void RestorePrimerState();
    void SavePrimerState();
}