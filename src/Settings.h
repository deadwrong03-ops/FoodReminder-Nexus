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
};

struct FoodReminderSettings
{
    bool enabled = true;

    bool showTracker = false;

    int foodWarningSeconds = 300;
    int utilityWarningSeconds = 300;

    int metabolicPrimerWarningSeconds = 1800;
    int utilityPrimerWarningSeconds = 1800;

    // Primers still use absolute expiration timestamps
    // for now. We are only correcting Food/Utility
    // character persistence in this change.
    int64_t metabolicPrimerExpiresAt = 0;
    int64_t utilityPrimerExpiresAt = 0;

    std::unordered_map<std::string, CharacterConsumableState>
        characterConsumables;
};

extern FoodReminderSettings g_Settings;

namespace Settings
{
    bool Load(void* moduleHandle);
    bool Save(void* moduleHandle);
}