#pragma once

#include <cstdint>
#include <vector>

struct SessionConsumableUsage
{
    uint32_t skillID = 0;
    uint32_t uses = 0;
};

struct SessionStats
{
    int64_t sessionMilliseconds = 0;
    int64_t combatMilliseconds = 0;

    int64_t foodActiveMilliseconds = 0;
    int64_t utilityActiveMilliseconds = 0;

    int64_t foodCombatMilliseconds = 0;
    int64_t utilityCombatMilliseconds = 0;

    uint32_t foodApplications = 0;
    uint32_t foodRefreshes = 0;
    uint32_t foodReplacements = 0;
    uint32_t foodExpiredInCombat = 0;

    uint32_t utilityApplications = 0;
    uint32_t utilityRefreshes = 0;
    uint32_t utilityReplacements = 0;
    uint32_t utilityExpiredInCombat = 0;

    std::vector<SessionConsumableUsage>
        foodUsage;

    std::vector<SessionConsumableUsage>
        utilityUsage;
};

namespace SessionTracker
{
    void Update(
        bool hasFood,
        bool hasUtility,
        bool inCombat
    );

    void RecordFoodApplication(
        uint32_t skillID
    );

    void RecordUtilityApplication(
        uint32_t skillID
    );

    void RecordFoodExpired(
        bool inCombat
    );

    void RecordUtilityExpired(
        bool inCombat
    );

    SessionStats GetStats();

    void Reset();
}