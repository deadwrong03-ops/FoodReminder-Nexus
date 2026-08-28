#include "SessionTracker.h"

#include <chrono>
#include <mutex>

namespace
{
    std::mutex g_SessionMutex;

    SessionStats g_Stats;

    uint32_t g_LastFoodSkillID = 0;
    uint32_t g_LastUtilitySkillID = 0;

    void IncrementUsage(
        std::vector<SessionConsumableUsage>& usageList,
        uint32_t skillID
    )
    {
        for (
            SessionConsumableUsage& entry :
            usageList
            )
        {
            if (entry.skillID == skillID)
            {
                ++entry.uses;
                return;
            }
        }

        SessionConsumableUsage newEntry;

        newEntry.skillID =
            skillID;

        newEntry.uses =
            1;

        usageList.push_back(
            newEntry
        );
    }

    void AddPrimerProtectedTime(
        std::vector<SessionPrimerProtectedUsage>& usageList,
        uint32_t skillID,
        int64_t elapsedMilliseconds
    )
    {
        if (
            skillID == 0 ||
            elapsedMilliseconds <= 0
            )
        {
            return;
        }

        for (
            SessionPrimerProtectedUsage& usage :
            usageList
            )
        {
            if (usage.skillID == skillID)
            {
                usage.protectedMilliseconds +=
                    elapsedMilliseconds;

                return;
            }
        }

        SessionPrimerProtectedUsage newUsage;

        newUsage.skillID =
            skillID;

        newUsage.protectedMilliseconds =
            elapsedMilliseconds;

        usageList.push_back(
            newUsage
        );
    }

    void AddHistoryEvent(
        uint32_t skillID,
        uint32_t previousSkillID,
        int64_t previousRemainingMilliseconds,
        bool isFood,
        bool inCombat,
        SessionConsumableEventType type
    )
    {
        SessionConsumableEvent event;

        event.sessionMilliseconds =
            g_Stats.sessionMilliseconds;

        event.skillID =
            skillID;

        event.previousSkillID =
            previousSkillID;

        event.previousRemainingMilliseconds =
            previousRemainingMilliseconds;

        event.isFood =
            isFood;

        event.inCombat =
            inCombat;

        event.type =
            type;

        g_Stats.consumableHistory.push_back(
            event
        );
    }

    void TrackMetabolicPrimerState(
        SessionPrimerState state,
        bool hasFood,
        uint32_t foodSkillID,
        int64_t elapsedMilliseconds
    )
    {
        switch (state)
        {
        case SessionPrimerState::ConfirmedActive:
            g_Stats.metabolicPrimerConfirmedMilliseconds +=
                elapsedMilliseconds;

            g_Stats.metabolicPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasFood)
            {
                AddPrimerProtectedTime(
                    g_Stats.foodPrimerProtection,
                    foodSkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::InferredActive:
            g_Stats.metabolicPrimerInferredMilliseconds +=
                elapsedMilliseconds;

            g_Stats.metabolicPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasFood)
            {
                AddPrimerProtectedTime(
                    g_Stats.foodPrimerProtection,
                    foodSkillID,
                    elapsedMilliseconds
                );

                AddPrimerProtectedTime(
                    g_Stats.foodPrimerInferredProtection,
                    foodSkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::Unknown:
            g_Stats.metabolicPrimerUnknownMilliseconds +=
                elapsedMilliseconds;
            break;

        case SessionPrimerState::Inactive:
        default:
            break;
        }
    }

    void TrackUtilityPrimerState(
        SessionPrimerState state,
        bool hasUtility,
        uint32_t utilitySkillID,
        int64_t elapsedMilliseconds
    )
    {
        switch (state)
        {
        case SessionPrimerState::ConfirmedActive:
            g_Stats.utilityPrimerConfirmedMilliseconds +=
                elapsedMilliseconds;

            g_Stats.utilityPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasUtility)
            {
                AddPrimerProtectedTime(
                    g_Stats.utilityPrimerProtection,
                    utilitySkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::InferredActive:
            g_Stats.utilityPrimerInferredMilliseconds +=
                elapsedMilliseconds;

            g_Stats.utilityPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasUtility)
            {
                AddPrimerProtectedTime(
                    g_Stats.utilityPrimerProtection,
                    utilitySkillID,
                    elapsedMilliseconds
                );

                AddPrimerProtectedTime(
                    g_Stats.utilityPrimerInferredProtection,
                    utilitySkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::Unknown:
            g_Stats.utilityPrimerUnknownMilliseconds +=
                elapsedMilliseconds;
            break;

        case SessionPrimerState::Inactive:
        default:
            break;
        }
    }

    bool g_HasLastUpdate = false;

    std::chrono::steady_clock::time_point
        g_LastUpdateTime;
}

void SessionTracker::Update(
    bool hasFood,
    uint32_t foodSkillID,
    bool hasUtility,
    uint32_t utilitySkillID,
    SessionPrimerState metabolicPrimerState,
    SessionPrimerState utilityPrimerState,
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

    TrackMetabolicPrimerState(
        metabolicPrimerState,
        hasFood,
        foodSkillID,
        elapsedMilliseconds
    );

    TrackUtilityPrimerState(
        utilityPrimerState,
        hasUtility,
        utilitySkillID,
        elapsedMilliseconds
    );

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

void SessionTracker::RecordFoodApplication(
    uint32_t skillID,
    int64_t previousRemainingMilliseconds
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.foodApplications;

    IncrementUsage(
        g_Stats.foodUsage,
        skillID
    );

    if (g_LastFoodSkillID == 0)
    {
        AddHistoryEvent(
            skillID,
            0,
            0,
            true,
            false,
            SessionConsumableEventType::Applied
        );
    }
    else if (g_LastFoodSkillID == skillID)
    {
        AddHistoryEvent(
            skillID,
            g_LastFoodSkillID,
            0,
            true,
            false,
            SessionConsumableEventType::Refreshed
        );
    }
    else
    {
        AddHistoryEvent(
            skillID,
            g_LastFoodSkillID,
            previousRemainingMilliseconds,
            true,
            false,
            SessionConsumableEventType::Replaced
        );
    }

    if (g_LastFoodSkillID != 0)
    {
        if (g_LastFoodSkillID == skillID)
        {
            ++g_Stats.foodRefreshes;
        }
        else
        {
            ++g_Stats.foodReplacements;

            g_Stats.foodWastedMilliseconds +=
                previousRemainingMilliseconds;

            if (
                previousRemainingMilliseconds >
                g_Stats.worstFoodWasteMilliseconds
                )
            {
                g_Stats.worstFoodWasteMilliseconds =
                    previousRemainingMilliseconds;

                g_Stats.worstFoodWasteSkillID =
                    g_LastFoodSkillID;
            }
        }
    }

    g_LastFoodSkillID =
        skillID;
}

void SessionTracker::RecordUtilityApplication(
    uint32_t skillID,
    int64_t previousRemainingMilliseconds
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.utilityApplications;

    IncrementUsage(
        g_Stats.utilityUsage,
        skillID
    );

    if (g_LastUtilitySkillID == 0)
    {
        AddHistoryEvent(
            skillID,
            0,
            0,
            false,
            false,
            SessionConsumableEventType::Applied
        );
    }
    else if (g_LastUtilitySkillID == skillID)
    {
        AddHistoryEvent(
            skillID,
            g_LastUtilitySkillID,
            0,
            false,
            false,
            SessionConsumableEventType::Refreshed
        );
    }
    else
    {
        AddHistoryEvent(
            skillID,
            g_LastUtilitySkillID,
            previousRemainingMilliseconds,
            false,
            false,
            SessionConsumableEventType::Replaced
        );
    }

    if (g_LastUtilitySkillID != 0)
    {
        if (g_LastUtilitySkillID == skillID)
        {
            ++g_Stats.utilityRefreshes;
        }
        else
        {
            ++g_Stats.utilityReplacements;

            g_Stats.utilityWastedMilliseconds +=
                previousRemainingMilliseconds;

            if (
                previousRemainingMilliseconds >
                g_Stats.worstUtilityWasteMilliseconds
                )
            {
                g_Stats.worstUtilityWasteMilliseconds =
                    previousRemainingMilliseconds;

                g_Stats.worstUtilityWasteSkillID =
                    g_LastUtilitySkillID;
            }
        }
    }

    g_LastUtilitySkillID =
        skillID;
}

void SessionTracker::RecordFoodExpired(
    bool inCombat
)
{
    if (!inCombat)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.foodExpiredInCombat;

    AddHistoryEvent(
        g_LastFoodSkillID,
        g_LastFoodSkillID,
        0,
        true,
        inCombat,
        SessionConsumableEventType::Expired
    );
}

void SessionTracker::RecordUtilityExpired(
    bool inCombat
)
{
    if (!inCombat)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.utilityExpiredInCombat;

    AddHistoryEvent(
        g_LastUtilitySkillID,
        g_LastUtilitySkillID,
        0,
        false,
        inCombat,
        SessionConsumableEventType::Expired
    );
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

    g_LastFoodSkillID = 0;
    g_LastUtilitySkillID = 0;

    g_HasLastUpdate = false;

    g_LastUpdateTime = {};
}
