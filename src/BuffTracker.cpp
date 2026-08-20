#include "BuffTracker.h"
#include "Settings.h"

#include <mutex>
#include <vector>
#include <chrono>
#include <string>
#include <ctime>

namespace
{
    std::mutex g_BuffMutex;

    uint64_t g_TotalEventCount = 0;
    uint64_t g_BuffLikeEventCount = 0;

    std::vector<BuffEventDebug> g_RecentBuffEvents;

    constexpr size_t MAX_DEBUG_EVENTS = 100;

    bool g_HasFood = false;
    bool g_HasUtility = false;

    bool g_HasMetabolicPrimer = false;
    bool g_HasUtilityPrimer = false;

    int64_t g_FoodDurationMilliseconds = 0;
    int64_t g_UtilityDurationMilliseconds = 0;

    int64_t g_MetabolicPrimerDurationMilliseconds = 0;
    int64_t g_UtilityPrimerDurationMilliseconds = 0;

    std::chrono::steady_clock::time_point g_FoodReceivedTime;
    std::chrono::steady_clock::time_point g_UtilityReceivedTime;

    std::chrono::steady_clock::time_point g_MetabolicPrimerReceivedTime;
    std::chrono::steady_clock::time_point g_UtilityPrimerReceivedTime;
}

void BuffTracker::Reset()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    g_TotalEventCount = 0;
    g_BuffLikeEventCount = 0;
    g_RecentBuffEvents.clear();
}

void BuffTracker::ProcessEvent(const EvCombatData* combatData)
{
    if (combatData == nullptr || combatData->ev == nullptr)
    {
        return;
    }

    const ArcDPS::CombatEvent& ev = *combatData->ev;

    std::lock_guard<std::mutex> lock(g_BuffMutex);

    ++g_TotalEventCount;

    const bool destinationIsSelf =
        combatData->dst != nullptr &&
        combatData->dst->IsSelf != 0;

    if (destinationIsSelf)
    {
        const std::string skillName =
            combatData->skillname != nullptr
            ? combatData->skillname
            : "";

        const bool isFoodEvent =
            skillName == "Nourishment";

        const bool isUtilityEvent =
            skillName == "Enhancement";

        const bool isMetabolicPrimerEvent =
            skillName == "Metabolic Primer";

        const bool isUtilityPrimerEvent =
            skillName == "Utility Primer";

        const bool hasDuration =
            ev.Value > 0;

        const bool isRemoved =
            ev.IsBuffRemove != 0;

        if (isFoodEvent && isRemoved)
        {
            g_HasFood = false;
            g_FoodDurationMilliseconds = 0;
            return;
        }

        if (isUtilityEvent && isRemoved)
        {
            g_HasUtility = false;
            g_UtilityDurationMilliseconds = 0;
            return;
        }

        if (isMetabolicPrimerEvent && isRemoved)
        {
            g_HasMetabolicPrimer = false;
            g_MetabolicPrimerDurationMilliseconds = 0;
            g_Settings.metabolicPrimerExpiresAt = 0;
            return;
        }

        if (isUtilityPrimerEvent && isRemoved)
        {
            g_HasUtilityPrimer = false;
            g_UtilityPrimerDurationMilliseconds = 0;
            g_Settings.utilityPrimerExpiresAt = 0;
            return;
        }

        if (isFoodEvent && hasDuration)
        {
            g_HasFood = true;

            g_FoodDurationMilliseconds =
                static_cast<int64_t>(ev.Value);

            g_FoodReceivedTime =
                std::chrono::steady_clock::now();
        }
        else if (isUtilityEvent && hasDuration)
        {
            g_HasUtility = true;

            g_UtilityDurationMilliseconds =
                static_cast<int64_t>(ev.Value);

            g_UtilityReceivedTime =
                std::chrono::steady_clock::now();
        }
        else if (isMetabolicPrimerEvent && hasDuration)
        {
            g_HasMetabolicPrimer = true;

            g_MetabolicPrimerDurationMilliseconds =
                static_cast<int64_t>(ev.Value);

            g_MetabolicPrimerReceivedTime =
                std::chrono::steady_clock::now();

            const int64_t durationSeconds =
                g_MetabolicPrimerDurationMilliseconds / 1000;

            g_Settings.metabolicPrimerExpiresAt =
                static_cast<int64_t>(std::time(nullptr)) +
                durationSeconds;
        }
        else if (isUtilityPrimerEvent && hasDuration)
        {
            g_HasUtilityPrimer = true;

            g_UtilityPrimerDurationMilliseconds =
                static_cast<int64_t>(ev.Value);

            g_UtilityPrimerReceivedTime =
                std::chrono::steady_clock::now();

            const int64_t durationSeconds =
                g_UtilityPrimerDurationMilliseconds / 1000;

            g_Settings.utilityPrimerExpiresAt =
                static_cast<int64_t>(std::time(nullptr)) +
                durationSeconds;
        }
    }

    const bool isBuffLike =
        (ev.Buff != 0 && ev.BuffDamage >= 0) ||
        ev.IsBuffRemove != 0 ||
        ev.IsStatechange == 18;

    if (!isBuffLike)
    {
        return;
    }

    ++g_BuffLikeEventCount;

    BuffEventDebug record;

    record.eventID = combatData->id;
    record.skillID = ev.SkillID;

    if (combatData->skillname != nullptr)
    {
        record.skillName =
            combatData->skillname;
    }
    else
    {
        record.skillName = "Unknown";
    }

    record.value = ev.Value;
    record.buffDamage = ev.BuffDamage;
    record.overstackValue = ev.OverstackValue;

    record.buff = ev.Buff;
    record.buffRemove = ev.IsBuffRemove;
    record.stateChange = ev.IsStatechange;

    record.sourceIsSelf =
        combatData->src != nullptr &&
        combatData->src->IsSelf != 0;

    record.destinationIsSelf =
        combatData->dst != nullptr &&
        combatData->dst->IsSelf != 0;

    g_RecentBuffEvents.push_back(record);

    if (g_RecentBuffEvents.size() > MAX_DEBUG_EVENTS)
    {
        g_RecentBuffEvents.erase(
            g_RecentBuffEvents.begin()
        );
    }
}

uint64_t BuffTracker::GetTotalEventCount()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    return g_TotalEventCount;
}

uint64_t BuffTracker::GetBuffLikeEventCount()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    return g_BuffLikeEventCount;
}

std::vector<BuffEventDebug> BuffTracker::GetRecentBuffEvents()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    return g_RecentBuffEvents;
}

bool BuffTracker::HasFood()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasFood)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_FoodReceivedTime
        ).count();

    if (elapsed >= g_FoodDurationMilliseconds)
    {
        g_HasFood = false;
        g_FoodDurationMilliseconds = 0;

        return false;
    }

    return true;
}

bool BuffTracker::HasUtility()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasUtility)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_UtilityReceivedTime
        ).count();

    if (elapsed >= g_UtilityDurationMilliseconds)
    {
        g_HasUtility = false;
        g_UtilityDurationMilliseconds = 0;

        return false;
    }

    return true;
}

bool BuffTracker::HasMetabolicPrimer()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasMetabolicPrimer)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_MetabolicPrimerReceivedTime
        ).count();

    if (elapsed >= g_MetabolicPrimerDurationMilliseconds)
    {
        g_HasMetabolicPrimer = false;
        g_MetabolicPrimerDurationMilliseconds = 0;
        g_Settings.metabolicPrimerExpiresAt = 0;

        return false;
    }

    return true;
}

bool BuffTracker::HasUtilityPrimer()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasUtilityPrimer)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_UtilityPrimerReceivedTime
        ).count();

    if (elapsed >= g_UtilityPrimerDurationMilliseconds)
    {
        g_HasUtilityPrimer = false;
        g_UtilityPrimerDurationMilliseconds = 0;
        g_Settings.utilityPrimerExpiresAt = 0;

        return false;
    }

    return true;
}

int64_t BuffTracker::GetFoodRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasFood)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_FoodReceivedTime
        ).count();

    const int64_t remaining =
        g_FoodDurationMilliseconds - elapsed;

    return remaining > 0
        ? remaining
        : 0;
}

int64_t BuffTracker::GetUtilityRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasUtility)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_UtilityReceivedTime
        ).count();

    const int64_t remaining =
        g_UtilityDurationMilliseconds - elapsed;

    return remaining > 0
        ? remaining
        : 0;
}

int64_t BuffTracker::GetMetabolicPrimerRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasMetabolicPrimer)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_MetabolicPrimerReceivedTime
        ).count();

    const int64_t remaining =
        g_MetabolicPrimerDurationMilliseconds - elapsed;

    return remaining > 0
        ? remaining
        : 0;
}

int64_t BuffTracker::GetUtilityPrimerRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    if (!g_HasUtilityPrimer)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() -
            g_UtilityPrimerReceivedTime
        ).count();

    const int64_t remaining =
        g_UtilityPrimerDurationMilliseconds - elapsed;

    return remaining > 0
        ? remaining
        : 0;
}

void BuffTracker::RestorePrimerState()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);

    const int64_t now =
        static_cast<int64_t>(
            std::time(nullptr)
            );

    if (g_Settings.metabolicPrimerExpiresAt > now)
    {
        const int64_t remainingSeconds =
            g_Settings.metabolicPrimerExpiresAt - now;

        g_HasMetabolicPrimer = true;

        g_MetabolicPrimerDurationMilliseconds =
            remainingSeconds * 1000;

        g_MetabolicPrimerReceivedTime =
            std::chrono::steady_clock::now();
    }
    else
    {
        g_HasMetabolicPrimer = false;
        g_MetabolicPrimerDurationMilliseconds = 0;
        g_Settings.metabolicPrimerExpiresAt = 0;
    }

    if (g_Settings.utilityPrimerExpiresAt > now)
    {
        const int64_t remainingSeconds =
            g_Settings.utilityPrimerExpiresAt - now;

        g_HasUtilityPrimer = true;

        g_UtilityPrimerDurationMilliseconds =
            remainingSeconds * 1000;

        g_UtilityPrimerReceivedTime =
            std::chrono::steady_clock::now();
    }
    else
    {
        g_HasUtilityPrimer = false;
        g_UtilityPrimerDurationMilliseconds = 0;
        g_Settings.utilityPrimerExpiresAt = 0;
    }
}
void BuffTracker::SavePrimerState()
{
    // Primer expiration timestamps are stored in g_Settings.
    // Settings.cpp handles writing them to disk.
}