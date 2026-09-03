#include "TrackerUI.h"

#include <array>
#include <cfloat>
#include <string>

#include "imgui/imgui.h"

#include "BuffTracker.h"
#include "ConsumableData.h"
#include "Settings.h"

namespace
{
    // Developer-only tracker color test.
    // 0 = Live
    // 1 = Normal
    // 2 = Warning
    // 3 = Critical
    // 4 = Missing
    int g_ColorTestMode = 0;

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
}

void TrackerUI::Render(
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

    if (g_ColorTestMode == 1)
    {
        displayHasFood = true;
        displayHasUtility = true;

        displayFoodRemaining =
            2LL * 60LL * 60LL * 1000LL;

        displayUtilityRemaining =
            2LL * 60LL * 60LL * 1000LL;
    }
    else if (
        g_ColorTestMode == 2)
    {
        displayHasFood = true;
        displayHasUtility = true;

        displayFoodRemaining =
            5LL * 60LL * 1000LL;

        displayUtilityRemaining =
            5LL * 60LL * 1000LL;
    }
    else if (
        g_ColorTestMode == 3)
    {
        displayHasFood = true;
        displayHasUtility = true;

        displayFoodRemaining =
            30LL * 1000LL;

        displayUtilityRemaining =
            30LL * 1000LL;
    }
    else if (
        g_ColorTestMode == 4)
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

            if (g_ColorTestMode == 2)
            {
                foodColor =
                    &warningColor;
            }
            else if (
                g_ColorTestMode == 3)
            {
                foodColor =
                    &criticalColor;
            }
            else if (
                g_ColorTestMode != 1)
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

            if (g_ColorTestMode == 2)
            {
                utilityColor =
                    &warningColor;
            }
            else if (
                g_ColorTestMode == 3)
            {
                utilityColor =
                    &criticalColor;
            }
            else if (
                g_ColorTestMode != 1)
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

                        ImGui::PushTextWrapPos(
                            ImGui::GetFontSize() * 35.0f
                        );

                        ImGui::TextUnformatted(
                            "The Food duration is clearly Primer-extended, "
                            "so Food Reminder can infer that a Metabolic "
                            "Primer is active. ArcDPS does not provide the "
                            "Primer's exact remaining timer after login or "
                            "character switch, so no countdown is shown."
                        );

                        ImGui::PopTextWrapPos();

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

                        ImGui::PushTextWrapPos(
                            ImGui::GetFontSize() * 35.0f
                        );

                        ImGui::TextUnformatted(
                            "The Utility duration is clearly Primer-extended, "
                            "so Food Reminder can infer that a Utility Primer "
                            "is active. ArcDPS does not provide the Primer's "
                            "exact remaining timer after login or character "
                            "switch, so no countdown is shown."
                        );

                        ImGui::PopTextWrapPos();

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

void TrackerUI::SetColorTestMode(
    int mode
)
{
    g_ColorTestMode = mode;
}

int TrackerUI::GetColorTestMode()
{
    return g_ColorTestMode;
}
