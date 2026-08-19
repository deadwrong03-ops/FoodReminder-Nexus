#pragma once

#include <cstdint>

namespace ReminderManager
{
    void TriggerTestReminder();

    void Update(
        bool hasFood,
        int64_t foodRemainingMilliseconds,
        int foodWarningSeconds,
        bool hasUtility,
        int64_t utilityRemainingMilliseconds,
        int utilityWarningSeconds
    );

    bool IsReminderActive();

    const char* GetReminderTitle();
    const char* GetReminderMessage();

    float GetReminderSecondsRemaining();
}