# FoodReminder-Nexus

> ⚠️ **Early Development Build — v0.1.0**
>
> FoodReminder-Nexus is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food, utility, and primer buffs, with reminders, squad consumable tracking, session usage analysis, and Trading Post monitoring.

> **Know when your important consumables are missing or about to expire without constantly watching the buff bar.**

---

## Features

### Food & Utility Tracking

- Tracks active Food and Utility buffs
- Displays remaining duration
- Warns before Food or Utility expires
- Warns when entering combat without Food and/or Utility
- Detects consumable applications, refreshes, replacements, and expirations
- Clears stale consumable state when switching characters
- Persists per-character Food and Utility state across addon/game reloads
- Distinguishes between `Unknown`, active, and confirmed-missing state

### Primer Support

- Tracks Metabolic Primer and Utility Primer when reliable ArcDPS state is available
- Keeps Primer state character-specific
- Prevents Primer state from leaking between characters
- Can infer Primer **presence only** from clearly Primer-extended Food or Utility duration
- Does not infer a false Primer countdown from extended consumable duration
- Displays `Active*` when Primer presence is inferred rather than directly confirmed
- Displays `Unknown` when ArcDPS does not provide enough information to determine Primer state
- Provides tooltip explanations for Primer data limitations
- Session reporting separates confirmed, inferred, and unknown Primer time

> **Primer limitation:** ArcDPS does not reliably resend already-active Primer state after login or character switching, and current testing did not expose reliable Primer application events through the combat-event stream used by FoodReminder-Nexus. The addon therefore prefers an honest `Unknown` or inferred-presence state instead of displaying a fabricated countdown.

### Compact Tracker

- Compact always-available Food/Utility tracker
- Remaining Food and Utility duration
- Recognized consumable labels
- Primer state display
- Warning and critical timer colors
- `Unknown` and `Not detected` states
- Hover tooltips for consumable and Primer information
- Right-click consumable actions
- Compact width for reduced screen space
- Distinctive gold tracker border for quick visual recognition
- Draggable position
- Optional position locking

### Squad Consumable Tracker

- Tracks nearby players reported by ArcDPS
- Displays Food and Utility status and remaining duration
- Identifies recognized consumables
- Distinguishes unknown, missing, and not-yet-known consumable states
- Automatically collects unidentified consumable effect IDs for future database expansion

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
- Total consumable cost
- Confirmed Primer-active time
- Inferred Primer-active time
- Unknown Primer-state time
- Estimated Primer uses saved
- Estimated Primer gold saved

Primer protection is handled conservatively:

- **Confirmed** — direct trustworthy Primer state
- **Inferred*** — Primer presence inferred from clearly extended Food/Utility duration
- **Unknown** — ArcDPS has not supplied enough information
- Unknown Primer time is excluded from Primer savings estimates
- Extended Food/Utility duration is never copied into the Primer countdown

### Trading Post Integration

FoodReminder-Nexus can retrieve Guild Wars 2 Trading Post pricing for recognized consumables and independently watched Trading Post items.

This allows the Session Report to estimate the cost of Food and Utility consumed during a gameplay session.

The built-in Trading Post Watcher currently supports:

- Multiple watched items
- Name-first Trading Post item search with live autocomplete suggestions
- Local searchable Trading Post item index
- Current lowest sell price
- Current highest buy price
- Per-item Sell Targets
- Target status
- Fresh-API-only target alerts
- Anti-spam target alert latching
- Dismissible target-reached notifications
- Standalone gameplay-visible target-hit overlay
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

FoodReminder-Nexus supports an external `FoodReminder_Consumables.tsv` database stored beside the addon DLL.

- Consumable mappings can be added or corrected without rebuilding the DLL
- The database automatically reloads after the TSV is saved
- Verified built-in mappings remain authoritative
- The compiled database remains available as a fallback
- Shared effect IDs can use combined/shared descriptions when ArcDPS cannot uniquely identify the consumed item

This allows normal gameplay and the Unknown Consumable Collector to drive database expansion without requiring repeated Visual Studio rebuilds for data-only changes.

---

## How Tracking Works

FoodReminder-Nexus receives combat and buff information through **ArcDPS combat events**.

ArcDPS does not always resend every already-active buff immediately after login, map changes, character changes, or addon reloads. Because of this, some consumable state may remain unknown until ArcDPS reports a relevant event.

Food and Utility state can often resynchronize when fresh combat events arrive.

Primer state is more limited. Current testing showed that already-active Primer state is not reliably resent after login or character switching, and Primer application did not appear in the Recent Buff Events stream used during testing. FoodReminder-Nexus therefore does not fabricate Primer timers.

The addon intentionally prefers displaying an effect as **unknown** rather than guessing its identity or remaining duration.

Squad consumable states may appear as:

- `?` — state has not yet been established
- `None` — no consumable is active
- Recognized label — known active consumable
- `Unknown (ID)` — active effect not yet mapped

Trading Post history is collected locally while FoodReminder-Nexus observes watched items.

The ArenaNet Trading Post API provides current market information but does not provide historical price data. Historical charts, trends, averages, deal ratings, and opportunity signals are therefore generated from the addon’s own locally collected observations.

---

## Requirements

FoodReminder-Nexus currently requires:

- **Guild Wars 2**
- **Nexus**
- **ArcDPS**

The addon is written in **C++** and developed using **Visual Studio**.

---

## Current Development Status

FoodReminder-Nexus is currently an experimental development build.

Core consumable tracking, reminders, squad tracking, session reporting, unknown-consumable collection, Trading Post cost integration, Trading Post Watcher, standalone Trading Post target alerts, and conservative Primer-state handling are operational and undergoing live in-game testing.

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
- Deal-quality analysis
- Opportunity-signal testing
- Standalone gameplay-visible target-hit overlay testing

Recent consumable-state work has also passed:

- Per-character Food/Utility state restoration
- `Unknown` vs confirmed-missing state handling
- False missing-warning suppression while state is unknown
- Character-specific Primer state isolation
- Removal of inaccurate Food/Utility-to-Primer countdown inference
- Primer presence-only inference
- Primer limitation tooltips
- Primer-safe Session Report tracking
- Compact tracker width cleanup
- Gold tracker border styling
- External consumable TSV loading and automatic reload
- Resolved unknown effects automatically disappearing from the Unknown Consumable Collector

Current development is focused on:

- Expanding the external consumable database through normal gameplay and reliable public-data reconciliation
- Resolving only genuinely unknown effect IDs that remain after database lookup
- Refining consumable cost and savings analysis
- Improving Squad and Session interfaces
- Improving reminder presentation
- Continued gameplay and stability testing
- Evaluating additional reliable data sources for initial-state synchronization

---

## Planned Features

Future development may include:

- Additional Trading Post market-analysis tools
- Additional Trading Post notification customization
- Longer-term Trading Post history analysis
- Session cost-per-hour analysis
- Historical consumable usage statistics
- Additional reminder customization
- Combat-aware reminder behavior
- Per-character settings and preferred consumables
- Squad tracker customization
- Improved initial-state synchronization where reliable data is available
- Additional consumable database coverage
- Jade Tech tracking if a reliable data source becomes available

---

## Development Log

Detailed implementation history, experiments, bug fixes, and in-game testing checkpoints are maintained separately in:

`DEVELOPMENT_LOG.md`

The README is intended to describe **what FoodReminder-Nexus is and what it currently does**, while the development log records **how it got there**.

---

## Disclaimer

FoodReminder-Nexus is an unofficial **Guild Wars 2** addon.

It is not affiliated with or endorsed by ArenaNet.

Guild Wars 2 and all associated trademarks are property of their respective owners.
