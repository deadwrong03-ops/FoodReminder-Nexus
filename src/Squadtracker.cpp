#include "SquadTracker.h"
#include "ConsumableData.h"
#include "Settings.h"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    struct SquadTrackedPlayerState
    {
        SquadTrackedPlayer player;

        int64_t foodDurationMilliseconds = 0;
        int64_t utilityDurationMilliseconds = 0;

        std::chrono::steady_clock::time_point
            foodReceivedTime;

        std::chrono::steady_clock::time_point
            utilityReceivedTime;
    };

    std::mutex g_SquadTrackerMutex;

    std::unordered_map<
        uintptr_t,
        SquadTrackedPlayerState
    > g_Players;
    std::unordered_map<
        uint64_t,
        UnknownConsumable
    > g_UnknownConsumables;

    uint64_t MakeUnknownConsumableKey(
        uint32_t skillID,
        bool isUtility
    )
    {
        return
            (static_cast<uint64_t>(
                isUtility ? 1 : 0
                ) << 32) |
            static_cast<uint64_t>(
                skillID
                );
    }

    void RecordUnknownConsumable(
        uint32_t skillID,
        bool isFood,
        bool isUtility
    )
    {
        const ConsumableInfo& info =
            isFood
            ? ConsumableData::GetFoodInfo(
                skillID
            )
            : ConsumableData::GetUtilityInfo(
                skillID
            );

        const std::string label =
            info.label != nullptr
            ? info.label
            : "";

        if (label != "Unknown")
        {
            return;
        }

        const uint64_t key =
            MakeUnknownConsumableKey(
                skillID,
                isUtility
            );

        UnknownConsumable& unknown =
            g_UnknownConsumables[
                key
            ];

        unknown.skillID =
            skillID;

        unknown.isFood =
            isFood;

        unknown.isUtility =
            isUtility;

        ++unknown.seenCount;
    }

    std::string ReadName(
        const char* value
    )
    {
        if (value == nullptr)
        {
            return "";
        }

        return value;
    }

    std::string NormalizeAccountName(
        const char* value
    )
    {
        std::string result =
            ReadName(
                value
            );

        if (!result.empty() &&
            result.front() == ':')
        {
            result.erase(
                result.begin()
            );
        }

        return result;
    }

    bool IsIgnoredSquadBuff(
        uint32_t skillID
    )
    {
        switch (skillID)
        {
        case 10110:
        case 10104:
        case 64528:
        case 32289:
        case 32293:
        case 33046:
        case 65475:
            return true;

        default:
            return false;
        }
    }

    void MarkUnknownStatesAsNone()
    {
        for (
            auto& entry :
            g_Players
            )
        {
            SquadTrackedPlayerState& state =
                entry.second;

            if (!state.player.foodStateKnown)
            {
                state.player.foodStateKnown = true;
                state.player.hasFood = false;
            }

            if (!state.player.utilityStateKnown)
            {
                state.player.utilityStateKnown = true;
                state.player.hasUtility = false;
            }
        }
    }

    int64_t GetRemainingMilliseconds(
        bool hasBuff,
        int64_t durationMilliseconds,
        const std::chrono::steady_clock::time_point&
        receivedTime
    )
    {
        if (!hasBuff ||
            durationMilliseconds <= 0)
        {
            return 0;
        }

        const int64_t elapsed =
            std::chrono::duration_cast<
            std::chrono::milliseconds
            >(
                std::chrono::steady_clock::now() -
                receivedTime
            ).count();

        const int64_t remaining =
            durationMilliseconds -
            elapsed;

        return
            remaining > 0
            ? remaining
            : 0;
    }

    void ClearFood(
        SquadTrackedPlayerState& state
    )
    {
        state.player.foodStateKnown = true;
        state.player.hasFood = false;
        state.player.foodSkillID = 0;
        state.player.foodRemainingMilliseconds = 0;

        state.foodDurationMilliseconds = 0;
    }

    void ClearUtility(
        SquadTrackedPlayerState& state
    )
    {
        state.player.utilityStateKnown = true;
        state.player.hasUtility = false;
        state.player.utilitySkillID = 0;
        state.player.utilityRemainingMilliseconds = 0;

        state.utilityDurationMilliseconds = 0;
    }
}

void SquadTracker::ProcessEvent(
    const EvCombatData* combatData
)
{
    if (combatData == nullptr ||
        combatData->src == nullptr)
    {
        return;
    }

    //
    // ArcDPS tracking-change notifications.
    //
    if (combatData->ev == nullptr)
    {
        //
        // Targeted-agent notifications are not
        // player tracking changes.
        //
        if (combatData->src->Elite == 1)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(
            g_SquadTrackerMutex
        );

        //
        // Player added to ArcDPS tracking.
        //
        if (combatData->src->Profession != 0)
        {
            if (combatData->dst == nullptr)
            {
                return;
            }

            const uintptr_t agentID =
                combatData->src->ID;

            if (agentID == 0)
            {
                return;
            }

            SquadTrackedPlayerState state;

            //
            // Preserve already-known Food/Utility
            // if ArcDPS repeats the same tracking add.
            //
            const auto existing =
                g_Players.find(
                    agentID
                );

            if (existing !=
                g_Players.end())
            {
                state =
                    existing->second;
            }

            state.player.agentID =
                agentID;

            state.player.characterName =
                ReadName(
                    combatData->src->Name
                );

            state.player.accountName =
                NormalizeAccountName(
                    combatData->dst->Name
                );

            state.player.instanceID =
                combatData->dst->ID;

            state.player.profession =
                combatData->dst->Profession;

            state.player.elite =
                combatData->dst->Elite;

            state.player.isSelf =
                combatData->dst->IsSelf != 0;

            state.player.team =
                combatData->src->Team;

            state.player.subgroup =
                combatData->dst->Team;

            g_Players[
                agentID
            ] = state;

            return;
        }

        //
        // Player removed from ArcDPS tracking.
        //
        const uintptr_t agentID =
            combatData->src->ID;

        if (agentID != 0)
        {
            g_Players.erase(
                agentID
            );
        }

        return;
    }

    const ArcDPS::CombatEvent& ev =
        *combatData->ev;

    //
    // Ignore known proc/secondary effects that should
    // not replace the player's actual consumable state.
    //
    if (IsIgnoredSquadBuff(
        ev.SkillID))
    {
        return;
    }

    const std::string skillName =
        combatData->skillname != nullptr
        ? combatData->skillname
        : "";

    const bool isFood =
        skillName == "Nourishment";

    const bool isUtility =
        skillName == "Enhancement";

    if (!isFood &&
        !isUtility)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    //
    // ArcDPS sends BuffInitial records as the combat-start
    // snapshot. Once that snapshot begins, any player state
    // that is still unknown can safely become known-none.
    // A following Food/Utility BuffInitial record will then
    // overwrite the appropriate state with the active buff.
    //
    constexpr uint8_t STATECHANGE_BUFF_INITIAL = 18;

    if (ev.IsStatechange ==
        STATECHANGE_BUFF_INITIAL)
    {
        MarkUnknownStatesAsNone();
    }

    //
    // ArcDPS buff removals use the source agent ID.
    //
    if (ev.IsBuffRemove != 0)
    {
        const uintptr_t playerID =
            combatData->src->ID;

        const auto existing =
            g_Players.find(
                playerID
            );

        if (existing ==
            g_Players.end())
        {
            return;
        }

        if (isFood)
        {
            ClearFood(
                existing->second
            );
        }
        else
        {
            ClearUtility(
                existing->second
            );
        }

        return;
    }

    //
    // BuffInitial (state change 18) is a real buff
    // application snapshot. ArcDPS uses BuffDamage
    // differently for these records, so do NOT reject
    // them just because BuffDamage is non-zero.
    //
    const bool isBuffInitial =
        ev.IsStatechange ==
        STATECHANGE_BUFF_INITIAL;

    //
    // Ignore activations and malformed events.
    //
    if (ev.IsActivation != 0 ||
        combatData->dst == nullptr)
    {
        return;
    }

    //
    // Normal buff applications have BuffDamage == 0.
    // BuffInitial is the exception: BuffDamage contains
    // the original/full stack duration.
    //
    if (ev.Buff == 0 ||
        (!isBuffInitial &&
            ev.BuffDamage != 0))
    {
        return;
    }
    RecordUnknownConsumable(
        ev.SkillID,
        isFood,
        isUtility
    );

    const uintptr_t playerID =
        combatData->dst->ID;

    const auto existing =
        g_Players.find(
            playerID
        );

    if (existing ==
        g_Players.end())
    {
        return;
    }

    SquadTrackedPlayerState& state =
        existing->second;

    if (isFood)
    {
        state.player.foodStateKnown = true;
        state.player.hasFood = true;

        state.player.foodSkillID =
            ev.SkillID;

        //
        // Value is the remaining duration for normal
        // and BuffInitial applications when available.
        // Even if ArcDPS gives no usable remaining
        // duration, keep the buff marked as present.
        //
        state.foodDurationMilliseconds =
            ev.Value > 0
            ? static_cast<int64_t>(
                ev.Value
                )
            : 0;

        state.foodReceivedTime =
            std::chrono::steady_clock::now();

        state.player.foodRemainingMilliseconds =
            state.foodDurationMilliseconds;
    }
    else
    {
        state.player.utilityStateKnown = true;
        state.player.hasUtility = true;

        state.player.utilitySkillID =
            ev.SkillID;

        state.utilityDurationMilliseconds =
            ev.Value > 0
            ? static_cast<int64_t>(
                ev.Value
                )
            : 0;

        state.utilityReceivedTime =
            std::chrono::steady_clock::now();

        state.player.utilityRemainingMilliseconds =
            state.utilityDurationMilliseconds;
    }
}

std::vector<SquadTrackedPlayer>
SquadTracker::GetPlayers()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    std::vector<SquadTrackedPlayer> result;

    result.reserve(
        g_Players.size()
    );

    for (
        auto& entry :
        g_Players
        )
    {
        SquadTrackedPlayerState& state =
            entry.second;

        state.player.foodRemainingMilliseconds =
            GetRemainingMilliseconds(
                state.player.hasFood,
                state.foodDurationMilliseconds,
                state.foodReceivedTime
            );

        if (state.player.hasFood &&
            state.foodDurationMilliseconds > 0 &&
            state.player.foodRemainingMilliseconds <= 0)
        {
            ClearFood(
                state
            );
        }

        state.player.utilityRemainingMilliseconds =
            GetRemainingMilliseconds(
                state.player.hasUtility,
                state.utilityDurationMilliseconds,
                state.utilityReceivedTime
            );

        if (state.player.hasUtility &&
            state.utilityDurationMilliseconds > 0 &&
            state.player.utilityRemainingMilliseconds <= 0)
        {
            ClearUtility(
                state
            );
        }

        result.push_back(
            state.player
        );
    }

    std::sort(
        result.begin(),
        result.end(),
        [](
            const SquadTrackedPlayer& a,
            const SquadTrackedPlayer& b
            )
        {
            if (a.subgroup !=
                b.subgroup)
            {
                return
                    a.subgroup <
                    b.subgroup;
            }

            return
                a.characterName <
                b.characterName;
        }
    );

    return result;
}
std::vector<UnknownConsumable>
SquadTracker::GetUnknownConsumables()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    std::vector<UnknownConsumable> result;

    result.reserve(
        g_UnknownConsumables.size()
    );

    for (
        const auto& entry :
        g_UnknownConsumables
        )
    {
        result.push_back(
            entry.second
        );
    }

    std::sort(
        result.begin(),
        result.end(),
        [](
            const UnknownConsumable& a,
            const UnknownConsumable& b
            )
        {
            if (a.isFood != b.isFood)
            {
                return a.isFood;
            }

            return
                a.skillID <
                b.skillID;
        }
    );

    return result;
}

void SquadTracker::ClearUnknownConsumables()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    g_UnknownConsumables.clear();
    g_Settings.unknownConsumables.clear();
}

void SquadTracker::RestoreUnknownConsumables()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    g_UnknownConsumables.clear();

    for (
        const auto& entry :
        g_Settings.unknownConsumables
        )
    {
        const SavedUnknownConsumable& saved =
            entry.second;

        const ConsumableInfo& info =
            saved.isFood
            ? ConsumableData::GetFoodInfo(
                saved.skillID
            )
            : ConsumableData::GetUtilityInfo(
                saved.skillID
            );

        const std::string label =
            info.label != nullptr
            ? info.label
            : "";

        if (label != "Unknown")
        {
            continue;
        }

        UnknownConsumable unknown;

        unknown.skillID =
            saved.skillID;

        unknown.isFood =
            saved.isFood;

        unknown.isUtility =
            saved.isUtility;

        unknown.seenCount =
            saved.seenCount;

        g_UnknownConsumables[
            entry.first
        ] = unknown;
    }
}

void SquadTracker::SaveUnknownConsumables()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    g_Settings.unknownConsumables.clear();

    for (
        const auto& entry :
        g_UnknownConsumables
        )
    {
        const UnknownConsumable& unknown =
            entry.second;

        SavedUnknownConsumable saved;

        saved.skillID =
            unknown.skillID;

        saved.isFood =
            unknown.isFood;

        saved.isUtility =
            unknown.isUtility;

        saved.seenCount =
            unknown.seenCount;

        g_Settings.unknownConsumables[
            entry.first
        ] = saved;
    }
}
void SquadTracker::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    g_Players.clear();
}
