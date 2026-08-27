#pragma once

#include <cstdint>
#include <vector>

struct TradingPostHistoryPoint
{
    uint64_t timestampUnixSeconds = 0;
    uint32_t buyUnitPrice = 0;
    uint32_t sellUnitPrice = 0;
};

namespace TradingPostHistoryManager
{
    //
    // Loads persistent Trading Post history from disk.
    //
    void Start(
        void* moduleHandle
    );

    //
    // Stops history recording and clears in-memory state.
    // Persistent history on disk is preserved.
    //
    void Shutdown();

    //
    // Marks an item as currently watched.
    // Only watched items are allowed to record new history.
    //
    void RegisterWatchedItem(
        uint32_t itemID
    );

    //
    // Stops future history recording for an item.
    // Existing historical observations are preserved.
    //
    void UnregisterWatchedItem(
        uint32_t itemID
    );

    //
    // Records one successful Trading Post API observation.
    // The call is ignored when the item is not currently watched.
    //
    void RecordObservation(
        uint32_t itemID,
        uint32_t buyUnitPrice,
        uint32_t sellUnitPrice,
        uint64_t timestampUnixSeconds
    );

    //
    // Returns all locally stored observations for one item,
    // oldest to newest.
    //
    std::vector<TradingPostHistoryPoint>
        GetHistory(
            uint32_t itemID
        );

    size_t GetObservationCount(
        uint32_t itemID
    );
}
