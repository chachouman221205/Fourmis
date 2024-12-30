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
