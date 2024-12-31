#include <stdlib.h>
#include <stdio.h>
#include "simulation.h"


/* -----< Main >----- */
int main(){
    printf("\n\n \b"); // Pour éviter les problèmes de scanf, peut etre supprimé

    // Simulation
    Simulation_data* simulation_1 = init_simulation(); 
    init_variables(simulation_1);
    simulation_1->debug_msgs = 1;
    init_seasons(simulation_1, simulation_1->start_season);

    // Début de la simulation
    Nest* nest = start(simulation_1);
    printf("| Simulation démarée\n");

    // Initialisation d'une reine
    Egg* queen = init_new_egg(simulation_1, nest, "Queen1", 0, nest->Queen_chamber);
    nest->Queen_chamber->Egg_list = realloc(nest->Queen_chamber->Egg_list, (++nest->Queen_chamber->Egg_count)*sizeof(Egg*));
    nest->Queen_chamber->Egg_list[nest->Queen_chamber->Egg_count-1] = queen;

    // Simulation

    while(1){
        simulation_choice(simulation_1);
        if(simulation_1->debug_msgs >= 1){print_numbers(simulation_1);}
    }

    // Fin de la simulation
    printf("| Simulation finie\n");
    fin(simulation_1);

    return 0;
}
