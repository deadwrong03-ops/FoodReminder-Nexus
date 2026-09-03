#pragma once

#include <cstdint>

namespace ReminderUI
{
    void Render(
        bool hasFood,
        int64_t foodRemaining,
        bool hasUtility,
        int64_t utilityRemaining
    );
}
