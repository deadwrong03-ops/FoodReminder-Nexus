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
            30,
            7200
        );

    g_Settings.utilityWarningSeconds =
        std::clamp(
            g_Settings.utilityWarningSeconds,
            30,
            7200
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
        << "foodWarningSeconds="
        << g_Settings.foodWarningSeconds
        << '\n';

    file
        << "utilityWarningSeconds="
        << g_Settings.utilityWarningSeconds
        << '\n';

    return true;
}