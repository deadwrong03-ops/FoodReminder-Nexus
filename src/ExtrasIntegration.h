#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ExtrasSquadMember
{
    std::string accountName;

    uint8_t subgroup = 0;

    bool ready = false;

    bool isCommander = false;
    bool isLieutenant = false;
};

namespace ExtrasIntegration
{
    bool IsAvailable();

    std::string GetVersion();

    std::vector<ExtrasSquadMember>
        GetSquadMembers();

    void Reset();
}
