player* new_player(int id ,char name[], int age);
int setP_hp(player* p, int change);
void print_inventory(player* p);
int add_to_inventory(player* p, char* item);
char* remove_from_inventory(player* p, int option);
int peek_inventory(player* p, char* item);
void check_damage(player* p);