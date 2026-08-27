#pragma once

#include <cstdint>
#include <string>

struct TradingPostPrice
{
    uint32_t itemID = 0;

    // Highest current Trading Post buy order,
    // measured in copper.
    uint32_t buyUnitPrice = 0;

    // Lowest current Trading Post sell listing,
    // measured in copper.
    uint32_t sellUnitPrice = 0;

    // Unix timestamp for the most recent successful API update.
    uint64_t lastUpdatedUnixSeconds = 0;

    bool available = false;
};

struct TradingPostItemLookup
{
    uint32_t itemID = 0;

    // Official item name returned by ArenaNet.
    std::string name;

    // True once the asynchronous lookup has finished.
    bool complete = false;

    // True when /v2/items/<id> confirmed the item exists.
    bool validItem = false;

    // True when the commerce API confirmed that the item
    // can be looked up on the Trading Post.
    bool availableOnTradingPost = false;
};

namespace TradingPostPriceManager
{
    void Start();

    void Shutdown();

    //
    // Queues an asynchronous price request.
    //
    // When forceRefresh is false, an existing cached price is reused.
    // When true, a fresh API request is queued even if a cache entry exists.
    //
    void RequestPrice(
        uint32_t itemID,
        bool forceRefresh = false
    );

    //
    // Queues an asynchronous item identity/Trading Post validation request.
    //
    void RequestItemLookup(
        uint32_t itemID,
        bool forceRefresh = false
    );

    //
    // Attempts to retrieve the result of an item lookup.
    //
    bool TryGetItemLookup(
        uint32_t itemID,
        TradingPostItemLookup& outLookup
    );

    //
    // Clears all cached Trading Post prices and item lookups.
    //
    void Reset();

    //
    // Stores or replaces a cached Trading Post price.
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

    bool FetchItemLookup(
        uint32_t itemID
    );
}