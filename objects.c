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

    if(simulation_data->debug_msgs){
        printf("| DEBUG : new obj \"%s\" initialized\n", new_obj->Name_ID);
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
        if(simulation_data->debug_msgs){
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

void move_object(Object* object, Room* room) {
    if (object == NULL) {
        perror("| ERROR : Cannot move NULL object");
    }
    if (room == NULL) {
        perror("| ERROR : Cannot move object to NULL room");
    }

    // Vérification de la possibilité de déplacement
    bool possible = false;
    for (int i = 0; i < ant->Position->connexion_list_size; i++) {
        if (ant->Position->connexion_list == room) {
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
    for (int i = 0; i < object->Position->obj_count; i++) {
        if (object->Position->Obj_list[i] == object) {
            object->Position->Obj_list[i] = object->Position->Obj_list[--obj_count];
            object->Position->Obj_list = realloc(object->Position->Obj_list, object->Position->obj_count * sizeof(Object*));
        }
    }

    // on ajoute la fourmi à la nouvelle salle
    room->Obj_list = realloc(room->Obj_list, ++room->obj_count);
    room->Obj_list[obj_count-1] = object;
    object-> Position = room;

    // on déplace l'objet que porte la fourmi
    if (ant->Held_object != NULL) {
        move_object(ant->Held_object, room);
    }

}
