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

    struct Ant **All_Ant_list;
    int Total_Ant_number;
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

    struct Ant** Ant_list;
    int Ant_number;
} Nest;


// Représente une phéromone (stack)
typedef struct Pheromone {
    char *Action;   // Type ou action associée à la phéromone (pour la compréhension)
    int Density;    // Densité de la phéromone (indique sa force)
    int ph_ID;

    struct Pheromone* Next;
} Pheromone;



// Représente les saisons (liste chainée bouclée)
typedef struct Season {
    char *Name;
    int Number;               // Numéro de la saison (0 à 3)
    struct Season *Next;
} Season;

typedef struct Simulation_data {
    int tick;           // Temps actuel dans la simulation
    int start_season;
    int counter;        // On change de saison et counter = 0 quand counter = 30;
    int current_season;
    struct Season* season_chain;

    // IDs
    int room_IDs;
    int egg_IDs;        // IDs++ à chaque nouvel(le) room/ant/object/predator, permet de ne jamais avoir 2 fois le meme id
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

    bool debug_msgs; // Printf de messages si = 1, pour le debug
} Simulation_data;


// Pre-déclaration
typedef struct Room Room;


/* -----< Récupération des variables de départ >----- */
void init_variables(Simulation_data* simulation);

// Seasons
void init_seasons(Simulation_data* simulation_data, int start_season);

void free_seasons(Season* season);

// Nest
Nest* init_nest(Simulation_data* simulation_data, char* specie, char* clan, int* pv, int* dmg, int life_min, int life_max, int hunger, Room* entry, Exterior* Exterior);

void free_nest(Simulation_data* simulation_data, Nest* nest);

// Exterior
Exterior* init_exterior(Simulation_data* simulation_data, int size);

void free_exterior(Exterior* exterior);
// Rooms



// Display
void print_numbers(Simulation_data* sim);


/* -----< Simulation >----- */
/*
 I *ci sont gérées les étapes de la simulation, init et itérations
 */

void simuler_room(Simulation_data* simulation_data, Room* room);


void simulation(Simulation_data* simulation_data, Exterior* exterior, int iterations);

/* -----< Initialisation de la simulation >----- */
void start(Simulation_data* simulation_data, Nest** nest, Exterior** exterior);

#endif
