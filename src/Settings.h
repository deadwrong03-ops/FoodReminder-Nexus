#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct CharacterConsumableState
{
    int64_t foodExpiresAt = 0;
    int64_t utilityExpiresAt = 0;
};

struct FoodReminderSettings
{
    bool enabled = true;

    bool showTracker = false;

    int foodWarningSeconds = 300;
    int utilityWarningSeconds = 300;

    int metabolicPrimerWarningSeconds = 1800;
    int utilityPrimerWarningSeconds = 1800;

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