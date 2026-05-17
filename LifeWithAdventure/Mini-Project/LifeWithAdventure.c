//Authors:
//Chandadeep Kaur and Frans Ekberg
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.c"
#include "player.h"
#include "dtekv-lib.h"
#include "room.h"
#include "enemy.h"
#include "npc.h"

extern void print(char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupts(void);
extern int strcmp(const char *s1, const char *s2); //assembly strcmp function, so that it doesn't take C standardlibrary strcmp function
void reset(void);

#define talkdelay 4000

int mytime = 0x0000;
int hourtime = 0;
int prime = 1234567;
char textstring[] = "text, more text, and even more text!";
int timeoutcount = 0;


room rooms[10];
player* p;

int option = 0;
int alive = 1;
int in_combat = 0;

int current_room = 1; 
int next_room = 0;
int prev_room = 0;

void clearsc(){
  for(int i = 0; i < 40; i++){
    print("\n");
  }
}

void set_leds(int led_mask) { //an integer where the 10 lsb correspond to the state of the corresponding led
  volatile int* leds = (volatile int*) 0x04000000; //casting addressen till en int pekare via (volatile int*)
  
  *leds = led_mask; //set the LEDs to the value of led_mask 
}

void set_displays(int display_number, int value) { //display_number: the display the user wants to set, value: the value to the display
  volatile int* segm7 = (volatile int*)(0x04000050 + display_number * 0x10);
  *segm7 = value;
}

int get_sw(void) {
  volatile  int* switches = (volatile int*) 0x04000010;
  int value = *switches;

  //maskera de 10 lsb, dvs switches
  value = value & 0b1111111111;
  return value;
}

int get_btn(void) {
  volatile int* btn = (volatile int*) 0x040000d0;
  int value = *btn; 
  *btn = 0;
  value = value & 0b1;
  return value;
}



//display player hp on the board
//Frans Ekberg
void hp2display(int hp){
  int display_num[10] = {0b11000000, //0
                          0b11111001, //1
                          0b10100100, //2
                          0b10110000, //3
                          0b10011001, //4
                          0b10010010, //5
                          0b10000010, //6
                          0b11111000, //7
                          0b10000000, //8
                          0b10010000, //9
                          };

  int h = 0b10001001; //bokstaven H
  int p = 0b10001100; //bokstaven P
  int off = 0b11111111;

  set_displays(0, display_num[hp%10]);   // ental hp
  if(hp >= 10) {
    int tiotal = (hp/10) % 10;
    set_displays(1, display_num[tiotal]); // tiotal hp
  }  
    else {
      set_displays(1, off);
  }
  if(hp >= 100) {
    set_displays(2, display_num[hp/100]);   //hundratal hp
  }
  else {
    set_displays(2, off);
  }
  set_displays(3, off);
  set_displays(5, h);     //H
  set_displays(4, p);     //P
}

//set the player hp to their hp + change and displays it on the board
//Frans Ekberg
void set_hp(player* p, int change){
  alive = setP_hp(p, change);
  hp2display(p->hp);
}

//reads the values of the switches and sets the hourtime to that value
void set_hourtime(void){
  //uppgift h
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  int right_sw_value = get_sw() & 0b0000111111; //mask out everything but the six lsb
  int msb_sw_value = (get_sw() >> 8) & 0b11; //shift to get the 2 msb -> 2 lsb and then mask out everything but those 2 lsb

  int exit_value = (get_sw() >> 7) & 0b1; //shift so that SW7 is lsb and mask out everything but that
  if(exit_value == 1) {
    return;  //exit if SW7 is on
  }

  int value = get_btn(); //read the button value
  if (value == 1) {
    if(msb_sw_value == 0b01) {
      seconds = right_sw_value;
      int single_second = seconds%10;
      int ten_second = seconds/10; 
      int hex_second = (ten_second << 4)+(single_second); 
      hourtime = (0xFFFF00 & hourtime) + hex_second;
    }
    if(msb_sw_value == 0b10) {
      minutes = right_sw_value;
      int single_minute = minutes%10;
      int ten_minute = minutes/10; 
      int hex_minute = (ten_minute << 12)+(single_minute << 8); 
      hourtime = (0xFF00FF & hourtime) + hex_minute;
    }
    if(msb_sw_value == 0b11) {
      hours = right_sw_value;
      int single_hour = hours%10;
      int ten_hour = hours/10; 
      int hex_hour = (ten_hour << 20)+(single_hour << 16); 
      hourtime = (0x00FFFF & hourtime) + hex_hour;
    }
    mytime = hourtime;
  } 

  return;
}

//displays hourtime on the displays
void time2display(int hourtime){
  int single_second = hourtime & 0xF;
  int ten_second = (hourtime >> 4) & 0xF;
  int single_minute = (hourtime >> 8) & 0xF;
  int ten_minute = (hourtime >> 12) & 0xF;
  int single_hour = (hourtime >> 16) & 0xF;
  int ten_hour = (hourtime >> 20) & 0xF;

  int display_num[10] = {0b11000000, //0
                          0b11111001, //1
                          0b10100100, //2
                          0b10110000, //3
                          0b10011001, //4
                          0b10010010, //5
                          0b10000010, //6
                          0b11111000, //7
                          0b10000000, //8
                          0b10010000, //9
                          };

  int display_num_dot[10] = {0b01000000, //0
                          0b01111001, //1
                          0b00100100, //2
                          0b00110000, //3
                          0b00011001, //4
                          0b00010010, //5
                          0b00000010, //6
                          0b01111000, //7
                          0b00000000, //8
                          0b00010000, //9
                          };

  set_displays(0, display_num[single_second]);   // ental sekunder
  set_displays(1, display_num[ten_second]);   // tiotal sekunder
  set_displays(2, display_num_dot[single_minute]);   // ental minuter
  set_displays(3, display_num[ten_minute]);   // tiotal minuter
  set_displays(4, display_num_dot[single_hour]);     // ental timmar
  set_displays(5, display_num[ten_hour]);     // tiotal timma
  return;
}

//ticks hourtime
void tick_hour(void){
  if(mytime % 0x10000 == 0){
    if ((hourtime >> 16) == 0x09){
      hourtime = 0x100000;
    }else if((hourtime >> 16) == 0x19){
      hourtime = 0x200000;
    }else if((hourtime >> 16) == 0x23){
      hourtime = 0x0;
    }else{
      hourtime += 0x10000;
      hourtime = 0xFF0000 & hourtime;
      hourtime += (mytime & 0xFFFF);
    }
  }else{
    hourtime = 0xFF0000 & hourtime;
    hourtime += (mytime & 0xFFFF);
  }
  return;
}


//waits for an switch to be flicked by a player
//Frans Ekberg
void wait_for_option(void){
  while(option == 0){
    delay(10);
  }
}


//if a player enters a room with combat, the combat function is called
//Frans Ekberg
void combat(room* r, player* p){
  if(r->has_enemy == 0){
    return;
  }else if(r->enemy.hp <= 0){
    return;
  }
  in_combat = 1;
  int def = 0;
  clearsc();
  print("\nYou encounter a ");
  print(r->enemy.name);
  print("!\n");
  delay(talkdelay);
  while(p->hp > 0 && r->enemy.hp > 0){
    clearsc();
    print("\nWhat do you do now? \n1. Attack \n2. Defend \n3. Heal \n4. Flee\n");
    
    wait_for_option();
    clearsc();
    if(option == 1){//Attack
      option = 0;
      print("\nYou choose to Attack.\n");
      delay(talkdelay/2);
      if(p->damage == 5){
        print("\nYou punch the ");
        print(r->enemy.name);
        print(" and you hit!\n");
      }else{
        print("\nYou swing your sword at the ");
        print(r->enemy.name);
        print(" and you hit!\n");
      }
      r->enemy.hp = (r->enemy.hp - p->damage);
      delay(talkdelay/2);

      if(r->enemy.hp < 1){
        print("\nYou defeated the ");
        print(r->enemy.name);
        print(" and got some loot!\n +1 ");
        print(r->enemy.loot);
        print("\n");
        add_to_inventory(p, r->enemy.loot);
        delay((int)(talkdelay*1.5));
        in_combat = 0;
        return;
      }

      if(r->enemy.maxhp/4 >= r->enemy.hp){
        print("You see that the "); 
        print(r->enemy.name);
        print(" has almost no fight left in them.\n");
        delay(talkdelay/2);
      }else if(r->enemy.maxhp/2 >= r->enemy.hp){
        print("You see that the "); 
        print(r->enemy.name);
        print(" is bleeding.\n");
        delay(talkdelay/2);
      }
      

    }else if(option == 2){ //Defend
      option = 0;
      print("\nYou choose to Defend.\n");
      delay(talkdelay/2);
      print("\nYou steady your feet and brace yourself to defend the incoming attack.\n");
      delay(talkdelay);
      def = 1;

    }else if(option == 3){ //Heal
      option = 0;
      int pos = peek_inventory(p, "Health Potion");
      if(pos != 0){
        print("\nYou drink a health potion and gain some hp!\n");
        delay(talkdelay/4);
        print("-1 Health Potion\n");
        remove_from_inventory(p, pos);
        set_hp(p, 50);
      }else{
        print("You don't have any health potions.\n");
        delay(talkdelay/2);
        continue;
      }
      delay(talkdelay);

    }else if(option == 4){//Flee
      option = 0;
      print("You decide to flee back the way you came.\n");
      delay(talkdelay);
      next_room = prev_room;
      in_combat = 0;
      return;
    }else{
      option = 0;
      continue;
    }

    //enemy attack
    print("\n\nThe ");
    print(r->enemy.name);
    print(" charges towards you, preparing to attack.\n");
    delay(talkdelay/2);
    print("\nThe ");
    print(r->enemy.name);
    print(" hits you!");
    delay(talkdelay/4);

    if(def == 1){
      set_hp(p, -(r->enemy.damage/2));
      print("\nYou defend against the attack and only take ");
      print_dec(r->enemy.damage/2);
      def = 0;

    }else{
      set_hp(p, -(r->enemy.damage));
      print("\nYou take ");
      print_dec(r->enemy.damage);
    }
    print(" damage.\n");
    delay(talkdelay/2);


    if(alive == 0){
      print("\nYou stumble to the ground and you sight slowly fades...\n");
      delay(talkdelay);
      in_combat = 0;
      return;
    }

    print("\nYou pull yourself together and regain focus.\n");
    delay(talkdelay);

  }
  in_combat = 0;
  return;
}

//if the player is in a shop
//Frans Ekberg & Chandadeep Kaur
void shop(player* p){
  while(1){
    clearsc();
    print("What do you want to do? \n1. Buy. \n2. Sell. \n3. Leave.");
  
    wait_for_option();
    clearsc();
    if(option == 1){//Buy
      buy:
      option = 0;
      print("\nWhat do you want to buy?\nYour coins: ");
      print_dec(p->money);
      print("\n\n1. Shortsword - 75 coins. \n2. Health Potion - 25 coins.\n3. Back.");
      wait_for_option();
      clearsc();
      if(option == 1){//Shortsword
        option = 0;
        if((p->money-75) >= 0){
          if(add_to_inventory(p, "Shortsword") == 1){
            p->money = p->money-75;
            print("\n+1 Shortsword\n");
          }
        }else{
          print("\nYou don't have enough money to buy that.\n");
        }

      }else if(option == 2){//Health Potion
        option = 0;
        if((p->money-25) >= 0){
          if(add_to_inventory(p, "Health Potion") == 1){
            p->money = p->money-25;
            print("\n+1 Health Potion\n");
          }
        }else{
          print("You don't have enough money to buy that.");
        }
      }else if(option == 3){//Back
        option = 0;
        continue;
      }else{
        option = 0;
        goto buy;
      }
      delay(talkdelay);

    }else if(option == 2){//Sell
      sell:
      option = 0;
      print("\nWhat do you want to sell? \n1. Longsword - 75 coins \n2. Helth Potion - 20 coins\n3. Wolf meat - 100 coins\n4. Shortsword - 50 coin \n5. Back");
      
      wait_for_option();
      clearsc();

      if(option == 1){//Longsword
        option = 0;
        int pos = peek_inventory(p, "Longsword");
        if(pos != 0){
          remove_from_inventory(p, pos);
          print("\nLongsword sold \n+75 coins\n");
          p->money = p->money + 25;
          
        }else{
          print("\nYou don't have a Longsword to sell.\n");
        }

      }else if(option == 2){//Health Potion
        option = 0;
        int pos = peek_inventory(p, "Health Potion");
        if(pos != 0){
          remove_from_inventory(p, pos);
          print("\nHealth Potion sold \n+20 coins\n");
          p->money = p->money + 20;
        }else{
          print("\nYou don't have a Health Potion to sell.\n");
        }

      }else if(option == 3){//Wolf Meat
        option = 0;
        int pos = peek_inventory(p, "Wolf Meat");
        if(pos != 0){
          remove_from_inventory(p, pos);
          print("\nWolf Meat sold \n+100 coins\n");
          p->money = p->money + 100;
        }else{
          print("\nYou don't have Wolf Meat to sell.\n");
        }

      }else if(option == 4){//Shortsword
        option = 0;
        int pos = peek_inventory(p, "Shortsword");
        if(pos != 0){
          remove_from_inventory(p, pos);
          print("\nShortsword sold \n+50 coins\n");
          p->money = p->money + 50;
        }else{
          print("\nYou don't have a Shortsword to sell.\n");
        }
      }else if(option == 5){//Back
        option = 0;
        continue;
      }else{//Wrong option default
        option = 0;
        goto sell;
      }
      delay(talkdelay);
    }else if(option == 3){//Leave
      option = 0;
      print("\nVendor: Okay, bye traveler!\n ");
      delay(talkdelay/2);
      print("\n- Bye!\n");
      delay(talkdelay/2);
      return;
    }else{//Wrong option default
      option = 0;
      continue;
    }
  }
}


/* Below is the function that will be called when an interrupt is triggered. */
//gets called in the function external_irq in boot.S
void handle_interrupt(unsigned cause) {

  //if the swiches are turned on
  if(cause == 17){
    volatile unsigned int* edgecapture = (volatile unsigned int*)(0x400001C);
    if(*edgecapture == 0b100000){//switch 5
      *edgecapture = 0b100000;
      delay(1);
      option = 5;
    }else if(*edgecapture == 0b10000){ //switch 4
      *edgecapture = 0b10000;
      delay(1);
      option = 4;
    }else if(*edgecapture == 0b1000){ //switch 3
      *edgecapture = 0b1000;
      delay(1);
      option = 3;
    }else if(*edgecapture == 0b100){ //switch 2
      *edgecapture = 0b100;
      delay(1);
      option = 2;
    }else if(*edgecapture == 0b10){ //switch 1
      *edgecapture = 0b10; 
      delay(1);
      option = 1;
    }else if(*edgecapture == 0b1){ //switch 0 (inventory)
      *edgecapture = 0b1;
      clearsc();
      print_inventory(p);
      delay(2000);
      print("\nPress Button 1 to continue.\n");
      while(get_btn() == 0){
        delay(10);
      }
      option = 6; //resets the current text on sceen to re-print
    }else{
      *edgecapture = 0b1111111111;
      delay(1);
    }

  }
  //if timer times out
  if(cause == 16){
    volatile unsigned short* status = (volatile unsigned short*)(0x4000020); //adress to timer status
    *status = 0b0000000000000010;     //reset the timeout bit.
    timeoutcount++;

    //deathtrigger
    if(alive == 0 && in_combat == 0){
      //trigger death
      clearsc();
      print("\n\nYou Died!\n");
      delay(8000);
      print("\n\nRestarting game...");
      delay(4000);
      reset();
    }
    
    //checka option
    /*if(option != 0){
      print("\nDu valde nr ");
      print_dec(option);
      option = 0;
    }*/

    //combat(&rooms[1], p);

    if(timeoutcount % 10 == 0){
      set_hourtime();         // Sets the hourtime using switches and key1
      // time2display(hourtime); // defendDisplays the hourtime on the display
      tick(&mytime);     // Ticks the clock once
      tick_hour();        //  Ticks the hourtime
    }
  }
}

//checks if the timer has timed out
int timer_timeout(void){
  int timeout = 0;                    //timeout status is set to 0, the clock hasn't timed out
  volatile unsigned short* status = (volatile unsigned short*)(0x4000020); //adress to timer status
  if(*status == 0b0000000000000011){  //if the lsb, the TO bit, is one, the timer has timed out
    timeout = 1;                      //set the timeout status to 1, yes it has timed out;
    *status = 0b0000000000000010;     //reset the timeout bit.
  }
  return timeout;                     //return the timeout status
}


//initializing interrupts
void init(void) {
  //gets all adresses for control, periodl periodh
  volatile unsigned short* control = (volatile unsigned short*)(0x4000024);
  volatile unsigned short* periodl = (volatile unsigned short*)(0x4000028);
  volatile unsigned short* periodh = (volatile unsigned short*)(0x400002C);
  volatile unsigned int* interruptmask = (volatile unsigned int*)(0x4000018);
  volatile unsigned int* edgecapture = (volatile unsigned int*)(0x400001C);

  //sets periodl and h to be 2 999 999 togehter because (3 000 000 - 1) = 10 restarts / s
  *periodl = 0b1100011010111111;
  *periodh = 0b0000000000101101;
  //sets control bits 2 -> START = 1, 1 -> CONT = 1 and 0 -> ITO = 1
  *control = 0b0000000000000111;

  *interruptmask = 0b111111;
  *edgecapture = 0b111111;


  enable_interrupts();
}

//room initiation
//Chandadeep Kaur
void room_init(void){
  //rum 1
  char* intro = "You wake up in a small village, as any usual day. There is a rumor about a mysterious old clocktower at the other side of the mountain. \nYou have always wondered if the rumors are true. As you go about your day, you walk by an old man.";
  rooms[1] = new_room(1, "Village", intro, 0, 1);
  //rum 2
  intro = "You walk along a forest path with crowded space and mystical sounds. Birds are singing, but you sense something moving in the bushes.";
  rooms[2] = new_room(2, "Forest Path", intro, 1, 0);
  //rum 3
  intro = "You find an abandoned and ancient well covered in moss. The air feels heavy and although it is empty inside, strange noises can be heard.";
  rooms[3] = new_room(3, "Old Well", intro, 0, 0);
  //rum 4
  intro = "Feeling exhausted, you look for a place to relax. There, you see the village bar, known for its cozy atmosphere and amazing drinks. \nYou enter the village bar and the smell of beer fills the air. As you sit down, you cross  eyes with the bartender, looking at you fascinated.";
  rooms[4] = new_room(4, "Village Bar", intro, 0, 1);
  //rum 5
  intro = "The market is bustling with merchants. Anything can be found here - weapons, food, drinks - you name it. However, they come at a price.";
  rooms[5] = new_room(5, "Marketplace", intro, 0, 1);
  //rum 6 
  intro = "At the edge of the market lies a dark cave entrance. Cold air seeps from the entrance.";
  rooms[6] = new_room(6, "Cave Entrance", intro, 1, 1);
  //rum 7
  intro = "As you go deeper into the cave, you find yourself surrounded with the sound of dripping water and echoes of your own footsteps. \nIt might be dangerous to go further, but it is a risk you are willing to take.";
  rooms[7] = new_room(7, "Deep Cave", intro, 0, 0);
  //rum 8
  intro = "When you finally get out of the cave, you reach a deserted wooden bridge. The fog limits your sights. There is no turning back now.";
  rooms[8] = new_room(8, "Abandoned Bridge", intro, 0, 0);
  //room 9
  intro = "Having crossed the bridge, you see the place you have searched for all this time - the well known clocktower. Its massive clock ticks ominously. \nYou think to yourself, \"the treasure must be inside\".";
  rooms[9] = new_room(9, "Clocktower", intro, 1, 0);

  //connect rooms
  //room 1 
  rooms[1].exits[0] = 2; //Village -> Forest Path
  //room 2
  rooms[2].exits[0] = 1; //Forest Path -> Village
  rooms[2].exits[1] = 3; //Forest Path -> Old Well
  rooms[2].exits[2] = 4; //Forest Path -> Village Bar
  //room 3
  rooms[3].exits[0] = 2; //Old Well -> Forest Path
  rooms[3].exits[1] = 5; //Old Well -> Marketplace
  //room 4
  rooms[4].exits[0] = 2; //Village Bar -> Forest Path
  rooms[4].exits[1] = 5; //Village Bar -> Marketplace
  //room 5
  rooms[5].exits[0] = 3; //Marketplace -> Old Well
  rooms[5].exits[1] = 4; //Marketplace -> Village Bar
  rooms[5].exits[2] = 6; //Marketplace -> Cave Entrance
  //room 6
  rooms[6].exits[0] = 5; //Cave Entrance -> Marketplace
  rooms[6].exits[1] = 7; //Cave Entrance -> Deep Cave
  //room 7
  rooms[7].exits[0] = 6; //Deep Cave -> Cave Entrance
  rooms[7].exits[1] = 8; //Deep Cave -> Abandoned Bridge
  //room 8
  rooms[8].exits[0] = 7; //Abandoned Bridge -> Deep Cave
  rooms[8].exits[1] = 9; //Abadoned Bridge -> Clocktower
  //room 9
  rooms[9].exits[0] = 8; //Clocktower -> Abandoned Bridge
  
  //enemy placement
  rooms[2].enemy = new_enemy(2, "Wolf", 20, 5, "Wolf Meat");
  rooms[6].enemy = new_enemy(6, "Bandit", 50, 12, "Longsword");
  rooms[9].enemy = new_enemy(9, "Dragon", 100, 20, "Chest Key");

  //npc placment
  //questions from the player and answers from npc
  char* oldman_dialogue[] = {"Hey there, traveler.", 
                             "Who are you?", 
                             "I am just your average old man, one who has lived too long and seen too much.", 
                             "Do you know anything about the Clocktower?", 
                             "Gold, jewels, and treasure... all waiting to be claimed. Yet no one has ever come close. Perhaps you will be the one to succeed where all others have failed.", NULL};

  char* bartender_dialogue[] = {"Welcome to Village Bar! What can I get you?\n \n- I would like a drink, please.\n \nBartender: It's on the house. I have heard you are going to the Clocktower, and I might know something interesting about that. But knowledge isn't free, adventurer. The choice is yours.", 
                                "\n1. I want to know (-25 coins)", "\n2. Maybe another time.", 
                                "I have heard whispers… A beast guards the treasure inside. The monster is no ordinary foe. it is fierce and merciless. \nBring weapons, bring courage, and maybe you'll live to tell the tale.",
                                "Empty pockets, eh? Come back when you have coins."};

  char* vendor_dialogue[] = {"Welcome to our famous shop! What can I get you?"};

  char* bandit_dialogue[] = {"\nWell, well... what do we have here? A lonely traveler, carrying shiny coins and maybe some treasures? Hand it over, and I might just let you walk away.", 
                             "\n1. Yes (-50 coins).", "\n2. No (Fight).", 
                             "You have made a wise choice, traveler.\n", "So that's how it's gonna be? Be prepared to die!"};

  rooms[1].npc = new_npc(1, "Old man", oldman_dialogue, 0);
  rooms[4].npc = new_npc(4, "Bartender", bartender_dialogue, 0); //the following consequences of the options will develop in talk_to_npc
  rooms[5].npc = new_npc(5, "Vendor", vendor_dialogue, 1);
  rooms[6].npc = new_npc(6, "Bandit", bandit_dialogue, 0);
}

//player initiation
void player_init(void){
  p = new_player(1,"Alex", 22);
}


//winning the game causes this function to run
//Frans Ekberg
void win(void){
  clearsc();
  char* gametime = "";
  time2string(gametime, mytime);
  print("As the dragon falls to the ground, you pick up a key hanging around the neck of the beast.");
  delay(talkdelay);
  print("\nYou look across the room and see a massive chest appering behind the fallen foe.");
  delay(talkdelay);
  print("\nYou approach the chest that looks to be bigger that any coffin you have ever seen before, and you try the key...");
  delay(talkdelay);
  print("\nIt fits! You hastily turn the key and the chest swings open!\n");
  delay(talkdelay);
  print("\nA mountain of gold appeares in front of you and you are filled with joy!");
  delay(talkdelay);
  print("\nNot so much because of the riches, but becasue you were the adventurer to slay the dragon and prove the rumors to be true.");
  delay(talkdelay);
  
  print("\n\n Congrats! You won the game!\n");
  print("\nYou played for ");
  print(gametime);
  delay(talkdelay);

  print("\n\nYippie!!\n");
  delay(talkdelay);

  print("\nWould you like to play again?\nPress Button 1.");
  while(get_btn() == 0){
    delay(10);
  }
  reset();
}

//when leave option is chosen, this function is called
//Chandadeep Kaur
void room_exits(room *r, player *p) {
  // delay(talkdelay);
  exits:
  clearsc();
  print("You leave the ");
  print(r->name);
  print(".\n\nWhere would you like to go?\n");

    for(int i = 0; i < 3; i++){
      if(r->exits[i] != 0) {
        print_dec(i + 1);
        print(". "); //1., 2., 3.
        print(rooms[r->exits[i]].name);
        print("\n");
      }
    }

    option = 0;
    wait_for_option();

    if(option >= 1 && option <= 3 && r->exits[option - 1] != 0) {
      prev_room = current_room;
      current_room = r->exits[option - 1];
      next_room = 0;
    }
    else{ //wrong option default
      option = 0;
      goto exits;
    }

}

//if the room has an npc, this function is called
//Chandadeep Kaur
void talk_to_npc(room *r, player *p) {
  npc *current_npc = &r->npc;
  delay(talkdelay);
  clearsc();

  if(strcmp(current_npc->name, "Old man") == 0) { //if the npc is the old man
    int i = 0;
    while(current_npc->dialogue[i] != NULL){
      if(i % 2 == 0) {
        print("\n");
        print(current_npc->name);
        print(": ");
        print(current_npc->dialogue[i]);
      } else {
        print("\n- ");
        print(current_npc->dialogue[i]);
      }
      print("\n");
      delay(talkdelay);
      i++;
    }
    print("\n1. Thank you.");
    wait_for_option();
    room_exits(r, p);
    return;
  }
  else if(strcmp(current_npc->name, "Bartender") == 0){
    //intro
    print("\n");
    print(current_npc->name);
    print(": ");
    print(current_npc->dialogue[0]);
    print("\n");

    //view choices
    print("\nYour coins: ");
    print_dec(p->money);
    print("\n");
    print(current_npc->dialogue[1]);
    print(current_npc->dialogue[2]);
    
    
    bartender:
    option = 0;
    wait_for_option();
    if(p->money >= 25){
      if(option == 1) { //get information
        option = 0;
        p->money = p->money - 25;
        print("\n-25 coins. You now have ");
        print_dec(p->money);
        print(" coins.");
        delay(talkdelay);
        print("\n\nBartender: ");
        print(current_npc->dialogue[3]); //information about monster
        print("\n");
        delay(2*talkdelay);
        print("\n1. Thank you!");
        wait_for_option();
        room_exits(r, p);
        return;
      }
      else if(option == 2) { //dont pay and leave
        option = 0;
      }
      else{//Wrong option default
        option = 0;
        goto bartender;
      }
    }else if(option == 2){
      option = 0;
    }else{
      print("\n You don't have enough coins.");
      delay(talkdelay/2);
    }
    option = 0;
    delay(talkdelay/2);
    print("\n\nBartender: ");
    print(current_npc->dialogue[4]);
    delay(talkdelay);
    print("\n1. Leave");
    wait_for_option();
    room_exits(r, p);
    return;
  }
  else if(strcmp(current_npc->name, "Vendor") == 0) {
    print("\n");
    print(current_npc->name);
    print(": ");
    print(current_npc->dialogue[0]);
    print("\n");
    delay(3000);
    shop(p);
    room_exits(r, p);
    return;
  }
  else if(strcmp(current_npc->name, "Bandit") == 0) {
    if(r->enemy.hp <= 0){
      print("\nThe corpse of the bandit still lies here.\n");
      delay(talkdelay/2);
      print("\n1. Leave");
      wait_for_option();
      room_exits(r, p);
      return;
    }
    print("\n");
    print(current_npc->name);
    print(": ");
    print(current_npc->dialogue[0]);

    //view choices
    print(current_npc->dialogue[1]);
    print(current_npc->dialogue[2]);

    option = 0;
    bandit:
    wait_for_option();
    if(p->money >= 50) { //give money to bandit
      if(option == 1){
        option = 0;
        p->money = p->money - 50;
        print("\n-50 coins. You now have ");
        print_dec(p->money);
        print(" coins.");
        delay(2000);
        print("\n\nBandit: ");
        print(current_npc->dialogue[3]);
        delay(talkdelay);
        room_exits(r, p);
        return;
      }else if(option == 2) { //dont pay and fight
        option = 0;
      }
      else{
        option = 0;
        goto bandit;
      }
    }
    else if(option == 2) { //dont pay and fight
        option = 0;
    }
    else{//Wrong option default
      print("\n You don't have enough coins.");
      delay(1000);
    }
    option = 0;
    print("\n");
    print(current_npc->dialogue[4]);
    delay(talkdelay);
    combat(r, p);
    if(next_room != 0){
      prev_room = current_room;
      current_room = next_room;
      next_room = 0;
      return;
    }
    room_exits(r, p);
    return;
  }
}

//story and choices for each room
//Chandadeep Kaur
void room_actions(room *r, player *p) {
  goto start;
  re:
  clearsc();
  print(r->intro);
  print(".\n\n");
  start:
  if(strcmp(r->name, "Village") == 0){ //room 1
    print("1. Approach the old man.\n");
    print("2. Leave.\n");

    option = 0;
    wait_for_option();

    if(option == 1) {
      option = 0;
      print("\nYou decide to talk to the old man.\n");
      talk_to_npc(r, p);
    }
    else if(option == 2){
      print("\nYou decide to leave.");
      option = 0;
      room_exits(r, p);
    }
    else {
      option = 0; //wrong choice default
      goto re;
    }
  }
  if(strcmp(r->name, "Forest Path") == 0){ //room 2
    print("1. Explore the forest.\n");
    print("2. Leave.\n");

    option = 0;
    wait_for_option();

    if(option == 1){
      print("\nYou decide to explore the forest.\n");
      delay(talkdelay/2);
      option = 0;
      if(r->enemy.hp <= 0){
        print("\nThe Wolf lies slain beside the path, but no other creatures are in sight.\n");
        delay(2000);
        print("\n1. Leave");
        wait_for_option();
        room_exits(r, p);
      }else{
        combat(r, p);
        if(next_room != 0){
          prev_room = current_room;
          current_room = next_room;
          next_room = 0;
          return;
        }
        room_exits(r, p);
      }
    }
    else if(option == 2){
      print("\nYou decide to leave.");
      option = 0;
      room_exits(r, p);
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Old Well") == 0){//room 3
    print("1. Inspect the well.\n");
    print("2. Leave.\n");

    option = 0;
    wait_for_option();

    if(option == 1){
      option = 0;
      //lägg till logik för att bara ha en nyckel
      print("\nYou decide to inspect the well.\n");
      delay(talkdelay/2);
      if(peek_inventory(p, "Key") != 0){
        print("\nAll you can see down there is darkness, but you can hear the sound of drops hitting the water below.");
        delay(talkdelay);
      }else{
        print("\nAs you look closely, something shining catches your eye...It's a key!");
        delay(talkdelay);
        print("\nYou obtained a key!\n");
        delay(talkdelay/2);
        print("\n+1 Key\n");
        add_to_inventory(p, "Key");
      }
      delay(talkdelay);
      room_exits(r, p);
    }
    else if(option == 2){
      option = 0;
      print("\nYou decide to leave.");
      room_exits(r, p);
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Village Bar") == 0){ //room 4
    print("1. Approach the Bartender.\n");
    print("2. Leave.\n");

    option = 0;
    wait_for_option();

    if(option == 1){
      option = 0;
      print("\nYou decide to talk to the bartender.\n");
      talk_to_npc(r, p);
    }
    else if(option == 2){
      option = 0;
      print("\nYou decide to leave.");
      room_exits(r, p);
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Marketplace") == 0){ //room 5
    print("1. Approach the Vendor.\n");
    print("2. Leave.\n");

    option = 0;
    wait_for_option();

    if(option == 1){
      option = 0;
      print("\nYou decide to approach the vendor.\n");
      talk_to_npc(r, p);
    }
    else if(option == 2){
      option = 0;
      print("\nYou decide to leave.");
      room_exits(r, p);
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Cave Entrance") == 0){ //room 6
    print("1. Explore the ominous cave.\n");
    print("2. Go back to the Marketplace.");

    option = 0;
    wait_for_option();

    if(option == 1){
      option = 0;
      print("\nAs you move into the cave you feel a chilling breeze.\n");
      talk_to_npc(r, p);
    }
    else if(option == 2){
      option = 0;
      print("\nYou decide to leave.");
      prev_room = current_room;
      current_room = 5; //marketplace is room 5
      next_room = 0;
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Deep Cave") == 0){ //room 7
    print("1. Explore further.\n");
    //You cannot proceed until you pass the traps

    option = 0;
    wait_for_option();

    if(option == 1){
      option = 0;
      print("\nAs you move deeper into the cave, the darkness creeps in around you.\n");
      delay(talkdelay);
      print("\nYou notice that there are traps ahead of you that you need to avoid!\n");
      print("\n(Avoid with switch 3!)");
      delay(5000);
      if(option == 3){
        option = 0;
        print("\nYou managed to avoid the traps and proceed safely.\n");
        delay(talkdelay);
        room_exits(r, p);
      }
      else{
        set_hp(p, -25);
        delay(100);
        print("\nYou lost 25 hp but managed to get through anyway.");
        delay(talkdelay);
        room_exits(r, p);
      }
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Abandoned Bridge") == 0){ //room 8
    print("1. Cross the bridge.\n");
    //You cannot proceed until you cross the bridge

    option = 0;
    wait_for_option();

    if(option == 1) {
      option = 0;
      print("\nYou decide to cross the bridge.\n");
      delay(talkdelay);
      print("\nYou notice that it has a big gap in the middle, however, it is possible to jump to the other side.");
      print("\n(Jump by pressing Button 1!)");
      
      int jumped = 0;
      int timeout = 600; //6 sec
      
      while (timeout > 0){
        if(get_btn()) {
          jumped = 1;
          break;
        }
        delay(10); //waiting 10 ms
        timeout--;
      }

      if(jumped) {
        print("\nYou jump and barely make it to the other side. But you land on the edge and get up safely.\n");
        delay(talkdelay);
        print("You made it!\n");
        delay(talkdelay);
        room_exits(r, p);
      } else {
        print("\nThe second your foot leaves the ground, you slip with your other foot!\n");
        delay(talkdelay/2);
        print("You look down and see nothing but darkness as you fall to your demise.\n");
        delay(talkdelay);
        alive = 0;
        in_combat = 0;
      }
    }
    else{
      option = 0;
      goto re;
    }
  }
  if(strcmp(r->name, "Clocktower") == 0){ //room 9
    print("1. Unlock the door to the Clocktower.\n");
    print("2. Leave.\n");

    option = 0;
    wait_for_option();

    if(option == 1){
      option = 0;
      print("\nYou attempt to unlock the door...");
      delay(talkdelay/2);
      //if the player has the key
      int pos = peek_inventory(p, "Key");

      if(pos != 0) {
        print("\nYou unlock the door using your key!\n");
        remove_from_inventory(p, pos);
        delay(talkdelay-1000);
        combat(r, p);
        if(next_room != 0){
          prev_room = current_room;
          current_room = next_room;
          next_room = 0;
          return;
        }
        delay(1000);
        win();

      } else if (pos == 0) { //if the player does not have the key
        print("\nYou do not have the key, you need to find it somewhere.\n");
        delay(talkdelay/2);
        print("\n1. Leave\n");
        wait_for_option();
        room_exits(r, p);
      }
    }
    else if(option == 2){
      option = 0;
      print("\nYou decide to leave.");
      room_exits(r, p);
    }
    else{
      option = 0;
      goto re;
    }
  }
}


//Frans Ekberg & Chandadeep Kaur
int main(){
  init();
  room_init();
  player_init();
  clearsc();
  print("---------------------\n");
  print("-Life With Adventure-\n");
  print("---------------------\n");
  delay(2000);
  print("\nCreated by Frans Ekberg and Chandadeep Kaur.\n\n");
  delay(2000);
  print("\nPress Button 1 to continue.");
  while(get_btn() == 0){
    delay(10);
  }
  clearsc();

  print("\nYou play as an adventurer named ");
  print(p->name);
  print(".\nSee your hp on the screen on the board.");
  hp2display(p->hp);
  print("\n");
  print("All prompts with numbers in front of them are answered by toggling the corresponding switch.\n");
  print("Check your inventory by toggling switch 0.\n");
  delay(2000);
  print("\nPress Button 1 to continue.");
  while(get_btn() == 0){
    delay(10);
  }
  clearsc();
  /*delay(3000);
  set_hp(p, -10);
  print("\nPlayer hp: ");
  print_dec(p->hp);

  delay(2000);
  set_hp(p, -85);*/
  
  while(alive){
    room *r = &rooms[current_room]; //r pointer to the current room

    /* print("\nYou are in ");
    print(r->name); */
    clearsc();
    print(r->intro);
    print(".\n\n");

    room_actions(r, p);
  }
  
  return 0;
}

//Frans Ekberg
void reset(void){
  
  mytime = 0x0000;
  hourtime = 0;
  timeoutcount = 0;
  option = 0;
  alive = 1;
  in_combat = 0;
  current_room = 1;
  next_room = 0;
  prev_room = 0;
  main();
}
