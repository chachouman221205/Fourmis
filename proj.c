#include <stdlib.h>
#include <stdio.h>
#include "simulation.h"


/* -----< Main >----- */
int main(){
    printf("\n\n \b"); // Pour éviter les problèmes de scanf, peut etre supprimé

    // Simulation
    Simulation_data* simulation_1 = malloc(sizeof(Simulation_data));
    init_variables(simulation_1);
    simulation_1->debug_msgs = true;

    init_seasons(simulation_1, simulation_1->start_season);

    // Début de la simulation
    start(simulation_1);
    printf("| Simulation démarée\n");

    // Simulation
    while(1){
        print_numbers(simulation_1);
        simulation_choice(simulation_1);
    }

    // Fin de la simulation
    printf("| Simulation finie\n");
    fin(simulation_1);

    return 0;
}
