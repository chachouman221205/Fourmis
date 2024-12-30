#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulation.h"

// Egg
Egg* init_new_egg(Simulation_data* simulation_data, Nest* nest, char *name, int ant_type, Room* room) {
    Egg* new_egg = malloc(sizeof(Egg));
    if(new_egg == NULL){
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    simulation_data->egg_NB++;

    // Initialisation des champs de l'oeuf, on initialise en fonction de la nest
    if (name != NULL) {
        new_egg->Name_ID = name;
    } else {
        sprintf(new_egg->Name_ID, "Ant%d", simulation_data->egg_IDs++);
    }
    new_egg->Ant_type = ant_type;
    new_egg->PV = 1;
    new_egg->Grow = (nest->Life_min + rand() % (nest->Life_max - nest->Life_min + 1))/2; // Grow entre (Life_min et Life_max)/2
    new_egg->Hunger = nest->Hunger;
    new_egg->Nest = nest;
    new_egg->Clan = nest->Clan;
    new_egg->Position = room;     // Position NULL au départ, assignation plus tard

    if(simulation_data->debug_msgs){
        printf("| DEBUG : new egg \"%s\" initialized in nest \"%s\"\n", new_egg->Name_ID, nest->Clan);
    }

    return new_egg;
}

void free_egg(Simulation_data* simulation_data, Egg* egg){
    if(egg != NULL){
        if(simulation_data->debug_msgs){
            printf("| DEBUG : egg \"%s\" freed\n", egg->Name_ID);
        }
        free(egg);
        simulation_data->egg_NB--;
    }
}

void test_kill_egg(Simulation_data* simulation_data, Egg* egg){
    if(egg != NULL){
        char* death_message[] = {"PV <= %d", "Hunger <= %d"};
        int condition = 0;

        if(egg->PV <= 0){
            condition = 1;
        }
        if(egg->Hunger <= -5){
            condition = 2;
        }

        if(condition != 0){
            if(simulation_data->debug_msgs){
                printf("| DEBUG : egg \"%s\" died : ", egg->Name_ID);
                printf(death_message[condition-1], (condition == 1)? egg->PV : egg->Hunger);
            }
            free_egg(simulation_data, egg);
        }
    }

}


bool test_grow_egg(Simulation_data* simulation_data, Egg* egg){
    if(egg != NULL){
        if(egg->Grow <= 0){
            if(simulation_data->debug_msgs){
                printf("| DEBUG : egg \"%s\" can evolve", egg->Name_ID);
            }
            return true;
        }

        if(simulation_data->debug_msgs){
            printf("| DEBUG : egg \"%s\" cannot evolve", egg->Name_ID);
        }
        return false;
    }
    return false;
}

// Larve
Larve* init_new_larve(Simulation_data* simulation_data, Egg* egg) {
    Larve* new_larve = malloc(sizeof(Larve));
    if(new_larve == NULL){
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    // Initialisation des champs de la larve, on initialise en fonction de l'oeuf
    new_larve->Name_ID = egg->Name_ID;
    simulation_data->larve_NB++;

    new_larve->Ant_type = egg->Ant_type;
    new_larve->PV = egg->Nest->PV[new_larve->Ant_type]/2;
    new_larve->Grow = (egg->Nest->Life_min + rand() % (egg->Nest->Life_max - egg->Nest->Life_min + 1))/2; // Grow entre (Life_min et Life_max)/2
    new_larve->Hunger = egg->Nest->Hunger;
    new_larve->Clan = egg->Nest->Clan;
    new_larve->Nest = egg->Nest;
    new_larve->Position = egg->Position;     // Position NULL au départ, assignation plus tard

    if(simulation_data->debug_msgs){
        printf("| DEBUG : new larve \"%s\" initialized in nest \"%s\"\n", new_larve->Name_ID, egg->Nest->Clan);
    }
    free_egg(simulation_data, egg);
    return new_larve;
}

void free_larve(Simulation_data* simulation_data, Larve* larve){
    if(larve != NULL){
        if(simulation_data->debug_msgs){
            printf("| DEBUG : larve \"%s\" freed\n", larve->Name_ID);
        }
        free(larve);
        simulation_data->larve_NB--;
    }
}

void test_kill_larve(Simulation_data* simulation_data, Larve* larve){
    if(larve != NULL){
        char* death_message[] = {"PV <= %d", "Hunger <= %d"};
        int condition = 0;

        if(larve->PV <= 0){
            condition = 1;
        }
        if(larve->Hunger <= -5){
            condition = 2;
        }

        if(condition != 0){
            if(simulation_data->debug_msgs){
                printf("| DEBUG : larve \"%s\" died : ", larve->Name_ID);
                printf(death_message[condition-1], (condition == 1)? larve->PV : larve->Hunger);
            }
            free_larve(simulation_data, larve);
        }
    }

}

bool test_grow_larve(Simulation_data* simulation_data, Larve* larve){
    if(larve != NULL){
        if(larve->Grow <= 0){
            if(simulation_data->debug_msgs){
                printf("| DEBUG : larve \"%s\" can evolve", larve->Name_ID);
            }
            return true;
        }

        if(simulation_data->debug_msgs){
            printf("| DEBUG : larve \"%s\" cannot evolve", larve->Name_ID);
        }
        return false;
    }
    return false;
}

// Ants

void attach_ant_to_nest(Ant* ant, Nest* nest) {
    // On retire les liens avec l'ancienne colonie
    if (ant->Nest != NULL) {
        for (int i = 0; i < ant->Nest->Ant_number; i++) {
            if (ant->Nest->Ant_list[i] == ant) {
                ant->Nest->Ant_list[i] = ant->Nest->Ant_list[--ant->Nest->Ant_number];
            }
        }
    }

    // On établi le lien avec la nouvelle colonie
    ant->Nest = nest;
    ant->Clan = nest->Clan;
    nest->Ant_list = realloc(nest->Ant_list, (++nest->Ant_number) * sizeof(Ant*));
    nest->Ant_list[nest->Ant_number-1] = ant;
}

Ant* init_new_ant(Simulation_data* simulation_data, Larve* larve) {
    Ant* new_ant = malloc(sizeof(Ant));
    if(new_ant == NULL){
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    // Initialisation des champs de la fourmi, on initialise en fonction de la larve
    new_ant->Name_ID = larve->Name_ID;
    simulation_data->ant_NB++;

    new_ant->PV = larve->Nest->PV[larve->Ant_type];
    new_ant->DMG = larve->Nest->DMG[larve->Ant_type];
    new_ant->Hunger = larve->Nest->Hunger - (larve->Hunger / 2);
    new_ant->Life = larve->Nest->Life_min + rand() % (larve->Nest->Life_max - larve->Nest->Life_min + 1); // Life entre Life_min et Life_max
    new_ant->Position = larve->Position;     // Position NULL au départ, assignation plus tard
    new_ant->Held_object = NULL;  // Pas d'objet au départ
    new_ant->Action = NULL;       // Pas de phéromone assignée au départ

    attach_ant_to_nest(new_ant, larve->Nest);


    if(simulation_data->debug_msgs){
        printf("| DEBUG : new ant \"%s\" initialized in nest \"%s\"\n", new_ant->Name_ID, larve->Nest->Clan);
    }

    free_larve(simulation_data, larve);
    return new_ant;
}

void Action_ant(Simulation_data* simulation_data, Ant* ant){    //fonction qui défini l'action d'une fourmis ouvrière/reine lors du cycle
    char salle_de_ponte[] = "salle de ponte";
    int condition = strcmp(ant->Position->Name_ID, salle_de_ponte);
    if(ant->Ant_type == 0){  // actions possibles des reines

        //si hunger < 10 --> aller manger
        //si stamina < 10 --> aller dormir ( si on fait le système du cycle de repos)

        if(ant->Hunger > 10 && condition == 0){ // si reinne a bien la nourriture requise (ici 10 pr l'exemple) et que reine est bien dans "salle de ponte"
            ant->Hunger = ant->Hunger - 10;   // on lui retire la nouriture utilisée
            init_new_egg(simulation_data, ant->Nest, NULL , 0 , ant->Position); //REGARDER COMMENT DEFINIR LE ANT_TYPE
            simulation_data->egg_IDs++;
            // ici faut rajouter une phéromnone qui indique qu'il faut déplacer l'oeuf
        }



    }
    else if(ant->Ant_type == 1){ // actions possibles des ouvrières

    }
    ant->Hunger--;
    ant->Life--;
    //faire vérif si il meurt ou pas
}

Ant* search_AntID(char* AntID, Exterior* Exterior) {
    if (Exterior == NULL) {
        perror("| ERROR : Cannot search a non existing Exterior");
        exit(1);
    }
    if (AntID == NULL) {
        perror("| ERROR : Cannot search NULL ant ID");
        exit(1);
    }
    for (int i = 0; i < Exterior->Total_Ant_number; i++) {
        if (strcmp(Exterior->All_Ant_list[i]->Name_ID, AntID) == 0) {
            return Exterior->All_Ant_list[i];
        }
    }
    printf("| ERROR : \"%s\" does not exist", AntID);
    exit(1);
}

void free_ant(Simulation_data* simulation_data, Ant* ant){
    if(ant != NULL){
        if(ant->Held_object != NULL){ // Si la fourmi porte un objet, on le pose dans la salle avant de free la fourmi
            ant->Held_object->Held = false;
            ant->Position->Obj_count++;
            ant->Position->Obj_list = realloc(ant->Position->Obj_list, ant->Position->Obj_count * sizeof(Object*));
            ant->Position->Obj_list[ant->Position->Obj_count-1] = ant->Held_object;
        }
        if(simulation_data->debug_msgs){
            printf("| DEBUG : ant \"%s\" freed\n", ant->Name_ID);
        }
        free(ant);
        simulation_data->ant_NB--;
    }
}

void test_kill_ant(Simulation_data* simulation_data, Ant* ant){
    if(ant != NULL){
        char* death_message[] = {"PV <= %d", "Life = %d", "Hunger <= %d"};
        int condition = 0;

        if(ant->PV <= 0){
            condition = 1;
        }
        if(ant->Life <= 0){
            condition = 2;
        }
        if(ant->Hunger <= -5){
            condition = 3;
        }

        if(condition != 0){
            if(simulation_data->debug_msgs){
                printf("| DEBUG : ant \"%s\" died : ", ant->Name_ID);
                printf(death_message[condition-1], (condition == 1)? ant->PV : (condition == 2)? ant->Life : ant->Hunger);
            }
            free_ant(simulation_data, ant);
        }
    }
}

void combat_ants(Simulation_data* simulation_data, Ant* ant1, Ant* ant2){
    if(ant1 == NULL){
        perror("Échec du combat_ants : ant1 NULL");
        return;
    }
    if(ant2 == NULL){
        perror("Échec du combat_ants : ant2 NULL");
        return ;
    }

    ant1->PV -= ant2->DMG;
    ant2->PV -= ant1->DMG;

    if(simulation_data->debug_msgs){
        printf("| DEBUG : ant \"%s\" : %d PV\n", ant1->Name_ID, ant1->PV);
        printf("| DEBUG : ant \"%s\" : %d PV\n", ant2->Name_ID, ant2->PV);
    }

    if(ant1->PV <= 0){
        ant2->Hunger += 10;  // ant2 se nourrit
    }
    if(ant2->PV <= 0){
        ant1->Hunger += 10;  // ant1 se nourrit
    }
    test_kill_ant(simulation_data, ant1);
    test_kill_ant(simulation_data, ant2);
}

int total_size(Ant* ant){
    if (ant->Held_object == NULL) {
        return 1; // 1 = Ant size
    }
    return 1 + ant->Held_object->Size; // 1 = ant size
}
