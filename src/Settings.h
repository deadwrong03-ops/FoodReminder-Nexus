#pragma once

struct FoodReminderSettings
{
    bool enabled = true;
    int foodWarningSeconds = 300;
    int utilityWarningSeconds = 300;
};

extern FoodReminderSettings g_Settings;
