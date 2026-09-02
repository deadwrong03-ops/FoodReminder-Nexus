#include "SquadUI.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "ConsumableData.h"
#include "ExtrasIntegration.h"
#include "RTAPIIntegration.h"
#include "SquadTracker.h"

namespace
{
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

void SquadUI::Render()
{
    ImGui::TextUnformatted(
        "Squad Consumables"
    );

    ImGui::TextDisabled(
        "Food and Utility status reported by ArcDPS."
    );

    RTAPIIntegration::Update();

    if (RTAPIIntegration::HasAuthoritativeRoster())
    {
        ImGui::TextDisabled(
            "Roster: RTAPI | Consumables: ArcDPS"
        );
    }
    else if (RTAPIIntegration::IsAvailable())
    {
        ImGui::TextDisabled(
            "Roster: ArcDPS fallback | RTAPI syncing"
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Roster: ArcDPS fallback | RTAPI unavailable"
        );
    }

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

    static bool showSquadDebugColumns = false;

    // 0 = All players
    // 1 = Missing / Unknown / Unmapped
    // 2 = Unknown / Unmapped only
    static int squadAttentionFilterMode = 0;

    if (ImGui::CollapsingHeader(
        "Display Options"
    ))
    {
        ImGui::Checkbox(
            "Show squad debug columns",
            &showSquadDebugColumns
        );

        const char* attentionFilterLabels[] =
        {
            "All players",
            "Missing / Unknown / Unmapped",
            "Unknown / Unmapped only"
        };

        ImGui::SetNextItemWidth(230.0f);

        ImGui::Combo(
            "Player filter",
            &squadAttentionFilterMode,
            attentionFilterLabels,
            3
        );

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(
                "All players: show everyone.\n"
                "Missing / Unknown / Unmapped: include confirmed missing states.\n"
                "Unknown / Unmapped only: focus on unresolved or not-yet-known data."
            );
        }
    }

    size_t visiblePlayerCount = 0;

    for (
        const SquadTrackedPlayer& player :
        trackedPlayers
        )
    {
        const ConsumableInfo& foodInfo =
            ConsumableData::GetFoodInfo(
                player.foodSkillID
            );

        const ConsumableInfo& utilityInfo =
            ConsumableData::GetUtilityInfo(
                player.utilitySkillID
            );

        const bool foodUnmapped =
            player.hasFood &&
            foodInfo.label != nullptr &&
            std::string(foodInfo.label) == "Unknown";

        const bool utilityUnmapped =
            player.hasUtility &&
            utilityInfo.label != nullptr &&
            std::string(utilityInfo.label) == "Unknown";

        const bool hasUnknownOrUnmapped =
            !player.foodStateKnown ||
            !player.utilityStateKnown ||
            foodUnmapped ||
            utilityUnmapped;

        const bool hasMissing =
            (player.foodStateKnown && !player.hasFood) ||
            (player.utilityStateKnown && !player.hasUtility);

        const bool shouldShow =
            squadAttentionFilterMode == 0 ||
            (
                squadAttentionFilterMode == 1 &&
                (hasUnknownOrUnmapped || hasMissing)
                ) ||
            (
                squadAttentionFilterMode == 2 &&
                hasUnknownOrUnmapped
                );

        if (shouldShow)
        {
            ++visiblePlayerCount;
        }
    }

    ImGui::Spacing();

    ImGui::Spacing();

    ImGui::TextUnformatted(
        "Player Status"
    );

    ImGui::TextDisabled(
        "Current Food and Utility state for tracked players."
    );

    if (squadAttentionFilterMode != 0)
    {
        ImGui::Text(
            "Showing: %llu of %llu tracked players",
            static_cast<
            unsigned long long
            >(
                visiblePlayerCount
                ),
            static_cast<
            unsigned long long
            >(
                trackedPlayers.size()
                )
        );
    }
    else
    {
        ImGui::Text(
            "Tracked players: %llu",
            static_cast<
            unsigned long long
            >(
                trackedPlayers.size()
                )
        );
    }

    ImGui::TextDisabled(
        "? = consumable state not established yet."
    );

    ImGui::Separator();

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
                "Group",
                ImGuiTableColumnFlags_WidthFixed,
                72.0f
            );

            ImGui::TableSetupColumn(
                "Character",
                ImGuiTableColumnFlags_WidthStretch,
                1.25f
            );

            ImGui::TableSetupColumn(
                "Food Status",
                ImGuiTableColumnFlags_WidthStretch,
                1.0f
            );

            ImGui::TableSetupColumn(
                "Utility Status",
                ImGuiTableColumnFlags_WidthStretch,
                1.0f
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
                const ConsumableInfo& filterFoodInfo =
                    ConsumableData::GetFoodInfo(
                        player.foodSkillID
                    );

                const ConsumableInfo& filterUtilityInfo =
                    ConsumableData::GetUtilityInfo(
                        player.utilitySkillID
                    );

                const bool filterFoodUnmapped =
                    player.hasFood &&
                    filterFoodInfo.label != nullptr &&
                    std::string(
                        filterFoodInfo.label
                    ) == "Unknown";

                const bool filterUtilityUnmapped =
                    player.hasUtility &&
                    filterUtilityInfo.label != nullptr &&
                    std::string(
                        filterUtilityInfo.label
                    ) == "Unknown";

                const bool filterHasUnknownOrUnmapped =
                    !player.foodStateKnown ||
                    !player.utilityStateKnown ||
                    filterFoodUnmapped ||
                    filterUtilityUnmapped;

                const bool filterHasMissing =
                    (player.foodStateKnown &&
                        !player.hasFood) ||
                    (player.utilityStateKnown &&
                        !player.hasUtility);

                const bool shouldShowPlayer =
                    squadAttentionFilterMode == 0 ||
                    (
                        squadAttentionFilterMode == 1 &&
                        (
                            filterHasUnknownOrUnmapped ||
                            filterHasMissing
                            )
                        ) ||
                    (
                        squadAttentionFilterMode == 2 &&
                        filterHasUnknownOrUnmapped
                        );

                if (!shouldShowPlayer)
                {
                    continue;
                }

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
                    ImGui::TextDisabled(
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
                        ImGui::TextColored(
                            ImVec4(
                                0.35f,
                                0.90f,
                                0.45f,
                                1.00f
                            ),
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
                    ImGui::TextDisabled(
                        "None"
                    );
                }

                ImGui::TableSetColumnIndex(3);

                if (!player.utilityStateKnown)
                {
                    ImGui::TextDisabled(
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
                        ImGui::TextColored(
                            ImVec4(
                                0.45f,
                                0.75f,
                                1.00f,
                                1.00f
                            ),
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
                    ImGui::TextDisabled(
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
        "Unknown Consumable IDs"
    ))
    {
        std::vector<UnknownConsumable>
            unknownConsumables =
            SquadTracker::GetUnknownConsumables();

        std::sort(
            unknownConsumables.begin(),
            unknownConsumables.end(),
            [](
                const UnknownConsumable& left,
                const UnknownConsumable& right
                )
            {
                if (left.seenCount != right.seenCount)
                {
                    return left.seenCount >
                        right.seenCount;
                }

                if (left.isFood != right.isFood)
                {
                    return left.isFood;
                }

                return left.skillID <
                    right.skillID;
            }
        );

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
        "Developer Tools"
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
