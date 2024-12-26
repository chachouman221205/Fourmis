#include <stdio.h>
#include <stdbool.h>

typedef struct Room Room;
struct Room {
    char name;
    Room* entry;
    bool visited;
    int size;
    Ant* ant_list;
    int ant_count;
    Object* obj_list;
    int obj_count;
    Room* connexion_list;
    int connexion_list_size;
    Pheromone* pheromone_stack;
};

typedef struct Creature Creature;
struct Creature
{
	char name;
	int pv;
	int dmg;
	int age;
	Room* position;
};

typedef struct Pheromone Pheromone;
struct Pheromone
{
	char action;
	int density;
	
};

typedef struct Object Object;
struct Object
{
	char name;
	int size;
	bool held;
	
};

typedef struct Ant Ant;
struct Ant
{
	char name;
	int pv;
	int dmg;
	int hunger;
	int age;
	char clan;
	Room* position;
	Object* held_obj;
	Pheromone* action;
	
};

typedef struct Nest Nest;
struct Nest
{
	char species;
	char clan;
	Room* entrance;
	Ant* ant_list;
	int ant_number;
};

typedef struct Exterior Exterior;
struct Exterior
{
	Room* entry;
	Ant* ant_list;
	int ant_number;
	
};

int main() {
    return 0;
}

