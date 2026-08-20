#include "SquadTracker.h"

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    std::mutex g_SquadTrackerMutex;

    std::unordered_map<
        uintptr_t,
        SquadTrackedPlayer
    > g_Players;

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
}

void SquadTracker::ProcessEvent(
    const EvCombatData* combatData
)
{
    if (combatData == nullptr ||
        combatData->ev != nullptr ||
        combatData->src == nullptr)
    {
        return;
    }

    //
    // ArcDPS targeted-agent notifications are not
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
    // Tracking add:
    //
    // src->ID         = ArcDPS player agent ID
    // src->Name       = character name
    // src->Team       = team
    //
    // dst->ID         = instance ID
    // dst->Name       = account name
    // dst->Profession = profession
    // dst->Elite      = elite specialization
    // dst->IsSelf     = whether this is the local player
    // dst->Team       = subgroup
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

        SquadTrackedPlayer player;

        player.agentID =
            agentID;

        player.characterName =
            ReadName(
                combatData->src->Name
            );

        player.accountName =
            NormalizeAccountName(
                combatData->dst->Name
            );

        player.instanceID =
            combatData->dst->ID;

        player.profession =
            combatData->dst->Profession;

        player.elite =
            combatData->dst->Elite;

        player.isSelf =
            combatData->dst->IsSelf != 0;

        player.team =
            combatData->src->Team;

        player.subgroup =
            combatData->dst->Team;

        g_Players[
            agentID
        ] = player;

        return;
    }

    //
    // Tracking remove.
    //
    const uintptr_t agentID =
        combatData->src->ID;

    if (agentID != 0)
    {
        g_Players.erase(
            agentID
        );
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
        const auto& entry :
        g_Players
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

void SquadTracker::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_SquadTrackerMutex
    );

    g_Players.clear();
}
