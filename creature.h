// Représente une créature quelconque (prédateur ou autre)
typedef struct Creature {
    char *Name_ID;

    int PV;
    int DMG;
    int Life;
    int Hunger;
    int Size;

    struct Room *Position;
} Creature;

// Creatures
Creature* init_creature(Simulation_data* simulation_data, char* name_ID, int pv, int dmg, int life, int hunger, Room* position){
    Creature* new_creature = malloc(sizeof(Creature));
    if(new_creature == NULL){
        perror("Échec de l'allocation mémoire pour la creature");
        return NULL;
    }

    simulation_data->crea_NB++;

    if (name_ID != NULL) {
        new_creature->Name_ID = name_ID;
        simulation_data->crea_IDs++;
    } else {
        sprintf(new_creature->Name_ID, "Crea%d", simulation_data->crea_IDs++);
    }
    new_creature->PV = pv;
    new_creature->DMG = dmg;
    new_creature->Life = life;
    new_creature->Hunger = hunger;
    new_creature->Position = position;

    if(debug_msgs){
        printf("| DEBUG : new creature \"%s\" initialized\n", new_creature->Name_ID);
    }
}

void free_creature(Simulation_data* simulation_data, Creature* creature){
    if(creature != NULL){
        if(debug_msgs){
            printf("| DEBUG : creature \"%s\" freed\n", creature->Name_ID);
        }
        free(creature);
        simulation_data->crea_NB--;
    }
}

void test_kill_creature(Simulation_data* simulation_data, Creature* crea){
    if(crea != NULL){
        char* death_message[] = {"PV <= %d", "Life = %d", "Hunger <= %d"};
        int condition = 0;

        if(crea->PV <= 0){
            condition = 1;
        }
        if(crea->Life <= 0){
            condition = 2;
        }
        if(crea->Hunger <= -5){
            condition = 3;
        }

        if(condition != 0){
            if(debug_msgs){
                printf("| DEBUG : crea \"%s\" died : ", crea->Name_ID);
                printf(death_message[condition], (condition == 1)? crea->PV : (condition == 2)? crea->Life : crea->Hunger);
            }
            free_creature(simulation_data, crea);
        }
    }

}

void combat_ant_creature(Simulation_data* simulation_data, Ant* ant, Creature* crea){
    if(ant == NULL){
        perror("Échec du combat_ant_crea : ant NULL");
        return;
    }
    if(crea == NULL){
        perror("Échec du combat_ant_crea : crea NULL");
        return;
    }

    ant->PV -= crea->DMG;
    crea->PV -= ant->DMG;

    if(debug_msgs){
        printf("| DEBUG : ant \"%s\" : %d PV\n", ant->Name_ID, ant->PV);
        printf("| DEBUG : crea \"%s\" : %d PV\n", crea->Name_ID, crea->PV);
    }

    if(crea->PV <= 0){
        ant->Hunger += 15;  // la fourmi se nourrit, la fourmi se nourrit beaucoup avec une creature
    }
    if(ant->PV <= 0){
        crea->Hunger += 3;  // la creature se nourrit, la creature se nourrit peu avec une fourmi
    }
    test_kill_ant(simulation_data, ant);
    test_kill_creature(simulation_data, crea);
}