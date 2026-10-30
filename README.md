# FoodReminder-Nexus

> ⚠️ **Early Development Build — v0.3.0**
>
> FoodReminder-Nexus is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food, utility, and primer buffs, with reminders, squad consumable tracking, session usage analysis, persistent per-character history, and current Trading Post pricing for consumable cost estimates.

> **Know when your important consumables are missing or about to expire without constantly watching the buff bar.**

---

## Features

### Food & Utility Tracking

- Tracks active Food and Utility buffs
- Displays remaining duration
- Warns before Food or Utility expires
- Warns when entering combat without Food and/or Utility
- Independent enable/disable controls for Food expiration, Utility expiration, missing-consumable, and Primer expiration reminders
- Configurable reminder popup display duration from 3 to 10 seconds
- Detects consumable applications, refreshes, replacements, and expirations
- Handles modern ArcDPS typed buff state changes for initial state, applications, duration changes, and removals
- Clears stale consumable state when switching characters
- Preserves active consumable timers across same-character ArcDPS agent/map transitions
- Persists per-character Food and Utility state across addon/game reloads
- Rejects implausible multi-day consumable durations instead of displaying corrupted timer values
- Distinguishes between `Unknown`, active, and confirmed-missing state
- Avoids treating unknown state as confirmed missing when ArcDPS has not yet supplied enough information

### Primer Support

- Tracks Metabolic Primer and Utility Primer when reliable ArcDPS state is available
- Keeps Primer state character-specific
- Prevents Primer state from leaking between characters
- Can infer Primer **presence only** from clearly Primer-extended Food or Utility duration
- Does not infer a false Primer countdown from extended consumable duration
- Displays `Active*` when Primer presence is inferred rather than directly confirmed
- Keeps inferred Primer presence latched for the current character session after it has been established
- Prevents inferred `Active*` state from reverting to `Unknown` simply because Food or Utility duration later falls below the original inference threshold
- Displays `Unknown` when ArcDPS does not provide enough information to determine Primer state
- Persists a confirmed direct Primer countdown per character once trustworthy state has been captured
- Confirmed Primer timers pause while the game is closed, matching observed Food/Utility offline behavior
- Provides tooltip explanations for Primer data limitations
- Session reporting separates confirmed, inferred, and unknown Primer time

> **Primer limitation:** ArcDPS does not consistently provide enough already-active Primer state after login or character switching to establish a trustworthy countdown. Once a confirmed direct Primer countdown has been captured, FoodReminder-Nexus can preserve that per-character remaining time across a full restart. When direct state is unavailable, the addon prefers an honest `Unknown` or inferred `Active*` state instead of displaying a fabricated countdown.

Inferred Primer presence remains conservative:

- No exact Primer countdown is fabricated
- Inference is character-specific
- Inference does not carry across character switches
- Inferred-only presence does not become a fabricated persisted countdown after restart
- A confirmed direct countdown may be restored when trustworthy Primer state was previously captured
- Reliable contradictory Primer information can clear the inferred state

### Compact Tracker


<img width="291" height="103" alt="tracker1" src="https://github.com/user-attachments/assets/2233a5d7-c23a-4fc3-923c-74a0d758950a" />


- Compact always-available Food/Utility tracker
- Remaining Food and Utility duration
- Recognized consumable names, with long names shortened in the compact view
- Primer state display
- Warning and critical timer colors
- `Unknown` and `Not detected` states
- Hover tooltips for consumable and Primer information
- Right-click consumable actions
- Compact width for reduced screen space
- Distinctive gold tracker border for quick visual recognition
- Draggable position
- Optional position locking

Generic database labels such as `Food`, `Power`, or other short categories are no longer displayed in place of the actual recognized consumable name.

### Squad Consumable Tracker

- Uses RTAPI as the current squad roster/subgroup source when RTAPI is installed and synchronized
- Falls back automatically to the existing ArcDPS-only roster when RTAPI is unavailable or still syncing
- Continues using ArcDPS for Food/Utility effect IDs, active state, and remaining duration
- Displays Food and Utility status and remaining duration
- Identifies recognized consumables
- Distinguishes unknown, missing, and not-yet-known consumable states
- Shows the active roster source directly in the Squad tab
- Provides three player-filter modes:
  - All players
  - Missing / Unknown / Unmapped
  - Unknown / Unmapped only
- Automatically collects unidentified consumable effect IDs for future database expansion

The Squad filters have been validated during live RTAPI-backed squad gameplay. Live Food/Utility timers continue updating while filters change.

### Session Report

Tracks consumable usage during the current gameplay session, including:

- Session and combat time
- Food and Utility coverage
- In-combat consumable coverage
- Applications
- Refreshes
- Replacements
- Consumables expiring during combat
- Per-consumable usage history
- Consumable cost tracking when Trading Post pricing is available
- Duplicate/resync buff events are filtered so observed uses are not inflated by simple state synchronization
- Total consumable cost
- Uses tradable representative item IDs for cost estimates when multiple item variants share the same detected effect
- Confirmed Primer-active time
- Inferred Primer-active time
- Unknown Primer-state time
- Estimated Primer uses saved
- Estimated Primer gold saved

Primer protection is handled conservatively:

- **Confirmed** — direct trustworthy Primer state
- **Inferred*** — Primer presence inferred from clearly extended Food/Utility duration
- **Unknown** — ArcDPS has not supplied enough information
- **Inactive** — reliable state indicates no active Primer
- Unknown Primer time is excluded from Primer savings estimates
- Extended Food/Utility duration is never copied into the Primer countdown

Session usage tracking also filters ArcDPS buff resynchronization so an already-active consumable being resent does not count as another real use.

A new use is recorded when:

- No matching consumable was already active
- A different consumable replaces the current one
- The same consumable's remaining duration increases enough to indicate a real reapplication

### Personal History


FoodReminder-Nexus keeps a local history of completed consumable-tracking sessions.

The History tab includes:

- Selectable 1 Day / 7 Days / 30 Days / All Time ranges
- Per-character History filtering
- `Current Character`
- `All Characters`
- Searchable saved-character list
- `Legacy / Unknown` handling for older sessions recorded before character tagging was added
- Automatic Current Character default when switching characters
- Completed-session count
- Tracked time and combat time
- Food and Utility use counts
- Food and Utility coverage
- Estimated current-price consumable cost
- Separate reporting for unpriced consumable uses
- Per-session Food coverage markers
- Per-session Utility coverage markers
- Estimated Spend sparkline
- Per-item Food and Utility usage history
- Primer detail history
- Coverage and waste details

History is stored locally in:

`FoodReminder_SessionHistory.tsv`

and survives game/addon restarts.

Newly archived sessions retain the character name associated with that session.

Switching characters archives the previous character's current session and starts a new character-specific session so a single History record does not mix multiple characters.

The selected character filter applies to:

- Session counts
- Tracked time
- Combat time
- Food and Utility uses
- Coverage
- Coverage markers
- Estimated Spend
- Per-item usage
- Primer details

Sessions recorded before per-character History tagging was added are preserved under `Legacy / Unknown`.

> **History cost limitation:** Historical cost uses current Trading Post sell prices when available. It does not reconstruct the price actually paid when a consumable was used.


### External Consumable Database

FoodReminder-Nexus supports an external:

`FoodReminder_Consumables.tsv`

database stored beside the addon DLL.

- Consumable mappings can be added or corrected without rebuilding the DLL
- The database automatically reloads after the TSV is saved
- Verified built-in mappings remain authoritative
- The compiled database remains available as a fallback
- Shared effect IDs can use combined/shared descriptions when ArcDPS cannot uniquely identify the consumed item

This allows normal gameplay and the Unknown Consumable Collector to drive database expansion without requiring repeated Visual Studio rebuilds for data-only changes.

---

## How Tracking Works

FoodReminder-Nexus receives Food/Utility combat and buff information through **ArcDPS combat events**.

When **RTAPI** is installed and synchronized, FoodReminder-Nexus uses RTAPI for the live squad roster, subgroup membership, profession/specialization, self identity, and same-instance status.

ArcDPS remains the source for actual Food/Utility effect IDs and remaining duration.

If RTAPI is unavailable, unloaded, or still synchronizing, the Squad tracker automatically falls back to the existing ArcDPS-only roster behavior.

FoodReminder-Nexus handles the modern ArcDPS typed buff state-change model for initial-state snapshots, applications, duration changes, and removals.

ArcDPS can assign a new self-agent context during a same-character map transition. FoodReminder-Nexus preserves live Food, Utility, and Primer timers across that transition instead of treating the new agent ID as a character switch.

ArcDPS does not always resend every already-active buff immediately after login, map changes, character changes, or addon reloads.

Because of this, some consumable state may remain unknown until ArcDPS reports a relevant event.

Food and Utility state can resynchronize when fresh buff events arrive.

Primer state remains more limited. Initial direct Primer countdown capture is intermittent, but testing confirmed that once a trustworthy direct Primer timer is captured, its remaining per-character time can persist across a full restart.

FoodReminder-Nexus therefore does not fabricate Primer timers when direct state is unavailable.

The addon intentionally prefers displaying an effect as **unknown** rather than guessing its identity or remaining duration.

Squad consumable states may appear as:

- `?` — state has not yet been established
- `None` — no consumable is active
- Recognized label — known active consumable
- `Unknown (ID)` — active effect not yet mapped

FoodReminder-Nexus uses current Trading Post sell prices only for consumable cost estimates in Session and Personal History reporting.

The standalone Trading Post watcher, watch lists, item search/index, sell targets, target alerts, local price-history collection, trend windows, and market-analysis UI have been removed from FoodReminder-Nexus. Those features are no longer part of the addon's current interface or runtime.

---

## Requirements

FoodReminder-Nexus currently requires:

- **Guild Wars 2**
- **Nexus**
- **ArcDPS**

> [!IMPORTANT]
> **RTAPI IS OPTIONAL.**
>
> FoodReminder-Nexus does not require RTAPI to function.
>
> **RTAPI itself is not bundled with FoodReminder-Nexus.**
>
> RTAPI integration/support code is already compiled into `FoodReminder.dll`.
>
> If RTAPI is separately installed, FoodReminder-Nexus can automatically use its read-only squad roster information.
>
> If RTAPI is not installed, unavailable, or still synchronizing, FoodReminder-Nexus automatically falls back to ArcDPS-only Squad tracking.
>
> RTAPI project/source:
> https://github.com/gwdevcommunity/GW2-RealTime-API-Releases
>
> FoodReminder-Nexus does not use RTAPI to automate gameplay, control your character, perform combat actions, send inputs, manipulate game memory, or provide an unfair gameplay advantage.

---

## Installation

The release contains:

- `FoodReminder.dll`
- `FoodReminder_Consumables.tsv`

Place both files in the same Nexus addon folder.

Do not separate the TSV from the DLL if you want the external consumable database to load automatically.

RTAPI is **not included** with FoodReminder-Nexus.

---

## Current Development Status

FoodReminder-Nexus is currently an experimental v0.3.0 build.

Current version:

**v0.3.0**

Core consumable tracking, customizable reminders, RTAPI/ArcDPS hybrid squad tracking, session reporting, persistent per-character personal history, unknown-consumable collection, conservative Primer-state handling, and current-price consumable cost integration are operational and undergoing continued live in-game testing.

### Standalone Trading Post Removal

The standalone Trading Post feature has been removed from FoodReminder-Nexus.

Removed functionality includes:

- Trading Post tab/UI
- Watch lists
- Item search and local item index
- Sell Targets
- Target alerts and celebration overlays
- Local Trading Post price-history collection
- Trend windows
- Market-analysis signals and charts
- Trading Post-specific settings and startup/shutdown plumbing

`TradingPostPriceManager` remains intentionally in the project in a reduced role so Session and Personal History can continue retrieving current sell prices for consumable cost estimates.

The cleanup checkpoint has passed:

- Release x64 build
- Addon startup
- Compact Food/Utility tracker
- Food and Utility countdowns
- Metabolic Primer countdown
- Utility Primer countdown
- Session tab
- Personal History tab
- Existing History summaries and estimated spend display
- Squad tab
- Current-price consumable cost integration

### Consumable & History Validation

Recent consumable-state and History work has passed:

- Per-character Food/Utility state restoration
- Modern ArcDPS typed buff-state handling for states 18 / 69 / 70 / 71 / 72
- Correct state-70 duration-change handling using the new stack duration
- Per-stack trackable-ID handling for typed buff removals
- Same-character map/agent transitions preserving live consumable timers
- Rejection of implausible 13+ hour corrupted consumable durations
- `Unknown` vs confirmed-missing state handling
- False missing-warning suppression while state is unknown
- False Food/Utility combat-warning regression eliminated in live testing with active consumables
- Character-specific Primer state isolation
- Removal of inaccurate Food/Utility-to-Primer countdown inference
- Primer presence-only inference
- Inferred Primer-presence session latch
- Confirmed direct Primer countdown persistence across full restart once captured
- Confirmed Primer timers observed to pause while the game is closed
- Primer limitation tooltips
- Primer-safe Session Report tracking
- Compact tracker width cleanup
- Gold tracker border styling
- External consumable TSV loading and automatic reload
- Resolved unknown effects automatically disappearing from the Unknown Consumable Collector
- Persistent session-history save/load/append behavior
- Per-character History tagging and filtering
- Automatic Current Character History selection
- Legacy / Unknown preservation for older History
- Personal History coverage markers
- Estimated Spend sparkline
- Per-item History usage
- Session usage-count filtering so ArcDPS resync events do not count as extra consumable uses
- Compact tracker actual-name display with long-name truncation
- Independent reminder-type enable/disable controls
- Configurable 3–10 second reminder display duration
- Three-mode Squad player filtering
- RTAPI roster integration with ArcDPS consumable-state pairing
- RTAPI live-roster status display and automatic ArcDPS fallback
- HistoryUI, SquadUI, SessionUI, TrackerUI, and ReminderUI modular extraction build/in-game verification
- Compact Tracker inferred-Primer hover tooltip regression fixed and verified in-game
- Temporary Primer investigation tracing removed after non-reproducible testing; conservative Primer behavior retained
- Superior Sharpening Stone Trading Post cost lookup corrected to use its tradable representative item ID
- Standalone Trading Post components removed while current-price consumable cost lookup remained functional

Current development remains focused on:

- Expanding the external consumable database through normal gameplay and reliable public-data reconciliation
- Resolving genuinely unknown effect IDs
- Refining consumable cost and savings analysis
- Continued RTAPI/ArcDPS hybrid roster testing
- Continued gameplay and stability testing
- Continued validation of modern ArcDPS typed buff events across map changes, relogs, combat entry/exit, and booster-heavy gameplay
- Improving initial-state synchronization only where reliable data is available

---

## Planned Features

Future development may include:

- Additional reminder customization where useful
- Per-character settings
- Further Squad tracker filtering/presentation improvements
- Improved initial-state synchronization where reliable data is available
- Additional consumable database coverage
- Further consumable cost and savings analysis
- Jade Tech tracking if a reliable data source becomes available

---

## Known Limitations

- Food and Utility state may briefly show an older saved value after login or character switching until fresh ArcDPS events resynchronize the state
- Same-character map/agent transitions now preserve live timers, but continued live validation is still in progress
- ArcDPS does not reliably resend enough already-active Primer state after login or character switching to establish a direct countdown every time
- Exact Primer countdowns therefore cannot always be established from live ArcDPS state
- Inferred Primer presence does not provide an exact remaining Primer timer
- Once a confirmed direct Primer countdown has been captured, current testing shows its per-character remaining time can persist across a full restart; the unresolved limitation is reliable initial capture
- Some consumable effects share the same effect ID and cannot always be uniquely identified
- Unknown consumable effects may still appear until they are added to the database
- Historical consumable cost uses current Trading Post sell prices rather than the original purchase price
- Historical sessions recorded before the Session usage-count fix may retain inflated legacy use totals
- Historical sessions recorded before per-character History tagging remain under `Legacy / Unknown`
- RTAPI does not provide Food/Utility buff IDs or remaining duration and does not replace ArcDPS consumable detection
- Squad members outside the player's current map instance may not have usable ArcDPS consumable state available for RTAPI roster pairing

---

## Development Log

Detailed implementation history, experiments, bug fixes, and in-game testing checkpoints are maintained separately in:

`DEVELOPMENT_LOG.md`

The README is intended to describe **what FoodReminder-Nexus is and what it currently does**, while the development log records **how it got there**.

---

## Disclaimer

Addon author: **spectre9510**

FoodReminder-Nexus is an unofficial **Guild Wars 2** addon.

It is not affiliated with or endorsed by ArenaNet.

Guild Wars 2 and all associated trademarks are property of their respective owners.
