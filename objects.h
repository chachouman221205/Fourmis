#ifndef OBJECTS_H
#define OBJECTS_H

#include "simulation.h"

// Représente un objet
typedef struct Object {
    char *Name_ID;

    int Size;       // Taille de l'objet
    bool Held;      // Indique si l'objet est transporté par une fourmi
} Object;

// Objects
Object* init_object(Simulation_data* simulation_data, char* name_ID, int size, bool held);

void free_object(Simulation_data* simulation_data, Object* object);

Object* search_object(Room* room, char* NameID);

void drop_object(Ant* ant);
void pick_up(Ant* ant, Object* object);

void move_object(Object* object, Room* start, Room* end);

#endif