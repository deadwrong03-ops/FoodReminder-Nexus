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

    const ImVec2 overlayPosition(
        displaySize.x * 0.5f,
        displaySize.y * 0.10f
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
            150.0f
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
            18.0f,
            12.0f
        )
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowRounding,
        6.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowBorderSize,
        2.0f +
        1.5f * pulse
    );

    ImGui::PushStyleColor(
        ImGuiCol_WindowBg,
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
            IM_COL32(255, 196, 40, 255),
            IM_COL32(80, 220, 110, 255),
            IM_COL32(90, 170, 255, 255),
            IM_COL32(220, 90, 255, 255),
            IM_COL32(255, 90, 90, 255),
            IM_COL32(255, 145, 35, 255)
        };

        constexpr int CONFETTI_COUNT = 44;

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
                0.92f,
                0.78f,
                1.00f,
                1.00f
            ),
            "%s",
            alert.name.c_str()
        );

        const std::string sellText =
            FormatCoinValue(
                alert.sellUnitPrice
            );

        const std::string targetText =
            FormatCoinValue(
                alert.targetSellCopper
            );

        const std::string priceLine =
            "SELL " +
            sellText +
            "   <=   SELL TARGET " +
            targetText;

        const float priceWidth =
            ImGui::CalcTextSize(
                priceLine.c_str()
            ).x;

        ImGui::SetCursorPosX(
            (
                ImGui::GetWindowSize().x -
                priceWidth
                ) *
            0.5f
        );

        ImGui::TextColored(
            ImVec4(
                0.45f,
                0.90f,
                0.55f,
                1.00f
            ),
            "%s",
            priceLine.c_str()
        );

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

                double spreadPercent = 0.0;

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

                    spreadPercent =
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

                const char* opportunityLabel =
                    "WATCH";

                ImVec4 opportunityColor =
                    attentionColor;

                //
                // Buying signal:
                // GOOD BUY requires a favorable current sell price
                // and a sell trend that is not actively moving up.
                //
                if (
                    std::string(
                        dealLabel
                    ) ==
                    "FAVORABLE" &&
                    sellTrend.available &&
                    sellTrend.trend !=
                    PriceTrend::Up
                    )
                {
                    opportunityLabel =
                        "GOOD BUY";

                    opportunityColor =
                        goodColor;
                }
                else if (
                    std::string(
                        dealLabel
                    ) ==
                    "EXPENSIVE" ||
                    (
                        sellTrend.available &&
                        sellTrend.trend ==
                        PriceTrend::Up &&
                        sellTrend.percentChange >=
                        1.00
                        )
                    )
                {
                    opportunityLabel =
                        "OVERPRICED";

                    opportunityColor =
                        trendDownColor;
                }

                ImGui::TextColored(
                    opportunityColor,
                    "Signal %s",
                    opportunityLabel
                );
            }
            else
            {
                ImGui::TextDisabled(
                    "Deal - collecting %s history",
                    trendWindowLabel
                );

                ImGui::TextDisabled(
                    "Signal - collecting history"
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
