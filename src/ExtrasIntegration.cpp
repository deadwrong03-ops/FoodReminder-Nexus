#include "ExtrasIntegration.h"
#include "Definitions.h"

#include <Windows.h>

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

namespace
{
    std::mutex g_ExtrasMutex;

    bool g_ExtrasAvailable = false;

    std::string g_ExtrasVersion;

    std::vector<ExtrasSquadMember>
        g_SquadMembers;

    std::string NormalizeAccountName(
        const char* accountName
    )
    {
        if (accountName == nullptr)
        {
            return "";
        }

        std::string result =
            accountName;

        //
        // Unofficial Extras account names include
        // a leading ':'.
        //
        // Keep our internal copy cleaner for display.
        //
        if (!result.empty() &&
            result.front() == ':')
        {
            result.erase(
                result.begin()
            );
        }

        return result;
    }

    void OnSquadUpdate(
        const UserInfo* updatedUsers,
        uint64_t updatedUsersCount
    )
    {
        if (updatedUsers == nullptr)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(
            g_ExtrasMutex
        );

        for (
            uint64_t i = 0;
            i < updatedUsersCount;
            ++i
            )
        {
            const UserInfo& user =
                updatedUsers[i];

            const std::string accountName =
                NormalizeAccountName(
                    user.AccountName
                );

            if (accountName.empty())
            {
                continue;
            }

            const auto existing =
                std::find_if(
                    g_SquadMembers.begin(),
                    g_SquadMembers.end(),
                    [&accountName](
                        const ExtrasSquadMember& member
                        )
                    {
                        return
                            member.accountName ==
                            accountName;
                    }
                );

            //
            // Extras reports Role::None when someone
            // leaves the party or squad.
            //
            if (user.Role ==
                UserRole::None)
            {
                if (existing !=
                    g_SquadMembers.end())
                {
                    g_SquadMembers.erase(
                        existing
                    );
                }

                continue;
            }

            ExtrasSquadMember member;

            member.accountName =
                accountName;

            member.subgroup =
                user.Subgroup;

            member.ready =
                user.ReadyStatus;

            member.isCommander =
                user.Role ==
                UserRole::SquadLeader;

            member.isLieutenant =
                user.Role ==
                UserRole::Lieutenant;

            if (existing !=
                g_SquadMembers.end())
            {
                *existing =
                    member;
            }
            else
            {
                g_SquadMembers.push_back(
                    member
                );
            }
        }

        std::sort(
            g_SquadMembers.begin(),
            g_SquadMembers.end(),
            [](
                const ExtrasSquadMember& a,
                const ExtrasSquadMember& b
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
                    a.accountName <
                    b.accountName;
            }
        );
    }
}

bool ExtrasIntegration::IsAvailable()
{
    std::lock_guard<std::mutex> lock(
        g_ExtrasMutex
    );

    return g_ExtrasAvailable;
}

std::string ExtrasIntegration::GetVersion()
{
    std::lock_guard<std::mutex> lock(
        g_ExtrasMutex
    );

    return g_ExtrasVersion;
}

std::vector<ExtrasSquadMember>
ExtrasIntegration::GetSquadMembers()
{
    std::lock_guard<std::mutex> lock(
        g_ExtrasMutex
    );

    return g_SquadMembers;
}

void ExtrasIntegration::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_ExtrasMutex
    );

    g_SquadMembers.clear();

    g_ExtrasAvailable = false;
    g_ExtrasVersion.clear();
}

//
// Unofficial Extras discovers this export automatically.
//
// We only request subscriber-info version 1 because the
// SquadUpdateCallback already exists in V1. That keeps
// Food Reminder compatible with more Extras versions.
//
extern "C"
__declspec(dllexport)
void arcdps_unofficial_extras_subscriber_init(
    const ExtrasAddonInfo* extrasInfo,
    void* subscriberInfo
)
{
    if (extrasInfo == nullptr ||
        subscriberInfo == nullptr)
    {
        return;
    }

    //
    // Definitions.h documents API version 2 as the
    // current Unofficial Extras API.
    //
    if (extrasInfo->ApiVersion != 2 ||
        extrasInfo->MaxInfoVersion < 1)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(
            g_ExtrasMutex
        );

        g_ExtrasAvailable = true;

        if (extrasInfo->StringVersion != nullptr)
        {
            g_ExtrasVersion =
                extrasInfo->StringVersion;
        }
        else
        {
            g_ExtrasVersion =
                "Unknown";
        }
    }

    ExtrasSubscriberInfoV1* info =
        static_cast<ExtrasSubscriberInfoV1*>(
            subscriberInfo
            );

    info->InfoVersion = 1;

    info->SubscriberName =
        "Food Reminder";

    info->SquadUpdateCallback =
        OnSquadUpdate;

    //
    // We do not need these features.
    //
    info->LanguageChangedCallback =
        nullptr;

    info->KeyBindChangedCallback =
        nullptr;
}