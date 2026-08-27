# FoodReminder-Nexus — Development Log

A chronological record of major development changes, testing results, fixes, and stable checkpoints.

---

## 2026-08-27 — Trading Post Watcher, Search, History, Alerts & Market Analysis

### Added
- Built-in Trading Post Watcher.
- Support for watching multiple Trading Post items simultaneously.
- Current lowest sell price display.
- Current highest buy price display.
- Per-item Sell Targets.
- Target status reporting.
- Manual per-item price refresh.
- Refresh All control.
- Automatic periodic Trading Post price checks.
- Persistent watched-item storage.
- Persistent Sell Target storage.
- Asynchronous Trading Post price retrieval.
- Local searchable Trading Post item index.
- Name-first Trading Post item search.
- Live autocomplete suggestions while typing an item name.
- Item ID display for reference without requiring manual item-ID entry.
- Persistent local Trading Post price-history storage.
- `TradingPostHistoryManager`.
- Buy and sell history observations stored with timestamps.
- History persistence across addon/game restarts.
- Compact buy and sell history sparklines.
- Historical minimum, average, and maximum prices.
- Tiered Trading Post history retention:
  - Full-detail recent history.
  - Reduced sampling for older history.
  - Long-term historical sampling.
- Time-based Trading Post trend analysis.
- Selectable trend windows:
  - 15 minutes
  - 30 minutes
  - 1 hour
  - 6 hours
  - 24 hours
- Actual coin-value and percentage change over the selected trend window.
- Buy/sell spread calculation.
- Deal-quality analysis:
  - `FAVORABLE`
  - `TYPICAL`
  - `EXPENSIVE`
- Combined buying-opportunity signal:
  - `GOOD BUY`
  - `WATCH`
  - `OVERPRICED`
- Fresh-API-only Sell Target alert triggering.
- Persistent target-reached latch state.
- Anti-spam target alert behavior.
- Dismissible target-reached banner.
- Dragon Bash-style target celebration effects.
- Animated confetti.
- Animated sparkle/firework effects.
- Pulsing celebration frame.
- Highlighting of the watched item that triggered the alert.

### Changed
- Trading Post monitoring is no longer limited to consumables used by the Session Report.
- Watched Trading Post items persist across addon/game restarts.
- Trading Post price requests run asynchronously so network requests do not block the game/render thread.
- Item addition was redesigned so manually knowing and entering an item ID is no longer required.
- Trading Post item search is now name-first.
- Item IDs remain visible for reference but are no longer required user input.
- The target-price column and help text were renamed/clarified as `Sell Target`.
- Sell Target is defined as the maximum price the user is willing to pay.
- A target is considered reached when the current lowest sell listing is equal to or below the configured Sell Target.
- Target alerts now wait for a fresh Trading Post API observation instead of firing immediately from an already-cached price after a target edit.
- Target alerts fire once when entering the target range and do not repeat on every automatic refresh.
- A dismissed alert remains dismissed while the price stays within the same target range.
- Target alerts automatically re-arm after the sell price rises back above the Sell Target.
- Price trends were changed from recent-sample comparisons to actual time-window comparisons.
- Trend display now reports actual coin movement, percentage movement, and the selected time period.
- Deal analysis uses the selected trend window rather than a fixed time range.
- Deal quality was changed from a universal percentage threshold to a price-distribution-based system using each item’s own locally collected market history.
- Deal/history calculations were adjusted to tolerate gaps caused by the game or addon not running continuously.
- Watched-item layout spacing was increased slightly to improve readability as more market information was added.
- Long item names were moved onto their own line so item ID text could no longer spill into the Sell column.
- The next automatic API refresh timer was moved to a prominent colored display.
- History details were tightened so sparklines use available horizontal space and Min/Avg/Max statistics no longer leave excessive empty space.

### Fixed
- Trading Post item-index construction previously stopped at exactly 8,800 items.
- ArenaNet bulk `/v2/items` responses using HTTP `206 Partial Content` are now accepted.
- Full local Trading Post item index successfully built after the HTTP 206 fix.
- Stale duplicate-item warning remained visible after changing or clearing the item search text.
- Duplicate warning now clears when the search text changes.
- History collapsing sections previously closed when the observation count changed because the visible count was part of the ImGui identifier.
- History headers now use stable internal ImGui IDs.
- Flat price history initially rendered as visually empty chart areas.
- Sparkline scaling now preserves visible flat-price lines.
- Long watched-item names caused truncated Item ID text to appear inside the Sell column.
- Item name and Item ID/update information are now separated cleanly.
- Initial target-alert implementation could fire immediately after changing a Sell Target using an already-cached market price.
- Target editing now marks the current cached price as already processed so the alert waits for the next fresh API result.
- Target alert banner spam was prevented with a persistent reached-state latch.
- Deal/Signal analysis initially remained in `collecting history` after history gaps because it required too many observations strictly inside the selected time window.
- Deal/Signal analysis now uses a historical boundary anchor and can operate with sparse observations after game-off periods.
- Small price movement could previously display as `FLAT +0.0%` even while the sparkline visibly moved.
- Trend reporting now preserves actual copper movement and higher percentage precision.

### Tested
- Trading Post Watcher successfully retrieved live buy and sell prices.
- Multiple watched items were successfully tracked simultaneously.
- Watch-list persistence passed a full restart test.
- Saved Sell Targets restored correctly.
- Automatic periodic price checks operated across watched items.
- Local Trading Post index successfully completed with approximately 27,961 searchable Trading Post items during testing.
- Name-first autocomplete successfully located and added real Trading Post items.
- Duplicate watched items were correctly rejected.
- Search-status clearing behavior passed.
- Persistent Trading Post history file was created successfully.
- New observations appended successfully.
- History survived full game/addon restart.
- Historical observation counts restored correctly.
- Min/Avg/Max calculations displayed correctly.
- Buy and sell sparklines displayed live price movement.
- Tiered history-retention build passed without deleting recent history.
- 15-minute trend analysis displayed real values.
- 30-minute trend analysis displayed real values.
- 1-hour trend analysis displayed real values.
- 6-hour trend analysis correctly reported that additional history was still being collected.
- 24-hour trend analysis correctly reported that additional history was still being collected.
- Sell Target banner triggered only after a fresh API refresh.
- Sell Target status could immediately display `TARGET REACHED` while the alert itself correctly waited for the next API observation.
- Dismissing a target alert prevented the banner from firing again on later refreshes while the price remained within target.
- Persistent `TARGET REACHED` row status remained visible after dismissing the one-time banner.
- Dragon Bash-style celebration rendered successfully in-game.
- Animated confetti, sparkle effects, pulsing border, and watched-row highlighting displayed correctly.
- Trading Post deal-quality analysis produced live classifications.
- Spread calculation displayed the difference between lowest Sell and highest Buy.
- Combined opportunity signals produced live `GOOD BUY`, `WATCH`, and `OVERPRICED` states.
- History-gap handling populated Deal/Signal analysis without requiring continuous gameplay.
- Real-world Aurene’s Bite testing captured a sell-price drop to approximately 1,910 gold.
- During that live price movement, Aurene’s Bite was correctly identified as `FAVORABLE` with a `GOOD BUY` signal based on its locally collected recent history.

### Known Limitations
- Trading Post historical data begins only after FoodReminder-Nexus observes an item.
- ArenaNet does not provide historical Trading Post price data through the current commerce API.
- Historical trends, averages, deal ratings, and signals therefore depend on the addon’s own locally collected observations.
- Longer trend windows such as 6 hours and 24 hours require sufficient locally collected history before analysis is available.
- Deal-quality and opportunity signals are informational only and are not guarantees that an item will continue rising or falling.
- The target celebration currently appears inside the Trading Post Watcher interface.
- A standalone target-hit notification outside the Trading Post tab is not yet implemented.
- Custom image-based Dragon Bash assets are not currently used; celebration effects are generated through ImGui drawing.

### Next
- Add a standalone Trading Post target-hit notification overlay that can appear while the player is actively playing and the Trading Post tab is closed.
- Reuse the existing fresh-API-only alert and anti-spam latch logic for the standalone overlay.
- Keep the larger Dragon Bash-style celebration inside the Trading Post Watcher.
- Continue long-term Trading Post history collection for 6-hour and 24-hour trend validation.
- Continue live testing of deal-quality and opportunity-signal behavior across expensive and inexpensive Trading Post items.

### Status
✅ Trading Post price retrieval working  
✅ Multi-item watching passed  
✅ Watch-list persistence passed  
✅ Sell Target persistence passed  
✅ Name-first item search/autocomplete passed  
✅ Full local Trading Post item index built successfully  
✅ HTTP 206 Partial Content handling fixed  
✅ Persistent local price history working  
✅ History restart persistence passed  
✅ Tiered history retention implemented  
✅ Buy/sell sparklines working  
✅ Min/Avg/Max analysis working  
✅ Selectable 15m / 30m / 1h / 6h / 24h trend windows working  
✅ Sell Target alert backend passed  
✅ Fresh-API-only target triggering passed  
✅ Anti-spam alert behavior passed  
✅ Dragon Bash-style celebration working  
✅ Deal-quality analysis working  
✅ Spread analysis working  
✅ GOOD BUY / WATCH / OVERPRICED opportunity signals working  
✅ History-gap handling working  
✅ Latest Trading Post milestones committed and pushed  
➡️ Next checkpoint: standalone Trading Post target-hit overlay

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