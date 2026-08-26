#pragma once

#include <cstdint>


struct TradingPostPrice
{
    uint32_t itemID = 0;

    // Highest current Trading Post buy order,
    // measured in copper.
    uint32_t buyUnitPrice = 0;

    // Lowest current Trading Post sell listing,
    // measured in copper.
    uint32_t sellUnitPrice = 0;

    bool available = false;
};

namespace TradingPostPriceManager
{
    void Start();

    void Shutdown();

    void RequestPrice(
        uint32_t itemID
    );
    //
    // Clears all cached Trading Post prices.
    //
    void Reset();

    //
    // Stores or replaces a cached Trading Post price.
    //
    // This will eventually be called by the API/network layer.
    //
    void StorePrice(
        uint32_t itemID,
        uint32_t buyUnitPrice,
        uint32_t sellUnitPrice
    );

    //
    // Attempts to retrieve a cached Trading Post price.
    //
    // Returns true when a cached price exists.
    //
    bool TryGetPrice(
        uint32_t itemID,
        TradingPostPrice& outPrice
    );
    bool FetchPrice(
        uint32_t itemID
    );
   
}