#include "ConsumableData.h"

namespace
{
    const ConsumableInfo UnknownFood =
    {
        "Unknown",
        "Unknown Food",
        "This food effect has not been added to the Food Reminder database yet."
    };

    const ConsumableInfo UnknownUtility =
    {
        "Unknown",
        "Unknown Utility",
        "This utility effect has not been added to the Food Reminder database yet."
    };

    const ConsumableInfo SweetSpicyButternutSoup =
    {
        "Power / Ferocity",
        "Bowl of Sweet and Spicy Butternut Squash Soup",
        "+100 Power\n"
        "+70 Ferocity"
    };

    const ConsumableInfo SharpeningStone =
    {
        "Power",
        "Sharpening Stone",
        "Gain Power based on Precision\n"
        "Gain Power based on Ferocity\n"
        "+10% Experience from Kills"
    };
}

namespace ConsumableData
{
    const ConsumableInfo& GetFoodInfo(
        uint32_t skillID
    )
    {
        switch (skillID)
        {
        case 17825:
            return SweetSpicyButternutSoup;

        default:
            return UnknownFood;
        }
    }

    const ConsumableInfo& GetUtilityInfo(
        uint32_t skillID
    )
    {
        switch (skillID)
        {
        case 9963:
            return SharpeningStone;

        default:
            return UnknownUtility;
        }
    }
}