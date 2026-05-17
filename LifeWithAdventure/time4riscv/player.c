//Frans Ekberg
//contains funtions for player struct
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.c"
#include "player.h"
#include "dtekv-lib.h"

#define start_hp 100

player* new_player(int id ,char name[], int age){
    player *new = (player*)(0x03000000 + id*300);
    new->id = id;
    new->name = name;
    new->age = age;
    new->hp = start_hp;
    new->money = 100;
    new->damage = 5;
    for (int i = 1; i < 6; i++){
        new->inventory[i] = "empty";
    }
    return new;
}

//set player hp, returns 1 if player is still alive
int setP_hp(player* p, int change){
    if ((p->hp + change) <= 0){
        p->hp = 0;
        return 0; 
    }else{
        p->hp = (p->hp+change);
        return 1;
    }
}

void check_damage(player* p){
    int priority = 0;
    int i = 1;
    while (i < 6){
        char* item = p->inventory[i];
        if(strcmp(item, "Longsword") == 0){
            p->damage = 20;
            break;
        }else if(strcmp(item, "Shortsword") == 0){
            p->damage = 15;
            priority = 2;
        }else if(strcmp(item, "Dagger") == 0 && priority < 2){
            p->damage = 8;
            priority = 1;
        }else if(priority < 1){
            p->damage = 5;
        }
        i++;
    }
}

//returns the position of the item in the players inventory, returns 0 if there is no such item
int peek_inventory(player* p, char* item){
    int i = 1;
    while (i < 6){
        if(strcmp(p->inventory[i], item) == 0){
            return i;
        }
        i++;
    }
    return 0;
}

int add_to_inventory(player* p, char* item){
    int i = 1;
    while (i < 6){
        if(strcmp(p->inventory[i], "empty") == 0){
            p->inventory[i] = item;
            check_damage(p);
            return 1;
        }
        i++;
    }
    print("\nInventory full!");
    return 0;
}

char* remove_from_inventory(player* p, int option){
    char* dropped = p->inventory[option];
    p->inventory[option] = "empty";
    check_damage(p);
    return dropped;
}

void print_inventory(player* p){
    print("\nYou have ");
    print_dec(p->money);
    print(" coins\n");
    print("Inventory:\n");
    int i = 1;
    while (i < 6){
        if(strcmp(p->inventory[i], "empty") != 0){
            print_dec(i);
            print(". ");
            print(p->inventory[i]);
            print("\n");
        }else{
            print_dec(i);
            print(". ");
            print("_____\n");
        }
        i++;
    }
    print("\n");
}