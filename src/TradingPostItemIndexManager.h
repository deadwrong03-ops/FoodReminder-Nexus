#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct TradingPostIndexedItem
{
    uint32_t itemID = 0;
    std::string name;
};

namespace TradingPostItemIndexManager
{
    //
    // Loads the cached searchable Trading Post item index immediately,
    // then refreshes it in the background when the cache is missing or stale.
    //
    void Start(
        void* moduleHandle
    );

    void Shutdown();

    //
    // Clears the in-memory index only.
    // The on-disk cache is left intact.
    //
    void Reset();

    //
    // Requests a fresh Trading Post item-index rebuild.
    // The current cached index remains searchable while the rebuild runs.
    //
    void RequestRefresh();

    //
    // Searches the local in-memory index.
    //
    // Matching is case-insensitive and favors:
    // exact match -> starts-with -> word-prefix -> contains.
    //
    std::vector<TradingPostIndexedItem> Search(
        const std::string& query,
        size_t maxResults = 8
    );

    bool IsReady();

    bool IsBuilding();

    size_t GetItemCount();

    size_t GetBuildProcessedCount();

    size_t GetBuildTotalCount();

    std::string GetLastError();
}
