# FoodReminder-Nexus

> ⚠️ **Early Development Build**
>
> FoodReminder-Nexus is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food and utility buff durations and providing configurable expiration warnings.

The goal is simple:

> **Know when your food and utility buffs are about to expire without constantly watching the buff bar.**

---

## Current Version

**0.1.0 Development Build**

FoodReminder-Nexus is currently an experimental development project.

Features, interfaces, configuration formats, and internal systems may change as development continues.

---

## Current Status

FoodReminder-Nexus has a working Nexus addon framework and an early ArcDPS-powered buff detection system.

The addon can currently receive ArcDPS combat events, identify food and utility buffs belonging to the local player, calculate their remaining duration, and display that information through the Nexus configuration/debug interface.

The current development focus is making buff detection reliable across normal gameplay situations before building the final reminder interface.

---

## Completed

### Core Addon Framework

- Nexus addon loading and unloading
- Nexus configuration panel
- ArcDPS combat event integration
- Food and utility reminder settings
- Configurable food early-warning time
- Configurable utility early-warning time
- Enable/disable reminders option
- Test Reminder button
- Development/debug interface

### Buff Tracking Foundation

- Local-player buff filtering
- ArcDPS buff-event processing
- Food buff detection
- Utility buff detection
- Food remaining-time calculation
- Utility remaining-time calculation
- Buff expiration timestamp tracking
- Buff removal handling
- Detection state reset when buffs disappear
- Map-change/reload testing
- Combat and non-combat buff testing
- Debug event counters
- Recent buff-event inspection

---

## Current Detection Behavior

The addon receives buff information through ArcDPS combat events.

During testing, food and utility buffs have successfully been detected and their remaining durations calculated.

Because ArcDPS does not necessarily provide every existing buff immediately when a character loads into a map, detection may depend on ArcDPS emitting a relevant buff-state event.

This behavior is still being investigated and improved.

---

## In Development

Current priorities include:

- Improve initial food/utility detection after login or map change
- Improve synchronization with existing buffs
- Verify behavior while solo and while grouped
- Verify buff replacement and refresh behavior
- Improve expiration handling
- Build the final player-facing food/utility timer display
- Add reliable early-warning notifications
- Reduce/remove development debug information once tracking is stable
- Continue stability testing across maps and characters

---

## Planned Features

Future development may include:

- Compact food timer
- Compact utility timer
- Configurable reminder timing
- Visual expiration warnings
- Cleaner reminder popup
- Persistent settings
- Optional HUD display
- Improved buff-name recognition
- Support for additional food and utility effects
- Better handling of unusual buff durations
- Improved map-change synchronization
- Improved character-change synchronization

---

## Debugging

The current development build contains an **ArcDPS Buff Debug** section.

This currently displays information such as:

- ArcDPS event count
- Detected buff count
- Detected food state
- Detected utility state
- Remaining food duration
- Remaining utility duration
- Buff-like event count
- Recent buff events

This interface exists for development and testing and is not intended to represent the final addon UI.

---

## Project Philosophy

FoodReminder-Nexus is intended to remain lightweight and focused.

The addon should:

- Provide useful information without cluttering the screen
- Avoid requiring the player to constantly monitor the GW2 buff bar
- Give clear warnings before important consumable buffs expire
- Remain useful for normal open-world and solo PvE gameplay
- Avoid unnecessary complexity
- Prioritize stability before additional features

---

## Development

This project is written in **C++** and developed using **Visual Studio**.

It integrates with:

- Guild Wars 2
- Nexus
- ArcDPS combat events

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
