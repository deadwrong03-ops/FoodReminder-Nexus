#include "ReminderManager.h"

#include <chrono>

namespace
{
    using Clock = std::chrono::steady_clock;
    Clock::time_point g_TestReminderEnds{};
}

void ReminderManager::TriggerTestReminder()
{
    g_TestReminderEnds = Clock::now() + std::chrono::seconds(5);
}

bool ReminderManager::IsTestReminderActive()
{
    return Clock::now() < g_TestReminderEnds;
}

float ReminderManager::GetTestReminderSecondsRemaining()
{
    if (!IsTestReminderActive())
    {
        return 0.0f;
    }

    const auto remaining = g_TestReminderEnds - Clock::now();
    return std::chrono::duration<float>(remaining).count();
}
