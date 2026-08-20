#pragma once

#include <cstdint>

struct ConsumableInfo
{
    const char* label;
    const char* name;
    const char* effects;
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