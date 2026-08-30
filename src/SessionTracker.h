#pragma once

#include <cstdint>
#include <vector>

struct SessionConsumableUsage
{
    uint32_t skillID = 0;
    uint32_t uses = 0;
};

struct SessionPrimerProtectedUsage
{
    uint32_t skillID = 0;
    int64_t protectedMilliseconds = 0;
};

enum class SessionPrimerState
{
    Unknown,
    Inactive,
    InferredActive,
    ConfirmedActive
};

enum class SessionConsumableEventType
{
    Applied,
    Refreshed,
    Replaced,
    Expired
};

struct SessionConsumableEvent
{
    int64_t sessionMilliseconds = 0;

    uint32_t skillID = 0;
    uint32_t previousSkillID = 0;
    int64_t previousRemainingMilliseconds = 0;

    bool isFood = false;
    bool inCombat = false;

    SessionConsumableEventType type =
        SessionConsumableEventType::Applied;
};

struct SessionStats
{
    int64_t sessionMilliseconds = 0;
    int64_t combatMilliseconds = 0;

    int64_t foodActiveMilliseconds = 0;
    int64_t utilityActiveMilliseconds = 0;

    int64_t foodCombatMilliseconds = 0;
    int64_t utilityCombatMilliseconds = 0;

    uint32_t foodApplications = 0;
    uint32_t foodRefreshes = 0;
    uint32_t foodReplacements = 0;
    uint32_t foodExpiredInCombat = 0;
    int64_t foodWastedMilliseconds = 0;
    int64_t worstFoodWasteMilliseconds = 0;
    uint32_t worstFoodWasteSkillID = 0;

    uint32_t utilityApplications = 0;
    uint32_t utilityRefreshes = 0;
    uint32_t utilityReplacements = 0;
    uint32_t utilityExpiredInCombat = 0;
    int64_t utilityWastedMilliseconds = 0;
    int64_t worstUtilityWasteMilliseconds = 0;
    uint32_t worstUtilityWasteSkillID = 0;

    //
    // Primer session state.
    //
    // ArcDPS does not reliably resend active Primer state after login
    // or character switch, so SessionTracker keeps confirmed, inferred,
    // and unknown time separate instead of treating every false bool as
    // "Primer missing".
    //
    int64_t metabolicPrimerConfirmedMilliseconds = 0;
    int64_t metabolicPrimerInferredMilliseconds = 0;
    int64_t metabolicPrimerUnknownMilliseconds = 0;

    int64_t utilityPrimerConfirmedMilliseconds = 0;
    int64_t utilityPrimerInferredMilliseconds = 0;
    int64_t utilityPrimerUnknownMilliseconds = 0;

    //
    // Kept as total known-active Primer time for compatibility with
    // existing session calculations. This equals confirmed + inferred.
    //
    int64_t metabolicPrimerActiveMilliseconds = 0;
    int64_t utilityPrimerActiveMilliseconds = 0;

    uint32_t estimatedFoodUsesSaved = 0;
    uint32_t estimatedUtilityUsesSaved = 0;

    //
    // Aggregate protection contains both confirmed and inferred Primer
    // protection. Inferred protection is also tracked separately so the
    // UI can be transparent about how much data was inferred.
    //
    std::vector<SessionPrimerProtectedUsage>
        foodPrimerProtection;

    std::vector<SessionPrimerProtectedUsage>
        utilityPrimerProtection;

    std::vector<SessionPrimerProtectedUsage>
        foodPrimerInferredProtection;

    std::vector<SessionPrimerProtectedUsage>
        utilityPrimerInferredProtection;

    std::vector<SessionConsumableUsage>
        foodUsage;

    std::vector<SessionConsumableUsage>
        utilityUsage;

    std::vector<SessionConsumableEvent>
        consumableHistory;
};

struct SessionHistoryRecord
{
    uint64_t startedUnixSeconds = 0;
    uint64_t endedUnixSeconds = 0;

    SessionStats stats;
};

namespace SessionTracker
{
    //
    // Initializes persistent session-history storage.
    // History is stored beside the addon DLL in:
    // FoodReminder_SessionHistory.tsv
    //
    void Start(
        void* moduleHandle
    );

    //
    // Archives the active session, if it contains meaningful elapsed
    // gameplay time, and writes persistent history to disk.
    //
    void Shutdown();

    void Update(
        bool hasFood,
        uint32_t foodSkillID,
        bool hasUtility,
        uint32_t utilitySkillID,
        SessionPrimerState metabolicPrimerState,
        SessionPrimerState utilityPrimerState,
        bool inCombat
    );

    void RecordFoodApplication(
        uint32_t skillID,
        int64_t previousRemainingMilliseconds
    );

    void RecordUtilityApplication(
        uint32_t skillID,
        int64_t previousRemainingMilliseconds
    );

    void RecordFoodExpired(
        bool inCombat
    );

    void RecordUtilityExpired(
        bool inCombat
    );

    SessionStats GetStats();

    //
    // Returns completed sessions loaded from / written to local history.
    // Records are ordered oldest to newest.
    //
    std::vector<SessionHistoryRecord>
        GetHistory();

    //
    // Archives the current session first, then starts a fresh session.
    //
    void Reset();
}
