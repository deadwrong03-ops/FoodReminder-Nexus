#include "TradingPostHistoryManager.h"

#include <Windows.h>

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
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
}

void TradingPostHistoryManager::Shutdown()
{
    std::lock_guard<std::mutex> lock(
        g_HistoryMutex
    );

    g_WatchedItemIDs.clear();
    g_History.clear();

    g_HistoryPath.clear();

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
