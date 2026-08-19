#include "BuffTracker.h"

#include <mutex>
#include <vector>
#include <chrono>

namespace
{
    std::mutex g_BuffMutex;

    uint64_t g_TotalEventCount = 0;
    uint64_t g_BuffLikeEventCount = 0;

    std::vector<BuffEventDebug> g_RecentBuffEvents;

    constexpr size_t MAX_DEBUG_EVENTS = 100;
    constexpr uint32_t FOOD_BUFF_ID = 10001;
    constexpr uint32_t UTILITY_BUFF_ID = 9963;

    bool g_HasFood = false;
    bool g_HasUtility = false;

    int64_t g_FoodDurationMilliseconds = 0;
    int64_t g_UtilityDurationMilliseconds = 0;

    std::chrono::steady_clock::time_point g_FoodReceivedTime;
    std::chrono::steady_clock::time_point g_UtilityReceivedTime;
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

    if (destinationIsSelf && ev.IsStatechange == 18)
    {
        if (ev.SkillID == FOOD_BUFF_ID)
        {
            g_HasFood = true;
            g_FoodDurationMilliseconds =
                static_cast<int64_t>(ev.Value);

            g_FoodReceivedTime =
                std::chrono::steady_clock::now();
        }
        else if (ev.SkillID == UTILITY_BUFF_ID)
        {
            g_HasUtility = true;
            g_UtilityDurationMilliseconds =
                static_cast<int64_t>(ev.Value);

            g_UtilityReceivedTime =
                std::chrono::steady_clock::now();
        }
    }

    // Ignore normal condition-damage ticks.
    //
    // For this first diagnostic pass we want:
    // - buff application-looking records
    // - buff removal records
    // - ArcDPS BUFFINITIAL state records
    //
    // We are deliberately NOT deciding what is Food or Utility yet.
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
        record.skillName = combatData->skillname;
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
        g_RecentBuffEvents.erase(g_RecentBuffEvents.begin());
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
    return g_HasFood;
}

bool BuffTracker::HasUtility()
{
    std::lock_guard<std::mutex> lock(g_BuffMutex);
    return g_HasUtility;
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

    return remaining > 0 ? remaining : 0;
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

    return remaining > 0 ? remaining : 0;
}
