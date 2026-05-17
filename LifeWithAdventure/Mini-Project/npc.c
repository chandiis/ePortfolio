//Frans Ekberg
//contains funtions for npc struct
#include <stdio.h>
#include <stdlib.h>
#include "structs.c"
#include "npc.h"
#include "dtekv-lib.h"

npc new_npc(int room, char* name, char* dialogue[], int shop){
    npc new;
    new.room = room;
    new.name = name;
    for(int i=0; i < 10; i++){
        new.dialogue[i] = dialogue[i];
    }
    new.shop = shop;
    return new;
}