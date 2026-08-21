#include <Windows.h>
#include <string>
#include <cfloat>

#include "nexus/Nexus.h"
#include "imgui/imgui.h"

#include "Settings.h"
#include "ReminderManager.h"
#include "ArcDPS.h"
#include "BuffTracker.h"
#include "ConsumableData.h"
#include "ExtrasIntegration.h"
#include "SquadTracker.h"

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();
void OnArcDPSCombat(void* eventArgs);

void RenderCompactTracker(
    bool hasFood,
    int64_t foodRemaining,
    bool hasUtility,
    int64_t utilityRemaining
);

void RenderConsumableTooltip(
    const ConsumableInfo& info
);

void RenderGeneralTab();
void RenderSquadTab();

AddonDefinition_t AddonDef = {};
HMODULE hSelf = nullptr;
AddonAPI_t* APIDefs = nullptr;
NexusLinkData_t* NexusLink = nullptr;

// Developer-only tracker color test.
// 0 = Live
// 1 = Normal
// 2 = Warning
// 3 = Critical
// 4 = Missing
int g_TrackerColorTestMode = 0;

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
    AddonDef.Version.Minor = 1;
    AddonDef.Version.Build = 0;
    AddonDef.Version.Revision = 0;

    AddonDef.Author =
        "Scott";

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
    BuffTracker::RestorePrimerState();

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
    Settings::Save(hSelf);

    ExtrasIntegration::Reset();
    SquadTracker::Reset();

    if (APIDefs != nullptr)
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

void RenderConsumableTooltip(
    const ConsumableInfo& info
)
{
    if (!ImGui::IsItemHovered())
    {
        return;
    }

    ImGui::BeginTooltip();

    ImGui::TextUnformatted(
        info.name
    );

    ImGui::Separator();

    ImGui::TextUnformatted(
        info.effects
    );

    ImGui::EndTooltip();
}

void RenderCompactTracker(
    bool hasFood,
    int64_t foodRemaining,
    bool hasUtility,
    int64_t utilityRemaining
)
{
    ImGuiWindowFlags trackerFlags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse;

    if (g_Settings.lockTrackerPosition)
    {
        trackerFlags |=
            ImGuiWindowFlags_NoMove;
    }

    const ImVec4 normalColor(
        0.40f,
        1.00f,
        0.40f,
        1.00f
    );

    const ImVec4 warningColor(
        1.00f,
        0.85f,
        0.20f,
        1.00f
    );

    const ImVec4 criticalColor(
        1.00f,
        0.30f,
        0.30f,
        1.00f
    );

    const ImVec4 missingColor(
        0.65f,
        0.65f,
        0.65f,
        1.00f
    );

    bool displayHasFood =
        hasFood;

    bool displayHasUtility =
        hasUtility;

    int64_t displayFoodRemaining =
        foodRemaining;

    int64_t displayUtilityRemaining =
        utilityRemaining;

    if (g_TrackerColorTestMode == 1)
    {
        displayHasFood = true;
        displayHasUtility = true;

        displayFoodRemaining =
            2LL * 60LL * 60LL * 1000LL;

        displayUtilityRemaining =
            2LL * 60LL * 60LL * 1000LL;
    }
    else if (
        g_TrackerColorTestMode == 2)
    {
        displayHasFood = true;
        displayHasUtility = true;

        displayFoodRemaining =
            5LL * 60LL * 1000LL;

        displayUtilityRemaining =
            5LL * 60LL * 1000LL;
    }
    else if (
        g_TrackerColorTestMode == 3)
    {
        displayHasFood = true;
        displayHasUtility = true;

        displayFoodRemaining =
            30LL * 1000LL;

        displayUtilityRemaining =
            30LL * 1000LL;
    }
    else if (
        g_TrackerColorTestMode == 4)
    {
        displayHasFood = false;
        displayHasUtility = false;

        displayFoodRemaining = 0;
        displayUtilityRemaining = 0;
    }

    //
    // Stable but tighter tracker width.
    //
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(260.0f, 0.0f),
        ImVec2(FLT_MAX, FLT_MAX)
    );

    if (ImGui::Begin(
        "Food Reminder Tracker",
        nullptr,
        trackerFlags))
    {
        if (displayHasFood)
        {
            const int64_t totalSeconds =
                displayFoodRemaining / 1000;

            const int64_t hours =
                totalSeconds / 3600;

            const int64_t minutes =
                (totalSeconds % 3600) / 60;

            const int64_t seconds =
                totalSeconds % 60;

            const ImVec4* foodColor =
                &normalColor;

            if (g_TrackerColorTestMode == 2)
            {
                foodColor =
                    &warningColor;
            }
            else if (
                g_TrackerColorTestMode == 3)
            {
                foodColor =
                    &criticalColor;
            }
            else if (
                g_TrackerColorTestMode != 1)
            {
                if (totalSeconds <= 60)
                {
                    foodColor =
                        &criticalColor;
                }
                else if (
                    totalSeconds <=
                    g_Settings.foodWarningSeconds)
                {
                    foodColor =
                        &warningColor;
                }
            }

            const uint32_t foodSkillID =
                BuffTracker::GetFoodSkillID();

            const ConsumableInfo& foodInfo =
                ConsumableData::GetFoodInfo(
                    foodSkillID
                );

            ImGui::TextColored(
                *foodColor,
                "Food:    %02lld:%02lld:%02lld",
                hours,
                minutes,
                seconds
            );

            ImGui::SameLine(
                155.0f
            );

            ImGui::TextUnformatted(
                foodInfo.label
            );

            RenderConsumableTooltip(
                foodInfo
            );
        }
        else
        {
            ImGui::TextColored(
                missingColor,
                "Food:    Not detected"
            );
        }

        if (displayHasUtility)
        {
            const int64_t totalSeconds =
                displayUtilityRemaining / 1000;

            const int64_t hours =
                totalSeconds / 3600;

            const int64_t minutes =
                (totalSeconds % 3600) / 60;

            const int64_t seconds =
                totalSeconds % 60;

            const ImVec4* utilityColor =
                &normalColor;

            if (g_TrackerColorTestMode == 2)
            {
                utilityColor =
                    &warningColor;
            }
            else if (
                g_TrackerColorTestMode == 3)
            {
                utilityColor =
                    &criticalColor;
            }
            else if (
                g_TrackerColorTestMode != 1)
            {
                if (totalSeconds <= 60)
                {
                    utilityColor =
                        &criticalColor;
                }
                else if (
                    totalSeconds <=
                    g_Settings.utilityWarningSeconds)
                {
                    utilityColor =
                        &warningColor;
                }
            }

            const uint32_t utilitySkillID =
                BuffTracker::GetUtilitySkillID();

            const ConsumableInfo& utilityInfo =
                ConsumableData::GetUtilityInfo(
                    utilitySkillID
                );

            ImGui::TextColored(
                *utilityColor,
                "Utility: %02lld:%02lld:%02lld",
                hours,
                minutes,
                seconds
            );

            ImGui::SameLine(
                155.0f
            );

            ImGui::TextUnformatted(
                utilityInfo.label
            );

            RenderConsumableTooltip(
                utilityInfo
            );
        }
        else
        {
            ImGui::TextColored(
                missingColor,
                "Utility: Not detected"
            );
        }
    }

    ImGui::End();
}

void AddonRender()
{
    const bool hasFood =
        BuffTracker::HasFood();

    const bool hasUtility =
        BuffTracker::HasUtility();

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

    if (g_Settings.showTracker &&
        NexusLink != nullptr &&
        NexusLink->IsGameplay)
    {
        RenderCompactTracker(
            hasFood,
            foodRemaining,
            hasUtility,
            utilityRemaining
        );
    }

    if (!g_Settings.enabled)
    {
        return;
    }

    ReminderManager::Update(
        hasFood,
        foodRemaining,
        g_Settings.foodWarningSeconds,
        hasUtility,
        utilityRemaining,
        g_Settings.utilityWarningSeconds
    );

    ReminderManager::UpdatePrimerWarnings(
        hasMetabolicPrimer,
        metabolicPrimerRemaining,
        g_Settings
        .metabolicPrimerWarningSeconds,
        hasUtilityPrimer,
        utilityPrimerRemaining,
        g_Settings
        .utilityPrimerWarningSeconds
    );

    const bool inCombat =
        BuffTracker::IsInCombat();

    ReminderManager::
        UpdateMissingBuffWarnings(
            inCombat,
            hasFood,
            hasUtility
        );

    if (
        !ReminderManager::
        IsReminderActive()
        )
    {
        return;
    }

    const ImVec2 displaySize =
        ImGui::GetIO().DisplaySize;

    const ImVec2 center(
        displaySize.x * 0.5f,
        displaySize.y * 0.25f
    );

    ImGui::SetNextWindowPos(
        center,
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f)
    );

    ImGui::SetNextWindowBgAlpha(
        0.90f
    );

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (ImGui::Begin(
        "##FoodReminderAlert",
        nullptr,
        flags))
    {
        int64_t remainingMs = 0;

        const char* reminderTitle =
            ReminderManager::
            GetReminderTitle();

        const std::string title =
            reminderTitle;

        if (title ==
            "FOOD REMINDER")
        {
            remainingMs =
                foodRemaining;
        }
        else if (
            title ==
            "UTILITY REMINDER")
        {
            remainingMs =
                utilityRemaining;
        }
        else if (
            title ==
            "METABOLIC PRIMER EXPIRING" ||
            title ==
            "UTILITY PRIMER EXPIRING" ||
            title ==
            "PRIMERS EXPIRING")
        {
            remainingMs =
                ReminderManager::
                GetBuffRemainingMilliseconds();
        }
        else if (
            title ==
            "FOOD + UTILITY REMINDER")
        {
            if (hasFood &&
                hasUtility)
            {
                remainingMs =
                    foodRemaining <
                    utilityRemaining
                    ? foodRemaining
                    : utilityRemaining;
            }
        }

        const int64_t remainingSeconds =
            remainingMs / 1000;

        const int64_t hours =
            remainingSeconds / 3600;

        const int64_t minutes =
            (remainingSeconds % 3600) /
            60;

        const int64_t seconds =
            remainingSeconds % 60;

        ImGui::TextUnformatted(
            ReminderManager::
            GetReminderTitle()
        );

        ImGui::Separator();

        const bool isMissingBuffReminder =
            title == "FOOD MISSING" ||
            title == "UTILITY MISSING" ||
            title ==
            "FOOD + UTILITY MISSING";

        if (isMissingBuffReminder)
        {
            ImGui::TextUnformatted(
                ReminderManager::
                GetReminderMessage()
            );
        }
        else
        {
            ImGui::Text(
                "%02lld:%02lld:%02lld remaining",
                hours,
                minutes,
                seconds
            );
        }
    }

    ImGui::End();
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
    else
    {
        const int64_t foodMs =
            BuffTracker::
            GetFoodRemainingMilliseconds();

        const int64_t twoHoursMs =
            2LL * 60LL * 60LL *
            1000LL;

        if (foodMs > twoHoursMs)
        {
            const int64_t foodSeconds =
                foodMs / 1000;

            const int64_t foodHours =
                foodSeconds / 3600;

            const int64_t foodMinutes =
                (foodSeconds % 3600) /
                60;

            const int64_t
                foodRemainingSeconds =
                foodSeconds % 60;

            ImGui::Text(
                "Metabolic Primer: ~%02lld:%02lld:%02lld (inferred)",
                foodHours,
                foodMinutes,
                foodRemainingSeconds
            );
        }
        else
        {
            ImGui::Text(
                "Metabolic Primer: Not detected"
            );
        }
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
    else
    {
        const int64_t utilityMs =
            BuffTracker::
            GetUtilityRemainingMilliseconds();

        const int64_t twoHoursMs =
            2LL * 60LL * 60LL *
            1000LL;

        if (utilityMs > twoHoursMs)
        {
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
                "Utility Primer:   ~%02lld:%02lld:%02lld (inferred)",
                utilityHours,
                utilityMinutes,
                utilityRemainingSeconds
            );
        }
        else
        {
            ImGui::Text(
                "Utility Primer:   Not detected"
            );
        }
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
            g_TrackerColorTestMode = 0;
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Tracker Normal"))
        {
            g_TrackerColorTestMode = 1;
        }

        if (ImGui::Button(
            "Tracker Warning"))
        {
            g_TrackerColorTestMode = 2;
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Tracker Critical"))
        {
            g_TrackerColorTestMode = 3;
        }

        if (ImGui::Button(
            "Tracker Missing"))
        {
            g_TrackerColorTestMode = 4;
        }

        ImGui::Text(
            "Tracker test mode: %s",
            g_TrackerColorTestMode == 0
            ? "Live"
            : g_TrackerColorTestMode == 1
            ? "Normal"
            : g_TrackerColorTestMode == 2
            ? "Warning"
            : g_TrackerColorTestMode == 3
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


void RenderSquadTab()
{
    ImGui::TextUnformatted(
        "Squad Tracking"
    );

    ImGui::Separator();

    const std::vector<
        SquadTrackedPlayer
    > trackedPlayers =
        SquadTracker::GetPlayers();

    ImGui::Text(
        "ArcDPS tracked players: %llu",
        static_cast<
        unsigned long long
        >(
            trackedPlayers.size()
            )
    );

    ImGui::TextWrapped(
        "Players appear here when ArcDPS begins tracking them in your current area/instance. "
        "? means ArcDPS has not established that consumable state yet."
    );

    ImGui::Spacing();

    if (trackedPlayers.empty())
    {
        ImGui::TextDisabled(
            "No ArcDPS players are currently tracked."
        );
    }
    else
    {
        const ImGuiTableFlags playerTableFlags =
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_SizingStretchProp |
            ImGuiTableFlags_ScrollX;

        if (ImGui::BeginTable(
            "##FoodReminderArcPlayers",
            7,
            playerTableFlags))
        {
            ImGui::TableSetupColumn(
                "Sub"
            );

            ImGui::TableSetupColumn(
                "Character"
            );

            ImGui::TableSetupColumn(
                "Food"
            );

            ImGui::TableSetupColumn(
                "Utility"
            );

            ImGui::TableSetupColumn(
                "Account"
            );

            ImGui::TableSetupColumn(
                "Agent ID"
            );

            ImGui::TableSetupColumn(
                "Self"
            );

            ImGui::TableHeadersRow();

            for (
                const SquadTrackedPlayer&
                player :
                trackedPlayers
                )
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);

                ImGui::Text(
                    "%u",
                    static_cast<unsigned int>(
                        player.subgroup
                        )
                );

                ImGui::TableSetColumnIndex(1);

                ImGui::TextUnformatted(
                    player.characterName.empty()
                    ? "-"
                    : player.characterName.c_str()
                );

                ImGui::TableSetColumnIndex(2);

                if (!player.foodStateKnown)
                {
                    ImGui::TextUnformatted(
                        "?"
                    );

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(
                            "Food state has not been established by ArcDPS yet."
                        );
                    }
                }
                else if (player.hasFood)
                {
                    const int64_t totalSeconds =
                        player.foodRemainingMilliseconds /
                        1000;

                    const int64_t hours =
                        totalSeconds / 3600;

                    const int64_t minutes =
                        (totalSeconds % 3600) /
                        60;

                    const int64_t seconds =
                        totalSeconds % 60;

                    const ConsumableInfo& foodInfo =
                        ConsumableData::GetFoodInfo(
                            player.foodSkillID
                        );

                    const std::string foodLabel =
                        foodInfo.label != nullptr
                        ? foodInfo.label
                        : "";

                    if (foodLabel == "Unknown")
                    {
                        ImGui::Text(
                            "Unknown (%u) %02lld:%02lld:%02lld",
                            player.foodSkillID,
                            hours,
                            minutes,
                            seconds
                        );
                    }
                    else
                    {
                        ImGui::Text(
                            "%s %02lld:%02lld:%02lld",
                            foodInfo.label,
                            hours,
                            minutes,
                            seconds
                        );
                    }

                    RenderConsumableTooltip(
                        foodInfo
                    );
                }
                else
                {
                    ImGui::TextUnformatted(
                        "None"
                    );
                }

                ImGui::TableSetColumnIndex(3);

                if (!player.utilityStateKnown)
                {
                    ImGui::TextUnformatted(
                        "?"
                    );

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(
                            "Utility state has not been established by ArcDPS yet."
                        );
                    }
                }
                else if (player.hasUtility)
                {
                    const int64_t totalSeconds =
                        player.utilityRemainingMilliseconds /
                        1000;

                    const int64_t hours =
                        totalSeconds / 3600;

                    const int64_t minutes =
                        (totalSeconds % 3600) /
                        60;

                    const int64_t seconds =
                        totalSeconds % 60;

                    const ConsumableInfo& utilityInfo =
                        ConsumableData::GetUtilityInfo(
                            player.utilitySkillID
                        );

                    const std::string utilityLabel =
                        utilityInfo.label != nullptr
                        ? utilityInfo.label
                        : "";

                    if (utilityLabel == "Unknown")
                    {
                        ImGui::Text(
                            "Unknown (%u) %02lld:%02lld:%02lld",
                            player.utilitySkillID,
                            hours,
                            minutes,
                            seconds
                        );
                    }
                    else
                    {
                        ImGui::Text(
                            "%s %02lld:%02lld:%02lld",
                            utilityInfo.label,
                            hours,
                            minutes,
                            seconds
                        );
                    }

                    RenderConsumableTooltip(
                        utilityInfo
                    );
                }
                else
                {
                    ImGui::TextUnformatted(
                        "None"
                    );
                }

                ImGui::TableSetColumnIndex(4);

                ImGui::TextUnformatted(
                    player.accountName.empty()
                    ? "-"
                    : player.accountName.c_str()
                );

                ImGui::TableSetColumnIndex(5);

                ImGui::Text(
                    "%llu",
                    static_cast<
                    unsigned long long
                    >(
                        player.agentID
                        )
                );

                ImGui::TableSetColumnIndex(6);

                ImGui::TextUnformatted(
                    player.isSelf
                    ? "Yes"
                    : "No"
                );
            }

            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    const bool extrasAvailable =
        ExtrasIntegration::IsAvailable();

    ImGui::Text(
        "Unofficial Extras: %s",
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
        > squadMembers =
            ExtrasIntegration::
            GetSquadMembers();

        ImGui::Text(
            "Extras squad updates received: %llu",
            static_cast<
            unsigned long long
            >(
                squadMembers.size()
                )
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Extras is optional. ArcDPS player and consumable tracking does not depend on it."
        );
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Test stage: ArcDPS player discovery + Food/Utility buff tracking."
    );
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
            RenderSquadTab();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Food Reminder v0.1.0 - Development Build"
    );
}
