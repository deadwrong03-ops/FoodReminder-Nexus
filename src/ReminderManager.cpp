#include "ReminderManager.h"

#include <chrono>
#include <string>

namespace
{
    using Clock = std::chrono::steady_clock;

    Clock::time_point g_ReminderEnds{};

    std::string g_ReminderTitle = "FOOD REMINDER";
    std::string g_ReminderMessage =
        "Your food or utility is expiring soon.";

    bool g_FoodWarningSent = false;
    bool g_UtilityWarningSent = false;

    bool g_MetabolicPrimerWarningSent = false;
    bool g_UtilityPrimerWarningSent = false;

    constexpr int64_t TEST_PRIMER_WARNING_MILLISECONDS =
        30LL * 60LL * 1000LL;

    int64_t g_BuffRemainingMilliseconds = 0;

    constexpr int REMINDER_DISPLAY_SECONDS = 5;

    void ShowReminder(
        const char* title,
        const char* message
    )
    {
        g_ReminderTitle = title;
        g_ReminderMessage = message;

        g_ReminderEnds =
            Clock::now() +
            std::chrono::seconds(
                REMINDER_DISPLAY_SECONDS
            );
    }
}

void ReminderManager::TriggerTestReminder()
{
    g_BuffRemainingMilliseconds = 0;

    ShowReminder(
        "FOOD REMINDER",
        "Test reminder is working."
    );
}

void ReminderManager::TriggerMetabolicPrimerTest()
{
    g_BuffRemainingMilliseconds =
        TEST_PRIMER_WARNING_MILLISECONDS;

    ShowReminder(
        "METABOLIC PRIMER EXPIRING",
        "Your Metabolic Primer is expiring soon."
    );
}

void ReminderManager::TriggerUtilityPrimerTest()
{
    g_BuffRemainingMilliseconds =
        TEST_PRIMER_WARNING_MILLISECONDS;

    ShowReminder(
        "UTILITY PRIMER EXPIRING",
        "Your Utility Primer is expiring soon."
    );
}

void ReminderManager::TriggerBothPrimerTest()
{
    g_BuffRemainingMilliseconds =
        TEST_PRIMER_WARNING_MILLISECONDS;

    ShowReminder(
        "PRIMERS EXPIRING",
        "Your Metabolic and Utility Primers are expiring soon."
    );
}

void ReminderManager::Update(
    bool hasFood,
    int64_t foodRemainingMilliseconds,
    int foodWarningSeconds,
    bool hasUtility,
    int64_t utilityRemainingMilliseconds,
    int utilityWarningSeconds
)
{
    const int64_t foodWarningMilliseconds =
        static_cast<int64_t>(foodWarningSeconds) * 1000;

    const int64_t utilityWarningMilliseconds =
        static_cast<int64_t>(utilityWarningSeconds) * 1000;

    if (!hasFood)
    {
        g_FoodWarningSent = false;
    }

    if (!hasUtility)
    {
        g_UtilityWarningSent = false;
    }

    if (hasFood &&
        foodRemainingMilliseconds >
        foodWarningMilliseconds)
    {
        g_FoodWarningSent = false;
    }

    if (hasUtility &&
        utilityRemainingMilliseconds >
        utilityWarningMilliseconds)
    {
        g_UtilityWarningSent = false;
    }

    const bool foodWarningDue =
        hasFood &&
        foodRemainingMilliseconds > 0 &&
        foodRemainingMilliseconds <=
        foodWarningMilliseconds &&
        !g_FoodWarningSent;

    const bool utilityWarningDue =
        hasUtility &&
        utilityRemainingMilliseconds > 0 &&
        utilityRemainingMilliseconds <=
        utilityWarningMilliseconds &&
        !g_UtilityWarningSent;

    if (foodWarningDue && utilityWarningDue)
    {
        g_BuffRemainingMilliseconds =
            foodRemainingMilliseconds <
            utilityRemainingMilliseconds
            ? foodRemainingMilliseconds
            : utilityRemainingMilliseconds;

        ShowReminder(
            "FOOD + UTILITY REMINDER",
            "Your food and utility are expiring soon."
        );

        g_FoodWarningSent = true;
        g_UtilityWarningSent = true;

        return;
    }

    if (foodWarningDue)
    {
        g_BuffRemainingMilliseconds =
            foodRemainingMilliseconds;

        ShowReminder(
            "FOOD REMINDER",
            "Your food is expiring soon."
        );

        g_FoodWarningSent = true;
    }

    if (utilityWarningDue)
    {
        g_BuffRemainingMilliseconds =
            utilityRemainingMilliseconds;

        ShowReminder(
            "UTILITY REMINDER",
            "Your utility is expiring soon."
        );

        g_UtilityWarningSent = true;
    }
}

void ReminderManager::UpdatePrimerWarnings(
    bool hasMetabolicPrimer,
    int64_t metabolicPrimerRemainingMilliseconds,
    int metabolicPrimerWarningSeconds,
    bool hasUtilityPrimer,
    int64_t utilityPrimerRemainingMilliseconds,
    int utilityPrimerWarningSeconds
)
{
    const int64_t metabolicPrimerWarningMilliseconds =
        static_cast<int64_t>(
            metabolicPrimerWarningSeconds
            ) * 1000;

    const int64_t utilityPrimerWarningMilliseconds =
        static_cast<int64_t>(
            utilityPrimerWarningSeconds
            ) * 1000;

    if (!hasMetabolicPrimer)
    {
        g_MetabolicPrimerWarningSent = false;
    }

    if (!hasUtilityPrimer)
    {
        g_UtilityPrimerWarningSent = false;
    }

    if (hasMetabolicPrimer &&
        metabolicPrimerRemainingMilliseconds >
        metabolicPrimerWarningMilliseconds)
    {
        g_MetabolicPrimerWarningSent = false;
    }

    if (hasUtilityPrimer &&
        utilityPrimerRemainingMilliseconds >
        utilityPrimerWarningMilliseconds)
    {
        g_UtilityPrimerWarningSent = false;
    }

    const bool metabolicWarningDue =
        hasMetabolicPrimer &&
        metabolicPrimerRemainingMilliseconds > 0 &&
        metabolicPrimerRemainingMilliseconds <=
        metabolicPrimerWarningMilliseconds &&
        !g_MetabolicPrimerWarningSent;

    const bool utilityWarningDue =
        hasUtilityPrimer &&
        utilityPrimerRemainingMilliseconds > 0 &&
        utilityPrimerRemainingMilliseconds <=
        utilityPrimerWarningMilliseconds &&
        !g_UtilityPrimerWarningSent;

    if (metabolicWarningDue && utilityWarningDue)
    {
        g_BuffRemainingMilliseconds =
            metabolicPrimerRemainingMilliseconds <
            utilityPrimerRemainingMilliseconds
            ? metabolicPrimerRemainingMilliseconds
            : utilityPrimerRemainingMilliseconds;

        ShowReminder(
            "PRIMERS EXPIRING",
            "Your Metabolic and Utility Primers are expiring soon."
        );

        g_MetabolicPrimerWarningSent = true;
        g_UtilityPrimerWarningSent = true;

        return;
    }

    if (metabolicWarningDue)
    {
        g_BuffRemainingMilliseconds =
            metabolicPrimerRemainingMilliseconds;

        ShowReminder(
            "METABOLIC PRIMER EXPIRING",
            "Your Metabolic Primer is expiring soon."
        );

        g_MetabolicPrimerWarningSent = true;
    }

    if (utilityWarningDue)
    {
        g_BuffRemainingMilliseconds =
            utilityPrimerRemainingMilliseconds;

        ShowReminder(
            "UTILITY PRIMER EXPIRING",
            "Your Utility Primer is expiring soon."
        );

        g_UtilityPrimerWarningSent = true;
    }
}

bool ReminderManager::IsReminderActive()
{
    return Clock::now() < g_ReminderEnds;
}

const char* ReminderManager::GetReminderTitle()
{
    return g_ReminderTitle.c_str();
}

const char* ReminderManager::GetReminderMessage()
{
    return g_ReminderMessage.c_str();
}

int64_t ReminderManager::GetBuffRemainingMilliseconds()
{
    return g_BuffRemainingMilliseconds;
}

float ReminderManager::GetReminderSecondsRemaining()
{
    if (!IsReminderActive())
    {
        return 0.0f;
    }

    const auto remaining =
        g_ReminderEnds - Clock::now();

    return std::chrono::duration<float>(
        remaining
    ).count();
}