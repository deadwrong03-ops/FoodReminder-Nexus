# Food Reminder Development Log

## 2026-08-18 — Nexus migration starter

- Started a clean C++ Nexus addon project.
- Based project structure on Raidcore's official Nexus C++ addon template.
- Added Nexus load/unload lifecycle.
- Added ImGui setup.
- Added Food Reminder options.
- Added test reminder overlay.
- Real buff detection intentionally deferred until the base addon loads and unloads cleanly in game.

### First test target

1. Build `Debug | x64`.
2. Load `FoodReminder.dll` through Nexus.
3. Confirm the addon appears in Nexus.
4. Confirm the Food Reminder options render.
5. Click `Test Reminder`.
6. Confirm the alert appears for five seconds.
7. Unload/reload the addon and confirm no crash.
