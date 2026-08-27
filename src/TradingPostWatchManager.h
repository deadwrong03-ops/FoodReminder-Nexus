#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct TradingPostWatchItem
{
    uint32_t itemID = 0;
    std::string name;

    uint64_t targetSellCopper = 0;

    bool isDefault = false;
};

struct TradingPostTargetAlert
{
    uint32_t itemID = 0;

    std::string name;

    uint64_t sellUnitPrice = 0;
    uint64_t targetSellCopper = 0;

    uint64_t triggeredUnixSeconds = 0;
};

namespace TradingPostWatchManager
{
    void Start(
        void* moduleHandle
    );

    void Shutdown();

    void Update();

    std::vector<
        TradingPostWatchItem
    > GetItems();

    bool AddItem(
        uint32_t itemID,
        const std::string& name
    );

    void RemoveItem(
        uint32_t itemID
    );

    void SetTargetSellPrice(
        uint32_t itemID,
        uint64_t targetCopper
    );

    bool IsAutoWatchEnabled();

    void SetAutoWatchEnabled(
        bool enabled
    );

    void RefreshItem(
        uint32_t itemID
    );

    void RefreshAll();

    int64_t GetSecondsUntilNextCheck();

    //
    // Returns the current session alert, if one is waiting
    // for the user to dismiss it.
    //
    bool TryGetActiveTargetAlert(
        TradingPostTargetAlert& outAlert
    );

    //
    // Dismisses the currently visible alert. This does NOT
    // re-arm the target. The target automatically re-arms only
    // after a later observed sell price rises above the target.
    //
    void DismissActiveTargetAlert();
}
