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

#endif