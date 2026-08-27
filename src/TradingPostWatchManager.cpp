#include "TradingPostWatchManager.h"

#include "TradingPostPriceManager.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace
{
    constexpr uint32_t
        AURENES_BITE_ITEM_ID =
        96356;

    constexpr int64_t
        WATCH_INTERVAL_SECONDS =
        60;

    std::mutex g_WatchMutex;

    std::vector<
        TradingPostWatchItem
    > g_WatchedItems;

    bool g_AutoWatchEnabled = true;

    bool g_HasLastCheck = false;

    std::chrono::steady_clock::
        time_point g_LastCheckTime;

    std::filesystem::path
        g_SettingsPath;

    std::filesystem::path
        BuildSettingsPath(
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

            std::filesystem::path path(
                modulePath
            );

            return
                path.parent_path() /
                L"FoodReminder_TradingPostWatch.ini";
    }

    void EnsureDefaultItemLocked()
    {
        const auto it =
            std::find_if(
                g_WatchedItems.begin(),
                g_WatchedItems.end(),
                [](
                    const TradingPostWatchItem&
                    item
                    )
                {
                    return
                        item.itemID ==
                        AURENES_BITE_ITEM_ID;
                }
            );

        if (
            it ==
            g_WatchedItems.end()
            )
        {
            TradingPostWatchItem item;

            item.itemID =
                AURENES_BITE_ITEM_ID;

            item.name =
                "Aurene's Bite";

            item.isDefault =
                true;

            g_WatchedItems.insert(
                g_WatchedItems.begin(),
                item
            );
        }
        else
        {
            it->isDefault =
                true;

            if (it->name.empty())
            {
                it->name =
                    "Aurene's Bite";
            }
        }
    }

    void SaveLocked()
    {
        if (g_SettingsPath.empty())
        {
            return;
        }

        std::ofstream file(
            g_SettingsPath,
            std::ios::trunc
        );

        if (!file.is_open())
        {
            return;
        }

        file
            << "autoWatch="
            << (
                g_AutoWatchEnabled
                ? 1
                : 0
                )
            << '\n';

        for (
            const TradingPostWatchItem&
            item :
            g_WatchedItems
            )
        {
            file
                << "item="
                << item.itemID
                << '|'
                << item.targetSellCopper
                << '|'
                << item.name
                << '\n';
        }
    }

    void LoadLocked()
    {
        g_WatchedItems.clear();

        if (!g_SettingsPath.empty())
        {
            std::ifstream file(
                g_SettingsPath
            );

            std::string line;

            while (
                file.is_open() &&
                std::getline(
                    file,
                    line
                )
                )
            {
                if (
                    line.rfind(
                        "autoWatch=",
                        0
                    ) == 0
                    )
                {
                    const std::string value =
                        line.substr(
                            10
                        );

                    g_AutoWatchEnabled =
                        value == "1" ||
                        value == "true";

                    continue;
                }

                if (
                    line.rfind(
                        "item=",
                        0
                    ) != 0
                    )
                {
                    continue;
                }

                const std::string payload =
                    line.substr(
                        5
                    );

                const size_t firstSeparator =
                    payload.find(
                        '|'
                    );

                if (
                    firstSeparator ==
                    std::string::npos
                    )
                {
                    continue;
                }

                const size_t secondSeparator =
                    payload.find(
                        '|',
                        firstSeparator + 1
                    );

                if (
                    secondSeparator ==
                    std::string::npos
                    )
                {
                    continue;
                }

                try
                {
                    TradingPostWatchItem item;

                    item.itemID =
                        static_cast<uint32_t>(
                            std::stoul(
                                payload.substr(
                                    0,
                                    firstSeparator
                                )
                            )
                            );

                    item.targetSellCopper =
                        std::stoull(
                            payload.substr(
                                firstSeparator + 1,
                                secondSeparator -
                                firstSeparator - 1
                            )
                        );

                    item.name =
                        payload.substr(
                            secondSeparator + 1
                        );

                    if (item.itemID != 0)
                    {
                        g_WatchedItems.push_back(
                            item
                        );
                    }
                }
                catch (...)
                {
                    // Ignore malformed saved rows.
                }
            }
        }

        EnsureDefaultItemLocked();
    }

    void QueueAllLocked(
        bool forceRefresh
    )
    {
        for (
            const TradingPostWatchItem&
            item :
            g_WatchedItems
            )
        {
            TradingPostPriceManager::
                RequestPrice(
                    item.itemID,
                    forceRefresh
                );
        }
    }
}

void TradingPostWatchManager::Start(
    void* moduleHandle
)
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    g_SettingsPath =
        BuildSettingsPath(
            moduleHandle
        );

    LoadLocked();

    QueueAllLocked(
        false
    );

    g_LastCheckTime =
        std::chrono::steady_clock::
        now();

    g_HasLastCheck =
        true;
}

void TradingPostWatchManager::Shutdown()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    SaveLocked();

    g_WatchedItems.clear();

    g_SettingsPath.clear();

    g_HasLastCheck =
        false;
}

void TradingPostWatchManager::Update()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    if (!g_AutoWatchEnabled)
    {
        return;
    }

    const auto now =
        std::chrono::steady_clock::
        now();

    if (!g_HasLastCheck)
    {
        g_LastCheckTime =
            now;

        g_HasLastCheck =
            true;

        QueueAllLocked(
            true
        );

        return;
    }

    const int64_t elapsedSeconds =
        std::chrono::
        duration_cast<
        std::chrono::seconds
        >(
            now -
            g_LastCheckTime
        ).count();

    if (
        elapsedSeconds <
        WATCH_INTERVAL_SECONDS
        )
    {
        return;
    }

    g_LastCheckTime =
        now;

    QueueAllLocked(
        true
    );
}

std::vector<
    TradingPostWatchItem
> TradingPostWatchManager::GetItems()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    return
        g_WatchedItems;
}

bool TradingPostWatchManager::AddItem(
    uint32_t itemID,
    const std::string& name
)
{
    if (itemID == 0)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    const auto it =
        std::find_if(
            g_WatchedItems.begin(),
            g_WatchedItems.end(),
            [
                itemID
            ](
                const TradingPostWatchItem&
                item
                )
            {
                return
                    item.itemID ==
                    itemID;
            }
                    );

    if (
        it !=
        g_WatchedItems.end()
        )
    {
        return false;
    }

    TradingPostWatchItem item;

    item.itemID =
        itemID;

    item.name =
        name;

    g_WatchedItems.push_back(
        item
    );

    SaveLocked();

    TradingPostPriceManager::
        RequestPrice(
            itemID,
            true
        );

    return true;
}

void TradingPostWatchManager::RemoveItem(
    uint32_t itemID
)
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    if (
        itemID ==
        AURENES_BITE_ITEM_ID
        )
    {
        return;
    }

    g_WatchedItems.erase(
        std::remove_if(
            g_WatchedItems.begin(),
            g_WatchedItems.end(),
            [
                itemID
            ](
                const TradingPostWatchItem&
                item
                )
            {
                return
                    item.itemID ==
                    itemID;
            }
                    ),
        g_WatchedItems.end()
    );

    SaveLocked();
}

void TradingPostWatchManager::
SetTargetSellPrice(
    uint32_t itemID,
    uint64_t targetCopper
)
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    for (
        TradingPostWatchItem& item :
        g_WatchedItems
        )
    {
        if (item.itemID == itemID)
        {
            item.targetSellCopper =
                targetCopper;

            SaveLocked();

            return;
        }
    }
}

bool TradingPostWatchManager::
IsAutoWatchEnabled()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    return
        g_AutoWatchEnabled;
}

void TradingPostWatchManager::
SetAutoWatchEnabled(
    bool enabled
)
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    g_AutoWatchEnabled =
        enabled;

    g_LastCheckTime =
        std::chrono::steady_clock::
        now();

    g_HasLastCheck =
        true;

    SaveLocked();
}

void TradingPostWatchManager::RefreshItem(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return;
    }

    TradingPostPriceManager::
        RequestPrice(
            itemID,
            true
        );
}

void TradingPostWatchManager::RefreshAll()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    QueueAllLocked(
        true
    );

    g_LastCheckTime =
        std::chrono::steady_clock::
        now();

    g_HasLastCheck =
        true;
}

int64_t TradingPostWatchManager::
GetSecondsUntilNextCheck()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    if (
        !g_AutoWatchEnabled ||
        !g_HasLastCheck
        )
    {
        return 0;
    }

    const int64_t elapsedSeconds =
        std::chrono::
        duration_cast<
        std::chrono::seconds
        >(
            std::chrono::steady_clock::
            now() -
            g_LastCheckTime
        ).count();

    if (
        elapsedSeconds >=
        WATCH_INTERVAL_SECONDS
        )
    {
        return 0;
    }

    return
        WATCH_INTERVAL_SECONDS -
        elapsedSeconds;
}
