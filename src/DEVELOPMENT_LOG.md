---

## 2026-08-22 to 2026-08-23 — Squad Validation, Unknown Consumable Collector, Persistence, and Database Expansion

### Squad State Semantics Validation

Continued live testing of the Squad Food/Utility state model introduced during the previous development checkpoint.

The Squad tracker distinguishes:

- `?` — ArcDPS has not yet established the player's Food/Utility state.
- `None` — ArcDPS has established that no Food/Utility consumable is active.
- Known label — a recognized active consumable.
- `Unknown (ID)` — an active consumable whose ArcDPS effect ID is not yet mapped.

Live group and open-world testing confirmed that the state model behaves correctly for ArcDPS-tracked players.

The Squad tracker may initially contain only a subset of a larger squad because it represents players currently known to ArcDPS rather than every squad member automatically.

Food and Utility state often becomes more complete after combat begins and ArcDPS provides BuffInitial information.

### Consumable Mapping Investigation

Continued identifying Food and Utility effect IDs through live comparison testing.

Testing used:

- FoodReminder-Nexus Squad tracking.
- ArcDPS effect IDs.
- Comparison against the older Food Tracker where useful.
- Manual inspection of player consumables when necessary.

The older tracker remains useful for identifying some consumable names but is not considered authoritative for timer state because previous testing demonstrated stale-duration behavior.

### Unknown Consumable Collector

Added an automatic Unknown Consumable Collector to the Squad development interface.

Previously, unmapped consumables could be displayed as:

`Unknown (ID)`

but identifying them required manually watching the Squad window and recording effect IDs.

The new collector captures unmapped consumable effects automatically as ArcDPS events are processed.

The Squad window does not need to remain open for collection to occur.

Each collected entry stores:

- Consumable type: Food or Utility.
- ArcDPS effect ID.
- Seen count.

### Background Collection Validation

Background collection was tested during live group and world-boss gameplay.

A key test was performed with the Squad interface closed.

Effect ID:

`9992`

was observed as an unknown Food effect.

After reopening the Squad interface, the Unknown Consumable Collector contained:

`Food,9992`

with an increased Seen count.

This confirmed that collection occurs during event processing rather than only while the Squad interface is being rendered.

### Unknown Deduplication

The collector stores one entry per unique Food/Utility effect.

Repeated observations do not create duplicate rows.

Instead, the entry's Seen counter increases.

Example observed during testing:

`Food,9992,Seen:31`

The Seen value represents the number of observations and should not be interpreted as a unique-player count because ArcDPS may report the same effect multiple times.

### Unknown Export

Added:

`Copy All Unknown IDs`

This produces a compact list suitable for development and database-mapping work.

Example format:

`Food,9992,Seen:31`

`Utility,9942,Seen:9`

This significantly reduces the need to manually record effect IDs while monitoring live groups.

### Unknown List Clearing

Added:

`Clear Unknown List`

This allows the collected development list to be reset when desired.

The list is not automatically cleared during normal tracking.

### Persistent Unknown Consumables

Initial unknown collection existed only in memory.

This meant restarting Guild Wars 2 or reloading the addon would lose the collected IDs.

Persistence was added through the existing:

`FoodReminder.ini`

settings system.

A saved unknown consumable stores:

- Effect ID.
- Food/Utility classification.
- Seen count.

The live SquadTracker collector can now:

- Restore unknown consumables from saved settings.
- Copy the current collector state into saved settings.
- Preserve Seen counts.
- Preserve Food/Utility classification.

### Persistence Validation

Unknown-consumable persistence was successfully tested across a full restart.

After restarting Guild Wars 2, the addon was inspected while still at character select.

ArcDPS tracked-player count was:

`0`

Despite no players being tracked in the new session, the Unknown Consumables section restored previously collected entries including:

- Utility `34187`
- Utility `38605`

with their saved Seen counts.

Because no ArcDPS players were currently tracked, these entries could not have been newly rediscovered.

This confirmed that unknown-consumable persistence and restoration through `FoodReminder.ini` were working.

### Collector Workflow Improvement

The new development workflow is now:

1. Play normal group/open-world content.
2. Allow FoodReminder-Nexus to collect unknown effects automatically.
3. Review the Unknown Consumables list later.
4. Export the list using Copy All Unknown IDs.
5. Identify effects through reliable comparison or manual inspection.
6. Add confirmed mappings to the internal consumable database.
7. Leave genuinely unidentified effects as Unknown rather than guessing.

This removes the need to continuously watch the Squad interface or manually chase effect IDs as they appear.

### Consumable Database Expansion

The internal consumable definition database was expanded substantially during live testing.

Newly identified Food effects during this development period include:

- Mushroom Pizza
- Soy-Sesame Sous-Vide Steak
- Tray of Decade Desserts
- Plate of Coq Au Vin with Salsa
- Clove and Veggie Flatbread
- Spherified Cilantro Oyster Soup
- Peppercorn and Veggie Flatbread

Newly identified Utility effects include:

- Powerful Potion of Demon Slaying
- Writ of Masterful Strength
- Writ of Masterful Malice
- Peppermint Oil
- Magnanimous Maintenance Oil

### Confirmed Effect IDs

Mappings identified during testing include:

#### Food

- `10006` — Mushroom Pizza
- `57241` — Soy-Sesame Sous-Vide Steak
- `57253` — Plate of Coq Au Vin with Salsa
- `57344` — Clove and Veggie Flatbread
- `57356` — Spherified Cilantro Oyster Soup
- `57382` — Peppercorn and Veggie Flatbread
- `68232` — Tray of Decade Desserts

#### Utility

- `9901` — Powerful Potion of Demon Slaying
- `33297` — Writ of Masterful Strength
- `33836` — Writ of Masterful Malice
- `34187` — Peppermint Oil
- `38605` — Magnanimous Maintenance Oil

Several of these mappings were directly confirmed during live comparison against visible consumables.

### Effect-ID Correction

An earlier development mapping associated:

`57421`

with Soy-Sesame Sous-Vide Steak.

Further investigation identified the correct effect ID as:

`57241`

The mapping was corrected in the consumable database.

This reinforces the development rule that uncertain effect IDs should remain Unknown until they can be reliably identified.

### Remaining Unknown Utilities

At the end of the mapping session, several Utility IDs remained unresolved, including:

- `9958`
- `9962`
- `46925`

These remain intentionally unmapped.

FoodReminder-Nexus will continue displaying and collecting these effects as Unknown until a reliable identification is available.

No guessed consumable names will be added.

### Build Validation

Multiple mapping and collector changes were built successfully during development.

Successful rebuild checkpoints included:

- Unknown Consumable Collector implementation.
- Collector persistence support.
- Consumable database corrections.
- Food database expansion.
- Utility database expansion.

No compile errors remained at the final checkpoint.

### Git Checkpoints

The Unknown Consumable Collector and persistence work were committed and pushed as a dedicated stable checkpoint.

The subsequent consumable database expansion was also committed and pushed separately.

Keeping these changes in separate Git checkpoints preserves a clean distinction between:

- Collector/persistence functionality.
- Consumable data expansion.

### Current Working Checkpoint

At the end of this development period:

- Squad player discovery is working.
- Squad Food tracking is working.
- Squad Utility tracking is working.
- Squad countdown timers are working.
- BuffInitial handling is working.
- Food/Utility state semantics have been live tested.
- Known consumables display classifications.
- Unmapped consumables display their effect IDs.
- Unknown consumables are captured automatically.
- The Squad window does not need to remain open for collection.
- Unknown IDs are deduplicated.
- Seen counts accumulate repeated observations.
- Unknown IDs can be exported as a batch.
- Unknown entries can be cleared manually.
- Unknown consumables persist across restart.
- Saved Food/Utility classification is restored.
- Saved Seen counts are restored.
- The consumable database has been substantially expanded.
- Uncertain IDs remain Unknown instead of being guessed.
- Recent builds complete successfully.
- Collector/persistence work has been committed and pushed.
- Recent consumable mapping work has been committed and pushed.

### Next Development Steps

1. Automatically remove entries from the persistent Unknown Consumables list once their effect IDs become recognized by the consumable database.
2. Continue expanding Food and Utility mappings naturally during normal gameplay.
3. Investigate remaining unknown Utility IDs when reliable identification becomes available.
4. Continue Squad interface cleanup.
5. Improve initial Food/Utility synchronization after login and map changes.
6. Improve character-switch synchronization before the first ArcDPS self event.
7. Continue live testing of newly added consumable mappings.
8. Continue expiration and reminder edge-case testing.
9. Continue stability testing during normal open-world and group gameplay.
10. Reduce development-only interface elements as the underlying tracking systems stabilize.