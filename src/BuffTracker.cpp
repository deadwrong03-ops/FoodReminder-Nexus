#include "BuffTracker.h"
#include "ConsumableData.h"
#include "Settings.h"
#include "SessionTracker.h"
#include <mutex>
#include <vector>
#include <chrono>
#include <string>
#include <ctime>

namespace
{
    std::mutex g_BuffMutex;

    uint64_t g_TotalEventCount = 0;
    uint64_t g_BuffLikeEventCount = 0;

    std::vector<BuffEventDebug> g_RecentBuffEvents;

    constexpr size_t MAX_DEBUG_EVENTS = 100;

    constexpr uint8_t STATECHANGE_ENTER_COMBAT = 1;
    constexpr uint8_t STATECHANGE_EXIT_COMBAT = 2;

    bool g_HasFood = false;
    bool g_HasUtility = false;

    ConsumableDetectionState g_FoodDetectionState =
        ConsumableDetectionState::Unknown;

    ConsumableDetectionState g_UtilityDetectionState =
        ConsumableDetectionState::Unknown;

    uint32_t g_FoodSkillID = 0;
    uint32_t g_UtilitySkillID = 0;

    std::string g_FoodSkillName;
    std::string g_UtilitySkillName;

    bool g_HasMetabolicPrimer = false;
    bool g_HasUtilityPrimer = false;

    ConsumableDetectionState g_MetabolicPrimerDetectionState =
        ConsumableDetectionState::Unknown;

    ConsumableDetectionState g_UtilityPrimerDetectionState =
        ConsumableDetectionState::Unknown;

    bool g_HasCandyCane = false;
    int64_t g_CandyCaneDurationMilliseconds = 0;
    std::chrono::steady_clock::time_point g_CandyCaneReceivedTime;


    bool g_IsInCombat = false;

    // Used to detect character changes.
    // Food and Utility are character-specific and
    // must not carry over to another character.
    uintptr_t g_SelfAgentID = 0;
    std::string g_SelfCharacterName;

    bool g_SettingsChanged = false;

    int64_t g_FoodDurationMilliseconds = 0;
    int64_t g_UtilityDurationMilliseconds = 0;

    int64_t g_MetabolicPrimerDurationMilliseconds = 0;
    int64_t g_UtilityPrimerDurationMilliseconds = 0;

    std::chrono::steady_clock::time_point g_FoodReceivedTime;
    std::chrono::steady_clock::time_point g_UtilityReceivedTime;

    std::chrono::steady_clock::time_point g_MetabolicPrimerReceivedTime;
    std::chrono::steady_clock::time_point g_UtilityPrimerReceivedTime;

    int64_t GetRemainingSecondsLocked(
        bool hasBuff,
        int64_t durationMilliseconds,
        const std::chrono::steady_clock::time_point& receivedTime
    )
    {
        if (!hasBuff ||
            durationMilliseconds <= 0)
        {
            return 0;
        }

        const auto elapsed =
            std::chrono::duration_cast<
            std::chrono::milliseconds
            >(
                std::chrono::steady_clock::now() -
                receivedTime
            ).count();

        const int64_t remainingMilliseconds =
            durationMilliseconds - elapsed;

        if (remainingMilliseconds <= 0)
        {
            return 0;
        }

        // Round upward so saving/restoring does not
        // lose almost a full second on every switch.
        return
            (remainingMilliseconds + 999) /
            1000;
    }


    int64_t GetRemainingMillisecondsLocked(
        bool hasBuff,
        int64_t durationMilliseconds,
        const std::chrono::steady_clock::time_point& receivedTime
    )
    {
        if (!hasBuff ||
            durationMilliseconds <= 0)
        {
            return 0;
        }

        const auto elapsed =
            std::chrono::duration_cast<
            std::chrono::milliseconds
            >(
                std::chrono::steady_clock::now() -
                receivedTime
            ).count();

        const int64_t remainingMilliseconds =
            durationMilliseconds - elapsed;

        return remainingMilliseconds > 0
            ? remainingMilliseconds
            : 0;
    }


    int64_t GetDirectMetabolicPrimerRemainingMillisecondsLocked()
    {
        return GetRemainingMillisecondsLocked(
            g_HasMetabolicPrimer,
            g_MetabolicPrimerDurationMilliseconds,
            g_MetabolicPrimerReceivedTime
        );
    }

    int64_t GetDirectUtilityPrimerRemainingMillisecondsLocked()
    {
        return GetRemainingMillisecondsLocked(
            g_HasUtilityPrimer,
            g_UtilityPrimerDurationMilliseconds,
            g_UtilityPrimerReceivedTime
        );
    }

    void SaveCurrentCharacterConsumablesLocked()
    {
        if (g_SelfCharacterName.empty())
        {
            return;
        }

        CharacterConsumableState& state =
            g_Settings.characterConsumables[
                g_SelfCharacterName
            ];

        const bool foodStateKnown =
            g_FoodDetectionState !=
            ConsumableDetectionState::Unknown;

        const bool utilityStateKnown =
            g_UtilityDetectionState !=
            ConsumableDetectionState::Unknown;

        const bool metabolicPrimerStateKnown =
            g_MetabolicPrimerDetectionState !=
            ConsumableDetectionState::Unknown;

        const bool utilityPrimerStateKnown =
            g_UtilityPrimerDetectionState !=
            ConsumableDetectionState::Unknown;

        const int64_t foodRemainingSeconds =
            GetRemainingSecondsLocked(
                g_HasFood,
                g_FoodDurationMilliseconds,
                g_FoodReceivedTime
            );

        const int64_t utilityRemainingSeconds =
            GetRemainingSecondsLocked(
                g_HasUtility,
                g_UtilityDurationMilliseconds,
                g_UtilityReceivedTime
            );

        const int64_t metabolicPrimerRemainingSeconds =
            GetRemainingSecondsLocked(
                g_HasMetabolicPrimer,
                g_MetabolicPrimerDurationMilliseconds,
                g_MetabolicPrimerReceivedTime
            );

        const int64_t utilityPrimerRemainingSeconds =
            GetRemainingSecondsLocked(
                g_HasUtilityPrimer,
                g_UtilityPrimerDurationMilliseconds,
                g_UtilityPrimerReceivedTime
            );

        if (state.foodStateKnown !=
            foodStateKnown)
        {
            state.foodStateKnown =
                foodStateKnown;

            g_SettingsChanged = true;
        }

        if (state.utilityStateKnown !=
            utilityStateKnown)
        {
            state.utilityStateKnown =
                utilityStateKnown;

            g_SettingsChanged = true;
        }

        if (state.foodRemainingSeconds !=
            foodRemainingSeconds)
        {
            state.foodRemainingSeconds =
                foodRemainingSeconds;

            g_SettingsChanged = true;
        }

        if (state.utilityRemainingSeconds !=
            utilityRemainingSeconds)
        {
            state.utilityRemainingSeconds =
                utilityRemainingSeconds;

            g_SettingsChanged = true;
        }

        if (state.metabolicPrimerStateKnown !=
            metabolicPrimerStateKnown)
        {
            state.metabolicPrimerStateKnown =
                metabolicPrimerStateKnown;

            g_SettingsChanged = true;
        }

        if (state.utilityPrimerStateKnown !=
            utilityPrimerStateKnown)
        {
            state.utilityPrimerStateKnown =
                utilityPrimerStateKnown;

            g_SettingsChanged = true;
        }

        if (state.metabolicPrimerRemainingSeconds !=
            metabolicPrimerRemainingSeconds)
        {
            state.metabolicPrimerRemainingSeconds =
                metabolicPrimerRemainingSeconds;

            g_SettingsChanged = true;
        }

        if (state.utilityPrimerRemainingSeconds !=
            utilityPrimerRemainingSeconds)
        {
            state.utilityPrimerRemainingSeconds =
                utilityPrimerRemainingSeconds;

            g_SettingsChanged = true;
        }

        const uint32_t savedFoodSkillID =
            foodRemainingSeconds > 0
            ? g_FoodSkillID
            : 0;

        const uint32_t savedUtilitySkillID =
            utilityRemainingSeconds > 0
            ? g_UtilitySkillID
            : 0;

        if (state.foodSkillID !=
            savedFoodSkillID)
        {
            state.foodSkillID =
                savedFoodSkillID;

            g_SettingsChanged = true;
        }

        if (state.utilitySkillID !=
            savedUtilitySkillID)
        {
            state.utilitySkillID =
                savedUtilitySkillID;

            g_SettingsChanged = true;
        }
    }

    void RestoreCharacterConsumablesLocked(
        const std::string& characterName
    )
    {
        g_HasFood = false;
        g_HasUtility = false;
        g_HasMetabolicPrimer = false;
        g_HasUtilityPrimer = false;

        g_MetabolicPrimerDetectionState =
            ConsumableDetectionState::Unknown;

        g_UtilityPrimerDetectionState =
            ConsumableDetectionState::Unknown;

        g_FoodDetectionState =
            ConsumableDetectionState::Unknown;

        g_UtilityDetectionState =
            ConsumableDetectionState::Unknown;

        g_FoodDurationMilliseconds = 0;
        g_UtilityDurationMilliseconds = 0;
        g_MetabolicPrimerDurationMilliseconds = 0;
        g_UtilityPrimerDurationMilliseconds = 0;

        g_FoodSkillID = 0;
        g_UtilitySkillID = 0;

        g_FoodSkillName.clear();
        g_UtilitySkillName.clear();

        if (characterName.empty())
        {
            return;
        }

        const auto it =
            g_Settings.characterConsumables.find(
                characterName
            );

        if (it ==
            g_Settings.characterConsumables.end())
        {
            return;
        }

        CharacterConsumableState& state =
            it->second;

        if (state.foodRemainingSeconds > 0 &&
            state.foodSkillID != 0)
        {
            g_HasFood = true;
            g_FoodDetectionState =
                ConsumableDetectionState::Active;

            if (!state.foodStateKnown)
            {
                state.foodStateKnown = true;
                g_SettingsChanged = true;
            }

            g_FoodDurationMilliseconds =
                state.foodRemainingSeconds *
                1000;

            g_FoodReceivedTime =
                std::chrono::steady_clock::now();

            g_FoodSkillID =
                state.foodSkillID;

            if (g_FoodSkillID != 0)
            {
                g_FoodSkillName =
                    "Nourishment";
            }
        }
        else
        {
            g_FoodDetectionState =
                state.foodStateKnown
                ? ConsumableDetectionState::Missing
                : ConsumableDetectionState::Unknown;

            if (state.foodRemainingSeconds < 0)
            {
                state.foodRemainingSeconds = 0;
                g_SettingsChanged = true;
            }

            if (state.foodSkillID != 0)
            {
                state.foodSkillID = 0;
                g_SettingsChanged = true;
            }
        }

        if (state.utilityRemainingSeconds > 0 &&
            state.utilitySkillID != 0)
        {
            g_HasUtility = true;
            g_UtilityDetectionState =
                ConsumableDetectionState::Active;

            if (!state.utilityStateKnown)
            {
                state.utilityStateKnown = true;
                g_SettingsChanged = true;
            }

            g_UtilityDurationMilliseconds =
                state.utilityRemainingSeconds *
                1000;

            g_UtilityReceivedTime =
                std::chrono::steady_clock::now();

            g_UtilitySkillID =
                state.utilitySkillID;

            if (g_UtilitySkillID != 0)
            {
                g_UtilitySkillName =
                    "Enhancement";
            }
        }
        else
        {
            g_UtilityDetectionState =
                state.utilityStateKnown
                ? ConsumableDetectionState::Missing
                : ConsumableDetectionState::Unknown;

            if (state.utilityRemainingSeconds < 0)
            {
                state.utilityRemainingSeconds = 0;
                g_SettingsChanged = true;
            }

            if (state.utilitySkillID != 0)
            {
                state.utilitySkillID = 0;
                g_SettingsChanged = true;
            }
        }

        if (state.metabolicPrimerRemainingSeconds > 0)
        {
            g_HasMetabolicPrimer = true;
            g_MetabolicPrimerDetectionState =
                ConsumableDetectionState::Active;

            g_MetabolicPrimerDurationMilliseconds =
                state.metabolicPrimerRemainingSeconds *
                1000;

            g_MetabolicPrimerReceivedTime =
                std::chrono::steady_clock::now();

            if (!state.metabolicPrimerStateKnown)
            {
                state.metabolicPrimerStateKnown = true;
                g_SettingsChanged = true;
            }
        }
        else
        {
            g_MetabolicPrimerDetectionState =
                state.metabolicPrimerStateKnown
                ? ConsumableDetectionState::Missing
                : ConsumableDetectionState::Unknown;

            if (state.metabolicPrimerRemainingSeconds < 0)
            {
                state.metabolicPrimerRemainingSeconds = 0;
                g_SettingsChanged = true;
            }
        }

        if (state.utilityPrimerRemainingSeconds > 0)
        {
            g_HasUtilityPrimer = true;
            g_UtilityPrimerDetectionState =
                ConsumableDetectionState::Active;

            g_UtilityPrimerDurationMilliseconds =
                state.utilityPrimerRemainingSeconds *
                1000;

            g_UtilityPrimerReceivedTime =
                std::chrono::steady_clock::now();

            if (!state.utilityPrimerStateKnown)
            {
                state.utilityPrimerStateKnown = true;
                g_SettingsChanged = true;
            }
        }
        else
        {
            g_UtilityPrimerDetectionState =
                state.utilityPrimerStateKnown
                ? ConsumableDetectionState::Missing
                : ConsumableDetectionState::Unknown;

            if (state.utilityPrimerRemainingSeconds < 0)
            {
                state.utilityPrimerRemainingSeconds = 0;
                g_SettingsChanged = true;
            }
        }
    }
}

void BuffTracker::Reset()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    g_TotalEventCount = 0;
    g_BuffLikeEventCount = 0;
    g_RecentBuffEvents.clear();
}

void BuffTracker::ProcessEvent(
    const EvCombatData* combatData
)
{
    if (combatData == nullptr)
    {
        return;
    }

    //
    // ArcDPS agent add/remove events.
    // These arrive with ev == nullptr.
    //
    if (combatData->ev == nullptr)
    {
        std::lock_guard<std::mutex> lock(
            g_BuffMutex
        );

        if (combatData->src == nullptr)
        {
            return;
        }

        //
        // Ignore targeted-agent notifications.
        //
        if (combatData->src->Elite == 1)
        {
            return;
        }

        //
        // Agent added.
        //
        if (combatData->src->Profession != 0 &&
            combatData->dst != nullptr &&
            combatData->dst->IsSelf != 0)
        {
            const uintptr_t newSelfAgentID =
                combatData->src->ID;

            const std::string newCharacterName =
                combatData->src->Name != nullptr
                ? combatData->src->Name
                : "";

            const bool hadIdentity =
                g_SelfAgentID != 0 ||
                !g_SelfCharacterName.empty();

            const bool characterChanged =
                !newCharacterName.empty() &&
                !g_SelfCharacterName.empty() &&
                newCharacterName !=
                g_SelfCharacterName;

            const bool agentChanged =
                newSelfAgentID != 0 &&
                g_SelfAgentID != 0 &&
                newSelfAgentID !=
                g_SelfAgentID;

            const bool identityChanged =
                characterChanged ||
                agentChanged;

            //
            // Freeze the old character's current
            // remaining Food/Utility time and IDs
            // before changing identity.
            //
            if (identityChanged)
            {
                SaveCurrentCharacterConsumablesLocked();

                g_HasFood = false;
                g_HasUtility = false;
                g_HasMetabolicPrimer = false;
                g_HasUtilityPrimer = false;

                g_MetabolicPrimerDetectionState =
                    ConsumableDetectionState::Unknown;

                g_UtilityPrimerDetectionState =
                    ConsumableDetectionState::Unknown;

                g_FoodDetectionState =
                    ConsumableDetectionState::Unknown;

                g_UtilityDetectionState =
                    ConsumableDetectionState::Unknown;

                g_FoodDurationMilliseconds = 0;
                g_UtilityDurationMilliseconds = 0;
                g_MetabolicPrimerDurationMilliseconds = 0;
                g_UtilityPrimerDurationMilliseconds = 0;

                g_FoodSkillID = 0;
                g_UtilitySkillID = 0;

                g_FoodSkillName.clear();
                g_UtilitySkillName.clear();

                g_IsInCombat = false;
            }

            if (newSelfAgentID != 0)
            {
                g_SelfAgentID =
                    newSelfAgentID;
            }

            if (!newCharacterName.empty())
            {
                g_SelfCharacterName =
                    newCharacterName;
            }

            //
            // Restore only on first identification
            // or a real identity/agent change.
            //
            // Do not restore on duplicate agent
            // notifications because that would
            // rewind a running timer.
            //
            if ((!hadIdentity ||
                identityChanged) &&
                !g_SelfCharacterName.empty())
            {
                RestoreCharacterConsumablesLocked(
                    g_SelfCharacterName
                );
            }

            return;
        }

        //
        // Agent removed.
        //
        if (combatData->src->Profession == 0 &&
            g_SelfAgentID != 0 &&
            combatData->src->ID ==
            g_SelfAgentID)
        {
            //
            // Freeze remaining Food/Utility time
            // and IDs before clearing identity.
            //

            SaveCurrentCharacterConsumablesLocked();

            g_HasFood = false;
            g_HasUtility = false;

            g_FoodDetectionState =
                ConsumableDetectionState::Unknown;

            g_UtilityDetectionState =
                ConsumableDetectionState::Unknown;

            g_FoodDurationMilliseconds = 0;
            g_UtilityDurationMilliseconds = 0;

            g_FoodSkillID = 0;
            g_UtilitySkillID = 0;

            g_FoodSkillName.clear();
            g_UtilitySkillName.clear();

            g_IsInCombat = false;

            g_SelfAgentID = 0;
            g_SelfCharacterName.clear();
        }

        return;
    }

    const ArcDPS::CombatEvent& ev =
        *combatData->ev;

    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    ++g_TotalEventCount;

    const bool sourceIsSelf =
        combatData->src != nullptr &&
        combatData->src->IsSelf != 0;

    const bool destinationIsSelf =
        combatData->dst != nullptr &&
        combatData->dst->IsSelf != 0;

    uintptr_t currentSelfAgentID = 0;
    std::string currentSelfCharacterName;

    if (sourceIsSelf &&
        combatData->src != nullptr)
    {
        currentSelfAgentID =
            combatData->src->ID;

        if (combatData->src->Name != nullptr)
        {
            currentSelfCharacterName =
                combatData->src->Name;
        }
    }
    else if (
        destinationIsSelf &&
        combatData->dst != nullptr)
    {
        currentSelfAgentID =
            combatData->dst->ID;

        if (combatData->dst->Name != nullptr)
        {
            currentSelfCharacterName =
                combatData->dst->Name;
        }
    }

    const bool hadIdentity =
        g_SelfAgentID != 0 ||
        !g_SelfCharacterName.empty();

    const bool characterNameChanged =
        !currentSelfCharacterName.empty() &&
        !g_SelfCharacterName.empty() &&
        currentSelfCharacterName !=
        g_SelfCharacterName;

    const bool agentChanged =
        currentSelfAgentID != 0 &&
        g_SelfAgentID != 0 &&
        currentSelfAgentID !=
        g_SelfAgentID;

    const bool identityChanged =
        characterNameChanged ||
        agentChanged;

    if (identityChanged)
    {
        //
        // Freeze old character state before
        // switching to the new identity.
        //
        SaveCurrentCharacterConsumablesLocked();

        g_HasFood = false;
        g_HasUtility = false;

        g_FoodDetectionState =
            ConsumableDetectionState::Unknown;

        g_UtilityDetectionState =
            ConsumableDetectionState::Unknown;

        g_FoodDurationMilliseconds = 0;
        g_UtilityDurationMilliseconds = 0;

        g_FoodSkillID = 0;
        g_UtilitySkillID = 0;

        g_FoodSkillName.clear();
        g_UtilitySkillName.clear();

        g_IsInCombat = false;
    }

    if (currentSelfAgentID != 0)
    {
        g_SelfAgentID =
            currentSelfAgentID;
    }

    if (!currentSelfCharacterName.empty())
    {
        g_SelfCharacterName =
            currentSelfCharacterName;
    }

    if ((!hadIdentity ||
        identityChanged) &&
        !g_SelfCharacterName.empty())
    {
        RestoreCharacterConsumablesLocked(
            g_SelfCharacterName
        );
    }

    //
    // Combat state tracking.
    //
    if (sourceIsSelf)
    {
        if (ev.IsStatechange ==
            STATECHANGE_ENTER_COMBAT)
        {
            g_IsInCombat = true;
        }
        else if (
            ev.IsStatechange ==
            STATECHANGE_EXIT_COMBAT)
        {
            g_IsInCombat = false;
        }
    }

    //
    // Food / Utility / Primer tracking.
    //
    if (destinationIsSelf)
    {
        const std::string skillName =
            combatData->skillname != nullptr
            ? combatData->skillname
            : "";
        const ConsumableInfo& detectedFoodInfo =
            ConsumableData::GetFoodInfo(
                ev.SkillID
            );

        const bool isKnownFoodEffect =
            std::string(
                detectedFoodInfo.label
            ) != "Unknown";

        const ConsumableInfo& detectedUtilityInfo =
            ConsumableData::GetUtilityInfo(
                ev.SkillID
            );

        const bool isKnownUtilityEffect =
            std::string(
                detectedUtilityInfo.label
            ) != "Unknown";

        const bool isFoodEvent =
            ev.SkillID != 34210 &&
            (
                skillName == "Nourishment" ||
                skillName == "Home-Cooked Nourishment" ||
                isKnownFoodEffect
                );

        const bool isUtilityEvent =
            skillName == "Enhancement" ||
            isKnownUtilityEffect;

        const bool isMetabolicPrimerEvent =
            skillName == "Metabolic Primer";

        const bool isUtilityPrimerEvent =
            skillName == "Utility Primer";

        const bool isCandyCaneEvent =
            ev.SkillID == 34210;

        const bool hasDuration =
            ev.Value > 0;

        const bool isRemoved =
            ev.IsBuffRemove != 0;

        if (isCandyCaneEvent &&
            isRemoved)
        {
            //
            // Candy Cane / Minty Breath can coexist with
            // normal Nourishment and stacks duration.
            // ArcDPS may report removal for an individual
            // application while stacked duration remains,
            // so do not clear the dedicated timer here.
            //
            return;
        }

        if (isFoodEvent &&
            isRemoved)
        {
            g_HasFood = false;
            g_FoodDetectionState =
                ConsumableDetectionState::Missing;
            g_FoodDurationMilliseconds = 0;

            g_FoodSkillID = 0;
            g_FoodSkillName.clear();

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.foodRemainingSeconds = 0;
                state.foodSkillID = 0;
                state.foodStateKnown = true;

                g_SettingsChanged = true;
            }

            return;
        }

        if (isUtilityEvent &&
            isRemoved)
        {
            g_HasUtility = false;
            g_UtilityDetectionState =
                ConsumableDetectionState::Missing;
            g_UtilityDurationMilliseconds = 0;

            g_UtilitySkillID = 0;
            g_UtilitySkillName.clear();

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.utilityRemainingSeconds = 0;
                state.utilitySkillID = 0;
                state.utilityStateKnown = true;

                g_SettingsChanged = true;
            }

            return;
        }

        if (isMetabolicPrimerEvent &&
            isRemoved)
        {
            g_HasMetabolicPrimer = false;
            g_MetabolicPrimerDetectionState =
                ConsumableDetectionState::Missing;
            g_MetabolicPrimerDurationMilliseconds = 0;

            g_Settings.metabolicPrimerExpiresAt = 0;

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.metabolicPrimerRemainingSeconds = 0;
                state.metabolicPrimerStateKnown = true;
            }

            g_SettingsChanged = true;

            return;
        }

        if (isUtilityPrimerEvent &&
            isRemoved)
        {
            g_HasUtilityPrimer = false;
            g_UtilityPrimerDetectionState =
                ConsumableDetectionState::Missing;
            g_UtilityPrimerDurationMilliseconds = 0;

            g_Settings.utilityPrimerExpiresAt = 0;

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.utilityPrimerRemainingSeconds = 0;
                state.utilityPrimerStateKnown = true;
            }

            g_SettingsChanged = true;

            return;
        }

        if (isCandyCaneEvent &&
            hasDuration)
        {


            const auto now =
                std::chrono::steady_clock::now();

            int64_t previousRemainingMilliseconds = 0;

            if (g_HasCandyCane &&
                g_CandyCaneDurationMilliseconds > 0)
            {
                const auto elapsed =
                    std::chrono::duration_cast<
                    std::chrono::milliseconds
                    >(
                        now -
                        g_CandyCaneReceivedTime
                    ).count();

                const int64_t remaining =
                    g_CandyCaneDurationMilliseconds -
                    elapsed;

                if (remaining > 0)
                {
                    previousRemainingMilliseconds =
                        remaining;
                }
            }

            g_HasCandyCane = true;

            if (!g_HasCandyCane)
            {
                //
                // ArcDPS does not expose the initial Candy Cane
                // application through this event path. The first
                // event we receive occurs when the second stack
                // is applied, so account for both stacks.
                //
                g_CandyCaneDurationMilliseconds =
                    static_cast<int64_t>(
                        ev.Value
                        ) * 2;
            }
            else
            {
                g_CandyCaneDurationMilliseconds =
                    previousRemainingMilliseconds +
                    static_cast<int64_t>(
                        ev.Value
                        );
            }

            g_CandyCaneReceivedTime = now;

            return;
        }

        if (isFoodEvent &&
            hasDuration)
        {
            const bool wasAlreadyActive =
                g_HasFood;

            const bool sameConsumable =
                wasAlreadyActive &&
                g_FoodSkillID == ev.SkillID;
            int64_t previousRemainingMilliseconds = 0;

            if (wasAlreadyActive)
            {
                const auto previousElapsed =
                    std::chrono::duration_cast<
                    std::chrono::milliseconds
                    >(
                        std::chrono::steady_clock::now() -
                        g_FoodReceivedTime
                    ).count();

                const int64_t remaining =
                    g_FoodDurationMilliseconds -
                    previousElapsed;

                if (remaining > 0)
                {
                    previousRemainingMilliseconds =
                        remaining;
                }
            }

            g_HasFood = true;
            g_FoodDetectionState =
                ConsumableDetectionState::Active;

            g_FoodDurationMilliseconds =
                static_cast<int64_t>(
                    ev.Value
                    );

            g_FoodReceivedTime =
                std::chrono::steady_clock::now();

            g_FoodSkillID =
                ev.SkillID;

            g_FoodSkillName =
                skillName;

            SessionTracker::RecordFoodApplication(
                ev.SkillID,
                previousRemainingMilliseconds
            );

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.foodRemainingSeconds =
                    (
                        g_FoodDurationMilliseconds +
                        999
                        ) /
                    1000;

                state.foodSkillID =
                    g_FoodSkillID;

                state.foodStateKnown = true;

                g_SettingsChanged = true;
            }
        }
        else if (
            isUtilityEvent &&
            hasDuration)
        {
            const bool wasAlreadyActive =
                g_HasUtility;

            const bool sameConsumable =
                wasAlreadyActive &&
                g_UtilitySkillID == ev.SkillID;

            int64_t previousRemainingMilliseconds = 0;

            if (wasAlreadyActive)
            {
                const auto previousElapsed =
                    std::chrono::duration_cast<
                    std::chrono::milliseconds
                    >(
                        std::chrono::steady_clock::now() -
                        g_UtilityReceivedTime
                    ).count();

                const int64_t remaining =
                    g_UtilityDurationMilliseconds -
                    previousElapsed;

                if (remaining > 0)
                {
                    previousRemainingMilliseconds =
                        remaining;
                }
            }

            g_HasUtility = true;
            g_UtilityDetectionState =
                ConsumableDetectionState::Active;

            g_UtilityDurationMilliseconds =
                static_cast<int64_t>(
                    ev.Value
                    );

            g_UtilityReceivedTime =
                std::chrono::steady_clock::now();

            g_UtilitySkillID =
                ev.SkillID;

            g_UtilitySkillName =
                skillName;

            SessionTracker::RecordUtilityApplication(
                ev.SkillID,
                previousRemainingMilliseconds
            );
            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.utilityRemainingSeconds =
                    (
                        g_UtilityDurationMilliseconds +
                        999
                        ) /
                    1000;

                state.utilitySkillID =
                    g_UtilitySkillID;

                state.utilityStateKnown = true;

                g_SettingsChanged = true;
            }
        }
        else if (
            isMetabolicPrimerEvent &&
            hasDuration)
        {
            g_HasMetabolicPrimer = true;
            g_MetabolicPrimerDetectionState =
                ConsumableDetectionState::Active;

            g_MetabolicPrimerDurationMilliseconds =
                static_cast<int64_t>(
                    ev.Value
                    );

            g_MetabolicPrimerReceivedTime =
                std::chrono::steady_clock::now();

            const int64_t durationSeconds =
                (
                    g_MetabolicPrimerDurationMilliseconds +
                    999
                    ) /
                1000;

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.metabolicPrimerRemainingSeconds =
                    durationSeconds;

                state.metabolicPrimerStateKnown = true;
            }

            g_Settings.metabolicPrimerExpiresAt = 0;
            g_SettingsChanged = true;
        }
        else if (
            isUtilityPrimerEvent &&
            hasDuration)
        {
            g_HasUtilityPrimer = true;
            g_UtilityPrimerDetectionState =
                ConsumableDetectionState::Active;

            g_UtilityPrimerDurationMilliseconds =
                static_cast<int64_t>(
                    ev.Value
                    );

            g_UtilityPrimerReceivedTime =
                std::chrono::steady_clock::now();

            const int64_t durationSeconds =
                (
                    g_UtilityPrimerDurationMilliseconds +
                    999
                    ) /
                1000;

            if (!g_SelfCharacterName.empty())
            {
                CharacterConsumableState& state =
                    g_Settings.characterConsumables[
                        g_SelfCharacterName
                    ];

                state.utilityPrimerRemainingSeconds =
                    durationSeconds;

                state.utilityPrimerStateKnown = true;
            }

            g_Settings.utilityPrimerExpiresAt = 0;
            g_SettingsChanged = true;
        }
    }

    //
    // Developer debug event tracking.
    //
    const bool isBuffLike =
        (ev.Buff != 0 &&
            ev.BuffDamage >= 0) ||
        ev.IsBuffRemove != 0 ||
        ev.IsStatechange == 18;

    if (!isBuffLike)
    {
        return;
    }

    ++g_BuffLikeEventCount;

    BuffEventDebug record;

    record.eventID =
        combatData->id;

    record.skillID =
        ev.SkillID;

    if (combatData->skillname != nullptr)
    {
        record.skillName =
            combatData->skillname;
    }
    else
    {
        record.skillName =
            "Unknown";
    }

    record.value =
        ev.Value;

    record.buffDamage =
        ev.BuffDamage;

    record.overstackValue =
        ev.OverstackValue;

    record.buff =
        ev.Buff;

    record.buffRemove =
        ev.IsBuffRemove;

    record.stateChange =
        ev.IsStatechange;

    record.sourceIsSelf =
        sourceIsSelf;

    record.destinationIsSelf =
        destinationIsSelf;

    g_RecentBuffEvents.push_back(
        record
    );

    if (g_RecentBuffEvents.size() >
        MAX_DEBUG_EVENTS)
    {
        g_RecentBuffEvents.erase(
            g_RecentBuffEvents.begin()
        );
    }
}

uint64_t BuffTracker::GetTotalEventCount()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_TotalEventCount;
}

uint64_t BuffTracker::GetBuffLikeEventCount()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_BuffLikeEventCount;
}

std::vector<BuffEventDebug>
BuffTracker::GetRecentBuffEvents()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_RecentBuffEvents;
}

ConsumableDetectionState
BuffTracker::GetFoodDetectionState()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_FoodDetectionState;
}

ConsumableDetectionState
BuffTracker::GetUtilityDetectionState()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_UtilityDetectionState;
}

bool BuffTracker::HasCandyCane()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    if (!g_HasCandyCane)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now() -
            g_CandyCaneReceivedTime
        ).count();

    if (elapsed >=
        g_CandyCaneDurationMilliseconds)
    {
        g_HasCandyCane = false;
        g_CandyCaneDurationMilliseconds = 0;
        return false;
    }

    return true;
}

int64_t BuffTracker::GetCandyCaneRemainingMilliseconds()

{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    if (!g_HasCandyCane ||
        g_CandyCaneDurationMilliseconds <= 0)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now() -
            g_CandyCaneReceivedTime
        ).count();

    const int64_t remaining =
        g_CandyCaneDurationMilliseconds -
        elapsed;

    if (remaining <= 0)
    {
        g_HasCandyCane = false;
        g_CandyCaneDurationMilliseconds = 0;
        return 0;
    }

    return remaining;
}


bool BuffTracker::HasFood()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    if (!g_HasFood)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now() -
            g_FoodReceivedTime
        ).count();

    if (elapsed >=
        g_FoodDurationMilliseconds)
    {
        SessionTracker::RecordFoodExpired(
            g_IsInCombat
        );

        g_HasFood = false;
        g_FoodDetectionState =
            ConsumableDetectionState::Missing;
        g_FoodDurationMilliseconds = 0;

        g_FoodSkillID = 0;
        g_FoodSkillName.clear();

        if (!g_SelfCharacterName.empty())
        {
            CharacterConsumableState& state =
                g_Settings.characterConsumables[
                    g_SelfCharacterName
                ];

            state.foodRemainingSeconds = 0;
            state.foodSkillID = 0;
            state.foodStateKnown = true;

            g_SettingsChanged = true;
        }

        return false;
    }

    return true;
}

bool BuffTracker::HasUtility()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    if (!g_HasUtility)
    {
        return false;
    }

    const auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now() -
            g_UtilityReceivedTime
        ).count();

    if (elapsed >=
        g_UtilityDurationMilliseconds)
    {
        SessionTracker::RecordUtilityExpired(
            g_IsInCombat
        );

        g_HasUtility = false;
        g_UtilityDetectionState =
            ConsumableDetectionState::Missing;
        g_UtilityDurationMilliseconds = 0;

        g_UtilitySkillID = 0;
        g_UtilitySkillName.clear();

        if (!g_SelfCharacterName.empty())
        {
            CharacterConsumableState& state =
                g_Settings.characterConsumables[
                    g_SelfCharacterName
                ];

            state.utilityRemainingSeconds = 0;
            state.utilitySkillID = 0;
            state.utilityStateKnown = true;

            g_SettingsChanged = true;
        }

        return false;
    }

    return true;
}

bool BuffTracker::HasMetabolicPrimer()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    const int64_t remaining =
        GetDirectMetabolicPrimerRemainingMillisecondsLocked();

    if (remaining > 0)
    {
        return true;
    }

    if (g_HasMetabolicPrimer ||
        g_MetabolicPrimerDurationMilliseconds != 0 ||
        g_Settings.metabolicPrimerExpiresAt != 0)
    {
        g_HasMetabolicPrimer = false;
        g_MetabolicPrimerDetectionState =
            ConsumableDetectionState::Missing;
        g_MetabolicPrimerDurationMilliseconds = 0;

        g_Settings.metabolicPrimerExpiresAt = 0;

        if (!g_SelfCharacterName.empty())
        {
            CharacterConsumableState& state =
                g_Settings.characterConsumables[
                    g_SelfCharacterName
                ];

            state.metabolicPrimerRemainingSeconds = 0;
            state.metabolicPrimerStateKnown = true;
        }

        g_SettingsChanged = true;
    }

    return false;
}

bool BuffTracker::HasUtilityPrimer()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    const int64_t remaining =
        GetDirectUtilityPrimerRemainingMillisecondsLocked();

    if (remaining > 0)
    {
        return true;
    }

    if (g_HasUtilityPrimer ||
        g_UtilityPrimerDurationMilliseconds != 0 ||
        g_Settings.utilityPrimerExpiresAt != 0)
    {
        g_HasUtilityPrimer = false;
        g_UtilityPrimerDetectionState =
            ConsumableDetectionState::Missing;
        g_UtilityPrimerDurationMilliseconds = 0;

        g_Settings.utilityPrimerExpiresAt = 0;

        if (!g_SelfCharacterName.empty())
        {
            CharacterConsumableState& state =
                g_Settings.characterConsumables[
                    g_SelfCharacterName
                ];

            state.utilityPrimerRemainingSeconds = 0;
            state.utilityPrimerStateKnown = true;
        }

        g_SettingsChanged = true;
    }

    return false;
}

ConsumableDetectionState
BuffTracker::GetMetabolicPrimerDetectionState()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_MetabolicPrimerDetectionState;
}

ConsumableDetectionState
BuffTracker::GetUtilityPrimerDetectionState()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_UtilityPrimerDetectionState;
}

bool BuffTracker::IsInCombat()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_IsInCombat;
}

int64_t BuffTracker::GetFoodRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    if (!g_HasFood)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now() -
            g_FoodReceivedTime
        ).count();

    const int64_t remaining =
        g_FoodDurationMilliseconds -
        elapsed;

    return remaining > 0
        ? remaining
        : 0;
}

int64_t BuffTracker::GetUtilityRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    if (!g_HasUtility)
    {
        return 0;
    }

    const auto elapsed =
        std::chrono::duration_cast<
        std::chrono::milliseconds
        >(
            std::chrono::steady_clock::now() -
            g_UtilityReceivedTime
        ).count();

    const int64_t remaining =
        g_UtilityDurationMilliseconds -
        elapsed;

    return remaining > 0
        ? remaining
        : 0;
}

int64_t BuffTracker::
GetMetabolicPrimerRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return
        GetDirectMetabolicPrimerRemainingMillisecondsLocked();
}

int64_t BuffTracker::
GetUtilityPrimerRemainingMilliseconds()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return
        GetDirectUtilityPrimerRemainingMillisecondsLocked();
}

uint32_t BuffTracker::GetFoodSkillID()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_FoodSkillID;
}

uint32_t BuffTracker::GetUtilitySkillID()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_UtilitySkillID;
}

std::string BuffTracker::GetFoodSkillName()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_FoodSkillName;
}

std::string BuffTracker::GetUtilitySkillName()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    return g_UtilitySkillName;
}

void BuffTracker::RestorePrimerState()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    //
    // Primers are character-specific. At addon load we do not yet
    // know which character is active, so do not restore a global
    // Primer timer here. Per-character Primer state is restored when
    // ArcDPS identifies the self character.
    //
    g_HasMetabolicPrimer = false;
    g_HasUtilityPrimer = false;

    g_MetabolicPrimerDetectionState =
        ConsumableDetectionState::Unknown;

    g_UtilityPrimerDetectionState =
        ConsumableDetectionState::Unknown;

    g_MetabolicPrimerDurationMilliseconds = 0;
    g_UtilityPrimerDurationMilliseconds = 0;

    // Clear obsolete global timestamps so they cannot leak Primer
    // state onto a different character.
    if (g_Settings.metabolicPrimerExpiresAt != 0)
    {
        g_Settings.metabolicPrimerExpiresAt = 0;
        g_SettingsChanged = true;
    }

    if (g_Settings.utilityPrimerExpiresAt != 0)
    {
        g_Settings.utilityPrimerExpiresAt = 0;
        g_SettingsChanged = true;
    }
}

void BuffTracker::SavePrimerState()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    //
    // Also freeze the current character's
    // Food/Utility state and IDs before unload.
    //
    SaveCurrentCharacterConsumablesLocked();

    // Food, Utility, and Primer remaining timers are stored
    // per character by SaveCurrentCharacterConsumablesLocked().
}

bool BuffTracker::ConsumeSettingsChanged()
{
    std::lock_guard<std::mutex> lock(
        g_BuffMutex
    );

    const bool changed =
        g_SettingsChanged;

    g_SettingsChanged = false;

    return changed;
}
