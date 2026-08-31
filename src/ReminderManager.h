#pragma once

#include <cstdint>

namespace ReminderManager
{
    void TriggerTestReminder();

    void TriggerMetabolicPrimerTest();
    void TriggerUtilityPrimerTest();
    void TriggerBothPrimerTest();

    void Update(
        bool hasFood,
        int64_t foodRemainingMilliseconds,
        int foodWarningSeconds,
        bool hasUtility,
        int64_t utilityRemainingMilliseconds,
        int utilityWarningSeconds
    );

    void UpdatePrimerWarnings(
        bool hasMetabolicPrimer,
        int64_t metabolicPrimerRemainingMilliseconds,
        int metabolicPrimerWarningSeconds,
        bool hasUtilityPrimer,
        int64_t utilityPrimerRemainingMilliseconds,
        int utilityPrimerWarningSeconds
    );

    void UpdateMissingBuffWarnings(
        bool inCombat,
        bool hasFood,
        bool hasUtility
    );

    bool IsReminderActive();

    const char* GetReminderTitle();
    const char* GetReminderMessage();

    int64_t GetBuffRemainingMilliseconds();

    float GetReminderSecondsRemaining();
}
