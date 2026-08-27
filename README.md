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

### Primer Support

- Tracks Metabolic Primer and Utility Primer
- Displays remaining Primer duration
- Warns before Primers expire
- Restores previously detected Primer state after addon reload
- Can infer Primer protection from extended consumable duration when direct state is unavailable

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

Primer-protected time is tracked separately so extended consumable durations are not mistaken for ordinary consumable waste.

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

---

## How Tracking Works

FoodReminder-Nexus receives combat and buff information through **ArcDPS combat events**.

ArcDPS does not always resend every already-active buff immediately after login, map changes, character changes, or addon reloads. Because of this, some consumable state may remain unknown until ArcDPS reports a relevant event.

The addon intentionally prefers displaying an effect as **unknown** rather than guessing its identity.

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

Core consumable tracking, reminders, Primer support, squad tracking, session reporting, unknown-consumable collection, Trading Post cost integration, and the Trading Post Watcher are operational and undergoing live in-game testing.

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

Current development is focused on:

- Adding a standalone Trading Post target-hit notification that can appear outside the Trading Post tab
- Improving already-active buff synchronization
- Expanding the consumable database
- Refining consumable cost analysis
- Improving the Squad and Session interfaces
- Improving reminder presentation
- Continued gameplay and stability testing

---

## Planned Features

Future development may include:

- Standalone Trading Post target-hit overlay
- Additional Trading Post market-analysis tools
- Additional Trading Post notification customization
- Longer-term Trading Post history analysis
- Session cost-per-hour analysis
- Historical consumable usage statistics
- Compact optional Food/Utility timer HUD
- Additional reminder customization
- Combat-aware reminder behavior
- Per-character settings and preferred consumables
- Squad tracker customization
- Improved initial-state synchronization
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