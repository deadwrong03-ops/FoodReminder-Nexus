#include "ConsumableMetadataManager.h"

#include <Windows.h>
#include <winhttp.h>

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
    std::mutex g_MetadataMutex;

    std::unordered_map<
        uint32_t,
        ConsumableMetadata
    > g_Metadata;

    std::mutex g_RequestMutex;

    std::condition_variable
        g_RequestCondition;

    std::queue<uint32_t>
        g_MetadataRequests;

    std::unordered_set<uint32_t>
        g_QueuedItemIDs;

    std::thread g_WorkerThread;

    bool g_WorkerRunning = false;
    bool g_StopWorker = false;

    void MetadataWorker()
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
                            !g_MetadataRequests.empty();
                    }
                );

                if (
                    g_StopWorker &&
                    g_MetadataRequests.empty()
                    )
                {
                    break;
                }

                itemID =
                    g_MetadataRequests.front();

                g_MetadataRequests.pop();
            }

            ConsumableMetadataManager::
                FetchMetadata(
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

void ConsumableMetadataManager::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_MetadataMutex
    );

    g_Metadata.clear();
}

void ConsumableMetadataManager::Start()
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
            MetadataWorker
        );
}

void ConsumableMetadataManager::Shutdown()
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

        while (!g_MetadataRequests.empty())
        {
            g_MetadataRequests.pop();
        }

        g_QueuedItemIDs.clear();
    }
}

void ConsumableMetadataManager::RequestMetadata(
    uint32_t itemID
)
{
    if (itemID == 0)
    {
        return;
    }

    ConsumableMetadata existingMetadata;

    if (
        ConsumableMetadataManager::
        TryGetMetadata(
            itemID,
            existingMetadata
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

        g_MetadataRequests.push(
            itemID
        );
    }

    g_RequestCondition.notify_one();
}

void ConsumableMetadataManager::StoreMetadata(
    uint32_t itemID,
    uint32_t durationMilliseconds
)
{
    if (
        itemID == 0 ||
        durationMilliseconds == 0
        )
    {
        return;
    }

    ConsumableMetadata metadata;

    metadata.itemID =
        itemID;

    metadata.durationMilliseconds =
        durationMilliseconds;

    metadata.available =
        true;

    std::lock_guard<std::mutex> lock(
        g_MetadataMutex
    );

    g_Metadata[itemID] =
        metadata;
}

bool ConsumableMetadataManager::TryGetMetadata(
    uint32_t itemID,
    ConsumableMetadata& outMetadata
)
{
    if (itemID == 0)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(
        g_MetadataMutex
    );

    const auto it =
        g_Metadata.find(
            itemID
        );

    if (
        it ==
        g_Metadata.end()
        )
    {
        return false;
    }

    outMetadata =
        it->second;

    return true;
}

bool ConsumableMetadataManager::FetchMetadata(
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
        WinHttpCloseHandle(
            session
        );

        return false;
    }

    const std::wstring path =
        L"/v2/items/" +
        std::to_wstring(
            itemID
        );

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
        WinHttpCloseHandle(
            connection
        );

        WinHttpCloseHandle(
            session
        );

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
        WinHttpCloseHandle(
            request
        );

        WinHttpCloseHandle(
            connection
        );

        WinHttpCloseHandle(
            session
        );

        return false;
    }

    std::string response;

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

        if (
            !WinHttpReadData(
                request,
                buffer.data(),
                available,
                &bytesRead
            )
            )
        {
            break;
        }

        response.append(
            buffer.data(),
            bytesRead
        );
    }

    WinHttpCloseHandle(
        request
    );

    WinHttpCloseHandle(
        connection
    );

    WinHttpCloseHandle(
        session
    );

    const std::string durationKey =
        "\"duration_ms\"";

    const size_t durationPos =
        response.find(
            durationKey
        );

    if (
        durationPos ==
        std::string::npos
        )
    {
        return false;
    }

    const size_t colonPos =
        response.find(
            ':',
            durationPos
        );

    if (
        colonPos ==
        std::string::npos
        )
    {
        return false;
    }

    try
    {
        const uint32_t durationMilliseconds =
            static_cast<uint32_t>(
                std::stoul(
                    response.substr(
                        colonPos + 1
                    )
                )
                );

        if (durationMilliseconds == 0)
        {
            return false;
        }

        StoreMetadata(
            itemID,
            durationMilliseconds
        );
    }
    catch (...)
    {
        return false;
    }

    return true;
}