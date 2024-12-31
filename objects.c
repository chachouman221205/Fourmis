#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "simulation.h"


// Objects
Object* init_object(Simulation_data* simulation_data, char* name_ID, int size, bool held){
    Object* new_obj = malloc(sizeof(Object));
    if(new_obj == NULL){
        perror("Échec de l'allocation mémoire pour l'objet");
        return NULL;
    }

    simulation_data->obj_NB++;

    if (name_ID != NULL) {
        new_obj->Name_ID = name_ID;
        simulation_data->obj_IDs++;
    } else {
        sprintf(new_obj->Name_ID, "Obj%d", simulation_data->obj_IDs++);
    }
    new_obj->Size = size;
    new_obj->Held = held;

    if(simulation_data->debug_msgs >= 5){
        printf("\033[0;33m| DEBUG : new obj \"%s\" initialized\n\033[0m", new_obj->Name_ID);
    }

    return new_obj;
}

Object* search_object(Room* room, char* NameID) {
    for (int i = 0; i < room->Obj_count; i++) {
        if (strcmp(room->Obj_list[i]->Name_ID, NameID) == 0) {
            return room->Obj_list[i];
        }
    }
    return NULL;
}

void free_object(Simulation_data* simulation_data, Object* object){
    if(object != NULL){
        if(simulation_data->debug_msgs >= 5){
            printf("| DEBUG : object \"%s\" freed\n", object->Name_ID);
        }
        free(object);
        simulation_data->obj_NB--;
    }
}

void drop_object(Ant* ant) {
    if (ant == NULL) {
        perror("| ERROR : Cannot drop objet from NULL ant");
    }
    if (ant->Held_object == NULL) {
        return;
    }

    ant->Held_object->Held = false;
    ant->Held_object = NULL;
}

void pick_up(Ant* ant, Object* object) {
    if (ant == NULL) {
        perror("| ERROR : Cannot pickup objet on NULL ant");
    }
    if (object == NULL) {
        perror("| ERROR : Cannot pickup NULL object on ant");
    }

    if (ant->Held_object != NULL) {
        drop_object(ant);
    }
    ant->Held_object = object;
    object->Held = true;
}

void move_object(Object* object, Room* start, Room* end) {
    if (object == NULL) {
        perror("| ERROR : Cannot move NULL object");
    }
    if (start == NULL) {
        perror("| ERROR : Cannot move object from NULL room");
    }
    if (end == NULL) {
        perror("| ERROR : Cannot move object to NULL room");
    }

    // Vérification de la possibilité de déplacement
    bool possible = false;
    for (int i = 0; i < start->Connexion_list_size; i++) {
        if (start->Connexion_list[i] == end) {
            possible = true;
            break;
        }
    }
    if (!possible) {
        printf("| WARNING : trying to move object to unaccessible location : move cancelled\n");
        return;
    }

    // Déplacement :

    // on retire l'objet de l'ancienne salle
    for (int i = 0; i < start->Obj_count; i++) {
        if (start->Obj_list[i] == object) {
            start->Obj_list[i] = start->Obj_list[--start->Obj_count];
            start->Obj_list = realloc(start->Obj_list, start->Obj_count * sizeof(Object*));
        }
    }

    // on ajoute l'objet à la nouvelle salle
    end->Obj_list = realloc(end->Obj_list, (++end->Obj_count) * sizeof(Object*));
    end->Obj_list[end->Obj_count-1] = object;


}