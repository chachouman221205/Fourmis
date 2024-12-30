#ifndef ROOMS_H
#define ROOMS_H

#include "simulation.h"

// Représente une pièce dans l'environnement
typedef struct Room {
    char *Name_ID;
    bool Visited;   // Pour l'exploration du graphe (graphe d'objets "Room")
    int Size;       // Taille max de la Room, limite d'objet et de fourmis pouvant entrer

    struct Ant **Ant_list;
    int Ant_count;

    struct Object **Obj_list;
    int Obj_count;

    struct Egg** Egg_list;
    int Egg_count;

    struct Creature **Creature_list;
    int Creature_count;

    struct Room **Connexion_list;
    int Connexion_list_size;

    struct Pheromone *Pheromone_stack;
} Room;

typedef struct Path {
    struct Room* room;
    struct Path* next;
    int length;
} Path;


void connect_rooms(Room* room1, Room* room2);
void disconnect_rooms(Room* room1, Room* room2);

Room* init_room(Simulation_data* simulation_data, char* name_ID, int size);
void free_room(Simulation_data* simulation_data, Room* room);
void free_room_rec(Simulation_data* simulation_data, Room* room);

int remaining_space(Room* room);

void reinitialiser_rooms(Simulation_data* simulation_data, Room* room);


void free_Path(Path* p);
void use_path(Path* p);

Path* find_path_to_food(Room* start, bool entry_blocked);
Path* find_path_to_name(Room* start, char* NameID, bool entry_blocked);

#endif
