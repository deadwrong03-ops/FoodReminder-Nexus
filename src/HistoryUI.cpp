#include "HistoryUI.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "ConsumableData.h"
#include "SessionTracker.h"
#include "TradingPostPriceManager.h"

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

void HistoryUI::Render(
    const std::string& currentCharacterName
)
{
    const std::vector<SessionHistoryRecord> history =
        SessionTracker::GetHistory();

    ImGui::TextUnformatted(
        "Personal Consumable History"
    );

    ImGui::TextDisabled(
        "Completed FoodReminder sessions saved on this PC."
    );

    ImGui::Spacing();

    static int selectedRange = 1;

    const char* rangeLabels[] =
    {
        "1 Day",
        "7 Days",
        "30 Days",
        "All Time"
    };

    ImGui::SetNextItemWidth(
        150.0f
    );

    ImGui::Combo(
        "Range",
        &selectedRange,
        rangeLabels,
        IM_ARRAYSIZE(
            rangeLabels
        )
    );

    //
    // Character filter.
    // 0 = All Characters
    // 1 = Current Character
    // 2 = Specific saved character
    //
    static int selectedCharacterMode = 1;
    static std::string selectedCharacterName;
    static char characterSearch[96] = {};

    std::vector<std::string> historyCharacterNames;
    bool hasLegacyHistory = false;

    for (
        const SessionHistoryRecord& record :
        history
        )
    {
        if (record.characterName.empty())
        {
            hasLegacyHistory = true;
            continue;
        }

        if (
            std::find(
                historyCharacterNames.begin(),
                historyCharacterNames.end(),
                record.characterName
            ) ==
            historyCharacterNames.end()
            )
        {
            historyCharacterNames.push_back(
                record.characterName
            );
        }
    }

    std::sort(
        historyCharacterNames.begin(),
        historyCharacterNames.end()
    );

    std::string characterPreview =
        "All Characters";

    if (selectedCharacterMode == 1)
    {
        characterPreview =
            currentCharacterName.empty()
            ? "Current Character"
            : "Current Character (" +
            currentCharacterName +
            ")";
    }
    else if (selectedCharacterMode == 2)
    {
        characterPreview =
            selectedCharacterName.empty()
            ? "Legacy / Unknown"
            : selectedCharacterName;
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(
        250.0f
    );

    if (
        ImGui::BeginCombo(
            "Character",
            characterPreview.c_str()
        )
        )
    {
        ImGui::SetNextItemWidth(
            -1.0f
        );

        ImGui::InputTextWithHint(
            "##HistoryCharacterSearch",
            "Search characters...",
            characterSearch,
            IM_ARRAYSIZE(
                characterSearch
            )
        );

        if (
            ImGui::Selectable(
                "All Characters",
                selectedCharacterMode == 0
            )
            )
        {
            selectedCharacterMode = 0;
            selectedCharacterName.clear();
        }

        if (!currentCharacterName.empty())
        {
            const std::string currentLabel =
                "Current Character (" +
                currentCharacterName +
                ")";

            if (
                ImGui::Selectable(
                    currentLabel.c_str(),
                    selectedCharacterMode == 1
                )
                )
            {
                selectedCharacterMode = 1;
                selectedCharacterName.clear();
            }
        }
        else
        {
            ImGui::TextDisabled(
                "Current Character (unavailable)"
            );
        }

        ImGui::Separator();

        std::string searchText =
            characterSearch;

        std::transform(
            searchText.begin(),
            searchText.end(),
            searchText.begin(),
            [](unsigned char value)
            {
                return static_cast<char>(
                    std::tolower(value)
                    );
            }
        );

        const auto MatchesCharacterSearch =
            [&searchText](
                const std::string& name
                )
            {
                if (searchText.empty())
                {
                    return true;
                }

                std::string lowered =
                    name;

                std::transform(
                    lowered.begin(),
                    lowered.end(),
                    lowered.begin(),
                    [](unsigned char value)
                    {
                        return static_cast<char>(
                            std::tolower(value)
                            );
                    }
                );

                return
                    lowered.find(
                        searchText
                    ) !=
                    std::string::npos;
            };

        for (
            const std::string& characterName :
            historyCharacterNames
            )
        {
            if (
                !MatchesCharacterSearch(
                    characterName
                )
                )
            {
                continue;
            }

            const bool selected =
                selectedCharacterMode == 2 &&
                selectedCharacterName ==
                characterName;

            if (
                ImGui::Selectable(
                    characterName.c_str(),
                    selected
                )
                )
            {
                selectedCharacterMode = 2;
                selectedCharacterName =
                    characterName;
            }
        }

        if (
            hasLegacyHistory &&
            MatchesCharacterSearch(
                "Legacy / Unknown"
            )
            )
        {
            if (
                ImGui::Selectable(
                    "Legacy / Unknown",
                    selectedCharacterMode == 2 &&
                    selectedCharacterName.empty()
                )
                )
            {
                selectedCharacterMode = 2;
                selectedCharacterName.clear();
            }

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(
                    "Sessions saved before character tracking was added."
                );
            }
        }

        ImGui::EndCombo();
    }

    const uint64_t nowUnixSeconds =
        static_cast<uint64_t>(
            std::chrono::duration_cast<
            std::chrono::seconds
            >(
                std::chrono::system_clock::
                now().
                time_since_epoch()
            ).count()
            );

    uint64_t rangeSeconds = 0;

    switch (selectedRange)
    {
    case 0:
        rangeSeconds =
            24ULL * 60ULL * 60ULL;
        break;

    case 1:
        rangeSeconds =
            7ULL * 24ULL * 60ULL * 60ULL;
        break;

    case 2:
        rangeSeconds =
            30ULL * 24ULL * 60ULL * 60ULL;
        break;

    case 3:
    default:
        rangeSeconds = 0;
        break;
    }

    uint64_t cutoffUnixSeconds = 0;

    if (
        rangeSeconds > 0 &&
        nowUnixSeconds > rangeSeconds
        )
    {
        cutoffUnixSeconds =
            nowUnixSeconds -
            rangeSeconds;
    }

    uint32_t includedSessions = 0;

    int64_t totalSessionMilliseconds = 0;
    int64_t totalCombatMilliseconds = 0;

    int64_t totalFoodActiveMilliseconds = 0;
    int64_t totalUtilityActiveMilliseconds = 0;

    int64_t totalFoodCombatMilliseconds = 0;
    int64_t totalUtilityCombatMilliseconds = 0;

    uint64_t totalFoodUses = 0;
    uint64_t totalUtilityUses = 0;

    int64_t totalMetabolicConfirmedMilliseconds = 0;
    int64_t totalMetabolicInferredMilliseconds = 0;
    int64_t totalMetabolicUnknownMilliseconds = 0;

    int64_t totalUtilityPrimerConfirmedMilliseconds = 0;
    int64_t totalUtilityPrimerInferredMilliseconds = 0;
    int64_t totalUtilityPrimerUnknownMilliseconds = 0;

    uint64_t totalFoodReplacements = 0;
    uint64_t totalUtilityReplacements = 0;

    int64_t totalFoodWastedMilliseconds = 0;
    int64_t totalUtilityWastedMilliseconds = 0;

    std::vector<float>
        foodCoverageHistory;

    std::vector<float>
        utilityCoverageHistory;

    std::vector<const SessionHistoryRecord*>
        selectedHistoryRecords;

    std::vector<SessionConsumableUsage>
        aggregateFoodUsage;

    std::vector<SessionConsumableUsage>
        aggregateUtilityUsage;

    const auto AddUsage =
        [](
            std::vector<SessionConsumableUsage>& aggregate,
            const SessionConsumableUsage& usage
            )
        {
            for (
                SessionConsumableUsage& existing :
                aggregate
                )
            {
                if (
                    existing.skillID ==
                    usage.skillID
                    )
                {
                    existing.uses +=
                        usage.uses;

                    return;
                }
            }

            aggregate.push_back(
                usage
            );
        };

    for (
        const SessionHistoryRecord& record :
        history
        )
    {
        if (
            cutoffUnixSeconds != 0 &&
            record.endedUnixSeconds <
            cutoffUnixSeconds
            )
        {
            continue;
        }

        if (selectedCharacterMode == 1)
        {
            if (
                currentCharacterName.empty() ||
                record.characterName !=
                currentCharacterName
                )
            {
                continue;
            }
        }
        else if (selectedCharacterMode == 2)
        {
            if (
                record.characterName !=
                selectedCharacterName
                )
            {
                continue;
            }
        }

        ++includedSessions;

        selectedHistoryRecords.push_back(
            &record
        );

        const SessionStats& stats =
            record.stats;

        const float foodCoverageForSession =
            stats.sessionMilliseconds > 0
            ? static_cast<float>(
                static_cast<double>(
                    stats.foodActiveMilliseconds
                    ) *
                100.0 /
                static_cast<double>(
                    stats.sessionMilliseconds
                    )
                )
            : 0.0f;

        const float utilityCoverageForSession =
            stats.sessionMilliseconds > 0
            ? static_cast<float>(
                static_cast<double>(
                    stats.utilityActiveMilliseconds
                    ) *
                100.0 /
                static_cast<double>(
                    stats.sessionMilliseconds
                    )
                )
            : 0.0f;

        foodCoverageHistory.push_back(
            foodCoverageForSession
        );

        utilityCoverageHistory.push_back(
            utilityCoverageForSession
        );

        totalSessionMilliseconds +=
            stats.sessionMilliseconds;

        totalCombatMilliseconds +=
            stats.combatMilliseconds;

        totalFoodActiveMilliseconds +=
            stats.foodActiveMilliseconds;

        totalUtilityActiveMilliseconds +=
            stats.utilityActiveMilliseconds;

        totalFoodCombatMilliseconds +=
            stats.foodCombatMilliseconds;

        totalUtilityCombatMilliseconds +=
            stats.utilityCombatMilliseconds;

        totalMetabolicConfirmedMilliseconds +=
            stats.metabolicPrimerConfirmedMilliseconds;

        totalMetabolicInferredMilliseconds +=
            stats.metabolicPrimerInferredMilliseconds;

        totalMetabolicUnknownMilliseconds +=
            stats.metabolicPrimerUnknownMilliseconds;

        totalUtilityPrimerConfirmedMilliseconds +=
            stats.utilityPrimerConfirmedMilliseconds;

        totalUtilityPrimerInferredMilliseconds +=
            stats.utilityPrimerInferredMilliseconds;

        totalUtilityPrimerUnknownMilliseconds +=
            stats.utilityPrimerUnknownMilliseconds;

        totalFoodReplacements +=
            stats.foodReplacements;

        totalUtilityReplacements +=
            stats.utilityReplacements;

        totalFoodWastedMilliseconds +=
            stats.foodWastedMilliseconds;

        totalUtilityWastedMilliseconds +=
            stats.utilityWastedMilliseconds;

        for (
            const SessionConsumableUsage& usage :
            stats.foodUsage
            )
        {
            totalFoodUses +=
                usage.uses;

            AddUsage(
                aggregateFoodUsage,
                usage
            );
        }

        for (
            const SessionConsumableUsage& usage :
            stats.utilityUsage
            )
        {
            totalUtilityUses +=
                usage.uses;

            AddUsage(
                aggregateUtilityUsage,
                usage
            );
        }
    }

    if (includedSessions == 0)
    {
        ImGui::Spacing();

        ImGui::TextDisabled(
            "No completed sessions match the selected range and character."
        );

        ImGui::TextDisabled(
            "A session is saved when you press Reset Session or when FoodReminder unloads."
        );

        return;
    }

    const auto FormatHistoryDuration =
        [](int64_t milliseconds)
        {
            const int64_t totalSeconds =
                milliseconds / 1000;

            const int64_t days =
                totalSeconds /
                86400;

            const int64_t hours =
                (
                    totalSeconds %
                    86400
                    ) /
                3600;

            const int64_t minutes =
                (
                    totalSeconds %
                    3600
                    ) /
                60;

            char buffer[64] = {};

            if (days > 0)
            {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%lldd %02lldh %02lldm",
                    days,
                    hours,
                    minutes
                );
            }
            else
            {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%02lldh %02lldm",
                    hours,
                    minutes
                );
            }

            return std::string(
                buffer
            );
        };

    const auto CoveragePercent =
        [](
            int64_t activeMilliseconds,
            int64_t totalMilliseconds
            )
        {
            if (totalMilliseconds <= 0)
            {
                return 0.0;
            }

            return
                static_cast<double>(
                    activeMilliseconds
                    ) *
                100.0 /
                static_cast<double>(
                    totalMilliseconds
                    );
        };

    const double foodSessionCoverage =
        CoveragePercent(
            totalFoodActiveMilliseconds,
            totalSessionMilliseconds
        );

    const double utilitySessionCoverage =
        CoveragePercent(
            totalUtilityActiveMilliseconds,
            totalSessionMilliseconds
        );

    const double foodCombatCoverage =
        CoveragePercent(
            totalFoodCombatMilliseconds,
            totalCombatMilliseconds
        );

    const double utilityCombatCoverage =
        CoveragePercent(
            totalUtilityCombatMilliseconds,
            totalCombatMilliseconds
        );

    uint64_t estimatedFoodCostCopper = 0;
    uint64_t estimatedUtilityCostCopper = 0;
    bool hasMissingFoodPrice = false;
    bool hasMissingUtilityPrice = false;

    uint64_t unpricedFoodUses = 0;
    uint64_t unpricedUtilityUses = 0;

    for (
        const SessionConsumableUsage& usage :
        aggregateFoodUsage
        )
    {
        const ConsumableInfo& info =
            ConsumableData::GetFoodInfo(
                usage.skillID
            );

        if (
            std::string(info.label) ==
            "Unknown" ||
            info.itemID == 0
            )
        {
            hasMissingFoodPrice = true;

            unpricedFoodUses +=
                usage.uses;

            continue;
        }

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
            estimatedFoodCostCopper +=
                static_cast<uint64_t>(
                    price.sellUnitPrice
                    ) *
                static_cast<uint64_t>(
                    usage.uses
                    );
        }
        else
        {
            hasMissingFoodPrice = true;

            unpricedFoodUses +=
                usage.uses;
        }
    }

    for (
        const SessionConsumableUsage& usage :
        aggregateUtilityUsage
        )
    {
        const ConsumableInfo& info =
            ConsumableData::GetUtilityInfo(
                usage.skillID
            );

        if (
            std::string(info.label) ==
            "Unknown" ||
            info.itemID == 0
            )
        {
            hasMissingUtilityPrice = true;

            unpricedUtilityUses +=
                usage.uses;

            continue;
        }

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
            estimatedUtilityCostCopper +=
                static_cast<uint64_t>(
                    price.sellUnitPrice
                    ) *
                static_cast<uint64_t>(
                    usage.uses
                    );
        }
        else
        {
            hasMissingUtilityPrice = true;

            unpricedUtilityUses +=
                usage.uses;
        }
    }

    const uint64_t estimatedTotalCostCopper =
        estimatedFoodCostCopper +
        estimatedUtilityCostCopper;

    std::vector<float>
        estimatedSpendHistoryGold;

    estimatedSpendHistoryGold.reserve(
        selectedHistoryRecords.size()
    );

    for (
        const SessionHistoryRecord* record :
        selectedHistoryRecords
        )
    {
        uint64_t sessionCostCopper = 0;

        if (record != nullptr)
        {
            for (
                const SessionConsumableUsage& usage :
                record->stats.foodUsage
                )
            {
                const ConsumableInfo& info =
                    ConsumableData::GetFoodInfo(
                        usage.skillID
                    );

                if (
                    std::string(info.label) ==
                    "Unknown" ||
                    info.itemID == 0
                    )
                {
                    continue;
                }

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
                    sessionCostCopper +=
                        static_cast<uint64_t>(
                            price.sellUnitPrice
                            ) *
                        static_cast<uint64_t>(
                            usage.uses
                            );
                }
            }

            for (
                const SessionConsumableUsage& usage :
                record->stats.utilityUsage
                )
            {
                const ConsumableInfo& info =
                    ConsumableData::GetUtilityInfo(
                        usage.skillID
                    );

                if (
                    std::string(info.label) ==
                    "Unknown" ||
                    info.itemID == 0
                    )
                {
                    continue;
                }

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
                    sessionCostCopper +=
                        static_cast<uint64_t>(
                            price.sellUnitPrice
                            ) *
                        static_cast<uint64_t>(
                            usage.uses
                            );
                }
            }
        }

        estimatedSpendHistoryGold.push_back(
            static_cast<float>(
                static_cast<double>(
                    sessionCostCopper
                    ) /
                10000.0
                )
        );
    }

    const ImVec4 foodColor(
        0.35f,
        0.90f,
        0.45f,
        1.00f
    );

    const ImVec4 utilityColor(
        0.45f,
        0.75f,
        1.00f,
        1.00f
    );

    const ImVec4 attentionColor(
        1.00f,
        0.78f,
        0.25f,
        1.00f
    );

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextColored(
        attentionColor,
        "SUMMARY"
    );

    ImGui::Text(
        "Sessions: %u  |  Tracked: %s  |  Combat: %s",
        includedSessions,
        FormatHistoryDuration(
            totalSessionMilliseconds
        ).c_str(),
        FormatHistoryDuration(
            totalCombatMilliseconds
        ).c_str()
    );

    ImGui::TextColored(
        foodColor,
        "Food"
    );

    ImGui::SameLine();

    ImGui::Text(
        "Uses %llu  |  Coverage %.1f%%  |  Cost %s%s",
        static_cast<
        unsigned long long
        >(
            totalFoodUses
            ),
        foodSessionCoverage,
        FormatCoinValue(
            estimatedFoodCostCopper
        ).c_str(),
        hasMissingFoodPrice
        ? " + unpriced"
        : ""
    );

    ImGui::TextColored(
        utilityColor,
        "Utility"
    );

    ImGui::SameLine();

    ImGui::Text(
        "Uses %llu  |  Coverage %.1f%%  |  Cost %s%s",
        static_cast<
        unsigned long long
        >(
            totalUtilityUses
            ),
        utilitySessionCoverage,
        FormatCoinValue(
            estimatedUtilityCostCopper
        ).c_str(),
        hasMissingUtilityPrice
        ? " + unpriced"
        : ""
    );

    ImGui::TextColored(
        attentionColor,
        "Total"
    );

    ImGui::SameLine();

    ImGui::Text(
        "%s",
        FormatCoinValue(
            estimatedTotalCostCopper
        ).c_str()
    );

    const uint64_t totalUnpricedUses =
        unpricedFoodUses +
        unpricedUtilityUses;

    if (totalUnpricedUses > 0)
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "+ %llu unpriced use%s",
            static_cast<
            unsigned long long
            >(
                totalUnpricedUses
                ),
            totalUnpricedUses == 1
            ? ""
            : "s"
        );
    }

    if (
        ImGui::CollapsingHeader(
            "Coverage & Waste Details"
        )
        )
    {
        ImGui::Indent();

        ImGui::TextColored(
            foodColor,
            "FOOD"
        );

        ImGui::Text(
            "Session Coverage: %.1f%%",
            foodSessionCoverage
        );

        if (totalCombatMilliseconds > 0)
        {
            ImGui::Text(
                "Combat Coverage: %.1f%%",
                foodCombatCoverage
            );
        }
        else
        {
            ImGui::TextDisabled(
                "Combat Coverage: No combat recorded"
            );
        }

        ImGui::Text(
            "Replacements: %llu",
            static_cast<
            unsigned long long
            >(
                totalFoodReplacements
                )
        );

        ImGui::Text(
            "Wasted Duration: %s",
            FormatHistoryDuration(
                totalFoodWastedMilliseconds
            ).c_str()
        );

        ImGui::Spacing();

        ImGui::TextColored(
            utilityColor,
            "UTILITY"
        );

        ImGui::Text(
            "Session Coverage: %.1f%%",
            utilitySessionCoverage
        );

        if (totalCombatMilliseconds > 0)
        {
            ImGui::Text(
                "Combat Coverage: %.1f%%",
                utilityCombatCoverage
            );
        }
        else
        {
            ImGui::TextDisabled(
                "Combat Coverage: No combat recorded"
            );
        }

        ImGui::Text(
            "Replacements: %llu",
            static_cast<
            unsigned long long
            >(
                totalUtilityReplacements
                )
        );

        ImGui::Text(
            "Wasted Duration: %s",
            FormatHistoryDuration(
                totalUtilityWastedMilliseconds
            ).c_str()
        );

        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (
        ImGui::CollapsingHeader(
            "Trends",
            ImGuiTreeNodeFlags_DefaultOpen
        )
        )
    {
        ImGui::Indent();

        ImGui::TextDisabled(
            "Each point represents one completed session in the selected range."
        );

        const auto DrawSessionCoverageDots =
            [](
                const char* label,
                const char* id,
                const std::vector<float>& values,
                double averageCoverage,
                const ImVec4& goodColor,
                const ImVec4& attentionColor
                )
            {
                if (values.empty())
                {
                    return;
                }

                ImGui::TextColored(
                    goodColor,
                    "%s",
                    label
                );

                ImGui::SameLine();

                ImGui::TextDisabled(
                    "Avg %.1f%%  |  %d sessions",
                    averageCoverage,
                    static_cast<int>(
                        values.size()
                        )
                );

                const float radius =
                    4.0f;

                const float diameter =
                    radius * 2.0f;

                const float spacing =
                    8.0f;

                const float availableWidth =
                    ImGui::GetContentRegionAvail().x;

                const float rowHeight =
                    diameter + 4.0f;

                const ImVec2 start =
                    ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton(
                    id,
                    ImVec2(
                        availableWidth,
                        rowHeight
                    )
                );

                ImDrawList* drawList =
                    ImGui::GetWindowDrawList();

                float x =
                    start.x + radius;

                const float y =
                    start.y + rowHeight * 0.5f;

                for (
                    size_t i = 0;
                    i < values.size();
                    ++i
                    )
                {
                    if (
                        x + radius >
                        start.x + availableWidth
                        )
                    {
                        break;
                    }

                    ImVec4 pointColor =
                        goodColor;

                    if (values[i] < 90.0f)
                    {
                        pointColor =
                            ImVec4(
                                1.00f,
                                0.40f,
                                0.40f,
                                1.00f
                            );
                    }
                    else if (values[i] < 99.95f)
                    {
                        pointColor =
                            attentionColor;
                    }

                    drawList->AddCircleFilled(
                        ImVec2(
                            x,
                            y
                        ),
                        radius,
                        ImGui::ColorConvertFloat4ToU32(
                            pointColor
                        )
                    );

                    x +=
                        diameter +
                        spacing;
                }
            };

        const auto DrawSessionSpendSparkline =
            [](
                const char* label,
                const char* id,
                const std::vector<float>& values,
                const ImVec4& lineColor
                )
            {
                if (values.empty())
                {
                    return;
                }

                ImGui::TextColored(
                    lineColor,
                    "%s",
                    label
                );

                const float chartHeight =
                    24.0f;

                const float chartWidth =
                    ImGui::GetContentRegionAvail().x;

                const ImVec2 start =
                    ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton(
                    id,
                    ImVec2(
                        chartWidth,
                        chartHeight
                    )
                );

                ImDrawList* drawList =
                    ImGui::GetWindowDrawList();

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
                        minValue = value;
                    }

                    if (value > maxValue)
                    {
                        maxValue = value;
                    }
                }

                const float denominator =
                    maxValue > minValue
                    ? maxValue - minValue
                    : 1.0f;

                std::vector<ImVec2>
                    points;

                points.reserve(
                    values.size()
                );

                for (
                    size_t i = 0;
                    i < values.size();
                    ++i
                    )
                {
                    const float normalized =
                        maxValue > minValue
                        ? (
                            values[i] -
                            minValue
                            ) /
                        denominator
                        : 0.5f;

                    const float x =
                        values.size() > 1
                        ? start.x +
                        (
                            static_cast<float>(
                                i
                                ) /
                            static_cast<float>(
                                values.size() - 1
                                )
                            ) *
                        chartWidth
                        : start.x +
                        chartWidth * 0.5f;

                    const float y =
                        start.y +
                        chartHeight -
                        2.0f -
                        (
                            normalized *
                            (
                                chartHeight -
                                4.0f
                                )
                            );

                    points.emplace_back(
                        x,
                        y
                    );
                }

                const ImU32 lineColorU32 =
                    ImGui::ColorConvertFloat4ToU32(
                        lineColor
                    );

                if (points.size() >= 2)
                {
                    drawList->AddPolyline(
                        points.data(),
                        static_cast<int>(
                            points.size()
                            ),
                        lineColorU32,
                        false,
                        2.0f
                    );
                }
                else
                {
                    drawList->AddCircleFilled(
                        points.front(),
                        2.5f,
                        lineColorU32
                    );
                }
            };

        if (!foodCoverageHistory.empty())
        {
            DrawSessionCoverageDots(
                "Food Coverage",
                "##FoodCoverageHistoryDots",
                foodCoverageHistory,
                foodSessionCoverage,
                foodColor,
                attentionColor
            );
        }

        if (!utilityCoverageHistory.empty())
        {
            DrawSessionCoverageDots(
                "Utility Coverage",
                "##UtilityCoverageHistoryDots",
                utilityCoverageHistory,
                utilitySessionCoverage,
                utilityColor,
                attentionColor
            );
        }

        if (!estimatedSpendHistoryGold.empty())
        {
            float maxSpendGold = 0.0f;

            for (
                const float value :
            estimatedSpendHistoryGold
                )
            {
                if (value > maxSpendGold)
                {
                    maxSpendGold = value;
                }
            }

            const float spendScaleMax =
                maxSpendGold > 0.0f
                ? maxSpendGold
                : 1.0f;

            DrawSessionSpendSparkline(
                "Estimated Spend",
                "##EstimatedSpendHistoryLine",
                estimatedSpendHistoryGold,
                attentionColor
            );

            ImGui::TextDisabled(
                "Total: %s  |  Sessions: %d  |  Current Trading Post sell prices",
                FormatCoinValue(
                    estimatedTotalCostCopper
                ).c_str(),
                static_cast<int>(
                    estimatedSpendHistoryGold.size()
                    )
            );
        }

        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (
        ImGui::CollapsingHeader(
            "Per-Item Usage"
        )
        )
    {
        ImGui::Indent();

        ImGui::TextDisabled(
            "Usage totals for completed sessions in the selected range."
        );

        const auto DrawItemUsageTable =
            [](
                const char* tableID,
                const char* heading,
                const std::vector<SessionConsumableUsage>& usageList,
                bool isFood,
                const ImVec4& headingColor
                )
            {
                ImGui::Spacing();

                ImGui::TextColored(
                    headingColor,
                    "%s",
                    heading
                );

                if (usageList.empty())
                {
                    ImGui::TextDisabled(
                        "No recorded uses."
                    );

                    return;
                }

                std::vector<SessionConsumableUsage>
                    sortedUsage =
                    usageList;

                for (
                    size_t i = 0;
                    i < sortedUsage.size();
                    ++i
                    )
                {
                    for (
                        size_t j = i + 1;
                        j < sortedUsage.size();
                        ++j
                        )
                    {
                        if (
                            sortedUsage[j].uses >
                            sortedUsage[i].uses
                            )
                        {
                            const SessionConsumableUsage temp =
                                sortedUsage[i];

                            sortedUsage[i] =
                                sortedUsage[j];

                            sortedUsage[j] =
                                temp;
                        }
                    }
                }

                if (
                    ImGui::BeginTable(
                        tableID,
                        3,
                        ImGuiTableFlags_BordersInnerH |
                        ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingStretchProp
                    )
                    )
                {
                    ImGui::TableSetupColumn(
                        "Item",
                        ImGuiTableColumnFlags_WidthStretch,
                        2.3f
                    );

                    ImGui::TableSetupColumn(
                        "Uses",
                        ImGuiTableColumnFlags_WidthFixed,
                        62.0f
                    );

                    ImGui::TableSetupColumn(
                        "Est. Spend",
                        ImGuiTableColumnFlags_WidthFixed,
                        110.0f
                    );

                    ImGui::TableHeadersRow();

                    for (
                        const SessionConsumableUsage& usage :
                        sortedUsage
                        )
                    {
                        const ConsumableInfo& info =
                            isFood
                            ? ConsumableData::GetFoodInfo(
                                usage.skillID
                            )
                            : ConsumableData::GetUtilityInfo(
                                usage.skillID
                            );

                        std::string itemName;

                        if (
                            std::string(info.label) !=
                            "Unknown"
                            )
                        {
                            itemName =
                                info.name != nullptr &&
                                info.name[0] != '\0'
                                ? info.name
                                : info.label;
                        }
                        else
                        {
                            char unknownBuffer[64] = {};

                            snprintf(
                                unknownBuffer,
                                sizeof(unknownBuffer),
                                "Effect ID %u",
                                usage.skillID
                            );

                            itemName =
                                unknownBuffer;
                        }

                        bool hasPrice = false;
                        uint64_t spendCopper = 0;

                        if (info.itemID != 0)
                        {
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
                                hasPrice = true;

                                spendCopper =
                                    static_cast<uint64_t>(
                                        price.sellUnitPrice
                                        ) *
                                    static_cast<uint64_t>(
                                        usage.uses
                                        );
                            }
                        }

                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);

                        ImGui::TextUnformatted(
                            itemName.c_str()
                        );

                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();

                            ImGui::Text(
                                "Effect ID: %u",
                                usage.skillID
                            );

                            if (info.itemID != 0)
                            {
                                ImGui::Text(
                                    "Item ID: %u",
                                    info.itemID
                                );
                            }

                            ImGui::EndTooltip();
                        }

                        ImGui::TableSetColumnIndex(1);

                        ImGui::Text(
                            "%u",
                            usage.uses
                        );

                        ImGui::TableSetColumnIndex(2);

                        if (hasPrice)
                        {
                            ImGui::TextUnformatted(
                                FormatCoinValue(
                                    spendCopper
                                ).c_str()
                            );
                        }
                        else
                        {
                            ImGui::TextDisabled(
                                "Unpriced"
                            );
                        }
                    }

                    ImGui::EndTable();
                }
            };

        DrawItemUsageTable(
            "HistoryFoodUsageTable",
            "FOOD ITEMS",
            aggregateFoodUsage,
            true,
            foodColor
        );

        DrawItemUsageTable(
            "HistoryUtilityUsageTable",
            "UTILITY ITEMS",
            aggregateUtilityUsage,
            false,
            utilityColor
        );

        ImGui::Unindent();
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (
        ImGui::CollapsingHeader(
            "Primer Details"
        )
        )
    {
        ImGui::Indent();

        ImGui::Text(
            "Metabolic Confirmed: %s",
            FormatHistoryDuration(
                totalMetabolicConfirmedMilliseconds
            ).c_str()
        );

        ImGui::Text(
            "Metabolic Inferred*: %s",
            FormatHistoryDuration(
                totalMetabolicInferredMilliseconds
            ).c_str()
        );

        ImGui::TextDisabled(
            "Metabolic Unknown: %s",
            FormatHistoryDuration(
                totalMetabolicUnknownMilliseconds
            ).c_str()
        );

        ImGui::Text(
            "Utility Confirmed: %s",
            FormatHistoryDuration(
                totalUtilityPrimerConfirmedMilliseconds
            ).c_str()
        );

        ImGui::Text(
            "Utility Inferred*: %s",
            FormatHistoryDuration(
                totalUtilityPrimerInferredMilliseconds
            ).c_str()
        );

        ImGui::TextDisabled(
            "Utility Unknown: %s",
            FormatHistoryDuration(
                totalUtilityPrimerUnknownMilliseconds
            ).c_str()
        );

        ImGui::TextDisabled(
            "* Inferred Primer time is kept separate because ArcDPS may not resend already-active Primer state."
        );

        ImGui::Unindent();
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Cost uses current Trading Post sell prices, not historical purchase prices."
    );
}
