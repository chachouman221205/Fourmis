#ifndef ANTS_H
#define ANTS_H

#include "simulation.h"


// Représente un oeuf de fourmi
typedef struct Egg {
    char *Name_ID;
    int Ant_type;

    int PV;

    int Grow;       // nombre de ticks avant que l'oeuf passe au stade supérieur
    int Hunger;     // l'oeuf n'utilise pas "hunger" mais stocke l'information


    char *Clan;
    Nest* Nest;
    struct Room *Position;      // Pointeur vers la pièce où se trouve l'oeuf
} Egg;

// Représente une larve de fourmi
typedef struct Larve {
    char *Name_ID;
    int Ant_type;

    int PV;

    int Grow;       // nombre de ticks avant que la larve devienne une fourmi
    int Hunger;     // Nombre d'itérations avant que la larve ait besoin de manger, meurt si <= -5
    // La larve a faim quand hunger = 0, et doit manger en moins de 5 itérations pour survivre

    char *Clan;
    Nest* Nest;
    struct Room *Position;      // Pointeur vers la pièce où se trouve la larve
} Larve;

// Représente une fourmi
typedef struct Ant {
    char *Name_ID;
    int Ant_type;

    int PV;
    int DMG;

    int Life;       // Itérations restantes pour la fourmis, elle meurt quand Life = 0
    int Hunger;     // Nombre d'itérations avant que la fourmi ait besoin de manger, meurt si <= -5
    // La fourmi a faim quand hunger = 0, et doit manger en moins de 5 itérations pour survivre

    char *Clan;
    struct Nest* Nest;

    struct Room *Position;      // Pointeur vers la pièce où se trouve la fourmi
    struct Object *Held_object; // Pointeur vers l'objet que la fourmi transporte (NULL si aucun)
    struct Pheromone *Action;   // Pointeur vers la phéromone qui détermine son objectif
    struct Path *Path;          // Objectif actuel de la fourmi
} Ant;

// Egg
Egg* init_new_egg(Simulation_data* simulation_data, Nest* nest, char *name, int ant_type, Room* room);

void free_egg(Simulation_data* simulation_data, Egg* egg);

void test_kill_egg(Simulation_data* simulation_data, Egg* egg);


bool test_grow_egg(Simulation_data* simulation_data, Egg* egg);

// Larve
Larve* init_new_larve(Simulation_data* simulation_data, Egg* egg);

void free_larve(Simulation_data* simulation_data, Larve* larve);

void test_kill_larve(Simulation_data* simulation_data, Larve* larve);

bool test_grow_larve(Simulation_data* simulation_data, Larve* larve);

// Ants
void attach_ant_to_nest(Ant* ant, Nest* nest);

Ant* init_new_ant(Simulation_data* simulation_data, Larve* larve);

bool move_ant(Simulation_data* simulation_data, Ant* ant, Room* room);
void use_path(Ant* ant);


void Action_ant(Simulation_data* simulation_data, Ant* ant);

Ant* search_AntID(char* AntID, Exterior* Exterior);

void free_ant(Simulation_data* simulation_data, Ant* ant);

void test_kill_ant(Simulation_data* simulation_data, Ant* ant);

void combat_ants(Simulation_data* simulation_data, Ant* ant1, Ant* ant2);

int total_size(Ant* ant);


#endif
