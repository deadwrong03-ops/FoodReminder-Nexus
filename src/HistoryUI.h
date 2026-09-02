#pragma once
#pragma once

#include <string>

namespace HistoryUI
{
    //
    // Renders the persistent Personal History tab.
    // currentCharacterName is supplied by entry.cpp so HistoryUI remains
    // presentation-only and does not own squad/self discovery.
    //
    void Render(
        const std::string& currentCharacterName
    );
}
