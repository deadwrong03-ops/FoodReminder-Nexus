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
}
