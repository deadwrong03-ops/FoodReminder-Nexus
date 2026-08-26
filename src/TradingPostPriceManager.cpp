#include "TradingPostPriceManager.h"

#include <Windows.h>
#include <winhttp.h>

#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>
#include <condition_variable>
#include <queue>
#include <thread>
#include <unordered_set>

#pragma comment(lib, "winhttp.lib")

namespace
{
    std::mutex g_TradingPostPriceMutex;

    std::unordered_map<
        uint32_t,
        TradingPostPrice
    > g_TradingPostPrices;

    std::mutex g_RequestMutex;
    std::condition_variable g_RequestCondition;

    std::queue<uint32_t> g_PriceRequests;

    std::unordered_set<uint32_t>
        g_QueuedItemIDs;

    std::thread g_WorkerThread;

    bool g_WorkerRunning = false;
    bool g_StopWorker = false;

    void PriceWorker()
    {
        while (true)
        {
            uint32_t itemID = 0;

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
                            !g_PriceRequests.empty();
                    }
                );

                if (
                    g_StopWorker &&
                    g_PriceRequests.empty()
                    )
                {
                    break;
                }

                itemID =
                    g_PriceRequests.front();

                g_PriceRequests.pop();
            }

            TradingPostPriceManager::FetchPrice(
                itemID
            );

            {
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
    std::lock_guard<std::mutex> lock(
        g_TradingPostPriceMutex
    );

    g_TradingPostPrices.clear();
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

        g_QueuedItemIDs.clear();
    }
}

void TradingPostPriceManager::RequestPrice(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return;
    }

    TradingPostPrice existingPrice;

    if (
        TradingPostPriceManager::TryGetPrice(
            itemID,
            existingPrice
        )
        )
    {
        return;
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

    price.available =
        true;

    std::lock_guard<std::mutex> lock(
        g_TradingPostPriceMutex
    );

    g_TradingPostPrices[itemID] =
        price;
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

    const std::wstring path =
        L"/v2/commerce/prices/" +
        std::to_wstring(itemID);

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

    if (!sent ||
        !WinHttpReceiveResponse(
            request,
            nullptr
        ))
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    std::string response;

    while (true)
    {
        DWORD available = 0;

        if (!WinHttpQueryDataAvailable(
            request,
            &available
        ))
        {
            break;
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

        if (!WinHttpReadData(
            request,
            buffer.data(),
            available,
            &bytesRead
        ))
        {
            break;
        }

        response.append(
            buffer.data(),
            bytesRead
        );
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

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

    return true;
}