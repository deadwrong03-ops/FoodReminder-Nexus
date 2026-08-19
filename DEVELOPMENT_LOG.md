# Food Reminder Development Log

This log records major development checkpoints, tests, discoveries, and known issues for the Food Reminder Nexus addon.

---

## 2026-08-19 — Nexus Foundation and Buff Tracking

### Project Foundation

- Created Food Reminder as a native C++ Nexus addon.
- Established separate `FoodReminder-Nexus` Git repository.
- Added Nexus addon loading/unloading.
- Added basic options interface.
- Added configurable food and utility warning values.
- Added Test Reminder functionality.
- Confirmed addon loads successfully in Guild Wars 2 through Nexus.

### ArcDPS Integration

- Added ArcDPS combat/buff event capture.
- Added development debug output for inspecting incoming ArcDPS events.
- Confirmed ArcDPS does not necessarily provide useful food/utility information immediately when entering the game.
- Confirmed relevant buff information becomes available through ArcDPS buff events.

### Food and Utility Detection

Initial detection used specific skill IDs:

- Nourishment: `10001`
- Enhancement: `9963`

Testing revealed this approach was insufficient.

A different food produced:

- Nourishment: `17825`

This demonstrated that food detection cannot depend on one hardcoded Nourishment skill ID.

Detection was changed to use ArcDPS skill names/categories:

- `Nourishment` → Food
- `Enhancement` → Utility

Hardcoded food/utility IDs were subsequently removed from the tracking logic.

### Duration Tracking

Confirmed ArcDPS provides buff duration values in milliseconds.

Example:

`3600000 ms = 60 minutes`

Food Reminder now stores the received duration and maintains its own countdown using a steady clock.

Live testing confirmed:

- Food countdown works.
- Utility countdown works.
- Food and utility can be tracked simultaneously.
- New consumables reset their respective timers.
- Different Nourishment IDs are recognized correctly.

### Refresh / Replacement Testing

Tested replacing active food and utility buffs before expiration.

Observed that replacement may arrive as a new positive-duration event rather than an explicit removal event.

Food Reminder correctly overwrites the previous duration and restarts the appropriate countdown.

This behavior was tested successfully in game.

### Expiration / Removal Handling

Added natural expiration handling.

When the locally tracked duration reaches zero:

- Food is no longer considered active.
- Utility is no longer considered active.
- Stored duration is cleared.

Added handling for ArcDPS buff-removal events when they are provided.

Further live testing of explicit removal events is still required.

### Important Finding

The existing/older Food Tracker addon used during comparison testing displayed stale food and utility durations after consumables were replaced.

Food Reminder correctly detected the new ArcDPS events and reset its timers.

Therefore, the older tracker should not be treated as the authoritative reference when its displayed timer conflicts with the live ArcDPS event data.

### Current Working Checkpoint

At the end of this session:

- Project builds successfully.
- Nexus loads the addon successfully.
- ArcDPS events are being received.
- Food is detected.
- Utility is detected.
- Different food IDs are supported.
- Food countdown works.
- Utility countdown works.
- Refresh/replacement works.
- Natural expiration logic is implemented.
- Buff-removal handling is implemented.
- In-game refresh/replacement testing passed.

### Git

Working lifecycle changes were committed and pushed to the `FoodReminder-Nexus` repository.

Visual Studio's Push command currently opens the "Create a Git repository" dialog despite the repository already having a valid GitHub `origin`.

Terminal `git push` works correctly and is currently the reliable push method.

---

## Next Development Steps

1. Continue expiration/removal testing.
2. Connect live countdown state to actual reminder triggering.
3. Verify food and utility warnings independently.
4. Improve reminder presentation.
5. Add settings persistence.
6. Remove or hide development/debug UI for release builds.
