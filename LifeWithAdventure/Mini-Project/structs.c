//Frans Ekberg
//All structs are defiened here for easy access.
#include <stdio.h>
#include <stdlib.h>

typedef struct player{
    int id;
    char *name;
    int age;
    int hp;
    int money;
    int damage;
    char* inventory[6]; 
}player;

typedef struct enemy{
    int room;
    char* name;
    int hp;
    int maxhp;
    int damage;
    int attackDelay;
    char* loot;
}enemy;

typedef struct npc{
    int room;
    char *name;
    char *dialogue[10];
    int shop;
}npc;

typedef struct room{
    int id;
    char *name;
    char *intro;
    int exits[3];
    struct enemy enemy;
    int has_enemy;
    struct npc npc;
    int has_npc;
}room;


