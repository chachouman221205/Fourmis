#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    new_room->Larve_list = malloc(0);
    new_room->Larve_count = 0;
    new_room->Egg_list = malloc(0);
    new_room->Egg_count = 0;
    new_room->Obj_list = malloc(0);
    new_room->Obj_count = 0;
    new_room->Creature_list = malloc(0);
    new_room->Creature_count = 0;
    new_room->Connexion_list = malloc(0);
    new_room->Connexion_list_size = 0;

    new_room->Pheromone_stack = malloc(0);

    if(simulation_data->debug_msgs >= 3){
        printf("| DEBUG : new room \"%s\" %p initialized\n", new_room->Name_ID, new_room);
    }

    return new_room;
}

void connect_rooms(Simulation_data* simulation_data, Room* room1, Room* room2) {

    if (room1 == NULL || room2 == NULL) {
        printf("ERROR : attempting connection to a non existing room (\"%s\" - \"%s\")", room1->Name_ID, room2->Name_ID);
        exit(1);
    }

    if (room1 == room2) {
        return; // Inutile de connecter une pièce à elle-même
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

    if(room1->Connexion_list == NULL || room2->Connexion_list == NULL){
        perror("Échec de l'allocation mémoire pour la connexion entre pièces");
        exit(1);
    }

    // Ajouter une connection
    room1->Connexion_list[room1->Connexion_list_size-1] = room2;
    room2->Connexion_list[room2->Connexion_list_size-1] = room1;

    if(simulation_data->debug_msgs >= 8){
        printf("| DEBUG : new connection between room \"%s\" and room \"%s\"\n", room1->Name_ID, room2->Name_ID);
    }
}

void disconnect_rooms(Room* room1, Room* room2) {
    // on retire la connection de room1 à room2
    for (int j = 0; j < room1->Connexion_list_size; j++) {
        if (room1->Connexion_list[j] == room2) {
            room1->Connexion_list[j] = room1->Connexion_list[--room1->Connexion_list_size];
            room1->Connexion_list = realloc(room1->Connexion_list, (room1->Connexion_list_size) * sizeof(Room*));
        }
    }

    // on retire la connection de room2 à room1
    for (int j = 0; j < room2->Connexion_list_size; j++) {
        if (room2->Connexion_list[j] == room1) {
            room2->Connexion_list[j] = room2->Connexion_list[--room2->Connexion_list_size];
            room2->Connexion_list = realloc(room2->Connexion_list, (room2->Connexion_list_size) * sizeof(Room*));
        }
    }
}

void free_room(Simulation_data* simulation_data, Room* room){
    if(room != NULL){
        // On retire les connections avec les autres Rooms
        Room* R2;
        for (int i = 0; i < room->Connexion_list_size; i++) {
            R2 = room->Connexion_list[i];
            // chercher la connection
            for (int j = 0; j < R2->Connexion_list_size; j++) {
                if (R2->Connexion_list[j] == room) {

                    R2->Connexion_list[j] = R2->Connexion_list[--(R2->Connexion_list_size)]; // On retire la connection
                    R2->Connexion_list = realloc(R2->Connexion_list, R2->Connexion_list_size);
                    if (R2->Connexion_list == NULL) {
                        perror("échec de l'allocation pour les connections de Room");
                    }
                }
            }
        }
        free(room->Connexion_list);

        simulation_data->room_NB--;

        free_all_pheromones(room->Pheromone_stack);

        // free Ants
        for (int i = 0; i < room->Ant_count; i++) {
            free_ant(simulation_data, room->Ant_list[i]);
        }
        free(room->Ant_list);

        // free Larve
        for (int i = 0; i < room->Larve_count; i++) {
            free_larve(simulation_data, room->Larve_list[i]);
        }
        free(room->Larve_list);

        // free Egg
        for (int i = 0; i < room->Egg_count; i++) {
            free_egg(simulation_data, room->Egg_list[i]);
        }
        free(room->Egg_list);

        // free Objects
        for (int i = 0; i < room->Obj_count; i++) {
            free_object(simulation_data, room->Obj_list[i]);
        }
        free(room->Obj_list);

        /* Creatures not added
        // free Creatures
        for (int i = 0; i < room->Creature_count; i++) {
            free_creature(simulation_data, room->Creature_list[i]);
        }
        free(room->Creature_list);*/

        if(simulation_data->debug_msgs >= 3){
            printf("\033[1;31m| DEBUG : room \"%s\" %p freed\n\033[0m", room->Name_ID, room);
        }
        free(room);
    }
}
void free_room_rec(Simulation_data* simulation_data, Room* room) {
    if (room != NULL && !room->Visited) {

        if (simulation_data->debug_msgs >= 3) {
            printf("| DEBUG : freeing room \"%s\" %p recursively\n", room->Name_ID, room);
        }

        room->Visited = true;
        for (int i = 0; room->Connexion_list_size-i > 0; i++) {
            free_room_rec(simulation_data, room);
        }
        free_room(simulation_data, room);
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

    /* Creatures not added
    // Creatures
    for (int i = 0; i < room->Creature_count; i++) {
        if (room->Creature_list[i] != NULL) {
            space_used += room->Creature_list[i]->Size;
        }
    }
    */

    return room->Size - space_used;
}

void reinitialiser_rooms(Simulation_data* simulation_data, Room* room) {
    if (!room->Visited) {
        return;
    }
    if(simulation_data->debug_msgs >= 7){
        printf("\033[1;35m| DEBUG : room \"%s\" re-initialized\n\033[0m", room->Name_ID);
    }
    room->Visited = false;
    for (int i = 0; i < room->Connexion_list_size; i++) {
        reinitialiser_rooms(simulation_data, room->Connexion_list[i]);
    }
}

void free_Path(Path* p) {
    if (p->next != NULL) {
        free_Path(p->next);
    }
    free(p);
}

void use_path(Path* p) {
    Path* second = p->next;
    p->next = second->next;
    p->room = second->room;
    p->length = second->length;
    free(second);
}

Path* find_path_to_food(Room* start, bool entry_blocked) {
    if (start == NULL) {
        perror("| ERROR : Need a starting point for pathfinding");
    }

    if (start->Visited || (start->Name_ID && entry_blocked)) {
        return NULL;
    }

    Path* result = malloc(sizeof(Path));
    if (result == NULL) {
        perror("échec de l'allocation mémoire pour un chemin");
    }

    // Si un objet se trouve dans la salle actuelle
    if (search_object(start, "food") != NULL) {


        result->room = start;
        result->next = NULL;
        result->length = 0;

        return result;
    }

    // Liste de touts les chemins générés récursivement
    Path** paths= malloc(start->Connexion_list_size * sizeof(Path*));
    for (int i = 0; i < start->Connexion_list_size; i++) {
        paths[i] = find_path_to_food(start->Connexion_list[i], entry_blocked);
    }

    // recherche du chemin trouvé le plus court
    unsigned int min_distance = -1;
    result->next = NULL;
    result->room = start;
    for (int i = 0; i < start->Connexion_list_size; i++) {
        if (paths[i]->length < min_distance) {
            free_Path(result);
            result->next = paths[i];
            min_distance = result->length;

        } else if (paths[i] != NULL) {
            free_Path(paths[i]);
        }
    }
    return result;
}

Path* find_path_to_name(Room* start, char* NameID, bool entry_blocked) {
    if (start == NULL) {
        perror("| ERROR : Need a starting point for pathfinding");
    }

    if (start->Visited || (start->Name_ID && entry_blocked)) {
        return NULL;
    }

    Path* result = malloc(sizeof(Path));
    if (result == NULL) {
        perror("échec de l'allocation mémoire pour un chemin");
    }

    // Si un objet se trouve dans la salle actuelle
    if (strcmp(start->Name_ID, NameID)) {

        result->room = start;
        result->next = NULL;
        result->length = 0;

        return result;
    }

    Path** paths= malloc(start->Connexion_list_size * sizeof(Path*));
    for (int i = 0; i < start->Connexion_list_size; i++) {
        paths[i] = find_path_to_food(start->Connexion_list[i], entry_blocked);
    }

    unsigned int min_distance = -1;
    result->next = NULL;
    result->room = NULL;
    for (int i = 0; i < start->Connexion_list_size; i++) {
        if (paths[i]->length < min_distance) {
            free_Path(result);
            result = paths[i];
            min_distance = result->length;

        } else if (paths[i] != NULL) {
            free_Path(paths[i]);
        }
    }

    return result;
}
