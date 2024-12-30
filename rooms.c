#include <stdlib.h>
#include <stdio.h>

#include "simulation.h"

// Rooms
Room* init_room(Simulation_data* simulation_data, char* name_ID, int size){
    Room* new_room = malloc(sizeof(Room));
    if(new_room == NULL){
        perror("Échec de l'allocation mémoire pour la pièce");
        return NULL;
    }

    simulation_data->room_NB++;

    if (name_ID != NULL) {
        new_room->Name_ID = name_ID;
        simulation_data->room_IDs++;
    } else {
        sprintf(new_room->Name_ID, "Room%d", simulation_data->room_IDs++);
    }
    new_room->Visited = false;  // pas visité à l'initialisation
    new_room->Size = size;
    new_room->Ant_list = malloc(0);
    new_room->Ant_count = 0;
    new_room->Obj_list = malloc(0);
    new_room->Obj_count = 0;
    new_room->Creature_list = malloc(0);
    new_room->Creature_count = 0;
    new_room->Connexion_list = malloc(0);
    new_room->Connexion_list_size = 0;

    new_room->Pheromone_stack = NULL;

    if(simulation_data->debug_msgs){
        printf("| DEBUG : new room \"%s\" initialized\n", new_room->Name_ID);
    }

    return new_room;
}

void connect_rooms(Room* room1, Room* room2) {

    if (room1 == NULL || room2 == NULL) {
        printf("ERROR : attempting connection to a non existing room (\"%s\" - \"%s\")", room1->Name_ID, room2->Name_ID);
        exit(1);
    }

    if (room1 == room2) {
        printf("ERROR : Can't connect a room to itself: \"%s\"", room1->Name_ID);
        exit(1);
    }

    // Verifier si les salles sont déjà connectées
    int existing_connection_status = 0;
    for (int i = 0; i < room1->Connexion_list_size; i++) {
        if (room1->Connexion_list[i] == room2) {
            existing_connection_status++;
            break;
        }
    }
    for (int i = 0; i < room2->Connexion_list_size; i++) {
        if (room2->Connexion_list[i] == room1) {
            existing_connection_status++;
            break;

        }
    }


    if (existing_connection_status == 2) {
        return; // Les deux rooms sont déjà connectés
    }
    if (existing_connection_status == 1) {
        printf("ERROR : room \"%s\" and room \"%s\" have a bad connection", room1->Name_ID, room2->Name_ID);
        exit(1);
    }

    // Ajustement de la taille de chaque tableau
    room1->Connexion_list_size++;
    room1->Connexion_list = realloc(room1->Connexion_list, room1->Connexion_list_size * sizeof(Room*));
    room2->Connexion_list_size++;
    room2->Connexion_list = realloc(room2->Connexion_list, room2->Connexion_list_size * sizeof(Room*));

    if(room1->Connexion_list == NULL || room2->Connexion_list){
        perror("Échec de l'allocation mémoire pour la connexion entre pièces");
        exit(1);
    }

    // Ajouter une connection
    room1->Connexion_list[room1->Connexion_list_size-1] = room2;
    room2->Connexion_list[room2->Connexion_list_size-1] = room1;
}

void free_room(Simulation_data* simulation_data, Room* room){
    if(room != NULL){
        if(simulation_data->debug_msgs){
            printf("| DEBUG : room \"%s\" freed\n", room->Name_ID);
        }

        // On retire les connections avec les autres R2ooms
        Room* R2;
        for (int i = 0; i < room->Connexion_list_size; i++) {
            R2 = room->Connexion_list[i];
            // chercher la connection
            for (int j = 0; j < R2->Connexion_list_size; j++) {
                if (R2->Connexion_list[j] == room) {
                    R2->Connexion_list[j] = R2->Connexion_list[--R2->Connexion_list_size]; // On retire la connection
                    R2->Connexion_list = realloc(R2->Connexion_list, R2->Connexion_list_size);
                }
            }
        }

        free(room->Ant_list);
        free(room->Obj_list);
        free(room->Creature_list);
        free(room->Connexion_list);
        free(room);
    }
}

int remaining_space(Room* room) {
    int space_used = 0;
    // Objects
    for (int i = 0; i < room->Obj_count; i++) {
        if (room->Obj_list[i] != NULL) {
            space_used += room->Obj_list[i]->Size;
        }
    }

    // Ants
    space_used += room->Ant_count;

    // Creatures
    for (int i = 0; i < room->Creature_count; i++) {
        if (room->Creature_list[i] != NULL) {
            space_used += room->Creature_list[i]->Size;
        }
    }

    return room->Size - space_used;
}

void reinitialiser_rooms(Simulation_data* simulation_data, Room* room) {
    if (!room->Visited) {
        return;
    }
    room->Visited = false;
    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(simulation_data, room->Connexion_list[i]);
    }
}