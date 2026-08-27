#include "TradingPostHistoryManager.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr uint64_t SECONDS_PER_MINUTE =
        60;

    constexpr uint64_t SECONDS_PER_HOUR =
        60 * SECONDS_PER_MINUTE;

    constexpr uint64_t SECONDS_PER_DAY =
        24 * SECONDS_PER_HOUR;

    constexpr uint64_t FULL_DETAIL_WINDOW_SECONDS =
        1 * SECONDS_PER_DAY;

    constexpr uint64_t FIVE_MINUTE_WINDOW_END_SECONDS =
        7 * SECONDS_PER_DAY;

    constexpr uint64_t FIVE_MINUTE_BUCKET_SECONDS =
        5 * SECONDS_PER_MINUTE;

    constexpr uint64_t THIRTY_MINUTE_BUCKET_SECONDS =
        30 * SECONDS_PER_MINUTE;

    //
    // Do not rewrite the TSV after every observation.
    // Once per hour is frequent enough to keep the file bounded
    // while avoiding unnecessary disk churn.
    //
    constexpr uint64_t COMPACTION_INTERVAL_SECONDS =
        1 * SECONDS_PER_HOUR;

    std::mutex g_HistoryMutex;

    std::unordered_map<
        uint32_t,
        std::vector<
        TradingPostHistoryPoint
        >
    > g_History;

    std::unordered_set<uint32_t>
        g_WatchedItemIDs;

    std::filesystem::path
        g_HistoryPath;

    bool g_Started = false;

    uint64_t
        g_LastCompactionUnixSeconds = 0;

    uint64_t GetCurrentUnixSeconds()
    {
        return
            static_cast<uint64_t>(
                std::chrono::duration_cast<
                std::chrono::seconds
                >(
                    std::chrono::
                    system_clock::now().
                    time_since_epoch()
                ).count()
                );
    }

    std::filesystem::path BuildHistoryPath(
        void* moduleHandle
    )
    {
        wchar_t modulePath[
            MAX_PATH
        ] = {};

            const DWORD length =
                GetModuleFileNameW(
                    static_cast<HMODULE>(
                        moduleHandle
                        ),
                    modulePath,
                    MAX_PATH
                );

            if (length == 0)
            {
                return {};
            }

            const std::filesystem::path path(
                modulePath
            );

            return
                path.parent_path() /
                L"FoodReminder_TradingPostHistory.tsv";
    }

    void SortHistoryLocked()
    {
        for (
            auto& historyEntry :
            g_History
            )
        {
            std::vector<
                TradingPostHistoryPoint
            >& points =
                historyEntry.second;

            std::sort(
                points.begin(),
                points.end(),
                [](
                    const TradingPostHistoryPoint& a,
                    const TradingPostHistoryPoint& b
                    )
                {
                    return
                        a.timestampUnixSeconds <
                        b.timestampUnixSeconds;
                }
            );
        }
    }

    void LoadHistoryLocked()
    {
        g_History.clear();

        if (
            g_HistoryPath.empty() ||
            !std::filesystem::exists(
                g_HistoryPath
            )
            )
        {
            return;
        }

        std::ifstream file(
            g_HistoryPath,
            std::ios::binary
        );

        if (!file.is_open())
        {
            return;
        }

        std::string line;

        while (
            std::getline(
                file,
                line
            )
            )
        {
            if (
                line.empty() ||
                line[0] == '#'
                )
            {
                continue;
            }

            const size_t firstTab =
                line.find(
                    '\t'
                );

            if (
                firstTab ==
                std::string::npos
                )
            {
                continue;
            }

            const size_t secondTab =
                line.find(
                    '\t',
                    firstTab + 1
                );

            if (
                secondTab ==
                std::string::npos
                )
            {
                continue;
            }

            const size_t thirdTab =
                line.find(
                    '\t',
                    secondTab + 1
                );

            if (
                thirdTab ==
                std::string::npos
                )
            {
                continue;
            }

            try
            {
                const uint64_t timestamp =
                    std::stoull(
                        line.substr(
                            0,
                            firstTab
                        )
                    );

                const uint32_t itemID =
                    static_cast<uint32_t>(
                        std::stoul(
                            line.substr(
                                firstTab + 1,
                                secondTab -
                                firstTab - 1
                            )
                        )
                        );

                const uint32_t buyUnitPrice =
                    static_cast<uint32_t>(
                        std::stoul(
                            line.substr(
                                secondTab + 1,
                                thirdTab -
                                secondTab - 1
                            )
                        )
                        );

                const uint32_t sellUnitPrice =
                    static_cast<uint32_t>(
                        std::stoul(
                            line.substr(
                                thirdTab + 1
                            )
                        )
                        );

                if (
                    timestamp == 0 ||
                    itemID == 0
                    )
                {
                    continue;
                }

                TradingPostHistoryPoint point;

                point.timestampUnixSeconds =
                    timestamp;

                point.buyUnitPrice =
                    buyUnitPrice;

                point.sellUnitPrice =
                    sellUnitPrice;

                g_History[
                    itemID
                ].push_back(
                    point
                );
            }
            catch (...)
            {
                //
                // Ignore malformed or partially written rows.
                //
            }
        }

        SortHistoryLocked();
    }

    bool AppendObservationLocked(
        uint32_t itemID,
        const TradingPostHistoryPoint& point
    )
    {
        if (g_HistoryPath.empty())
        {
            return false;
        }

        const bool fileExists =
            std::filesystem::exists(
                g_HistoryPath
            );

        std::ofstream file(
            g_HistoryPath,
            std::ios::app |
            std::ios::binary
        );

        if (!file.is_open())
        {
            return false;
        }

        if (!fileExists)
        {
            file
                << "# timestamp_unix\titem_id\tbuy_unit_price\tsell_unit_price\n";
        }

        file
            << point.timestampUnixSeconds
            << '\t'
            << itemID
            << '\t'
            << point.buyUnitPrice
            << '\t'
            << point.sellUnitPrice
            << '\n';

        file.flush();

        return
            file.good();
    }

    std::vector<
        TradingPostHistoryPoint
    > BuildCompactedHistory(
        const std::vector<
        TradingPostHistoryPoint
        >& source,
        uint64_t nowUnixSeconds
    )
    {
        std::vector<
            TradingPostHistoryPoint
        > compacted;

        compacted.reserve(
            source.size()
        );

        uint64_t lastFiveMinuteBucket =
            UINT64_MAX;

        uint64_t lastThirtyMinuteBucket =
            UINT64_MAX;

        for (
            const TradingPostHistoryPoint& point :
            source
            )
        {
            uint64_t ageSeconds = 0;

            if (
                nowUnixSeconds >
                point.timestampUnixSeconds
                )
            {
                ageSeconds =
                    nowUnixSeconds -
                    point.timestampUnixSeconds;
            }

            //
            // Last 24 hours:
            // preserve every observation exactly as recorded.
            //
            if (
                ageSeconds <=
                FULL_DETAIL_WINDOW_SECONDS
                )
            {
                compacted.push_back(
                    point
                );

                continue;
            }

            //
            // Older than 24 hours but no older than 7 days:
            // retain one representative point per 5-minute bucket.
            //
            if (
                ageSeconds <=
                FIVE_MINUTE_WINDOW_END_SECONDS
                )
            {
                const uint64_t bucket =
                    point.timestampUnixSeconds /
                    FIVE_MINUTE_BUCKET_SECONDS;

                if (
                    !compacted.empty() &&
                    bucket ==
                    lastFiveMinuteBucket
                    )
                {
                    //
                    // Keep the newest observation in the bucket.
                    //
                    compacted.back() =
                        point;
                }
                else
                {
                    compacted.push_back(
                        point
                    );

                    lastFiveMinuteBucket =
                        bucket;
                }

                continue;
            }

            //
            // Older than 7 days:
            // retain one representative point per 30-minute bucket.
            //
            const uint64_t bucket =
                point.timestampUnixSeconds /
                THIRTY_MINUTE_BUCKET_SECONDS;

            if (
                !compacted.empty() &&
                bucket ==
                lastThirtyMinuteBucket
                )
            {
                //
                // Keep the newest observation in the bucket.
                //
                compacted.back() =
                    point;
            }
            else
            {
                compacted.push_back(
                    point
                );

                lastThirtyMinuteBucket =
                    bucket;
            }
        }

        return
            compacted;
    }

    bool RewriteHistoryFileLocked(
        const std::unordered_map<
        uint32_t,
        std::vector<
        TradingPostHistoryPoint
        >
        >& history
    )
    {
        if (g_HistoryPath.empty())
        {
            return false;
        }

        const std::filesystem::path
            temporaryPath =
            g_HistoryPath.string() +
            ".tmp";

        {
            std::ofstream file(
                temporaryPath,
                std::ios::trunc |
                std::ios::binary
            );

            if (!file.is_open())
            {
                return false;
            }

            file
                << "# timestamp_unix\titem_id\tbuy_unit_price\tsell_unit_price\n";

            struct FileRow
            {
                uint32_t itemID = 0;
                TradingPostHistoryPoint
                    point;
            };

            std::vector<FileRow> rows;

            size_t totalRowCount = 0;

            for (
                const auto& historyEntry :
                history
                )
            {
                totalRowCount +=
                    historyEntry.second.size();
            }

            rows.reserve(
                totalRowCount
            );

            for (
                const auto& historyEntry :
                history
                )
            {
                for (
                    const TradingPostHistoryPoint&
                    point :
                    historyEntry.second
                    )
                {
                    FileRow row;

                    row.itemID =
                        historyEntry.first;

                    row.point =
                        point;

                    rows.push_back(
                        row
                    );
                }
            }

            //
            // Keep the TSV globally chronological. This makes the
            // file easy to inspect manually and keeps future loads
            // deterministic.
            //
            std::sort(
                rows.begin(),
                rows.end(),
                [](
                    const FileRow& a,
                    const FileRow& b
                    )
                {
                    if (
                        a.point.timestampUnixSeconds !=
                        b.point.timestampUnixSeconds
                        )
                    {
                        return
                            a.point.timestampUnixSeconds <
                            b.point.timestampUnixSeconds;
                    }

                    return
                        a.itemID <
                        b.itemID;
                }
            );

            for (
                const FileRow& row :
                rows
                )
            {
                file
                    << row.point.timestampUnixSeconds
                    << '\t'
                    << row.itemID
                    << '\t'
                    << row.point.buyUnitPrice
                    << '\t'
                    << row.point.sellUnitPrice
                    << '\n';
            }

            file.flush();

            if (!file.good())
            {
                file.close();

                std::error_code errorCode;

                std::filesystem::remove(
                    temporaryPath,
                    errorCode
                );

                return false;
            }
        }

        //
        // Replace the original only after the temporary file was
        // written successfully. MOVEFILE_WRITE_THROUGH asks Windows
        // to finish the replacement before returning.
        //
        if (
            !MoveFileExW(
                temporaryPath.c_str(),
                g_HistoryPath.c_str(),
                MOVEFILE_REPLACE_EXISTING |
                MOVEFILE_WRITE_THROUGH
            )
            )
        {
            std::error_code errorCode;

            std::filesystem::remove(
                temporaryPath,
                errorCode
            );

            return false;
        }

        return true;
    }

    void CompactHistoryLocked(
        uint64_t nowUnixSeconds,
        bool forceRewrite
    )
    {
        if (
            g_HistoryPath.empty() ||
            nowUnixSeconds == 0
            )
        {
            return;
        }

        std::unordered_map<
            uint32_t,
            std::vector<
            TradingPostHistoryPoint
            >
        > compactedHistory;

        compactedHistory.reserve(
            g_History.size()
        );

        size_t originalCount = 0;
        size_t compactedCount = 0;

        for (
            const auto& historyEntry :
            g_History
            )
        {
            originalCount +=
                historyEntry.second.size();

            std::vector<
                TradingPostHistoryPoint
            > compacted =
                BuildCompactedHistory(
                    historyEntry.second,
                    nowUnixSeconds
                );

            compactedCount +=
                compacted.size();

            compactedHistory.emplace(
                historyEntry.first,
                std::move(
                    compacted
                )
            );
        }

        //
        // If nothing was removed, avoid rewriting unless the caller
        // explicitly requested a startup normalization pass.
        //
        if (
            !forceRewrite &&
            compactedCount ==
            originalCount
            )
        {
            g_LastCompactionUnixSeconds =
                nowUnixSeconds;

            return;
        }

        if (
            RewriteHistoryFileLocked(
                compactedHistory
            )
            )
        {
            g_History =
                std::move(
                    compactedHistory
                );

            g_LastCompactionUnixSeconds =
                nowUnixSeconds;
        }
    }

    void MaybeCompactHistoryLocked(
        uint64_t nowUnixSeconds
    )
    {
        if (
            g_LastCompactionUnixSeconds != 0 &&
            nowUnixSeconds >=
            g_LastCompactionUnixSeconds &&
            (
                nowUnixSeconds -
                g_LastCompactionUnixSeconds
                ) <
            COMPACTION_INTERVAL_SECONDS
            )
        {
            return;
        }

        CompactHistoryLocked(
            nowUnixSeconds,
            false
        );
    }
}

void TradingPostHistoryManager::Start(
    void* moduleHandle
)
{
    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    if (g_Started)
    {
        return;
    }

    g_HistoryPath =
        BuildHistoryPath(
            moduleHandle
        );

    g_WatchedItemIDs.clear();

    LoadHistoryLocked();

    g_Started = true;

    const uint64_t nowUnixSeconds =
        GetCurrentUnixSeconds();

    //
    // Normalize/compact old history immediately on startup.
    //
    CompactHistoryLocked(
        nowUnixSeconds,
        true
    );

    //
    // Even if the rewrite failed, do not hammer the disk repeatedly
    // during this same startup session.
    //
    if (
        g_LastCompactionUnixSeconds == 0
        )
    {
        g_LastCompactionUnixSeconds =
            nowUnixSeconds;
    }
}

void TradingPostHistoryManager::Shutdown()
{
    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    g_WatchedItemIDs.clear();
    g_History.clear();

    g_HistoryPath.clear();

    g_LastCompactionUnixSeconds = 0;

    g_Started = false;
}

void TradingPostHistoryManager::
RegisterWatchedItem(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    if (!g_Started)
    {
        return;
    }

    g_WatchedItemIDs.insert(
        itemID
    );
}

void TradingPostHistoryManager::
UnregisterWatchedItem(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    g_WatchedItemIDs.erase(
        itemID
    );
}

void TradingPostHistoryManager::
RecordObservation(
    uint32_t itemID,
    uint32_t buyUnitPrice,
    uint32_t sellUnitPrice,
    uint64_t timestampUnixSeconds
)
{
    if (
        itemID == 0 ||
        timestampUnixSeconds == 0
        )
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    if (
        !g_Started ||
        g_WatchedItemIDs.find(
            itemID
        ) ==
        g_WatchedItemIDs.end()
        )
    {
        return;
    }

    std::vector<
        TradingPostHistoryPoint
    >& itemHistory =
        g_History[
            itemID
        ];

    //
    // Prevent an accidental exact duplicate when two successful
    // requests for the same item finish in the same second.
    //
    if (!itemHistory.empty())
    {
        const TradingPostHistoryPoint& last =
            itemHistory.back();

        if (
            last.timestampUnixSeconds ==
            timestampUnixSeconds &&
            last.buyUnitPrice ==
            buyUnitPrice &&
            last.sellUnitPrice ==
            sellUnitPrice
            )
        {
            return;
        }
    }

    TradingPostHistoryPoint point;

    point.timestampUnixSeconds =
        timestampUnixSeconds;

    point.buyUnitPrice =
        buyUnitPrice;

    point.sellUnitPrice =
        sellUnitPrice;

    if (
        !AppendObservationLocked(
            itemID,
            point
        )
        )
    {
        //
        // Do not pretend an observation is persistent when the
        // append failed. A later successful refresh can try again.
        //
        return;
    }

    itemHistory.push_back(
        point
    );

    //
    // The newest 24 hours stay at full detail. Once per hour,
    // compact older data into the longer-term sampling tiers.
    //
    MaybeCompactHistoryLocked(
        timestampUnixSeconds
    );
}

std::vector<
    TradingPostHistoryPoint
> TradingPostHistoryManager::
GetHistory(
    uint32_t itemID
)
{
    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    const auto it =
        g_History.find(
            itemID
        );

    if (
        it ==
        g_History.end()
        )
    {
        return {};
    }

    return
        it->second;
}

size_t TradingPostHistoryManager::
GetObservationCount(
    uint32_t itemID
)
{
    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    const auto it =
        g_History.find(
            itemID
        );

    if (
        it ==
        g_History.end()
        )
    {
        return 0;
    }

    return
        it->second.size();
}
