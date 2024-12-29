#include <stdlib.h>
#include <stdio.h>
#include "simulation.h"


/* -----< Main >----- */
int main(){
    printf(" \b"); // Pour éviter les problèmes de scanf, peut etre supprimé

    // Simulation
    Simulation_data* simulation_1 = malloc(sizeof(Simulation_data));
    init_variables(simulation_1);
    init_seasons(simulation_1, simulation_1->start_season);

    // ...

    // start(simulation_1, );

    // ...

    free(simulation_1);
    // Fin de la simulation

    return 0;
}
