#include "TradingPostItemIndexManager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <winhttp.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace
{
    constexpr uint64_t
        CACHE_MAX_AGE_SECONDS =
        24ULL * 60ULL * 60ULL;

    constexpr size_t
        ITEM_BATCH_SIZE =
        200;

    struct IndexedRecord
    {
        TradingPostIndexedItem item;
        std::string searchName;
    };

    struct SearchCandidate
    {
        TradingPostIndexedItem item;
        int rank = 0;
        size_t nameLength = 0;
        std::string normalizedName;
    };

    std::mutex g_IndexMutex;

    std::vector<IndexedRecord>
        g_Index;

    bool g_IndexReady = false;
    bool g_Building = false;

    size_t g_BuildProcessedCount = 0;
    size_t g_BuildTotalCount = 0;

    std::string g_LastError;

    uint64_t g_CacheUpdatedUnixSeconds = 0;

    std::filesystem::path
        g_CachePath;

    std::mutex g_RequestMutex;

    std::condition_variable
        g_RequestCondition;

    bool g_RefreshRequested = false;

    std::thread g_WorkerThread;

    bool g_WorkerRunning = false;

    std::atomic<bool>
        g_StopWorker = false;

    uint64_t GetUnixSeconds()
    {
        return
            static_cast<uint64_t>(
                std::chrono::duration_cast<
                std::chrono::seconds
                >(
                    std::chrono::system_clock::
                    now().time_since_epoch()
                ).count()
                );
    }

    std::filesystem::path BuildCachePath(
        void* moduleHandle
    )
    {
        wchar_t modulePath[
            MAX_PATH
        ] = {};

            const DWORD length =
                GetModuleFileNameW(
                    static_cast<HMODULE>(
                        moduleHandle
                        ),
                    modulePath,
                    MAX_PATH
                );

            if (length == 0)
            {
                return {};
            }

            const std::filesystem::path path(
                modulePath
            );

            return
                path.parent_path() /
                L"FoodReminder_TradingPostItemIndex.tsv";
    }

    std::string NormalizeSearchText(
        const std::string& value
    )
    {
        std::string normalized;
        normalized.reserve(
            value.size()
        );

        bool previousWasSpace = true;

        for (unsigned char byte : value)
        {
            if (
                byte == ' ' ||
                byte == '\t' ||
                byte == '\r' ||
                byte == '\n'
                )
            {
                if (
                    !previousWasSpace &&
                    !normalized.empty()
                    )
                {
                    normalized += ' ';
                }

                previousWasSpace = true;
                continue;
            }

            if (byte < 128)
            {
                normalized +=
                    static_cast<char>(
                        std::tolower(
                            byte
                        )
                        );
            }
            else
            {
                //
                // Preserve UTF-8 bytes. ASCII matching remains
                // case-insensitive while non-ASCII names remain intact.
                //
                normalized +=
                    static_cast<char>(
                        byte
                        );
            }

            previousWasSpace = false;
        }

        while (
            !normalized.empty() &&
            normalized.back() == ' '
            )
        {
            normalized.pop_back();
        }

        return normalized;
    }

    std::vector<std::string> SplitSearchTokens(
        const std::string& normalizedQuery
    )
    {
        std::vector<std::string> tokens;

        size_t start = 0;

        while (start < normalizedQuery.size())
        {
            while (
                start < normalizedQuery.size() &&
                normalizedQuery[start] == ' '
                )
            {
                ++start;
            }

            if (start >= normalizedQuery.size())
            {
                break;
            }

            size_t end =
                normalizedQuery.find(
                    ' ',
                    start
                );

            if (end == std::string::npos)
            {
                end =
                    normalizedQuery.size();
            }

            tokens.push_back(
                normalizedQuery.substr(
                    start,
                    end - start
                )
            );

            start =
                end + 1;
        }

        return tokens;
    }

    bool IsAsciiAlphaNumeric(
        char character
    )
    {
        const unsigned char value =
            static_cast<unsigned char>(
                character
                );

        return
            std::isalnum(
                value
            ) != 0;
    }

    bool HasWordPrefix(
        const std::string& normalizedName,
        const std::string& token
    )
    {
        size_t position = 0;

        while (true)
        {
            position =
                normalizedName.find(
                    token,
                    position
                );

            if (
                position ==
                std::string::npos
                )
            {
                return false;
            }

            if (
                position == 0 ||
                !IsAsciiAlphaNumeric(
                    normalizedName[
                        position - 1
                    ]
                )
                )
            {
                return true;
            }

            ++position;
        }
    }

    int GetSearchRank(
        const std::string& normalizedName,
        const std::string& normalizedQuery,
        const std::vector<std::string>& tokens
    )
    {
        if (
            normalizedName ==
            normalizedQuery
            )
        {
            return 0;
        }

        if (
            normalizedName.rfind(
                normalizedQuery,
                0
            ) == 0
            )
        {
            return 1;
        }

        for (const std::string& token : tokens)
        {
            if (
                normalizedName.find(
                    token
                ) ==
                std::string::npos
                )
            {
                return -1;
            }
        }

        bool allWordPrefixes = true;

        for (const std::string& token : tokens)
        {
            if (
                !HasWordPrefix(
                    normalizedName,
                    token
                )
                )
            {
                allWordPrefixes = false;
                break;
            }
        }

        return
            allWordPrefixes
            ? 2
            : 3;
    }

    void AppendUtf8CodePoint(
        std::string& output,
        uint32_t codePoint
    )
    {
        if (codePoint <= 0x7F)
        {
            output +=
                static_cast<char>(
                    codePoint
                    );
        }
        else if (codePoint <= 0x7FF)
        {
            output +=
                static_cast<char>(
                    0xC0 |
                    (
                        codePoint >> 6
                        )
                    );

            output +=
                static_cast<char>(
                    0x80 |
                    (
                        codePoint &
                        0x3F
                        )
                    );
        }
        else
        {
            output +=
                static_cast<char>(
                    0xE0 |
                    (
                        codePoint >> 12
                        )
                    );

            output +=
                static_cast<char>(
                    0x80 |
                    (
                        codePoint >> 6
                        ) &
                    0x3F
                    );

            output +=
                static_cast<char>(
                    0x80 |
                    (
                        codePoint &
                        0x3F
                        )
                    );
        }
    }

    int HexValue(
        char character
    )
    {
        if (
            character >= '0' &&
            character <= '9'
            )
        {
            return
                character - '0';
        }

        if (
            character >= 'a' &&
            character <= 'f'
            )
        {
            return
                character - 'a' + 10;
        }

        if (
            character >= 'A' &&
            character <= 'F'
            )
        {
            return
                character - 'A' + 10;
        }

        return -1;
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

        for (
            size_t i =
            quotePosition + 1;
            i < json.size();
            ++i
            )
        {
            const char character =
                json[i];

            if (character == '"')
            {
                return true;
            }

            if (character != '\\')
            {
                outValue += character;
                continue;
            }

            ++i;

            if (i >= json.size())
            {
                return false;
            }

            const char escaped =
                json[i];

            switch (escaped)
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

            case 'b':
                outValue += '\b';
                break;

            case 'f':
                outValue += '\f';
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

            case 'u':
            {
                if (
                    i + 4 >=
                    json.size()
                    )
                {
                    return false;
                }

                uint32_t codePoint = 0;

                for (int digit = 0;
                    digit < 4;
                    ++digit)
                {
                    const int value =
                        HexValue(
                            json[
                                i + 1 +
                                    static_cast<size_t>(
                                        digit
                                        )
                            ]
                        );

                    if (value < 0)
                    {
                        return false;
                    }

                    codePoint =
                        (codePoint << 4) |
                        static_cast<uint32_t>(
                            value
                            );
                }

                AppendUtf8CodePoint(
                    outValue,
                    codePoint
                );

                i += 4;
                break;
            }

            default:
                outValue += escaped;
                break;
            }
        }

        return false;
    }

    bool ExtractJsonUInt(
        const std::string& json,
        const std::string& key,
        uint32_t& outValue
    )
    {
        outValue = 0;

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

        size_t valueStart =
            colonPosition + 1;

        while (
            valueStart <
            json.size() &&
            (
                json[valueStart] == ' ' ||
                json[valueStart] == '\t' ||
                json[valueStart] == '\r' ||
                json[valueStart] == '\n'
                )
            )
        {
            ++valueStart;
        }

        size_t valueEnd =
            valueStart;

        while (
            valueEnd <
            json.size() &&
            json[valueEnd] >= '0' &&
            json[valueEnd] <= '9'
            )
        {
            ++valueEnd;
        }

        if (
            valueEnd ==
            valueStart
            )
        {
            return false;
        }

        try
        {
            outValue =
                static_cast<uint32_t>(
                    std::stoul(
                        json.substr(
                            valueStart,
                            valueEnd -
                            valueStart
                        )
                    )
                    );
        }
        catch (...)
        {
            return false;
        }

        return
            outValue != 0;
    }

    std::vector<std::string> ExtractJsonStringValues(
        const std::string& json,
        const std::string& key
    )
    {
        std::vector<std::string> values;

        const std::string searchKey =
            "\"" + key + "\"";

        size_t searchPosition = 0;

        while (searchPosition < json.size())
        {
            const size_t keyPosition =
                json.find(
                    searchKey,
                    searchPosition
                );

            if (keyPosition == std::string::npos)
            {
                break;
            }

            const size_t colonPosition =
                json.find(
                    ':',
                    keyPosition +
                    searchKey.size()
                );

            if (colonPosition == std::string::npos)
            {
                break;
            }

            const size_t quotePosition =
                json.find(
                    '"',
                    colonPosition + 1
                );

            if (quotePosition == std::string::npos)
            {
                break;
            }

            std::string value;
            bool escaped = false;

            size_t i =
                quotePosition + 1;

            for (; i < json.size(); ++i)
            {
                const char character =
                    json[i];

                if (escaped)
                {
                    value += character;
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
                    break;
                }

                value += character;
            }

            if (!value.empty())
            {
                values.push_back(
                    std::move(
                        value
                    )
                );
            }

            searchPosition =
                i < json.size()
                ? i + 1
                : json.size();
        }

        std::sort(
            values.begin(),
            values.end()
        );

        values.erase(
            std::unique(
                values.begin(),
                values.end()
            ),
            values.end()
        );

        return values;
    }

    std::string ToFriendlyAttributeName(
        const std::string& attribute
    )
    {
        if (attribute == "Power")
        {
            return "Power";
        }

        if (attribute == "Precision")
        {
            return "Precision";
        }

        if (attribute == "Toughness")
        {
            return "Toughness";
        }

        if (attribute == "Vitality")
        {
            return "Vitality";
        }

        if (attribute == "Ferocity")
        {
            return "Ferocity";
        }

        if (attribute == "ConditionDamage")
        {
            return "Condition Damage";
        }

        if (attribute == "Healing")
        {
            return "Healing Power";
        }

        if (attribute == "BoonDuration")
        {
            return "Concentration";
        }

        if (attribute == "ConditionDuration")
        {
            return "Expertise";
        }

        if (attribute == "AgonyResistance")
        {
            return {};
        }

        return attribute;
    }

    std::string BuildVariantLabel(
        const std::string& itemObject
    )
    {
        const std::vector<std::string> attributes =
            ExtractJsonStringValues(
                itemObject,
                "attribute"
            );

        if (attributes.empty())
        {
            return {};
        }

        std::vector<std::string> friendlyAttributes;

        for (
            const std::string& attribute :
            attributes
            )
        {
            const std::string friendly =
                ToFriendlyAttributeName(
                    attribute
                );

            if (friendly.empty())
            {
                continue;
            }

            friendlyAttributes.push_back(
                friendly
            );
        }

        if (friendlyAttributes.empty())
        {
            return {};
        }

        std::string label;

        for (size_t i = 0;
            i < friendlyAttributes.size();
            ++i)
        {
            if (i > 0)
            {
                label += " / ";
            }

            label +=
                friendlyAttributes[i];
        }

        return label;
    }

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

        WinHttpSetTimeouts(
            session,
            5000,
            5000,
            5000,
            10000
        );

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
            (
                statusCode != 200 &&
                statusCode != 206
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

            if (available == 0)
            {
                break;
            }

            std::vector<char> buffer(
                available
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

        return true;
    }

    std::vector<uint32_t> ParseIdArray(
        const std::string& json
    )
    {
        std::vector<uint32_t> ids;

        size_t position = 0;

        while (position < json.size())
        {
            while (
                position < json.size() &&
                (
                    json[position] < '0' ||
                    json[position] > '9'
                    )
                )
            {
                ++position;
            }

            if (position >= json.size())
            {
                break;
            }

            const size_t start =
                position;

            while (
                position < json.size() &&
                json[position] >= '0' &&
                json[position] <= '9'
                )
            {
                ++position;
            }

            try
            {
                const uint32_t id =
                    static_cast<uint32_t>(
                        std::stoul(
                            json.substr(
                                start,
                                position - start
                            )
                        )
                        );

                if (id != 0)
                {
                    ids.push_back(
                        id
                    );
                }
            }
            catch (...)
            {
                // Ignore malformed numeric fragments.
            }
        }

        std::sort(
            ids.begin(),
            ids.end()
        );

        ids.erase(
            std::unique(
                ids.begin(),
                ids.end()
            ),
            ids.end()
        );

        return ids;
    }

    std::vector<TradingPostIndexedItem>
        ParseItemArray(
            const std::string& json
        )
    {
        std::vector<TradingPostIndexedItem>
            items;

        bool inString = false;
        bool escaped = false;

        int objectDepth = 0;

        size_t objectStart =
            std::string::npos;

        for (size_t i = 0;
            i < json.size();
            ++i)
        {
            const char character =
                json[i];

            if (inString)
            {
                if (escaped)
                {
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
                    inString = false;
                }

                continue;
            }

            if (character == '"')
            {
                inString = true;
                continue;
            }

            if (character == '{')
            {
                if (objectDepth == 0)
                {
                    objectStart =
                        i;
                }

                ++objectDepth;
                continue;
            }

            if (
                character == '}' &&
                objectDepth > 0
                )
            {
                --objectDepth;

                if (
                    objectDepth == 0 &&
                    objectStart !=
                    std::string::npos
                    )
                {
                    const std::string object =
                        json.substr(
                            objectStart,
                            i -
                            objectStart + 1
                        );

                    uint32_t itemID = 0;
                    std::string name;

                    if (
                        ExtractJsonUInt(
                            object,
                            "id",
                            itemID
                        ) &&
                        ExtractJsonString(
                            object,
                            "name",
                            name
                        ) &&
                        !name.empty()
                        )
                    {
                        TradingPostIndexedItem item;

                        item.itemID =
                            itemID;

                        item.name =
                            name;

                        item.variantLabel =
                            BuildVariantLabel(
                                object
                            );

                        items.push_back(
                            std::move(
                                item
                            )
                        );
                    }

                    objectStart =
                        std::string::npos;
                }
            }
        }

        return items;
    }

    std::vector<IndexedRecord> BuildRecords(
        const std::vector<TradingPostIndexedItem>& items
    )
    {
        std::vector<IndexedRecord>
            records;

        records.reserve(
            items.size()
        );

        for (
            const TradingPostIndexedItem& item :
            items
            )
        {
            if (
                item.itemID == 0 ||
                item.name.empty()
                )
            {
                continue;
            }

            IndexedRecord record;

            record.item =
                item;

            std::string searchableText =
                item.name;

            if (!item.variantLabel.empty())
            {
                searchableText +=
                    " " +
                    item.variantLabel;
            }

            record.searchName =
                NormalizeSearchText(
                    searchableText
                );

            records.push_back(
                std::move(
                    record
                )
            );
        }

        std::sort(
            records.begin(),
            records.end(),
            [](
                const IndexedRecord& a,
                const IndexedRecord& b
                )
            {
                if (
                    a.item.itemID !=
                    b.item.itemID
                    )
                {
                    return
                        a.item.itemID <
                        b.item.itemID;
                }

                return
                    a.item.name <
                    b.item.name;
            }
        );

        records.erase(
            std::unique(
                records.begin(),
                records.end(),
                [](
                    const IndexedRecord& a,
                    const IndexedRecord& b
                    )
                {
                    return
                        a.item.itemID ==
                        b.item.itemID;
                }
            ),
            records.end()
        );

        return records;
    }

    bool SaveCache(
        const std::vector<IndexedRecord>& records,
        uint64_t updatedUnixSeconds
    )
    {
        if (g_CachePath.empty())
        {
            return false;
        }

        std::filesystem::path tempPath =
            g_CachePath;

        tempPath +=
            L".tmp";

        std::ofstream file(
            tempPath,
            std::ios::trunc |
            std::ios::binary
        );

        if (!file.is_open())
        {
            return false;
        }

        file
            << "updated="
            << updatedUnixSeconds
            << '\n';

        for (
            const IndexedRecord& record :
            records
            )
        {
            std::string safeName =
                record.item.name;

            safeName.erase(
                std::remove(
                    safeName.begin(),
                    safeName.end(),
                    '\r'
                ),
                safeName.end()
            );

            std::replace(
                safeName.begin(),
                safeName.end(),
                '\n',
                ' '
            );

            std::replace(
                safeName.begin(),
                safeName.end(),
                '\t',
                ' '
            );

            std::string safeVariant =
                record.item.variantLabel;

            safeVariant.erase(
                std::remove(
                    safeVariant.begin(),
                    safeVariant.end(),
                    '\r'
                ),
                safeVariant.end()
            );

            std::replace(
                safeVariant.begin(),
                safeVariant.end(),
                '\n',
                ' '
            );

            std::replace(
                safeVariant.begin(),
                safeVariant.end(),
                '\t',
                ' '
            );

            file
                << record.item.itemID
                << '\t'
                << safeName
                << '\t'
                << safeVariant
                << '\n';
        }

        file.flush();

        if (!file.good())
        {
            file.close();

            std::error_code removeError;

            std::filesystem::remove(
                tempPath,
                removeError
            );

            return false;
        }

        file.close();

        const BOOL moved =
            MoveFileExW(
                tempPath.c_str(),
                g_CachePath.c_str(),
                MOVEFILE_REPLACE_EXISTING |
                MOVEFILE_WRITE_THROUGH
            );

        if (!moved)
        {
            std::error_code removeError;

            std::filesystem::remove(
                tempPath,
                removeError
            );

            return false;
        }

        return true;
    }

    bool LoadCache()
    {
        if (
            g_CachePath.empty() ||
            !std::filesystem::exists(
                g_CachePath
            )
            )
        {
            return false;
        }

        std::ifstream file(
            g_CachePath,
            std::ios::binary
        );

        if (!file.is_open())
        {
            return false;
        }

        std::string line;

        uint64_t updatedUnixSeconds = 0;

        if (
            std::getline(
                file,
                line
            ) &&
            line.rfind(
                "updated=",
                0
            ) == 0
            )
        {
            try
            {
                updatedUnixSeconds =
                    std::stoull(
                        line.substr(
                            8
                        )
                    );
            }
            catch (...)
            {
                updatedUnixSeconds = 0;
            }
        }
        else
        {
            return false;
        }

        std::vector<
            TradingPostIndexedItem
        > items;

        bool legacyCacheFormat =
            false;

        while (
            std::getline(
                file,
                line
            )
            )
        {
            const size_t separator =
                line.find(
                    '\t'
                );

            if (
                separator ==
                std::string::npos
                )
            {
                continue;
            }

            try
            {
                TradingPostIndexedItem item;

                item.itemID =
                    static_cast<uint32_t>(
                        std::stoul(
                            line.substr(
                                0,
                                separator
                            )
                        )
                        );

                const size_t secondSeparator =
                    line.find(
                        '\t',
                        separator + 1
                    );

                if (
                    secondSeparator ==
                    std::string::npos
                    )
                {
                    legacyCacheFormat =
                        true;

                    item.name =
                        line.substr(
                            separator + 1
                        );
                }
                else
                {
                    item.name =
                        line.substr(
                            separator + 1,
                            secondSeparator -
                            separator - 1
                        );

                    item.variantLabel =
                        line.substr(
                            secondSeparator + 1
                        );
                }

                if (
                    item.itemID != 0 &&
                    !item.name.empty()
                    )
                {
                    items.push_back(
                        std::move(
                            item
                        )
                    );
                }
            }
            catch (...)
            {
                // Ignore malformed cache rows.
            }
        }

        std::vector<IndexedRecord> records =
            BuildRecords(
                items
            );

        if (records.empty())
        {
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(
                g_IndexMutex
            );

            g_Index =
                std::move(
                    records
                );

            g_IndexReady =
                true;

            g_CacheUpdatedUnixSeconds =
                legacyCacheFormat
                ? 0
                : updatedUnixSeconds;
        }

        return true;
    }

    bool ShouldRefreshCache()
    {
        std::lock_guard<std::mutex> lock(
            g_IndexMutex
        );

        if (!g_IndexReady)
        {
            return true;
        }

        if (
            g_CacheUpdatedUnixSeconds == 0
            )
        {
            return true;
        }

        const uint64_t now =
            GetUnixSeconds();

        if (
            now <
            g_CacheUpdatedUnixSeconds
            )
        {
            return true;
        }

        return
            now -
            g_CacheUpdatedUnixSeconds >=
            CACHE_MAX_AGE_SECONDS;
    }

    std::wstring BuildItemsBatchPath(
        const std::vector<uint32_t>& ids,
        size_t start,
        size_t count
    )
    {
        std::wstring path =
            L"/v2/items?lang=en&ids=";

        for (size_t i = 0;
            i < count;
            ++i)
        {
            if (i > 0)
            {
                path += L",";
            }

            path +=
                std::to_wstring(
                    ids[
                        start + i
                    ]
                );
        }

        return path;
    }

    void SetBuildFailure(
        const std::string& message
    )
    {
        std::lock_guard<std::mutex> lock(
            g_IndexMutex
        );

        g_Building = false;
        g_LastError = message;
    }

    void BuildIndex()
    {
        {
            std::lock_guard<std::mutex> lock(
                g_IndexMutex
            );

            g_Building = true;
            g_BuildProcessedCount = 0;
            g_BuildTotalCount = 0;
            g_LastError.clear();
        }

        std::string priceIdsResponse;

        if (
            !HttpGet(
                L"/v2/commerce/prices",
                priceIdsResponse
            )
            )
        {
            SetBuildFailure(
                "Could not download the Trading Post item list."
            );

            return;
        }

        std::vector<uint32_t> itemIDs =
            ParseIdArray(
                priceIdsResponse
            );

        if (itemIDs.empty())
        {
            SetBuildFailure(
                "Trading Post item list was empty."
            );

            return;
        }

        {
            std::lock_guard<std::mutex> lock(
                g_IndexMutex
            );

            g_BuildTotalCount =
                itemIDs.size();
        }

        std::vector<
            TradingPostIndexedItem
        > rebuiltItems;

        rebuiltItems.reserve(
            itemIDs.size()
        );

        bool hadUsableIndex = false;

        {
            std::lock_guard<std::mutex> lock(
                g_IndexMutex
            );

            hadUsableIndex =
                g_IndexReady &&
                !g_Index.empty();
        }

        for (size_t start = 0;
            start < itemIDs.size();
            start += ITEM_BATCH_SIZE)
        {
            if (g_StopWorker.load())
            {
                SetBuildFailure(
                    "Item-index build stopped."
                );

                return;
            }

            const size_t remaining =
                itemIDs.size() -
                start;

            const size_t batchCount =
                std::min(
                    ITEM_BATCH_SIZE,
                    remaining
                );

            const std::wstring path =
                BuildItemsBatchPath(
                    itemIDs,
                    start,
                    batchCount
                );

            std::string itemResponse;

            if (
                !HttpGet(
                    path,
                    itemResponse
                )
                )
            {
                SetBuildFailure(
                    "Could not download item metadata while building the search index."
                );

                return;
            }

            std::vector<
                TradingPostIndexedItem
            > batchItems =
                ParseItemArray(
                    itemResponse
                );

            //
            // Build the searchable representation for this batch once.
            // On the first-ever build we can append each completed batch
            // directly to the live index instead of rebuilding all previous
            // records every time.
            //
            std::vector<IndexedRecord> batchRecords =
                BuildRecords(
                    batchItems
                );

            rebuiltItems.insert(
                rebuiltItems.end(),
                std::make_move_iterator(
                    batchItems.begin()
                ),
                std::make_move_iterator(
                    batchItems.end()
                )
            );

            {
                std::lock_guard<std::mutex> lock(
                    g_IndexMutex
                );

                g_BuildProcessedCount =
                    std::min(
                        start +
                        batchCount,
                        itemIDs.size()
                    );

                //
                // On the first-ever build there is no cache to search.
                // Publish completed batches as they arrive so autocomplete
                // becomes useful before the full catalog has finished.
                //
                if (!hadUsableIndex)
                {
                    g_Index.insert(
                        g_Index.end(),
                        std::make_move_iterator(
                            batchRecords.begin()
                        ),
                        std::make_move_iterator(
                            batchRecords.end()
                        )
                    );

                    g_IndexReady =
                        !g_Index.empty();
                }
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(
                    25
                )
            );
        }

        std::vector<IndexedRecord> rebuiltRecords =
            BuildRecords(
                rebuiltItems
            );

        if (rebuiltRecords.empty())
        {
            SetBuildFailure(
                "No searchable Trading Post items were returned."
            );

            return;
        }

        const uint64_t updatedUnixSeconds =
            GetUnixSeconds();

        const bool cacheSaved =
            SaveCache(
                rebuiltRecords,
                updatedUnixSeconds
            );

        {
            std::lock_guard<std::mutex> lock(
                g_IndexMutex
            );

            g_Index =
                std::move(
                    rebuiltRecords
                );

            g_IndexReady = true;
            g_Building = false;

            g_BuildProcessedCount =
                itemIDs.size();

            g_BuildTotalCount =
                itemIDs.size();

            g_CacheUpdatedUnixSeconds =
                updatedUnixSeconds;

            if (!cacheSaved)
            {
                g_LastError =
                    "Item search works, but the local item-index cache could not be saved.";
            }
            else
            {
                g_LastError.clear();
            }
        }
    }

    void IndexWorker()
    {
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(
                    g_RequestMutex
                );

                g_RequestCondition.wait(
                    lock,
                    []()
                    {
                        return
                            g_StopWorker.load() ||
                            g_RefreshRequested;
                    }
                );

                if (g_StopWorker.load())
                {
                    break;
                }

                g_RefreshRequested =
                    false;
            }

            BuildIndex();
        }
    }
}

void TradingPostItemIndexManager::Start(
    void* moduleHandle
)
{
    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        if (g_WorkerRunning)
        {
            return;
        }

        g_CachePath =
            BuildCachePath(
                moduleHandle
            );

        g_StopWorker.store(
            false
        );

        g_RefreshRequested =
            false;

        g_WorkerRunning =
            true;
    }

    LoadCache();

    g_WorkerThread =
        std::thread(
            IndexWorker
        );

    if (ShouldRefreshCache())
    {
        RequestRefresh();
    }
}

void TradingPostItemIndexManager::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        if (!g_WorkerRunning)
        {
            return;
        }

        g_StopWorker.store(
            true
        );

        g_RefreshRequested =
            false;
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

        g_WorkerRunning =
            false;
    }
}

void TradingPostItemIndexManager::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    g_Index.clear();

    g_IndexReady = false;
    g_Building = false;

    g_BuildProcessedCount = 0;
    g_BuildTotalCount = 0;

    g_LastError.clear();

    g_CacheUpdatedUnixSeconds = 0;
}

void TradingPostItemIndexManager::RequestRefresh()
{
    {
        std::lock_guard<std::mutex> lock(
            g_RequestMutex
        );

        if (
            !g_WorkerRunning ||
            g_StopWorker.load()
            )
        {
            return;
        }

        g_RefreshRequested =
            true;
    }

    g_RequestCondition.notify_one();
}

std::vector<
    TradingPostIndexedItem
> TradingPostItemIndexManager::Search(
    const std::string& query,
    size_t maxResults
)
{
    std::vector<
        TradingPostIndexedItem
    > results;

    if (maxResults == 0)
    {
        return results;
    }

    const std::string normalizedQuery =
        NormalizeSearchText(
            query
        );

    if (normalizedQuery.empty())
    {
        return results;
    }

    const std::vector<std::string> tokens =
        SplitSearchTokens(
            normalizedQuery
        );

    if (tokens.empty())
    {
        return results;
    }

    std::vector<SearchCandidate>
        candidates;

    {
        std::lock_guard<std::mutex> lock(
            g_IndexMutex
        );

        candidates.reserve(
            std::min(
                g_Index.size(),
                static_cast<size_t>(
                    256
                    )
            )
        );

        for (
            const IndexedRecord& record :
            g_Index
            )
        {
            const int rank =
                GetSearchRank(
                    record.searchName,
                    normalizedQuery,
                    tokens
                );

            if (rank < 0)
            {
                continue;
            }

            SearchCandidate candidate;

            candidate.item =
                record.item;

            candidate.rank =
                rank;

            candidate.nameLength =
                record.item.name.size() +
                record.item.variantLabel.size();

            candidate.normalizedName =
                record.searchName;

            candidates.push_back(
                std::move(
                    candidate
                )
            );
        }
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](
            const SearchCandidate& a,
            const SearchCandidate& b
            )
        {
            if (a.rank != b.rank)
            {
                return
                    a.rank <
                    b.rank;
            }

            if (
                a.nameLength !=
                b.nameLength
                )
            {
                return
                    a.nameLength <
                    b.nameLength;
            }

            if (
                a.normalizedName !=
                b.normalizedName
                )
            {
                return
                    a.normalizedName <
                    b.normalizedName;
            }

            return
                a.item.itemID <
                b.item.itemID;
        }
    );

    const size_t resultCount =
        std::min(
            maxResults,
            candidates.size()
        );

    results.reserve(
        resultCount
    );

    for (size_t i = 0;
        i < resultCount;
        ++i)
    {
        results.push_back(
            std::move(
                candidates[i].item
            )
        );
    }

    return results;
}

bool TradingPostItemIndexManager::IsReady()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    return
        g_IndexReady &&
        !g_Index.empty();
}

bool TradingPostItemIndexManager::IsBuilding()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    return
        g_Building;
}

size_t TradingPostItemIndexManager::GetItemCount()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    return
        g_Index.size();
}

size_t TradingPostItemIndexManager::
GetBuildProcessedCount()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    return
        g_BuildProcessedCount;
}

size_t TradingPostItemIndexManager::
GetBuildTotalCount()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    return
        g_BuildTotalCount;
}

std::string TradingPostItemIndexManager::
GetLastError()
{
    std::lock_guard<std::mutex> lock(
        g_IndexMutex
    );

    return
        g_LastError;
}
