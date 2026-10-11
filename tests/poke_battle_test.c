#include "poke_battle/poke_battle.h"

#include <assert.h>
#include <stdio.h>

static pb_step_result move(pb_battle *battle)
{
    const pb_action action = {
        .kind = PB_ACTION_MOVE, .actor = 0, .move_id = 1, .target = 1,
    };
    return pb_step(battle, &action, 1);
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
        pb_step_result a = move(first);
        pb_step_result b = move(second);
        assert((a.kind == PB_STEP_EVENTS || a.kind == PB_STEP_FINISHED) &&
               a.count >= 2);
        assert(a.count == b.count);
        for (uint32_t i = 0; i < a.count; ++i) {
            assert(a.events[i].kind == b.events[i].kind);
            assert(a.events[i].value == b.events[i].value);
            assert(a.events[i].data_id == b.events[i].data_id);
        }
        assert(a.events[1].kind == PB_EVENT_DAMAGE);
        assert(a.events[1].value <= -1 && a.events[1].value >= -30);
    }

    pb_battle_destroy(first);
    pb_battle_destroy(second);
    puts("poke_battle determinism: PASS");
    return 0;
}
