#include <algorithm>
#include <Windows.h>
#include <string>
#include <cfloat>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
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
#include "ConsumableData.h"
#include "ExtrasIntegration.h"
#include "SquadTracker.h"
#include "SessionTracker.h"
#include "RTAPIIntegration.h"
#include "HistoryUI.h"
#include "TradingPostUI.h"
#include "SquadUI.h"

void AddonLoad(AddonAPI_t* aApi);
void AddonUnload();
void AddonRender();
void AddonOptions();
void OnArcDPSCombat(void* eventArgs);

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

void RenderGeneralTab();
void RenderSessionTab();

namespace
{
    std::string FormatCoinValue(
        uint64_t copper
    );

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
        ImVec2(220.0f, 0.0f),
        ImVec2(FLT_MAX, FLT_MAX)
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowBorderSize,
        2.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(
            1.00f,
            0.78f,
            0.20f,
            1.00f
        )
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
                140.0f
            );
            std::string foodDisplayName =
                foodInfo.name != nullptr
                ? foodInfo.name
                : "";

            if (foodDisplayName.length() > 24)
            {
                foodDisplayName =
                    foodDisplayName.substr(0, 21) +
                    "...";
            }

            ImGui::TextUnformatted(
                foodDisplayName.c_str()
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
            if (
                BuffTracker::GetFoodDetectionState() ==
                ConsumableDetectionState::Unknown
                )
            {
                ImGui::TextColored(
                    missingColor,
                    "Food:    Unknown"
                );
            }
            else
            {
                ImGui::TextColored(
                    missingColor,
                    "Food:    Not detected"
                );
            }
        }
        // Candy Cane is a separate nourishment-style buff.
// It does not replace the player's normal Food buff.
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
                140.0f
            );

            std::string utilityDisplayName =
                utilityInfo.name != nullptr
                ? utilityInfo.name
                : "";

            if (utilityDisplayName.length() > 24)
            {
                utilityDisplayName =
                    utilityDisplayName.substr(0, 21) +
                    "...";
            }

            ImGui::TextUnformatted(
                utilityDisplayName.c_str()
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
            if (
                BuffTracker::GetUtilityDetectionState() ==
                ConsumableDetectionState::Unknown
                )
            {
                ImGui::TextColored(
                    missingColor,
                    "Utility: Unknown"
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
            if (
                BuffTracker::GetMetabolicPrimerDetectionState() ==
                ConsumableDetectionState::Unknown
                )
            {
                const bool inferredMetabolicPrimer =
                    BuffTracker::
                    HasInferredMetabolicPrimerPresence();

                if (inferredMetabolicPrimer)
                {
                    ImGui::TextColored(
                        warningColor,
                        "Metabolic: Active*"
                    );

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();

                        ImGui::TextUnformatted(
                            "Primer presence inferred"
                        );

                        ImGui::Separator();

                        ImGui::TextWrapped(
                            "The Food duration is clearly Primer-extended, "
                            "so Food Reminder can infer that a Metabolic "
                            "Primer is active. ArcDPS does not provide the "
                            "Primer's exact remaining timer after login or "
                            "character switch, so no countdown is shown."
                        );

                        ImGui::EndTooltip();
                    }
                }
                else
                {
                    ImGui::TextColored(
                        missingColor,
                        "Metabolic: Unknown"
                    );

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();

                        ImGui::TextUnformatted(
                            "Primer status unavailable"
                        );

                        ImGui::Separator();

                        ImGui::TextWrapped(
                            "ArcDPS does not resend active Primer state "
                            "after login or character switch. "
                            "Food Reminder cannot confirm whether a "
                            "Metabolic Primer is active."
                        );

                        ImGui::EndTooltip();
                    }
                }
            }
            else
            {
                ImGui::TextColored(
                    missingColor,
                    "Metabolic: Not detected"
                );
            }
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
            if (
                BuffTracker::GetUtilityPrimerDetectionState() ==
                ConsumableDetectionState::Unknown
                )
            {
                const bool inferredUtilityPrimer =
                    BuffTracker::
                    HasInferredUtilityPrimerPresence();

                if (inferredUtilityPrimer)
                {
                    ImGui::TextColored(
                        warningColor,
                        "Utility P: Active*"
                    );

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();

                        ImGui::TextUnformatted(
                            "Primer presence inferred"
                        );

                        ImGui::Separator();

                        ImGui::TextWrapped(
                            "The Utility duration is clearly Primer-extended, "
                            "so Food Reminder can infer that a Utility Primer "
                            "is active. ArcDPS does not provide the Primer's "
                            "exact remaining timer after login or character "
                            "switch, so no countdown is shown."
                        );

                        ImGui::EndTooltip();
                    }
                }
                else
                {
                    ImGui::TextColored(
                        missingColor,
                        "Utility P: Unknown"
                    );

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();

                        ImGui::TextUnformatted(
                            "Primer status unavailable"
                        );

                        ImGui::Separator();

                        ImGui::TextWrapped(
                            "ArcDPS does not resend active Primer state "
                            "after login or character switch. "
                            "Food Reminder cannot confirm whether a "
                            "Utility Primer is active."
                        );

                        ImGui::EndTooltip();
                    }
                }
            }
            else
            {
                ImGui::TextColored(
                    missingColor,
                    "Utility P: Not detected"
                );
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
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

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowBorderSize,
        3.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(
            0.95f,
            0.15f,
            0.15f,
            1.00f
        )
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

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
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

        //
        // Conservative session-only savings estimate.
        //
        // We cannot reliably know how much Primer extension had already
        // been consumed before the current session began. Therefore the
        // first normal Food duration is treated as the baseline that the
        // consumable itself would have provided without a Primer.
        //
        // Only Primer-protected time beyond that baseline is credited as
        // estimated avoided reapplications.
        //
        const int64_t savingsEligibleMilliseconds =
            usage.protectedMilliseconds >
            static_cast<int64_t>(
                metadata.durationMilliseconds
                )
            ? usage.protectedMilliseconds -
            static_cast<int64_t>(
                metadata.durationMilliseconds
                )
            : 0;

        const double usesSaved =
            static_cast<double>(
                savingsEligibleMilliseconds
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

        //
        // Conservative session-only savings estimate.
        //
        // As with Food, the first normal Utility duration is treated as
        // baseline coverage. This avoids crediting Primer savings merely
        // because the session began while an already-extended Utility was
        // active.
        //
        const int64_t savingsEligibleMilliseconds =
            usage.protectedMilliseconds >
            static_cast<int64_t>(
                metadata.durationMilliseconds
                )
            ? usage.protectedMilliseconds -
            static_cast<int64_t>(
                metadata.durationMilliseconds
                )
            : 0;

        const double usesSaved =
            static_cast<double>(
                savingsEligibleMilliseconds
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

    ImGui::TextColored(
        goodColor,
        "FOOD"
    );

    ImGui::TextDisabled(
        "Coverage | Primer | Cost"
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

    ImGui::TextColored(
        goodColor,
        "Primer Confirmed: %s",
        FormatDuration(
            stats.metabolicPrimerConfirmedMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Session time during which Food Reminder had direct, trustworthy Metabolic Primer state."
    );

    ImGui::TextColored(
        attentionColor,
        "Primer Inferred*: %s",
        FormatDuration(
            stats.metabolicPrimerInferredMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Primer presence inferred from clearly Primer-extended Food duration. The Primer countdown itself is not inferred."
    );

    ImGui::TextDisabled(
        "Primer Unknown: %s",
        FormatDuration(
            stats.metabolicPrimerUnknownMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Session time where ArcDPS did not provide enough information to confirm whether a Metabolic Primer was active."
    );

    ImGui::TextColored(
        goodColor,
        "Estimated Primer Uses Saved: %.2f",
        metabolicPrimerUsesSaved
    );

    HelpMarker(
        "Conservative session-only estimate. The first normal Food duration is treated as baseline coverage, so savings are credited only after Primer-protected time exceeds that duration. Unknown Primer time is excluded."
    );

    ImGui::TextColored(
        goodColor,
        "Estimated Primer Gold Saved: %llug %llus %lluc",
        metabolicPrimerCopperSavedRounded / 10000,
        (metabolicPrimerCopperSavedRounded % 10000) / 100,
        metabolicPrimerCopperSavedRounded % 100
    );

    HelpMarker(
        "Conservative Trading Post estimate based only on Primer-protected Food time beyond one normal Food duration. Unknown Primer time is excluded."
    );

    if (ImGui::TreeNodeEx(
        "Usage & Waste Details##Food",
        ImGuiTreeNodeFlags_SpanAvailWidth
    ))
    {
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

        ImGui::TreePop();
    }

    ImGui::Spacing();

    uint64_t foodCostCopper = 0;

    if (!stats.foodUsage.empty())
    {
        ImGui::TextColored(
            attentionColor,
            "Food Used / Cost"
        );

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

            if (stats.sessionMilliseconds > 0)
            {
                const uint64_t foodCostPerHourCopper =
                    static_cast<uint64_t>(
                        static_cast<long double>(
                            foodCostCopper
                            ) *
                        3600000.0L /
                        static_cast<long double>(
                            stats.sessionMilliseconds
                            )
                        );

                const std::string foodCostPerHourText =
                    FormatCoinValue(
                        foodCostPerHourCopper
                    );

                ImGui::TextDisabled(
                    "Food Cost / Hour: %s",
                    foodCostPerHourText.c_str()
                );

                HelpMarker(
                    "Projects the current session's recorded Food spending to a one-hour rate. Short sessions can fluctuate heavily."
                );
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextColored(
        ImVec4(
            0.45f,
            0.75f,
            1.00f,
            1.00f
        ),
        "UTILITY"
    );

    ImGui::TextDisabled(
        "Coverage | Primer | Cost"
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


    ImGui::TextColored(
        goodColor,
        "Primer Confirmed: %s",
        FormatDuration(
            stats.utilityPrimerConfirmedMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Session time during which Food Reminder had direct, trustworthy Utility Primer state."
    );

    ImGui::TextColored(
        attentionColor,
        "Primer Inferred*: %s",
        FormatDuration(
            stats.utilityPrimerInferredMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Primer presence inferred from clearly Primer-extended Utility duration. The Primer countdown itself is not inferred."
    );

    ImGui::TextDisabled(
        "Primer Unknown: %s",
        FormatDuration(
            stats.utilityPrimerUnknownMilliseconds
        ).c_str()
    );

    HelpMarker(
        "Session time where ArcDPS did not provide enough information to confirm whether a Utility Primer was active."
    );

    ImGui::TextColored(
        goodColor,
        "Estimated Primer Uses Saved: %.2f",
        utilityPrimerUsesSaved
    );

    HelpMarker(
        "Conservative session-only estimate. The first normal Utility duration is treated as baseline coverage, so savings are credited only after Primer-protected time exceeds that duration. Unknown Primer time is excluded."
    );

    ImGui::TextColored(
        goodColor,
        "Estimated Primer Gold Saved: %llug %llus %lluc",
        utilityPrimerCopperSavedRounded / 10000,
        (utilityPrimerCopperSavedRounded % 10000) / 100,
        utilityPrimerCopperSavedRounded % 100
    );

    HelpMarker(
        "Conservative Trading Post estimate based only on Primer-protected Utility time beyond one normal Utility duration. Unknown Primer time is excluded."
    );
    if (ImGui::TreeNodeEx(
        "Usage & Waste Details##Utility",
        ImGuiTreeNodeFlags_SpanAvailWidth
    ))
    {
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

        ImGui::TreePop();
    }

    ImGui::Spacing();

    uint64_t utilityCostCopper = 0;

    if (!stats.utilityUsage.empty())
    {
        ImGui::TextColored(
            attentionColor,
            "Utility Used / Cost"
        );

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

                    if (stats.sessionMilliseconds > 0)
                    {
                        const uint64_t utilityCostPerHourCopper =
                            static_cast<uint64_t>(
                                static_cast<long double>(
                                    utilityCostCopper
                                    ) *
                                3600000.0L /
                                static_cast<long double>(
                                    stats.sessionMilliseconds
                                    )
                                );

                        const std::string utilityCostPerHourText =
                            FormatCoinValue(
                                utilityCostPerHourCopper
                            );

                        ImGui::TextDisabled(
                            "Utility Cost / Hour: %s",
                            utilityCostPerHourText.c_str()
                        );

                        HelpMarker(
                            "Projects the current session's recorded Utility spending to a one-hour rate. Short sessions can fluctuate heavily."
                        );
                    }
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

    ImGui::TextColored(
        attentionColor,
        "SESSION TOTALS"
    );

    ImGui::TextDisabled(
        "Coverage | Spending | Savings | Waste"
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

        if (stats.sessionMilliseconds > 0)
        {
            const uint64_t sessionCostPerHourCopper =
                static_cast<uint64_t>(
                    static_cast<long double>(
                        totalUsedCostCopper
                        ) *
                    3600000.0L /
                    static_cast<long double>(
                        stats.sessionMilliseconds
                        )
                    );

            const std::string sessionCostPerHourText =
                FormatCoinValue(
                    sessionCostPerHourCopper
                );

            ImGui::TextColored(
                attentionColor,
                "Session Cost Rate: %s / hour",
                sessionCostPerHourText.c_str()
            );

            HelpMarker(
                "Projects all priced Food and Utility spending recorded in this session to a one-hour rate. It becomes more representative as the session gets longer."
            );
        }
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
        "Estimated Total Primer Gold Saved: %llug %llus %lluc",
        totalPrimerCopperSaved / 10000,
        (totalPrimerCopperSaved % 10000) / 100,
        totalPrimerCopperSaved % 100
    );

    HelpMarker(
        "Combined conservative Trading Post estimate. Savings are credited only after each consumable exceeds one normal-duration baseline under confirmed or inferred Primer protection. Unknown Primer time is excluded."
    );

    const double totalPrimerUsesSaved =
        metabolicPrimerUsesSaved +
        utilityPrimerUsesSaved;

    ImGui::TextColored(
        goodColor,
        "Estimated Total Primer Uses Saved: %.2f",
        totalPrimerUsesSaved
    );

    HelpMarker(
        "Combined conservative session-only estimate. Each consumable must exceed one normal-duration baseline before Primer savings are credited. Unknown Primer time is excluded."
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

    if (ImGui::CollapsingHeader(
        "Consumable History"
    ))
    {
        ImGui::TextDisabled(
            "Timeline of Food and Utility changes during this session."
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
            SquadUI::Render();

            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem(
            "Session"))
        {
            RenderSessionTab();

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
