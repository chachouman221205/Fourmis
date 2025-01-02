/* -----< Structures Principales >----- */
/*
 * Pour plus d'explications sur chacun des paramètres, ce référer au document pdf "Rapport modélisation projet"
 * Seules les explications importantes sont en commentaire
 */


#include <stdbool.h>


#include "objects.h"
#include "ants.h"
#include "rooms.h"

#ifndef SIMULATION_H
#define SIMULATION_H





// Représente l'extérieur de l'environnement (graphe de "Room")
typedef struct Exterior {
    struct Nest** Nests;
    int Nest_number;
    struct Room *Entry;     // Pointeur vers l'entrée de l'environnement

    struct Ant **Ant_list;
    int Ant_number;
} Exterior;

// Représente une fourmilière (graphe de "Room")
typedef struct Nest {
    char *Specie;
    char *Clan;     // Identifiant du clan auquel appartient la fourmilière

    // Liste des paramètres des fourmis de cette nest
    int* PV;        // tableau des PVs en fonction du type de fourmi : {1, 5, 10}; 1 ouvrière, 5 garde, 10 reine
    int* DMG;       // comme pour les PVs
    int Life_min;   // random pour plus de diversité
    int Life_max;
    int Hunger;

    struct Room* Entry;     // Pointeur vers la pièce d'entrée de la fourmilière
    struct Exterior* Exterior;
    struct Room* Queen_chamber;

    struct Ant** Ant_list;
    int Ant_number;
    struct Larve** Larve_list;
    int Larve_number;
    struct Egg** Egg_list;
    int Egg_number;
} Nest;


// Représente une phéromone (stack)
typedef struct Pheromone {
    char *Action;   // Type ou action associée à la phéromone (pour la compréhension)
    int Density;    // Densité de la phéromone (indique sa force)
    int ph_ID;

    struct Pheromone* next;
} Pheromone;



// Représente les saisons (liste chainée bouclée)
typedef struct Season {
    char *Name;
    int Number;               // Numéro de la saison (0 à 3)
    int Chance;
    struct Season *Next;
} Season;

typedef struct Simulation_data {
    int tick;           // Temps actuel dans la simulation

    int pause;
    bool pause_enable;
    char* space_tab[4];

    // Seasons
    int start_season;
    int counter;        // On change de saison et counter = 0 quand counter = 30;
    int current_season;
    struct Season* season_chain;
    struct Exterior* Exterior;

    // IDs
    int room_IDs;
    int egg_IDs;        // IDs++ à chaque nouvel(le) room/ant/object/predator, permet de ne jamais avoir 2 fois le meme id
    int queen_IDs;
    int obj_IDs;
    int crea_IDs;

    // Counts
    int nest_NB;
    int room_NB;
    int egg_NB;
    int larve_NB;
    int ant_NB;
    int obj_NB;
    int crea_NB;


    int debug_msgs;
} Simulation_data;

/* Simulation messages :
    0 : No msgs
    1 : ant & larve & egg & creature (init + free + evolution), print_numbers
    2 : 1+ environment & seasons (init + free), iterations left
    3 : 2+ room (init + free + connections)
    4 : 3+ ant & larve & egg & creature (death + pv)
    5 : 4+ food (creation + free)
    6 : 5+ evolution (test + time_left)
    7 : 6+ room (reset)
    8 : 7+ room connexion (init + free)
*/

// Pre-déclaration
typedef struct Room Room;


/* -----< Récupération des variables de départ >----- */
void init_variables(Simulation_data* simulation);

// Seasons
void init_seasons(Simulation_data* simulation_data, int start_season);

void free_seasons(Simulation_data* simulation_data, Season* season);

// Nest
Nest* init_nest(Simulation_data* simulation_data, char* specie, char* clan, int* pv, int* dmg, int life_min, int life_max, int hunger, Room* entry);

void free_nest(Simulation_data* simulation_data, Nest* nest);

// Exterior
Exterior* init_exterior(Simulation_data* simulation_data, int size);

void free_exterior(Simulation_data* simulation_data, Exterior* exterior);

// Pheromones
Pheromone* init_pheromone(char *action, int density, int ID);
void free_all_pheromones(Pheromone *stack);

Pheromone* get_first_pheromone(Pheromone **stack);
void insert_pheromone(Pheromone **stack, Pheromone *new_pheromone);



// Display
void print_numbers(Simulation_data* sim);


/* -----< Simulation >----- */
/*
Ici sont gérées les étapes de la simulation, init et itérations
*/

void simuler_room(Simulation_data* simulation_data, Room* room);

void simulation(Simulation_data* simulation_data, int iterations);

void simulation_choice(Simulation_data* simulation_data);

/* -----< Initialisation de la simulation >----- */
Simulation_data* init_simulation();

Nest* start(Simulation_data* simulation_data);

void fin(Simulation_data* simulation_data);

#endif
