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
        "+70 Ferocity",
        41569
    };

    const ConsumableInfo SpicyMoaWings =
    {
        "Power",
        "Plate of Spicy Moa Wings",
        "+100 Power\n"
        "+70 Ferocity",
        91917
    };

    const ConsumableInfo OrrianSteakFrittes =
    {
        "Power",
        "Plate of Orrian Steak Frittes / Crunchy Grasshopper",
        "+100 Power\n"
        "+70 Vitality"
    };

    //
    // FOOD - PRECISION
    //
    const ConsumableInfo CurryButternutSoup =
    {
        "Prec",
        "Bowl of Curry Butternut Squash Soup",
        "+100 Precision\n"
        "+70 Power",
        12486
    };

    const ConsumableInfo TruffleSteak =
    {
        "Prec",
        "Plate of Truffle Steak",
        "+100 Power\n"
        "+70 Precision",
        12467
    };

    const ConsumableInfo WinterberrySteak =
    {
        "Prec",
        "Steak with Winterberry Sauce",
        "+100 Power\n"
        "+70 Precision",
        79786
    };

    const ConsumableInfo SesameCoqAuVin =
    {
        "Prec",
        "Plate of Sesame-Crusted Coq Au Vin",
        "Health every second\n"
        "+100 Power\n"
        "+70 Precision",
        9219
    };

    //
    // FOOD - CONDITION DAMAGE
    //
    const ConsumableInfo BeefRendang =
    {
        "Condi",
        "Plate of Beef Rendang",
        "+100 Condition Damage\n"
        "+70 Expertise",
        86997
    };

    const ConsumableInfo SpicyPumpkinCookie =
    {
        "Condi",
        "Spicy Pumpkin Cookie",
        "+70 Condition Damage\n"
        "+30% Magic Find\n"
        "+15% Experience from Kills",
        36084
    };

    //
    // FOOD - EXPERTISE
    //
    const ConsumableInfo RareVeggiePizza =
    {
        "Exper",
        "Rare Veggie Pizza",
        "+100 Expertise\n"
        "+70 Condition Damage",
        12465
    };

    const ConsumableInfo RedLentilSaobosa =
    {
        "Exper",
        "Red Lentil Saobosa",
        "+100 Expertise\n"
        "+70 Condition Damage",
        81079
    };

    //
    // FOOD - CONCENTRATION
    //
    const ConsumableInfo BeefCarpaccioSalsa =
    {
        "PConc",
        "Beef Carpaccio with Salsa Garnish",
        "66% Life Steal Chance\n"
        "+100 Concentration\n"
        "+70 Power",
        91862
    };

    //
    // FOOD - ALL ATTRIBUTES
    //
    const ConsumableInfo BirthdayCake =
    {
        "All",
        "Birthday Cake / Cake Shot",
        "+40 All Attributes\n"
        "+10% Karma\n"
        "+15% Magic Find\n"
        "+15% Experience from Kills"
    };

    //
    // FOOD - MAGIC FIND
    //
    const ConsumableInfo PeppermintOmnomberryBar =
    {
        "MF",
        "Peppermint Omnomberry Bar",
        "+30% Magic Find\n"
        "+40% Gold from Monsters\n"
        "+10% Karma\n"
        "+10% Experience from Kills",
        36041
    };

    //
    // FOOD - MOVEMENT
    //
    const ConsumableInfo WinterberrySeaweedSalad =
    {
        "Move",
        "Bowl of Winterberry Seaweed Salad",
        "60% Chance to Gain Swiftness on Kill\n"
        "+5% Damage While Moving\n"
        "+30% Magic Find\n"
        "+10% Experience from Kills",
        79793
    };

    //
    // FOOD - ON KILL
    //
    const ConsumableInfo PowerOnKillFood =
    {
        "Kill",
        "Dragon's Breath Bun / Carrot Souffle",
        "+200 Power for 30 Seconds on Kill\n"
        "+70 Ferocity\n"
        "+10% Experience from Kills"
    };

    //
    // FOOD - ASCENDED POWER
    //
    const ConsumableInfo CilantroLimeSousVideSteak =
    {
        "Power",
        "Cilantro Lime Sous-Vide Steak",
        "66% Life Steal Chance\n"
        "+100 Power\n"
        "+70 Ferocity",
        91805
    };

    const ConsumableInfo PeppercornSousVideSteak =
    {
        "Power",
        "Peppercorn-Crusted Sous-Vide Steak",
        "-10% Incoming Damage\n"
        "+100 Power\n"
        "+70 Ferocity",
        91734
    };

    //
    // FOOD - CONCENTRATION
    //
    const ConsumableInfo SoulPastry =
    {
        "PConc",
        "Soul Pastry",
        "+100 Concentration\n"
        "+70 Power",
        89002
    };

    //
    // FOOD - ON KILL
    //
    const ConsumableInfo BlockOfTofu =
    {
        "Kill",
        "Block of Tofu",
        "+100 Power & Ferocity on Kill\n"
        "+70 Precision",
        96793
    };

    //
    // FOOD - ALL ATTRIBUTES
    //
    const ConsumableInfo PeppercornOysterSoup =
    {
        "All",
        "Spherified Peppercorn-Spiced Oyster Soup",
        "-10% Incoming Damage\n"
        "+45 All Attributes"
    };

    const ConsumableInfo CilantroCuredMeatFlatbread =
    {
        "Condi",
        "Cilantro and Cured Meat Flatbread",
        "66% Life Steal Chance\n"
        "+100 Condition Damage\n"
        "+70 Expertise"
    };

    const ConsumableInfo PoultryAspicSalsa =
    {
        "Condi",
        "Plate of Poultry Aspic with Salsa Garnish",
        "+100 Condition Damage\n"
        "+70 Expertise"
    };
    const ConsumableInfo GuildBanquetNourishment =
    {
        "All",
        "Guild Banquet Table Nourishment",
        "+2 All Attributes"
    };

    const ConsumableInfo ChocolateOmnomberryCream =
    {
        "Boon",
        "Chocolate Omnomberry Cream",
        "+40% Magic Find while under a Boon\n"
        "+20% Boon Duration\n"
        "+10% Experience from Kills",
        12453
    };

    const ConsumableInfo OmnomberryBar =
    {
        "Gold",
        "Omnomberry Bar",
        "+40% Gold from Monsters\n"
        "+10% Magic Find\n"
        "+10% Experience from Kills",
        12452
    };
    const ConsumableInfo GrumbleCake =
    {
        "Food",
        "Grumble Cake",
        "+20 Vitality\n"
        "+10% Experience from Kills"
    };
    const ConsumableInfo ChiliPepperPopper =
    {
        "Might",
        "Chili Pepper Popper",
        "8% Chance to Gain Might on Critical Hit during the Day\n"
        "+10% Experience from Kills"
    };
    const ConsumableInfo PepperedCuredMeatFlatbread =
    {
        "Condi",
        "Peppered Cured Meat Flatbread",
        "Food effect"
    };

    const ConsumableInfo PeppercornSpicedCoqAuVin =
    {
        "Prec",
        "Plate of Peppercorn-Spiced Coq Au Vin",
        "Food effect"
    };

    const ConsumableInfo RaspberryPeachBar =
    {
        "Food",
        "Raspberry Peach Bar",
        "Food effect"
    };

    const ConsumableInfo MushroomCloveSousVideSteak =
    {
        "Food",
        "Mushroom Clove Sous-Vide Steak",
        "Food effect"
    };

    const ConsumableInfo FruitSaladMintGarnish =
    {
        "Food",
        "Bowl of Fruit Salad with Mint Garnish",
        "Food effect"
    };
    const ConsumableInfo OmnomberryTart =
    {
        "MF",
        "Omnomberry Tart",
        "+30% Magic Find\n"
        "+70 Power\n"
        "+10% Experience from Kills"
    };
    const ConsumableInfo BeefCarpaccioMintGarnish =
    {
        "Power",
        "Plate of Beef Carpaccio with Mint Garnish",
        "Food effect"
    };
    const ConsumableInfo SoySesameSousVideSteak =
    {
        "Power",
        "Soy-Sesame Sous-Vide Steak",
        "Food effect"
    };
    const ConsumableInfo CoqAuVinSalsa =
    {
        "Food",
        "Plate of Coq Au Vin with Salsa",
        "Nourishment"
    };

    const ConsumableInfo CloveVeggieFlatbread =
    {
        "Food",
        "Clove and Veggie Flatbread",
        "Nourishment"
    };

    const ConsumableInfo SpherifiedCilantroOysterSoup =
    {
        "Food",
        "Spherified Cilantro Oyster Soup",
        "Nourishment"
    };

    const ConsumableInfo PeppercornVeggieFlatbread =
    {
        "Food",
        "Peppercorn and Veggie Flatbread",
        "Nourishment"
    };
    const ConsumableInfo MushroomPizza =
    {
        "Food",
        "Mushroom Pizza",
        "Nourishment"
    };

    const ConsumableInfo DecadeDesserts =
    {
        "All",
        "Tray of Decade Desserts",
        "Nourishment"
    };
    const ConsumableInfo SharedCondiFood9822 =
    {
        "Condi",
        "Loaf of Zucchini Bread / Red Bean Cake",
        "Shared Nourishment effect"
    };

    const ConsumableInfo MangoPie =
    {
        "Food",
        "Mango Pie",
        "Nourishment"
    };

    const ConsumableInfo SesameAsparagusCuredMeatFlatbread =
    {
        "Condi",
        "Sesame-Asparagus and Cured Meat Flatbread",
        "Gain Health Every Second\n"
        "+100 Condition Damage\n"
        "+70 Expertise",
        91867
    };

    const ConsumableInfo ChefsTastingPlatter =
    {
        "Kill",
        "Chef's Tasting Platter",
        "+80 Power for 30 Seconds on Kill\n"
        "+50 Precision\n"
        "+50 Condition Damage\n"
        "+30% Magic Find\n"
        "+10% Experience from Kills",
        91689
    };

    const ConsumableInfo SharedFood9994 =
    {
        "Food",
        "Shared Food Effect",
        "Multiple foods share this nourishment effect"
    };

    //
    // UTILITY - POWER
    //

    //
    // UTILITY - POWER
    //
    const ConsumableInfo SuperiorSharpeningStone =
    {
        "Power",
        "Superior Sharpening Stone",
        "3% Power from Precision\n"
        "6% Power from Ferocity",
        78305
    };
    const ConsumableInfo HardenedSharpeningStone =
    {
        "Power",
        "Hardened Sharpening Stone",
        "2% Power from Precision\n"
        "4% Power from Ferocity",
        9440
    };
    const ConsumableInfo RoughTinySharpeningStone =
    {
        "Power",
        "Rough / Tiny Sharpening Stone",
        "Gain Power from Precision\n"
        "+10% Experience from Kills"
    };
    const ConsumableInfo SharpeningGolem =
    {
        "Power",
        "Sharpening Golem",
        "Utility effect"
    };

    const ConsumableInfo TinOfFruitcake =
    {
        "Power",
        "Tin of Fruitcake",
        "3% Power from Precision\n"
        "6% Power from Ferocity",
        77569
    };

    const ConsumableInfo FuriousSharpeningStone =
    {
        "Power",
        "Furious Sharpening Stone",
        "3% Power from Precision\n"
        "3% Ferocity from Precision"
    };
    const ConsumableInfo CorsairSharpeningStone =
    {
        "Power",
        "Corsair Sharpening Stone",
        "3% Power from Toughness\n"
        "3% Expertise from Toughness\n"
        "+10% Experience from Kills",
        86378
    };

    //
    // UTILITY - CONDITION DAMAGE
    //
    const ConsumableInfo ToxicFocusingCrystal =
    {
        "Condi",
        "Toxic Focusing Crystal",
        "3% Condition Damage from Power\n"
        "3% Condition Damage from Precision",
        48917
    };

    const ConsumableInfo MasterTuningCrystal =
    {
        "Condi",
        "Master Tuning Crystal",
        "3% Condition Damage from Precision\n"
        "8% Condition Damage from Expertise",
        9476
    };
    const ConsumableInfo ApprenticeTuningCrystal =
    {
        "Condi",
        "Apprentice Tuning Crystal",
        "1% Condition Damage from Precision\n"
        "+10% Experience from Kills",
        9464
    };
    const ConsumableInfo TuningIcicle =
    {
        "Condi",
        "Tuning Icicle",
        "3% Condition Damage from Precision\n"
        "8% Condition Damage from Expertise",
        77567
    };

    const ConsumableInfo FuriousTuningCrystal =
    {
        "Exper",
        "Furious Tuning Crystal",
        "3% Condition Damage from Precision\n"
        "3% Expertise from Precision",
        67524
    };
    const ConsumableInfo BountifulMaintenanceOil =
    {
        "Condi",
        "Bountiful Maintenance Oil",
        "Condition-oriented utility",
        67528
    };

    //
    // UTILITY - CONCENTRATION
    //
    const ConsumableInfo PotentLucentOil =
    {
        "PConc",
        "Potent Lucent Oil",
        "3% Concentration from Power\n"
        "3% Concentration from Precision",
        89203
    };

    const ConsumableInfo EnhancedLucentOil =
    {
        "PConc",
        "Enhanced Lucent Oil",
        "6% Concentration from Condition Damage\n"
        "3% Concentration from Precision",
        89157
    };

    const ConsumableInfo ToxicMaintenanceOil =
    {
        "CConc",
        "Toxic Maintenance Oil",
        "3% Concentration from Power\n"
        "6% Concentration from Condition Damage",
        48916
    };

    const ConsumableInfo MasterMaintenanceOil =
    {
        "HConc",
        "Master Maintenance Oil",
        "3% Concentration from Precision\n"
        "6% Concentration from Healing Power",
        9461
    };
    const ConsumableInfo MasterfulAccuracy =
    {
        "Prec",
        "Writ / Thesis of Masterful Accuracy",
        "+200 Precision while above 90% Health\n"
        "+10% Experience from Kills"
    };

    const ConsumableInfo StudiedSpeed =
    {
        "Speed",
        "Writ / Thesis of Studied Speed",
        "Movement-speed utility effect"
    };

    //
    // UTILITY - SLAYING
    //
    const ConsumableInfo MordremSlayingPotion =
    {
        "Slay",
        "Powerful Potion of Mordrem Slaying",
        "+10% Damage vs. Mordrem\n"
        "-10% Damage from Mordrem\n"
        "+10% Experience from Kills",
        91350
    };
    const ConsumableInfo WeakSonsOfSvanirSlayingPotion =
    {
        "Slay",
        "Weak Potion of Sons of Svanir Slaying",
        "+3% Damage vs. Sons of Svanir\n"
        "+10% Experience from Kills"
    };

    const ConsumableInfo ScarletsArmiesSlayingPotion =
    {
        "Slay",
        "Powerful Potion of Slaying Scarlet's Armies",
        "+10% Damage against Scarlet's Armies\n"
        "-10% Damage from Scarlet's Armies"
    };
    const ConsumableInfo PowerfulDemonSlayingPotion =
    {
        "Slay",
        "Powerful Potion of Demon Slaying",
        "Utility effect"
    };

    const ConsumableInfo WritMasterfulStrength =
    {
        "Power",
        "Writ of Masterful Strength",
        "Utility effect"
    };
    const ConsumableInfo WritMasterfulMalice =
    {
        "Condi",
        "Writ of Masterful Malice",
        "Utility effect"
    };

    const ConsumableInfo PeppermintOil =
    {
        "Condi",
        "Peppermint Oil",
        "Utility effect"
    };

    const ConsumableInfo MagnanimousMaintenanceOil =
    {
        "Condi",
        "Magnanimous Maintenance Oil",
        "Utility effect"
    };

    //
    // UTILITY - ALL ATTRIBUTES
    //
    const ConsumableInfo DecadeEnhancement =
    {
        "All",
        "Decade Enhancement",
        "+3% All Attributes"
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

        case 57883:
            return SpicyMoaWings;

        case 9773:
            return OrrianSteakFrittes;

        case 9829:
            return CurryButternutSoup;

        case 9769:
            return TruffleSteak;

        case 37540:
            return WinterberrySteak;

        case 49686:
            return BeefRendang;

        case 15260:
            return SpicyPumpkinCookie;

        case 10009:
            return RareVeggiePizza;

        case 46273:
            return RedLentilSaobosa;

        case 57042:
            return BeefCarpaccioSalsa;

        case 25318:
            return BirthdayCake;

        case 34188:
            return PeppermintOmnomberryBar;

        case 37111:
            return WinterberrySeaweedSalad;

        case 9750:
            return PowerOnKillFood;

        case 57244:
            return CilantroLimeSousVideSteak;

        case 57051:
            return PeppercornSousVideSteak;

        case 53222:
            return SoulPastry;

        case 65937:
            return BlockOfTofu;

        case 57165:
            return PeppercornOysterSoup;

        case 57290:
            return SesameCoqAuVin;

        case 57409:
            return CilantroCuredMeatFlatbread;

        case 57341:
            return PoultryAspicSalsa;
        case 937:
            return GuildBanquetNourishment;

        case 9987:
            return ChocolateOmnomberryCream;

        case 10001:
            return OmnomberryBar;
        case 9778:
            return GrumbleCake;
        case 10069:
            return ChiliPepperPopper;
        case 57127:
            return PepperedCuredMeatFlatbread;

        case 57260:
            return PeppercornSpicedCoqAuVin;

        case 10000:
            return RaspberryPeachBar;

        case 57393:
            return MushroomCloveSousVideSteak;

        case 57100:
            return FruitSaladMintGarnish;
        case 10137:
            return OmnomberryTart;
        case 57241:
            return SoySesameSousVideSteak;
        case 57253:
            return CoqAuVinSalsa;

        case 57344:
            return CloveVeggieFlatbread;

        case 57356:
            return SpherifiedCilantroOysterSoup;

        case 57382:
            return PeppercornVeggieFlatbread;
        case 10006:
            return MushroomPizza;

        case 68232:
            return DecadeDesserts;
        case 37339:
            return BeefCarpaccioMintGarnish;
        case 9822:
            return SharedCondiFood9822;

        case 9993:
            return MangoPie;
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
            return SuperiorSharpeningStone;

        case 9962:
            return HardenedSharpeningStone;

        case 9958:
            return RoughTinySharpeningStone;
        case 77466:
            return SharpeningGolem;

        case 34211:
            return TinOfFruitcake;

        case 25882:
            return FuriousSharpeningStone;
        case 46925:
            return CorsairSharpeningStone;

        case 21828:
            return ToxicFocusingCrystal;

        case 9967:
            return MasterTuningCrystal;

        case 10113:
            return ApprenticeTuningCrystal;

        case 34206:
            return TuningIcicle;

        case 25878:
            return FuriousTuningCrystal;
        case 25879:
            return BountifulMaintenanceOil;

        case 53374:
            return PotentLucentOil;

        case 53304:
            return EnhancedLucentOil;

        case 21827:
            return ToxicMaintenanceOil;

        case 9968:
            return MasterMaintenanceOil;

        case 56772:
            return MordremSlayingPotion;
        case 9902:
            return WeakSonsOfSvanirSlayingPotion;

        case 68235:
            return DecadeEnhancement;

        case 23228:
            return ScarletsArmiesSlayingPotion;
        case 9901:
            return PowerfulDemonSlayingPotion;

        case 33297:
            return WritMasterfulStrength;
        case 33836:
            return WritMasterfulMalice;

        case 34187:
            return PeppermintOil;

        case 38605:
            return MagnanimousMaintenanceOil;
        case 31970:
            return MasterfulAccuracy;

        case 33005:
            return StudiedSpeed;

        default:
            return UnknownUtility;
        }
    }

    bool IsIgnoredBuff(
        uint32_t skillID
    )
    {
        switch (skillID)
        {
        case 10110:
        case 10104:
        case 64528:
        case 32289:
        case 32293:
        case 33046:
        case 65475:
            return true;

        default:
            return false;
        }
    }
}
