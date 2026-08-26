#include "ConsumableData.h"
#include <cstddef>

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
         9443
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
        "+10% Experience from Kills",
        8808
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
    //
    // MASTER DATABASE FALLBACK
    //
    // Existing hand-authored switch entries remain the first choice so
    // verified item IDs and detailed tooltips are preserved. These tables
    // fill in the rest of the prepared Food/Utility effect database.
    //
    struct FallbackConsumable
    {
        uint32_t skillID;
        ConsumableInfo info;
    };

    const FallbackConsumable MasterFoodFallback[] =
    {
        { 9736, { "Food", "Bowl of Avocado Stirfry", "Known nourishment effect", 12397 } },
        { 9743, { "Food", "Moa Haunch", "Known nourishment effect", 8525 } },
        { 9745, { "Food", "Pepper Steak Dinner", "Known nourishment effect", 12307 } },
        { 9750, { "Kill", "Dragon's Breath Bun", "+200 Power on Kill\n+70 Ferocity", 43360 } },
        { 9752, { "Food", "Sage-Stuffed Mushroom", "Known nourishment effect", 12360 } },
        { 9756, { "Kill", "Saffron Stuffed Mushroom", "+200 Condition Damage on Kill\n+70 Precision", 12476 } },
        { 9758, { "Food", "Roasted Meaty Sandwich", "Known nourishment effect", 12212 } },
        { 9769, { "Prec", "Plate of Truffle Steak", "+100 Power\n+70 Precision", 12467 } },
        { 9776, { "Food", "Bowl of Beet and Bean Stew", "Known nourishment effect", 12348 } },
        { 9782, { "Food", "Blueberry Muffin / Passion Fruit Soufflé", "Known nourishment effect" } },
        { 9784, { "Food", "Blackened Fish Steak", "Known nourishment effect", 8563 } },
        { 9786, { "Food", "Seared Beef Steak / Ogre Pet Snack; Peeled Spikeroot Fruit", "Known nourishment effect" } },
        { 9790, { "Food", "Apple Passion Fruit Pie / Blueberry Cookie; Prototype Nutriment", "Known nourishment effect" } },
        { 9792, { "Food", "Passion Fruit Bar / Prismatic Soylent", "Known nourishment effect" } },
        { 9796, { "Food", "Peach Cookie", "Known nourishment effect", 12418 } },
        { 9797, { "Food", "Omnomberry Cookie", "Known nourishment effect", 12455 } },
        { 9798, { "Food", "Murrellow Stimulant Snack", "Known nourishment effect", 8628 } },
        { 9805, { "Prec", "Bowl of Fancy Potato and Leek Soup", "+100 Precision\n+70 Condition Damage", 12485 } },
        { 9808, { "Food", "Bowl of Artichoke Soup", "Known nourishment effect", 12450 } },
        { 9810, { "Food", "Boiled Crawdad", "Known nourishment effect", 8544 } },
        { 9817, { "Prec", "Plate of Truffle Risotto", "+100 Condition Damage\n+70 Precision" } },
        { 9822, { "Food", "Red Bean Cake", "Known nourishment effect", 8575 } },
        { 9828, { "Food", "Bowl of Butternut Squash Soup", "Known nourishment effect", 12449 } },
        { 9829, { "Prec", "Bowl of Curry Butternut Squash Soup", "+100 Precision\n+70 Power", 12486 } },
        { 9950, { "Food", "Blueberry Pie", "Known nourishment effect", 12316 } },
        { 9955, { "Food", "Blackberry Pie", "Known nourishment effect", 12385 } },
        { 9956, { "Food", "Mixed Berry Pie", "Known nourishment effect", 12420 } },
        { 9957, { "Food", "Omnomberry Pie", "Known nourishment effect", 12457 } },
        { 9988, { "Food", "Apple Pie / Apple Tart; Seraph Standard Ration", "Known nourishment effect" } },
        { 9989, { "Food", "Banana Cream Pie / Seraph Spicy Ration", "Known nourishment effect" } },
        { 9990, { "Food", "Raspberry Pie", "Known nourishment effect", 12264 } },
        { 9991, { "Food", "Pumpkin Pie", "Known nourishment effect", 12384 } },
        { 9992, { "Food", "Peach Pie", "Known nourishment effect", 12419 } },
        { 9994, { "Food", "Bowl of Apple Sauce / Shared nourishment effect", "Known nourishment effect" } },
        { 9999, { "Food", "Orange Coconut Bar", "Known nourishment effect", 12380 } },
        { 10000, { "Food", "Raspberry Peach Bar", "Known nourishment effect", 12415 } },
        { 10001, { "Food", "Omnomberry Bar", "Known nourishment effect", 12452 } },
        { 10006, { "Food", "Mushroom Pizza", "Known nourishment effect", 12368 } },
        { 10009, { "Exper", "Rare Veggie Pizza", "+100 Expertise\n+70 Condition Damage", 12464 } },
        { 10010, { "Food", "Bowl of Basic Poultry Soup", "Known nourishment effect", 12731 } },
        { 10012, { "Food", "Moa Egg Omelet", "Known nourishment effect", 8614 } },
        { 10017, { "Food", "Quiche of Darkness", "Known nourishment effect", 76944 } },
        { 10023, { "Food", "Roasted Rutabaga", "Known nourishment effect", 12130 } },
        { 10024, { "Food", "Roasted Parsnip", "Known nourishment effect", 12438 } },
        { 10026, { "Food", "Bowl of Blueberry Apple Compote", "Known nourishment effect", 12311 } },
        { 10029, { "Food", "Bowl of Blackberry Pear Compote", "Known nourishment effect", 12386 } },
        { 10030, { "Food", "Raspberry Peach Compote", "Known nourishment effect", 12421 } },
        { 10031, { "Food", "Omnomberry Compote", "Known nourishment effect", 12458 } },
        { 10041, { "Food", "Quaggan Fish Snack", "Known nourishment effect", 8581 } },
        { 10043, { "Food", "Bag of Simple Cat Food / Sage-Stuffed Poultry", "Known nourishment effect" } },
        { 10048, { "Power", "Dragonfish Candy", "+100 Vitality\n+200 Power when Health below 50%\n+10% Experience from Kills", 43363 } },
        { 10049, { "Food", "Rabbit Offering", "Known nourishment effect", 8562 } },
        { 10051, { "Food", "Bowl of Bean Salad", "Known nourishment effect", 12361 } },
        { 10053, { "Food", "Bowl of Avocado Salsa", "Known nourishment effect", 12288 } },
        { 10057, { "Food", "Bowl of Basic Vegetable Soup", "Known nourishment effect", 12729 } },
        { 10067, { "Food", "Roasted Artichoke", "Known nourishment effect", 12442 } },
        { 10075, { "Food", "Bowl of Ascalonian Salad", "Known nourishment effect", 12196 } },
        { 10079, { "Food", "Bowl of Asparagus and Sage Salad", "Known nourishment effect", 12434 } },
        { 10085, { "Food", "Bowl of Poultry Noodle Soup / Roasted Aloe Seed", "Known nourishment effect" } },
        { 10096, { "Endu", "Bowl of Orrian Truffle and Meat Stew", "100% Might on Dodge\n+40% Endurance Regeneration", 12488 } },
        { 10119, { "Burn", "Bowl of Fire Meat Chili", "+15% Burning Duration\n+70 Precision", 12484 } },
        { 10125, { "Food", "Orange Cake / Orange Coconut Cake", "Known nourishment effect" } },
        { 10136, { "Food", "Peach Tart", "Known nourishment effect", 12422 } },
        { 10137, { "Food", "Omnomberry Tart", "Known nourishment effect", 12459 } },
        { 15250, { "Food", "Bowl of Candy Corn Custard", "Known nourishment effect", 36074 } },
        { 15259, { "Food", "Omnomberry Ghost", "Known nourishment effect", 36083 } },
        { 15838, { "Food", "Orange Passion Fruit Tart", "Known nourishment effect", 36777 } },
        { 15839, { "Food", "Passion Fruit Coconut Cookie", "Known nourishment effect", 36828 } },
        { 15840, { "Food", "Raspberry Passion Fruit Compote", "Known nourishment effect", 36782 } },
        { 16501, { "Heal", "Kralkachocolate Bar", "+100 Healing Power\n+70 Toughness\n+5% Karma", 43358 } },
        { 16505, { "Food", "Bowl of Blueberry Chocolate Chunk Ice Cream", "Known nourishment effect", 38214 } },
        { 17825, { "Power", "Bowl of Sweet and Spicy Butternut Squash Soup", "+100 Power\n+70 Ferocity", 41569 } },
        { 19451, { "All", "Dragon's Revelry Starcake", "+45 All Attributes", 43550 } },
        { 24807, { "Food", "Bowl of Black Pepper Cactus Salad / Rock Candy", "Known nourishment effect" } },
        { 26445, { "Food", "Bloodstone Pot Pie", "Known nourishment effect", 68562 } },
        { 26529, { "Heal", "Delicious Rice Ball", "+10% Outgoing Healing\n+100 Healing Power", 68634 } },
        { 26532, { "Food", "Sweet Bean Bun", "Known nourishment effect", 68636 } },
        { 26534, { "Food", "Spring Roll", "Known nourishment effect", 68632 } },
        { 33084, { "Food", "Order of Whispers Rations / Pact Ration", "Known nourishment effect" } },
        { 33856, { "OnHeal", "Jerk Poultry and Nopal Flatbread Sandwich", "+200 Power on Heal\n+70 Precision", 73788 } },
        { 34207, { "Food", "Scoop of Mintberry Swirl Ice Cream", "Known nourishment effect", 77630 } },
        { 34210, { "Food", "Candy Cane", "+10% Karma Bonus", 77651 } },
        { 34570, { "Matt", "Bowl of Bloodstone Bisque", "Special Bloodstone effect", 78005 } },
        { 37540, { "Prec", "Steak with Winterberry Sauce", "+100 Power\n+70 Precision", 79786 } },
        { 38079, { "Food", "Saffron Mussels", "Known nourishment effect", 80466 } },
        { 39042, { "Food", "Oysters with Pesto Sauce", "Known nourishment effect", 81621 } },
        { 39067, { "Food", "Oysters Gnashblade", "Known nourishment effect", 81786 } },
        { 39341, { "Food", "Oysters with Cocktail Sauce", "Known nourishment effect", 81615 } },
        { 39344, { "Food", "Oysters with Zesty Sauce", "Known nourishment effect", 81799 } },
        { 39500, { "Food", "Oysters with Spicy Sauce", "Known nourishment effect", 81867 } },
        { 43333, { "Food", "Bowl of \"Elon Red\"", "Known nourishment effect" } },
        { 45694, { "Food", "Red Lentil and Flatbread Feast", "Known nourishment effect", 82657 } },
        { 46273, { "Exper", "Red Lentil Saobosa", "+100 Expertise\n+70 Condition Damage" } },
        { 49296, { "Heal", "Bowl of Poultry Satay", "+100 Healing Power\n+70 Concentration", 87076 } },
        { 49686, { "Condi", "Plate of Beef Rendang", "+100 Condition Damage\n+70 Expertise", 86997 } },
        { 50091, { "Food", "Avocado Smoothie", "Known nourishment effect", 87029 } },
        { 53222, { "PConc", "Soul Pastry", "+100 Concentration\n+70 Power", 89002 } },
        { 57037, { "All", "Spherified Sesame Oyster Soup", "Health every second\n+45 All Attributes", 91756 } },
        { 57042, { "PConc", "Beef Carpaccio with Salsa Garnish", "66% Life Steal Chance\n+100 Concentration\n+70 Power" } },
        { 57050, { "Exper", "Sesame Veggie Flatbread", "Health every second\n+100 Expertise\n+70 Condition Damage", 91823 } },
        { 57051, { "Power", "Peppercorn-Crusted Sous-Vide Steak", "-10% Incoming Damage\n+100 Power\n+70 Ferocity", 91734 } },
        { 57064, { "Condi", "Mint-Pear Cured Meat Flatbread", "+10% Outgoing Healing\n+100 Condition Damage\n+70 Expertise", 91703 } },
        { 57072, { "Condi", "Clove-Spiced Pear and Cured Meat Flatbread", "-20% Incoming Condition Duration\n+100 Condition Damage\n+70 Expertise", 91784 } },
        { 57084, { "CConc", "Sesame Eggs Benedict", "Health every second\n+100 Concentration\n+70 Expertise", 91855 } },
        { 57100, { "Heal", "Bowl of Fruit Salad with Mint Garnish", "+10% Outgoing Healing\n+100 Healing Power\n+70 Concentration", 91690 } },
        { 57101, { "Heal", "Bowl of Sesame Fruit Salad", "Health every second\n+100 Healing Power\n+70 Concentration", 91732 } },
        { 57114, { "PConc", "Plate of Peppercorn-Spiced Beef Carpaccio", "-10% Incoming Damage\n+100 Concentration\n+70 Power", 91858 } },
        { 57117, { "CConc", "Salsa Eggs Benedict", "66% Life Steal Chance\n+100 Concentration\n+70 Expertise", 91847 } },
        { 57127, { "Condi", "Peppered Cured Meat Flatbread", "-10% Incoming Damage\n+100 Condition Damage\n+70 Expertise", 91848 } },
        { 57165, { "All", "Spherified Peppercorn-Spiced Oyster Soup", "-10% Incoming Damage\n+45 All Attributes", 91713 } },
        { 57187, { "CConc", "Plate of Eggs Benedict", "+100 Concentration\n+70 Expertise", 91842 } },
        { 57201, { "All", "Spherified Oyster Soup with Mint Garnish", "+10% Outgoing Healing\n+45 All Attributes", 91748 } },
        { 57210, { "CConc", "Peppercorn-Spiced Eggs Benedict", "-10% Incoming Damage\n+100 Concentration\n+70 Expertise", 91865 } },
        { 57222, { "Condi", "Sesame-Asparagus and Cured Meat Flatbread", "Health every second\n+100 Condition Damage\n+70 Expertise", 91867 } },
        { 57231, { "PConc", "Plate of Sesame-Ginger Beef Carpaccio", "Health every second\n+100 Concentration\n+70 Power", 91718 } },
        { 57237, { "PConc", "Plate of Clove-Spiced Beef Carpaccio", "-20% Incoming Condition Duration\n+100 Concentration\n+70 Power", 91806 } },
        { 57241, { "Power", "Soy-Sesame Sous-Vide Steak", "Health every second\n+100 Power\n+70 Ferocity", 91736 } },
        { 57242, { "CConc", "Clove-Spiced Eggs Benedict", "-20% Incoming Condition Duration\n+100 Concentration\n+70 Expertise", 91705 } },
        { 57244, { "Power", "Cilantro Lime Sous-Vide Steak", "66% Life Steal Chance\n+100 Power\n+70 Ferocity", 91805 } },
        { 57251, { "PConc", "Plate of Beef Carpaccio with Mint Garnish", "+10% Outgoing Healing\n+100 Concentration\n+70 Power", 91809 } },
        { 57253, { "Prec", "Plate of Coq Au Vin with Salsa", "66% Life Steal Chance\n+100 Power\n+70 Precision", 91709 } },
        { 57259, { "CConc", "Eggs Benedict with Mint-Parsley Sauce", "+10% Outgoing Healing\n+100 Concentration\n+70 Expertise", 91758 } },
        { 57260, { "Prec", "Plate of Peppercorn-Spiced Coq Au Vin", "-10% Incoming Damage\n+100 Power\n+70 Precision", 91753 } },
        { 57263, { "Exper", "Mint and Veggie Flatbread", "+10% Outgoing Healing\n+100 Expertise\n+70 Condition Damage", 91727 } },
        { 57269, { "Exper", "Salsa-Topped Veggie Flatbread", "66% Life Steal Chance\n+100 Expertise\n+70 Condition Damage", 91876 } },
        { 57276, { "Heal", "Bowl of Spiced Fruit Salad", "-10% Incoming Damage\n+100 Healing Power\n+70 Concentration", 91737 } },
        { 57290, { "Prec", "Plate of Sesame-Crusted Coq Au Vin", "Health every second\n+100 Power\n+70 Precision", 91711 } },
        { 57342, { "Power", "Sous-Vide Steak with Mint-Parsley Sauce", "+10% Outgoing Healing\n+100 Power\n+70 Ferocity", 91801 } },
        { 57344, { "Exper", "Clove and Veggie Flatbread", "-20% Incoming Condition Duration\n+100 Expertise\n+70 Condition Damage", 91698 } },
        { 57348, { "Prec", "Plate of Clove-Spiced Coq Au Vin", "-20% Incoming Condition Duration\n+100 Power\n+70 Precision", 91780 } },
        { 57356, { "All", "Spherified Cilantro Oyster Soup", "66% Life Steal Chance\n+45 All Attributes", 91804 } },
        { 57362, { "Prec", "Plate of Coq Au Vin with Mint Garnish", "+10% Outgoing Healing\n+100 Power\n+70 Precision", 91822 } },
        { 57374, { "All", "Spherified Clove-Spiced Oyster Soup", "-20% Incoming Condition Duration\n+45 All Attributes", 91754 } },
        { 57382, { "Exper", "Peppercorn and Veggie Flatbread", "-10% Incoming Damage\n+100 Expertise\n+70 Condition Damage", 91771 } },
        { 57393, { "Power", "Mushroom Clove Sous-Vide Steak", "-20% Incoming Condition Duration\n+100 Power\n+70 Ferocity", 91790 } },
        { 57397, { "Heal", "Bowl of Fruit Salad with Orange-Clove Syrup", "-20% Incoming Condition Duration\n+100 Healing Power\n+70 Concentration", 91841 } },
        { 57401, { "Heal", "Bowl of Fruit Salad with Cilantro Garnish", "66% Life Steal Chance\n+100 Healing Power\n+70 Concentration", 91769 } },
        { 57409, { "Condi", "Cilantro and Cured Meat Flatbread", "66% Life Steal Chance\n+100 Condition Damage\n+70 Expertise", 91878 } },
        { 57883, { "Power", "Plate of Spicy Moa Wings", "+100 Power\n+70 Ferocity", 91917 } },
        { 58105, { "Food", "Saint Bones", "Known nourishment effect", 92138 } },
        { 64357, { "Power", "Bowl of Jade Sea", "+150 Fishing Power\n+100 Power\n+70 Ferocity" } },
        { 64568, { "Torm", "Meaty Asparagus Skewer", "+15% Torment Duration\n+70 Condition Damage", 95942 } },
        { 64896, { "Food", "Orangefish Sushi", "Known nourishment effect", 96898 } },
        { 64931, { "Food", "Redfish Sushi", "Known nourishment effect", 97771 } },
        { 65197, { "Poison", "Bowl of Kimchi Tofu Stew", "+15% Poison Duration\n+70 Condition Damage", 97422 } },
        { 65354, { "Confu", "Meaty Rice Bowl", "+15% Confusion Duration\n+70 Condition Damage", 96916 } },
        { 65769, { "Bleed", "Plate of Kimchi Pancakes", "+15% Bleed Duration\n+70 Condition Damage", 96578 } },
        { 65937, { "Kill", "Block of Tofu", "+100 Power & Ferocity on Kill\n+70 Precision", 96793 } },
        { 66503, { "Burn", "Fishy Rice Bowl", "+15% Burning Duration\n+70 Condition Damage", 97767 } },
        { 66663, { "Condi", "Bowl of Echovald Hotpot", "+150 Fishing Power\n+100 Condition Damage\n+70 Expertise", 97592 } },
        { 67265, { "Heal", "Plate of Imperial Palace Special", "+150 Fishing Power\n+100 Healing Power\n+70 Concentration", 97472 } },
        { 67705, { "All", "Flight of Sushi", "+150 Fishing Power\n+45 All Attributes", 97200 } },
        { 77728, { "Food", "Bag of Popped Candy Corn", "Known nourishment effect", 105402 } },
        { 79403, { "Food", "New Year Rice Cake", "Known nourishment effect", 107101 } },
    };

    const FallbackConsumable MasterUtilityFallback[] =
    {
        { 9834, { "Slay", "Undead Battle Potion", "Known enhancement effect", 8798 } },
        { 9837, { "Slay", "Powerful Potion of Undead Slaying", "+10% Damage vs Undead\n-10% Damage from Undead", 8893 } },
        { 9845, { "Slay", "Powerful Potion of Centaur Slaying", "+10% Damage vs Centaur\n-10% Damage from Centaur", 8880 } },
        { 9853, { "Slay", "Powerful Potion of Grawl Slaying", "+10% Damage vs Grawl\n-10% Damage from Grawl", 8890 } },
        { 9861, { "Slay", "Powerful Potion of Ice Brood Slaying", "+10% Damage vs Ice Brood\n-10% Damage from Ice Brood", 8889 } },
        { 9869, { "Slay", "Powerful Potion of Destroyer Slaying", "+10% Damage vs Destroyers\n-10% Damage from Destroyers", 8884 } },
        { 9877, { "Slay", "Powerful Potion of Ogre Slaying", "+10% Damage vs Ogres\n-10% Damage from Ogres", 8888 } },
        { 9885, { "Slay", "Powerful Potion of Krait Slaying", "+10% Damage vs Krait\n-10% Damage from Krait", 8891 } },
        { 9893, { "Slay", "Powerful Potion of Elemental Slaying", "+10% Damage vs Elementals\n-10% Damage from Elementals", 8885 } },
        { 9901, { "Slay", "Powerful Potion of Demon Slaying", "+10% Damage vs Demons\n-10% Damage from Demons", 8886 } },
        { 9909, { "Slay", "Powerful Potion of Sons of Svanir Slaying", "+10% Damage vs Sons of Svanir\n-10% Damage from Sons of Svanir", 8883 } },
        { 9917, { "Slay", "Powerful Potion of Inquest Slaying", "+10% Damage vs Inquest\n-10% Damage from Inquest", 8887 } },
        { 9925, { "Slay", "Powerful Potion of Flame Legion Slaying", "+10% Damage vs Flame Legion\n-10% Damage from Flame Legion", 8879 } },
        { 9933, { "Slay", "Powerful Potion of Outlaw Slaying", "+10% Damage vs Outlaws\n-10% Damage from Outlaws", 8881 } },
        { 9941, { "Slay", "Powerful Potion of Nightmare Court Slaying", "+10% Damage vs Nightmare Court\n-10% Damage from Nightmare Court", 8882 } },
        { 9949, { "Slay", "Powerful Potion of Dredge Slaying", "+10% Damage vs Dredge\n-10% Damage from Dredge", 8892 } },
        { 9963, { "Power", "Superior Sharpening Stone", "3% Power from Precision\n6% Power from Ferocity" } },
        { 9965, { "Utility", "Artisan Tuning Crystal", "Known enhancement effect", 9472 } },
        { 9967, { "Condi", "Master Tuning Crystal", "3% Condition Damage from Precision\n8% Condition Damage from Expertise", 9476 } },
        { 9968, { "HConc", "Master Maintenance Oil", "3% Concentration from Precision\n6% Concentration from Healing Power", 9461 } },
        { 9970, { "Utility", "Artisan Maintenance Oil", "Known enhancement effect", 9458 } },
        { 10111, { "Utility", "Apprentice Maintenance Oil", "Known enhancement effect", 9452 } },
        { 10113, { "Utility", "Apprentice Tuning Crystal", "Known enhancement effect", 9464 } },
        { 15279, { "Slay", "Powerful Potion of Halloween Slaying", "+10% Damage vs Halloween Creatures\n-10% Damage from Halloween Creatures", 36092 } },
        { 21827, { "CConc", "Toxic Maintenance Oil", "3% Concentration from Power\n6% Concentration from Condition Damage", 48916 } },
        { 21828, { "Condi", "Toxic Focusing Crystal", "3% Condition Damage from Power\n3% Condition Damage from Precision" } },
        { 23228, { "Slay", "Powerful Potion of Slaying Scarlet's Armies", "+10% Damage vs Scarlet's Armies\n-10% Damage from Scarlet's Armies", 50082 } },
        { 25630, { "Res", "Sharpening Skull", "+75 All Attributes after reviving", 67368 } },
        { 25631, { "Res", "Lump of Crystallized Nougat", "+100 Condition Damage, Precision & Toughness after reviving", 67367 } },
        { 25632, { "Res", "Flask of Pumpkin Oil", "+100 Power, Toughness & Vitality after reviving", 67371 } },
        { 25877, { "Utility", "Bountiful Tuning Crystal", "Known enhancement effect", 67522 } },
        { 25878, { "Exper", "Furious Tuning Crystal", "3% Condition Damage from Precision\n3% Expertise from Precision", 67524 } },
        { 25879, { "Heal", "Bountiful Maintenance Oil", "0.6% Outgoing Healing per 100 Healing Power\n0.8% per 100 Concentration", 67528 } },
        { 25880, { "Utility", "Bountiful Sharpening Stone", "Known enhancement effect", 67531 } },
        { 25882, { "Power", "Furious Sharpening Stone", "3% Power from Precision\n3% Ferocity from Precision", 67530 } },
        { 31970, { "Writ", "Writ of Masterful Accuracy", "+200 Precision above 90% Health", 76833 } },
        { 33297, { "Writ", "Writ of Masterful Strength", "+200 Power above 90% Health", 73191 } },
        { 33836, { "Writ", "Writ of Masterful Malice", "+200 Condition Damage above 90% Health", 72510 } },
        { 34187, { "HConc", "Peppermint Oil", "3% Concentration from Precision\n6% Concentration from Healing Power", 77632 } },
        { 34206, { "Condi", "Tuning Icicle", "3% Condition Damage from Precision\n8% Condition Damage from Expertise", 77567 } },
        { 34211, { "Power", "Tin of Fruitcake", "3% Power from Precision\n6% Power from Ferocity", 77569 } },
        { 34657, { "Utility", "Compact Hardened Sharpening Stone", "Known enhancement effect", 78116 } },
        { 34671, { "Utility", "Compact Quality Maintenance Oil", "Known enhancement effect", 78695 } },
        { 34677, { "Utility", "Compact Quality Tuning Crystal", "Known enhancement effect", 78219 } },
        { 38605, { "TConc", "Magnanimous Maintenance Oil", "3% Concentration from Vitality\n3% Concentration from Toughness", 81157 } },
        { 38678, { "Condi", "Magnanimous Tuning Crystal", "3% Condition Damage from Vitality\n3% Condition Damage from Toughness", 81079 } },
        { 46925, { "Utility", "Corsair Sharpening Stone", "Known enhancement effect", 86378 } },
        { 47734, { "Utility", "Corsair Maintenance Oil", "Known enhancement effect", 86016 } },
        { 48348, { "Utility", "Corsair Tuning Crystal", "Known enhancement effect", 86287 } },
        { 50302, { "HConc", "Holographic Super Drumstick", "8% Healing Power from Concentration\n3% Concentration from Precision", 87329 } },
        { 50307, { "CConc", "Holographic Super Apple", "8% Power from Condition Damage\n3% Concentration from Precision", 87336 } },
        { 50320, { "PConc", "Holographic Super Cheese", "8% Power from Concentration\n3% Concentration from Precision", 87358 } },
        { 53304, { "PConc", "Enhanced Lucent Oil", "6% Concentration from Condition Damage\n3% Concentration from Precision", 89157 } },
        { 53374, { "PConc", "Potent Lucent Oil", "3% Concentration from Power\n3% Concentration from Precision", 89203 } },
        { 54561, { "Utility", "Dragon Crystal Potion", "Known enhancement effect", 89461 } },
        { 59643, { "Utility", "Chatoyant Elixir", "Known enhancement effect", 93241 } },
        { 68235, { "All", "Decade Enhancement", "+3% All Attributes" } },
    };

    const ConsumableInfo* FindMasterFallback(
        const FallbackConsumable* entries,
        size_t count,
        uint32_t skillID
    )
    {
        for (size_t i = 0; i < count; ++i)
        {
            if (entries[i].skillID == skillID)
            {
                return &entries[i].info;
            }
        }

        return nullptr;
    }

    const ConsumableInfo* FindMasterFood(
        uint32_t skillID
    )
    {
        return FindMasterFallback(
            MasterFoodFallback,
            sizeof(MasterFoodFallback) /
            sizeof(MasterFoodFallback[0]),
            skillID
        );
    }

    const ConsumableInfo* FindMasterUtility(
        uint32_t skillID
    )
    {
        return FindMasterFallback(
            MasterUtilityFallback,
            sizeof(MasterUtilityFallback) /
            sizeof(MasterUtilityFallback[0]),
            skillID
        );
    }

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
        {
            const ConsumableInfo* masterInfo =
                FindMasterFood(skillID);

            return masterInfo != nullptr
                ? *masterInfo
                : UnknownFood;
        }
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
        {
            const ConsumableInfo* masterInfo =
                FindMasterUtility(skillID);

            return masterInfo != nullptr
                ? *masterInfo
                : UnknownUtility;
        }
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

