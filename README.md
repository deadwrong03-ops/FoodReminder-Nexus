# Food Reminder

Early-development Guild Wars 2 addon for the Raidcore Nexus addon framework.

## Current milestone

The starter project provides:

- Nexus addon load/unload entry points
- ImGui initialization
- Nexus options panel
- Enable/disable reminder setting
- Food and utility early-warning settings
- Test Reminder button
- A simple on-screen test reminder
- Clean source files ready for real food/utility tracking

No real food/utility buff detection is wired yet. That is the next development phase.

## First-time setup

1. Install Git and Visual Studio 2022 with **Desktop development with C++**.
2. Open a terminal in this folder.
3. Run `setup_submodules.bat`.
4. Open `FoodReminder.sln`.
5. Select **Debug** and **x64**.
6. Build the solution.
7. The DLL will be created as `x64\FoodReminder.dll`.
8. Copy it into `<Guild Wars 2>\addons\FoodReminder.dll`.
9. Launch Guild Wars 2 with Nexus.

## Important

The addon signature in `entry.cpp` is provisional for development. Change/register it before public release.

Version: 0.1.0 Development
