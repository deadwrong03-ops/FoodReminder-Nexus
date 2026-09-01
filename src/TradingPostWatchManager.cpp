#include "TradingPostWatchManager.h"

#include "TradingPostPriceManager.h"
#include "TradingPostHistoryManager.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr uint32_t
        AURENES_BITE_ITEM_ID =
        96356;

    constexpr int64_t
        WATCH_INTERVAL_SECONDS =
        60;

    struct TargetAlertState
    {
        //
        // True once this target has been reached. It stays true
        // until a later observed sell price rises back ABOVE the
        // target, which re-arms the alert.
        //
        bool reachedLatch = false;

        //
        // Prevents evaluating the same successful price response
        // repeatedly on every render frame.
        //
        uint64_t lastProcessedPriceTimestamp = 0;
    };

    std::mutex g_WatchMutex;

    std::vector<
        TradingPostWatchItem
    > g_WatchedItems;

    std::unordered_map<
        uint32_t,
        TargetAlertState
    > g_TargetAlertStates;

    TradingPostTargetAlert
        g_ActiveTargetAlert;

    bool g_HasActiveTargetAlert = false;

    std::vector<
        TradingPostTargetAlert
    > g_PendingTargetAlerts;

    bool g_AutoWatchEnabled = true;

    bool g_HasLastCheck = false;

    std::chrono::steady_clock::
        time_point g_LastCheckTime;

    std::filesystem::path
        g_SettingsPath;

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

        g_TargetAlertStates[
            AURENES_BITE_ITEM_ID
        ];
    }

    void PromoteNextTargetAlertLocked()
    {
        if (g_HasActiveTargetAlert)
        {
            return;
        }

        if (g_PendingTargetAlerts.empty())
        {
            g_ActiveTargetAlert =
                TradingPostTargetAlert{};

            return;
        }

        g_ActiveTargetAlert =
            g_PendingTargetAlerts.front();

        g_PendingTargetAlerts.erase(
            g_PendingTargetAlerts.begin()
        );

        g_HasActiveTargetAlert =
            true;
    }

    void QueueTargetAlertLocked(
        const TradingPostTargetAlert& alert
    )
    {
        if (alert.itemID == 0)
        {
            return;
        }

        if (
            g_HasActiveTargetAlert &&
            g_ActiveTargetAlert.itemID ==
            alert.itemID
            )
        {
            return;
        }

        const auto queuedIt =
            std::find_if(
                g_PendingTargetAlerts.begin(),
                g_PendingTargetAlerts.end(),
                [&alert](
                    const TradingPostTargetAlert& queuedAlert
                    )
                {
                    return
                        queuedAlert.itemID ==
                        alert.itemID;
                }
            );

        if (
            queuedIt !=
            g_PendingTargetAlerts.end()
            )
        {
            return;
        }

        if (!g_HasActiveTargetAlert)
        {
            g_ActiveTargetAlert =
                alert;

            g_HasActiveTargetAlert =
                true;

            return;
        }

        g_PendingTargetAlerts.push_back(
            alert
        );
    }

    void RemoveQueuedTargetAlertLocked(
        uint32_t itemID
    )
    {
        g_PendingTargetAlerts.erase(
            std::remove_if(
                g_PendingTargetAlerts.begin(),
                g_PendingTargetAlerts.end(),
                [itemID](
                    const TradingPostTargetAlert& alert
                    )
                {
                    return
                        alert.itemID ==
                        itemID;
                }
            ),
            g_PendingTargetAlerts.end()
        );
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

            const auto alertStateIt =
                g_TargetAlertStates.find(
                    item.itemID
                );

            const bool reachedLatch =
                alertStateIt !=
                g_TargetAlertStates.end()
                ? alertStateIt->
                second.reachedLatch
                : false;

            file
                << "alertLatch="
                << item.itemID
                << '|'
                << (
                    reachedLatch
                    ? 1
                    : 0
                    )
                << '\n';
        }
    }

    void LoadLocked()
    {
        g_WatchedItems.clear();
        g_TargetAlertStates.clear();

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
                        "alertLatch=",
                        0
                    ) == 0
                    )
                {
                    const std::string payload =
                        line.substr(
                            11
                        );

                    const size_t separator =
                        payload.find(
                            '|'
                        );

                    if (
                        separator ==
                        std::string::npos
                        )
                    {
                        continue;
                    }

                    try
                    {
                        const uint32_t itemID =
                            static_cast<uint32_t>(
                                std::stoul(
                                    payload.substr(
                                        0,
                                        separator
                                    )
                                )
                                );

                        const std::string value =
                            payload.substr(
                                separator + 1
                            );

                        if (itemID != 0)
                        {
                            g_TargetAlertStates[
                                itemID
                            ].reachedLatch =
                                value == "1" ||
                                    value == "true";
                        }
                    }
                    catch (...)
                    {
                        // Ignore malformed saved rows.
                    }

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

                        g_TargetAlertStates[
                            item.itemID
                        ];
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

    void EvaluateTargetAlertsLocked()
    {
        bool settingsChanged = false;

        for (
            const TradingPostWatchItem&
            item :
            g_WatchedItems
            )
        {
            TradingPostPrice price;

            if (
                !TradingPostPriceManager::
                TryGetPrice(
                    item.itemID,
                    price
                ) ||
                !price.available ||
                price.lastUpdatedUnixSeconds == 0
                )
            {
                continue;
            }

            TargetAlertState& alertState =
                g_TargetAlertStates[
                    item.itemID
                ];

            if (
                price.lastUpdatedUnixSeconds <=
                alertState.
                lastProcessedPriceTimestamp
                )
            {
                continue;
            }

            alertState.
                lastProcessedPriceTimestamp =
                price.lastUpdatedUnixSeconds;

            //
            // A zero target means target alerting is disabled.
            //
            if (
                item.targetSellCopper == 0 ||
                price.sellUnitPrice == 0
                )
            {
                if (alertState.reachedLatch)
                {
                    alertState.reachedLatch =
                        false;

                    settingsChanged = true;
                }

                continue;
            }

            const bool targetReached =
                static_cast<uint64_t>(
                    price.sellUnitPrice
                    ) <=
                item.targetSellCopper;

            if (targetReached)
            {
                //
                // Only fire on the transition into the target range.
                // Once latched, repeated 60-second refreshes are quiet.
                //
                if (!alertState.reachedLatch)
                {
                    alertState.reachedLatch =
                        true;

                    settingsChanged = true;

                    //
                    // Queue every newly-triggered target hit. Only one alert
                    // is visible at a time; dismissing it promotes the next.
                    //
                    TradingPostTargetAlert targetAlert;

                    targetAlert.itemID =
                        item.itemID;

                    targetAlert.name =
                        item.name;

                    targetAlert.sellUnitPrice =
                        price.sellUnitPrice;

                    targetAlert.targetSellCopper =
                        item.targetSellCopper;

                    targetAlert.triggeredUnixSeconds =
                        GetCurrentUnixSeconds();

                    QueueTargetAlertLocked(
                        targetAlert
                    );
                }
            }
            else
            {
                //
                // Price moved back above target. Re-arm so a later
                // drop back into range can create a new alert.
                //
                if (alertState.reachedLatch)
                {
                    alertState.reachedLatch =
                        false;

                    settingsChanged = true;
                }
            }
        }

        if (settingsChanged)
        {
            SaveLocked();
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

    g_HasActiveTargetAlert =
        false;

    g_ActiveTargetAlert =
        TradingPostTargetAlert{};

    g_PendingTargetAlerts.clear();

    LoadLocked();

    for (
        const TradingPostWatchItem& item :
        g_WatchedItems
        )
    {
        TradingPostHistoryManager::
            RegisterWatchedItem(
                item.itemID
            );
    }

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

    g_HasActiveTargetAlert =
        false;

    g_ActiveTargetAlert =
        TradingPostTargetAlert{};

    g_PendingTargetAlerts.clear();

    g_TargetAlertStates.clear();

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

    //
    // This runs every render update so completed asynchronous price
    // requests are evaluated as soon as their new timestamp appears.
    //
    EvaluateTargetAlertsLocked();

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

    g_TargetAlertStates[
        itemID
    ] = TargetAlertState{};

        SaveLocked();

        TradingPostHistoryManager::
            RegisterWatchedItem(
                itemID
            );

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

    g_TargetAlertStates.erase(
        itemID
    );

    RemoveQueuedTargetAlertLocked(
        itemID
    );

    if (
        g_HasActiveTargetAlert &&
        g_ActiveTargetAlert.itemID ==
        itemID
        )
    {
        g_HasActiveTargetAlert =
            false;

        g_ActiveTargetAlert =
            TradingPostTargetAlert{};

        PromoteNextTargetAlertLocked();
    }

    TradingPostHistoryManager::
        UnregisterWatchedItem(
            itemID
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
            if (
                item.targetSellCopper !=
                targetCopper
                )
            {
                item.targetSellCopper =
                    targetCopper;

                //
                // A changed target is a new alert condition, but the
                // target edit itself must NOT fire an alert from an
                // already-cached price. Mark the current cached price
                // timestamp as processed so only a NEW API observation
                // can evaluate the new target.
                //
                TargetAlertState& alertState =
                    g_TargetAlertStates[
                        itemID
                    ];

                alertState.reachedLatch =
                    false;

                TradingPostPrice currentPrice;

                if (
                    TradingPostPriceManager::
                    TryGetPrice(
                        itemID,
                        currentPrice
                    ) &&
                    currentPrice.available
                    )
                {
                    alertState.
                        lastProcessedPriceTimestamp =
                        currentPrice.
                        lastUpdatedUnixSeconds;
                }
                else
                {
                    alertState.
                        lastProcessedPriceTimestamp =
                        0;
                }

                RemoveQueuedTargetAlertLocked(
                    itemID
                );

                if (
                    g_HasActiveTargetAlert &&
                    g_ActiveTargetAlert.itemID ==
                    itemID
                    )
                {
                    g_HasActiveTargetAlert =
                        false;

                    g_ActiveTargetAlert =
                        TradingPostTargetAlert{};

                    PromoteNextTargetAlertLocked();
                }

                SaveLocked();
            }

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

bool TradingPostWatchManager::
TryGetActiveTargetAlert(
    TradingPostTargetAlert& outAlert
)
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    if (!g_HasActiveTargetAlert)
    {
        return false;
    }

    outAlert =
        g_ActiveTargetAlert;

    return true;
}

void TradingPostWatchManager::
DismissActiveTargetAlert()
{
    std::lock_guard<std::mutex> lock(
        g_WatchMutex
    );

    g_HasActiveTargetAlert =
        false;

    g_ActiveTargetAlert =
        TradingPostTargetAlert{};

    PromoteNextTargetAlertLocked();
}
