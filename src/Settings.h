#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct CharacterConsumableState
{
    // Saved remaining duration for this character.
    // These timers pause while the character is not loaded.
    // 0 means no saved active buff.
    int64_t foodRemainingSeconds = 0;
    int64_t utilityRemainingSeconds = 0;

    // Whether Food/Utility state is actually known.
    // false means the addon has not yet confirmed Active or Missing.
    bool foodStateKnown = false;
    bool utilityStateKnown = false;

    // Primer timers are character-specific and pause
    // while the character is not loaded.
    int64_t metabolicPrimerRemainingSeconds = 0;
    int64_t utilityPrimerRemainingSeconds = 0;

    // Whether Primer state is actually known for this character.
    // false means ArcDPS has not yet confirmed Active or Missing.
    bool metabolicPrimerStateKnown = false;
    bool utilityPrimerStateKnown = false;

    // ArcDPS effect IDs for the currently
    // active Food and Utility effects.
    uint32_t foodSkillID = 0;
    uint32_t utilitySkillID = 0;
};

struct SavedUnknownConsumable
{
    uint32_t skillID = 0;

    bool isFood = false;
    bool isUtility = false;

    uint64_t seenCount = 0;
};

struct FoodReminderSettings
{
    bool enabled = true;

    bool showTracker = false;
    bool lockTrackerPosition = false;
    bool lockReminderPosition = false;

    int foodWarningSeconds = 300;
    int utilityWarningSeconds = 300;

    int metabolicPrimerWarningSeconds = 1800;
    int utilityPrimerWarningSeconds = 1800;

    // Trading Post trend window:
    // 0 = 15m, 1 = 30m, 2 = 1h, 3 = 6h, 4 = 24h.
    int tradingPostTrendWindowIndex = 1;

    // Legacy global Primer timestamps retained only for settings-file
    // compatibility. New code does not restore Primers from these values.
    int64_t metabolicPrimerExpiresAt = 0;
    int64_t utilityPrimerExpiresAt = 0;

    std::unordered_map<std::string, CharacterConsumableState>
        characterConsumables;

    std::unordered_map<uint64_t, SavedUnknownConsumable>
        unknownConsumables;
};

extern FoodReminderSettings g_Settings;

namespace Settings
{
    bool Load(void* moduleHandle);
    bool Save(void* moduleHandle);
}
