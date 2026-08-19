#include <Windows.h>

#include "nexus/Nexus.h"
#include "imgui/imgui.h"

#include "Settings.h"
#include "ReminderManager.h"
#include "ArcDPS.h"
#include "BuffTracker.h"

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();
void OnArcDPSCombat(void* eventArgs);

AddonDefinition_t AddonDef = {};
HMODULE hSelf = nullptr;
AddonAPI_t* APIDefs = nullptr;
NexusLinkData_t* NexusLink = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        hSelf = hModule;
    }

    return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
    // Provisional development signature. Replace/register before public release.
    AddonDef.Signature = (uint32_t)-26081801;
    AddonDef.APIVersion = NEXUS_API_VERSION;
    AddonDef.Name = "Food Reminder";

    AddonDef.Version.Major = 0;
    AddonDef.Version.Minor = 1;
    AddonDef.Version.Build = 0;
    AddonDef.Version.Revision = 0;

    AddonDef.Author = "Scott";
    AddonDef.Description = "Food and utility expiration reminders for Guild Wars 2.";

    AddonDef.Load = AddonLoad;
    AddonDef.Unload = AddonUnload;
    AddonDef.Flags = AF_None;

    return &AddonDef;
}

void AddonLoad(AddonAPI_t* aApi)
{
    APIDefs = aApi;

    ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
    ImGui::SetAllocatorFunctions(
        (void* (*)(size_t, void*))APIDefs->ImguiMalloc,
        (void (*)(void*, void*))APIDefs->ImguiFree
    );

    NexusLink = (NexusLinkData_t*)APIDefs->DataLink_Get("DL_NEXUS_LINK");
    APIDefs->Events_Subscribe(
        "EV_ARCDPS_COMBATEVENT_SQUAD_RAW",
        OnArcDPSCombat
    );

    APIDefs->GUI_Register(RT_Render, AddonRender);
    APIDefs->GUI_Register(RT_OptionsRender, AddonOptions);

    APIDefs->Log(LOGL_INFO, "Food Reminder", "Food Reminder loaded.");
}
void OnArcDPSCombat(void* eventArgs)
{
    EvCombatData* combatData =
        static_cast<EvCombatData*>(eventArgs);

    BuffTracker::ProcessEvent(combatData);
}

void AddonUnload()
{
    if (APIDefs != nullptr)
    {
        APIDefs->Events_Unsubscribe(
            "EV_ARCDPS_COMBATEVENT_SQUAD_RAW",
            OnArcDPSCombat
        );

        APIDefs->GUI_Deregister(AddonRender);
        APIDefs->GUI_Deregister(AddonOptions);
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
    if (!g_Settings.enabled)
    {
        return;
    }

    const bool hasFood =
        BuffTracker::HasFood();

    const bool hasUtility =
        BuffTracker::HasUtility();

    const int64_t foodRemaining =
        hasFood
        ? BuffTracker::GetFoodRemainingMilliseconds()
        : 0;

    const int64_t utilityRemaining =
        hasUtility
        ? BuffTracker::GetUtilityRemainingMilliseconds()
        : 0;

    ReminderManager::Update(
        hasFood,
        foodRemaining,
        g_Settings.foodWarningSeconds,
        hasUtility,
        utilityRemaining,
        g_Settings.utilityWarningSeconds
    );

    if (!ReminderManager::IsReminderActive())
    {
        return;
    }

    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    const ImVec2 center(
        displaySize.x * 0.5f,
        displaySize.y * 0.25f
    );

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.90f);

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("##FoodReminderAlert", nullptr, flags))
    {
        ImGui::TextUnformatted(
            ReminderManager::GetReminderTitle()
        );

        ImGui::Separator();

        ImGui::TextUnformatted(
            ReminderManager::GetReminderMessage()
        );

        ImGui::Text(
            "Closes in %.1f seconds",
            ReminderManager::GetReminderSecondsRemaining()
        );
    }

    ImGui::End();
}

void AddonOptions()
{
    ImGui::TextUnformatted("Food Reminder");
    ImGui::Separator();

    ImGui::Checkbox("Enable reminders", &g_Settings.enabled);

    ImGui::SliderInt(
        "Food early warning",
        &g_Settings.foodWarningSeconds,
        30,
        7200,
        "%d sec"
    );

    ImGui::SliderInt(
        "Utility early warning",
        &g_Settings.utilityWarningSeconds,
        30,
        7200,
        "%d sec"
    );

    ImGui::Spacing();

    if (ImGui::Button("Test Reminder"))
    {
        ReminderManager::TriggerTestReminder();
    }
    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::CollapsingHeader("ArcDPS Buff Debug"))
    {
        ImGui::Text(
            "ArcDPS Events: %llu",
            static_cast<unsigned long long>(
                BuffTracker::GetTotalEventCount()
                )
        );
        ImGui::Text("Detected Buffs:");

        ImGui::Text("Detected Buffs:");

        if (BuffTracker::HasFood())
        {
            const int64_t foodMs =
                BuffTracker::GetFoodRemainingMilliseconds();

            const int64_t foodSeconds = foodMs / 1000;

            const int64_t foodHours =
                foodSeconds / 3600;

            const int64_t foodMinutes =
                (foodSeconds % 3600) / 60;

            const int64_t foodRemainingSeconds =
                foodSeconds % 60;

            ImGui::Text(
                "Food: %02lld:%02lld:%02lld",
                foodHours,
                foodMinutes,
                foodRemainingSeconds
            );
        }
        else
        {
            ImGui::Text("Food: Not detected");
        }

        if (BuffTracker::HasUtility())
        {
            const int64_t utilityMs =
                BuffTracker::GetUtilityRemainingMilliseconds();

            const int64_t utilitySeconds = utilityMs / 1000;

            const int64_t utilityHours =
                utilitySeconds / 3600;

            const int64_t utilityMinutes =
                (utilitySeconds % 3600) / 60;

            const int64_t utilityRemainingSeconds =
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
            ImGui::Text("Utility: Not detected");
        }

        ImGui::Spacing();

        ImGui::Spacing();

        ImGui::Text(
            "Buff-like Events: %llu",
            static_cast<unsigned long long>(
                BuffTracker::GetBuffLikeEventCount()
                )
        );

        if (ImGui::Button("Clear Buff Debug"))
        {
            BuffTracker::Reset();
        }

        ImGui::Separator();

        const std::vector<BuffEventDebug> recentEvents =
            BuffTracker::GetRecentBuffEvents();

        static bool selfOnly = true;

        ImGui::Checkbox(
            "Show only events targeting me",
            &selfOnly
        );

        ImGui::Text("Recent Buff Events:");
        ImGui::Text("Recent Buff Events:");

        for (auto it = recentEvents.rbegin();
            it != recentEvents.rend();
            ++it)
        {
            
            const BuffEventDebug& event = *it;

            if (selfOnly && !event.destinationIsSelf)
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
                static_cast<unsigned int>(event.buff),
                static_cast<unsigned int>(event.buffRemove),
                static_cast<unsigned int>(event.stateChange),
                event.sourceIsSelf ? "Y" : "N",
                event.destinationIsSelf ? "Y" : "N"
            );
        }
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Food Reminder v0.1.0 - Development Build");
}
