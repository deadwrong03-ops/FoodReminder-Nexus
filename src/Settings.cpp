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
            else if (
                key == "showTracker")
            {
                g_Settings.showTracker =
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
                const std::string foodSuffix =
                    ".foodExpiresAt";

                const std::string utilitySuffix =
                    ".utilityExpiresAt";

                if (EndsWith(key, foodSuffix))
                {
                    const size_t nameStart =
                        std::string("character.").size();

                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        foodSuffix.size();

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
                            .foodExpiresAt =
                            std::stoll(value);
                    }
                }
                else if (
                    EndsWith(key, utilitySuffix))
                {
                    const size_t nameStart =
                        std::string("character.").size();

                    const size_t nameLength =
                        key.size() -
                        nameStart -
                        utilitySuffix.size();

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
                            .utilityExpiresAt =
                            std::stoll(value);
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
            << ".foodExpiresAt="
            << state.foodExpiresAt
            << '\n';

        file
            << "character."
            << characterName
            << ".utilityExpiresAt="
            << state.utilityExpiresAt
            << '\n';
    }

    return true;
}