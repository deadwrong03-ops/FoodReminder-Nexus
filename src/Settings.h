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

    // Primers still use absolute expiration timestamps
    // for now.
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