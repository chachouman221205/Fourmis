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

    struct Object **Creature_list;
    int Creature_count;

    struct Room **Connexion_list;
    int Connexion_list_size;

    struct Pheromone *Pheromone_stack;
} Room;

Room* init_room(Simulation_data* simulation_data, char* name_ID, int size);

void connect_rooms(Room* room1, Room* room2);

void disconnect_rooms(Room* room1, Room* room2);

void free_room(Simulation_data* simulation_data, Room* room);

void free_room_rec(Simulation_data* simulation_data, Room* room);

int remaining_space(Room* room);

void reinitialiser_rooms(Simulation_data* simulation_data, Room* room);


#endif
