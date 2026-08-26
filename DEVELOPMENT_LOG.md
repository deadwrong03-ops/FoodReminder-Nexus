# FoodReminder-Nexus — Development Log

A chronological record of major development changes, testing results, fixes, and stable checkpoints.

---

## 2026-08-26 — Primer-Aware Session Tracking

### Added
- Session Report now tracks time protected by Metabolic Primer.
- Session Report now tracks time protected by Utility Primer.
- Primer protection is separated from normal consumable-duration analysis.

### Fixed
- Extended Food/Utility duration caused by an active Primer is no longer treated as ordinary wasted consumable time.

### Tested
- Metabolic Primer timer confirmed active during normal gameplay.
- Utility Primer timer confirmed active during normal gameplay.
- Session Report correctly displayed both active Primer timers.

### Status
✅ Build successful  
✅ In-game test passed  
✅ Committed  
✅ Pushed

---

## 2026-08-25 — Consumable History & Trading Post Cost Tracking

### Added
- Per-consumable Food usage history.
- Per-consumable Utility usage history.
- Usage counts for individual consumables.
- Trading Post price lookup for recognized consumables.
- Session consumable cost calculation.

### Tested
- Soul Pastry successfully retrieved Trading Post pricing.
- Session Report calculated Utility cost correctly.
- Grumble Cake usage was tracked, but Trading Post pricing was unavailable.

### Known Limitations
- Not every consumable has Trading Post pricing.
- Items without a usable Trading Post listing display `Price not loaded`.
- Cost calculations only include consumables with successfully retrieved pricing.

### Status
✅ Build successful  
✅ In-game test passed  
✅ Git repository cleaned of generated catalog/cache files  
✅ Pushed

---

## 2026-08-24 — Session Report

### Added
- Session Report tab.
- Session time and combat time.
- Food and Utility session coverage.
- Food and Utility in-combat coverage.
- Consumable applications.
- Refresh detection.
- Replacement detection.
- Expiration-during-combat tracking.
- Reset Session control.

### Fixed
- Refresh/replacement classification.
- Utility statistics appearing in the Food section.

### Tested
- Food application, refresh, and replacement tracking.
- Utility application, refresh, and replacement tracking.
- Food expiration during combat.
- Utility expiration during combat.
- Session reset.
- Independent session/in-combat coverage calculations.

### Status
✅ Build successful  
✅ In-game validation passed  
✅ Temporary test hooks removed  
✅ Committed and pushed

---

## 2026-08-22–23 — Squad Tracker & Unknown Consumable Collector

### Added
- Squad Food/Utility tracking.
- Live consumable countdowns.
- Known/unknown/missing state handling.
- Automatic Unknown Consumable Collector.
- Seen counters.
- Unknown-ID export.
- Persistent unknown-ID storage.
- Automatic restoration after restart.

### Consumable Database
Expanded Food and Utility mappings using live ArcDPS testing.

Unknown effects remain intentionally unmapped until positively identified.

### Tested
- Background collection with Squad window closed.
- Unknown-ID deduplication.
- Persistence across full game restart.
- Restoration with zero currently tracked ArcDPS players.
- Squad timer comparison against existing ArcDPS Food Tracker.

### Status
✅ Squad tracking working  
✅ Collector working  
✅ Persistence working  
✅ Database expanded  
✅ Builds successful  
✅ Committed and pushed

---

## Log Format

Each development checkpoint uses the same structure when applicable:

### Added
New functionality.

### Changed
Changes to existing behavior.

### Fixed
Bug fixes and corrections.

### Tested
Important in-game or development validation.

### Known Limitations
Anything intentionally unresolved.

### Status
Build, testing, and Git checkpoint state.