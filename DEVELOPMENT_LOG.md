# FoodReminder-Nexus — Development Log

A chronological record of major development changes, testing results, fixes, and stable checkpoints.

---


## 2026-09-04 — Trading Post UI & Item Index Synced with Copper&Claw

### Changed
- Brought FoodReminder-Nexus's Trading Post interface up to the current Copper&Claw market-analysis implementation.
- Replaced the older combined Deal / Signal presentation with separate Sell Listings and Buy Orders analysis so buyer-facing and seller-facing information is no longer conflated.
- Sell-side direction now describes whether listings are getting more expensive, getting cheaper, or remaining stable.
- Buy-side direction now describes whether buyers are offering more, offering less, or remaining stable.
- Direction displays now include percentage movement for the active trend window.
- Added current-price position relative to the locally observed recent average.
- Added separate buyer and seller guidance:
  - Sell Listings: `Favorable`, `Worth Watching`, `Typical`, or `Caution`.
  - Buy Orders: `Favorable`, `Worth Watching`, `Typical`, or `Weak`.
- Price position now takes priority over short-term direction so a rising Sell trend alone cannot incorrectly label a still-below-average listing as overpriced.
- Added market-analysis Confidence, Coverage, and Samples reporting.
- Added developing-history handling so newly watched items can show locally observed direction before a full selected trend window is available.
- Added clear local-history messaging explaining that Trading Post analysis begins when FoodReminder starts observing an item and does not include earlier market history.
- Updated the Trading Post target celebration to the newer larger price-card layout used by Copper&Claw.
- Updated the local Trading Post item index to support optional stat-based variant labels for duplicate-name items.
- Variant labels are now searchable and shown in item-search results when available.
- The local item-index cache now stores item ID, name, and variant label.
- Legacy two-column item-index caches remain loadable and automatically trigger a refresh into the new format.
- FoodReminder-specific persistence names and HTTP user agent remain unchanged.

### Fixed
- Older Trading Post Deal / Signal logic could produce contradictory analysis by treating a sufficiently rising Sell trend as `OVERPRICED` even when the current Sell price was still favorable relative to its recent local history.
- Duplicate-name Trading Post items could be difficult to distinguish in autocomplete when separate item IDs represented different stat variants.
- Market analysis for newly tracked items could remain unnecessarily uninformative while the selected trend window was still developing.

### Tested
- Full FoodReminder rebuild passed after the Trading Post UI and item-index update.
- FoodReminder loaded normally in-game after the sync.
- Trading Post tab opened and rendered normally.
- Existing watched items and Sell Targets remained intact.
- Updated Sell Listings / Buy Orders analysis rendered normally.
- Updated target-alert presentation rendered normally.
- No crash or regression was observed during the post-update in-game smoke test.

### Status
✅ Trading Post UI synced with current Copper&Claw implementation  
✅ Separate Sell Listings / Buy Orders analysis working  
✅ Direction percentages working  
✅ Price-position logic working  
✅ Buyer / seller signals working  
✅ Confidence / Coverage / Samples working  
✅ Local-history disclaimer added  
✅ Variant-aware item search/index support added  
✅ Legacy item-index cache compatibility retained  
✅ Full rebuild passed  
✅ In-game smoke test passed  

### Next
- Commit and push the verified FoodReminder Trading Post synchronization checkpoint.
- Continue normal Trading Post history accumulation.
- Preserve FoodReminder's existing Trading Post history while preparing the planned history merge into the standalone Copper&Claw addon.

---

## 2026-09-03 — UI Refactor Complete: SessionUI, TrackerUI & ReminderUI

### Changed
- Completed the controlled zero-behavior-change UI extraction from `entry.cpp`.
- Moved the Session Report interface into new `SessionUI.h/.cpp` files.
- Moved the compact Food Reminder tracker into new `TrackerUI.h/.cpp` files.
- Moved the reminder popup rendering into new `ReminderUI.h/.cpp` files.
- Preserved existing SessionTracker accounting, persistence, Primer-state handling, reminder logic, Trading Post alerts, and addon runtime behavior.
- Tracker developer color-test controls now route through `TrackerUI` instead of a global `entry.cpp` test variable.
- ReminderManager update/state logic remains in `entry.cpp`; only the reminder popup presentation moved into `ReminderUI`.
- `entry.cpp` is now reduced to roughly 1,260 lines and is substantially closer to addon lifecycle, event routing, shared runtime state, settings, and tab wiring.

### Fixed
- The first SessionUI extraction build exposed a missing `ConsumableMetadataManager.h` include; the include was added and the build passed.
- After TrackerUI extraction, inferred-Primer hover tooltips could render as an extremely tall/narrow dark tooltip window.
- The Metabolic and Utility `Active*` tooltips now use an explicit wrap width, restoring the intended compact tooltip shape.

### Tested
- SessionUI extraction passed a full build.
- Session tab rendered normally in-game.
- Live Session Time, active Food/Utility time, and inferred Primer time continued advancing together as expected.
- TrackerUI extraction passed a full build.
- Compact tracker rendered normally with live Food/Utility timers, consumable names, Primer `Active*` state, and gold border.
- Primer hover tooltip regression was reproduced, fixed, rebuilt, and verified gone in-game.
- ReminderUI extraction passed a full build.
- The Food reminder popup rendered correctly in-game with the expected title, countdown, border, size, and position.
- No observed regressions were found in the previously extracted HistoryUI, TradingPostUI, SquadUI, or SessionUI paths during this checkpoint.

### Primer Investigation
- A temporary dedicated Primer investigation trace was added after a rare real Metabolic Primer countdown appeared during normal gameplay.
- Combat, map change, same-character relog, addon reload, world-boss activity, and Drakkar did not reliably reproduce the direct Primer countdown.
- Addon reload caused the real countdown to fall back to conservative inferred `Active*`, reinforcing that the direct timer depended on transient runtime ArcDPS state.
- The temporary investigation buffer/UI was removed after testing so diagnostic-only code would not remain in the normal build.
- Primer behavior remains conservative: direct countdowns are used when trustworthy live data is available; otherwise inferred presence or Unknown is shown without fabricating a timer.
- Further Primer investigation is shelved until a reproducible trigger or better data source becomes available.

### Status
✅ HistoryUI verified  
✅ TradingPostUI verified  
✅ SquadUI verified  
✅ SessionUI extraction/build/in-game verification passed  
✅ TrackerUI extraction/build/in-game verification passed  
✅ Tracker Primer tooltip regression fixed  
✅ ReminderUI extraction/build/in-game verification passed  
✅ Planned UI modularization complete  
✅ Temporary Primer diagnostics removed  
⚠️ Rare direct Metabolic Primer countdown trigger remains unknown  

### Next
- Perform a final low-risk cleanup/verification pass for dead includes/helpers left in `entry.cpp`.
- Keep behavior unchanged during cleanup.
- Commit and push this completed UI-refactor checkpoint.
- Return to feature/bug work after the refactor checkpoint is safely committed.

---

## 2026-09-02 — SquadUI Refactor & Primer State Observation

### Changed
- Continued the controlled zero-behavior-change `entry.cpp` refactor.
- Moved the Squad interface into new `SquadUI.h/.cpp` files.
- Preserved existing RTAPI/ArcDPS roster handling, Squad filters, Unknown Consumable collector, and Developer Tools behavior.
- `SquadTracker.*` and `RTAPIIntegration.*` behavior were intentionally left unchanged during the extraction.

### Tested
- `SquadUI` extraction passed a full build.
- Squad tab rendered normally in-game after extraction.
- RTAPI/ArcDPS roster-source status displayed normally.
- All three Squad player filters remained functional:
  - All players
  - Missing / Unknown / Unmapped
  - Unknown / Unmapped only
- Player rows, subgroup, character, Food, and Utility state rendered normally.
- Unknown Consumable IDs section remained functional.
- Developer Tools / Extras status continued to render normally.
- No observed Squad functionality regression was found after extraction.
- During later Primer observation testing, an active Metabolic Primer unexpectedly upgraded from inferred `Active*` to a real countdown.
- Entering combat did not resynchronize the Primer countdown.
- A normal map change did not reproduce or refresh the real Primer countdown.
- Same-character relog did not reliably reproduce the real Primer countdown.
- Unloading and reloading FoodReminder caused the Metabolic Primer to fall back from the real countdown to inferred `Active*`.
- This indicates the confirmed Primer countdown was likely supplied by a live runtime ArcDPS event/state delivery and was not reconstructible from the addon's persisted state alone.

### Known Limitations
- The exact ArcDPS event or state transition that supplied the temporary real Metabolic Primer countdown has not yet been identified.
- Reliable Primer countdown recovery remains intermittent.
- A dedicated Primer-specific event trace is planned for a future debugging pass so the exact transition from inferred/unknown state to confirmed countdown can be captured.

### Status
✅ HistoryUI refactor verified  
✅ TradingPostUI refactor verified  
✅ SquadUI extraction complete  
✅ SquadUI build passed  
✅ SquadUI in-game verification passed  
✅ No Squad regression observed  
⚠️ Rare live Metabolic Primer countdown recovery observed but trigger not yet identified  

### Next
- Commit and push the verified SquadUI refactor checkpoint.
- Add dedicated Primer event/state tracing in a later debugging session.
- Continue the controlled UI extraction one section at a time.
- Next planned extraction: `SessionUI`.

---

## 2026-09-02 — UI Refactor Checkpoint: HistoryUI & TradingPostUI

### Changed
- Began a controlled zero-behavior-change refactor of the large `entry.cpp`.
- Moved the Personal History interface into new `HistoryUI.h/.cpp` files.
- Moved the Trading Post interface into new `TradingPostUI.h/.cpp` files.
- Moved the large gameplay `TARGET REACHED!` celebration rendering into `TradingPostUI`.
- Moved Trading Post tab visibility tracking/suppression logic into `TradingPostUI` so the large gameplay celebration remains hidden while the compact in-tab target card is visible.
- `entry.cpp` is now focused more heavily on addon wiring and shared runtime flow instead of containing all History and Trading Post rendering code.

### Tested
- `HistoryUI` extraction passed a full build.
- Personal History opened and rendered normally after extraction.
- Current Character filtering, saved-character selection, time-range selection, Trends, Per-Item Usage, and Primer Details remained functional.
- `TradingPostUI` extraction passed a full build.
- Trading Post watch list, current Sell/Buy prices, Sell Target editing, status display, trend selector, history charts, Min/Avg/Max statistics, spread, deal-quality signal, item-search count, and automatic refresh countdown rendered normally after extraction.
- Large gameplay `TARGET REACHED!` celebration rendered correctly with the Trading Post tab closed after the refactor.
- Existing compact in-tab target presentation behavior remained intact.
- No observed functional regression was found in either extracted UI section.

### Status
✅ HistoryUI extraction complete  
✅ HistoryUI build passed  
✅ HistoryUI in-game verification passed  
✅ TradingPostUI extraction complete  
✅ TradingPostUI build passed  
✅ TradingPostUI in-game verification passed  
✅ Target-hit gameplay celebration verified after refactor  
✅ Zero-behavior-change refactor checkpoint accepted  

### Next
- Commit and push the verified HistoryUI/TradingPostUI refactor checkpoint.
- Continue the controlled UI extraction one section at a time.
- Next planned extraction: `SquadUI`.
- Build and test after each future extraction before moving to the next section.

---

## 2026-09-01 — Per-Character History & Trading Post Alert Queue

### Added
- Personal History now stores the character name with newly archived sessions.
- History character filtering now supports:
  - All Characters
  - Current Character
  - Searchable saved character names
  - Legacy / Unknown for sessions recorded before character tagging was added.
- Trading Post target alerts now queue when multiple watched items enter their Sell Target range at the same time.

### Changed
- Personal History now defaults to **Current Character** so players who regularly switch characters do not need to manually reselect the active character.
- Switching characters archives the previous character's session and starts a new character-specific session so one history record does not mix multiple characters.
- History aggregation, coverage markers, estimated spend, per-item usage, and Primer details all respect the selected character filter.
- Only one Trading Post target alert is presented at a time; dismissing the current alert immediately advances to the next queued target hit.
- Trading Post target presentation is now context-aware:
  - The large Dragon Bash-style celebration overlay is shown during normal gameplay.
  - While the FoodReminder Trading Post tab is open, the large overlay is suppressed and the compact in-tab target card remains visible.

### Fixed
- Account-wide History could make coverage markers appear unrelated to the character currently being played.
- Multiple watched items reaching their targets together could latch correctly but only the first item surfaced as an alert.
- The large gameplay target celebration and compact Trading Post-tab card could both display the same active alert simultaneously.

### Tested
- Newly archived sessions were tagged to the correct character and filtered correctly in Personal History.
- The History character selector displayed saved characters and preserved legacy history separately.
- Switching characters confirmed History defaults to the current character automatically.
- Multi-target Trading Post test with three simultaneous target hits verified sequential alert delivery:
  - Aurene's Bite
  - Bolt of Silk
  - Piece of Zhaitaffy
- Dismissing each target alert immediately promoted the next queued alert without waiting for another API refresh.
- Trading Post presentation test verified:
  - Trading Post tab closed -> large standalone celebration overlay shown.
  - Trading Post tab open -> large overlay hidden and compact in-tab card shown.
- Squad filter modes were revalidated in a live RTAPI-backed squad:
  - All players
  - Missing / Unknown / Unmapped
  - Unknown / Unmapped only
- Squad table remained stable while filters changed and live Food/Utility timers continued updating.

### Known Limitations
- History sessions recorded before character tagging remain under `Legacy / Unknown` because their original character cannot be reconstructed reliably.
- Exact Primer countdown recovery remains limited by the ArcDPS behavior documented in earlier checkpoints.

### Status
✅ Per-character Personal History working  
✅ Current-character History default working  
✅ Legacy History preserved  
✅ Multi-target Trading Post alert queue working  
✅ Sequential dismiss/advance behavior verified  
✅ Trading Post overlay/card presentation separation verified  
✅ Squad filter set revalidated in live gameplay  
✅ Current batch ready to commit and push  

### Next
- Commit and push the current v0.2.0 checkpoint.
- Produce a fresh Release build of `FoodReminder.dll`.
- Perform the final v0.2.0 smoke test.
- Package `FoodReminder.dll` with `FoodReminder_Consumables.tsv` for the GitHub pre-release.

---

## 2026-08-31 — v0.2.0 Release Prep, Session Accuracy & History UI

### Changed
- Addon version advanced to `0.2.0.0` for the v0.2.0 release checkpoint.
- Addon author changed from `Scott` to `spectre9510`.
- Personal History trend presentation was redesigned:
  - Food coverage now uses one colored session marker per completed session.
  - Utility coverage now uses one colored session marker per completed session.
  - Estimated Spend remains a compact sparkline because spend naturally varies over time.
  - Coverage markers were spaced farther apart for easier visual scanning.
- The visible addon footer now reports `Food Reminder v0.2.0 - Development Build`.

### Fixed
- Session usage totals could be inflated when ArcDPS resent or resynchronized an already-active Food or Utility effect.
- Food/Utility usage is now recorded only when:
  - no matching consumable was already active,
  - a different consumable replaces the active one, or
  - the same consumable's remaining duration jumps upward enough to indicate a real reapplication.
- A 5-second tolerance prevents minor timing differences from being mistaken for a new consumable use.
- Existing active Food/Utility can now resynchronize without being charged as another use.

### Tested
- Reset Session with existing Food and Utility active:
  - Session coverage resumed normally.
  - No new Food or Utility use was recorded from resynchronization alone.
- Consuming one fresh Food after reset recorded exactly `1 use`.
- Personal History coverage markers rendered correctly in-game.
- Estimated Spend sparkline remained readable and compact.
- Increased marker spacing rendered correctly and was accepted.
- Author `spectre9510` and visible `v0.2.0` footer confirmed in the current build.

### Known Limitations
- Historical sessions recorded before the usage-count fix may retain inflated legacy use totals in `FoodReminder_SessionHistory.tsv`.
- Reset Session starts a new session but does not erase previously archived Personal History.
- Historical cost still uses current Trading Post sell prices rather than historical purchase prices.

### Status
✅ Session resync duplicate-use bug fixed  
✅ Fresh Food use recorded exactly once  
✅ Existing active buffs no longer count as fresh uses during resync  
✅ Personal History coverage-dot layout accepted  
✅ Estimated Spend sparkline retained  
✅ Author changed to `spectre9510`  
✅ Version advanced to v0.2.0  
✅ Ready for release-prep commit/push  

### Next
- Commit and push the v0.2.0 release-prep changes.
- Produce a fresh Release build of `FoodReminder.dll`.
- Package `FoodReminder.dll` with `FoodReminder_Consumables.tsv`.
- Publish the GitHub v0.2.0 pre-release after the final smoke test.

---

## 2026-08-31 — Reminder Customization, Squad Filters & RTAPI Roster Integration

### Added
- Independent reminder enable/disable controls for:
  - Food expiration reminders
  - Utility expiration reminders
  - Missing Food/Utility combat warnings
  - Primer expiration reminders
- Configurable reminder popup display duration from 3 to 10 seconds.
- Squad player filter with three modes:
  - All players
  - Missing / Unknown / Unmapped
  - Unknown / Unmapped only
- Optional RTAPI integration through the public `RTAPI.h` interface.
- New `RTAPIIntegration.h/.cpp` wrapper for RTAPI DataLink and group-member events.
- Squad tab roster-source status display:
  - `Roster: RTAPI | Consumables: ArcDPS`
  - ArcDPS fallback status while RTAPI is unavailable or still syncing.

### Changed
- Squad roster handling now uses a hybrid data model when RTAPI is available:
  - RTAPI supplies current squad membership, subgroup, profession/specialization, self identity, and same-instance state.
  - ArcDPS remains authoritative for Food/Utility effect IDs, active state, and remaining duration.
- FoodReminder switches to the RTAPI roster only after the event-backed RTAPI roster matches RTAPI's reported group-member count.
- If RTAPI is missing, hot-unloaded, or still syncing, the Squad tracker preserves the previous ArcDPS-only roster behavior.
- RTAPI squad members outside the player's current map instance are omitted from the consumable table because reliable ArcDPS consumable state cannot be paired with them.
- The Squad attention filter was expanded from a single checkbox into selectable filter modes so missing consumables can be separated from unresolved/unknown data.

### Fixed
- The original `Show only players needing attention` filter was too broad in large squads because confirmed `None` Utility states caused most players to remain visible.
- `Unknown / Unmapped only` now provides a focused unresolved-data view without including every confirmed missing Food/Utility state.
- RTAPI project integration initially failed because `RTAPI.h` was not physically available on the compiler include path; placing the header in the project source directory resolved the build error.
- Superior Sharpening Stone cost tracking used item ID `78305`, a non-tradable duplicate that shares effect ID `9963`, so Trading Post pricing remained unavailable even though the consumable was recognized correctly.
- Superior Sharpening Stone now uses tradable item ID `9443` as the representative pricing item for effect `9963`.

### Tested
- Reminder-type checkboxes verified in-game:
  - Enabled reminders display.
  - Disabled reminders remain suppressed.
- Configurable reminder display duration tested successfully in-game.
- Three-mode Squad player filter rendered and switched correctly.
- Large-squad filter test:
  - `Missing / Unknown / Unmapped` correctly reduced the displayed roster based on attention state.
  - `Unknown / Unmapped only` reduced the active RTAPI-backed squad view to a single unresolved player during testing.
- RTAPI integration build passed.
- RTAPI loaded successfully through Nexus.
- Squad tab displayed `Roster: RTAPI | Consumables: ArcDPS`.
- Existing ArcDPS Food/Utility tracking continued to populate correctly with RTAPI roster authority enabled.
- RTAPI-backed subgroup/roster display remained stable during live squad gameplay.
- ArcDPS fallback path remains available when RTAPI is unavailable or not yet synchronized.
- Superior Sharpening Stone pricing retested in-game after switching the representative item ID to `9443`.
- Personal History changed from `Unpriced` to a live estimated spend for Superior Sharpening Stone, confirming the Session/History pricing pipeline was already working correctly.

### Known Limitations
- RTAPI does not provide Food/Utility effect IDs or remaining consumable duration; ArcDPS is still required for consumable-state tracking.
- RTAPI integration is optional at runtime. FoodReminder continues using ArcDPS-only squad tracking when RTAPI is not available.
- The RTAPI roster becomes authoritative only after its event-backed member list matches RTAPI's current reported group-member count.
- Squad members outside the current map instance are not shown in the RTAPI-backed consumable table.
- ArcDPS consumable state can still remain unknown until relevant buff/combat events are observed.
- Exact Primer countdown recovery remains subject to the existing ArcDPS limitations documented below.

### Status
✅ Reminder-type customization working  
✅ Reminder display-duration customization working  
✅ Expanded three-mode Squad filter working  
✅ RTAPI integration compiled successfully  
✅ RTAPI roster active in-game  
✅ ArcDPS consumable tracking preserved  
✅ RTAPI/ArcDPS hybrid Squad model working  
✅ Unknown/Unmapped-only filter validated in a live squad  
✅ Superior Sharpening Stone Trading Post pricing fixed and validated  
✅ Current batch ready to commit and push  

### Next
- Continue normal gameplay testing with RTAPI enabled.
- Verify RTAPI join/leave/subgroup updates across additional squad and map-change scenarios.
- Confirm ArcDPS fallback behavior during a future RTAPI unload/disable test.
- Continue resolving genuinely unknown consumable effect IDs.
- Continue stability testing before the next release checkpoint.

---

## 2026-08-30 — Persistent Personal History, Tracker Naming & Cleanup

### Added
- Persistent personal consumable session history stored locally in `FoodReminder_SessionHistory.tsv`.
- Session history survives full game/addon restarts and appends new completed sessions without overwriting older records.
- New `History` tab with selectable ranges:
  - 1 Day
  - 7 Days
  - 30 Days
  - All Time
- History summary for completed sessions, tracked time, combat time, Food/Utility usage, coverage, and estimated cost.
- Direct-draw History trend charts for:
  - Food coverage
  - Utility coverage
  - Estimated consumable spend
- Per-item Food and Utility usage history with use counts and estimated spend.
- Collapsible History detail sections for:
  - Coverage & Waste Details
  - Trends
  - Per-Item Usage
  - Primer Details
- Trends now opens by default while the heavier detail sections remain collapsed.

### Changed
- History tab layout was slimmed down so the default view focuses on the compact Summary and Trends rather than showing all details at once.
- History cost wording now separates priced consumables from unpriced uses instead of combining them into one ambiguous value.
- Session Report `UTILITY` heading now uses blue styling to distinguish it visually from Food.
- Trading Post selectable trend windows were extended to:
  - 15m
  - 30m
  - 1h
  - 6h
  - 24h
  - 3d
  - 7d
  - 30d
  - 90d
- Compact tracker now displays actual consumable names rather than database category/short labels.
- Long compact-tracker consumable names are truncated with `...` to preserve the compact tracker width; the full name remains available in the existing hover tooltip.
- Candy Cane / Minty Breath dedicated tracking support was removed.

### Fixed
- Compact tracker could display generic database labels such as `Food`, `Power`, or other short stat labels instead of the actual consumable name.
- Grumble Cake and other consumables using generic labels can now display their real item names in the compact tracker.
- Full consumable names initially caused the compact tracker to grow excessively wide; names are now safely shortened in the compact view.
- Removed stale Candy Cane-specific tracker code, state, display logic, and database mapping.
- Effect ID `34210` remains explicitly excluded from normal Food detection so Minty Breath cannot corrupt the regular Food timer.

### Tested
- Persistent session-history save tested through `Reset Session`.
- Session-history archive on game/addon unload tested.
- Full restart verified that existing history loads and new sessions append correctly.
- History tab summary, charts, per-item usage, Primer details, and collapsible layout displayed correctly in-game.
- Direct-draw History charts remained visible where built-in ImGui plotting was not visually useful in Nexus.
- Extended Trading Post trend selector displayed the longer windows correctly in-game.
- Compact tracker actual-name display tested in-game.
- Compact tracker long-name truncation tested in-game and restored the intended compact width.
- Candy Cane/Minty Breath code cleanup build passed.
- External consumable database entry for effect `34210` was removed.

### Known Limitations
- Historical consumable cost uses current Trading Post sell prices, not the price actually paid at the time of each completed session.
- Unpriced consumables remain counted but are excluded from priced cost totals.
- Session history begins only after the persistent history feature was introduced.
- History trend charts represent completed saved sessions rather than continuous time-series sampling.
- Effect ID `34210` is intentionally ignored by normal Food tracking as a protective exclusion.

### Status
✅ Persistent personal session history working  
✅ Restart/load/append persistence passed  
✅ History tab working  
✅ History layout slimmed and accepted  
✅ Direct-draw History charts working  
✅ Per-item historical usage working  
✅ Extended 3d / 7d / 30d / 90d Trading Post trend choices working  
✅ Compact tracker actual consumable names working  
✅ Long-name truncation working  
✅ Candy Cane/Minty Breath dedicated support removed  
✅ Latest build and in-game tracker test passed  

### Next
- Continue normal gameplay so personal History and longer Trading Post windows accumulate real data naturally.
- Continue stability testing.
- Move to the next outstanding feature or bug after this checkpoint.

---

## 2026-08-29 — External Consumable Database & Unknown Collector Cleanup

### Added
- External `FoodReminder_Consumables.tsv` consumable database.
- Automatic database loading from the same folder as the FoodReminder-Nexus DLL.
- Automatic TSV reload after the database file is saved, allowing consumable mappings to be added or corrected without rebuilding the DLL.
- Additional Food and Utility mappings from live testing and public-data reconciliation.
- Fried Banana Chips (`62677`) and other resolved effects from the latest consumable test batch.

### Changed
- Consumable lookup now supports the external TSV database while retaining the compiled database as a fallback.
- Existing hand-authored consumable mappings remain authoritative so verified item IDs and detailed metadata are preserved.
- Shared effect IDs use shared/combined descriptions rather than pretending a single effect uniquely identifies one consumable.
- Unknown Consumables now acts as a current unresolved list instead of retaining every historically unknown effect.

### Fixed
- Previously captured unknown IDs could remain in the Unknown Consumables list after their effects were added to the consumable database.
- `SquadTracker::GetUnknownConsumables()` now checks stored unknowns against the current consumable database and removes entries that have become recognized.

### Tested
- External TSV loading verified in-game by changing the display name for effect `58110` and observing the change immediately in FoodReminder-Nexus.
- DLL rebuild was not required for the TSV data change.
- Updated `SquadTracker.cpp` build passed.
- Latest external database and resolved-unknown pruning changes installed successfully.

### Known Limitations
- External TSV entries do not override existing hand-authored switch mappings; verified built-in mappings remain authoritative.
- Effect IDs `57334` and `57406` remain intentionally unresolved pending reliable identification.
- Some Guild Wars 2 consumables share the same effect ID and therefore cannot be uniquely identified from the ArcDPS effect alone.

### Status
✅ External consumable database working  
✅ Automatic TSV reload verified in-game  
✅ Consumable data can now be expanded without rebuilding the DLL  
✅ Resolved unknown-ID pruning implemented  
✅ Latest build passed  
✅ Changes committed/pushed checkpoint ready  

### Next
- Play normally and collect genuinely new unknown consumable effects.
- Resolve new unknowns from reliable public data before resorting to manual consumable testing.
- Continue expanding `FoodReminder_Consumables.tsv`.
- Manually isolate unresolved effects only when public data cannot identify them.

---

## 2026-08-28 — Consumable State Reliability, Primer Limitations & Tracker Cleanup

### Added
- Explicit `Unknown`, `Active`, and `Missing` consumable detection states.
- Persistent per-character Food/Utility detection-state flags so a zero timer no longer automatically means a confirmed missing buff.
- Character-specific Primer persistence fields.
- Separate Primer detection state for Metabolic Primer and Utility Primer.
- Primer presence-only inference from clearly Primer-extended Food or Utility duration.
- `Active*` tracker state for inferred Primer presence.
- Primer limitation tooltips explaining when ArcDPS data is unavailable.
- Primer-safe Session Report tracking:
  - Confirmed Primer time.
  - Inferred Primer time.
  - Unknown Primer-state time.
- Estimated Primer savings labels that explicitly distinguish calculated estimates from directly observed values.
- Standalone gameplay-visible Trading Post target-hit overlay.
- Compact tracker width cleanup.
- Gold tracker border for improved visual recognition.

### Changed
- Food and Utility timers continue to use paused per-character duration snapshots while the character/game is unloaded.
- Primer state is now character-specific instead of global.
- Primer countdown inference from extended Food/Utility duration was removed.
- Extended Food/Utility duration may now infer Primer **presence only**, never the Primer's remaining countdown.
- Unknown Food/Utility state no longer triggers false missing-buff warnings when entering combat.
- SessionTracker no longer receives only simple Primer booleans.
- Session Primer tracking now distinguishes:
  - `ConfirmedActive`
  - `InferredActive`
  - `Unknown`
  - `Inactive`
- Unknown Primer time is excluded from Primer savings estimates.
- Existing aggregate Primer-protected time remains available for savings calculations but now consists only of confirmed and inferred protection.
- Compact tracker minimum width reduced from 260 px to 220 px.
- Food/Utility label columns moved left to reduce unused horizontal space.
- Tracker border changed from experimental red to gold after in-game visual testing.
- Experimental decorative dragon-corner drawing was tested and reverted because it did not read clearly at the compact tracker size.

### Fixed
- Food/Utility were briefly changed to absolute expiration timestamps based on the incorrect assumption that GW2 consumable timers continue while offline.
- In-game testing proved Food/Utility timers pause while the character/game is unloaded, so the absolute-expiration change was fully reverted.
- Brand-new characters could inherit Primer state from another character because Primer persistence was global.
- Primer state no longer leaks between characters.
- Food/Utility `Unknown` state could become `Missing` after persistence because zero remaining duration did not retain whether the state was actually known.
- Detection-state persistence now preserves `Unknown` correctly across reloads.
- Primer timers could be inflated by incorrectly treating long Food/Utility duration as the Primer's own remaining duration.
- Food/Utility-to-Primer timer inference was removed.
- Session statistics no longer silently treat uncertain Primer state as directly confirmed Primer protection.
- Standalone Trading Post target overlay compile issue caused by the Windows `max` macro was fixed.

### Tested
- Food/Utility offline behavior tested by closing the game for several minutes:
  - GW2 consumable timers remained effectively paused.
  - Absolute-expiration implementation was rejected.
- Brand-new character test:
  - Food displayed `Unknown`.
  - Utility displayed `Unknown`.
  - No false missing-buff warning triggered after entering combat.
- Character-specific Primer test:
  - Brand-new character no longer inherited another character's Primer timers.
- Existing-character Food/Utility resynchronization:
  - Restored timers could initially be stale.
  - Entering combat resynchronized Food/Utility to within a few seconds of the in-game buff bar.
- Active Primer recovery test:
  - Logging in with existing Primers did not restore a trustworthy active Primer timer from ArcDPS.
  - Entering combat did not activate Primer state.
  - Recent Buff Events continued to show unrelated events such as Rescue Protocol rather than usable Primer events.
- Direct Primer application test:
  - No usable Primer event appeared in Recent Buff Events.
  - Exact Primer countdown recovery through the current ArcDPS event stream was therefore considered unreliable.
- Primer presence-only inference:
  - Extended Food/Utility duration correctly displayed `Metabolic: Active*` and `Utility P: Active*`.
  - No fabricated Primer countdown was shown.
- Primer-safe Session Report:
  - `Primer Confirmed` remained at zero when direct state was unavailable.
  - `Primer Inferred*` accumulated during clearly Primer-extended consumable protection.
  - `Primer Unknown` remained separate.
  - Estimated savings fields displayed correctly.
- Compact tracker:
  - Reduced-width layout displayed correctly in-game.
  - Red border was tested and rejected as too similar to an error state.
  - Gold border was tested and accepted as easier to locate without appearing as an error.
  - Decorative dragon-style corner drawing was tested and rejected; tracker reverted to the clean gold-border baseline.
- Standalone Trading Post target-hit overlay displayed correctly during normal gameplay with the Trading Post tab closed.

### Known Limitations
- ArcDPS does not reliably resend already-active Primer state after login or character switching through the combat-event stream currently used by FoodReminder-Nexus.
- Current testing did not expose reliable Primer application events in Recent Buff Events.
- Exact Primer countdowns therefore cannot always be reconstructed after login or character switching.
- Primer presence may be inferred from clearly extended Food/Utility duration, but the Primer countdown itself is never inferred.
- Food/Utility restored snapshots may briefly be stale until a fresh combat event resynchronizes them.
- Initial-state synchronization remains dependent on what ArcDPS exposes.

### Status
✅ Build successful  
✅ In-game Food/Utility pause behavior verified  
✅ Unknown-state persistence passed  
✅ False missing-warning suppression passed  
✅ Character-specific Primer isolation passed  
✅ Inaccurate Primer countdown inference removed  
✅ Primer presence-only inference working  
✅ Primer limitation UI working  
✅ Primer-safe Session Report working  
✅ Compact gold tracker styling accepted  
✅ Standalone Trading Post target overlay working  
✅ Latest Primer/session reliability batch ready to commit and push  

### Next
- Continue expanding the consumable database.
- Refine consumable cost-per-hour and Primer savings analysis.
- Continue long-term Trading Post history collection.
- Improve Squad and Session presentation where useful.
- Continue gameplay and stability testing.
- Revisit initial-state synchronization only if a more reliable ArcDPS/Nexus data source becomes available.

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
- Standalone target-hit notification outside the Trading Post tab has now been implemented and validated in-game.
- Custom image-based Dragon Bash assets are not currently used; celebration effects are generated through ImGui drawing.

### Next
- Continue validating the standalone Trading Post target-hit overlay during normal gameplay.
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
✅ Standalone Trading Post target-hit overlay completed and tested

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