#include "poke_battle/poke_battle.h"
#include "data/data.h"

#include <stdlib.h>

enum { PB_MAX_EVENTS = 5, PB_MAX_HP = 100 };

struct pb_battle {
    uint64_t rng;
    pb_battler battlers[2];
    uint32_t turn;
    pb_event events[PB_MAX_EVENTS];
    uint32_t event_count;
};

static uint32_t next_random(pb_battle *battle)
{
    /* xorshift64*: deterministic, local to the battle, and reproducible. */
    uint64_t value = battle->rng;
    if (value == 0)
        value = UINT64_C(0x9e3779b97f4a7c15);
    value ^= value >> 12;
    value ^= value << 25;
    value ^= value >> 27;
    battle->rng = value;
    return (uint32_t)((value * UINT64_C(0x2545f4914f6cdd1d)) >> 32);
}

static void emit(pb_battle *battle, uint32_t kind, uint32_t actor,
                 uint32_t target, int32_t value, uint32_t data_id)
{
    if (battle->event_count == PB_MAX_EVENTS)
        return;
    battle->events[battle->event_count++] = (pb_event){
        .kind = kind, .actor = actor, .target = target,
        .value = value, .data_id = data_id,
    };
}

/* Gen 3 type IDs. The compact table covers the type interactions used by the
   imported move/species slice and defaults to neutral. */
static uint32_t type_effectiveness(uint16_t move_type, uint16_t target_type)
{
    if (move_type == 0 && target_type == 13) /* Normal -> Ghost. */
        return 0;
    if (move_type == 1 && target_type == 4) /* Fire -> Grass. */
        return 20;
    if (move_type == 1 && target_type == 1) /* Fire -> Fire. */
        return 5;
    if (move_type == 4 && target_type == 1) /* Grass -> Fire. */
        return 5;
    return 10;
}

static uint32_t type_multiplier(uint16_t move_type, const pb_battler *target)
{
    uint32_t multiplier = 10;
    for (unsigned i = 0; i < 2; ++i)
        multiplier = multiplier * type_effectiveness(move_type, target->types[i]) / 10;
    return multiplier;
}

static int has_move(const pb_battler *battler, uint32_t move_id)
{
    for (unsigned i = 0; i < PB_MOVE_SLOTS; ++i)
        if (battler->moves[i].move_id == move_id && battler->moves[i].pp != 0)
            return 1;
    return 0;
}

pb_battle *pb_battle_create(const pb_battle_config *config)
{
    if (config == NULL || config->struct_size < sizeof(*config) ||
        config->abi_version != PB_ABI_VERSION)
        return NULL;

    pb_battle *battle = calloc(1, sizeof(*battle));
    if (battle == NULL)
        return NULL;
    battle->rng = config->seed;
    battle->battlers[0] = config->battlers[0];
    battle->battlers[1] = config->battlers[1];
    for (unsigned i = 0; i < 2; ++i) {
        if (battle->battlers[i].species == 0)
            battle->battlers[i].species = config->species[i] ? config->species[i] : (i ? 4 : 1);
        const struct pb_species_data *species = pb_species(battle->battlers[i].species);
        if (battle->battlers[i].level == 0)
            battle->battlers[i].level = 50;
        if (battle->battlers[i].max_hp == 0)
            battle->battlers[i].max_hp = species->base_hp;
        if (battle->battlers[i].current_hp == 0)
            battle->battlers[i].current_hp = battle->battlers[i].max_hp;
        if (battle->battlers[i].stats[PB_STAT_ATTACK] == 0)
            battle->battlers[i].stats[PB_STAT_ATTACK] = species->base_attack;
        if (battle->battlers[i].stats[PB_STAT_DEFENSE] == 0)
            battle->battlers[i].stats[PB_STAT_DEFENSE] = species->base_defense;
    }
    return battle;
}

void pb_battle_destroy(pb_battle *battle)
{
    free(battle);
}

pb_step_result pb_step(pb_battle *battle, const pb_action *actions,
                       uint32_t action_count)
{
    static const pb_step_result invalid = { sizeof(pb_step_result),
                                             PB_STEP_FINISHED, 0, NULL };
    if (battle == NULL || actions == NULL || action_count == 0 ||
        battle->battlers[0].current_hp == 0 || battle->battlers[1].current_hp == 0)
        return invalid;

    battle->event_count = 0;
    const pb_action action = actions[0];
    if (action.kind != PB_ACTION_MOVE || action.actor > 1 || action.target > 1 ||
        action.actor == action.target)
        return (pb_step_result){ sizeof(pb_step_result), PB_STEP_NEEDS_ACTION,
                                 0, battle->events };

    const pb_battler *attacker_state = &battle->battlers[action.actor];
    const pb_battler *defender_state = &battle->battlers[action.target];
    const struct pb_move_data *move_data = pb_move(action.move_id);
    if (move_data == NULL || !has_move(attacker_state, action.move_id))
        return (pb_step_result){ sizeof(pb_step_result), PB_STEP_NEEDS_ACTION,
                                 0, battle->events };
    emit(battle, PB_EVENT_MOVE_USED, action.actor, action.target, 0,
         action.move_id);
    if (next_random(battle) % 100u >= move_data->accuracy) {
        emit(battle, PB_EVENT_MISSED, action.actor, action.target, 0,
             action.move_id);
        battle->turn++;
        return (pb_step_result){ sizeof(pb_step_result), PB_STEP_EVENTS,
                                 battle->event_count, battle->events };
    }
    const int critical = next_random(battle) % 16u == 0;
    const uint32_t level = attacker_state->level;
    /* Integer-only physical damage slice derived from the upstream model. */
    uint32_t damage = ((2u * level / 5u + 2u) * move_data->power *
                       attacker_state->stats[PB_STAT_ATTACK] /
                       defender_state->stats[PB_STAT_DEFENSE]) / 50u + 2u;
    if (critical)
        damage = damage * 2u;
    damage = damage * (217u + (next_random(battle) % 39u)) / 255u;
    if (move_data->type == attacker_state->types[0] ||
        move_data->type == attacker_state->types[1])
        damage = damage * 15u / 10u;
    damage = damage * type_multiplier(move_data->type, defender_state) / 10u;
    if (damage == 0 && type_multiplier(move_data->type, defender_state) != 0)
        damage = 1;
    const uint32_t actual = damage > defender_state->current_hp
                                ? defender_state->current_hp : damage;
    battle->battlers[action.target].current_hp -= actual;
    if (critical)
        emit(battle, PB_EVENT_CRITICAL, action.actor, action.target, 0,
             action.move_id);
    emit(battle, PB_EVENT_DAMAGE, action.actor, action.target,
         -(int32_t)actual, action.move_id);
    if (defender_state->current_hp == 0)
        emit(battle, PB_EVENT_BATTLE_FINISHED, action.actor, action.target, 0,
             0);
    battle->turn++;
    const uint32_t kind = defender_state->current_hp == 0 ? PB_STEP_FINISHED
                                                         : PB_STEP_EVENTS;
    return (pb_step_result){ sizeof(pb_step_result), kind, battle->event_count,
                             battle->events };
}
