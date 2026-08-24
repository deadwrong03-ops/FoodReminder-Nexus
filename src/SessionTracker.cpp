#include "SessionTracker.h"

#include <chrono>
#include <mutex>

namespace
{
    std::mutex g_SessionMutex;

    SessionStats g_Stats;

    bool g_HasLastUpdate = false;

    std::chrono::steady_clock::time_point
        g_LastUpdateTime;
}

void SessionTracker::Update(
    bool hasFood,
    bool hasUtility,
    bool inCombat
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    const auto now =
        std::chrono::steady_clock::now();

    if (!g_HasLastUpdate)
    {
        g_LastUpdateTime = now;
        g_HasLastUpdate = true;
        return;
    }

    const int64_t elapsedMilliseconds =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            now -
            g_LastUpdateTime
        ).count();

    g_LastUpdateTime = now;

    if (elapsedMilliseconds <= 0)
    {
        return;
    }

    g_Stats.sessionMilliseconds +=
        elapsedMilliseconds;

    if (hasFood)
    {
        g_Stats.foodActiveMilliseconds +=
            elapsedMilliseconds;
    }

    if (hasUtility)
    {
        g_Stats.utilityActiveMilliseconds +=
            elapsedMilliseconds;
    }

    if (inCombat)
    {
        g_Stats.combatMilliseconds +=
            elapsedMilliseconds;

        if (hasFood)
        {
            g_Stats.foodCombatMilliseconds +=
                elapsedMilliseconds;
        }

        if (hasUtility)
        {
            g_Stats.utilityCombatMilliseconds +=
                elapsedMilliseconds;
        }
    }
}

SessionStats SessionTracker::GetStats()
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    return g_Stats;
}

void SessionTracker::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    g_Stats = {};

    g_HasLastUpdate = false;

    g_LastUpdateTime = {};
}