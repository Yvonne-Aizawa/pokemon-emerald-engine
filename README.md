# Pokémon Battle Engine

A renderer- and platform-independent battle engine extracted from the
`pokeemerald-expansion` battle systems.

The intended consumer is any game that can provide battle commands and render
the events emitted by the engine. The engine owns battle rules, battle state,
Pokémon species/move/item data, effects, status conditions, damage, turn order,
and deterministic random-number consumption. A host game owns graphics,
animation, audio, input, menus, persistence, networking, and its main loop.

## Status

This repository is an architectural starting point. The upstream battle code
has not yet been imported or made library-safe. The extraction will be done in
small, behavior-preserving steps with upstream tests and differential tests as
the acceptance criteria.

## Source baseline

The source baseline is [`rh-hideout/pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion).
Record the exact upstream commit used for each import in `docs/upstream.md`.
Keep upstream attribution and licensing notices with every imported or adapted
file.

## Boundary

```text
host game -> battle commands -> battle-core -> battle events -> host game
                         ^          |
                         |          +-- battle-data
                         +-- host callbacks (optional, narrow, deterministic)
```

The public API must not expose GBA globals, SDL, graphics types, save blocks,
task/sprite systems, audio sequencing, or host-specific allocators.

## Initial layout

- `include/poke_battle/`: stable public C ABI and versioned data types
- `src/core/`: battle state machine and rule execution
- `src/data/`: species, moves, abilities, items, effects, and generated tables
- `src/ffi/`: C ABI implementation and serialization helpers
- `tests/`: deterministic scenarios and compatibility tests
- `docs/`: extraction decisions, ownership rules, and upstream mapping

## Design constraints

1. A battle advances only through an explicit `pb_submit_action` / `pb_step`
   interface.
2. Randomness comes from an injected deterministic RNG; no global RNG calls.
3. The core emits data-only events. It never draws, plays audio, sleeps, or
   reads input.
4. Host callbacks are queries or explicit effect sinks, not hidden control
   flow.
5. Serialization is versioned so a host can save and restore battle state.
6. The first milestone is a single battle format and ruleset matching the
   imported upstream behavior; extra generations and custom rules are later
   configuration, not accidental coupling.

See [`docs/architecture.md`](docs/architecture.md) for the proposed API and
extraction sequence.
