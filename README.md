# FoodReminder-Nexus

> ⚠️ **Early Development Build — v0.2.1**
>
> FoodReminder-Nexus is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food, utility, and primer buffs, with reminders, squad consumable tracking, session usage analysis, persistent per-character history, and Trading Post monitoring.

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
- Clears stale consumable state when switching characters
- Persists per-character Food and Utility state across addon/game reloads
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
- Provides tooltip explanations for Primer data limitations
- Session reporting separates confirmed, inferred, and unknown Primer time

> **Primer limitation:** ArcDPS does not reliably resend already-active Primer state after login or character switching, and current testing did not expose reliable Primer application events through the combat-event stream used by FoodReminder-Nexus. The addon therefore prefers an honest `Unknown` or inferred-presence state instead of displaying a fabricated countdown.

Inferred Primer presence remains conservative:

- No exact Primer countdown is fabricated
- Inference is character-specific
- Inference does not carry across character switches
- A game/addon restart begins fresh
- Reliable contradictory Primer information can clear the inferred state

### Compact Tracker

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

### Trading Post Integration

FoodReminder-Nexus can retrieve Guild Wars 2 Trading Post pricing for recognized consumables and independently watched Trading Post items.

This allows the Session Report to estimate the cost of Food and Utility consumed during a gameplay session.

The built-in Trading Post Watcher supports:

- Multiple watched items
- Name-first Trading Post item search with live autocomplete suggestions
- Local searchable Trading Post item index
- Current lowest sell price
- Current highest buy price
- Per-item Sell Targets
- Target status
- Fresh-API-only target alerts
- Anti-spam target alert latching
- Queued alerts when multiple watched items reach their Sell Targets together
- Sequential alert presentation
- Immediate promotion of the next queued alert when the current alert is dismissed
- Dismissible target-reached notifications
- Standalone gameplay-visible target-hit overlay
- Compact target-hit card inside the Trading Post tab
- Context-aware alert presentation
- Dragon Bash-style target celebration effects
- Manual per-item price refresh
- Refresh All control
- Automatic periodic price checks
- Persistent watch lists
- Persistent Sell Targets
- Persistent local Trading Post price history
- History persistence across addon/game restarts
- Tiered history retention and sampling
- Compact buy/sell history sparklines
- Historical minimum, average, and maximum prices
- Selectable trend windows:
  - 15 minutes
  - 30 minutes
  - 1 hour
  - 6 hours
  - 24 hours
  - 3 days
  - 7 days
  - 30 days
  - 90 days
- Time-based buy and sell trend analysis
- Actual coin and percentage movement over the selected trend window
- Trading Post buy/sell spread analysis
- Deal-quality analysis based on the item’s own recent sell-price history:
  - `FAVORABLE`
  - `TYPICAL`
  - `EXPENSIVE`
- Combined buying-opportunity signals:
  - `GOOD BUY`
  - `WATCH`
  - `OVERPRICED`

The Sell Target represents the **maximum price the user is willing to pay**.

A target is considered reached when the current lowest sell listing is equal to or below the configured Sell Target.

Target notifications are based on a fresh Trading Post API observation rather than immediately firing from stale cached pricing after a target edit.

When multiple watched items reach their targets during the same update:

1. The first target becomes the active alert
2. Additional target hits are queued
3. Only one alert is shown at a time
4. Dismissing the current alert immediately advances to the next queued alert
5. No additional API refresh is required to advance the queue

The existing target latch behavior remains intact. A target only re-arms after a later observed sell price rises above its configured target.

Alert presentation also depends on which FoodReminder interface is open:

- During normal gameplay, the large `TARGET REACHED!` celebration overlay is shown
- While the FoodReminder Trading Post tab is open, the large overlay is suppressed
- The compact `TARGET PRICE HIT!` card remains visible inside the Trading Post tab

This prevents the same target from appearing simultaneously in both alert formats.

Deal-quality and opportunity signals are based on locally collected Trading Post observations and are intended as informational market context rather than guaranteed buying advice.

Items without available Trading Post pricing remain tracked normally and are reported without a cost rather than using an estimated or guessed value.

Trading Post item search is name-first. Users can begin typing an item name and select from matching Guild Wars 2 Trading Post items without needing to know an item ID beforehand.

Item IDs remain available for reference and internal validation but are not required user input.

### Unknown Consumable Collector

Unrecognized Food and Utility effects can be collected automatically during normal gameplay.

The collector records:

- Effect ID
- Food or Utility classification
- Number of observations

Collected effects persist between sessions and can be exported for later identification and database expansion.

The collector automatically removes stored entries once their effect IDs become recognized by the current consumable database, keeping the list focused on genuinely unresolved effects.

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

ArcDPS does not always resend every already-active buff immediately after login, map changes, character changes, or addon reloads.

Because of this, some consumable state may remain unknown until ArcDPS reports a relevant event.

Food and Utility state can often resynchronize when fresh combat events arrive.

Primer state is more limited. Current testing showed that already-active Primer state is not reliably resent after login or character switching, and Primer application did not appear reliably in the Recent Buff Events stream used during testing.

FoodReminder-Nexus therefore does not fabricate Primer timers.

The addon intentionally prefers displaying an effect as **unknown** rather than guessing its identity or remaining duration.

Squad consumable states may appear as:

- `?` — state has not yet been established
- `None` — no consumable is active
- Recognized label — known active consumable
- `Unknown (ID)` — active effect not yet mapped

Trading Post history is collected locally while FoodReminder-Nexus observes watched items.

The ArenaNet Trading Post API provides current market information but does not provide historical price data.

Historical charts, trends, averages, deal ratings, and opportunity signals are therefore generated from the addon’s own locally collected observations.

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

FoodReminder-Nexus is currently an experimental v0.2.1 development build.

Current version:

**v0.2.1**

Core consumable tracking, customizable reminders, RTAPI/ArcDPS hybrid squad tracking, session reporting, persistent per-character personal history, unknown-consumable collection, Trading Post cost integration, Trading Post Watcher, queued target alerts, and conservative Primer-state handling are operational and undergoing continued live in-game testing.

### Trading Post Watcher Validation

The Trading Post Watcher has successfully passed:

- Live buy/sell price retrieval
- Multi-item watch-list testing
- Watch-list persistence
- Sell Target persistence
- Name-first item search and autocomplete
- Local Trading Post index construction
- Persistent price-history collection
- History persistence across restarts
- Time-based trend analysis
- Tiered history retention
- Target alert testing
- Anti-spam target alert testing
- Multi-target alert queue testing
- Sequential dismiss/advance testing
- Gameplay-overlay vs in-tab alert presentation testing
- Deal-quality analysis
- Opportunity-signal testing
- Standalone gameplay-visible target-hit overlay testing

A live multi-target test successfully queued and presented three simultaneous target hits one at a time.

### Consumable & History Validation

Recent consumable-state and History work has passed:

- Per-character Food/Utility state restoration
- `Unknown` vs confirmed-missing state handling
- False missing-warning suppression while state is unknown
- Character-specific Primer state isolation
- Removal of inaccurate Food/Utility-to-Primer countdown inference
- Primer presence-only inference
- Inferred Primer-presence session latch
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
- HistoryUI, TradingPostUI, and SquadUI modular extraction build/in-game verification
- Superior Sharpening Stone Trading Post cost lookup corrected to use its tradable representative item ID

### v0.2.1 Focus

The v0.2.1 update primarily adds:

- Per-character Personal History
- Searchable character History filtering
- Automatic Current Character History default
- Preservation of older untagged History under `Legacy / Unknown`
- Multi-target Trading Post alert queue
- Immediate next-alert presentation after dismissal
- Context-aware gameplay vs Trading Post-tab target notifications
- Improved inferred Primer-presence persistence
- Additional Squad-filter and live-roster validation

The v0.2.1 Release build has completed an in-game smoke test covering:

- Addon startup
- Compact tracker
- Food and Utility tracking
- Current-character History selection
- Per-character History filtering
- Squad tracking
- Squad filters
- Trading Post Watcher
- Multi-target alert queue
- Gameplay target celebration
- Compact Trading Post-tab target card
- Alert dismissal and queue advancement

Current development remains focused on:

- Expanding the external consumable database through normal gameplay and reliable public-data reconciliation
- Resolving genuinely unknown effect IDs
- Refining consumable cost and savings analysis
- Continued RTAPI/ArcDPS hybrid roster testing
- Continued Trading Post history accumulation
- Continued gameplay and stability testing
- Incremental zero-behavior-change UI modularization, with History, Trading Post, and Squad rendering already extracted from `entry.cpp`
- Improving initial-state synchronization only where reliable data is available

---

## Planned Features

Future development may include:

- Additional Trading Post market-analysis tools
- Additional Trading Post notification customization
- Longer-term Trading Post history analysis
- Additional reminder customization where useful
- Per-character settings
- Further Squad tracker filtering/presentation improvements
- Improved initial-state synchronization where reliable data is available
- Additional consumable database coverage
- Jade Tech tracking if a reliable data source becomes available

---

## Known Limitations

- Food and Utility state may briefly show an older saved value after login or character switching until fresh ArcDPS events resynchronize the state
- ArcDPS does not reliably resend already-active Primer state after login or character switching
- Exact Primer countdowns therefore cannot always be reconstructed
- Direct Primer application is not reliably exposed through the ArcDPS combat-event stream currently used by FoodReminder-Nexus
- Inferred Primer presence does not provide an exact remaining Primer timer
- Rare live ArcDPS state delivery can sometimes provide a real Primer countdown after startup, but the exact trigger is not yet understood and the countdown may be lost again after addon reload
- Some consumable effects share the same effect ID and cannot always be uniquely identified
- Unknown consumable effects may still appear until they are added to the database
- Historical consumable cost uses current Trading Post sell prices rather than the original purchase price
- Historical sessions recorded before the Session usage-count fix may retain inflated legacy use totals
- Historical sessions recorded before per-character History tagging remain under `Legacy / Unknown`
- Trading Post history begins only after FoodReminder-Nexus observes an item
- ArenaNet's current Trading Post API does not provide historical price data
- Trading Post trends, averages, deal ratings, and opportunity signals depend on locally collected observations
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
