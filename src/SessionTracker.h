#pragma once

#include <cstdint>

struct SessionStats
{
    int64_t sessionMilliseconds = 0;
    int64_t combatMilliseconds = 0;

    int64_t foodActiveMilliseconds = 0;
    int64_t utilityActiveMilliseconds = 0;

    int64_t foodCombatMilliseconds = 0;
    int64_t utilityCombatMilliseconds = 0;
};

namespace SessionTracker
{
    void Update(
        bool hasFood,
        bool hasUtility,
        bool inCombat
    );

    SessionStats GetStats();

    void Reset();
}
