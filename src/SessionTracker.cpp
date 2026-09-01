#include "SessionTracker.h"

#include <Windows.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::mutex g_SessionMutex;

    SessionStats g_Stats;

    uint32_t g_LastFoodSkillID = 0;
    uint32_t g_LastUtilitySkillID = 0;

    bool g_HasLastUpdate = false;

    std::chrono::steady_clock::time_point
        g_LastUpdateTime;

    uint64_t g_SessionStartedUnixSeconds = 0;

    // Character attached to the active session. Character changes split
    // history into separate records so per-character filtering is accurate.
    std::string g_CurrentCharacterName;

    std::filesystem::path
        g_HistoryPath;

    std::vector<SessionHistoryRecord>
        g_History;

    uint64_t GetUnixSecondsNow()
    {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<
            std::chrono::seconds
            >(
                std::chrono::system_clock::
                now().
                time_since_epoch()
            ).count()
            );
    }

    std::filesystem::path BuildHistoryPath(
        void* moduleHandle
    )
    {
        if (moduleHandle == nullptr)
        {
            return {};
        }

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

            std::filesystem::path path(
                modulePath
            );

            return
                path.parent_path() /
                L"FoodReminder_SessionHistory.tsv";
    }

    void IncrementUsage(
        std::vector<SessionConsumableUsage>& usageList,
        uint32_t skillID
    )
    {
        for (
            SessionConsumableUsage& entry :
            usageList
            )
        {
            if (entry.skillID == skillID)
            {
                ++entry.uses;
                return;
            }
        }

        SessionConsumableUsage newEntry;

        newEntry.skillID =
            skillID;

        newEntry.uses =
            1;

        usageList.push_back(
            newEntry
        );
    }

    void AddPrimerProtectedTime(
        std::vector<SessionPrimerProtectedUsage>& usageList,
        uint32_t skillID,
        int64_t elapsedMilliseconds
    )
    {
        if (
            skillID == 0 ||
            elapsedMilliseconds <= 0
            )
        {
            return;
        }

        for (
            SessionPrimerProtectedUsage& usage :
            usageList
            )
        {
            if (usage.skillID == skillID)
            {
                usage.protectedMilliseconds +=
                    elapsedMilliseconds;

                return;
            }
        }

        SessionPrimerProtectedUsage newUsage;

        newUsage.skillID =
            skillID;

        newUsage.protectedMilliseconds =
            elapsedMilliseconds;

        usageList.push_back(
            newUsage
        );
    }

    void AddHistoryEvent(
        uint32_t skillID,
        uint32_t previousSkillID,
        int64_t previousRemainingMilliseconds,
        bool isFood,
        bool inCombat,
        SessionConsumableEventType type
    )
    {
        SessionConsumableEvent event;

        event.sessionMilliseconds =
            g_Stats.sessionMilliseconds;

        event.skillID =
            skillID;

        event.previousSkillID =
            previousSkillID;

        event.previousRemainingMilliseconds =
            previousRemainingMilliseconds;

        event.isFood =
            isFood;

        event.inCombat =
            inCombat;

        event.type =
            type;

        g_Stats.consumableHistory.push_back(
            event
        );
    }

    void TrackMetabolicPrimerState(
        SessionPrimerState state,
        bool hasFood,
        uint32_t foodSkillID,
        int64_t elapsedMilliseconds
    )
    {
        switch (state)
        {
        case SessionPrimerState::ConfirmedActive:
            g_Stats.metabolicPrimerConfirmedMilliseconds +=
                elapsedMilliseconds;

            g_Stats.metabolicPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasFood)
            {
                AddPrimerProtectedTime(
                    g_Stats.foodPrimerProtection,
                    foodSkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::InferredActive:
            g_Stats.metabolicPrimerInferredMilliseconds +=
                elapsedMilliseconds;

            g_Stats.metabolicPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasFood)
            {
                AddPrimerProtectedTime(
                    g_Stats.foodPrimerProtection,
                    foodSkillID,
                    elapsedMilliseconds
                );

                AddPrimerProtectedTime(
                    g_Stats.foodPrimerInferredProtection,
                    foodSkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::Unknown:
            g_Stats.metabolicPrimerUnknownMilliseconds +=
                elapsedMilliseconds;
            break;

        case SessionPrimerState::Inactive:
        default:
            break;
        }
    }

    void TrackUtilityPrimerState(
        SessionPrimerState state,
        bool hasUtility,
        uint32_t utilitySkillID,
        int64_t elapsedMilliseconds
    )
    {
        switch (state)
        {
        case SessionPrimerState::ConfirmedActive:
            g_Stats.utilityPrimerConfirmedMilliseconds +=
                elapsedMilliseconds;

            g_Stats.utilityPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasUtility)
            {
                AddPrimerProtectedTime(
                    g_Stats.utilityPrimerProtection,
                    utilitySkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::InferredActive:
            g_Stats.utilityPrimerInferredMilliseconds +=
                elapsedMilliseconds;

            g_Stats.utilityPrimerActiveMilliseconds +=
                elapsedMilliseconds;

            if (hasUtility)
            {
                AddPrimerProtectedTime(
                    g_Stats.utilityPrimerProtection,
                    utilitySkillID,
                    elapsedMilliseconds
                );

                AddPrimerProtectedTime(
                    g_Stats.utilityPrimerInferredProtection,
                    utilitySkillID,
                    elapsedMilliseconds
                );
            }
            break;

        case SessionPrimerState::Unknown:
            g_Stats.utilityPrimerUnknownMilliseconds +=
                elapsedMilliseconds;
            break;

        case SessionPrimerState::Inactive:
        default:
            break;
        }
    }

    std::string EncodeConsumableUsage(
        const std::vector<
        SessionConsumableUsage
        >& usageList
    )
    {
        if (usageList.empty())
        {
            return "-";
        }

        std::ostringstream output;

        for (
            size_t i = 0;
            i < usageList.size();
            ++i
            )
        {
            if (i > 0)
            {
                output << ',';
            }

            output
                << usageList[i].skillID
                << ':'
                << usageList[i].uses;
        }

        return output.str();
    }

    std::string EncodePrimerProtection(
        const std::vector<
        SessionPrimerProtectedUsage
        >& usageList
    )
    {
        if (usageList.empty())
        {
            return "-";
        }

        std::ostringstream output;

        for (
            size_t i = 0;
            i < usageList.size();
            ++i
            )
        {
            if (i > 0)
            {
                output << ',';
            }

            output
                << usageList[i].skillID
                << ':'
                << usageList[i].
                protectedMilliseconds;
        }

        return output.str();
    }

    void DecodeConsumableUsage(
        const std::string& text,
        std::vector<
        SessionConsumableUsage
        >& outUsage
    )
    {
        outUsage.clear();

        if (
            text.empty() ||
            text == "-"
            )
        {
            return;
        }

        std::stringstream stream(
            text
        );

        std::string token;

        while (
            std::getline(
                stream,
                token,
                ','
            )
            )
        {
            const size_t separator =
                token.find(':');

            if (
                separator ==
                std::string::npos
                )
            {
                continue;
            }

            try
            {
                SessionConsumableUsage usage;

                usage.skillID =
                    static_cast<uint32_t>(
                        std::stoul(
                            token.substr(
                                0,
                                separator
                            )
                        )
                        );

                usage.uses =
                    static_cast<uint32_t>(
                        std::stoul(
                            token.substr(
                                separator + 1
                            )
                        )
                        );

                if (
                    usage.skillID != 0 &&
                    usage.uses != 0
                    )
                {
                    outUsage.push_back(
                        usage
                    );
                }
            }
            catch (...)
            {
                // Ignore malformed history tokens.
            }
        }
    }

    void DecodePrimerProtection(
        const std::string& text,
        std::vector<
        SessionPrimerProtectedUsage
        >& outUsage
    )
    {
        outUsage.clear();

        if (
            text.empty() ||
            text == "-"
            )
        {
            return;
        }

        std::stringstream stream(
            text
        );

        std::string token;

        while (
            std::getline(
                stream,
                token,
                ','
            )
            )
        {
            const size_t separator =
                token.find(':');

            if (
                separator ==
                std::string::npos
                )
            {
                continue;
            }

            try
            {
                SessionPrimerProtectedUsage usage;

                usage.skillID =
                    static_cast<uint32_t>(
                        std::stoul(
                            token.substr(
                                0,
                                separator
                            )
                        )
                        );

                usage.protectedMilliseconds =
                    std::stoll(
                        token.substr(
                            separator + 1
                        )
                    );

                if (
                    usage.skillID != 0 &&
                    usage.protectedMilliseconds > 0
                    )
                {
                    outUsage.push_back(
                        usage
                    );
                }
            }
            catch (...)
            {
                // Ignore malformed history tokens.
            }
        }
    }

    void SaveHistoryLocked()
    {
        if (g_HistoryPath.empty())
        {
            return;
        }

        std::ofstream file(
            g_HistoryPath,
            std::ios::trunc |
            std::ios::binary
        );

        if (!file.is_open())
        {
            return;
        }

        file
            << "# FoodReminder-Nexus Session History v2\n";

        file
            << "# started_unix\tended_unix\tcharacter"
            << "\tsession_ms\tcombat_ms"
            << "\tfood_active_ms\tutility_active_ms"
            << "\tfood_combat_ms\tutility_combat_ms"
            << "\tfood_apps\tfood_refreshes\tfood_replacements"
            << "\tfood_expired_combat\tfood_wasted_ms"
            << "\tworst_food_waste_ms\tworst_food_skill"
            << "\tutility_apps\tutility_refreshes\tutility_replacements"
            << "\tutility_expired_combat\tutility_wasted_ms"
            << "\tworst_utility_waste_ms\tworst_utility_skill"
            << "\tmetabolic_confirmed_ms\tmetabolic_inferred_ms"
            << "\tmetabolic_unknown_ms\tmetabolic_active_ms"
            << "\tutility_primer_confirmed_ms\tutility_primer_inferred_ms"
            << "\tutility_primer_unknown_ms\tutility_primer_active_ms"
            << "\test_food_uses_saved\test_utility_uses_saved"
            << "\tfood_usage\tutility_usage"
            << "\tfood_primer_protection\tutility_primer_protection"
            << "\tfood_primer_inferred\tutility_primer_inferred\n";

        for (
            const SessionHistoryRecord& record :
            g_History
            )
        {
            const SessionStats& stats =
                record.stats;

            file
                << record.startedUnixSeconds
                << '\t'
                << record.endedUnixSeconds
                << '\t'
                << record.characterName
                << '\t'
                << stats.sessionMilliseconds
                << '\t'
                << stats.combatMilliseconds
                << '\t'
                << stats.foodActiveMilliseconds
                << '\t'
                << stats.utilityActiveMilliseconds
                << '\t'
                << stats.foodCombatMilliseconds
                << '\t'
                << stats.utilityCombatMilliseconds
                << '\t'
                << stats.foodApplications
                << '\t'
                << stats.foodRefreshes
                << '\t'
                << stats.foodReplacements
                << '\t'
                << stats.foodExpiredInCombat
                << '\t'
                << stats.foodWastedMilliseconds
                << '\t'
                << stats.worstFoodWasteMilliseconds
                << '\t'
                << stats.worstFoodWasteSkillID
                << '\t'
                << stats.utilityApplications
                << '\t'
                << stats.utilityRefreshes
                << '\t'
                << stats.utilityReplacements
                << '\t'
                << stats.utilityExpiredInCombat
                << '\t'
                << stats.utilityWastedMilliseconds
                << '\t'
                << stats.worstUtilityWasteMilliseconds
                << '\t'
                << stats.worstUtilityWasteSkillID
                << '\t'
                << stats.metabolicPrimerConfirmedMilliseconds
                << '\t'
                << stats.metabolicPrimerInferredMilliseconds
                << '\t'
                << stats.metabolicPrimerUnknownMilliseconds
                << '\t'
                << stats.metabolicPrimerActiveMilliseconds
                << '\t'
                << stats.utilityPrimerConfirmedMilliseconds
                << '\t'
                << stats.utilityPrimerInferredMilliseconds
                << '\t'
                << stats.utilityPrimerUnknownMilliseconds
                << '\t'
                << stats.utilityPrimerActiveMilliseconds
                << '\t'
                << stats.estimatedFoodUsesSaved
                << '\t'
                << stats.estimatedUtilityUsesSaved
                << '\t'
                << EncodeConsumableUsage(
                    stats.foodUsage
                )
                << '\t'
                << EncodeConsumableUsage(
                    stats.utilityUsage
                )
                << '\t'
                << EncodePrimerProtection(
                    stats.foodPrimerProtection
                )
                << '\t'
                << EncodePrimerProtection(
                    stats.utilityPrimerProtection
                )
                << '\t'
                << EncodePrimerProtection(
                    stats.foodPrimerInferredProtection
                )
                << '\t'
                << EncodePrimerProtection(
                    stats.utilityPrimerInferredProtection
                )
                << '\n';
        }
    }

    bool ParseHistoryLine(
        const std::string& line,
        SessionHistoryRecord& outRecord
    )
    {
        std::vector<std::string> fields;

        std::stringstream stream(
            line
        );

        std::string field;

        while (
            std::getline(
                stream,
                field,
                '\t'
            )
            )
        {
            fields.push_back(
                field
            );
        }

        const bool hasCharacterField =
            fields.size() == 39;

        if (
            fields.size() != 38 &&
            !hasCharacterField
            )
        {
            return false;
        }

        try
        {
            size_t i = 0;

            outRecord = {};

            outRecord.startedUnixSeconds =
                std::stoull(
                    fields[i++]
                );

            outRecord.endedUnixSeconds =
                std::stoull(
                    fields[i++]
                );

            if (hasCharacterField)
            {
                outRecord.characterName =
                    fields[i++];
            }

            SessionStats& stats =
                outRecord.stats;

            stats.sessionMilliseconds =
                std::stoll(fields[i++]);

            stats.combatMilliseconds =
                std::stoll(fields[i++]);

            stats.foodActiveMilliseconds =
                std::stoll(fields[i++]);

            stats.utilityActiveMilliseconds =
                std::stoll(fields[i++]);

            stats.foodCombatMilliseconds =
                std::stoll(fields[i++]);

            stats.utilityCombatMilliseconds =
                std::stoll(fields[i++]);

            stats.foodApplications =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.foodRefreshes =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.foodReplacements =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.foodExpiredInCombat =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.foodWastedMilliseconds =
                std::stoll(fields[i++]);

            stats.worstFoodWasteMilliseconds =
                std::stoll(fields[i++]);

            stats.worstFoodWasteSkillID =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.utilityApplications =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.utilityRefreshes =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.utilityReplacements =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.utilityExpiredInCombat =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.utilityWastedMilliseconds =
                std::stoll(fields[i++]);

            stats.worstUtilityWasteMilliseconds =
                std::stoll(fields[i++]);

            stats.worstUtilityWasteSkillID =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.metabolicPrimerConfirmedMilliseconds =
                std::stoll(fields[i++]);

            stats.metabolicPrimerInferredMilliseconds =
                std::stoll(fields[i++]);

            stats.metabolicPrimerUnknownMilliseconds =
                std::stoll(fields[i++]);

            stats.metabolicPrimerActiveMilliseconds =
                std::stoll(fields[i++]);

            stats.utilityPrimerConfirmedMilliseconds =
                std::stoll(fields[i++]);

            stats.utilityPrimerInferredMilliseconds =
                std::stoll(fields[i++]);

            stats.utilityPrimerUnknownMilliseconds =
                std::stoll(fields[i++]);

            stats.utilityPrimerActiveMilliseconds =
                std::stoll(fields[i++]);

            stats.estimatedFoodUsesSaved =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            stats.estimatedUtilityUsesSaved =
                static_cast<uint32_t>(
                    std::stoul(fields[i++])
                    );

            DecodeConsumableUsage(
                fields[i++],
                stats.foodUsage
            );

            DecodeConsumableUsage(
                fields[i++],
                stats.utilityUsage
            );

            DecodePrimerProtection(
                fields[i++],
                stats.foodPrimerProtection
            );

            DecodePrimerProtection(
                fields[i++],
                stats.utilityPrimerProtection
            );

            DecodePrimerProtection(
                fields[i++],
                stats.foodPrimerInferredProtection
            );

            DecodePrimerProtection(
                fields[i++],
                stats.utilityPrimerInferredProtection
            );

            return
                stats.sessionMilliseconds > 0;
        }
        catch (...)
        {
            return false;
        }
    }

    void LoadHistoryLocked()
    {
        g_History.clear();

        if (g_HistoryPath.empty())
        {
            return;
        }

        std::ifstream file(
            g_HistoryPath,
            std::ios::binary
        );

        if (!file.is_open())
        {
            return;
        }

        std::string line;

        while (
            std::getline(
                file,
                line
            )
            )
        {
            if (
                line.empty() ||
                line[0] == '#'
                )
            {
                continue;
            }

            SessionHistoryRecord record;

            if (
                ParseHistoryLine(
                    line,
                    record
                )
                )
            {
                g_History.push_back(
                    record
                );
            }
        }
    }

    bool HasMeaningfulSessionLocked()
    {
        //
        // Ignore empty/near-empty sessions caused by addon load/unload.
        //
        return
            g_Stats.sessionMilliseconds >=
            1000;
    }

    void ArchiveCurrentSessionLocked()
    {
        if (!HasMeaningfulSessionLocked())
        {
            return;
        }

        SessionHistoryRecord record;

        record.startedUnixSeconds =
            g_SessionStartedUnixSeconds;

        record.endedUnixSeconds =
            GetUnixSecondsNow();

        record.characterName =
            g_CurrentCharacterName;

        record.stats =
            g_Stats;

        //
        // Detailed per-event history is session-UI data. The persistent
        // history layer stores aggregate stats and per-item usage/protection
        // needed by the future History tab without duplicating every event.
        //
        record.stats.consumableHistory.clear();

        g_History.push_back(
            record
        );

        SaveHistoryLocked();
    }

    void BeginFreshSessionLocked()
    {
        g_Stats = {};

        g_LastFoodSkillID = 0;
        g_LastUtilitySkillID = 0;

        g_HasLastUpdate = false;

        g_LastUpdateTime = {};

        g_SessionStartedUnixSeconds =
            GetUnixSecondsNow();
    }
}

void SessionTracker::Start(
    void* moduleHandle
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    g_HistoryPath =
        BuildHistoryPath(
            moduleHandle
        );

    LoadHistoryLocked();

    BeginFreshSessionLocked();
}

void SessionTracker::Shutdown()
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ArchiveCurrentSessionLocked();

    BeginFreshSessionLocked();
}

void SessionTracker::Update(
    const std::string& characterName,
    bool hasFood,
    uint32_t foodSkillID,
    bool hasUtility,
    uint32_t utilitySkillID,
    SessionPrimerState metabolicPrimerState,
    SessionPrimerState utilityPrimerState,
    bool inCombat
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    if (!characterName.empty())
    {
        if (g_CurrentCharacterName.empty())
        {
            g_CurrentCharacterName =
                characterName;
        }
        else if (
            g_CurrentCharacterName !=
            characterName
            )
        {
            // End the previous character's session before tracking the new
            // character. This prevents one history dot from mixing characters.
            ArchiveCurrentSessionLocked();
            BeginFreshSessionLocked();

            g_CurrentCharacterName =
                characterName;
        }
    }

    const auto now =
        std::chrono::steady_clock::now();

    if (!g_HasLastUpdate)
    {
        g_LastUpdateTime = now;
        g_HasLastUpdate = true;

        if (
            g_SessionStartedUnixSeconds == 0
            )
        {
            g_SessionStartedUnixSeconds =
                GetUnixSecondsNow();
        }

        return;
    }

    const int64_t elapsedMilliseconds =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            now -
            g_LastUpdateTime
        ).count();

    g_LastUpdateTime = now;

    if (elapsedMilliseconds <= 0)
    {
        return;
    }

    g_Stats.sessionMilliseconds +=
        elapsedMilliseconds;

    if (hasFood)
    {
        g_Stats.foodActiveMilliseconds +=
            elapsedMilliseconds;
    }

    if (hasUtility)
    {
        g_Stats.utilityActiveMilliseconds +=
            elapsedMilliseconds;
    }

    TrackMetabolicPrimerState(
        metabolicPrimerState,
        hasFood,
        foodSkillID,
        elapsedMilliseconds
    );

    TrackUtilityPrimerState(
        utilityPrimerState,
        hasUtility,
        utilitySkillID,
        elapsedMilliseconds
    );

    if (inCombat)
    {
        g_Stats.combatMilliseconds +=
            elapsedMilliseconds;

        if (hasFood)
        {
            g_Stats.foodCombatMilliseconds +=
                elapsedMilliseconds;
        }

        if (hasUtility)
        {
            g_Stats.utilityCombatMilliseconds +=
                elapsedMilliseconds;
        }
    }
}

void SessionTracker::RecordFoodApplication(
    uint32_t skillID,
    int64_t previousRemainingMilliseconds
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.foodApplications;

    IncrementUsage(
        g_Stats.foodUsage,
        skillID
    );

    if (g_LastFoodSkillID == 0)
    {
        AddHistoryEvent(
            skillID,
            0,
            0,
            true,
            false,
            SessionConsumableEventType::Applied
        );
    }
    else if (g_LastFoodSkillID == skillID)
    {
        AddHistoryEvent(
            skillID,
            g_LastFoodSkillID,
            0,
            true,
            false,
            SessionConsumableEventType::Refreshed
        );
    }
    else
    {
        AddHistoryEvent(
            skillID,
            g_LastFoodSkillID,
            previousRemainingMilliseconds,
            true,
            false,
            SessionConsumableEventType::Replaced
        );
    }

    if (g_LastFoodSkillID != 0)
    {
        if (g_LastFoodSkillID == skillID)
        {
            ++g_Stats.foodRefreshes;
        }
        else
        {
            ++g_Stats.foodReplacements;

            g_Stats.foodWastedMilliseconds +=
                previousRemainingMilliseconds;

            if (
                previousRemainingMilliseconds >
                g_Stats.worstFoodWasteMilliseconds
                )
            {
                g_Stats.worstFoodWasteMilliseconds =
                    previousRemainingMilliseconds;

                g_Stats.worstFoodWasteSkillID =
                    g_LastFoodSkillID;
            }
        }
    }

    g_LastFoodSkillID =
        skillID;
}

void SessionTracker::RecordUtilityApplication(
    uint32_t skillID,
    int64_t previousRemainingMilliseconds
)
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.utilityApplications;

    IncrementUsage(
        g_Stats.utilityUsage,
        skillID
    );

    if (g_LastUtilitySkillID == 0)
    {
        AddHistoryEvent(
            skillID,
            0,
            0,
            false,
            false,
            SessionConsumableEventType::Applied
        );
    }
    else if (g_LastUtilitySkillID == skillID)
    {
        AddHistoryEvent(
            skillID,
            g_LastUtilitySkillID,
            0,
            false,
            false,
            SessionConsumableEventType::Refreshed
        );
    }
    else
    {
        AddHistoryEvent(
            skillID,
            g_LastUtilitySkillID,
            previousRemainingMilliseconds,
            false,
            false,
            SessionConsumableEventType::Replaced
        );
    }

    if (g_LastUtilitySkillID != 0)
    {
        if (g_LastUtilitySkillID == skillID)
        {
            ++g_Stats.utilityRefreshes;
        }
        else
        {
            ++g_Stats.utilityReplacements;

            g_Stats.utilityWastedMilliseconds +=
                previousRemainingMilliseconds;

            if (
                previousRemainingMilliseconds >
                g_Stats.worstUtilityWasteMilliseconds
                )
            {
                g_Stats.worstUtilityWasteMilliseconds =
                    previousRemainingMilliseconds;

                g_Stats.worstUtilityWasteSkillID =
                    g_LastUtilitySkillID;
            }
        }
    }

    g_LastUtilitySkillID =
        skillID;
}

void SessionTracker::RecordFoodExpired(
    bool inCombat
)
{
    if (!inCombat)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.foodExpiredInCombat;

    AddHistoryEvent(
        g_LastFoodSkillID,
        g_LastFoodSkillID,
        0,
        true,
        inCombat,
        SessionConsumableEventType::Expired
    );
}

void SessionTracker::RecordUtilityExpired(
    bool inCombat
)
{
    if (!inCombat)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ++g_Stats.utilityExpiredInCombat;

    AddHistoryEvent(
        g_LastUtilitySkillID,
        g_LastUtilitySkillID,
        0,
        false,
        inCombat,
        SessionConsumableEventType::Expired
    );
}

SessionStats SessionTracker::GetStats()
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    return g_Stats;
}

std::vector<SessionHistoryRecord>
SessionTracker::GetHistory()
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    return g_History;
}

void SessionTracker::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_SessionMutex
    );

    ArchiveCurrentSessionLocked();

    BeginFreshSessionLocked();
}
