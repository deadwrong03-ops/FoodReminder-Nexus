# FoodReminder-Nexus

> ⚠️ **Early Development Build**
>
> FoodReminder-Nexus is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food, utility, and primer durations and providing configurable reminders.

The goal is simple:

> **Know when your important consumable buffs are about to expire without constantly watching the buff bar.**

---

## Current Version

**0.1.0 Development Build**

FoodReminder-Nexus is currently an experimental development project.

Features, interfaces, configuration formats, and internal systems may change as development continues.

---

## Current Status

FoodReminder-Nexus has a working Nexus addon framework with ArcDPS-powered tracking for food, utility, combat state, primer-related reminders, nearby player consumables, and per-session consumable statistics.

The addon can currently:

- Receive ArcDPS combat events
- Identify Food and Utility buffs belonging to the local player
- Calculate remaining Food and Utility duration
- Track Metabolic Primer and Utility Primer state when directly detected
- Infer active Primer state from extended Food/Utility durations when necessary
- Warn before Food and Utility expire
- Warn before Metabolic and Utility Primers expire
- Warn when entering combat without Food and/or Utility
- Detect entering and leaving combat
- Detect character changes and clear stale Food/Utility state
- Persist reminder settings between sessions
- Track nearby players reported by ArcDPS
- Track Food and Utility buffs for ArcDPS-tracked players
- Display live squad Food and Utility countdown timers
- Distinguish unknown, missing, known, and unmapped consumable states
- Classify recognized consumable effects
- Display unknown consumable effect IDs for future database expansion
- Capture unknown consumable IDs automatically in the background
- Persist collected unknown consumables between game sessions
- Export collected unknown IDs for database expansion
- Track session and combat time
- Measure Food and Utility coverage for the full session
- Measure Food and Utility coverage specifically while in combat
- Track Food and Utility applications
- Distinguish consumable refreshes from replacements
- Track Food and Utility expirations that occur during combat

Live open-world testing has shown the squad consumable timers matching the existing ArcDPS Food Tracker to approximately one second.

The current development focus is expanding session reporting, continuing consumable identification, refining the Squad interface, improving synchronization, and continuing stability testing during normal gameplay.

---

## Completed

### Core Addon Framework

- Nexus addon loading and unloading
- Nexus configuration panel
- ArcDPS combat event integration
- Persistent settings through `FoodReminder.ini`
- Enable/disable reminders option
- Development/debug interface
- Test Reminder button
- Reminder popup system
- Live reminder countdown display

### Reminder Timing

- Configurable Food early-warning time
- Configurable Utility early-warning time
- Configurable Metabolic Primer early-warning time
- Configurable Utility Primer early-warning time
- Food and Utility warning range: 1–60 minutes
- Primer warning range: 5–60 minutes
- Reminder timing settings persist between sessions
- Reminder Timing UI grouped into:
  - Food & Utility
  - Primers

### Food and Utility Tracking

- Local-player buff filtering
- ArcDPS buff-event processing
- Food buff detection
- Utility buff detection
- Food remaining-time calculation
- Utility remaining-time calculation
- Buff expiration tracking
- Buff removal handling
- Detection-state reset when buffs disappear
- Buff refresh/replacement tracking
- Combat and non-combat buff testing
- Map-change/reload testing
- Character-change detection
- Clearing of stale Food/Utility state when a new character is detected

### Food and Utility Expiration Reminders

- Food expiration warning
- Utility expiration warning
- Combined Food + Utility warning
- Live remaining-duration countdown in reminder popup
- Warning state resets when buffs are refreshed above the configured warning threshold

### Missing Buff Reminders

- Combat-entry detection through ArcDPS
- Enter Combat state tracking
- Exit Combat state tracking
- Food Missing warning
- Utility Missing warning
- Combined Food + Utility Missing warning
- Missing-buff reminders fire once per combat
- Missing-buff reminders re-arm after leaving combat

### Primer Tracking

- Metabolic Primer detection
- Utility Primer detection
- Metabolic Primer remaining-duration tracking
- Utility Primer remaining-duration tracking
- Primer expiration timestamp persistence
- Primer state restoration after addon reload when previously detected
- Metabolic Primer expiration warning
- Utility Primer expiration warning
- Combined Primer expiration warning
- Developer test reminders for individual and combined Primer warnings
- Correct Primer countdown display in reminder popups
- Primer-state inference from unusually long Food/Utility durations when direct Primer state is unavailable

### Squad Consumable Tracking

- ArcDPS nearby-player discovery
- Character-name tracking
- Account-name tracking
- ArcDPS agent and instance ID tracking
- Profession and elite-specialization tracking
- Subgroup and team tracking
- Local-player identification
- Squad Food tracking
- Squad Utility tracking
- Live squad consumable countdown timers
- Food and Utility buff application handling
- Food and Utility buff removal handling
- ArcDPS `BuffInitial` snapshot handling
- Consumable proc-effect filtering
- Known consumable classification
- Unknown consumable effect ID display
- Expanded Food and Utility definition database
- Live comparison testing against the existing ArcDPS Food Tracker
- Live-tested Food/Utility state semantics
- Unknown-state display using `?`
- Confirmed missing-consumable display using `None`
- Known consumable short labels
- Unmapped consumable display using `Unknown (ID)`

### Unknown Consumable Collector

FoodReminder-Nexus includes a development-oriented collector for unidentified Food and Utility effects.

The collector supports:

- Automatic background capture of unmapped Food and Utility effect IDs
- Collection without requiring the Squad window to remain open
- Separate Food and Utility classification
- Unique effect-ID deduplication
- `Seen` counters for repeated observations
- Copy All Unknown IDs
- Clear Unknown List
- Persistent unknown-consumable storage through `FoodReminder.ini`
- Restoration of collected unknowns after restarting the game/addon

Persistence has been verified in game.

During testing, unknown Utility effects remained visible in the collector after a full restart while at character select with zero ArcDPS-tracked players, confirming that the entries were restored from disk rather than rediscovered during the new session.

The collector is intended to make expansion of the consumable database significantly easier without requiring the Squad interface to be watched continuously.

### Consumable Database Expansion

The internal Food and Utility database has been expanded substantially through live ArcDPS testing and comparison with known consumables.

Recent identified effects include examples such as:

- Mushroom Pizza
- Soy-Sesame Sous-Vide Steak
- Tray of Decade Desserts
- Plate of Coq Au Vin with Salsa
- Clove and Veggie Flatbread
- Spherified Cilantro Oyster Soup
- Peppercorn and Veggie Flatbread
- Sesame Asparagus and Cured Meat Flatbread
- Powerful Potion of Demon Slaying
- Writ of Masterful Strength
- Writ of Masterful Malice
- Peppermint Oil
- Magnanimous Maintenance Oil

Unknown effects are intentionally retained as `Unknown (ID)` rather than guessed.

This allows tracking to continue while unidentified effects are collected for later research and mapping.

### Session Reporting

FoodReminder-Nexus now includes a Session Report for analyzing consumable usage during the current gameplay session.

The report currently tracks:

- Total session time
- Total combat time
- Food active time
- Utility active time
- Food coverage across the full session
- Utility coverage across the full session
- Food coverage specifically while in combat
- Utility coverage specifically while in combat
- Food application count
- Utility application count
- Food refresh count
- Utility refresh count
- Food replacement count
- Utility replacement count
- Food expirations occurring during combat
- Utility expirations occurring during combat

Session Coverage and In-Combat Coverage are intentionally separate.

This allows the addon to distinguish between a player being without consumables while idle or traveling and actually entering combat without active consumables.

Consumable applications are also classified independently:

- An application occurs when a consumable becomes active after previously being inactive.
- A refresh occurs when the same active consumable is applied again.
- A replacement occurs when an active consumable is replaced with a different consumable.

Food and Utility maintain independent statistics.

A `Reset Session` control clears the current Session Report without altering active consumable or reminder state.

Application, refresh, replacement, and in-combat expiration tracking have been validated in game for both Food and Utility.

### Debugging and Testing

- ArcDPS event counter
- Buff-like event counter
- Recent buff-event inspection
- Local-player event filtering
- Combat-state debug display
- Primer warning test buttons
- Buff debug reset
- Source/destination event inspection during development
- Squad player tracking inspection
- Unknown squad consumable ID inspection
- Background unknown-consumable collection
- Unknown effect Seen counters
- Unknown effect export
- Persistent unknown-effect restoration testing
- Session/combat coverage validation
- Food/Utility application classification testing
- Food/Utility refresh and replacement testing
- In-combat expiration testing

---

## Current Detection Behavior

FoodReminder-Nexus receives buff and combat information through ArcDPS combat events.

Food and Utility buffs can be detected when ArcDPS emits the relevant buff-state events, and their remaining durations are calculated locally.

ArcDPS does not necessarily resend every existing buff immediately when:

- Logging in
- Changing maps
- Switching characters
- Reloading the addon

Because of this, some tracking state may not be known until ArcDPS emits a relevant event.

### Character Switching

Food and Utility state is treated as character-specific.

When the addon detects that ArcDPS is now reporting a different local character, stale Food and Utility state from the previous character is cleared.

There may still be a short period after switching characters where the old state remains visible until ArcDPS emits the first self-related event for the new character.

### Primer Detection

Primers are tracked directly when their ArcDPS events are observed.

Because ArcDPS does not always resend already-active Primer state after map changes or login, the addon may also display an **inferred Primer duration** when Food or Utility has an unusually long remaining duration consistent with Primer extension.

Inferred Primer values are clearly labeled as:

`(inferred)`

### Squad Tracking

Nearby player discovery uses ArcDPS tracking-change events.

Players appear in the Squad tracker when ArcDPS begins tracking them in the current area or instance.

This means the Squad tracker represents players currently known to ArcDPS and does not necessarily represent every member of a full squad at all times.

When combat begins, ArcDPS can provide `BuffInitial` records containing existing Food and Utility state for tracked players.

FoodReminder-Nexus uses these records to populate active consumables and calculate their remaining durations.

The Squad tracker distinguishes four states:

- `?` — ArcDPS has not yet established the player's Food/Utility state
- `None` — ArcDPS has established that no Food/Utility buff is active
- Known label — recognized active consumable
- `Unknown (ID)` — active consumable whose effect ID is not yet mapped

Known consumables are displayed using short classifications such as:

- `Power`
- `Prec`
- `Condi`
- `Exper`
- `PConc`
- `CConc`
- `Heal`
- `Slay`
- `All`

If ArcDPS reports an active Food or Utility effect that is not currently present in the internal consumable database, FoodReminder-Nexus displays:

`Unknown (Effect ID)`

The effect remains tracked normally while its ID is also captured by the Unknown Consumable Collector.

### Unknown Consumable Collection

Unknown consumables are captured from ArcDPS events in the background.

The Squad window does not need to remain open for collection to occur.

Each unique unknown is stored with:

- Food or Utility classification
- ArcDPS effect ID
- Seen count

The Seen count represents the number of times the collector has observed the effect. It does not necessarily represent a unique-player count because ArcDPS may report the same effect multiple times.

Collected unknowns are persisted through `FoodReminder.ini` and restored after restart.

This allows unidentified effects to accumulate naturally during normal group gameplay rather than requiring manual monitoring of the Squad window.

### Session Reporting Behavior

Session statistics are accumulated while the addon is active during gameplay.

Food and Utility coverage are measured independently.

The report separates total-session coverage from in-combat coverage so time spent traveling, waiting, or otherwise outside combat does not obscure consumable performance during actual combat.

The report is currently session-based rather than a permanent historical record.

Using `Reset Session` clears the current report statistics.

---

## Jade Tech Protocol Investigation

Jade Tech Offensive and Defensive Protocol tracking was investigated during development.

ArcDPS successfully reports interaction with the station as:

- `Generic gadget interact`
- Skill ID `23302`
- State changes `67` and `68`
- Destination agent: `Jade Tech Enhancement Console`

However, the ArcDPS combat-event stream currently does not provide a reliable way for FoodReminder-Nexus to determine whether the interaction granted:

- Jade Tech Offensive Protocol
- Jade Tech Defensive Protocol

Testing also did not reveal reliable direct buff events for the corresponding Jade Tech Overcharge effects through the current event stream.

Because the addon cannot reliably distinguish the two protocol types, Jade Tech Protocol tracking is **not currently implemented**.

This remains an experimental/future investigation.

---

## In Development

Current priorities include:

- Add per-consumable usage history to the Session Report
- Record Food and Utility effect IDs used during a session
- Display recognized consumable names in session history
- Track per-consumable usage counts
- Explore optional consumable usage/cost analysis after usage history is reliable
- Improve Food/Utility state synchronization after login or map change
- Improve synchronization with already-active buffs
- Improve character-switch synchronization before the first ArcDPS self event
- Continue expanding the consumable definition database
- Automatically remove newly recognized effects from the persistent Unknown Consumables list
- Improve Squad tracker presentation
- Continue live Squad tracking validation
- Continue expiration/reminder edge-case testing
- Improve the player-facing reminder interface
- Reduce/remove development debug information once tracking is stable
- Continue stability testing across maps and characters

Several unknown Utility effect IDs remain intentionally unmapped until they can be reliably identified.

FoodReminder-Nexus prefers displaying an effect as unknown rather than assigning an uncertain or guessed consumable name.

---

## Planned Features

Future development may include:

- Per-consumable session usage history
- Consumable usage analysis
- Optional consumable cost analysis
- Session cost-per-hour estimates
- Compact optional Food/Utility timer HUD
- Color stages for remaining duration
- Additional reminder customization
- Combat-aware reminder behavior
- Optional delayed large popup after combat
- Per-character settings
- Per-character preferred consumables
- Optional wrong-consumable warning
- Additional supported consumable effects
- Continued consumable database expansion
- Improved initial-state synchronization
- Improved character-change synchronization
- Squad tracker UI cleanup and customization
- Automatic cleanup of mapped entries from the Unknown Consumables collector
- Optional Jade Tech tracking if a reliable data source becomes available

---

## Debugging

The current development build contains a **Developer Debug** section.

This currently displays information such as:

- Combat state
- ArcDPS event count
- Buff-like event count
- Recent buff events
- Event skill ID
- Event value
- Buff state
- Removal state
- ArcDPS state-change value
- Local-player source/destination state
- Source agent name
- Destination agent name
- Primer reminder test controls

Additional Squad development information is currently visible through the Squad interface, including player identity data, unknown consumable effect IDs, and the Unknown Consumable Collector.

This interface exists for development and testing and is not intended to represent the final addon UI.

---

## Project Philosophy

FoodReminder-Nexus is intended to remain lightweight and focused.

The addon should:

- Provide useful information without cluttering the screen
- Avoid requiring the player to constantly monitor the GW2 buff bar
- Give clear warnings before important consumable buffs expire
- Warn when important consumables are missing at the start of combat
- Remain useful for normal open-world and solo PvE gameplay
- Provide useful squad consumable information without becoming a full combat meter
- Provide useful consumable-session information that is not readily visible in the normal Guild Wars 2 interface
- Avoid unnecessary complexity
- Prefer reliable tracking over guessed or misleading information
- Prioritize stability before additional features

---

## Development

This project is written in **C++** and developed using **Visual Studio**.

It integrates with:

- Guild Wars 2
- Nexus
- ArcDPS combat events

Optional integration with **Unofficial Extras** is also being investigated for additional squad metadata.

The project is currently being developed and tested directly in Guild Wars 2.

---

## Repository

This repository contains the active development source for **FoodReminder-Nexus**.

Development history and testing checkpoints are maintained separately in:

`DEVELOPMENT_LOG.md`

---

## Disclaimer

FoodReminder-Nexus is an unofficial Guild Wars 2 addon.

It is not affiliated with or endorsed by ArenaNet.

Guild Wars 2 and all associated trademarks are property of their respective owners.
