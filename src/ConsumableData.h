#pragma once

#include <cstdint>

struct ConsumableInfo
{
    const char* label;
    const char* name;
    const char* effects;

    uint32_t itemID = 0;
};

namespace ConsumableData
{
    const ConsumableInfo& GetFoodInfo(
        uint32_t skillID
    );

    const ConsumableInfo& GetUtilityInfo(
        uint32_t skillID
    );
}