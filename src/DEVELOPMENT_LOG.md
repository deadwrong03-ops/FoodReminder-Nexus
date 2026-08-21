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

## 2026-08-20 — Reminder Expansion, Primers, Combat State, and Character Handling

### Reminder Popup Improvements

Expanded the reminder system beyond the original test popup.

Food Reminder now supports:

- Food expiration reminders.
- Utility expiration reminders.
- Combined Food + Utility expiration reminders.
- Live countdown display inside reminder popups.
- Five-second popup duration.
- Automatic re-arming when a refreshed buff returns above the configured warning threshold.

Reminder popup timing was tested successfully in game.

### Persistent Settings

Added persistent configuration using:

`FoodReminder.ini`

The following settings now persist:

- Enable/disable reminders.
- Food warning time.
- Utility warning time.
- Metabolic Primer warning time.
- Utility Primer warning time.
- Saved Primer expiration timestamps.

Settings are loaded when the addon starts and saved when changed.

Food and Utility early-warning ranges:

- Minimum: 1 minute
- Maximum: 60 minutes

Primer early-warning ranges:

- Minimum: 5 minutes
- Maximum: 60 minutes

Default Primer warning time:

- 30 minutes

Persistence was verified by changing values, closing/reopening the options interface, and confirming the configured values remained.

### Reminder Timing Interface

The Reminder Timing section was reorganized for clarity.

The options are now grouped into:

#### Food & Utility

- Food early warning
- Utility early warning

#### Primers

- Metabolic Primer early warning
- Utility Primer early warning

This was a UI-only cleanup and did not change reminder behavior.

### Metabolic and Utility Primer Detection

ArcDPS testing identified direct Primer events.

Confirmed:

#### Metabolic Primer

- Name: `Metabolic Primer`
- Skill ID: `21487`
- Approximately 12-hour duration

#### Utility Primer

- Name: `Utility Primer`
- Skill ID: `21579`
- Approximately 12-hour duration

Primer tracking was added using the same steady-clock duration approach used for Food and Utility.

### Primer Persistence

Primer expiration timestamps are saved as Unix timestamps.

When a directly observed Primer event occurs, Food Reminder stores its expected expiration time.

On addon reload, saved Primer expiration timestamps can be reconstructed into a live remaining-duration timer.

Disk writes were optimized so Primer settings are only written when Primer state actually changes rather than on every ArcDPS combat event.

### Important Primer Discovery

Testing showed that ArcDPS does not necessarily resend an already-active Primer when:

- Changing maps.
- Logging out and back in.
- Reloading the addon.

However, Food and Utility durations can still reflect the extended duration caused by an active Primer.

Because of this, Food Reminder added an inferred Primer display.

If Food or Utility has an unusually long duration consistent with Primer extension, the configuration interface can display:

`Metabolic Primer: ~HH:MM:SS (inferred)`

or:

`Utility Primer: ~HH:MM:SS (inferred)`

The `(inferred)` label is intentionally shown so the addon does not present the estimated state as a directly observed Primer event.

### Primer Expiration Reminders

Added:

- Metabolic Primer expiration warning.
- Utility Primer expiration warning.
- Combined Primer expiration warning.

Developer test buttons were added for:

- Metabolic Primer warning.
- Utility Primer warning.
- Both Primer warnings.

Initial testing revealed Primer popup countdowns were incorrectly borrowing Food/Utility remaining time.

The popup timer-selection logic was updated so Primer reminders use the Primer reminder duration instead.

Testing confirmed:

- Metabolic Primer test popup works.
- Utility Primer test popup works.
- Combined Primer popup works.
- All test Primer popups display the expected 30-minute test countdown.
- No crashes occurred during testing.

### Configurable Primer Warning Times

The previously hardcoded 30-minute Primer warning was replaced with configurable settings.

Separate warning values now exist for:

- Metabolic Primer
- Utility Primer

The actual reminder logic now uses the configured warning values.

Testing confirmed that changing the Metabolic Primer warning to 15 minutes persisted correctly.

### Missing Food and Utility Reminders

Added warnings when entering combat without consumable buffs.

Possible reminders are:

- `FOOD MISSING`
- `UTILITY MISSING`
- `FOOD + UTILITY MISSING`

These reminders display a message rather than a zero-duration countdown.

Examples:

`You entered combat without food.`

`You entered combat without utility.`

`You entered combat without food or utility.`

### ArcDPS Combat State Tracking

Combat-state tracking was added using ArcDPS state-change events.

Confirmed working values:

- State `1` = Enter Combat
- State `2` = Exit Combat

An earlier test incorrectly used state values 9 and 10, which prevented combat detection from working.

After correcting the state values, live testing confirmed:

- Entering combat changes the tracker to `IN COMBAT`.
- Leaving combat changes the tracker to `OUT OF COMBAT`.
- Missing-buff reminders fire once when entering combat.
- They do not repeat continuously during the same combat.
- They re-arm after leaving combat.
- Re-entering combat triggers a new warning when appropriate.

A Combat State readout was added to Developer Debug to verify this behavior.

### Missing-Buff Combination Testing

The following scenarios were tested:

#### Neither Food nor Utility active

Result:

`FOOD + UTILITY MISSING`

Passed.

#### Food active, Utility missing

Result:

`UTILITY MISSING`

Passed.

#### Utility active, Food missing

Initial testing revealed a character-switch tracking issue.

After the character tracking fix described below:

Result:

`FOOD MISSING`

Passed.

### Character-Switch Tracking Bug

Testing found that Food and Utility state could remain in memory after switching characters.

Example:

- Character A had Food active.
- Switched to Character B with no Food.
- Current Buffs temporarily continued showing Character A's Food timer.

This caused the addon to incorrectly believe Character B still had Food.

### Character Detection Fix

Food Reminder now tracks the local ArcDPS agent and character name.

Character-change detection uses:

- Self-agent ID.
- Self-character name.

When a different character is detected:

- Food state is cleared.
- Utility state is cleared.
- Food duration is cleared.
- Utility duration is cleared.
- Combat state is reset.

Primer persistence is intentionally not cleared by this logic.

Live testing confirmed:

- A new character is detected once ArcDPS provides a self-related event.
- Stale Food/Utility state is cleared.
- `FOOD MISSING` correctly triggers on the new character when Utility is active but Food is absent.

### Known Character-Switch Limitation

Immediately after switching characters, before ArcDPS has emitted a self-related event for the new character, the Current Buffs interface may temporarily display the previous character's Food or Utility state.

Once the new character generates a relevant ArcDPS event, the addon recognizes the character change and clears the stale state.

This remains a synchronization limitation to improve later.

### Jade Tech Protocol Investigation

Investigated whether Food Reminder could track Jade Tech Offensive and Defensive Protocol buffs.

Testing found that ArcDPS reports interaction with a Jade Tech station as:

- Skill name: `Generic gadget interact`
- Skill ID: `23302`
- State change: `67` / `68`
- Source: local player
- Destination: `Jade Tech Enhancement Console`

Source and destination agent-name capture was temporarily added to Developer Debug to inspect these events.

The destination does not identify whether the station is:

- Offensive Protocol
- Defensive Protocol

Additional targeted testing for the actual Jade Tech Overcharge effects did not produce reliable buff events through the current ArcDPS combat-event stream.

Therefore:

- Jade Tech Protocol tracking was not implemented.
- The addon will not guess which Protocol was activated.
- Jade Tech tracking remains experimental/future work if a reliable data source becomes available.

### Developer Debug Improvements

Developer Debug currently supports:

- Combat-state display.
- ArcDPS event count.
- Buff-like event count.
- Recent event inspection.
- Skill IDs.
- Event values.
- Buff state.
- Buff-removal state.
- ArcDPS state-change values.
- Source-is-self state.
- Destination-is-self state.
- Source agent name during investigation.
- Destination agent name during investigation.
- Primer warning test buttons.
- Debug event reset.

Temporary Jade-specific diagnostic filtering should be removed before treating the current source as the next clean release checkpoint.

### Current Working Checkpoint

At the end of this session, the following were confirmed working:

- Food detection.
- Utility detection.
- Food countdown.
- Utility countdown.
- Food expiration reminder.
- Utility expiration reminder.
- Combined Food + Utility expiration reminder.
- Persistent reminder settings.
- Metabolic Primer tracking.
- Utility Primer tracking.
- Primer persistence when directly detected.
- Inferred Primer display.
- Metabolic Primer expiration reminder.
- Utility Primer expiration reminder.
- Combined Primer expiration reminder.
- Configurable Primer warning times.
- Missing Food warning.
- Missing Utility warning.
- Combined Food + Utility Missing warning.
- Enter-combat detection.
- Exit-combat detection.
- One-warning-per-combat behavior.
- Reminder re-arming after combat ends.
- Character-change detection.
- Clearing stale Food/Utility state after the new character is detected.
- Developer combat-state readout.
- Successful builds and in-game testing without crashes during the tested scenarios.

### Git

Multiple stable checkpoints were committed and pushed throughout the session.

Important checkpoint areas included:

- Persistent settings.
- Primer tracking and inferred Primer display.
- Primer settings write optimization.
- Primer expiration reminders.
- Configurable Primer warning times.
- Reminder Timing UI cleanup.
- Missing-buff reminders on combat entry.
- Character-switch stale-state handling.

Before the final end-of-session commit, temporary Jade Tech diagnostic changes should be cleaned from the source and the addon should receive one final rebuild/test.

---

## Next Development Steps

1. Remove temporary Jade Tech diagnostic code.
2. Perform a final rebuild and smoke test.
3. Commit/push the cleaned August 20 development checkpoint.
4. Improve initial Food/Utility synchronization after login and map changes.
5. Improve character-switch synchronization before the first ArcDPS self event.
6. Continue expiration/removal edge-case testing.
7. Continue combat-entry reminder testing in longer encounters.
8. Refine the player-facing reminder presentation.
9. Decide whether the Developer Debug interface should remain available behind a development option.
10. Revisit Jade Tech Protocol tracking only if a reliable Offensive/Defensive data source becomes available.



## 2026-08-21 - Squad Consumable Tracking Milestone

### Squad Player Discovery

Implemented ArcDPS-based nearby player tracking using ArcDPS tracking-change events.

Tracked player information now includes:

- Character name
- Account name
- ArcDPS agent ID
- Instance ID
- Profession
- Elite specialization
- Subgroup
- Team
- Self identification

This replaces earlier unsuccessful squad identity experiments and follows the same general tracking approach used by the original ArcDPS Food Reminder.

### Squad Food and Utility Tracking

Implemented Food and Utility tracking for ArcDPS-tracked players.

The Squad tab now supports:

- Food detection
- Utility detection
- Remaining duration
- Live countdown timers
- Buff application handling
- Buff removal handling
- ArcDPS BuffInitial snapshot handling
- Proc-effect ignore filtering
- Known consumable labels
- Unknown consumable effect ID display

Live comparison against the existing ArcDPS Food Tracker showed the new timers consistently matching within approximately one second.

Examples observed during live meta testing:

- Power Food timers matched
- Precision Food timers matched
- Power Utility timers matched
- None states matched
- Unknown consumables were successfully detected and displayed with their ArcDPS effect IDs

### Consumable Database Expansion

Expanded the internal consumable database with additional Food and Utility definitions.

Recent additions include:

- Power foods
- Precision foods
- Concentration foods
- All-stat foods
- On-kill foods
- Slaying utilities
- Decade Enhancement

Unknown consumable IDs remain visible in the Squad tracker so additional database entries can be identified without losing tracking information.

### Proc Filtering

Added filtering for known food-related proc effects that should not replace the player's actual Food or Utility state.

Ignored effect IDs currently include:

- 10110
- 10104
- 64528
- 32289
- 32293
- 33046
- 65475

### Validation

Squad consumable tracking has now been successfully tested during live open-world meta events.

Core player tracking, consumable detection, and countdown behavior are considered functional.

### In Progress

Started improving Food/Utility state semantics.

Planned display behavior:

- `?` = state has not yet been established by ArcDPS
- `None` = ArcDPS has established that no consumable is active
- Known label = recognized active consumable
- `Unknown (ID)` = active consumable with an unmapped effect ID

The state-semantics implementation currently builds successfully but still requires live testing before being considered complete.