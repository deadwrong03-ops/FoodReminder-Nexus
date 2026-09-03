#pragma once

#include <cstdint>

namespace TrackerUI
{
    void Render(
        bool hasFood,
        int64_t foodRemaining,
        bool hasUtility,
        int64_t utilityRemaining,
        bool hasMetabolicPrimer,
        int64_t metabolicPrimerRemaining,
        bool hasUtilityPrimer,
        int64_t utilityPrimerRemaining
    );

    void SetColorTestMode(
        int mode
    );

    int GetColorTestMode();
}
