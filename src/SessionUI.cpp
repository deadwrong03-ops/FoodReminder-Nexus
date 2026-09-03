#include "SessionUI.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "ConsumableData.h"
#include "ConsumableMetadataManager.h"
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

void SessionUI::Render()
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




