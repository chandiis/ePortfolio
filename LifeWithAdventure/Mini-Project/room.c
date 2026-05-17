//Frans Ekberg
//contains funtions for room struct
#include <stdio.h>
#include <stdlib.h>
#include "structs.c"
#include "player.h"
#include "dtekv-lib.h"
#include "room.h"

room new_room(int id, char* name, char* intro, int has_enemy, int has_npc){
    room new;
    new.id = id;
    new.name = name;
    new.intro = intro;
    new.exits[0] = 0;
    new.exits[1] = 0;
    new.exits[2] = 0;
    new.has_enemy = has_enemy;
    new.has_npc = has_npc;
    return new;
}
