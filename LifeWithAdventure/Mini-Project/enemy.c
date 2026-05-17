//Frans Ekberg
//contains funtions for enemy struct
#include <stdio.h>
#include <stdlib.h>
#include "structs.c"
#include "enemy.h"
#include "dtekv-lib.h"

enemy new_enemy(int room, char* name, int hp, int damage, char* loot){
    enemy new;
    new.room = room;
    new.name = name;
    new.hp = hp;
    new.maxhp = hp;
    new.damage = damage;
    new.loot = loot;
    return new;
}