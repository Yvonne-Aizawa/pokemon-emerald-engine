#ifndef POKE_BATTLE_H
#define POKE_BATTLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PB_ABI_VERSION 1u

enum {
    PB_STEP_NEEDS_ACTION = 1u,
    PB_STEP_EVENTS = 2u,
    PB_STEP_FINISHED = 3u,
};

enum {
    PB_ACTION_MOVE = 1u,
    PB_EVENT_MOVE_USED = 1u,
    PB_EVENT_DAMAGE = 2u,
    PB_EVENT_BATTLE_FINISHED = 3u,
};

typedef struct pb_battle pb_battle;

enum {
    PB_STAT_HP = 0,
    PB_STAT_ATTACK = 1,
    PB_STAT_DEFENSE = 2,
    PB_STAT_SPEED = 3,
    PB_STAT_SP_ATTACK = 4,
    PB_STAT_SP_DEFENSE = 5,
    PB_STAT_COUNT = 6,
    PB_MOVE_SLOTS = 4,
};

typedef struct {
    uint16_t move_id;
    uint8_t pp;
    uint8_t _reserved;
} pb_move_slot;

typedef struct {
    uint16_t species;
    uint8_t level;
    uint8_t _reserved;
    uint32_t current_hp;
    uint32_t max_hp;
    uint16_t stats[PB_STAT_COUNT];
    uint16_t types[2];
    uint16_t ability;
    uint16_t held_item;
    pb_move_slot moves[PB_MOVE_SLOTS];
} pb_battler;

typedef struct {
    uint32_t struct_size;
    uint32_t abi_version;
    uint64_t seed;
    uint32_t ruleset_id;
    uint32_t flags;
    uint16_t species[2];
    pb_battler battlers[2];
} pb_battle_config;

typedef struct {
    uint32_t kind;
    uint32_t actor;
    uint32_t target;
    int32_t value;
    uint32_t data_id;
} pb_event;

typedef struct {
    uint32_t kind;
    uint32_t actor;
    uint32_t move_id;
    uint32_t target;
} pb_action;

typedef struct {
    uint32_t struct_size;
    uint32_t kind;
    uint32_t count;
    const pb_event *events;
} pb_step_result;

pb_battle *pb_battle_create(const pb_battle_config *config);
void pb_battle_destroy(pb_battle *battle);
pb_step_result pb_step(pb_battle *battle, const pb_action *actions,
                       uint32_t action_count);

#ifdef __cplusplus
}
#endif

#endif
