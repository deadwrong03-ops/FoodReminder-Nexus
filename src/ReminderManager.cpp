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
    ShowReminder(
        "FOOD REMINDER",
        "Test reminder is working."
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

    // If the buff disappears, allow the next buff
    // to generate a fresh warning.
    if (!hasFood)
    {
        g_FoodWarningSent = false;
    }

    if (!hasUtility)
    {
        g_UtilityWarningSent = false;
    }

    // If a buff gets refreshed above its warning
    // threshold, re-arm the warning.
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

    // If both happen together, show one clean
    // combined reminder instead of overlapping alerts.
    if (foodWarningDue && utilityWarningDue)
    {
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
        ShowReminder(
            "FOOD REMINDER",
            "Your food is expiring soon."
        );

        g_FoodWarningSent = true;
    }

    if (utilityWarningDue)
    {
        ShowReminder(
            "UTILITY REMINDER",
            "Your utility is expiring soon."
        );

        g_UtilityWarningSent = true;
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