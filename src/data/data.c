#include "data.h"

#include <stddef.h>

/* Imported from upstream 65fbf322d1bf39791676c6699da3d2cd44130ef0:
   src/data/pokemon/species_info/gen_3_families.h (Bulbasaur/Charmander).
   The IDs and base stats are the upstream Gen 3 values. */
static const struct pb_species_data species[] = {
    { .id = 1, .base_attack = 49, .base_defense = 49, .base_hp = 45 },
    { .id = 4, .base_attack = 52, .base_defense = 43, .base_hp = 39 },
};

/* Imported from upstream src/data/moves_info.h. Pound is move 1. Tackle is
   move 33; its 40 power is the current upstream data configuration. */
static const struct pb_move_data moves[] = {
    { .id = 1, .power = 40 },
    { .id = 33, .power = 40 },
};

const struct pb_species_data *pb_species(uint16_t id)
{
    for (unsigned i = 0; i < sizeof(species) / sizeof(species[0]); ++i)
        if (species[i].id == id)
            return &species[i];
    return &species[0];
}

const struct pb_move_data *pb_move(uint16_t id)
{
    for (unsigned i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i)
        if (moves[i].id == id)
            return &moves[i];
    return NULL;
}
