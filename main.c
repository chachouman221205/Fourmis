#include <stdlib.h>
#include <stdio.h>
#include "simulation.h"

/* Simulation debug messages :
    0 : No msgs
    1 : ant & larve & egg & creature (init + free + evolution), print_numbers
    2 : 1+ environment & seasons (init + free), iterations left
    3 : 2+ room (init + free + connections)
    4 : 3+ ant & larve & egg & creature (death + pv)
    5 : 4+ food (creation + free)
    6 : 5+ evolution (time_left)
    7 : 6+ evolution (test), room (reset)
    8 : 7+ room connexion (init + free)
*/

/* -----< Main >----- */
int main(){
    printf("\n\n \b"); // Pour éviter les problèmes de scanf, peut etre supprimé

    // Simulation
    Simulation_data* simulation_1 = init_simulation(); 
    init_variables(simulation_1);
    simulation_1->debug_msgs = 4;   // Niveau de debug
    simulation_1->pause = 2;        // En seconde
    init_seasons(simulation_1, simulation_1->start_season);

    // Début de la simulation
    Nest* nest = start(simulation_1);
    printf("| Simulation démarée\n");

    // Initialisation de la food
    simulation(simulation_1, 1);
    simulation_1->tick--;

    // Initialisation d'une Queen
    Egg* queen = init_new_egg(simulation_1, nest, "Queen_ORIGIN", 0, nest->Queen_chamber);
    nest->Queen_chamber->Egg_list = realloc(nest->Queen_chamber->Egg_list, (++nest->Queen_chamber->Egg_count)*sizeof(Egg*));
    nest->Queen_chamber->Egg_list[nest->Queen_chamber->Egg_count-1] = queen;
    nest->Egg_list = realloc(nest->Egg_list, (++nest->Egg_number)*sizeof(Egg*));
    nest->Egg_list[nest->Egg_number-1] = queen;
    queen->Grow = 1;

    // Initialisation d'une Worker
    Egg* worker = init_new_egg(simulation_1, nest, "Worker_ORIGIN", 1, nest->Queen_chamber);
    nest->Queen_chamber->Egg_list = realloc(nest->Queen_chamber->Egg_list, (++nest->Queen_chamber->Egg_count)*sizeof(Egg*));
    nest->Queen_chamber->Egg_list[nest->Queen_chamber->Egg_count-1] = worker;
    nest->Egg_list = realloc(nest->Egg_list, (++nest->Egg_number)*sizeof(Egg*));
    nest->Egg_list[nest->Egg_number-1] = worker;
    worker->Grow = 1;

    // Simulation
    if(simulation_1->debug_msgs >= 1){print_numbers(simulation_1);}
    while(1){
        simulation_choice(simulation_1);
        // printf("Hunger : queen %d %d ; %p | worker %d %d ; %p\n", queen->Hunger, queen->Ant_type, &queen->Ant_type, worker->Hunger, worker->Ant_type, &worker->Ant_type);
        if(simulation_1->debug_msgs == -1){print_numbers(simulation_1);}
    }

    // Fin de la simulation
    printf("| Simulation finie\n");
    fin(simulation_1);

    return 0;
}
