#include "Settings.h"

#include <Windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

FoodReminderSettings g_Settings{};

namespace
{
    std::filesystem::path GetSettingsPath(
        void* moduleHandle
    )
    {
        wchar_t modulePath[MAX_PATH] = {};

        const DWORD length =
            GetModuleFileNameW(
                static_cast<HMODULE>(moduleHandle),
                modulePath,
                MAX_PATH
            );

        if (length == 0)
        {
            return {};
        }

        std::filesystem::path path(modulePath);

        return path.parent_path() /
            L"FoodReminder.ini";
    }

    bool ParseBool(const std::string& value)
    {
        return
            value == "1" ||
            value == "true" ||
            value == "True" ||
            value == "TRUE";
    }

    bool EndsWith(
        const std::string& value,
        const std::string& ending
    )
    {
        if (ending.size() > value.size())
        {
            return false;
        }

        return value.compare(
            value.size() - ending.size(),
            ending.size(),
            ending
        ) == 0;
    }
}

bool Settings::Load(void* moduleHandle)
{
    const std::filesystem::path path =
        GetSettingsPath(moduleHandle);

    if (path.empty())
    {
        return false;
    }

    std::ifstream file(path);

    if (!file.is_open())
    {
        return false;
    }

    g_Settings.characterConsumables.clear();
    g_Settings.unknownConsumables.clear();
    std::string line;

    while (std::getline(file, line))
    {
        const size_t separator =
            line.find('=');

        if (separator == std::string::npos)
        {
            continue;
        }

        const std::string key =
            line.substr(0, separator);

        const std::string value =
            line.substr(separator + 1);

        try
        {
            if (key == "enabled")
            {
                g_Settings.enabled =
                    ParseBool(value);
            }
            else if (key == "showTracker")
            {
                g_Settings.showTracker =
                    ParseBool(value);
            }
            else if (
                key == "lockTrackerPosition")
            {
                g_Settings.lockTrackerPosition =
                    ParseBool(value);
            }
            else if (
                key == "lockReminderPosition")
            {
                g_Settings.lockReminderPosition =
                    ParseBool(value);
            }
            else if (
                key == "foodWarningSeconds")
            {
                g_Settings.foodWarningSeconds =
                    std::stoi(value);
            }
            else if (
                key == "utilityWarningSeconds")
            {
                g_Settings.utilityWarningSeconds =
                    std::stoi(value);
            }
            else if (
                key == "metabolicPrimerWarningSeconds")
            {
                g_Settings.metabolicPrimerWarningSeconds =
                    std::stoi(value);
            }
            else if (
                key == "utilityPrimerWarningSeconds")
            {
                g_Settings.utilityPrimerWarningSeconds =
                    std::stoi(value);
            }
            else if (
                key == "metabolicPrimerExpiresAt")
            {
                g_Settings.metabolicPrimerExpiresAt =
                    std::stoll(value);
            }
            else if (
                key == "utilityPrimerExpiresAt")
            {
                g_Settings.utilityPrimerExpiresAt =
                    std::stoll(value);
            }
            else if (
                key.rfind("character.", 0) == 0)
            {
                const std::string foodRemainingSuffix =
                    ".foodRemainingSeconds";

                const std::string utilityRemainingSuffix =
                    ".utilityRemainingSeconds";

                const std::string foodStateKnownSuffix =
                    ".foodStateKnown";

                const std::string utilityStateKnownSuffix =
                    ".utilityStateKnown";

                const std::string metabolicPrimerStateKnownSuffix =
                    ".metabolicPrimerStateKnown";

                const std::string utilityPrimerStateKnownSuffix =
                    ".utilityPrimerStateKnown";

                const std::string metabolicPrimerRemainingSuffix =
                    ".metabolicPrimerRemainingSeconds";

                const std::string utilityPrimerRemainingSuffix =
                    ".utilityPrimerRemainingSeconds";

                const std::string foodSkillSuffix =
                    ".foodSkillID";

                const std::string utilitySkillSuffix =
                    ".utilitySkillID";

                const size_t nameStart =
                    std::string("character.").size();

                if (EndsWith(
                    key,
                    foodRemainingSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        foodRemainingSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .foodRemainingSeconds =
                            std::stoll(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        utilityRemainingSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        utilityRemainingSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .utilityRemainingSeconds =
                            std::stoll(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        foodStateKnownSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        foodStateKnownSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .foodStateKnown =
                            ParseBool(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        utilityStateKnownSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        utilityStateKnownSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .utilityStateKnown =
                            ParseBool(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        metabolicPrimerStateKnownSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        metabolicPrimerStateKnownSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .metabolicPrimerStateKnown =
                            ParseBool(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        utilityPrimerStateKnownSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        utilityPrimerStateKnownSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .utilityPrimerStateKnown =
                            ParseBool(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        metabolicPrimerRemainingSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        metabolicPrimerRemainingSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .metabolicPrimerRemainingSeconds =
                            std::stoll(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        utilityPrimerRemainingSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        utilityPrimerRemainingSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .utilityPrimerRemainingSeconds =
                            std::stoll(value);
                    }
                }
                else if (
                    EndsWith(
                        key,
                        foodSkillSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        foodSkillSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .foodSkillID =
                            static_cast<uint32_t>(
                                std::stoul(value)
                                );
                    }
                }
                else if (
                    EndsWith(
                        key,
                        utilitySkillSuffix))
                {
                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        utilitySkillSuffix.size();

                    const std::string characterName =
                        key.substr(
                            nameStart,
                            nameLength
                        );

                    if (!characterName.empty())
                    {
                        g_Settings
                            .characterConsumables[
                                characterName
                            ]
                            .utilitySkillID =
                            static_cast<uint32_t>(
                                std::stoul(value)
                                );
                    }
                }
            }
            else if (
                key.rfind("unknown.", 0) == 0)
            {
                const std::string typeSuffix =
                    ".type";

                const std::string seenSuffix =
                    ".seen";

                const size_t idStart =
                    std::string("unknown.").size();

                if (EndsWith(
                    key,
                    typeSuffix))
                {
                    const size_t idLength =
                        key.size() -
                        idStart -
                        typeSuffix.size();

                    const std::string idText =
                        key.substr(
                            idStart,
                            idLength
                        );

                    const uint32_t skillID =
                        static_cast<uint32_t>(
                            std::stoul(idText)
                            );

                    const bool isUtility =
                        value == "Utility";

                    const uint64_t storageKey =
                        (static_cast<uint64_t>(
                            isUtility ? 1 : 0
                            ) << 32) |
                        static_cast<uint64_t>(
                            skillID
                            );

                    SavedUnknownConsumable& unknown =
                        g_Settings.unknownConsumables[
                            storageKey
                        ];

                    unknown.skillID =
                        skillID;

                    unknown.isFood =
                        !isUtility;

                    unknown.isUtility =
                        isUtility;
                }
                else if (
                    EndsWith(
                        key,
                        seenSuffix))
                {
                    const size_t idLength =
                        key.size() -
                        idStart -
                        seenSuffix.size();

                    const std::string idText =
                        key.substr(
                            idStart,
                            idLength
                        );

                    const uint32_t skillID =
                        static_cast<uint32_t>(
                            std::stoul(idText)
                            );

                    for (
                        auto& entry :
                        g_Settings.unknownConsumables
                        )
                    {
                        SavedUnknownConsumable& unknown =
                            entry.second;

                        if (unknown.skillID ==
                            skillID)
                        {
                            unknown.seenCount =
                                std::stoull(value);

                            break;
                        }
                    }
                }
            }
        }
        catch (...)
        {
            // Ignore malformed values and keep
            // the current/default setting.
        }
    }

    g_Settings.foodWarningSeconds =
        std::clamp(
            g_Settings.foodWarningSeconds,
            60,
            3600
        );

    g_Settings.utilityWarningSeconds =
        std::clamp(
            g_Settings.utilityWarningSeconds,
            60,
            3600
        );

    g_Settings.metabolicPrimerWarningSeconds =
        std::clamp(
            g_Settings.metabolicPrimerWarningSeconds,
            300,
            3600
        );

    g_Settings.utilityPrimerWarningSeconds =
        std::clamp(
            g_Settings.utilityPrimerWarningSeconds,
            300,
            3600
        );

    return true;
}

bool Settings::Save(void* moduleHandle)
{
    const std::filesystem::path path =
        GetSettingsPath(moduleHandle);

    if (path.empty())
    {
        return false;
    }

    std::ofstream file(
        path,
        std::ios::trunc
    );

    if (!file.is_open())
    {
        return false;
    }

    file
        << "enabled="
        << (g_Settings.enabled ? 1 : 0)
        << '\n';

    file
        << "showTracker="
        << (g_Settings.showTracker ? 1 : 0)
        << '\n';

    file
        << "lockTrackerPosition="
        << (g_Settings.lockTrackerPosition ? 1 : 0)
        << '\n';

    file
        << "lockReminderPosition="
        << (g_Settings.lockReminderPosition ? 1 : 0)
        << '\n';

    file
        << "foodWarningSeconds="
        << g_Settings.foodWarningSeconds
        << '\n';

    file
        << "utilityWarningSeconds="
        << g_Settings.utilityWarningSeconds
        << '\n';

    file
        << "metabolicPrimerWarningSeconds="
        << g_Settings.metabolicPrimerWarningSeconds
        << '\n';

    file
        << "utilityPrimerWarningSeconds="
        << g_Settings.utilityPrimerWarningSeconds
        << '\n';

    file
        << "metabolicPrimerExpiresAt="
        << g_Settings.metabolicPrimerExpiresAt
        << '\n';

    file
        << "utilityPrimerExpiresAt="
        << g_Settings.utilityPrimerExpiresAt
        << '\n';

    for (const auto& entry :
        g_Settings.characterConsumables)
    {
        const std::string& characterName =
            entry.first;

        const CharacterConsumableState& state =
            entry.second;

        file
            << "character."
            << characterName
            << ".foodRemainingSeconds="
            << state.foodRemainingSeconds
            << '\n';

        file
            << "character."
            << characterName
            << ".utilityRemainingSeconds="
            << state.utilityRemainingSeconds
            << '\n';

        file
            << "character."
            << characterName
            << ".foodStateKnown="
            << (state.foodStateKnown ? 1 : 0)
            << '\n';

        file
            << "character."
            << characterName
            << ".utilityStateKnown="
            << (state.utilityStateKnown ? 1 : 0)
            << '\n';

        file
            << "character."
            << characterName
            << ".metabolicPrimerStateKnown="
            << (state.metabolicPrimerStateKnown ? 1 : 0)
            << '\n';

        file
            << "character."
            << characterName
            << ".utilityPrimerStateKnown="
            << (state.utilityPrimerStateKnown ? 1 : 0)
            << '\n';

        file
            << "character."
            << characterName
            << ".metabolicPrimerRemainingSeconds="
            << state.metabolicPrimerRemainingSeconds
            << '\n';

        file
            << "character."
            << characterName
            << ".utilityPrimerRemainingSeconds="
            << state.utilityPrimerRemainingSeconds
            << '\n';

        file
            << "character."
            << characterName
            << ".foodSkillID="
            << state.foodSkillID
            << '\n';

        file
            << "character."
            << characterName
            << ".utilitySkillID="
            << state.utilitySkillID
            << '\n';
    }
    for (const auto& entry :
        g_Settings.unknownConsumables)
    {
        const SavedUnknownConsumable& unknown =
            entry.second;

        file
            << "unknown."
            << unknown.skillID
            << ".type="
            << (unknown.isUtility
                ? "Utility"
                : "Food")
            << '\n';

        file
            << "unknown."
            << unknown.skillID
            << ".seen="
            << unknown.seenCount
            << '\n';
    }

    return true;
}