# Battle engine architecture

## Public model

The engine is a stateful, deterministic machine:

```c
pb_battle *battle = pb_battle_create(&config);
pb_step_result result = pb_step(battle, commands, command_count);
while (result.kind == PB_STEP_NEEDS_ACTION) {
    /* The host collects input and fills commands. */
    result = pb_step(battle, commands, command_count);
}
/* The host renders result.events; it does not drive battle internals. */
```

The eventual ABI should use fixed-width integers, opaque handles, explicit
buffer lengths, and a struct-size/version field. No public struct should
contain pointers to upstream globals or compiler-dependent bitfields.

## Ownership split

### Core owns

- participants, parties, active slots, volatile and persistent battle state
- action validation, priority, speed/order resolution, and turn progression
- move effects, abilities, held items, statuses, weather, terrain, and hazards
- stat calculation, accuracy, critical hits, damage, experience, and fainting
- battle RNG and replayable random decisions
- data lookup through an injected/read-only ruleset

### Host owns

- input, menus, camera, sprites, models, animation, particles, and text layout
- sound/music and timing
- save files and the broader game profile
- networking transport and matchmaking
- memory policy, threads, and the application loop

### Narrow host services

Only deterministic services belong in the callback table: resolving a configured
trainer/party, checking a custom rule, and receiving an event stream. Visual or
blocking callbacks are deliberately excluded.

## Event stream

Events should describe facts, not presentation instructions. Examples are
`PB_EVENT_ACTION_ACCEPTED`, `PB_EVENT_MOVE_USED`, `PB_EVENT_DAMAGE`,
`PB_EVENT_STATUS_CHANGED`, `PB_EVENT_FAINTED`, `PB_EVENT_SWITCH_REQUIRED`, and
`PB_EVENT_BATTLE_FINISHED`. Each event carries stable IDs and values sufficient
for a host to choose any visual treatment.

## Extraction sequence

1. Freeze and record the upstream commit; inventory battle entry points,
   globals, data tables, and tests.
2. Add a headless build target while behavior still lives in upstream-shaped C.
3. Replace battle-to-UI calls with an event sink and replace input globals with
   action structs.
4. Move battle state out of global/static storage into an opaque context.
5. Move species/move/item/ability data behind a read-only ruleset interface.
6. Inject RNG and allocation; remove platform and GBA-only assumptions.
7. Build a shared library plus a small command-line replay runner.
8. Prove parity with deterministic scenarios before changing internal types.

Do not rewrite the rule engine in another language during the extraction. A
mechanical, testable first separation makes later rewrites optional and keeps
behavior differences attributable.
