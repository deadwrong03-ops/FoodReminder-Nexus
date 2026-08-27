#include <algorithm>
#include <Windows.h>
#include <string>
#include <cfloat>
#include <array>
#include <chrono>
#include <cmath>
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
#include "ConsumableData.h"
#include "ExtrasIntegration.h"
#include "SquadTracker.h"
#include "SessionTracker.h"

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();
void OnArcDPSCombat(void* eventArgs);

const char* GetSquadConsumableLabel(
    const char* label
)
{
    if (label == nullptr)
    {
        return "";
    }

    const std::string value =
        label;

    if (value == "Prec")
    {
        return "Precision";
    }

    if (value == "Condi")
    {
        return "Condition";
    }

    if (value == "Exper")
    {
        return "Expertise";
    }

    if (value == "PConc")
    {
        return "Power/Conc.";
    }

    if (value == "CConc")
    {
        return "Condi/Conc.";
    }

    if (value == "HConc")
    {
        return "Heal/Conc.";
    }

    if (value == "TConc")
    {
        return "Tough/Conc.";
    }

    if (value == "Heal")
    {
        return "Healing";
    }

    if (value == "Slay")
    {
        return "Slaying";
    }

    if (value == "All")
    {
        return "All Stats";
    }

    if (value == "MF")
    {
        return "Magic Find";
    }

    if (value == "Move")
    {
        return "Movement";
    }

    if (value == "Kill")
    {
        return "On Kill";
    }

    if (value == "Burn")
    {
        return "Burning";
    }

    if (value == "Bleed")
    {
        return "Bleeding";
    }

    if (value == "Torm")
    {
        return "Torment";
    }

    if (value == "Confu")
    {
        return "Confusion";
    }

    if (value == "Endu")
    {
        return "Endurance";
    }

    if (value == "OnHeal")
    {
        return "On Heal";
    }

    if (value == "Res")
    {
        return "Revive";
    }

    return label;
}
std::string CreateItemChatLink(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return "";
    }

    const std::array<unsigned char, 6> data =
    {
        0x02,
        0x01,
        static_cast<unsigned char>(
            itemID & 0xFF
        ),
        static_cast<unsigned char>(
            (itemID >> 8) & 0xFF
        ),
        static_cast<unsigned char>(
            (itemID >> 16) & 0xFF
        ),
        static_cast<unsigned char>(
            (itemID >> 24) & 0xFF
        )
    };

    static const char base64Table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string encoded;

    for (size_t i = 0;
        i < data.size();
        i += 3)
    {
        const uint32_t value =
            (static_cast<uint32_t>(
                data[i]
                ) << 16) |
            (static_cast<uint32_t>(
                data[i + 1]
                ) << 8) |
            static_cast<uint32_t>(
                data[i + 2]
                );

        encoded +=
            base64Table[
                (value >> 18) & 0x3F
            ];

        encoded +=
            base64Table[
                (value >> 12) & 0x3F
            ];

        encoded +=
            base64Table[
                (value >> 6) & 0x3F
            ];

        encoded +=
            base64Table[
                value & 0x3F
            ];
    }

    return
        "[&" +
        encoded +
        "]";
}

void RenderCompactTracker(
    bool hasFood,
    int64_t foodRemaining,
    bool hasUtility,
    int64_t utilityRemaining,
    bool hasMetabolicPrimer,
    int64_t metabolicPrimerRemaining,
    bool hasUtilityPrimer,
    int64_t utilityPrimerRemaining
);

void RenderConsumableTooltip(
    const ConsumableInfo& info
);

const char* GetSquadConsumableLabel(
    const char* label
);

void RenderGeneralTab();
void RenderSquadTab();
void RenderSessionTab();
void RenderTradingPostTab();
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
    SquadTracker::RestoreUnknownConsumables();
    TradingPostPriceManager::Start();
    TradingPostHistoryManager::Start(hSelf);
    TradingPostItemIndexManager::Start(hSelf);
    TradingPostWatchManager::Start(hSelf);
    ConsumableMetadataManager::Start();
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

    TradingPostWatchManager::Shutdown();
    TradingPostItemIndexManager::Shutdown();
    TradingPostItemIndexManager::Reset();
    TradingPostPriceManager::Shutdown();
    TradingPostPriceManager::Reset();
    TradingPostHistoryManager::Shutdown();
    ConsumableMetadataManager::Shutdown();
    ConsumableMetadataManager::Reset();

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
    int64_t utilityRemaining,
    bool hasMetabolicPrimer,
    int64_t metabolicPrimerRemaining,
    bool hasUtilityPrimer,
    int64_t utilityPrimerRemaining
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
                foodSkillID == 34210
                ? foodInfo.name
                : foodInfo.label
            );

            RenderConsumableTooltip(
                foodInfo
            );

            if (ImGui::BeginPopupContextItem(
                "FoodContextMenu"
            ))
            {
                ImGui::TextDisabled(
                    "Food"
                );

                ImGui::Separator();

                if (foodInfo.itemID != 0)
                {
                    if (ImGui::MenuItem(
                        "Copy item chat link"
                    ))
                    {
                        const std::string chatLink =
                            CreateItemChatLink(
                                foodInfo.itemID
                            );

                        ImGui::SetClipboardText(
                            chatLink.c_str()
                        );
                    }
                }

                if (ImGui::MenuItem(
                    "Copy item name"
                ))
                {
                    ImGui::SetClipboardText(
                        foodInfo.name
                    );
                }

                if (ImGui::MenuItem(
                    "Copy effect ID"
                ))
                {
                    const std::string effectID =
                        std::to_string(
                            foodSkillID
                        );

                    ImGui::SetClipboardText(
                        effectID.c_str()
                    );
                }

                ImGui::EndPopup();
            }
        }
        else
        {
            ImGui::TextColored(
                missingColor,
                "Food:    Not detected"
            );
        }
        // Candy Cane is a separate nourishment-style buff.
// It does not replace the player's normal Food buff.
        if (BuffTracker::HasCandyCane())
        {
            const int64_t candyCaneMs =
                BuffTracker::GetCandyCaneRemainingMilliseconds();

            const int64_t candyCaneSeconds =
                candyCaneMs / 1000;

            const int64_t candyCaneMinutes =
                candyCaneSeconds / 60;

            const int64_t candyCaneRemainingSeconds =
                candyCaneSeconds % 60;

            ImGui::Text(
                "Candy Cane: %02lld:%02lld",
                candyCaneMinutes,
                candyCaneRemainingSeconds
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
            if (ImGui::BeginPopupContextItem(
                "UtilityContextMenu"
            ))
            {
                ImGui::TextDisabled(
                    "Utility"
                );

                ImGui::Separator();

                if (utilityInfo.itemID != 0)
                {
                    if (ImGui::MenuItem(
                        "Copy item chat link"
                    ))
                    {
                        const std::string chatLink =
                            CreateItemChatLink(
                                utilityInfo.itemID
                            );

                        ImGui::SetClipboardText(
                            chatLink.c_str()
                        );
                    }
                }

                if (ImGui::MenuItem(
                    "Copy item name"
                ))
                {
                    ImGui::SetClipboardText(
                        utilityInfo.name
                    );
                }

                if (ImGui::MenuItem(
                    "Copy effect ID"
                ))
                {
                    const std::string effectID =
                        std::to_string(
                            utilitySkillID
                        );

                    ImGui::SetClipboardText(
                        effectID.c_str()
                    );
                }

                ImGui::EndPopup();
            }
        }
        else
        {
            ImGui::TextColored(
                missingColor,
                "Utility: Not detected"
            );
        }
        //
// Metabolic Primer
//
        if (hasMetabolicPrimer)
        {
            const int64_t totalSeconds =
                metabolicPrimerRemaining / 1000;

            const int64_t hours =
                totalSeconds / 3600;

            const int64_t minutes =
                (totalSeconds % 3600) / 60;

            const int64_t seconds =
                totalSeconds % 60;

            const ImVec4* primerColor =
                &normalColor;

            if (totalSeconds <= 60)
            {
                primerColor =
                    &criticalColor;
            }
            else if (
                totalSeconds <=
                g_Settings.metabolicPrimerWarningSeconds)
            {
                primerColor =
                    &warningColor;
            }

            ImGui::TextColored(
                *primerColor,
                "Metabolic: %02lld:%02lld:%02lld",
                hours,
                minutes,
                seconds
            );
        }
        else
        {
            ImGui::TextColored(
                missingColor,
                "Metabolic: Not detected"
            );
        }

        //
        // Utility Primer
        //
        if (hasUtilityPrimer)
        {
            const int64_t totalSeconds =
                utilityPrimerRemaining / 1000;

            const int64_t hours =
                totalSeconds / 3600;

            const int64_t minutes =
                (totalSeconds % 3600) / 60;

            const int64_t seconds =
                totalSeconds % 60;

            const ImVec4* primerColor =
                &normalColor;

            if (totalSeconds <= 60)
            {
                primerColor =
                    &criticalColor;
            }
            else if (
                totalSeconds <=
                g_Settings.utilityPrimerWarningSeconds)
            {
                primerColor =
                    &warningColor;
            }

            ImGui::TextColored(
                *primerColor,
                "Utility P: %02lld:%02lld:%02lld",
                hours,
                minutes,
                seconds
            );
        }
        else
        {
            ImGui::TextColored(
                missingColor,
                "Utility P: Not detected"
            );
        }
    }

    ImGui::End();
}

void AddonRender()
{
    TradingPostWatchManager::Update();

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

    const bool inCombat =
        BuffTracker::IsInCombat();

    const bool isGameplay =
        NexusLink != nullptr &&
        NexusLink->IsGameplay;

    if (isGameplay)
    {
        SessionTracker::Update(
            hasFood,
            BuffTracker::GetFoodSkillID(),
            hasUtility,
            BuffTracker::GetUtilitySkillID(),
            hasMetabolicPrimer,
            hasUtilityPrimer,
            inCombat
        );
    }

    if (g_Settings.showTracker &&
        isGameplay)
    {
        RenderCompactTracker(
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
        ImGuiCond_FirstUseEver,
        ImVec2(0.5f, 0.5f)
    );

    ImGui::SetNextWindowBgAlpha(
        0.90f
    );
    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(24.0f, 18.0f)
    );
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (g_Settings.lockReminderPosition)
    {
        flags |=
            ImGuiWindowFlags_NoMove;
    }

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

        ImGui::SetWindowFontScale(1.35f);

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
            ImGui::SetWindowFontScale(1.15f);

            ImGui::TextUnformatted(
                ReminderManager::
                GetReminderMessage()
            );
        }
        else
        {
            ImGui::SetWindowFontScale(1.20f);

            ImGui::Text(
                "%02lld:%02lld:%02lld remaining",
                hours,
                minutes,
                seconds
            );
        }

        ImGui::SetWindowFontScale(1.0f);
    }

    ImGui::End();

    ImGui::PopStyleVar();
}



namespace
{
    std::string FormatGoldWithCommas(
        uint64_t gold
    )
    {
        std::string value =
            std::to_string(
                gold
            );

        for (
            int i =
            static_cast<int>(
                value.size()
                ) - 3;
            i > 0;
            i -= 3
            )
        {
            value.insert(
                static_cast<size_t>(
                    i
                    ),
                ","
            );
        }

        return value;
    }

    std::string FormatCoinValue(
        uint64_t copper
    )
    {
        const uint64_t gold =
            copper / 10000ULL;

        const uint64_t silver =
            (copper % 10000ULL) / 100ULL;

        const uint64_t copperOnly =
            copper % 100ULL;

        return
            FormatGoldWithCommas(
                gold
            ) +
            "g " +
            std::to_string(
                silver
            ) +
            "s " +
            std::to_string(
                copperOnly
            ) +
            "c";
    }
}

void RenderTradingPostTab()
{
    const ImVec4 goodColor(
        0.35f,
        0.90f,
        0.45f,
        1.00f
    );

    const ImVec4 attentionColor(
        1.00f,
        0.78f,
        0.25f,
        1.00f
    );

    const ImVec4 sellColor(
        0.45f,
        0.90f,
        0.55f,
        1.00f
    );

    const ImVec4 buyColor(
        0.45f,
        0.75f,
        1.00f,
        1.00f
    );

    const ImVec4 trendDownColor(
        1.00f,
        0.40f,
        0.40f,
        1.00f
    );

    const ImVec4 mutedColor(
        0.65f,
        0.65f,
        0.65f,
        1.00f
    );

    const ImVec4 itemNameColor(
        0.92f,
        0.78f,
        1.00f,
        1.00f
    );

    enum class PriceTrend
    {
        Neutral,
        Up,
        Down
    };

    static int trendWindowIndex = 1;

    const uint64_t trendWindowSecondsOptions[] =
    {
        15ULL * 60ULL,
        30ULL * 60ULL,
        60ULL * 60ULL,
        6ULL * 60ULL * 60ULL,
        24ULL * 60ULL * 60ULL
    };

    const char* trendWindowLabels[] =
    {
        "15m",
        "30m",
        "1h",
        "6h",
        "24h"
    };

    const uint64_t trendWindowSeconds =
        trendWindowSecondsOptions[
            trendWindowIndex
        ];

    const char* trendWindowLabel =
        trendWindowLabels[
            trendWindowIndex
        ];

    struct TrendInfo
    {
        PriceTrend trend =
            PriceTrend::Neutral;

        int64_t copperChange =
            0;

        double percentChange =
            0.0;

        bool available =
            false;
    };

    auto CalculateTrend =
        [](
            const std::vector<
            TradingPostHistoryPoint
            >& points,
            bool useSellPrice,
            uint64_t windowSeconds
            )
        {
            TrendInfo result;

            if (points.size() < 2)
            {
                return
                    result;
            }

            const TradingPostHistoryPoint&
                newestPoint =
                points.back();

            if (
                newestPoint.timestampUnixSeconds <
                windowSeconds
                )
            {
                return
                    result;
            }

            const uint64_t targetTimestamp =
                newestPoint.timestampUnixSeconds -
                windowSeconds;

            const TradingPostHistoryPoint*
                olderPoint =
                nullptr;

            //
            // Find the newest observation that is at least
            // 30 minutes older than the latest observation.
            //
            for (
                auto it =
                points.rbegin();
                it !=
                points.rend();
                ++it
                )
            {
                if (
                    it->timestampUnixSeconds <=
                    targetTimestamp
                    )
                {
                    olderPoint =
                        &(*it);

                    break;
                }
            }

            if (olderPoint == nullptr)
            {
                return
                    result;
            }

            const uint32_t olderPrice =
                useSellPrice
                ? olderPoint->
                sellUnitPrice
                : olderPoint->
                buyUnitPrice;

            const uint32_t newerPrice =
                useSellPrice
                ? newestPoint.
                sellUnitPrice
                : newestPoint.
                buyUnitPrice;

            if (olderPrice == 0)
            {
                return
                    result;
            }

            result.available =
                true;

            result.copperChange =
                static_cast<int64_t>(
                    newerPrice
                    ) -
                static_cast<int64_t>(
                    olderPrice
                    );

            result.percentChange =
                static_cast<double>(
                    result.copperChange
                    ) /
                static_cast<double>(
                    olderPrice
                    ) *
                100.0;

            if (
                result.copperChange > 0
                )
            {
                result.trend =
                    PriceTrend::Up;
            }
            else if (
                result.copperChange < 0
                )
            {
                result.trend =
                    PriceTrend::Down;
            }
            else
            {
                result.trend =
                    PriceTrend::Neutral;
            }

            return
                result;
        };

    auto FormatTrendCoinDelta =
        [](
            int64_t copperChange
            )
        {
            const bool negative =
                copperChange < 0;

            uint64_t magnitude =
                negative
                ? static_cast<uint64_t>(
                    -copperChange
                    )
                : static_cast<uint64_t>(
                    copperChange
                    );

            const uint64_t gold =
                magnitude /
                10000ULL;

            const uint64_t silver =
                (
                    magnitude %
                    10000ULL
                    ) /
                100ULL;

            const uint64_t copper =
                magnitude %
                100ULL;

            std::string value =
                negative
                ? "-"
                : "+";

            if (gold > 0)
            {
                value +=
                    FormatGoldWithCommas(
                        gold
                    ) +
                    "g ";

                value +=
                    std::to_string(
                        silver
                    ) +
                    "s ";

                value +=
                    std::to_string(
                        copper
                    ) +
                    "c";

                return
                    value;
            }

            if (silver > 0)
            {
                value +=
                    std::to_string(
                        silver
                    ) +
                    "s ";

                value +=
                    std::to_string(
                        copper
                    ) +
                    "c";

                return
                    value;
            }

            value +=
                std::to_string(
                    copper
                ) +
                "c";

            return
                value;
        };

    struct DealWindowStats
    {
        uint64_t averageSell = 0;
        uint64_t lowerQuartileSell = 0;
        uint64_t upperQuartileSell = 0;
        size_t sampleCount = 0;
        bool available = false;
    };

    auto CalculateDealWindowStats =
        [](
            const std::vector<
            TradingPostHistoryPoint
            >& points,
            uint64_t windowSeconds
            )
        {
            DealWindowStats result;

            if (points.size() < 2)
            {
                return
                    result;
            }

            const uint64_t newestTimestamp =
                points.back().
                timestampUnixSeconds;

            if (
                newestTimestamp <
                windowSeconds
                )
            {
                return
                    result;
            }

            const uint64_t startTimestamp =
                newestTimestamp -
                windowSeconds;

            std::vector<uint32_t>
                sellSamples;

            uint64_t totalSell = 0;

            for (
                const TradingPostHistoryPoint&
                point :
                points
                )
            {
                if (
                    point.timestampUnixSeconds <
                    startTimestamp ||
                    point.sellUnitPrice == 0
                    )
                {
                    continue;
                }

                sellSamples.push_back(
                    point.sellUnitPrice
                );

                totalSell +=
                    point.sellUnitPrice;
            }

            if (
                sellSamples.size() < 4
                )
            {
                return
                    result;
            }

            std::sort(
                sellSamples.begin(),
                sellSamples.end()
            );

            result.sampleCount =
                sellSamples.size();

            result.averageSell =
                totalSell /
                static_cast<uint64_t>(
                    sellSamples.size()
                    );

            const size_t lowerIndex =
                (
                    sellSamples.size() -
                    1
                    ) /
                4;

            const size_t upperIndex =
                (
                    (
                        sellSamples.size() -
                        1
                        ) *
                    3
                    ) /
                4;

            result.lowerQuartileSell =
                sellSamples[
                    lowerIndex
                ];

            result.upperQuartileSell =
                sellSamples[
                    upperIndex
                ];

            result.available =
                result.averageSell > 0;

            return
                result;
        };

    auto CalculateWindowAverageSell =
        [](
            const std::vector<
            TradingPostHistoryPoint
            >& points,
            uint64_t windowSeconds,
            uint64_t& outAverageSell
            )
        {
            outAverageSell = 0;

            if (points.size() < 2)
            {
                return false;
            }

            const uint64_t newestTimestamp =
                points.back().
                timestampUnixSeconds;

            if (
                newestTimestamp <
                windowSeconds
                )
            {
                return false;
            }

            const uint64_t startTimestamp =
                newestTimestamp -
                windowSeconds;

            uint64_t totalSell = 0;
            uint64_t sampleCount = 0;

            for (
                const TradingPostHistoryPoint&
                point :
                points
                )
            {
                if (
                    point.timestampUnixSeconds <
                    startTimestamp
                    )
                {
                    continue;
                }

                if (
                    point.sellUnitPrice == 0
                    )
                {
                    continue;
                }

                totalSell +=
                    point.sellUnitPrice;

                ++sampleCount;
            }

            if (sampleCount == 0)
            {
                return false;
            }

            outAverageSell =
                totalSell /
                sampleCount;

            return
                outAverageSell > 0;
        };

    auto DrawTrendText =
        [&](
            const char* label,
            const TrendInfo& trend,
            const ImVec4& labelColor,
            const char* windowLabel
            )
        {
            ImGui::TextColored(
                labelColor,
                "%s",
                label
            );

            ImGui::SameLine(
                0.0f,
                5.0f
            );

            if (!trend.available)
            {
                ImGui::TextDisabled(
                    "- collecting %s history",
                    windowLabel
                );

                return;
            }

            const char* arrowText =
                "-";

            ImVec4 stateColor =
                attentionColor;

            if (
                trend.trend ==
                PriceTrend::Up
                )
            {
                arrowText =
                    "^";

                stateColor =
                    goodColor;
            }
            else if (
                trend.trend ==
                PriceTrend::Down
                )
            {
                arrowText =
                    "v";

                stateColor =
                    trendDownColor;
            }

            if (
                trend.trend ==
                PriceTrend::Neutral
                )
            {
                ImGui::TextColored(
                    stateColor,
                    "%s 0c (0.00%%) over %s",
                    arrowText,
                    windowLabel
                );

                return;
            }

            const std::string deltaText =
                FormatTrendCoinDelta(
                    trend.copperChange
                );

            ImGui::TextColored(
                stateColor,
                "%s %s (%+.2f%%) over %s",
                arrowText,
                deltaText.c_str(),
                trend.percentChange,
                windowLabel
            );
        };

    auto DrawSparkline =
        [](
            const char* id,
            const std::vector<float>& values,
            const ImVec4& lineColor,
            float width,
            float height
            )
        {
            if (values.size() < 2)
            {
                return;
            }

            float minValue =
                values.front();

            float maxValue =
                values.front();

            for (
                const float value :
            values
                )
            {
                if (value < minValue)
                {
                    minValue =
                        value;
                }

                if (value > maxValue)
                {
                    maxValue =
                        value;
                }
            }

            const float range =
                maxValue -
                minValue;

            const float padding =
                range > 0.0f
                ? range * 0.10f
                : (
                    maxValue > 0.0f
                    ? maxValue * 0.005f
                    : 1.0f
                    );

            const float scaleMin =
                minValue -
                padding;

            const float scaleMax =
                maxValue +
                padding;

            const ImVec2 start =
                ImGui::
                GetCursorScreenPos();

            ImGui::InvisibleButton(
                id,
                ImVec2(
                    width,
                    height
                )
            );

            ImDrawList* drawList =
                ImGui::
                GetWindowDrawList();

            const ImU32 faintLineColor =
                ImGui::
                ColorConvertFloat4ToU32(
                    ImVec4(
                        1.0f,
                        1.0f,
                        1.0f,
                        0.08f
                    )
                );

            drawList->AddLine(
                ImVec2(
                    start.x,
                    start.y +
                    height * 0.5f
                ),
                ImVec2(
                    start.x +
                    width,
                    start.y +
                    height * 0.5f
                ),
                faintLineColor,
                1.0f
            );

            std::vector<ImVec2>
                points;

            points.reserve(
                values.size()
            );

            const float denominator =
                scaleMax -
                scaleMin;

            for (
                size_t i = 0;
                i < values.size();
                ++i
                )
            {
                const float x =
                    start.x +
                    (
                        static_cast<float>(
                            i
                            ) /
                        static_cast<float>(
                            values.size() - 1
                            )
                        ) *
                    width;

                float normalized =
                    0.5f;

                if (denominator > 0.0f)
                {
                    normalized =
                        (
                            values[i] -
                            scaleMin
                            ) /
                        denominator;
                }

                const float y =
                    start.y +
                    height -
                    (
                        normalized *
                        height
                        );

                points.emplace_back(
                    x,
                    y
                );
            }

            drawList->AddPolyline(
                points.data(),
                static_cast<int>(
                    points.size()
                    ),
                ImGui::
                ColorConvertFloat4ToU32(
                    lineColor
                ),
                false,
                2.0f
            );
        };

    ImGui::TextUnformatted(
        "Trading Post Watcher"
    );

    ImGui::TextDisabled(
        "Watch lowest sell listings and alert when an item reaches your Sell Target."
    );

    TradingPostTargetAlert activeTargetAlert;

    const bool hasActiveTargetAlert =
        TradingPostWatchManager::
        TryGetActiveTargetAlert(
            activeTargetAlert
        );

    if (hasActiveTargetAlert)
    {
        ImGui::Spacing();

        const float celebrationTime =
            static_cast<float>(
                ImGui::GetTime()
                );

        const float pulse =
            0.5f +
            0.5f *
            std::sin(
                celebrationTime *
                5.0f
            );

        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            ImVec4(
                0.10f,
                0.16f,
                0.05f,
                0.96f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_Border,
            ImVec4(
                0.65f +
                0.30f * pulse,
                0.80f +
                0.18f * pulse,
                0.16f,
                1.00f
            )
        );

        ImGui::BeginChild(
            "##TradingPostTargetCelebration",
            ImVec2(
                0.0f,
                142.0f
            ),
            true,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        ImDrawList* celebrationDrawList =
            ImGui::GetWindowDrawList();

        const ImVec2 celebrationMin =
            ImGui::GetWindowPos();

        const ImVec2 celebrationSize =
            ImGui::GetWindowSize();

        const ImVec2 celebrationMax(
            celebrationMin.x +
            celebrationSize.x,
            celebrationMin.y +
            celebrationSize.y
        );

        celebrationDrawList->
            PushClipRect(
                celebrationMin,
                celebrationMax,
                true
            );

        //
        // Animated confetti.
        //
        const ImU32 confettiColors[] =
        {
            IM_COL32(
                255, 196, 40, 255
            ),
            IM_COL32(
                80, 220, 110, 255
            ),
            IM_COL32(
                90, 170, 255, 255
            ),
            IM_COL32(
                220, 90, 255, 255
            ),
            IM_COL32(
                255, 90, 90, 255
            ),
            IM_COL32(
                255, 145, 35, 255
            )
        };

        constexpr int
            CONFETTI_COUNT =
            44;

        for (
            int i = 0;
            i < CONFETTI_COUNT;
            ++i
            )
        {
            const float seed =
                static_cast<float>(
                    i
                    );

            const float xFraction =
                std::fmod(
                    seed *
                    0.6180339f,
                    1.0f
                );

            const float fallSpeed =
                24.0f +
                std::fmod(
                    seed *
                    11.0f,
                    34.0f
                );

            const float y =
                celebrationMin.y +
                std::fmod(
                    celebrationTime *
                    fallSpeed +
                    seed *
                    19.0f,
                    celebrationSize.y +
                    18.0f
                ) -
                9.0f;

            const float sway =
                std::sin(
                    celebrationTime *
                    2.0f +
                    seed
                ) *
                8.0f;

            const float x =
                celebrationMin.x +
                xFraction *
                celebrationSize.x +
                sway;

            const float pieceSize =
                2.5f +
                std::fmod(
                    seed,
                    3.0f
                );

            celebrationDrawList->
                AddRectFilled(
                    ImVec2(
                        x -
                        pieceSize,
                        y -
                        1.5f
                    ),
                    ImVec2(
                        x +
                        pieceSize,
                        y +
                        1.5f
                    ),
                    confettiColors[
                        i %
                            (
                                sizeof(
                                    confettiColors
                                    ) /
                                sizeof(
                                    confettiColors[
                                        0
                                    ]
                                    )
                                )
                    ],
                    1.0f
                );
        }

        //
        // Firework-style sparkle bursts.
        //
        for (
            int burstIndex = 0;
            burstIndex < 3;
            ++burstIndex
            )
        {
            const float centerX =
                celebrationMin.x +
                celebrationSize.x *
                (
                    0.18f +
                    0.32f *
                    static_cast<float>(
                        burstIndex
                        )
                    );

            const float centerY =
                celebrationMin.y +
                34.0f +
                6.0f *
                std::sin(
                    celebrationTime *
                    2.4f +
                    static_cast<float>(
                        burstIndex
                        )
                );

            const float radius =
                9.0f +
                5.0f * pulse;

            for (
                int ray = 0;
                ray < 8;
                ++ray
                )
            {
                const float angle =
                    static_cast<float>(
                        ray
                        ) *
                    0.78539816f +
                    celebrationTime *
                    0.20f;

                const ImVec2 rayEnd(
                    centerX +
                    std::cos(
                        angle
                    ) *
                    radius,
                    centerY +
                    std::sin(
                        angle
                    ) *
                    radius
                );

                celebrationDrawList->
                    AddLine(
                        ImVec2(
                            centerX,
                            centerY
                        ),
                        rayEnd,
                        IM_COL32(
                            255,
                            215,
                            70,
                            190
                        ),
                        1.3f
                    );
            }

            celebrationDrawList->
                AddCircleFilled(
                    ImVec2(
                        centerX,
                        centerY
                    ),
                    2.3f +
                    1.5f *
                    pulse,
                    IM_COL32(
                        255,
                        245,
                        170,
                        255
                    )
                );
        }

        //
        // Pulsing festival frame.
        //
        celebrationDrawList->
            AddRect(
                ImVec2(
                    celebrationMin.x +
                    2.0f,
                    celebrationMin.y +
                    2.0f
                ),
                ImVec2(
                    celebrationMax.x -
                    2.0f,
                    celebrationMax.y -
                    2.0f
                ),
                ImGui::GetColorU32(
                    ImVec4(
                        0.60f +
                        0.35f * pulse,
                        0.82f +
                        0.15f * pulse,
                        0.10f,
                        0.95f
                    )
                ),
                5.0f,
                0,
                2.0f +
                1.5f * pulse
            );

        celebrationDrawList->
            PopClipRect();

        ImGui::SetWindowFontScale(
            1.55f
        );

        const char*
            celebrationTitle =
            "TARGET REACHED!";

        const float titleWidth =
            ImGui::CalcTextSize(
                celebrationTitle
            ).x;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                titleWidth
                ) *
            0.5f
        );

        ImGui::TextColored(
            ImVec4(
                1.00f,
                0.82f +
                0.12f * pulse,
                0.18f,
                1.00f
            ),
            "%s",
            celebrationTitle
        );

        ImGui::SetWindowFontScale(
            1.00f
        );

        const std::string
            celebrationItemLine =
            activeTargetAlert.name;

        const float itemLineWidth =
            ImGui::CalcTextSize(
                celebrationItemLine.c_str()
            ).x;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                itemLineWidth
                ) *
            0.5f
        );

        ImGui::TextColored(
            itemNameColor,
            "%s",
            celebrationItemLine.c_str()
        );

        const std::string
            sellText =
            FormatCoinValue(
                activeTargetAlert.sellUnitPrice
            );

        const std::string
            targetText =
            FormatCoinValue(
                activeTargetAlert.targetSellCopper
            );

        const std::string
            celebrationPriceLine =
            "SELL " +
            sellText +
            "   <=   SELL TARGET " +
            targetText;

        const float priceLineWidth =
            ImGui::CalcTextSize(
                celebrationPriceLine.c_str()
            ).x;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                priceLineWidth
                ) *
            0.5f
        );

        ImGui::TextColored(
            sellColor,
            "%s",
            celebrationPriceLine.c_str()
        );

        ImGui::Spacing();

        const float dismissWidth =
            118.0f;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                dismissWidth
                ) *
            0.5f
        );

        if (
            ImGui::Button(
                "Dismiss Party",
                ImVec2(
                    dismissWidth,
                    0.0f
                )
            )
            )
        {
            TradingPostWatchManager::
                DismissActiveTargetAlert();
        }

        ImGui::EndChild();

        ImGui::PopStyleColor(
            2
        );

        ImGui::Spacing();
    }

    ImGui::Spacing();

    bool autoWatch =
        TradingPostWatchManager::
        IsAutoWatchEnabled();

    if (
        ImGui::Checkbox(
            "Auto watch every 60 seconds",
            &autoWatch
        )
        )
    {
        TradingPostWatchManager::
            SetAutoWatchEnabled(
                autoWatch
            );
    }

    ImGui::SameLine();

    if (
        ImGui::Button(
            "Refresh All"
        )
        )
    {
        TradingPostWatchManager::
            RefreshAll();
    }

    ImGui::SameLine();

    const size_t indexItemCount =
        TradingPostItemIndexManager::
        GetItemCount();

    if (indexItemCount > 0)
    {
        ImGui::TextDisabled(
            "%llu items searchable",
            static_cast<
            unsigned long long
            >(
                indexItemCount
                )
        );
    }

    const int64_t topNextCheckSeconds =
        TradingPostWatchManager::
        GetSecondsUntilNextCheck();

    if (
        TradingPostWatchManager::
        IsAutoWatchEnabled()
        )
    {
        ImGui::Spacing();

        ImGui::TextColored(
            ImVec4(
                1.00f,
                0.72f,
                0.18f,
                1.00f
            ),
            "NEXT API REFRESH: %lld SEC",
            topNextCheckSeconds
        );
    }

    ImGui::Spacing();

    ImGui::SetNextItemWidth(
        90.0f
    );

    ImGui::Combo(
        "Trend Window",
        &trendWindowIndex,
        trendWindowLabels,
        static_cast<int>(
            sizeof(
                trendWindowLabels
                ) /
            sizeof(
                trendWindowLabels[
                    0
                ]
                )
            )
    );

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    std::vector<
        TradingPostWatchItem
    > watchedItems =
        TradingPostWatchManager::
        GetItems();

    if (watchedItems.empty())
    {
        ImGui::TextDisabled(
            "No watched items."
        );
    }

    for (
        const TradingPostWatchItem& item :
        watchedItems
        )
    {
        ImGui::PushID(
            static_cast<int>(
                item.itemID
                )
        );

        TradingPostPrice price;

        const bool hasPrice =
            TradingPostPriceManager::
            TryGetPrice(
                item.itemID,
                price
            );

        const std::vector<
            TradingPostHistoryPoint
        > history =
            TradingPostHistoryManager::
            GetHistory(
                item.itemID
            );

        const TrendInfo sellTrend =
            CalculateTrend(
                history,
                true,
                trendWindowSeconds
            );

        const TrendInfo buyTrend =
            CalculateTrend(
                history,
                false,
                trendWindowSeconds
            );

        std::string sellText =
            "Loading...";

        std::string buyText =
            "Loading...";

        uint64_t ageSeconds = 0;

        if (hasPrice)
        {
            sellText =
                FormatCoinValue(
                    price.sellUnitPrice
                );

            buyText =
                FormatCoinValue(
                    price.buyUnitPrice
                );

            if (
                price.lastUpdatedUnixSeconds >
                0
                )
            {
                const uint64_t nowUnixSeconds =
                    static_cast<uint64_t>(
                        std::chrono::
                        duration_cast<
                        std::chrono::seconds
                        >(
                            std::chrono::
                            system_clock::
                            now().
                            time_since_epoch()
                        ).count()
                        );

                ageSeconds =
                    nowUnixSeconds >=
                    price.lastUpdatedUnixSeconds
                    ? nowUnixSeconds -
                    price.lastUpdatedUnixSeconds
                    : 0;
            }
        }
        else
        {
            TradingPostPriceManager::
                RequestPrice(
                    item.itemID
                );
        }

        uint64_t targetCopper =
            item.targetSellCopper;

        int targetGold =
            static_cast<int>(
                targetCopper /
                10000ULL
                );

        int targetSilver =
            static_cast<int>(
                (
                    targetCopper %
                    10000ULL
                    ) /
                100ULL
                );

        int targetCopperOnly =
            static_cast<int>(
                targetCopper %
                100ULL
                );

        bool targetChanged =
            false;

        ImGui::PushStyleVar(
            ImGuiStyleVar_CellPadding,
            ImVec2(
                7.0f,
                5.0f
            )
        );

        if (
            ImGui::BeginTable(
                "##WatchItemSummary",
                5,
                ImGuiTableFlags_SizingStretchProp |
                ImGuiTableFlags_NoSavedSettings
            )
            )
        {
            ImGui::TableSetupColumn(
                "Item",
                ImGuiTableColumnFlags_WidthStretch,
                1.65f
            );

            ImGui::TableSetupColumn(
                "Sell",
                ImGuiTableColumnFlags_WidthStretch,
                1.15f
            );

            ImGui::TableSetupColumn(
                "Buy",
                ImGuiTableColumnFlags_WidthStretch,
                1.15f
            );

            ImGui::TableSetupColumn(
                "Sell Target",
                ImGuiTableColumnFlags_WidthFixed,
                190.0f
            );

            ImGui::TableSetupColumn(
                "Status",
                ImGuiTableColumnFlags_WidthFixed,
                128.0f
            );

            ImGui::TableNextRow();

            if (
                hasActiveTargetAlert &&
                activeTargetAlert.itemID ==
                item.itemID
                )
            {
                const float rowPulse =
                    0.5f +
                    0.5f *
                    std::sin(
                        static_cast<float>(
                            ImGui::GetTime()
                            ) *
                        5.0f
                    );

                ImGui::TableSetBgColor(
                    ImGuiTableBgTarget_RowBg0,
                    ImGui::GetColorU32(
                        ImVec4(
                            0.12f,
                            0.30f +
                            0.08f * rowPulse,
                            0.08f,
                            0.52f +
                            0.18f * rowPulse
                        )
                    )
                );
            }

            ImGui::TableSetColumnIndex(
                0
            );

            ImGui::PushStyleColor(
                ImGuiCol_Text,
                itemNameColor
            );

            ImGui::TextWrapped(
                "%s",
                item.name.c_str()
            );

            ImGui::PopStyleColor();

            if (hasPrice)
            {
                ImGui::TextDisabled(
                    "Item ID: %u  |  Updated %llu sec ago",
                    item.itemID,
                    ageSeconds
                );
            }
            else
            {
                ImGui::TextDisabled(
                    "Item ID: %u  |  Price data loading...",
                    item.itemID
                );
            }

            ImGui::TableSetColumnIndex(
                1
            );

            ImGui::TextDisabled(
                "Sell"
            );

            ImGui::TextColored(
                sellColor,
                "%s",
                sellText.c_str()
            );

            ImGui::TableSetColumnIndex(
                2
            );

            ImGui::TextDisabled(
                "Buy"
            );

            ImGui::TextColored(
                buyColor,
                "%s",
                buyText.c_str()
            );

            ImGui::TableSetColumnIndex(
                3
            );

            ImGui::TextDisabled(
                "Sell Target"
            );

            ImGui::SetNextItemWidth(
                52.0f
            );

            if (
                ImGui::InputInt(
                    "##TargetGold",
                    &targetGold,
                    0,
                    0
                )
                )
            {
                targetChanged =
                    true;
            }

            ImGui::SameLine(
                0.0f,
                3.0f
            );

            ImGui::TextDisabled(
                "g"
            );

            ImGui::SameLine(
                0.0f,
                6.0f
            );

            ImGui::SetNextItemWidth(
                42.0f
            );

            if (
                ImGui::InputInt(
                    "##TargetSilver",
                    &targetSilver,
                    0,
                    0
                )
                )
            {
                targetChanged =
                    true;
            }

            ImGui::SameLine(
                0.0f,
                3.0f
            );

            ImGui::TextDisabled(
                "s"
            );

            ImGui::SameLine(
                0.0f,
                6.0f
            );

            ImGui::SetNextItemWidth(
                42.0f
            );

            if (
                ImGui::InputInt(
                    "##TargetCopper",
                    &targetCopperOnly,
                    0,
                    0
                )
                )
            {
                targetChanged =
                    true;
            }

            ImGui::SameLine(
                0.0f,
                3.0f
            );

            ImGui::TextDisabled(
                "c"
            );

            ImGui::TableSetColumnIndex(
                4
            );

            ImGui::TextDisabled(
                "Status"
            );

            if (
                item.targetSellCopper ==
                0
                )
            {
                ImGui::TextDisabled(
                    "NO TARGET"
                );
            }
            else if (!hasPrice)
            {
                ImGui::TextDisabled(
                    "WAITING"
                );
            }
            else if (
                static_cast<uint64_t>(
                    price.sellUnitPrice
                    ) <=
                item.targetSellCopper
                )
            {
                ImGui::TextColored(
                    goodColor,
                    "TARGET REACHED"
                );
            }
            else
            {
                ImGui::TextColored(
                    attentionColor,
                    "WAIT"
                );
            }

            if (
                ImGui::SmallButton(
                    "Refresh"
                )
                )
            {
                TradingPostWatchManager::
                    RefreshItem(
                        item.itemID
                    );
            }

            if (!item.isDefault)
            {
                ImGui::SameLine();

                if (
                    ImGui::SmallButton(
                        "Remove"
                    )
                    )
                {
                    TradingPostWatchManager::
                        RemoveItem(
                            item.itemID
                        );
                }
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();

        if (targetGold < 0)
        {
            targetGold = 0;
        }

        if (targetSilver < 0)
        {
            targetSilver = 0;
        }

        if (targetSilver > 99)
        {
            targetSilver = 99;
        }

        if (targetCopperOnly < 0)
        {
            targetCopperOnly = 0;
        }

        if (targetCopperOnly > 99)
        {
            targetCopperOnly = 99;
        }

        if (targetChanged)
        {
            const uint64_t newTargetCopper =
                static_cast<uint64_t>(
                    targetGold
                    ) *
                10000ULL +
                static_cast<uint64_t>(
                    targetSilver
                    ) *
                100ULL +
                static_cast<uint64_t>(
                    targetCopperOnly
                    );

            TradingPostWatchManager::
                SetTargetSellPrice(
                    item.itemID,
                    newTargetCopper
                );
        }

        const std::string historyHeader =
            "History (" +
            std::to_string(
                history.size()
            ) +
            " observations)###History" +
            std::to_string(
                item.itemID
            );

        const bool historyOpen =
            ImGui::CollapsingHeader(
                historyHeader.c_str()
            );

        ImGui::Indent(
            18.0f
        );

        DrawTrendText(
            "Sell",
            sellTrend,
            sellColor,
            trendWindowLabel
        );

        DrawTrendText(
            "Buy",
            buyTrend,
            buyColor,
            trendWindowLabel
        );

        if (
            hasPrice &&
            price.available &&
            price.sellUnitPrice > 0
            )
        {
            const DealWindowStats dealStats =
                CalculateDealWindowStats(
                    history,
                    trendWindowSeconds
                );

            if (
                sellTrend.available &&
                dealStats.available
                )
            {
                const int64_t dealCopperDelta =
                    static_cast<int64_t>(
                        price.sellUnitPrice
                        ) -
                    static_cast<int64_t>(
                        dealStats.averageSell
                        );

                const double dealPercent =
                    static_cast<double>(
                        dealCopperDelta
                        ) /
                    static_cast<double>(
                        dealStats.averageSell
                        ) *
                    100.0;

                const char* dealLabel =
                    "TYPICAL";

                ImVec4 dealColor =
                    attentionColor;

                //
                // Price-scale independent classification:
                // FAVORABLE = current sell is in the cheapest
                // quarter of prices seen in this selected window.
                // EXPENSIVE = current sell is in the highest
                // quarter. Everything between is TYPICAL.
                //
                if (
                    price.sellUnitPrice <=
                    dealStats.lowerQuartileSell
                    )
                {
                    dealLabel =
                        "FAVORABLE";

                    dealColor =
                        goodColor;
                }
                else if (
                    price.sellUnitPrice >=
                    dealStats.upperQuartileSell
                    )
                {
                    dealLabel =
                        "EXPENSIVE";

                    dealColor =
                        trendDownColor;
                }

                ImGui::TextColored(
                    dealColor,
                    "Deal %s  %+.2f%% vs %s avg",
                    dealLabel,
                    dealPercent,
                    trendWindowLabel
                );
            }
            else
            {
                ImGui::TextDisabled(
                    "Deal - collecting %s history",
                    trendWindowLabel
                );
            }

            if (
                price.buyUnitPrice > 0 &&
                price.sellUnitPrice >=
                price.buyUnitPrice
                )
            {
                const uint64_t spreadCopper =
                    static_cast<uint64_t>(
                        price.sellUnitPrice -
                        price.buyUnitPrice
                        );

                const double spreadPercent =
                    static_cast<double>(
                        spreadCopper
                        ) /
                    static_cast<double>(
                        price.sellUnitPrice
                        ) *
                    100.0;

                const std::string spreadText =
                    FormatCoinValue(
                        spreadCopper
                    );

                ImGui::TextDisabled(
                    "Spread %s (%.2f%% of sell)",
                    spreadText.c_str(),
                    spreadPercent
                );
            }
        }

        ImGui::Unindent(
            18.0f
        );

        if (historyOpen)
        {
            if (history.empty())
            {
                ImGui::TextDisabled(
                    "No saved price observations yet."
                );
            }
            else
            {
                uint32_t minBuy =
                    history.front().
                    buyUnitPrice;

                uint32_t maxBuy =
                    history.front().
                    buyUnitPrice;

                uint32_t minSell =
                    history.front().
                    sellUnitPrice;

                uint32_t maxSell =
                    history.front().
                    sellUnitPrice;

                uint64_t totalBuy = 0;
                uint64_t totalSell = 0;

                for (
                    const TradingPostHistoryPoint&
                    point :
                    history
                    )
                {
                    if (
                        point.buyUnitPrice <
                        minBuy
                        )
                    {
                        minBuy =
                            point.buyUnitPrice;
                    }

                    if (
                        point.buyUnitPrice >
                        maxBuy
                        )
                    {
                        maxBuy =
                            point.buyUnitPrice;
                    }

                    if (
                        point.sellUnitPrice <
                        minSell
                        )
                    {
                        minSell =
                            point.sellUnitPrice;
                    }

                    if (
                        point.sellUnitPrice >
                        maxSell
                        )
                    {
                        maxSell =
                            point.sellUnitPrice;
                    }

                    totalBuy +=
                        point.buyUnitPrice;

                    totalSell +=
                        point.sellUnitPrice;
                }

                const uint64_t averageBuy =
                    totalBuy /
                    static_cast<uint64_t>(
                        history.size()
                        );

                const uint64_t averageSell =
                    totalSell /
                    static_cast<uint64_t>(
                        history.size()
                        );

                const std::string minBuyText =
                    FormatCoinValue(
                        minBuy
                    );

                const std::string maxBuyText =
                    FormatCoinValue(
                        maxBuy
                    );

                const std::string averageBuyText =
                    FormatCoinValue(
                        averageBuy
                    );

                const std::string minSellText =
                    FormatCoinValue(
                        minSell
                    );

                const std::string maxSellText =
                    FormatCoinValue(
                        maxSell
                    );

                const std::string averageSellText =
                    FormatCoinValue(
                        averageSell
                    );

                std::vector<float>
                    buyHistoryGold;

                std::vector<float>
                    sellHistoryGold;

                buyHistoryGold.reserve(
                    history.size()
                );

                sellHistoryGold.reserve(
                    history.size()
                );

                for (
                    const TradingPostHistoryPoint&
                    point :
                    history
                    )
                {
                    buyHistoryGold.push_back(
                        static_cast<float>(
                            point.buyUnitPrice
                            ) /
                        10000.0f
                    );

                    sellHistoryGold.push_back(
                        static_cast<float>(
                            point.sellUnitPrice
                            ) /
                        10000.0f
                    );
                }

                ImGui::Indent(
                    10.0f
                );

                if (
                    ImGui::BeginTable(
                        "##HistoryDetails",
                        2,
                        ImGuiTableFlags_SizingStretchProp |
                        ImGuiTableFlags_NoSavedSettings
                    )
                    )
                {
                    ImGui::TableSetupColumn(
                        "Charts",
                        ImGuiTableColumnFlags_WidthStretch,
                        1.00f
                    );

                    ImGui::TableSetupColumn(
                        "Stats",
                        ImGuiTableColumnFlags_WidthFixed,
                        190.0f
                    );

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(
                        0
                    );

                    ImGui::TextColored(
                        sellColor,
                        "Sell history"
                    );

                    DrawSparkline(
                        "##SellHistorySparkline",
                        sellHistoryGold,
                        sellColor,
                        ImGui::GetContentRegionAvail().x,
                        38.0f
                    );

                    ImGui::Spacing();

                    ImGui::TextColored(
                        buyColor,
                        "Buy history"
                    );

                    DrawSparkline(
                        "##BuyHistorySparkline",
                        buyHistoryGold,
                        buyColor,
                        ImGui::GetContentRegionAvail().x,
                        38.0f
                    );

                    ImGui::TableSetColumnIndex(
                        1
                    );

                    ImGui::TextColored(
                        sellColor,
                        "Sell"
                    );

                    ImGui::Text(
                        "Min  %s",
                        minSellText.c_str()
                    );

                    ImGui::Text(
                        "Avg  %s",
                        averageSellText.c_str()
                    );

                    ImGui::Text(
                        "Max  %s",
                        maxSellText.c_str()
                    );

                    ImGui::Spacing();

                    ImGui::TextColored(
                        buyColor,
                        "Buy"
                    );

                    ImGui::Text(
                        "Min  %s",
                        minBuyText.c_str()
                    );

                    ImGui::Text(
                        "Avg  %s",
                        averageBuyText.c_str()
                    );

                    ImGui::Text(
                        "Max  %s",
                        maxBuyText.c_str()
                    );

                    ImGui::EndTable();
                }

                ImGui::Unindent(
                    10.0f
                );
            }
        }

        // Add a little breathing room between watched items
        // so each item reads as its own block.
        ImGui::Dummy(
            ImVec2(
                0.0f,
                4.0f
            )
        );

        ImGui::Separator();

        ImGui::Dummy(
            ImVec2(
                0.0f,
                8.0f
            )
        );

        ImGui::PopID();
    }

    ImGui::TextUnformatted(
        "Add Watched Item"
    );

    static char itemSearchInput[160] = {};
    static std::string searchStatus;
    static bool searchStatusError = false;

    static std::string lastSearchText;
    static size_t lastSearchIndexCount = 0;

    static std::vector<
        TradingPostIndexedItem
    > searchResults;

    const bool indexReady =
        TradingPostItemIndexManager::
        IsReady();

    const bool indexBuilding =
        TradingPostItemIndexManager::
        IsBuilding();

    const size_t buildProcessed =
        TradingPostItemIndexManager::
        GetBuildProcessedCount();

    const size_t buildTotal =
        TradingPostItemIndexManager::
        GetBuildTotalCount();

    if (indexBuilding)
    {
        if (buildTotal > 0)
        {
            ImGui::TextDisabled(
                "Building item search index: %llu / %llu",
                static_cast<
                unsigned long long
                >(
                    buildProcessed
                    ),
                static_cast<
                unsigned long long
                >(
                    buildTotal
                    )
            );
        }
        else
        {
            ImGui::TextDisabled(
                "Loading Trading Post item list..."
            );
        }
    }
    else if (indexReady)
    {
        ImGui::TextDisabled(
            "%llu Trading Post items available to search.",
            static_cast<
            unsigned long long
            >(
                indexItemCount
                )
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Item search index is not ready yet."
        );
    }

    const std::string indexError =
        TradingPostItemIndexManager::
        GetLastError();

    if (!indexError.empty())
    {
        ImGui::TextColored(
            trendDownColor,
            "%s",
            indexError.c_str()
        );
    }

    ImGui::SetNextItemWidth(
        420.0f
    );

    const bool enterPressed =
        ImGui::InputText(
            "Search item name##NewWatch",
            itemSearchInput,
            sizeof(
                itemSearchInput
                ),
            ImGuiInputTextFlags_EnterReturnsTrue
        );

    ImGui::TextDisabled(
        "Start typing an item name. Matching Trading Post items appear below."
    );

    const std::string currentSearchText =
        itemSearchInput;

    if (
        currentSearchText !=
        lastSearchText ||
        indexItemCount !=
        lastSearchIndexCount
        )
    {
        const bool searchTextChanged =
            currentSearchText !=
            lastSearchText;

        lastSearchText =
            currentSearchText;

        lastSearchIndexCount =
            indexItemCount;

        if (searchTextChanged)
        {
            searchStatus.clear();
            searchStatusError =
                false;
        }

        searchResults.clear();

        if (
            currentSearchText.size() >= 2 &&
            indexReady
            )
        {
            searchResults =
                TradingPostItemIndexManager::
                Search(
                    currentSearchText,
                    8
                );
        }
    }

    bool selectedSearchItem =
        false;

    TradingPostIndexedItem
        selectedItem;

    if (
        currentSearchText.size() >= 2 &&
        indexReady
        )
    {
        if (searchResults.empty())
        {
            ImGui::TextDisabled(
                "No matching Trading Post items."
            );
        }
        else
        {
            const float rowHeight =
                ImGui::
                GetTextLineHeightWithSpacing();

            const float suggestionHeight =
                rowHeight *
                static_cast<float>(
                    searchResults.size()
                    ) +
                8.0f;

            if (
                ImGui::BeginChild(
                    "##TradingPostItemSuggestions",
                    ImVec2(
                        520.0f,
                        suggestionHeight
                    ),
                    true
                )
                )
            {
                for (
                    const TradingPostIndexedItem&
                    result :
                    searchResults
                    )
                {
                    const std::string label =
                        result.name +
                        "  (Item ID: " +
                        std::to_string(
                            result.itemID
                        ) +
                        ")##TPResult" +
                        std::to_string(
                            result.itemID
                        );

                    if (
                        ImGui::Selectable(
                            label.c_str()
                        )
                        )
                    {
                        selectedItem =
                            result;

                        selectedSearchItem =
                            true;

                        break;
                    }
                }
            }

            ImGui::EndChild();
        }
    }

    if (
        enterPressed &&
        !searchResults.empty()
        )
    {
        selectedItem =
            searchResults.front();

        selectedSearchItem =
            true;
    }

    if (selectedSearchItem)
    {
        const bool added =
            TradingPostWatchManager::
            AddItem(
                selectedItem.itemID,
                selectedItem.name
            );

        if (added)
        {
            searchStatus =
                "Added: " +
                selectedItem.name +
                " (Item ID " +
                std::to_string(
                    selectedItem.itemID
                ) +
                ")";

            searchStatusError =
                false;

            itemSearchInput[0] =
                '\0';

            lastSearchText.clear();

            searchResults.clear();
        }
        else
        {
            searchStatus =
                selectedItem.name +
                " is already on the watch list.";

            searchStatusError =
                true;
        }
    }

    if (!searchStatus.empty())
    {
        if (searchStatusError)
        {
            ImGui::TextColored(
                trendDownColor,
                "%s",
                searchStatus.c_str()
            );
        }
        else
        {
            ImGui::TextColored(
                goodColor,
                "%s",
                searchStatus.c_str()
            );
        }
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Watch list and target prices are saved automatically."
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

    std::vector<
        SquadTrackedPlayer
    > trackedPlayers =
        SquadTracker::GetPlayers();

    std::sort(
        trackedPlayers.begin(),
        trackedPlayers.end(),
        [](
            const SquadTrackedPlayer& a,
            const SquadTrackedPlayer& b
            )
        {
            if (a.subgroup != b.subgroup)
            {
                return a.subgroup < b.subgroup;
            }

            if (a.isSelf != b.isSelf)
            {
                return a.isSelf;
            }

            return a.characterName < b.characterName;
        }
    );

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

    static bool showSquadDebugColumns = false;

    ImGui::Checkbox(
        "Show squad debug columns",
        &showSquadDebugColumns
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

        const int squadColumnCount =
            showSquadDebugColumns
            ? 7
            : 4;

        if (ImGui::BeginTable(
            "##FoodReminderArcPlayers",
            squadColumnCount,
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

            if (showSquadDebugColumns)
            {
                ImGui::TableSetupColumn(
                    "Account"
                );

                ImGui::TableSetupColumn(
                    "Agent ID"
                );

                ImGui::TableSetupColumn(
                    "Self"
                );
            }

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

                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();

                            ImGui::TextUnformatted(
                                "Unmapped Food effect"
                            );

                            ImGui::Separator();

                            ImGui::Text(
                                "Effect ID: %u",
                                player.foodSkillID
                            );

                            ImGui::TextUnformatted(
                                "Type: Food"
                            );

                            ImGui::TextWrapped(
                                "Click this Food entry to copy the Effect ID."
                            );

                            ImGui::TextWrapped(
                                "This effect is being tracked correctly, "
                                "but it has not been added to the consumable database yet."
                            );

                            ImGui::EndTooltip();
                        }

                        if (ImGui::IsItemClicked())
                        {
                            ImGui::SetClipboardText(
                                std::to_string(
                                    player.foodSkillID
                                ).c_str()
                            );
                        }
                    }
                    else
                    {
                        ImGui::Text(
                            "%s %02lld:%02lld:%02lld",
                            GetSquadConsumableLabel(
                                foodInfo.label
                            ),
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

                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();

                            ImGui::TextUnformatted(
                                "Unmapped Utility effect"
                            );

                            ImGui::Separator();

                            ImGui::Text(
                                "Effect ID: %u",
                                player.utilitySkillID
                            );

                            ImGui::TextUnformatted(
                                "Type: Utility"
                            );

                            ImGui::TextWrapped(
                                "Click this Utility entry to copy the Effect ID."
                            );

                            ImGui::TextWrapped(
                                "This effect is being tracked correctly, "
                                "but it has not been added to the consumable database yet."
                            );

                            ImGui::EndTooltip();
                        }

                        if (ImGui::IsItemClicked())
                        {
                            ImGui::SetClipboardText(
                                std::to_string(
                                    player.utilitySkillID
                                ).c_str()
                            );
                        }
                    }
                    else
                    {
                        ImGui::Text(
                            "%s %02lld:%02lld:%02lld",
                            GetSquadConsumableLabel(
                                utilityInfo.label
                            ),
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

                if (showSquadDebugColumns)
                {
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
            }

            ImGui::EndTable();
        }
    }
    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::CollapsingHeader(
        "Unknown Consumables",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        const std::vector<UnknownConsumable>
            unknownConsumables =
            SquadTracker::GetUnknownConsumables();

        ImGui::Text(
            "Unique unknown effects captured: %llu",
            static_cast<unsigned long long>(
                unknownConsumables.size()
                )
        );

        ImGui::TextWrapped(
            "Unknown Food and Utility effects are captured automatically "
            "from ArcDPS squad events even when this tab is closed."
        );

        ImGui::Spacing();

        if (unknownConsumables.empty())
        {
            ImGui::TextDisabled(
                "No unknown consumables captured yet."
            );
        }
        else
        {
            if (ImGui::BeginTable(
                "##UnknownConsumablesTable",
                3,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_SizingStretchProp
            ))
            {
                ImGui::TableSetupColumn(
                    "Type"
                );

                ImGui::TableSetupColumn(
                    "Effect ID"
                );

                ImGui::TableSetupColumn(
                    "Seen"
                );

                ImGui::TableHeadersRow();

                for (
                    const UnknownConsumable& unknown :
                    unknownConsumables
                    )
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);

                    ImGui::TextUnformatted(
                        unknown.isFood
                        ? "Food"
                        : "Utility"
                    );

                    ImGui::TableSetColumnIndex(1);

                    ImGui::Text(
                        "%u",
                        unknown.skillID
                    );

                    ImGui::TableSetColumnIndex(2);

                    ImGui::Text(
                        "%llu",
                        static_cast<unsigned long long>(
                            unknown.seenCount
                            )
                    );
                }

                ImGui::EndTable();
            }

            ImGui::Spacing();

            if (ImGui::Button(
                "Copy All Unknown IDs"
            ))
            {
                std::string clipboardText;

                for (
                    const UnknownConsumable& unknown :
                    unknownConsumables
                    )
                {
                    clipboardText +=
                        unknown.isFood
                        ? "Food,"
                        : "Utility,";

                    clipboardText +=
                        std::to_string(
                            unknown.skillID
                        );

                    clipboardText +=
                        ",Seen:";

                    clipboardText +=
                        std::to_string(
                            unknown.seenCount
                        );

                    clipboardText +=
                        "\n";
                }

                ImGui::SetClipboardText(
                    clipboardText.c_str()
                );
            }

            ImGui::SameLine();

            if (ImGui::Button(
                "Clear Unknown List"
            ))
            {
                SquadTracker::
                    ClearUnknownConsumables();
            }
        }
    }
    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::CollapsingHeader(
        "Developer / Unofficial Extras"
    ))
    {
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
}
void RenderSessionTab()
{
    const SessionStats stats =
        SessionTracker::GetStats();

    const auto FormatDuration =
        [](int64_t milliseconds)
        {
            const int64_t totalSeconds =
                milliseconds / 1000;

            const int64_t hours =
                totalSeconds / 3600;

            const int64_t minutes =
                (totalSeconds % 3600) / 60;

            const int64_t seconds =
                totalSeconds % 60;

            char buffer[32] = {};

            snprintf(
                buffer,
                sizeof(buffer),
                "%02lld:%02lld:%02lld",
                hours,
                minutes,
                seconds
            );

            return std::string(buffer);
        };

    const auto CalculateCoverage =
        [](int64_t activeMilliseconds,
            int64_t totalMilliseconds)
        {
            if (totalMilliseconds <= 0)
            {
                return 0.0;
            }

            return
                (static_cast<double>(
                    activeMilliseconds
                    ) /
                    static_cast<double>(
                        totalMilliseconds
                        )) *
                100.0;
        };

    const auto HelpMarker =
        [](const char* explanation)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");

            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(
                    ImGui::GetFontSize() * 28.0f
                );
                ImGui::TextUnformatted(
                    explanation
                );
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        };

    const ImVec4 goodColor(
        0.35f,
        0.90f,
        0.45f,
        1.00f
    );

    const ImVec4 attentionColor(
        1.00f,
        0.78f,
        0.25f,
        1.00f
    );

    const ImVec4 badColor(
        1.00f,
        0.35f,
        0.35f,
        1.00f
    );

    const double foodSessionCoverage =
        CalculateCoverage(
            stats.foodActiveMilliseconds,
            stats.sessionMilliseconds
        );

    const double utilitySessionCoverage =
        CalculateCoverage(
            stats.utilityActiveMilliseconds,
            stats.sessionMilliseconds
        );

    const double foodCombatCoverage =
        CalculateCoverage(
            stats.foodCombatMilliseconds,
            stats.combatMilliseconds
        );

    const double utilityCombatCoverage =
        CalculateCoverage(
            stats.utilityCombatMilliseconds,
            stats.combatMilliseconds
        );

    double metabolicPrimerUsesSaved = 0.0;
    double metabolicPrimerCopperSaved = 0.0;

    for (
        const SessionPrimerProtectedUsage& usage :
        stats.foodPrimerProtection
        )
    {
        const ConsumableInfo& info =
            ConsumableData::GetFoodInfo(
                usage.skillID
            );

        if (info.itemID == 0)
        {
            continue;
        }

        ConsumableMetadataManager::RequestMetadata(
            info.itemID
        );

        ConsumableMetadata metadata;

        if (
            !ConsumableMetadataManager::TryGetMetadata(
                info.itemID,
                metadata
            ) ||
            metadata.durationMilliseconds == 0
            )
        {
            continue;
        }

        const double usesSaved =
            static_cast<double>(
                usage.protectedMilliseconds
                ) /
            static_cast<double>(
                metadata.durationMilliseconds
                );

        metabolicPrimerUsesSaved +=
            usesSaved;

        TradingPostPriceManager::RequestPrice(
            info.itemID
        );

        TradingPostPrice price;

        if (
            TradingPostPriceManager::TryGetPrice(
                info.itemID,
                price
            )
            )
        {
            metabolicPrimerCopperSaved +=
                usesSaved *
                static_cast<double>(
                    price.sellUnitPrice
                    );
        }
    }

    double utilityPrimerUsesSaved = 0.0;
    double utilityPrimerCopperSaved = 0.0;

    for (
        const SessionPrimerProtectedUsage& usage :
        stats.utilityPrimerProtection
        )
    {
        const ConsumableInfo& info =
            ConsumableData::GetUtilityInfo(
                usage.skillID
            );

        if (info.itemID == 0)
        {
            continue;
        }

        ConsumableMetadataManager::RequestMetadata(
            info.itemID
        );

        ConsumableMetadata metadata;

        if (
            !ConsumableMetadataManager::TryGetMetadata(
                info.itemID,
                metadata
            ) ||
            metadata.durationMilliseconds == 0
            )
        {
            continue;
        }

        const double usesSaved =
            static_cast<double>(
                usage.protectedMilliseconds
                ) /
            static_cast<double>(
                metadata.durationMilliseconds
                );

        utilityPrimerUsesSaved +=
            usesSaved;

        TradingPostPriceManager::RequestPrice(
            info.itemID
        );

        TradingPostPrice price;

        if (
            TradingPostPriceManager::TryGetPrice(
                info.itemID,
                price
            )
            )
        {
            utilityPrimerCopperSaved +=
                usesSaved *
                static_cast<double>(
                    price.sellUnitPrice
                    );
        }
    }

    const uint64_t metabolicPrimerCopperSavedRounded =
        static_cast<uint64_t>(
            metabolicPrimerCopperSaved + 0.5
            );

    const uint64_t utilityPrimerCopperSavedRounded =
        static_cast<uint64_t>(
            utilityPrimerCopperSaved + 0.5
            );

    const uint64_t totalPrimerCopperSaved =
        metabolicPrimerCopperSavedRounded +
        utilityPrimerCopperSavedRounded;

    ImGui::TextUnformatted(
        "Session Summary"
    );

    ImGui::TextDisabled(
        "Current-session food, utility, primer, cost, and efficiency tracking."
    );

    ImGui::Separator();

    ImGui::Text(
        "Session Time: %s",
        FormatDuration(
            stats.sessionMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Total time tracked since this Session report was started or reset."
    );

    ImGui::Text(
        "Combat Time:  %s",
        FormatDuration(
            stats.combatMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Total time ArcDPS reported this character as being in combat during the current session."
    );

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextUnformatted(
        "Food"
    );

    ImGui::TextDisabled(
        "Coverage, primer savings, and food usage."
    );

    if (foodSessionCoverage >= 99.95)
    {
        ImGui::TextColored(
            goodColor,
            "Session: %.1f%%",
            foodSessionCoverage
        );
    }
    else
    {
        ImGui::Text(
            "Session: %.1f%%",
            foodSessionCoverage
        );
    }

    HelpMarker(
        "Percent of the full session during which Food was active."
    );

    if (foodCombatCoverage >= 99.95)
    {
        ImGui::TextColored(
            goodColor,
            "In Combat: %.1f%%",
            foodCombatCoverage
        );
    }
    else
    {
        ImGui::Text(
            "In Combat: %.1f%%",
            foodCombatCoverage
        );
    }

    HelpMarker(
        "Percent of combat time during which Food was active."
    );

    const int64_t foodUnbuffedCombatMilliseconds =
        stats.combatMilliseconds >
        stats.foodCombatMilliseconds
        ? stats.combatMilliseconds -
        stats.foodCombatMilliseconds
        : 0;

    if (foodUnbuffedCombatMilliseconds > 0)
    {
        ImGui::TextColored(
            badColor,
            "Unbuffed in Combat: %s",
            FormatDuration(
                foodUnbuffedCombatMilliseconds
            ).c_str()
        );
    }
    else
    {
        ImGui::Text(
            "Unbuffed in Combat: %s",
            FormatDuration(
                foodUnbuffedCombatMilliseconds
            ).c_str()
        );
    }

    HelpMarker(
        "Total combat time spent without Food active."
    );

    ImGui::Text(
        "Active Time: %s",
        FormatDuration(
            stats.foodActiveMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Total session time Food has been active."
    );

    ImGui::Text(
        "Metabolic Primer Active: %s",
        FormatDuration(
            stats.metabolicPrimerActiveMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Session time during which active Food was protected by a Metabolic Primer."
    );

    ImGui::TextColored(
        goodColor,
        "Primer Uses Saved: %.2f",
        metabolicPrimerUsesSaved
    );

    HelpMarker(
        "Estimated Food applications avoided because of primer protection. Uses each Food item's normal duration from the Guild Wars 2 item API."
    );

    ImGui::TextColored(
        goodColor,
        "Primer Gold Saved: %llug %llus %lluc",
        metabolicPrimerCopperSavedRounded / 10000,
        (metabolicPrimerCopperSavedRounded % 10000) / 100,
        metabolicPrimerCopperSavedRounded % 100
    );

    HelpMarker(
        "Estimated Trading Post value of the Food applications the primer has saved, using the current lowest sell listing."
    );

    ImGui::Text(
        "Applications: %u",
        stats.foodApplications
    );

    HelpMarker(
        "Number of Food applications detected during this session."
    );

    ImGui::Text(
        "Refreshes: %u",
        stats.foodRefreshes
    );

    HelpMarker(
        "Times the same Food was reapplied while it was already active."
    );

    if (stats.foodReplacements > 0)
    {
        ImGui::TextColored(
            attentionColor,
            "Replacements: %u",
            stats.foodReplacements
        );
    }
    else
    {
        ImGui::Text(
            "Replacements: %u",
            stats.foodReplacements
        );
    }

    HelpMarker(
        "Times a different Food replaced an already-active Food."
    );

    if (stats.foodExpiredInCombat > 0)
    {
        ImGui::TextColored(
            badColor,
            "Expired In Combat: %u",
            stats.foodExpiredInCombat
        );
    }
    else
    {
        ImGui::Text(
            "Expired In Combat: %u",
            stats.foodExpiredInCombat
        );
    }

    HelpMarker(
        "Number of times Food expired while ArcDPS reported you were in combat."
    );
    if (stats.foodWastedMilliseconds > 0)
    {
        ImGui::TextColored(
            attentionColor,
            "Wasted Duration: %s",
            FormatDuration(
                stats.foodWastedMilliseconds
            ).c_str()
        );
    }
    else
    {
        ImGui::Text(
            "Wasted Duration: %s",
            FormatDuration(
                stats.foodWastedMilliseconds
            ).c_str()
        );
    }

    HelpMarker(
        "Remaining Food duration lost when an active Food was replaced."
    );
    ImGui::Spacing();

    ImGui::TextUnformatted(
        "Food Used / Cost"
    );
    uint64_t foodCostCopper = 0;
    if (stats.foodUsage.empty())
    {
        ImGui::TextDisabled(
            "No Food applications recorded."
        );
    }
    else
    {
        std::vector<SessionConsumableUsage>
            sortedFoodUsage =
            stats.foodUsage;

        std::stable_sort(
            sortedFoodUsage.begin(),
            sortedFoodUsage.end(),
            [](
                const SessionConsumableUsage& a,
                const SessionConsumableUsage& b
                )
            {
                return a.uses > b.uses;
            }
        );

        for (
            const SessionConsumableUsage& usage :
            sortedFoodUsage
            )
        {
            const ConsumableInfo& info =
                ConsumableData::GetFoodInfo(
                    usage.skillID
                );

            if (std::string(info.label) ==
                "Unknown")
            {
                ImGui::Text(
                    "Unknown (%u): %u use%s",
                    usage.skillID,
                    usage.uses,
                    usage.uses == 1
                    ? ""
                    : "s"
                );
            }
            else
            {
                TradingPostPrice price;

                if (info.itemID != 0)
                {
                    TradingPostPriceManager::RequestPrice(
                        info.itemID
                    );
                }

                const bool hasPrice =
                    info.itemID != 0 &&
                    TradingPostPriceManager::TryGetPrice(
                        info.itemID,
                        price
                    );

                if (hasPrice)
                {
                    const uint64_t totalCost =
                        static_cast<uint64_t>(
                            price.sellUnitPrice
                            ) *
                        static_cast<uint64_t>(
                            usage.uses
                            );

                    foodCostCopper +=
                        totalCost;

                    const uint32_t unitGold =
                        price.sellUnitPrice / 10000;

                    const uint32_t unitSilver =
                        (
                            price.sellUnitPrice %
                            10000
                            ) / 100;

                    const uint32_t unitCopper =
                        price.sellUnitPrice % 100;

                    const uint64_t totalGold =
                        totalCost / 10000;

                    const uint64_t totalSilver =
                        (
                            totalCost %
                            10000
                            ) / 100;

                    const uint64_t totalCopper =
                        totalCost % 100;

                    ImGui::Text(
                        "%s: %u use%s",
                        info.name,
                        usage.uses,
                        usage.uses == 1
                        ? ""
                        : "s"
                    );

                    ImGui::TextDisabled(
                        "  %ug %us %uc each | %llug %llus %lluc total",
                        unitGold,
                        unitSilver,
                        unitCopper,
                        totalGold,
                        totalSilver,
                        totalCopper
                    );
                }
                else
                {
                    ImGui::Text(
                        "%s: %u use%s",
                        info.name,
                        usage.uses,
                        usage.uses == 1
                        ? ""
                        : "s"
                    );

                    ImGui::TextDisabled(
                        "  Price not loaded"
                    );
                }
            }
        }
        if (foodCostCopper > 0)
        {
            const uint64_t foodGold =
                foodCostCopper / 10000;

            const uint64_t foodSilver =
                (
                    foodCostCopper %
                    10000
                    ) / 100;

            const uint64_t foodCopper =
                foodCostCopper % 100;

            ImGui::Text(
                "Food Used Cost: %llug %llus %lluc",
                foodGold,
                foodSilver,
                foodCopper
            );
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextUnformatted(
        "Utility"
    );

    ImGui::TextDisabled(
        "Coverage, primer savings, and utility usage."
    );

    if (utilitySessionCoverage >= 99.95)
    {
        ImGui::TextColored(
            goodColor,
            "Session: %.1f%%",
            utilitySessionCoverage
        );
    }
    else
    {
        ImGui::Text(
            "Session: %.1f%%",
            utilitySessionCoverage
        );
    }

    HelpMarker(
        "Percent of the full session during which Utility was active."
    );

    if (utilityCombatCoverage >= 99.95)
    {
        ImGui::TextColored(
            goodColor,
            "In Combat: %.1f%%",
            utilityCombatCoverage
        );
    }
    else
    {
        ImGui::Text(
            "In Combat: %.1f%%",
            utilityCombatCoverage
        );
    }

    HelpMarker(
        "Percent of combat time during which Utility was active."
    );

    const int64_t utilityUnbuffedCombatMilliseconds =
        stats.combatMilliseconds >
        stats.utilityCombatMilliseconds
        ? stats.combatMilliseconds -
        stats.utilityCombatMilliseconds
        : 0;

    if (utilityUnbuffedCombatMilliseconds > 0)
    {
        ImGui::TextColored(
            badColor,
            "Unbuffed in Combat: %s",
            FormatDuration(
                utilityUnbuffedCombatMilliseconds
            ).c_str()
        );
    }
    else
    {
        ImGui::Text(
            "Unbuffed in Combat: %s",
            FormatDuration(
                utilityUnbuffedCombatMilliseconds
            ).c_str()
        );
    }

    HelpMarker(
        "Total combat time spent without Utility active."
    );


    ImGui::Text(
        "Active Time: %s",
        FormatDuration(
            stats.utilityActiveMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Total session time Utility has been active."
    );


    ImGui::Text(
        "Utility Primer Active: %s",
        FormatDuration(
            stats.utilityPrimerActiveMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Session time during which active Utility was protected by a Utility Primer."
    );


    ImGui::TextColored(
        goodColor,
        "Primer Uses Saved: %.2f",
        utilityPrimerUsesSaved
    );

    HelpMarker(
        "Estimated Utility applications avoided because of primer protection. Uses each Utility item's normal duration from the Guild Wars 2 item API."
    );

    ImGui::TextColored(
        goodColor,
        "Primer Gold Saved: %llug %llus %lluc",
        utilityPrimerCopperSavedRounded / 10000,
        (utilityPrimerCopperSavedRounded % 10000) / 100,
        utilityPrimerCopperSavedRounded % 100
    );

    HelpMarker(
        "Estimated Trading Post value of the Utility applications the primer has saved, using the current lowest sell listing."
    );
    ImGui::Text(
        "Applications: %u",
        stats.utilityApplications
    );

    HelpMarker(
        "Number of Utility applications detected during this session."
    );

    ImGui::Text(
        "Refreshes: %u",
        stats.utilityRefreshes
    );

    HelpMarker(
        "Times the same Utility was reapplied while it was already active."
    );

    if (stats.utilityReplacements > 0)
    {
        ImGui::TextColored(
            attentionColor,
            "Replacements: %u",
            stats.utilityReplacements
        );
    }
    else
    {
        ImGui::Text(
            "Replacements: %u",
            stats.utilityReplacements
        );
    }

    HelpMarker(
        "Times a different Utility replaced an already-active Utility."
    );

    if (stats.utilityExpiredInCombat > 0)
    {
        ImGui::TextColored(
            badColor,
            "Expired In Combat: %u",
            stats.utilityExpiredInCombat
        );
    }
    else
    {
        ImGui::Text(
            "Expired In Combat: %u",
            stats.utilityExpiredInCombat
        );
    }

    HelpMarker(
        "Number of times Utility expired while ArcDPS reported you were in combat."
    );
    if (stats.utilityWastedMilliseconds > 0)
    {
        ImGui::TextColored(
            attentionColor,
            "Wasted Duration: %s",
            FormatDuration(
                stats.utilityWastedMilliseconds
            ).c_str()
        );
    }
    else
    {
        ImGui::Text(
            "Wasted Duration: %s",
            FormatDuration(
                stats.utilityWastedMilliseconds
            ).c_str()
        );
    }

    HelpMarker(
        "Remaining Utility duration lost when an active Utility was replaced."
    );
    ImGui::Spacing();

    ImGui::TextUnformatted(
        "Utility Used / Cost"
    );
    uint64_t utilityCostCopper = 0;

    if (stats.utilityUsage.empty())
    {
        ImGui::TextDisabled(
            "No Utility applications recorded."
        );
    }
    else
    {
        std::vector<SessionConsumableUsage>
            sortedUtilityUsage =
            stats.utilityUsage;

        std::stable_sort(
            sortedUtilityUsage.begin(),
            sortedUtilityUsage.end(),
            [](
                const SessionConsumableUsage& a,
                const SessionConsumableUsage& b
                )
            {
                return a.uses > b.uses;
            }
        );

        for (
            const SessionConsumableUsage& usage :
            sortedUtilityUsage
            )
        {
            const ConsumableInfo& info =
                ConsumableData::GetUtilityInfo(
                    usage.skillID
                );

            if (std::string(info.label) ==
                "Unknown")
            {
                ImGui::Text(
                    "Unknown (%u): %u use%s",
                    usage.skillID,
                    usage.uses,
                    usage.uses == 1
                    ? ""
                    : "s"
                );
            }
            else
            {
                TradingPostPrice price;

                if (info.itemID != 0)
                {
                    TradingPostPriceManager::RequestPrice(
                        info.itemID
                    );
                }

                const bool hasPrice =
                    info.itemID != 0 &&
                    TradingPostPriceManager::TryGetPrice(
                        info.itemID,
                        price
                    );

                if (hasPrice)
                {
                    const uint64_t totalCost =
                        static_cast<uint64_t>(
                            price.sellUnitPrice
                            ) *
                        static_cast<uint64_t>(
                            usage.uses
                            );

                    utilityCostCopper += totalCost;

                    ImGui::Text(
                        "%s: %u use%s",
                        info.name,
                        usage.uses,
                        usage.uses == 1
                        ? ""
                        : "s"
                    );

                    ImGui::TextDisabled(
                        "  %ug %us %uc each | %llug %llus %lluc total",
                        price.sellUnitPrice / 10000,
                        (price.sellUnitPrice % 10000) / 100,
                        price.sellUnitPrice % 100,
                        totalCost / 10000,
                        (totalCost % 10000) / 100,
                        totalCost % 100
                    );
                }
                else
                {
                    ImGui::Text(
                        "%s: %u use%s",
                        info.name,
                        usage.uses,
                        usage.uses == 1
                        ? ""
                        : "s"
                    );

                    ImGui::TextDisabled(
                        "  Price not loaded"
                    );
                }
                if (utilityCostCopper > 0)
                {
                    ImGui::Text(
                        "Utility Used Cost: %llug %llus %lluc",
                        utilityCostCopper / 10000,
                        (utilityCostCopper % 10000) / 100,
                        utilityCostCopper % 100
                    );
                }
                const uint64_t totalConsumableCostCopper =
                    foodCostCopper +
                    utilityCostCopper;

                if (totalConsumableCostCopper > 0)
                {
                    ImGui::Spacing();

                    ImGui::Text(
                        "Total Used Cost: %llug %llus %lluc",
                        totalConsumableCostCopper / 10000,
                        (totalConsumableCostCopper % 10000) / 100,
                        totalConsumableCostCopper % 100
                    );
                }
            }
        }
    }
    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextUnformatted(
        "Session Totals & Efficiency"
    );

    ImGui::TextDisabled(
        "Quick read on coverage, spending, primer savings, and avoidable waste."
    );

    const uint64_t totalUsedCostCopper =
        foodCostCopper +
        utilityCostCopper;

    const uint32_t totalReplacements =
        stats.foodReplacements +
        stats.utilityReplacements;

    const uint32_t totalExpiredInCombat =
        stats.foodExpiredInCombat +
        stats.utilityExpiredInCombat;

    const int64_t totalWastedMilliseconds =
        stats.foodWastedMilliseconds +
        stats.utilityWastedMilliseconds;

    //
    // Coverage summary.
    //
    if (stats.combatMilliseconds > 0)
    {
        const double lowestCombatCoverage =
            foodCombatCoverage <
            utilityCombatCoverage
            ? foodCombatCoverage
            : utilityCombatCoverage;

        if (lowestCombatCoverage >= 99.95)
        {
            ImGui::TextColored(
                goodColor,
                "Combat Coverage: Excellent - both buffs stayed active."
            );
        }
        else if (lowestCombatCoverage >= 95.0)
        {
            ImGui::TextColored(
                attentionColor,
                "Combat Coverage: Strong - %.1f%% minimum coverage.",
                lowestCombatCoverage
            );
        }
        else
        {
            ImGui::TextColored(
                badColor,
                "Combat Coverage: Needs attention - %.1f%% minimum coverage.",
                lowestCombatCoverage
            );
        }

        HelpMarker(
            "Uses the lower of Food and Utility combat coverage so the summary reflects whichever buff had more downtime."
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Combat Coverage: No combat recorded yet."
        );
    }

    //
    // Consumable spend.
    //
    if (totalUsedCostCopper > 0)
    {
        ImGui::Text(
            "Total Used Cost: %llug %llus %lluc",
            totalUsedCostCopper / 10000,
            (totalUsedCostCopper % 10000) / 100,
            totalUsedCostCopper % 100
        );

        HelpMarker(
            "Current Trading Post sell-price value of Food and Utility applications recorded during this session."
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Total Used Cost: No priced applications yet."
        );
    }

    //
    // Primer value.
    //
    ImGui::TextColored(
        goodColor,
        "Total Primer Gold Saved: %llug %llus %lluc",
        totalPrimerCopperSaved / 10000,
        (totalPrimerCopperSaved % 10000) / 100,
        totalPrimerCopperSaved % 100
    );

    HelpMarker(
        "Estimated Trading Post value of consumable applications avoided because your primers extended active Food and Utility."
    );

    const double totalPrimerUsesSaved =
        metabolicPrimerUsesSaved +
        utilityPrimerUsesSaved;

    ImGui::TextColored(
        goodColor,
        "Total Primer Uses Saved: %.2f",
        totalPrimerUsesSaved
    );

    HelpMarker(
        "Combined fractional Food and Utility applications avoided through primer protection."
    );

    //
    // Waste summary.
    //
    if (
        totalReplacements == 0 &&
        totalWastedMilliseconds == 0
        )
    {
        ImGui::TextColored(
            goodColor,
            "Replacement Waste: None."
        );
    }
    else
    {
        ImGui::TextColored(
            attentionColor,
            "Replacement Waste: %u replacement%s | %s lost",
            totalReplacements,
            totalReplacements == 1
            ? ""
            : "s",
            FormatDuration(
                totalWastedMilliseconds
            ).c_str()
        );

        HelpMarker(
            "Time remaining on an active consumable when it was replaced by a different one."
        );
    }

    //
    // Expiration summary.
    //
    if (totalExpiredInCombat == 0)
    {
        ImGui::TextColored(
            goodColor,
            "In-Combat Expirations: None."
        );
    }
    else
    {
        ImGui::TextColored(
            badColor,
            "In-Combat Expirations: %u",
            totalExpiredInCombat
        );

        HelpMarker(
            "Food or Utility expirations detected while ArcDPS reported you were still in combat."
        );
    }

    //
    // Detailed waste information appears only when something
    // actually happened, keeping healthy sessions compact.
    //
    const bool hasFoodWaste =
        stats.foodReplacements > 0 ||
        stats.foodWastedMilliseconds > 0;

    const bool hasUtilityWaste =
        stats.utilityReplacements > 0 ||
        stats.utilityWastedMilliseconds > 0;

    if (hasFoodWaste ||
        hasUtilityWaste)
    {
        ImGui::Spacing();
        ImGui::TextDisabled(
            "Waste Details"
        );

        if (hasFoodWaste)
        {
            ImGui::TextColored(
                attentionColor,
                "Food: %u replacement%s | %s wasted",
                stats.foodReplacements,
                stats.foodReplacements == 1
                ? ""
                : "s",
                FormatDuration(
                    stats.foodWastedMilliseconds
                ).c_str()
            );

            if (
                stats.worstFoodWasteMilliseconds > 0 &&
                stats.worstFoodWasteSkillID != 0
                )
            {
                const ConsumableInfo& worstFoodInfo =
                    ConsumableData::GetFoodInfo(
                        stats.worstFoodWasteSkillID
                    );

                if (
                    std::string(
                        worstFoodInfo.label
                    ) == "Unknown"
                    )
                {
                    ImGui::TextDisabled(
                        "Worst Food Waste: %s - Unknown (%u)",
                        FormatDuration(
                            stats.worstFoodWasteMilliseconds
                        ).c_str(),
                        stats.worstFoodWasteSkillID
                    );
                }
                else
                {
                    ImGui::TextDisabled(
                        "Worst Food Waste: %s - %s",
                        FormatDuration(
                            stats.worstFoodWasteMilliseconds
                        ).c_str(),
                        worstFoodInfo.name
                    );
                }
            }
        }

        if (hasUtilityWaste)
        {
            ImGui::TextColored(
                attentionColor,
                "Utility: %u replacement%s | %s wasted",
                stats.utilityReplacements,
                stats.utilityReplacements == 1
                ? ""
                : "s",
                FormatDuration(
                    stats.utilityWastedMilliseconds
                ).c_str()
            );

            if (
                stats.worstUtilityWasteMilliseconds > 0 &&
                stats.worstUtilityWasteSkillID != 0
                )
            {
                const ConsumableInfo& worstUtilityInfo =
                    ConsumableData::GetUtilityInfo(
                        stats.worstUtilityWasteSkillID
                    );

                if (
                    std::string(
                        worstUtilityInfo.label
                    ) == "Unknown"
                    )
                {
                    ImGui::TextDisabled(
                        "Worst Utility Waste: %s - Unknown (%u)",
                        FormatDuration(
                            stats.worstUtilityWasteMilliseconds
                        ).c_str(),
                        stats.worstUtilityWasteSkillID
                    );
                }
                else
                {
                    ImGui::TextDisabled(
                        "Worst Utility Waste: %s - %s",
                        FormatDuration(
                            stats.worstUtilityWasteMilliseconds
                        ).c_str(),
                        worstUtilityInfo.name
                    );
                }
            }
        }
    }
    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextUnformatted(
        "Consumable History"
    );

    ImGui::TextDisabled(
        "Timeline of food and utility changes during this session."
    );

    if (stats.consumableHistory.empty())
    {
        ImGui::TextDisabled(
            "No consumable events recorded."
        );
    }
    else
    {
        for (
            const SessionConsumableEvent& event :
            stats.consumableHistory
            )
        {
            const int64_t totalSeconds =
                event.sessionMilliseconds / 1000;

            const int64_t hours =
                totalSeconds / 3600;

            const int64_t minutes =
                (totalSeconds % 3600) / 60;

            const int64_t seconds =
                totalSeconds % 60;

            const ConsumableInfo& info =
                event.isFood
                ? ConsumableData::GetFoodInfo(
                    event.skillID
                )
                : ConsumableData::GetUtilityInfo(
                    event.skillID
                );

            const char* actionText =
                event.type ==
                SessionConsumableEventType::Applied
                ? "Applied"
                : event.type ==
                SessionConsumableEventType::Refreshed
                ? "Refreshed"
                : event.type ==
                SessionConsumableEventType::Replaced
                ? "Replaced"
                : "Expired";
            const ConsumableInfo* previousInfo = nullptr;

            if (
                event.type ==
                SessionConsumableEventType::Replaced &&
                event.previousSkillID != 0
                )
            {
                previousInfo =
                    event.isFood
                    ? &ConsumableData::GetFoodInfo(
                        event.previousSkillID
                    )
                    : &ConsumableData::GetUtilityInfo(
                        event.previousSkillID
                    );
            }
            if (
                event.type ==
                SessionConsumableEventType::Replaced &&
                previousInfo != nullptr
                )
            {
                const bool previousUnknown =
                    std::string(
                        previousInfo->label
                    ) == "Unknown";

                const bool currentUnknown =
                    std::string(
                        info.label
                    ) == "Unknown";

                const int64_t previousTotalSeconds =
                    event.previousRemainingMilliseconds / 1000;

                const int64_t previousMinutes =
                    previousTotalSeconds / 60;

                const int64_t previousSeconds =
                    previousTotalSeconds % 60;

                if (!previousUnknown &&
                    !currentUnknown)
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  %s -> %s  (%lld:%02lld remaining)",
                        hours,
                        minutes,
                        seconds,
                        previousInfo->name,
                        info.name,
                        previousMinutes,
                        previousSeconds
                    );
                }
                else if (previousUnknown &&
                    !currentUnknown)
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  Unknown (%u) -> %s  (%lld:%02lld remaining)",
                        hours,
                        minutes,
                        seconds,
                        event.previousSkillID,
                        info.name,
                        previousMinutes,
                        previousSeconds
                    );
                }
                else if (!previousUnknown &&
                    currentUnknown)
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  %s -> Unknown (%u)  (%lld:%02lld remaining)",
                        hours,
                        minutes,
                        seconds,
                        previousInfo->name,
                        event.skillID,
                        previousMinutes,
                        previousSeconds
                    );
                }
                else
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  Unknown (%u) -> Unknown (%u)  (%lld:%02lld remaining)",
                        hours,
                        minutes,
                        seconds,
                        event.previousSkillID,
                        event.skillID,
                        previousMinutes,
                        previousSeconds
                    );
                }
            }
            else if (std::string(info.label) ==
                "Unknown")
            {
                if (event.type ==
                    SessionConsumableEventType::Expired &&
                    event.inCombat)
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  Expired Unknown (%u) (in combat)",
                        hours,
                        minutes,
                        seconds,
                        event.skillID
                    );
                }
                else
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  %s Unknown (%u)",
                        hours,
                        minutes,
                        seconds,
                        actionText,
                        event.skillID
                    );
                }
            }
            else
            {
                if (event.type ==
                    SessionConsumableEventType::Expired &&
                    event.inCombat)
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  Expired %s (in combat)",
                        hours,
                        minutes,
                        seconds,
                        info.name
                    );
                }
                else
                {
                    ImGui::Text(
                        "%02lld:%02lld:%02lld  %s %s",
                        hours,
                        minutes,
                        seconds,
                        actionText,
                        info.name
                    );
                }
            }
        }
    }
    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button(
        "Reset Session"
    ))
    {
        SessionTracker::Reset();
    }

    ImGui::SameLine();

    ImGui::TextDisabled(
        "Resets session statistics only; active Food, Utility, and Primer timers continue."
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


        if (ImGui::BeginTabItem(
            "Session"))
        {
            RenderSessionTab();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(
            "Trading Post"))
        {
            RenderTradingPostTab();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Food Reminder v0.1.0 - Development Build"
    );
}
