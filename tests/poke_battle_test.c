#include "poke_battle/poke_battle.h"

#include <assert.h>
#include <stdio.h>

static pb_step_result move(pb_battle *battle, uint16_t move_id)
{
    const pb_action action = {
        .kind = PB_ACTION_MOVE, .actor = 0, .move_id = move_id, .target = 1,
    };
    return pb_step(battle, &action, 1);
}

static pb_battle *make_battle(uint16_t move_id, uint16_t target_type)
{
    const pb_battle_config config = {
        .struct_size = sizeof(config), .abi_version = PB_ABI_VERSION,
        .seed = 1234,
        .battlers = {
            { .species = 1, .level = 50, .current_hp = 200, .max_hp = 200,
              .stats = { 200, 60, 49, 55, 65, 65 }, .types = { 4, 0 },
              .moves = { { move_id, 35, 0 } } },
            { .species = 4, .level = 50, .current_hp = 200, .max_hp = 200,
              .stats = { 200, 52, 43, 65, 60, 50 }, .types = { target_type, 0 },
              .moves = { { 1, 35, 0 } } },
        },
    };
    return pb_battle_create(&config);
}

int main(void)
{
    const pb_battle_config config = {
        .struct_size = sizeof(config), .abi_version = PB_ABI_VERSION,
        .seed = 1234,
        .battlers = {
            { .species = 1, .level = 50, .current_hp = 45, .max_hp = 45,
              .stats = { 45, 60, 49, 55, 65, 65 }, .types = { 12, 0 },
              .ability = 65, .held_item = 0,
              .moves = { { 1, 35, 0 } } },
            { .species = 4, .level = 50, .current_hp = 39, .max_hp = 39,
              .stats = { 39, 52, 43, 65, 60, 50 }, .types = { 10, 0 },
              .ability = 66, .held_item = 0,
              .moves = { { 33, 35, 0 } } },
        },
    };
    pb_battle *first = pb_battle_create(&config);
    pb_battle *second = pb_battle_create(&config);
    assert(first != NULL && second != NULL);

    for (unsigned turn = 0; turn < 2; ++turn) {
        pb_step_result a = move(first, 1);
        pb_step_result b = move(second, 1);
        assert((a.kind == PB_STEP_EVENTS || a.kind == PB_STEP_FINISHED) &&
               a.count >= 2);
        assert(a.count == b.count);
        for (uint32_t i = 0; i < a.count; ++i) {
            assert(a.events[i].kind == b.events[i].kind);
            assert(a.events[i].value == b.events[i].value);
            assert(a.events[i].data_id == b.events[i].data_id);
        }
        assert(a.events[1].kind == PB_EVENT_DAMAGE);
        assert(a.events[1].value <= -1 && a.events[1].value >= -50);
    }

    pb_battle_destroy(first);
    pb_battle_destroy(second);

    pb_battle *turn = pb_battle_create(&config);
    const pb_action actions[2] = {
        { .kind = PB_ACTION_MOVE, .actor = 0, .move_id = 1, .target = 1 },
        { .kind = PB_ACTION_MOVE, .actor = 1, .move_id = 33, .target = 0 },
    };
    pb_step_result turn_result = pb_step(turn, actions, 2);
    assert(turn_result.count >= 5);
    assert(turn_result.events[0].kind == PB_EVENT_ACTION_ORDER);
    unsigned move_events = 0;
    for (uint32_t i = 0; i < turn_result.count; ++i)
        if (turn_result.events[i].kind == PB_EVENT_MOVE_USED)
            ++move_events;
    assert(move_events == 2);
    pb_battle_destroy(turn);

    pb_battle *neutral = make_battle(52, 0); /* Fire -> Normal. */
    pb_battle *super = make_battle(52, 4);   /* Fire -> Grass. */
    pb_battle *immune = make_battle(1, 13);  /* Normal -> Ghost. */
    pb_step_result neutral_result = move(neutral, 52);
    pb_step_result super_result = move(super, 52);
    pb_step_result immune_result = move(immune, 1);
    assert(-super_result.events[1].value > -neutral_result.events[1].value);
    assert(immune_result.count == 2);
    assert(immune_result.events[1].value == 0);
    pb_battle_destroy(neutral);
    pb_battle_destroy(super);
    pb_battle_destroy(immune);

    puts("poke_battle determinism: PASS");
    return 0;
}
