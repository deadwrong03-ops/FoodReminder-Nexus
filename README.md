# FoodReminder-Nexus

> ⚠️ **Early Development Build — v0.1.0**
>
> FoodReminder-Nexus is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food, utility, and primer buffs, with reminders, squad consumable tracking, and session usage analysis.

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

FoodReminder-Nexus can retrieve Guild Wars 2 Trading Post pricing for recognized consumables.

This allows the Session Report to estimate the cost of Food and Utility consumed during a gameplay session.

Items without available Trading Post pricing remain tracked normally and are reported without a cost rather than using an estimated or guessed value.

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

Core consumable tracking, reminders, Primer support, squad tracking, session reporting, unknown-consumable collection, and initial Trading Post cost integration are operational and undergoing live in-game testing.

Current development is focused on:

- Improving already-active buff synchronization
- Expanding the consumable database
- Refining consumable cost analysis
- Improving the Squad and Session interfaces
- Improving reminder presentation
- Continued gameplay and stability testing

---

## Planned Features

Future development may include:

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