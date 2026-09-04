#include <Windows.h>
#include <string>
#include <vector>
#include "ConsumableMetadataManager.h"

#include "nexus/Nexus.h"
#include "imgui/imgui.h"
#include "TradingPostPriceManager.h"
#include "TradingPostHistoryManager.h"
#include "TradingPostItemIndexManager.h"
#include "TradingPostWatchManager.h"

#include "Settings.h"
#include "ReminderManager.h"
#include "ArcDPS.h"
#include "BuffTracker.h"
#include "ExtrasIntegration.h"
#include "SquadTracker.h"
#include "SessionTracker.h"
#include "RTAPIIntegration.h"
#include "HistoryUI.h"
#include "TradingPostUI.h"
#include "SquadUI.h"
#include "SessionUI.h"
#include "TrackerUI.h"
#include "ReminderUI.h"

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();
void OnArcDPSCombat(void* eventArgs);

void RenderGeneralTab();

namespace
{
    std::string GetCurrentHistoryCharacterName()
    {
        const std::vector<SquadTrackedPlayer> players =
            SquadTracker::GetPlayers();

        for (
            const SquadTrackedPlayer& player :
            players
            )
        {
            if (
                player.isSelf &&
                !player.characterName.empty()
                )
            {
                return player.characterName;
            }
        }

        return "";
    }
}

AddonDefinition_t AddonDef = {};
HMODULE hSelf = nullptr;
AddonAPI_t* APIDefs = nullptr;
NexusLinkData_t* NexusLink = nullptr;

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID
)
{
    if (ul_reason_for_call ==
        DLL_PROCESS_ATTACH)
    {
        hSelf = hModule;
    }

    return TRUE;
}

extern "C"
__declspec(dllexport)
AddonDefinition_t* GetAddonDef()
{
    // Provisional development signature.
    // Replace/register before public release.
    AddonDef.Signature =
        (uint32_t)-26081801;

    AddonDef.APIVersion =
        NEXUS_API_VERSION;

    AddonDef.Name =
        "Food Reminder";

    AddonDef.Version.Major = 0;
    AddonDef.Version.Minor = 2;
    AddonDef.Version.Build = 1;
    AddonDef.Version.Revision = 0;

    AddonDef.Author =
        "spectre9510";

    AddonDef.Description =
        "Food and utility expiration reminders for Guild Wars 2.";

    AddonDef.Load =
        AddonLoad;

    AddonDef.Unload =
        AddonUnload;

    AddonDef.Flags =
        AF_None;

    return &AddonDef;
}

void AddonLoad(AddonAPI_t* aApi)
{
    APIDefs = aApi;

    ImGui::SetCurrentContext(
        (ImGuiContext*)APIDefs->ImguiContext
    );

    ImGui::SetAllocatorFunctions(
        (void* (*)(size_t, void*))
        APIDefs->ImguiMalloc,
        (void (*)(void*, void*))
        APIDefs->ImguiFree
    );
    Settings::Load(hSelf);
    SessionTracker::Start(hSelf);
    BuffTracker::RestorePrimerState();
    SquadTracker::RestoreUnknownConsumables();
    TradingPostPriceManager::Start();
    TradingPostHistoryManager::Start(hSelf);
    TradingPostItemIndexManager::Start(hSelf);
    TradingPostWatchManager::Start(hSelf);
    ConsumableMetadataManager::Start();
    RTAPIIntegration::Start(APIDefs);
    SquadTracker::SaveUnknownConsumables();
    Settings::Save(hSelf);

    NexusLink =
        (NexusLinkData_t*)
        APIDefs->DataLink_Get(
            "DL_NEXUS_LINK"
        );

    APIDefs->Events_Subscribe(
        "EV_ARCDPS_COMBATEVENT_SQUAD_RAW",
        OnArcDPSCombat
    );

    APIDefs->GUI_Register(
        RT_Render,
        AddonRender
    );

    APIDefs->GUI_Register(
        RT_OptionsRender,
        AddonOptions
    );

    APIDefs->Log(
        LOGL_INFO,
        "Food Reminder",
        "Food Reminder loaded."
    );
}

void OnArcDPSCombat(void* eventArgs)
{
    EvCombatData* combatData =
        static_cast<EvCombatData*>(
            eventArgs
            );

    SquadTracker::ProcessEvent(
        combatData
    );

    BuffTracker::ProcessEvent(
        combatData
    );

    if (
        BuffTracker::ConsumeSettingsChanged()
        )
    {
        Settings::Save(hSelf);
    }
}

void AddonUnload()
{
    BuffTracker::SavePrimerState();
    SquadTracker::SaveUnknownConsumables();
    Settings::Save(hSelf);
    SessionTracker::Shutdown();

    TradingPostWatchManager::Shutdown();
    TradingPostItemIndexManager::Shutdown();
    TradingPostItemIndexManager::Reset();
    TradingPostPriceManager::Shutdown();
    TradingPostPriceManager::Reset();
    TradingPostHistoryManager::Shutdown();
    ConsumableMetadataManager::Shutdown();
    ConsumableMetadataManager::Reset();
    RTAPIIntegration::Shutdown();

    ExtrasIntegration::Reset();
    SquadTracker::Reset();
    {
        APIDefs->Events_Unsubscribe(
            "EV_ARCDPS_COMBATEVENT_SQUAD_RAW",
            OnArcDPSCombat
        );

        APIDefs->GUI_Deregister(
            AddonRender
        );

        APIDefs->GUI_Deregister(
            AddonOptions
        );

        APIDefs->Log(
            LOGL_INFO,
            "Food Reminder",
            "Food Reminder unloaded."
        );
    }

    NexusLink = nullptr;
    APIDefs = nullptr;
}





void AddonRender()
{
    TradingPostWatchManager::Update();

    const bool hasFood =
        BuffTracker::HasFood();

    const bool hasUtility =
        BuffTracker::HasUtility();

    const ConsumableDetectionState foodDetectionState =
        BuffTracker::GetFoodDetectionState();

    const ConsumableDetectionState utilityDetectionState =
        BuffTracker::GetUtilityDetectionState();

    if (
        BuffTracker::ConsumeSettingsChanged()
        )
    {
        Settings::Save(hSelf);
    }

    const int64_t foodRemaining =
        hasFood
        ? BuffTracker::
        GetFoodRemainingMilliseconds()
        : 0;

    const int64_t utilityRemaining =
        hasUtility
        ? BuffTracker::
        GetUtilityRemainingMilliseconds()
        : 0;

    const bool hasMetabolicPrimer =
        BuffTracker::HasMetabolicPrimer();

    const bool hasUtilityPrimer =
        BuffTracker::HasUtilityPrimer();

    const ConsumableDetectionState
        metabolicPrimerDetectionState =
        BuffTracker::
        GetMetabolicPrimerDetectionState();

    const ConsumableDetectionState
        utilityPrimerDetectionState =
        BuffTracker::
        GetUtilityPrimerDetectionState();

    if (
        BuffTracker::ConsumeSettingsChanged()
        )
    {
        Settings::Save(hSelf);
    }

    const int64_t metabolicPrimerRemaining =
        hasMetabolicPrimer
        ? BuffTracker::
        GetMetabolicPrimerRemainingMilliseconds()
        : 0;

    const int64_t utilityPrimerRemaining =
        hasUtilityPrimer
        ? BuffTracker::
        GetUtilityPrimerRemainingMilliseconds()
        : 0;

    SessionPrimerState metabolicPrimerSessionState =
        SessionPrimerState::Unknown;

    if (hasMetabolicPrimer)
    {
        metabolicPrimerSessionState =
            SessionPrimerState::ConfirmedActive;
    }
    else if (
        metabolicPrimerDetectionState ==
        ConsumableDetectionState::Missing
        )
    {
        metabolicPrimerSessionState =
            SessionPrimerState::Inactive;
    }
    else if (
        hasFood &&
        foodRemaining >
        2LL * 60LL * 60LL * 1000LL
        )
    {
        //
        // Presence only is inferred from clearly Primer-extended Food.
        // Never infer the Primer's remaining countdown from Food duration.
        //
        metabolicPrimerSessionState =
            SessionPrimerState::InferredActive;
    }

    SessionPrimerState utilityPrimerSessionState =
        SessionPrimerState::Unknown;

    if (hasUtilityPrimer)
    {
        utilityPrimerSessionState =
            SessionPrimerState::ConfirmedActive;
    }
    else if (
        utilityPrimerDetectionState ==
        ConsumableDetectionState::Missing
        )
    {
        utilityPrimerSessionState =
            SessionPrimerState::Inactive;
    }
    else if (
        hasUtility &&
        utilityRemaining >
        2LL * 60LL * 60LL * 1000LL
        )
    {
        //
        // Presence only is inferred from clearly Primer-extended Utility.
        // Never infer the Primer's remaining countdown from Utility duration.
        //
        utilityPrimerSessionState =
            SessionPrimerState::InferredActive;
    }

    const bool inCombat =
        BuffTracker::IsInCombat();

    const bool isGameplay =
        NexusLink != nullptr &&
        NexusLink->IsGameplay;

    const bool tradingPostTabVisible =
        TradingPostUI::WasTabVisibleRecently();

    if (
        isGameplay &&
        !tradingPostTabVisible
        )
    {
        TradingPostUI::RenderTargetOverlay();
    }

    if (isGameplay)
    {
        SessionTracker::Update(
            GetCurrentHistoryCharacterName(),
            hasFood,
            BuffTracker::GetFoodSkillID(),
            hasUtility,
            BuffTracker::GetUtilitySkillID(),
            metabolicPrimerSessionState,
            utilityPrimerSessionState,
            inCombat
        );
    }

    if (g_Settings.showTracker &&
        isGameplay)
    {
        TrackerUI::Render(
            hasFood,
            foodRemaining,
            hasUtility,
            utilityRemaining,
            hasMetabolicPrimer,
            metabolicPrimerRemaining,
            hasUtilityPrimer,
            utilityPrimerRemaining
        );
    }

    if (!g_Settings.enabled)
    {
        return;
    }

    ReminderManager::Update(
        hasFood &&
        g_Settings.enableFoodExpirationReminder,
        foodRemaining,
        g_Settings.foodWarningSeconds,
        hasUtility &&
        g_Settings.enableUtilityExpirationReminder,
        utilityRemaining,
        g_Settings.utilityWarningSeconds
    );

    ReminderManager::UpdatePrimerWarnings(
        hasMetabolicPrimer &&
        g_Settings.enablePrimerExpirationReminder,
        metabolicPrimerRemaining,
        g_Settings
        .metabolicPrimerWarningSeconds,
        hasUtilityPrimer &&
        g_Settings.enablePrimerExpirationReminder,
        utilityPrimerRemaining,
        g_Settings
        .utilityPrimerWarningSeconds
    );



    ReminderManager::
        UpdateMissingBuffWarnings(
            inCombat,
            g_Settings.enableMissingConsumableWarning
            ? (
                foodDetectionState ==
                ConsumableDetectionState::Missing
                ? hasFood
                : true
                )
            : true,
            g_Settings.enableMissingConsumableWarning
            ? (
                utilityDetectionState ==
                ConsumableDetectionState::Missing
                ? hasUtility
                : true
                )
            : true
        );

    ReminderUI::Render(
        hasFood,
        foodRemaining,
        hasUtility,
        utilityRemaining
    );
}







void RenderGeneralTab()
{
    bool settingsChanged = false;

    if (ImGui::Checkbox(
        "Enable reminders",
        &g_Settings.enabled))
    {
        settingsChanged = true;
    }

    ImGui::Indent();

    if (ImGui::Checkbox(
        "Food expiration reminders",
        &g_Settings.enableFoodExpirationReminder))
    {
        settingsChanged = true;
    }

    if (ImGui::Checkbox(
        "Utility expiration reminders",
        &g_Settings.enableUtilityExpirationReminder))
    {
        settingsChanged = true;
    }

    if (ImGui::Checkbox(
        "Missing Food/Utility combat warnings",
        &g_Settings.enableMissingConsumableWarning))
    {
        settingsChanged = true;
    }

    if (ImGui::Checkbox(
        "Primer expiration reminders",
        &g_Settings.enablePrimerExpirationReminder))
    {
        settingsChanged = true;
    }

    ImGui::Unindent();

    ImGui::SetNextItemWidth(180.0f);

    if (ImGui::SliderInt(
        "Reminder display time (seconds)",
        &g_Settings.reminderDisplaySeconds,
        3,
        10
    ))
    {
        settingsChanged = true;
    }

    if (ImGui::Checkbox(
        "Show compact tracker",
        &g_Settings.showTracker))
    {
        settingsChanged = true;
    }

    if (ImGui::Checkbox(
        "Lock tracker position",
        &g_Settings.lockTrackerPosition))
    {
        settingsChanged = true;
    }

    if (ImGui::Checkbox(
        "Lock reminder position",
        &g_Settings.lockReminderPosition))
    {
        settingsChanged = true;
    }

    ImGui::Spacing();

    ImGui::TextUnformatted(
        "Reminder Timing"
    );

    int foodWarningMinutes =
        g_Settings.foodWarningSeconds /
        60;

    int utilityWarningMinutes =
        g_Settings.utilityWarningSeconds /
        60;

    int metabolicPrimerWarningMinutes =
        g_Settings
        .metabolicPrimerWarningSeconds /
        60;

    int utilityPrimerWarningMinutes =
        g_Settings
        .utilityPrimerWarningSeconds /
        60;

    ImGui::Spacing();

    ImGui::TextUnformatted(
        "Food & Utility"
    );

    if (ImGui::SliderInt(
        "Food early warning",
        &foodWarningMinutes,
        1,
        60,
        "%d min"))
    {
        g_Settings.foodWarningSeconds =
            foodWarningMinutes * 60;

        settingsChanged = true;
    }

    if (ImGui::SliderInt(
        "Utility early warning",
        &utilityWarningMinutes,
        1,
        60,
        "%d min"))
    {
        g_Settings.utilityWarningSeconds =
            utilityWarningMinutes * 60;

        settingsChanged = true;
    }

    ImGui::Spacing();

    ImGui::TextUnformatted(
        "Primers"
    );

    if (ImGui::SliderInt(
        "Metabolic Primer early warning",
        &metabolicPrimerWarningMinutes,
        5,
        60,
        "%d min"))
    {
        g_Settings
            .metabolicPrimerWarningSeconds =
            metabolicPrimerWarningMinutes *
            60;

        settingsChanged = true;
    }

    if (ImGui::SliderInt(
        "Utility Primer early warning",
        &utilityPrimerWarningMinutes,
        5,
        60,
        "%d min"))
    {
        g_Settings
            .utilityPrimerWarningSeconds =
            utilityPrimerWarningMinutes *
            60;

        settingsChanged = true;
    }

    if (settingsChanged)
    {
        Settings::Save(hSelf);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextUnformatted(
        "Current Buffs"
    );

    if (BuffTracker::HasFood())
    {
        const int64_t foodMs =
            BuffTracker::
            GetFoodRemainingMilliseconds();

        const int64_t foodSeconds =
            foodMs / 1000;

        const int64_t foodHours =
            foodSeconds / 3600;

        const int64_t foodMinutes =
            (foodSeconds % 3600) / 60;

        const int64_t foodRemainingSeconds =
            foodSeconds % 60;

        ImGui::Text(
            "Food:    %02lld:%02lld:%02lld",
            foodHours,
            foodMinutes,
            foodRemainingSeconds
        );
    }
    else
    {
        ImGui::Text(
            "Food:    Not detected"
        );
    }

    if (BuffTracker::HasUtility())
    {
        const int64_t utilityMs =
            BuffTracker::
            GetUtilityRemainingMilliseconds();

        const int64_t utilitySeconds =
            utilityMs / 1000;

        const int64_t utilityHours =
            utilitySeconds / 3600;

        const int64_t utilityMinutes =
            (utilitySeconds % 3600) /
            60;

        const int64_t
            utilityRemainingSeconds =
            utilitySeconds % 60;

        ImGui::Text(
            "Utility: %02lld:%02lld:%02lld",
            utilityHours,
            utilityMinutes,
            utilityRemainingSeconds
        );
    }
    else
    {
        ImGui::Text(
            "Utility: Not detected"
        );
    }

    if (
        BuffTracker::
        HasMetabolicPrimer()
        )
    {
        const int64_t primerMs =
            BuffTracker::
            GetMetabolicPrimerRemainingMilliseconds();

        const int64_t primerSeconds =
            primerMs / 1000;

        const int64_t primerHours =
            primerSeconds / 3600;

        const int64_t primerMinutes =
            (primerSeconds % 3600) /
            60;

        const int64_t
            primerRemainingSeconds =
            primerSeconds % 60;

        ImGui::Text(
            "Metabolic Primer: %02lld:%02lld:%02lld",
            primerHours,
            primerMinutes,
            primerRemainingSeconds
        );
    }
    else if (
        BuffTracker::
        HasInferredMetabolicPrimerPresence()
        )
    {
        ImGui::Text(
            "Metabolic Primer: Active*"
        );
    }
    else if (
        BuffTracker::
        GetMetabolicPrimerDetectionState() ==
        ConsumableDetectionState::Unknown
        )
    {
        ImGui::Text(
            "Metabolic Primer: Unknown"
        );
    }
    else
    {
        ImGui::Text(
            "Metabolic Primer: Not detected"
        );
    }

    if (
        BuffTracker::
        HasUtilityPrimer()
        )
    {
        const int64_t primerMs =
            BuffTracker::
            GetUtilityPrimerRemainingMilliseconds();

        const int64_t primerSeconds =
            primerMs / 1000;

        const int64_t primerHours =
            primerSeconds / 3600;

        const int64_t primerMinutes =
            (primerSeconds % 3600) /
            60;

        const int64_t
            primerRemainingSeconds =
            primerSeconds % 60;

        ImGui::Text(
            "Utility Primer:   %02lld:%02lld:%02lld",
            primerHours,
            primerMinutes,
            primerRemainingSeconds
        );
    }
    else if (
        BuffTracker::
        HasInferredUtilityPrimerPresence()
        )
    {
        ImGui::Text(
            "Utility Primer:   Active*"
        );
    }
    else if (
        BuffTracker::
        GetUtilityPrimerDetectionState() ==
        ConsumableDetectionState::Unknown
        )
    {
        ImGui::Text(
            "Utility Primer:   Unknown"
        );
    }
    else
    {
        ImGui::Text(
            "Utility Primer:   Not detected"
        );
    }

    ImGui::Spacing();

    if (ImGui::Button(
        "Test Reminder"))
    {
        ReminderManager::
            TriggerTestReminder();
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::CollapsingHeader(
        "Developer Debug"))
    {
        const bool debugInCombat =
            BuffTracker::IsInCombat();

        ImGui::Text(
            "Combat State: %s",
            debugInCombat
            ? "IN COMBAT"
            : "OUT OF COMBAT"
        );


        ImGui::Text(
            "ArcDPS Events: %llu",
            static_cast<
            unsigned long long
            >(
                BuffTracker::
                GetTotalEventCount()
                )
        );

        ImGui::Text(
            "Buff-like Events: %llu",
            static_cast<
            unsigned long long
            >(
                BuffTracker::
                GetBuffLikeEventCount()
                )
        );

        ImGui::Text(
            "Food Skill ID: %u",
            BuffTracker::GetFoodSkillID()
        );

        ImGui::Text(
            "Utility Skill ID: %u",
            BuffTracker::GetUtilitySkillID()
        );

        //
        // Unofficial Extras debug.
        //
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::TextUnformatted(
            "Unofficial Extras"
        );

        const bool extrasAvailable =
            ExtrasIntegration::IsAvailable();

        ImGui::Text(
            "Status: %s",
            extrasAvailable
            ? "Connected"
            : "Not detected"
        );

        if (extrasAvailable)
        {
            const std::string extrasVersion =
                ExtrasIntegration::GetVersion();

            ImGui::Text(
                "Version: %s",
                extrasVersion.empty()
                ? "Unknown"
                : extrasVersion.c_str()
            );

            const std::vector<
                ExtrasSquadMember
            > extrasSquadMembers =
                ExtrasIntegration::
                GetSquadMembers();

            ImGui::Text(
                "Squad Members: %llu",
                static_cast<
                unsigned long long
                >(
                    extrasSquadMembers.size()
                    )
            );

            if (extrasSquadMembers.empty())
            {
                ImGui::TextDisabled(
                    "No squad members reported yet."
                );
            }
            else
            {
                for (
                    const ExtrasSquadMember&
                    member :
                    extrasSquadMembers
                    )
                {
                    const char* role =
                        member.isCommander
                        ? "Commander"
                        : member.isLieutenant
                        ? "Lieutenant"
                        : "Member";

                    ImGui::Text(
                        "Sub %u | %s | %s | Ready: %s",
                        static_cast<unsigned int>(
                            member.subgroup
                            ),
                        member.accountName.c_str(),
                        role,
                        member.ready
                        ? "Yes"
                        : "No"
                    );
                }
            }
        }
        else
        {
            ImGui::TextDisabled(
                "Squad tracking requires Unofficial Extras."
            );
        }

        ImGui::Separator();

        if (ImGui::Button(
            "Clear Buff Debug"))
        {
            BuffTracker::Reset();
        }

        ImGui::Spacing();

        ImGui::TextUnformatted(
            "Reminder Tests"
        );


        if (ImGui::Button(
            "Test Metabolic Primer Warning"))
        {
            ReminderManager::
                TriggerMetabolicPrimerTest();
        }

        if (ImGui::Button(
            "Test Utility Primer Warning"))
        {
            ReminderManager::
                TriggerUtilityPrimerTest();
        }

        if (ImGui::Button(
            "Test Both Primer Warnings"))
        {
            ReminderManager::
                TriggerBothPrimerTest();
        }

        ImGui::Spacing();

        ImGui::TextUnformatted(
            "Tracker Color Tests"
        );

        if (ImGui::Button(
            "Tracker Live"))
        {
            TrackerUI::SetColorTestMode(0);
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Tracker Normal"))
        {
            TrackerUI::SetColorTestMode(1);
        }

        if (ImGui::Button(
            "Tracker Warning"))
        {
            TrackerUI::SetColorTestMode(2);
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Tracker Critical"))
        {
            TrackerUI::SetColorTestMode(3);
        }

        if (ImGui::Button(
            "Tracker Missing"))
        {
            TrackerUI::SetColorTestMode(4);
        }

        ImGui::Text(
            "Tracker test mode: %s",
            TrackerUI::GetColorTestMode() == 0
            ? "Live"
            : TrackerUI::GetColorTestMode() == 1
            ? "Normal"
            : TrackerUI::GetColorTestMode() == 2
            ? "Warning"
            : TrackerUI::GetColorTestMode() == 3
            ? "Critical"
            : "Missing"
        );

        ImGui::Separator();

        const std::vector<
            BuffEventDebug
        > recentEvents =
            BuffTracker::
            GetRecentBuffEvents();

        static bool selfOnly = true;

        ImGui::Checkbox(
            "Show only events targeting me",
            &selfOnly
        );

        ImGui::TextUnformatted(
            "Recent Buff Events:"
        );

        for (
            auto it =
            recentEvents.rbegin();
            it != recentEvents.rend();
            ++it
            )
        {
            const BuffEventDebug& event =
                *it;


            if (
                selfOnly &&
                !event.destinationIsSelf
                )
            {
                continue;
            }

            ImGui::TextWrapped(
                "%s (%u) | Val:%d BuffDmg:%d Over:%u "
                "Buff:%u Remove:%u State:%u "
                "SrcSelf:%s DstSelf:%s",
                event.skillName.c_str(),
                event.skillID,
                event.value,
                event.buffDamage,
                event.overstackValue,
                static_cast<
                unsigned int
                >(
                    event.buff
                    ),
                static_cast<
                unsigned int
                >(
                    event.buffRemove
                    ),
                static_cast<
                unsigned int
                >(
                    event.stateChange
                    ),
                event.sourceIsSelf
                ? "Y"
                : "N",
                event.destinationIsSelf
                ? "Y"
                : "N"
            );
        }
    }

}


void AddonOptions()
{
    ImGui::TextUnformatted(
        "Food Reminder"
    );

    ImGui::Separator();

    if (ImGui::BeginTabBar(
        "##FoodReminderTabs"))
    {
        if (ImGui::BeginTabItem(
            "General"))
        {
            RenderGeneralTab();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(
            "Squad"))
        {
            SquadUI::Render();

            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem(
            "Session"))
        {
            SessionUI::Render();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(
            "History"))
        {
            HistoryUI::Render(
                GetCurrentHistoryCharacterName()
            );

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(
            "Trading Post"))
        {
            TradingPostUI::RenderTab(
                hSelf
            );

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Food Reminder v0.2.1 - Development Build"
    );
}
