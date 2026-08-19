# Food Reminder

> ⚠️ **Early Development Build**
>
> Food Reminder is under active development and is not yet feature complete.

A lightweight **Guild Wars 2 Nexus addon** for tracking food and utility buffs and warning the player before they expire.

The goal is simple:

> **Know when your consumable buffs are about to expire without constantly checking the buff bar.**

Food Reminder is being developed as a native Nexus addon using ArcDPS combat/buff events for buff detection.

---

## Current Version

**0.1.0 Development Build**

Food Reminder is currently an experimental development project.

Features, interfaces, configuration formats, and internal systems may change as development continues.

---

## Current Status

Food Reminder has a working Nexus addon foundation and can detect the player's active food and utility buffs from ArcDPS buff events.

Food and utility durations are tracked locally after detection, including replacement and refresh events.

The current development focus is making buff lifecycle detection reliable before expanding the reminder interface and configuration options.

---

## Completed

### Core Addon Framework

- Native Nexus addon
- Nexus addon loading and unloading
- Options panel
- ImGui rendering
- ArcDPS event integration
- Food Reminder test notification
- Configurable food early-warning time
- Configurable utility early-warning time
- Enable/disable reminders

### Buff Detection

- ArcDPS buff-event capture
- Self-targeted buff filtering
- Food detection using the `Nourishment` buff category
- Utility detection using the `Enhancement` buff category
- Food duration tracking
- Utility duration tracking
- Live countdown timers
- Food refresh/replacement detection
- Utility refresh/replacement detection
- Support for different Nourishment skill IDs
- Natural timer expiration handling
- ArcDPS buff-removal handling
- Development/debug event display

---

## In Development

- Reliable buff lifecycle verification
- Expiration/removal testing
- Reminder triggering from live food and utility timers
- Reminder presentation and positioning
- Settings persistence
- Debug UI cleanup

---

## Planned

- Food expiration warnings
- Utility expiration warnings
- Configurable warning times
- Clear on-screen reminder notifications
- Improved reminder positioning
- Persistent settings
- Optional debug/development tools
- General UI cleanup

---

## Development Philosophy

Food Reminder is intended to remain small and focused.

The addon should:

- Detect consumable buffs reliably
- Warn the player at useful times
- Avoid unnecessary screen clutter
- Require minimal configuration
- Remain useful for normal open-world and PvE gameplay

---

## Development Status

This repository contains development code.

Expect changes while the ArcDPS buff-event integration and reminder system are being tested.