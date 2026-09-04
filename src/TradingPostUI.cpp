#include "TradingPostUI.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "imgui/imgui.h"

#include "Settings.h"
#include "TradingPostHistoryManager.h"
#include "TradingPostItemIndexManager.h"
#include "TradingPostPriceManager.h"
#include "TradingPostWatchManager.h"

namespace
{
    int g_TradingPostTabLastVisibleFrame =
        -1000;

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

bool TradingPostUI::WasTabVisibleRecently()
{
    const int currentImGuiFrame =
        ImGui::GetFrameCount();

    return
        currentImGuiFrame -
        g_TradingPostTabLastVisibleFrame <= 1;
}

void TradingPostUI::RenderTargetOverlay()
{
    TradingPostTargetAlert alert;

    if (
        !TradingPostWatchManager::
        TryGetActiveTargetAlert(
            alert
        )
        )
    {
        return;
    }

    const ImVec2 displaySize =
        ImGui::GetIO().
        DisplaySize;

    const float overlayWidth =
        displaySize.x > 800.0f
        ? 720.0f
        : displaySize.x - 40.0f;

    const float overlayHeight =
        230.0f;

    const ImVec2 overlayPosition(
        displaySize.x * 0.5f,
        displaySize.y * 0.08f
    );

    ImGui::SetNextWindowPos(
        overlayPosition,
        ImGuiCond_Always,
        ImVec2(
            0.5f,
            0.0f
        )
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            overlayWidth,
            overlayHeight
        ),
        ImGuiCond_Always
    );

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

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(
            24.0f,
            18.0f
        )
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowRounding,
        8.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowBorderSize,
        2.0f +
        1.25f * pulse
    );

    ImGui::PushStyleColor(
        ImGuiCol_WindowBg,
        ImVec4(
            0.055f,
            0.075f,
            0.045f,
            0.97f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(
            0.70f +
            0.22f * pulse,
            0.82f +
            0.13f * pulse,
            0.16f,
            1.00f
        )
    );

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    if (
        ImGui::Begin(
            "##TradingPostTargetOverlay",
            nullptr,
            flags
        )
        )
    {
        ImDrawList* drawList =
            ImGui::GetWindowDrawList();

        const ImVec2 windowMin =
            ImGui::GetWindowPos();

        const ImVec2 windowSize =
            ImGui::GetWindowSize();

        const ImVec2 windowMax(
            windowMin.x +
            windowSize.x,
            windowMin.y +
            windowSize.y
        );

        drawList->PushClipRect(
            windowMin,
            windowMax,
            true
        );

        const ImU32 confettiColors[] =
        {
            IM_COL32(255, 196, 40, 235),
            IM_COL32(80, 220, 110, 220),
            IM_COL32(90, 170, 255, 220),
            IM_COL32(220, 90, 255, 220),
            IM_COL32(255, 90, 90, 220),
            IM_COL32(255, 145, 35, 220)
        };

        constexpr int CONFETTI_COUNT =
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
                windowMin.y +
                std::fmod(
                    celebrationTime *
                    fallSpeed +
                    seed *
                    19.0f,
                    windowSize.y +
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
                windowMin.x +
                xFraction *
                windowSize.x +
                sway;

            const float pieceSize =
                2.5f +
                std::fmod(
                    seed,
                    3.0f
                );

            drawList->AddRectFilled(
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
                                confettiColors[0]
                                )
                            )
                ],
                1.0f
            );
        }

        for (
            int burstIndex = 0;
            burstIndex < 3;
            ++burstIndex
            )
        {
            const float centerX =
                windowMin.x +
                windowSize.x *
                (
                    0.18f +
                    0.32f *
                    static_cast<float>(
                        burstIndex
                        )
                    );

            const float centerY =
                windowMin.y +
                38.0f +
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
                5.0f *
                pulse;

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

                drawList->AddLine(
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

            drawList->AddCircleFilled(
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

        drawList->AddRect(
            ImVec2(
                windowMin.x +
                2.0f,
                windowMin.y +
                2.0f
            ),
            ImVec2(
                windowMax.x -
                2.0f,
                windowMax.y -
                2.0f
            ),
            ImGui::GetColorU32(
                ImVec4(
                    0.64f +
                    0.30f * pulse,
                    0.82f +
                    0.14f * pulse,
                    0.10f,
                    0.96f
                )
            ),
            7.0f,
            0,
            2.0f +
            1.25f * pulse
        );

        const float innerTop =
            windowMin.y +
            104.0f;

        const float innerBottom =
            windowMin.y +
            174.0f;

        const float innerLeft =
            windowMin.x +
            72.0f;

        const float innerRight =
            windowMax.x -
            72.0f;

        drawList->AddRectFilled(
            ImVec2(
                innerLeft,
                innerTop
            ),
            ImVec2(
                innerRight,
                innerBottom
            ),
            IM_COL32(
                14,
                22,
                13,
                210
            ),
            6.0f
        );

        drawList->AddRect(
            ImVec2(
                innerLeft,
                innerTop
            ),
            ImVec2(
                innerRight,
                innerBottom
            ),
            IM_COL32(
                120,
                145,
                52,
                105
            ),
            6.0f,
            0,
            1.0f
        );

        drawList->AddLine(
            ImVec2(
                windowMin.x +
                windowSize.x *
                0.50f,
                innerTop +
                12.0f
            ),
            ImVec2(
                windowMin.x +
                windowSize.x *
                0.50f,
                innerBottom -
                12.0f
            ),
            IM_COL32(
                170,
                185,
                100,
                85
            ),
            1.0f
        );

        drawList->PopClipRect();

        ImGui::SetWindowFontScale(
            1.55f
        );

        const char* title =
            "TARGET REACHED!";

        const float titleWidth =
            ImGui::CalcTextSize(
                title
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
            title
        );

        ImGui::SetWindowFontScale(
            1.00f
        );

        ImGui::Spacing();

        ImGui::SetWindowFontScale(
            1.16f
        );

        const float itemWidth =
            ImGui::CalcTextSize(
                alert.name.c_str()
            ).x;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                itemWidth
                ) *
            0.5f
        );

        ImGui::TextColored(
            ImVec4(
                0.94f,
                0.82f,
                1.00f,
                1.00f
            ),
            "%s",
            alert.name.c_str()
        );

        ImGui::SetWindowFontScale(
            1.00f
        );

        const std::string sellText =
            FormatCoinValue(
                alert.sellUnitPrice
            );

        const std::string targetText =
            FormatCoinValue(
                alert.targetSellCopper
            );

        ImGui::SetCursorPosY(
            112.0f
        );

        if (
            ImGui::BeginTable(
                "##TargetReachedPriceCards",
                2,
                ImGuiTableFlags_SizingStretchSame |
                ImGuiTableFlags_NoSavedSettings
            )
            )
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(
                0
            );

            const char* currentSellLabel =
                "CURRENT SELL";

            const float currentSellLabelWidth =
                ImGui::CalcTextSize(
                    currentSellLabel
                ).x;

            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (
                    ImGui::GetColumnWidth() -
                    currentSellLabelWidth
                    ) *
                0.5f
            );

            ImGui::TextDisabled(
                "%s",
                currentSellLabel
            );

            ImGui::SetWindowFontScale(
                1.12f
            );

            const float sellWidth =
                ImGui::CalcTextSize(
                    sellText.c_str()
                ).x;

            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (
                    ImGui::GetColumnWidth() -
                    sellWidth
                    ) *
                0.5f
            );

            ImGui::TextColored(
                ImVec4(
                    0.45f,
                    0.92f,
                    0.56f,
                    1.00f
                ),
                "%s",
                sellText.c_str()
            );

            ImGui::SetWindowFontScale(
                1.00f
            );

            ImGui::TableSetColumnIndex(
                1
            );

            const char* targetLabel =
                "YOUR TARGET";

            const float targetLabelWidth =
                ImGui::CalcTextSize(
                    targetLabel
                ).x;

            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (
                    ImGui::GetColumnWidth() -
                    targetLabelWidth
                    ) *
                0.5f
            );

            ImGui::TextDisabled(
                "%s",
                targetLabel
            );

            ImGui::SetWindowFontScale(
                1.12f
            );

            const float targetWidth =
                ImGui::CalcTextSize(
                    targetText.c_str()
                ).x;

            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() +
                (
                    ImGui::GetColumnWidth() -
                    targetWidth
                    ) *
                0.5f
            );

            ImGui::TextColored(
                ImVec4(
                    1.00f,
                    0.78f,
                    0.24f,
                    1.00f
                ),
                "%s",
                targetText.c_str()
            );

            ImGui::SetWindowFontScale(
                1.00f
            );

            ImGui::EndTable();
        }

        ImGui::SetCursorPosY(
            overlayHeight -
            42.0f
        );

        const float dismissWidth =
            138.0f;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                dismissWidth
                ) *
            0.5f
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_FrameRounding,
            5.0f
        );

        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(
                0.23f,
                0.30f,
                0.10f,
                0.95f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(
                0.38f,
                0.48f,
                0.14f,
                1.00f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            ImVec4(
                0.48f,
                0.58f,
                0.18f,
                1.00f
            )
        );

        if (
            ImGui::Button(
                "Dismiss Party",
                ImVec2(
                    dismissWidth,
                    28.0f
                )
            )
            )
        {
            TradingPostWatchManager::
                DismissActiveTargetAlert();
        }

        ImGui::PopStyleColor(
            3
        );

        ImGui::PopStyleVar();
    }

    ImGui::End();

    ImGui::PopStyleColor(
        2
    );

    ImGui::PopStyleVar(
        3
    );
}

void TradingPostUI::RenderTab(
    void* moduleHandle
)
{
    g_TradingPostTabLastVisibleFrame =
        ImGui::GetFrameCount();

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

    const uint64_t trendWindowSecondsOptions[] =
    {
        15ULL * 60ULL,
        30ULL * 60ULL,
        60ULL * 60ULL,
        6ULL * 60ULL * 60ULL,
        24ULL * 60ULL * 60ULL,
        3ULL * 24ULL * 60ULL * 60ULL,
        7ULL * 24ULL * 60ULL * 60ULL,
        30ULL * 24ULL * 60ULL * 60ULL,
        90ULL * 24ULL * 60ULL * 60ULL
    };

    const char* trendWindowLabels[] =
    {
        "15m",
        "30m",
        "1h",
        "6h",
        "24h",
        "3d",
        "7d",
        "30d",
        "90d"
    };

    const uint64_t trendWindowSeconds =
        trendWindowSecondsOptions[
            g_Settings.tradingPostTrendWindowIndex
        ];

    const char* trendWindowLabel =
        trendWindowLabels[
            g_Settings.tradingPostTrendWindowIndex
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


    auto CalculateObservedTrend =
        [](
            const std::vector<
            TradingPostHistoryPoint
            >& points,
            bool useSellPrice
            )
        {
            TrendInfo result;

            if (points.size() < 2)
            {
                return result;
            }

            const TradingPostHistoryPoint&
                olderPoint =
                points.front();

            const TradingPostHistoryPoint&
                newerPoint =
                points.back();

            const uint32_t olderPrice =
                useSellPrice
                ? olderPoint.sellUnitPrice
                : olderPoint.buyUnitPrice;

            const uint32_t newerPrice =
                useSellPrice
                ? newerPoint.sellUnitPrice
                : newerPoint.buyUnitPrice;

            if (olderPrice == 0)
            {
                return result;
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

            if (result.copperChange > 0)
            {
                result.trend =
                    PriceTrend::Up;
            }
            else if (result.copperChange < 0)
            {
                result.trend =
                    PriceTrend::Down;
            }
            else
            {
                result.trend =
                    PriceTrend::Neutral;
            }

            return result;
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

            //
            // Use the same historical anchor concept as the trend:
            // find the newest observation at or before the requested
            // window boundary. This lets Deal/Signal remain useful
            // after the game has been closed for part of the window.
            //
            size_t firstSampleIndex =
                points.size();

            for (
                size_t i =
                points.size();
                i > 0;
                --i
                )
            {
                const size_t index =
                    i - 1;

                if (
                    points[
                        index
                    ].timestampUnixSeconds <=
                    startTimestamp
                            )
                {
                    firstSampleIndex =
                        index;

                    break;
                }
            }

            if (
                firstSampleIndex ==
                points.size()
                )
            {
                return
                    result;
            }

            std::vector<uint32_t>
                sellSamples;

            uint64_t totalSell = 0;

            for (
                size_t i =
                firstSampleIndex;
                i <
                points.size();
                ++i
                )
            {
                const uint32_t sellPrice =
                    points[
                        i
                    ].sellUnitPrice;

                if (sellPrice == 0)
                {
                    continue;
                }

                sellSamples.push_back(
                    sellPrice
                );

                totalSell +=
                    sellPrice;
            }

            //
            // Two observations are enough to make a useful
            // beginning-to-end assessment. More observations improve
            // the quartile estimate naturally when available.
            //
            if (
                sellSamples.size() < 2
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

            if (
                sellSamples.size() == 2
                )
            {
                result.lowerQuartileSell =
                    sellSamples.front();

                result.upperQuartileSell =
                    sellSamples.back();
            }
            else
            {
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
                        3 +
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
            }

            result.available =
                result.averageSell > 0;

            return
                result;
        };



    auto CalculateBuyWindowStats =
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
                return result;
            }

            const uint64_t newestTimestamp =
                points.back().
                timestampUnixSeconds;

            if (newestTimestamp < windowSeconds)
            {
                return result;
            }

            const uint64_t startTimestamp =
                newestTimestamp -
                windowSeconds;

            size_t firstSampleIndex =
                points.size();

            for (
                size_t i = points.size();
                i > 0;
                --i
                )
            {
                const size_t index =
                    i - 1;

                if (
                    points[index].
                    timestampUnixSeconds <=
                    startTimestamp
                    )
                {
                    firstSampleIndex =
                        index;

                    break;
                }
            }

            if (
                firstSampleIndex ==
                points.size()
                )
            {
                return result;
            }

            std::vector<uint32_t>
                buySamples;

            uint64_t totalBuy = 0;

            for (
                size_t i = firstSampleIndex;
                i < points.size();
                ++i
                )
            {
                const uint32_t buyPrice =
                    points[i].
                    buyUnitPrice;

                if (buyPrice == 0)
                {
                    continue;
                }

                buySamples.push_back(
                    buyPrice
                );

                totalBuy +=
                    buyPrice;
            }

            if (buySamples.size() < 2)
            {
                return result;
            }

            std::sort(
                buySamples.begin(),
                buySamples.end()
            );

            result.sampleCount =
                buySamples.size();

            result.averageSell =
                totalBuy /
                static_cast<uint64_t>(
                    buySamples.size()
                    );

            if (buySamples.size() == 2)
            {
                result.lowerQuartileSell =
                    buySamples.front();

                result.upperQuartileSell =
                    buySamples.back();
            }
            else
            {
                const size_t lowerIndex =
                    (
                        buySamples.size() -
                        1
                        ) /
                    4;

                const size_t upperIndex =
                    (
                        (
                            buySamples.size() -
                            1
                            ) *
                        3 +
                        3
                        ) /
                    4;

                result.lowerQuartileSell =
                    buySamples[
                        lowerIndex
                    ];

                result.upperQuartileSell =
                    buySamples[
                        upperIndex
                    ];
            }

            result.available =
                result.averageSell > 0;

            return result;
        };

    auto FormatHistorySpan =
        [](
            uint64_t seconds
            )
        {
            if (seconds >= 24ULL * 60ULL * 60ULL)
            {
                const uint64_t days =
                    seconds /
                    (24ULL * 60ULL * 60ULL);

                return
                    std::to_string(
                        days
                    ) +
                    "d";
            }

            if (seconds >= 60ULL * 60ULL)
            {
                const uint64_t hours =
                    seconds /
                    (60ULL * 60ULL);

                return
                    std::to_string(
                        hours
                    ) +
                    "h";
            }

            const uint64_t minutes =
                seconds /
                60ULL;

            return
                std::to_string(
                    minutes
                ) +
                "m";
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

        ImGui::PushStyleVar(
            ImGuiStyleVar_ChildRounding,
            5.0f
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(
                10.0f,
                8.0f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            ImVec4(
                0.07f,
                0.10f,
                0.04f,
                0.92f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_Border,
            ImVec4(
                0.68f,
                0.80f,
                0.14f,
                1.00f
            )
        );

        ImGui::BeginChild(
            "##TradingPostTargetCompactAlert",
            ImVec2(
                430.0f,
                64.0f
            ),
            true,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
        );

        ImGui::TextColored(
            attentionColor,
            "TARGET PRICE HIT!"
        );

        ImGui::SameLine(
            0.0f,
            8.0f
        );

        ImGui::TextColored(
            itemNameColor,
            "%s",
            activeTargetAlert.name.c_str()
        );

        const std::string sellText =
            FormatCoinValue(
                activeTargetAlert.sellUnitPrice
            );

        const std::string targetText =
            FormatCoinValue(
                activeTargetAlert.targetSellCopper
            );

        ImGui::TextColored(
            sellColor,
            "Sell: %s",
            sellText.c_str()
        );

        ImGui::SameLine(
            0.0f,
            12.0f
        );

        ImGui::TextDisabled(
            "Target: %s",
            targetText.c_str()
        );

        ImGui::SameLine(
            0.0f,
            12.0f
        );

        if (
            ImGui::SmallButton(
                "Dismiss"
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

        ImGui::PopStyleVar(
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

    if (ImGui::Combo(
        "Trend Window",
        &g_Settings.tradingPostTrendWindowIndex,
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
    ))
    {
        Settings::Save(moduleHandle);
    }

    ImGui::Spacing();

    ImGui::TextDisabled(
        "Historical note: Food Reminder builds its own price history from the time you begin tracking an item."
    );

    ImGui::TextDisabled(
        "No earlier market history is available on initial install."
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

        const bool targetReached =
            item.targetSellCopper > 0 &&
            hasPrice &&
            price.available &&
            price.sellUnitPrice > 0 &&
            static_cast<uint64_t>(
                price.sellUnitPrice
                ) <=
            item.targetSellCopper;

        std::string targetSummary =
            "TARGET: -";

        if (item.targetSellCopper > 0)
        {
            targetSummary =
                "TARGET: " +
                FormatCoinValue(
                    item.targetSellCopper
                );
        }

        ImGui::PushStyleVar(
            ImGuiStyleVar_CellPadding,
            ImVec2(
                8.0f,
                5.0f
            )
        );

        if (
            ImGui::BeginTable(
                "##CompactWatchItem",
                5,
                ImGuiTableFlags_SizingStretchProp |
                ImGuiTableFlags_NoSavedSettings
            )
            )
        {
            ImGui::TableSetupColumn(
                "Item",
                ImGuiTableColumnFlags_WidthStretch,
                2.15f
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
                "SellTarget",
                ImGuiTableColumnFlags_WidthFixed,
                205.0f
            );

            ImGui::TableSetupColumn(
                "StatusActions",
                ImGuiTableColumnFlags_WidthFixed,
                180.0f
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

            ImGui::SetWindowFontScale(
                1.13f
            );

            ImGui::TextColored(
                itemNameColor,
                "%s",
                item.name.c_str()
            );

            ImGui::SetWindowFontScale(
                1.00f
            );

            ImGui::TableSetColumnIndex(
                3
            );

            ImGui::TextDisabled(
                "Sell Target"
            );

            ImGui::TableSetColumnIndex(
                4
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
            else if (targetReached)
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

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(
                0
            );

            if (hasPrice)
            {
                ImGui::TextDisabled(
                    "Item ID %u  |  Updated %llu sec ago",
                    item.itemID,
                    ageSeconds
                );
            }
            else
            {
                ImGui::TextDisabled(
                    "Item ID %u  |  Price data loading...",
                    item.itemID
                );
            }

            ImGui::TableSetColumnIndex(
                1
            );

            ImGui::TextDisabled(
                "Sell"
            );

            ImGui::SameLine(
                0.0f,
                5.0f
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

            ImGui::SameLine(
                0.0f,
                5.0f
            );

            ImGui::TextColored(
                buyColor,
                "%s",
                buyText.c_str()
            );

            ImGui::TableSetColumnIndex(
                3
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
                5.0f
            );

            ImGui::SetNextItemWidth(
                38.0f
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
                5.0f
            );

            ImGui::SetNextItemWidth(
                38.0f
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

            ImGui::TableSetColumnIndex(
                4
            );

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

        uint64_t historySpanSeconds = 0;

        if (history.size() >= 2)
        {
            const uint64_t newestTimestamp =
                history.back().
                timestampUnixSeconds;

            const uint64_t oldestTimestamp =
                history.front().
                timestampUnixSeconds;

            historySpanSeconds =
                newestTimestamp >= oldestTimestamp
                ? newestTimestamp - oldestTimestamp
                : 0;
        }

        const size_t minimumAnalysisSamples =
            15;

        const double selectedWindowCoverage =
            trendWindowSeconds > 0
            ? std::min(
                1.0,
                static_cast<double>(
                    historySpanSeconds
                    ) /
                static_cast<double>(
                    trendWindowSeconds
                    )
            )
            : 0.0;

        const bool marketAnalysisReady =
            history.size() >= minimumAnalysisSamples &&
            selectedWindowCoverage >= 0.75 &&
            sellTrend.available &&
            buyTrend.available;

        const TrendInfo observedSellTrend =
            sellTrend.available
            ? sellTrend
            : CalculateObservedTrend(
                history,
                true
            );

        const TrendInfo observedBuyTrend =
            buyTrend.available
            ? buyTrend
            : CalculateObservedTrend(
                history,
                false
            );

        if (
            hasPrice &&
            price.available &&
            price.sellUnitPrice > 0
            )
        {
            const std::string spanText =
                FormatHistorySpan(
                    historySpanSeconds
                );

            const std::string windowText =
                FormatHistorySpan(
                    trendWindowSeconds
                );

            const char* listingDirection =
                "Stable";

            if (
                observedSellTrend.available &&
                observedSellTrend.trend ==
                PriceTrend::Up
                )
            {
                listingDirection =
                    "More Expensive";
            }
            else if (
                observedSellTrend.available &&
                observedSellTrend.trend ==
                PriceTrend::Down
                )
            {
                listingDirection =
                    "Cheaper";
            }
            else if (!observedSellTrend.available)
            {
                listingDirection =
                    "Developing";
            }

            const char* buyOrderDirection =
                "Stable";

            if (
                observedBuyTrend.available &&
                observedBuyTrend.trend ==
                PriceTrend::Up
                )
            {
                buyOrderDirection =
                    "Higher";
            }
            else if (
                observedBuyTrend.available &&
                observedBuyTrend.trend ==
                PriceTrend::Down
                )
            {
                buyOrderDirection =
                    "Lower";
            }
            else if (!observedBuyTrend.available)
            {
                buyOrderDirection =
                    "Developing";
            }

            const char* confidenceLabel =
                selectedWindowCoverage >= 0.90 &&
                sellTrend.available &&
                buyTrend.available
                ? "HIGH"
                : (
                    marketAnalysisReady
                    ? "MED"
                    : "LOW"
                    );

            std::string listingSummary =
                listingDirection;

            if (observedSellTrend.available)
            {
                char percentBuffer[32] = {};

                std::snprintf(
                    percentBuffer,
                    sizeof(percentBuffer),
                    " %+.2f%%",
                    observedSellTrend.percentChange
                );

                listingSummary +=
                    percentBuffer;
            }

            std::string buyOrderSummary =
                buyOrderDirection;

            if (observedBuyTrend.available)
            {
                char percentBuffer[32] = {};

                std::snprintf(
                    percentBuffer,
                    sizeof(percentBuffer),
                    " %+.2f%%",
                    observedBuyTrend.percentChange
                );

                buyOrderSummary +=
                    percentBuffer;
            }

            ImGui::TextDisabled(
                "Listings: %s  |  Buy Orders: %s  |  Conf %s  |  %s / %s",
                listingSummary.c_str(),
                buyOrderSummary.c_str(),
                confidenceLabel,
                spanText.c_str(),
                windowText.c_str()
            );
        }

        const std::string detailsHeader =
            "Details / History (" +
            std::to_string(
                history.size()
            ) +
            " observations)###DetailsHistory" +
            std::to_string(
                item.itemID
            );

        const bool detailsOpen =
            ImGui::CollapsingHeader(
                detailsHeader.c_str()
            );

        if (detailsOpen)
        {
            ImGui::Indent(
                18.0f
            );

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (
                hasPrice &&
                price.available &&
                price.sellUnitPrice > 0
                )
            {
                const DealWindowStats sellStats =
                    CalculateDealWindowStats(
                        history,
                        trendWindowSeconds
                    );

                const DealWindowStats buyStats =
                    CalculateBuyWindowStats(
                        history,
                        trendWindowSeconds
                    );

                const std::string spanText =
                    FormatHistorySpan(
                        historySpanSeconds
                    );

                const std::string windowText =
                    FormatHistorySpan(
                        trendWindowSeconds
                    );

                const int coveragePercent =
                    static_cast<int>(
                        selectedWindowCoverage *
                        100.0 +
                        0.5
                        );

                ImGui::TextUnformatted(
                    "Market Analysis"
                );

                ImGui::Separator();

                ImGui::TextColored(
                    sellColor,
                    "SELL LISTINGS"
                );

                const char* listingDirection =
                    "Stable";

                ImVec4 listingDirectionColor =
                    attentionColor;

                if (
                    observedSellTrend.available &&
                    observedSellTrend.trend ==
                    PriceTrend::Up
                    )
                {
                    listingDirection =
                        "Getting More Expensive";

                    listingDirectionColor =
                        trendDownColor;
                }
                else if (
                    observedSellTrend.available &&
                    observedSellTrend.trend ==
                    PriceTrend::Down
                    )
                {
                    listingDirection =
                        "Getting Cheaper";

                    listingDirectionColor =
                        goodColor;
                }
                else if (!observedSellTrend.available)
                {
                    listingDirection =
                        "Developing";

                    listingDirectionColor =
                        mutedColor;
                }

                ImGui::TextDisabled(
                    "Direction"
                );

                ImGui::SameLine(
                    125.0f
                );

                if (observedSellTrend.available)
                {
                    ImGui::TextColored(
                        listingDirectionColor,
                        "%s  (%+.2f%%)",
                        listingDirection,
                        observedSellTrend.percentChange
                    );
                }
                else
                {
                    ImGui::TextColored(
                        listingDirectionColor,
                        "%s",
                        listingDirection
                    );
                }

                if (
                    !marketAnalysisReady ||
                    !sellStats.available
                    )
                {
                    ImGui::TextDisabled(
                        "Tracked Avg"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    if (
                        sellStats.available &&
                        sellStats.averageSell > 0
                        )
                    {
                        const std::string trackedSellAverage =
                            FormatCoinValue(
                                sellStats.averageSell
                            );

                        const double sellVsTrackedAverage =
                            (
                                static_cast<double>(
                                    price.sellUnitPrice
                                    ) -
                                static_cast<double>(
                                    sellStats.averageSell
                                    )
                                ) /
                            static_cast<double>(
                                sellStats.averageSell
                                ) *
                            100.0;

                        ImGui::Text(
                            "%s  (%+.2f%%)",
                            trackedSellAverage.c_str(),
                            sellVsTrackedAverage
                        );
                    }
                    else
                    {
                        ImGui::TextDisabled(
                            "Developing"
                        );
                    }

                    ImGui::TextDisabled(
                        "For Buyers"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextDisabled(
                        "Developing"
                    );
                }
                else
                {
                    const double sellVsAverage =
                        (
                            static_cast<double>(
                                price.sellUnitPrice
                                ) -
                            static_cast<double>(
                                sellStats.averageSell
                                )
                            ) /
                        static_cast<double>(
                            sellStats.averageSell
                            ) *
                        100.0;

                    const char* sellPosition =
                        "Near Recent Avg";

                    ImVec4 sellPositionColor =
                        attentionColor;

                    if (sellVsAverage <= -3.0)
                    {
                        sellPosition =
                            "Well Below Recent Avg";

                        sellPositionColor =
                            goodColor;
                    }
                    else if (sellVsAverage <= -1.0)
                    {
                        sellPosition =
                            "Below Recent Avg";

                        sellPositionColor =
                            goodColor;
                    }
                    else if (sellVsAverage >= 3.0)
                    {
                        sellPosition =
                            "Well Above Recent Avg";

                        sellPositionColor =
                            trendDownColor;
                    }
                    else if (sellVsAverage >= 1.0)
                    {
                        sellPosition =
                            "Above Recent Avg";

                        sellPositionColor =
                            trendDownColor;
                    }

                    const char* buyerSignal =
                        "Typical";

                    ImVec4 buyerSignalColor =
                        attentionColor;

                    if (
                        price.sellUnitPrice <=
                        sellStats.lowerQuartileSell
                        )
                    {
                        buyerSignal =
                            observedSellTrend.trend ==
                            PriceTrend::Up
                            ? "Worth Watching"
                            : "Favorable";

                        buyerSignalColor =
                            observedSellTrend.trend ==
                            PriceTrend::Up
                            ? attentionColor
                            : goodColor;
                    }
                    else if (
                        price.sellUnitPrice >=
                        sellStats.upperQuartileSell
                        )
                    {
                        buyerSignal =
                            "Caution";

                        buyerSignalColor =
                            trendDownColor;
                    }

                    const std::string sellAverageText =
                        FormatCoinValue(
                            sellStats.averageSell
                        );

                    ImGui::TextDisabled(
                        "Recent Avg"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::Text(
                        "%s  (%+.2f%%)",
                        sellAverageText.c_str(),
                        sellVsAverage
                    );

                    ImGui::TextDisabled(
                        "Price Position"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextColored(
                        sellPositionColor,
                        "%s",
                        sellPosition
                    );

                    ImGui::TextDisabled(
                        "For Buyers"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextColored(
                        buyerSignalColor,
                        "%s",
                        buyerSignal
                    );
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(
                    buyColor,
                    "BUY ORDERS"
                );

                const char* orderDirection =
                    "Stable";

                ImVec4 orderDirectionColor =
                    attentionColor;

                if (
                    observedBuyTrend.available &&
                    observedBuyTrend.trend ==
                    PriceTrend::Up
                    )
                {
                    orderDirection =
                        "Buyers Offering More";

                    orderDirectionColor =
                        goodColor;
                }
                else if (
                    observedBuyTrend.available &&
                    observedBuyTrend.trend ==
                    PriceTrend::Down
                    )
                {
                    orderDirection =
                        "Buyers Offering Less";

                    orderDirectionColor =
                        trendDownColor;
                }
                else if (!observedBuyTrend.available)
                {
                    orderDirection =
                        "Developing";

                    orderDirectionColor =
                        mutedColor;
                }

                ImGui::TextDisabled(
                    "Direction"
                );

                ImGui::SameLine(
                    125.0f
                );

                if (observedBuyTrend.available)
                {
                    ImGui::TextColored(
                        orderDirectionColor,
                        "%s  (%+.2f%%)",
                        orderDirection,
                        observedBuyTrend.percentChange
                    );
                }
                else
                {
                    ImGui::TextColored(
                        orderDirectionColor,
                        "%s",
                        orderDirection
                    );
                }

                if (
                    !marketAnalysisReady ||
                    !buyStats.available ||
                    price.buyUnitPrice == 0
                    )
                {
                    ImGui::TextDisabled(
                        "Tracked Avg"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    if (
                        buyStats.available &&
                        buyStats.averageSell > 0 &&
                        price.buyUnitPrice > 0
                        )
                    {
                        const std::string trackedBuyAverage =
                            FormatCoinValue(
                                buyStats.averageSell
                            );

                        const double buyVsTrackedAverage =
                            (
                                static_cast<double>(
                                    price.buyUnitPrice
                                    ) -
                                static_cast<double>(
                                    buyStats.averageSell
                                    )
                                ) /
                            static_cast<double>(
                                buyStats.averageSell
                                ) *
                            100.0;

                        ImGui::Text(
                            "%s  (%+.2f%%)",
                            trackedBuyAverage.c_str(),
                            buyVsTrackedAverage
                        );
                    }
                    else
                    {
                        ImGui::TextDisabled(
                            "Developing"
                        );
                    }

                    ImGui::TextDisabled(
                        "For Sellers"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextDisabled(
                        "Developing"
                    );

                    ImGui::Spacing();

                    ImGui::TextDisabled(
                        "Analysis is based only on locally collected history."
                    );
                }
                else
                {
                    const double buyVsAverage =
                        (
                            static_cast<double>(
                                price.buyUnitPrice
                                ) -
                            static_cast<double>(
                                buyStats.averageSell
                                )
                            ) /
                        static_cast<double>(
                            buyStats.averageSell
                            ) *
                        100.0;

                    const char* buyPosition =
                        "Near Recent Avg";

                    ImVec4 buyPositionColor =
                        attentionColor;

                    if (buyVsAverage >= 3.0)
                    {
                        buyPosition =
                            "Well Above Recent Avg";

                        buyPositionColor =
                            goodColor;
                    }
                    else if (buyVsAverage >= 1.0)
                    {
                        buyPosition =
                            "Above Recent Avg";

                        buyPositionColor =
                            goodColor;
                    }
                    else if (buyVsAverage <= -3.0)
                    {
                        buyPosition =
                            "Well Below Recent Avg";

                        buyPositionColor =
                            trendDownColor;
                    }
                    else if (buyVsAverage <= -1.0)
                    {
                        buyPosition =
                            "Below Recent Avg";

                        buyPositionColor =
                            trendDownColor;
                    }

                    const char* sellerSignal =
                        "Typical";

                    ImVec4 sellerSignalColor =
                        attentionColor;

                    if (
                        price.buyUnitPrice >=
                        buyStats.upperQuartileSell
                        )
                    {
                        sellerSignal =
                            "Favorable";

                        sellerSignalColor =
                            goodColor;
                    }
                    else if (
                        price.buyUnitPrice <=
                        buyStats.lowerQuartileSell
                        )
                    {
                        sellerSignal =
                            observedBuyTrend.trend ==
                            PriceTrend::Up
                            ? "Worth Watching"
                            : "Weak";

                        sellerSignalColor =
                            observedBuyTrend.trend ==
                            PriceTrend::Up
                            ? attentionColor
                            : trendDownColor;
                    }

                    const std::string buyAverageText =
                        FormatCoinValue(
                            buyStats.averageSell
                        );

                    ImGui::TextDisabled(
                        "Recent Avg"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::Text(
                        "%s  (%+.2f%%)",
                        buyAverageText.c_str(),
                        buyVsAverage
                    );

                    ImGui::TextDisabled(
                        "Price Position"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextColored(
                        buyPositionColor,
                        "%s",
                        buyPosition
                    );

                    ImGui::TextDisabled(
                        "For Sellers"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextColored(
                        sellerSignalColor,
                        "%s",
                        sellerSignal
                    );
                }

                ImGui::Spacing();

                const char* confidenceLabel =
                    selectedWindowCoverage >= 0.90 &&
                    sellTrend.available &&
                    buyTrend.available
                    ? "HIGH"
                    : (
                        marketAnalysisReady
                        ? "MEDIUM"
                        : "LOW"
                        );

                ImVec4 confidenceColor =
                    marketAnalysisReady
                    ? (
                        selectedWindowCoverage >= 0.90
                        ? goodColor
                        : attentionColor
                        )
                    : mutedColor;

                ImGui::TextDisabled(
                    "Confidence"
                );

                ImGui::SameLine(
                    125.0f
                );

                ImGui::TextColored(
                    confidenceColor,
                    "%s",
                    confidenceLabel
                );

                ImGui::TextDisabled(
                    "Coverage"
                );

                ImGui::SameLine(
                    125.0f
                );

                ImGui::TextDisabled(
                    "%s of %s (%d%%)",
                    spanText.c_str(),
                    windowText.c_str(),
                    coveragePercent
                );

                ImGui::TextDisabled(
                    "Samples"
                );

                ImGui::SameLine(
                    125.0f
                );

                ImGui::TextDisabled(
                    "%llu",
                    static_cast<
                    unsigned long long
                    >(
                        history.size()
                        )
                );

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
                        "Spread"
                    );

                    ImGui::SameLine(
                        125.0f
                    );

                    ImGui::TextDisabled(
                        "%s (%.2f%%)",
                        spreadText.c_str(),
                        spreadPercent
                    );
                }
            }

            ImGui::Spacing();

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
            }

            ImGui::Unindent(
                18.0f
            );
        }

        ImGui::Dummy(
            ImVec2(
                0.0f,
                3.0f
            )
        );

        ImGui::Separator();

        ImGui::Dummy(
            ImVec2(
                0.0f,
                5.0f
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
                    std::string displayName =
                        result.name;

                    if (!result.variantLabel.empty())
                    {
                        displayName +=
                            " - " +
                            result.variantLabel;
                    }

                    const std::string label =
                        displayName +
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
        std::string selectedDisplayName =
            selectedItem.name;

        if (!selectedItem.variantLabel.empty())
        {
            selectedDisplayName +=
                " - " +
                selectedItem.variantLabel;
        }

        const bool added =
            TradingPostWatchManager::
            AddItem(
                selectedItem.itemID,
                selectedDisplayName
            );

        if (added)
        {
            searchStatus =
                "Added: " +
                selectedDisplayName +
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
                selectedDisplayName +
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
