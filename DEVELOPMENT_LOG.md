# FoodReminder-Nexus — Development Log

A chronological record of major development changes, testing results, fixes, and stable checkpoints.

---

## 2026-08-27 — Trading Post Watcher & Item Search Development

### Added
- Built-in Trading Post Watcher.
- Support for watching multiple Trading Post items simultaneously.
- Current lowest sell price display.
- Current highest buy price display.
- Per-item target sell prices.
- Target status reporting.
- Manual per-item price refresh.
- Refresh All control.
- Automatic periodic Trading Post price checks.
- Persistent watched-item storage.
- Persistent target-price storage.
- Initial asynchronous item lookup and validation infrastructure.
- Trading Post lookup status and error feedback in the user interface.

### Changed
- Trading Post monitoring is no longer limited to consumables used by the Session Report.
- Watched Trading Post items persist across addon/game restarts.
- Trading Post price requests run asynchronously so network requests do not block the game/render thread.
- Item addition is being redesigned so manually knowing and entering an item ID is not required.
- Item IDs will remain available internally and may be displayed for reference, but the intended user-facing workflow is item-name search.

### Tested
- Trading Post Watcher successfully retrieved live buy and sell prices.
- Multiple watched items were successfully tracked simultaneously.
- Watch-list persistence passed a full restart test.
- Saved target prices restored correctly.
- Automatic periodic price checks operated across watched items.
- Invalid item input produced visible lookup feedback.
- Current automatic item lookup code rebuilt successfully.

### Known Limitations
- Current item-entry workflow is not considered complete.
- Requiring users to manually find Guild Wars 2 item IDs was rejected as poor user experience.
- The initial chat-link/ID lookup approach was also rejected as the primary user-facing workflow.
- Guild Wars 2 item-name autocomplete is not yet implemented.
- Persistent historical price observations and charts are not yet implemented.

### Next
- Build a local searchable Trading Post item index.
- Add live item-name autocomplete suggestions while the user types.
- Allow a search result to display its item ID for reference without requiring the user to know the ID.
- Validate selected items before adding them to the watch list.
- Persist Trading Post price observations over time.
- Build historical buy/sell price charts from the locally collected dataset.

### Status
✅ Trading Post price retrieval working  
✅ Multi-item watching passed  
✅ Watch-list persistence passed  
✅ Target-price persistence passed  
✅ Asynchronous lookup infrastructure builds successfully  
⚠️ Name-first item search/autocomplete not yet implemented  
⚠️ Current item-entry work not yet committed as a completed feature  
➡️ Next checkpoint: Trading Post item index + live autocomplete

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