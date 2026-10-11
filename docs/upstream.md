# Upstream baseline

Upstream: <https://github.com/rh-hideout/pokeemerald-expansion>

## Initial pinned revision

- Commit: `65fbf322d1bf39791676c6699da3d2cd44130ef0`
- Branch: `master`
- Inventory date: 2026-10-11

The revision contains the battle implementation and battle test suite used for
the first extraction pass. It is a source baseline, not yet a library import.

## Initial dependency inventory

The battle implementation is distributed across roughly 170,000 lines in
`src/battle_*.c`. The first headless slice should center on these rule paths:

- `src/battle_main.c`: battle state-machine entry and turn progression
- `src/battle_script_commands.c`: scripted rule/effect execution
- `src/battle_util.c` and `src/battle_util2.c`: shared rule helpers
- `src/battle_move_resolution.c`: move selection and resolution
- `src/battle_set_effect.c`, `src/battle_stat_change.c`, and
  `src/battle_end_turn.c`: state-changing effects
- `src/battle_hold_effects.c`: item effects
- `src/battle_ai_*.c`: optional AI consumer, not required by the core ABI

The following are initially host adapters or optional integrations and should
not enter the core library unchanged:

- `battle_anim*.c`, `battle_bg.c`, `battle_gfx_sfx_util.c`, and
  `battle_transition*.c`
- `battle_controller*.c`, `battle_interface.c`, and `battle_message.c`
- frontier/facility UI modules such as `battle_dome.c`, `battle_factory*.c`,
  `battle_tower.c`, and `battle_pyramid*.c`

The upstream tests already provide a useful parity target. The first imported
scenarios should come from `test/battle/damage_formula.c`, `test/battle/move.c`,
`test/battle/end_turn_effects.c`, and `test/battle/switch_in_abilities.c`.

The first local data import is intentionally limited to Bulbasaur, Charmander,
Pound, and Tackle. It is represented in `src/data/data.c` and sourced from
`src/data/pokemon/species_info/gen_3_families.h` and `src/data/moves_info.h`.

Every imported batch must include:

- upstream commit and import date;
- source paths and local destination paths;
- any behavior changes required by the library boundary;
- the upstream license/attribution notice;
- tests run and known parity gaps.
