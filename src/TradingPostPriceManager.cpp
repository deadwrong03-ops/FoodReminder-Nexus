#include "TradingPostPriceManager.h"

#include "TradingPostHistoryManager.h"

#include <Windows.h>
#include <winhttp.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace
{
    std::mutex g_TradingPostPriceMutex;

    std::unordered_map<
        uint32_t,
        TradingPostPrice
    > g_TradingPostPrices;

    std::mutex g_ItemLookupMutex;

    std::unordered_map<
        uint32_t,
        TradingPostItemLookup
    > g_ItemLookups;

    std::mutex g_RequestMutex;
    std::condition_variable g_RequestCondition;

    std::queue<uint32_t> g_PriceRequests;
    std::queue<uint32_t> g_ItemLookupRequests;

    std::unordered_set<uint32_t>
        g_QueuedItemIDs;

    std::unordered_set<uint32_t>
        g_QueuedLookupItemIDs;

    std::thread g_WorkerThread;

    bool g_WorkerRunning = false;
    bool g_StopWorker = false;

    bool HttpGet(
        const std::wstring& path,
        std::string& response
    )
    {
        response.clear();

        HINTERNET session =
            WinHttpOpen(
                L"FoodReminder-Nexus/1.0",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS,
                0
            );

        if (session == nullptr)
        {
            return false;
        }

        HINTERNET connection =
            WinHttpConnect(
                session,
                L"api.guildwars2.com",
                INTERNET_DEFAULT_HTTPS_PORT,
                0
            );

        if (connection == nullptr)
        {
            WinHttpCloseHandle(session);
            return false;
        }

        HINTERNET request =
            WinHttpOpenRequest(
                connection,
                L"GET",
                path.c_str(),
                nullptr,
                WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                WINHTTP_FLAG_SECURE
            );

        if (request == nullptr)
        {
            WinHttpCloseHandle(connection);
            WinHttpCloseHandle(session);
            return false;
        }

        const BOOL sent =
            WinHttpSendRequest(
                request,
                WINHTTP_NO_ADDITIONAL_HEADERS,
                0,
                WINHTTP_NO_REQUEST_DATA,
                0,
                0,
                0
            );

        if (
            !sent ||
            !WinHttpReceiveResponse(
                request,
                nullptr
            )
            )
        {
            WinHttpCloseHandle(request);
            WinHttpCloseHandle(connection);
            WinHttpCloseHandle(session);
            return false;
        }

        DWORD statusCode = 0;
        DWORD statusCodeSize =
            sizeof(statusCode);

        const BOOL statusRead =
            WinHttpQueryHeaders(
                request,
                WINHTTP_QUERY_STATUS_CODE |
                WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,
                &statusCode,
                &statusCodeSize,
                WINHTTP_NO_HEADER_INDEX
            );

        if (
            !statusRead ||
            statusCode != 200
            )
        {
            WinHttpCloseHandle(request);
            WinHttpCloseHandle(connection);
            WinHttpCloseHandle(session);
            return false;
        }

        while (true)
        {
            DWORD available = 0;

            if (
                !WinHttpQueryDataAvailable(
                    request,
                    &available
                )
                )
            {
                WinHttpCloseHandle(request);
                WinHttpCloseHandle(connection);
                WinHttpCloseHandle(session);
                return false;
            }

            if (available == 0)
            {
                break;
            }

            std::vector<char> buffer(
                available + 1,
                '\0'
            );

            DWORD bytesRead = 0;

            if (
                !WinHttpReadData(
                    request,
                    buffer.data(),
                    available,
                    &bytesRead
                )
                )
            {
                WinHttpCloseHandle(request);
                WinHttpCloseHandle(connection);
                WinHttpCloseHandle(session);
                return false;
            }

            response.append(
                buffer.data(),
                bytesRead
            );
        }

        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        return true;
    }

    bool ExtractJsonString(
        const std::string& json,
        const std::string& key,
        std::string& outValue
    )
    {
        outValue.clear();

        const std::string searchKey =
            "\"" + key + "\"";

        const size_t keyPosition =
            json.find(
                searchKey
            );

        if (
            keyPosition ==
            std::string::npos
            )
        {
            return false;
        }

        const size_t colonPosition =
            json.find(
                ':',
                keyPosition +
                searchKey.size()
            );

        if (
            colonPosition ==
            std::string::npos
            )
        {
            return false;
        }

        const size_t quotePosition =
            json.find(
                '"',
                colonPosition + 1
            );

        if (
            quotePosition ==
            std::string::npos
            )
        {
            return false;
        }

        bool escaped = false;

        for (
            size_t i =
            quotePosition + 1;
            i < json.size();
            ++i
            )
        {
            const char character =
                json[i];

            if (escaped)
            {
                switch (character)
                {
                case '"':
                    outValue += '"';
                    break;

                case '\\':
                    outValue += '\\';
                    break;

                case '/':
                    outValue += '/';
                    break;

                case 'n':
                    outValue += '\n';
                    break;

                case 'r':
                    outValue += '\r';
                    break;

                case 't':
                    outValue += '\t';
                    break;

                default:
                    outValue += character;
                    break;
                }

                escaped = false;
                continue;
            }

            if (character == '\\')
            {
                escaped = true;
                continue;
            }

            if (character == '"')
            {
                return true;
            }

            outValue += character;
        }

        return false;
    }

    void StoreItemLookup(
        const TradingPostItemLookup& lookup
    )
    {
        std::lock_guard<std::mutex> lock(
            g_ItemLookupMutex
        );

        g_ItemLookups[
            lookup.itemID
        ] =
            lookup;
    }

    void PriceWorker()
    {
        while (true)
        {
            uint32_t itemID = 0;
            bool isLookupRequest = false;

            {
                std::unique_lock<std::mutex> lock(
                    g_RequestMutex
                );

                g_RequestCondition.wait(
                    lock,
                    []()
                    {
                        return
                            g_StopWorker ||
                            !g_ItemLookupRequests.empty() ||
                            !g_PriceRequests.empty();
                    }
                );

                if (
                    g_StopWorker &&
                    g_ItemLookupRequests.empty() &&
                    g_PriceRequests.empty()
                    )
                {
                    break;
                }

                if (
                    !g_ItemLookupRequests.empty()
                    )
                {
                    itemID =
                        g_ItemLookupRequests.front();

                    g_ItemLookupRequests.pop();

                    isLookupRequest =
                        true;
                }
                else
                {
                    itemID =
                        g_PriceRequests.front();

                    g_PriceRequests.pop();
                }
            }

            if (isLookupRequest)
            {
                TradingPostPriceManager::
                    FetchItemLookup(
                        itemID
                    );

                std::lock_guard<std::mutex> lock(
                    g_RequestMutex
                );

                g_QueuedLookupItemIDs.erase(
                    itemID
                );
            }
            else
            {
                TradingPostPriceManager::
                    FetchPrice(
                        itemID
                    );

                std::lock_guard<std::mutex> lock(
                    g_RequestMutex
                );

                g_QueuedItemIDs.erase(
                    itemID
                );
            }
        }
    }
}

void TradingPostPriceManager::Reset()
{
    {
        std::lock_guard<std::mutex> lock(
            g_TradingPostPriceMutex
        );

        g_TradingPostPrices.clear();
    }

    {
        std::lock_guard<std::mutex> lock(
            g_ItemLookupMutex
        );

        g_ItemLookups.clear();
    }
}

void TradingPostPriceManager::Start()
{
    std::lock_guard<std::mutex> lock(
        g_RequestMutex
    );

    if (g_WorkerRunning)
    {
        return;
    }

    g_StopWorker = false;
    g_WorkerRunning = true;

    g_WorkerThread =
        std::thread(
            PriceWorker
        );
}

void TradingPostPriceManager::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        if (!g_WorkerRunning)
        {
            return;
        }

        g_StopWorker = true;
    }

    g_RequestCondition.notify_all();

    if (g_WorkerThread.joinable())
    {
        g_WorkerThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        g_WorkerRunning = false;

        while (!g_PriceRequests.empty())
        {
            g_PriceRequests.pop();
        }

        while (!g_ItemLookupRequests.empty())
        {
            g_ItemLookupRequests.pop();
        }

        g_QueuedItemIDs.clear();
        g_QueuedLookupItemIDs.clear();
    }
}

void TradingPostPriceManager::RequestPrice(
    uint32_t itemID,
    bool forceRefresh
)
{
    if (itemID == 0)
    {
        return;
    }

    if (!forceRefresh)
    {
        TradingPostPrice existingPrice;

        if (
            TradingPostPriceManager::
            TryGetPrice(
                itemID,
                existingPrice
            )
            )
        {
            return;
        }
    }

    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        if (
            g_QueuedItemIDs.find(
                itemID
            ) !=
            g_QueuedItemIDs.end()
            )
        {
            return;
        }

        g_QueuedItemIDs.insert(
            itemID
        );

        g_PriceRequests.push(
            itemID
        );
    }

    g_RequestCondition.notify_one();
}

void TradingPostPriceManager::RequestItemLookup(
    uint32_t itemID,
    bool forceRefresh
)
{
    if (itemID == 0)
    {
        return;
    }

    if (!forceRefresh)
    {
        TradingPostItemLookup existingLookup;

        if (
            TryGetItemLookup(
                itemID,
                existingLookup
            ) &&
            existingLookup.complete
            )
        {
            return;
        }
    }

    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        if (
            g_QueuedLookupItemIDs.find(
                itemID
            ) !=
            g_QueuedLookupItemIDs.end()
            )
        {
            return;
        }

        g_QueuedLookupItemIDs.insert(
            itemID
        );

        g_ItemLookupRequests.push(
            itemID
        );
    }

    g_RequestCondition.notify_one();
}

bool TradingPostPriceManager::TryGetItemLookup(
    uint32_t itemID,
    TradingPostItemLookup& outLookup
)
{
    if (itemID == 0)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(
        g_ItemLookupMutex
    );

    const auto it =
        g_ItemLookups.find(
            itemID
        );

    if (
        it ==
        g_ItemLookups.end()
        )
    {
        return false;
    }

    outLookup =
        it->second;

    return true;
}

void TradingPostPriceManager::StorePrice(
    uint32_t itemID,
    uint32_t buyUnitPrice,
    uint32_t sellUnitPrice
)
{
    if (itemID == 0)
    {
        return;
    }

    TradingPostPrice price;

    price.itemID =
        itemID;

    price.buyUnitPrice =
        buyUnitPrice;

    price.sellUnitPrice =
        sellUnitPrice;

    price.lastUpdatedUnixSeconds =
        static_cast<uint64_t>(
            std::chrono::duration_cast<
            std::chrono::seconds
            >(
                std::chrono::system_clock::
                now().time_since_epoch()
            ).count()
            );

    price.available =
        true;

    {
        std::lock_guard<std::mutex> lock(
            g_TradingPostPriceMutex
        );

        g_TradingPostPrices[itemID] =
            price;
    }

    TradingPostHistoryManager::
        RecordObservation(
            itemID,
            buyUnitPrice,
            sellUnitPrice,
            price.lastUpdatedUnixSeconds
        );
}

bool TradingPostPriceManager::TryGetPrice(
    uint32_t itemID,
    TradingPostPrice& outPrice
)
{
    if (itemID == 0)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(
        g_TradingPostPriceMutex
    );

    const auto it =
        g_TradingPostPrices.find(
            itemID
        );

    if (
        it ==
        g_TradingPostPrices.end()
        )
    {
        return false;
    }

    outPrice =
        it->second;

    return true;
}

bool TradingPostPriceManager::FetchPrice(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return false;
    }

    std::string response;

    const std::wstring path =
        L"/v2/commerce/prices/" +
        std::to_wstring(
            itemID
        );

    if (
        !HttpGet(
            path,
            response
        )
        )
    {
        return false;
    }

    const size_t buyPos =
        response.find(
            "\"buys\""
        );

    const size_t sellPos =
        response.find(
            "\"sells\""
        );

    if (
        buyPos == std::string::npos ||
        sellPos == std::string::npos
        )
    {
        return false;
    }

    const std::string priceKey =
        "\"unit_price\"";

    const size_t buyPricePos =
        response.find(
            priceKey,
            buyPos
        );

    const size_t sellPricePos =
        response.find(
            priceKey,
            sellPos
        );

    if (
        buyPricePos == std::string::npos ||
        sellPricePos == std::string::npos
        )
    {
        return false;
    }

    const size_t buyColon =
        response.find(
            ':',
            buyPricePos
        );

    const size_t sellColon =
        response.find(
            ':',
            sellPricePos
        );

    if (
        buyColon == std::string::npos ||
        sellColon == std::string::npos
        )
    {
        return false;
    }

    try
    {
        const uint32_t buyPrice =
            static_cast<uint32_t>(
                std::stoul(
                    response.substr(
                        buyColon + 1
                    )
                )
                );

        const uint32_t sellPrice =
            static_cast<uint32_t>(
                std::stoul(
                    response.substr(
                        sellColon + 1
                    )
                )
                );

        StorePrice(
            itemID,
            buyPrice,
            sellPrice
        );
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool TradingPostPriceManager::FetchItemLookup(
    uint32_t itemID
)
{
    TradingPostItemLookup lookup;

    lookup.itemID =
        itemID;

    lookup.complete =
        true;

    if (itemID == 0)
    {
        StoreItemLookup(
            lookup
        );

        return false;
    }

    std::string response;

    const std::wstring itemPath =
        L"/v2/items/" +
        std::to_wstring(
            itemID
        );

    if (
        !HttpGet(
            itemPath,
            response
        )
        )
    {
        StoreItemLookup(
            lookup
        );

        return false;
    }

    std::string officialName;

    if (
        !ExtractJsonString(
            response,
            "name",
            officialName
        ) ||
        officialName.empty()
        )
    {
        StoreItemLookup(
            lookup
        );

        return false;
    }

    lookup.name =
        officialName;

    lookup.validItem =
        true;

    //
    // This second request confirms that the valid GW2 item
    // actually exists on the Trading Post.
    //
    lookup.availableOnTradingPost =
        FetchPrice(
            itemID
        );

    StoreItemLookup(
        lookup
    );

    return
        lookup.validItem &&
        lookup.availableOnTradingPost;
}