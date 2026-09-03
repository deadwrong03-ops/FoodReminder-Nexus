#include "ReminderUI.h"

#include <string>

#include "imgui/imgui.h"

#include "ReminderManager.h"
#include "Settings.h"

void ReminderUI::Render(
    bool hasFood,
    int64_t foodRemaining,
    bool hasUtility,
    int64_t utilityRemaining
)
{
    if (
        !ReminderManager::
        IsReminderActive()
        )
    {
        return;
    }

    const ImVec2 displaySize =
        ImGui::GetIO().DisplaySize;

    const ImVec2 center(
        displaySize.x * 0.5f,
        displaySize.y * 0.25f
    );

    ImGui::SetNextWindowPos(
        center,
        ImGuiCond_FirstUseEver,
        ImVec2(0.5f, 0.5f)
    );

    ImGui::SetNextWindowBgAlpha(
        0.90f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(24.0f, 18.0f)
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowBorderSize,
        3.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(
            0.95f,
            0.15f,
            0.15f,
            1.00f
        )
    );

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (g_Settings.lockReminderPosition)
    {
        flags |=
            ImGuiWindowFlags_NoMove;
    }

    if (ImGui::Begin(
        "##FoodReminderAlert",
        nullptr,
        flags))
    {
        int64_t remainingMs = 0;

        const char* reminderTitle =
            ReminderManager::
            GetReminderTitle();

        const std::string title =
            reminderTitle;

        if (title ==
            "FOOD REMINDER")
        {
            remainingMs =
                foodRemaining;
        }
        else if (
            title ==
            "UTILITY REMINDER")
        {
            remainingMs =
                utilityRemaining;
        }
        else if (
            title ==
            "METABOLIC PRIMER EXPIRING" ||
            title ==
            "UTILITY PRIMER EXPIRING" ||
            title ==
            "PRIMERS EXPIRING")
        {
            remainingMs =
                ReminderManager::
                GetBuffRemainingMilliseconds();
        }
        else if (
            title ==
            "FOOD + UTILITY REMINDER")
        {
            if (hasFood &&
                hasUtility)
            {
                remainingMs =
                    foodRemaining <
                    utilityRemaining
                    ? foodRemaining
                    : utilityRemaining;
            }
        }

        const int64_t remainingSeconds =
            remainingMs / 1000;

        const int64_t hours =
            remainingSeconds / 3600;

        const int64_t minutes =
            (remainingSeconds % 3600) /
            60;

        const int64_t seconds =
            remainingSeconds % 60;

        ImGui::SetWindowFontScale(1.35f);

        ImGui::TextUnformatted(
            ReminderManager::
            GetReminderTitle()
        );

        ImGui::Separator();

        const bool isMissingBuffReminder =
            title == "FOOD MISSING" ||
            title == "UTILITY MISSING" ||
            title ==
            "FOOD + UTILITY MISSING";

        if (isMissingBuffReminder)
        {
            ImGui::SetWindowFontScale(1.15f);

            ImGui::TextUnformatted(
                ReminderManager::
                GetReminderMessage()
            );
        }
        else
        {
            ImGui::SetWindowFontScale(1.20f);

            ImGui::Text(
                "%02lld:%02lld:%02lld remaining",
                hours,
                minutes,
                seconds
            );
        }

        ImGui::SetWindowFontScale(1.0f);
    }

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}
