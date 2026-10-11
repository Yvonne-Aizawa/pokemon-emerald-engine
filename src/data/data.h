#ifndef PB_DATA_H
#define PB_DATA_H

#include <stdint.h>

struct pb_species_data {
    uint16_t id;
    uint16_t base_attack;
    uint16_t base_defense;
    uint16_t base_hp;
};

struct pb_move_data {
    uint16_t id;
    uint16_t power;
    uint16_t type;
};

const struct pb_species_data *pb_species(uint16_t id);
const struct pb_move_data *pb_move(uint16_t id);

#endif
