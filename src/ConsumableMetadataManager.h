#pragma once

#include <cstdint>

struct ConsumableMetadata
{
    uint32_t itemID = 0;

    // Normal consumable duration from the GW2 item API.
    uint32_t durationMilliseconds = 0;

    bool available = false;
};

namespace ConsumableMetadataManager
{
    void Start();

    void Shutdown();

    void Reset();

    void RequestMetadata(
        uint32_t itemID
    );

    void StoreMetadata(
        uint32_t itemID,
        uint32_t durationMilliseconds
    );

    bool TryGetMetadata(
        uint32_t itemID,
        ConsumableMetadata& outMetadata
    );

    bool FetchMetadata(
        uint32_t itemID
    );
}