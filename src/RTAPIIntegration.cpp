#include "RTAPIIntegration.h"

#include "RTAPI.h"

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    std::mutex g_RTAPIMutex;

    AddonAPI_t* g_API = nullptr;
    RealTimeData* g_RTAPI = nullptr;

    std::unordered_map<
        std::string,
        RTAPIGroupMemberSnapshot
    > g_GroupMembers;

    template <size_t N>
    std::string ReadFixedString(
        const char(&value)[N]
    )
    {
        size_t length = 0;

        while (length < N &&
            value[length] != '\0')
        {
            ++length;
        }

        return std::string(
            value,
            value + length
        );
    }

    std::string NormalizeAccountName(
        const std::string& value
    )
    {
        if (!value.empty() &&
            value.front() == ':')
        {
            return value.substr(1);
        }

        return value;
    }

    std::string MakeMemberKey(
        const std::string& accountName,
        const std::string& characterName
    )
    {
        if (!accountName.empty())
        {
            return
                "A:" +
                NormalizeAccountName(
                    accountName
                );
        }

        return
            "C:" +
            characterName;
    }

    RTAPIGroupMemberSnapshot MakeSnapshot(
        const GroupMember& member
    )
    {
        RTAPIGroupMemberSnapshot snapshot;

        snapshot.accountName =
            NormalizeAccountName(
                ReadFixedString(
                    member.AccountName
                )
            );

        snapshot.characterName =
            ReadFixedString(
                member.CharacterName
            );

        snapshot.subgroup =
            member.Subgroup;

        snapshot.profession =
            member.Profession;

        snapshot.eliteSpecialization =
            member.EliteSpecialization;

        snapshot.isSelf =
            member.IsSelf != 0;

        snapshot.isInInstance =
            member.IsInInstance != 0;

        snapshot.isCommander =
            member.IsCommander != 0;

        snapshot.isLieutenant =
            member.IsLieutenant != 0;

        return snapshot;
    }

    void UpsertGroupMember(
        const GroupMember* member
    )
    {
        if (member == nullptr)
        {
            return;
        }

        const RTAPIGroupMemberSnapshot snapshot =
            MakeSnapshot(
                *member
            );

        const std::string key =
            MakeMemberKey(
                snapshot.accountName,
                snapshot.characterName
            );

        if (key == "C:")
        {
            return;
        }

        std::lock_guard<std::mutex> lock(
            g_RTAPIMutex
        );

        //
        // If an update changes character name while keeping the same
        // account, remove any stale record before inserting the new one.
        //
        for (
            auto it = g_GroupMembers.begin();
            it != g_GroupMembers.end();
            )
        {
            const bool sameAccount =
                !snapshot.accountName.empty() &&
                NormalizeAccountName(
                    it->second.accountName
                ) ==
                snapshot.accountName;

            const bool sameCharacter =
                !snapshot.characterName.empty() &&
                it->second.characterName ==
                snapshot.characterName;

            if (sameAccount ||
                sameCharacter)
            {
                it =
                    g_GroupMembers.erase(
                        it
                    );
            }
            else
            {
                ++it;
            }
        }

        g_GroupMembers[
            key
        ] = snapshot;
    }

    void RemoveGroupMember(
        const GroupMember* member
    )
    {
        if (member == nullptr)
        {
            return;
        }

        const RTAPIGroupMemberSnapshot snapshot =
            MakeSnapshot(
                *member
            );

        std::lock_guard<std::mutex> lock(
            g_RTAPIMutex
        );

        for (
            auto it = g_GroupMembers.begin();
            it != g_GroupMembers.end();
            )
        {
            const bool sameAccount =
                !snapshot.accountName.empty() &&
                NormalizeAccountName(
                    it->second.accountName
                ) ==
                snapshot.accountName;

            const bool sameCharacter =
                !snapshot.characterName.empty() &&
                it->second.characterName ==
                snapshot.characterName;

            if (sameAccount ||
                sameCharacter)
            {
                it =
                    g_GroupMembers.erase(
                        it
                    );
            }
            else
            {
                ++it;
            }
        }
    }

    void OnGroupMemberJoined(
        void* eventArgs
    )
    {
        UpsertGroupMember(
            static_cast<GroupMember*>(
                eventArgs
                )
        );
    }

    void OnGroupMemberUpdated(
        void* eventArgs
    )
    {
        UpsertGroupMember(
            static_cast<GroupMember*>(
                eventArgs
                )
        );
    }

    void OnGroupMemberLeft(
        void* eventArgs
    )
    {
        RemoveGroupMember(
            static_cast<GroupMember*>(
                eventArgs
                )
        );
    }
}

void RTAPIIntegration::Start(
    AddonAPI_t* api
)
{
    {
        std::lock_guard<std::mutex> lock(
            g_RTAPIMutex
        );

        g_API = api;
        g_RTAPI = nullptr;
        g_GroupMembers.clear();
    }

    if (api == nullptr)
    {
        return;
    }

    api->Events_Subscribe(
        EV_RTAPI_GROUP_MEMBER_JOINED,
        OnGroupMemberJoined
    );

    api->Events_Subscribe(
        EV_RTAPI_GROUP_MEMBER_LEFT,
        OnGroupMemberLeft
    );

    api->Events_Subscribe(
        EV_RTAPI_GROUP_MEMBER_UPDATED,
        OnGroupMemberUpdated
    );

    Update();
}

void RTAPIIntegration::Shutdown()
{
    AddonAPI_t* api = nullptr;

    {
        std::lock_guard<std::mutex> lock(
            g_RTAPIMutex
        );

        api = g_API;
    }

    if (api != nullptr)
    {
        api->Events_Unsubscribe(
            EV_RTAPI_GROUP_MEMBER_JOINED,
            OnGroupMemberJoined
        );

        api->Events_Unsubscribe(
            EV_RTAPI_GROUP_MEMBER_LEFT,
            OnGroupMemberLeft
        );

        api->Events_Unsubscribe(
            EV_RTAPI_GROUP_MEMBER_UPDATED,
            OnGroupMemberUpdated
        );
    }

    std::lock_guard<std::mutex> lock(
        g_RTAPIMutex
    );

    g_GroupMembers.clear();
    g_RTAPI = nullptr;
    g_API = nullptr;
}

void RTAPIIntegration::Update()
{
    AddonAPI_t* api = nullptr;

    {
        std::lock_guard<std::mutex> lock(
            g_RTAPIMutex
        );

        api = g_API;

        if (g_RTAPI != nullptr)
        {
            if (g_RTAPI->GameBuild != 0)
            {
                return;
            }

            // RTAPI was hot-unloaded.
            g_RTAPI = nullptr;
            g_GroupMembers.clear();
        }
    }

    if (api == nullptr)
    {
        return;
    }

    RealTimeData* candidate =
        static_cast<RealTimeData*>(
            api->DataLink_Get(
                DL_RTAPI
            )
            );

    std::lock_guard<std::mutex> lock(
        g_RTAPIMutex
    );

    if (candidate != nullptr &&
        candidate->GameBuild != 0)
    {
        g_RTAPI = candidate;
    }
}

bool RTAPIIntegration::IsAvailable()
{
    Update();

    std::lock_guard<std::mutex> lock(
        g_RTAPIMutex
    );

    return
        g_RTAPI != nullptr &&
        g_RTAPI->GameBuild != 0;
}

bool RTAPIIntegration::HasAuthoritativeRoster()
{
    Update();

    std::lock_guard<std::mutex> lock(
        g_RTAPIMutex
    );

    if (g_RTAPI == nullptr ||
        g_RTAPI->GameBuild == 0)
    {
        return false;
    }

    const uint32_t expectedCount =
        g_RTAPI->GroupMemberCount;

    if (expectedCount == 0)
    {
        return true;
    }

    return
        g_GroupMembers.size() ==
        static_cast<size_t>(
            expectedCount
            );
}

uint32_t RTAPIIntegration::
GetExpectedGroupMemberCount()
{
    Update();

    std::lock_guard<std::mutex> lock(
        g_RTAPIMutex
    );

    if (g_RTAPI == nullptr ||
        g_RTAPI->GameBuild == 0)
    {
        return 0;
    }

    return
        g_RTAPI->GroupMemberCount;
}

std::vector<RTAPIGroupMemberSnapshot>
RTAPIIntegration::GetGroupMembers()
{
    Update();

    std::lock_guard<std::mutex> lock(
        g_RTAPIMutex
    );

    std::vector<
        RTAPIGroupMemberSnapshot
    > result;

    result.reserve(
        g_GroupMembers.size()
    );

    for (
        const auto& entry :
        g_GroupMembers
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
            const RTAPIGroupMemberSnapshot& a,
            const RTAPIGroupMemberSnapshot& b
            )
        {
            if (a.subgroup !=
                b.subgroup)
            {
                return
                    a.subgroup <
                    b.subgroup;
            }

            if (a.isSelf != b.isSelf)
            {
                return a.isSelf;
            }

            return
                a.characterName <
                b.characterName;
        }
    );

    return result;
}
