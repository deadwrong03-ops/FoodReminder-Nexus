#pragma once


#include <cstdint>
#include <string>
#include <vector>

#include "nexus/Nexus.h"

struct RTAPIGroupMemberSnapshot
{
    std::string accountName;
    std::string characterName;

    uint32_t subgroup = 0;
    uint32_t profession = 0;
    uint32_t eliteSpecialization = 0;

    bool isSelf = false;
    bool isInInstance = false;
    bool isCommander = false;
    bool isLieutenant = false;
};

namespace RTAPIIntegration
{
    void Start(AddonAPI_t* api);
    void Shutdown();

    // Refresh the DataLink pointer so RTAPI can be hot-loaded/unloaded.
    void Update();

    bool IsAvailable();

    // We only switch the Squad tab to the RTAPI roster after the
    // event-backed roster has caught up to RTAPI's reported member count.
    bool HasAuthoritativeRoster();

    uint32_t GetExpectedGroupMemberCount();

    std::vector<RTAPIGroupMemberSnapshot>
        GetGroupMembers();
}
