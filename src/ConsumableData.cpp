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

    //
    // FOOD - POWER
    //
    const ConsumableInfo SweetSpicyButternutSoup =
    {
        "Power",
        "Bowl of Sweet and Spicy Butternut Squash Soup",
        "+100 Power\n"
        "+70 Ferocity"
    };

    const ConsumableInfo SpicyMoaWings =
    {
        "Power",
        "Plate of Spicy Moa Wings",
        "+100 Power\n"
        "+70 Ferocity"
    };

    //
    // FOOD - PRECISION
    //
    const ConsumableInfo CurryButternutSoup =
    {
        "Prec",
        "Bowl of Curry Butternut Squash Soup",
        "+100 Precision\n"
        "+70 Power"
    };

    const ConsumableInfo TruffleSteak =
    {
        "Prec",
        "Plate of Truffle Steak",
        "+100 Power\n"
        "+70 Precision"
    };

    const ConsumableInfo WinterberrySteak =
    {
        "Prec",
        "Steak with Winterberry Sauce",
        "+100 Power\n"
        "+70 Precision"
    };

    //
    // FOOD - CONDITION DAMAGE
    //
    const ConsumableInfo BeefRendang =
    {
        "Condi",
        "Plate of Beef Rendang",
        "+100 Condition Damage\n"
        "+70 Expertise"
    };

    //
    // FOOD - EXPERTISE
    //
    const ConsumableInfo RareVeggiePizza =
    {
        "Exper",
        "Rare Veggie Pizza",
        "+100 Expertise\n"
        "+70 Condition Damage"
    };

    const ConsumableInfo RedLentilSaobosa =
    {
        "Exper",
        "Red Lentil Saobosa",
        "+100 Expertise\n"
        "+70 Condition Damage"
    };

    //
    // UTILITY - POWER
    //
    const ConsumableInfo SuperiorSharpeningStone =
    {
        "Power",
        "Superior Sharpening Stone",
        "3% Power from Precision\n"
        "6% Power from Ferocity"
    };

    const ConsumableInfo TinOfFruitcake =
    {
        "Power",
        "Tin of Fruitcake",
        "3% Power from Precision\n"
        "6% Power from Ferocity"
    };

    const ConsumableInfo FuriousSharpeningStone =
    {
        "Power",
        "Furious Sharpening Stone",
        "3% Power from Precision\n"
        "3% Ferocity from Precision"
    };

    //
    // UTILITY - CONDITION DAMAGE
    //
    const ConsumableInfo ToxicFocusingCrystal =
    {
        "Condi",
        "Toxic Focusing Crystal",
        "3% Condition Damage from Power\n"
        "3% Condition Damage from Precision"
    };

    const ConsumableInfo MasterTuningCrystal =
    {
        "Condi",
        "Master Tuning Crystal",
        "3% Condition Damage from Precision\n"
        "8% Condition Damage from Expertise"
    };

    const ConsumableInfo TuningIcicle =
    {
        "Condi",
        "Tuning Icicle",
        "3% Condition Damage from Precision\n"
        "8% Condition Damage from Expertise"
    };

    const ConsumableInfo FuriousTuningCrystal =
    {
        "Exper",
        "Furious Tuning Crystal",
        "3% Condition Damage from Precision\n"
        "3% Expertise from Precision"
    };

    //
    // UTILITY - CONCENTRATION
    //
    const ConsumableInfo PotentLucentOil =
    {
        "PConc",
        "Potent Lucent Oil",
        "3% Concentration from Power\n"
        "3% Concentration from Precision"
    };

    const ConsumableInfo EnhancedLucentOil =
    {
        "PConc",
        "Enhanced Lucent Oil",
        "6% Concentration from Condition Damage\n"
        "3% Concentration from Precision"
    };

    const ConsumableInfo ToxicMaintenanceOil =
    {
        "CConc",
        "Toxic Maintenance Oil",
        "3% Concentration from Power\n"
        "6% Concentration from Condition Damage"
    };

    const ConsumableInfo MasterMaintenanceOil =
    {
        "HConc",
        "Master Maintenance Oil",
        "3% Concentration from Precision\n"
        "6% Concentration from Healing Power"
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
            //
            // Power
            //
        case 17825:
            return SweetSpicyButternutSoup;

        case 57883:
            return SpicyMoaWings;

            //
            // Precision
            //
        case 9829:
            return CurryButternutSoup;

        case 9769:
            return TruffleSteak;

        case 37540:
            return WinterberrySteak;

            //
            // Condition Damage
            //
        case 49686:
            return BeefRendang;

            //
            // Expertise
            //
        case 10009:
            return RareVeggiePizza;

        case 46273:
            return RedLentilSaobosa;

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
            //
            // Power
            //
        case 9963:
            return SuperiorSharpeningStone;

        case 34211:
            return TinOfFruitcake;

        case 25882:
            return FuriousSharpeningStone;

            //
            // Condition Damage / Expertise
            //
        case 21828:
            return ToxicFocusingCrystal;

        case 9967:
            return MasterTuningCrystal;

        case 34206:
            return TuningIcicle;

        case 25878:
            return FuriousTuningCrystal;

            //
            // Concentration
            //
        case 53374:
            return PotentLucentOil;

        case 53304:
            return EnhancedLucentOil;

        case 21827:
            return ToxicMaintenanceOil;

        case 9968:
            return MasterMaintenanceOil;

        default:
            return UnknownUtility;
        }
    }
}