#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulation.h"

#define MAX_EGG_PER_ITERATION 4
#define EGG_COST 4

// Egg
Egg* init_new_egg(Simulation_data* simulation_data, Nest* nest, char *name, int ant_type, Room* room) {
    Egg* new_egg = malloc(sizeof(Egg));
    // Message d'erreur en cas de problèmes 
    if(new_egg == NULL){                                    
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    // Initialisation des champs de l'oeuf, on initialise en fonction de la nest
    if (name != NULL) {
        char* new_name = malloc(sizeof(name)+1);
        if (new_name == NULL) {
            perror("Échec de l'allocation mémoire pour le nom de la fourmi");
            exit(1);
        }
        for(int i = 0; i < strlen(name); i++){ //Permet de reasigner un nom dynamiquement

            new_name[i] = name[i];
        }
        new_egg->Name_ID = new_name;
    } else if (ant_type == 1) {
        new_egg->Name_ID = malloc(10 * sizeof(char)); // egg_IDs peut atteindre 1 000 000 : 10 caractères ("Ant" + jusqu'à 7 chiffres pour l'entier + \0)
        sprintf(new_egg->Name_ID, "Ant%d", simulation_data->egg_IDs++);
    } else {
        simulation_data->queen_IDs++;
        new_egg->Name_ID = malloc(10 * sizeof(char)); // egg_IDs peut atteindre 1 000 000 : 10 caractères ("Queen" + jusqu'à 4 chiffres pour l'entier + \0)
        sprintf(new_egg->Name_ID, "Queen%d", simulation_data->egg_IDs++);
    }
    new_egg->Ant_type = ant_type;
    new_egg->PV = 1;
    new_egg->Grow = (nest->Life_min + rand() % (nest->Life_max - nest->Life_min + 1))/3; // Grow entre (Life_min et Life_max)/3
    new_egg->Hunger = nest->Hunger; //L'oeuf apparait avec une quantité de nourriture propre à l'espèce ( on utilise "nest" aussi pour l'espèce)
    if (ant_type == 0) {
        new_egg->Hunger *= 5;
    }
    new_egg->Nest = nest;
    new_egg->Clan = nest->Clan;
    new_egg->Position = room;     // Position NULL au départ, assignation plus tard

    simulation_data->egg_NB++;   //Ajoute l'oeuf aux données simulées
    nest->Egg_list = realloc(nest->Egg_list, (++nest->Egg_number)*sizeof(Egg*));
    nest->Egg_list[nest->Egg_number-1] = new_egg;

    char* ant_types[] = {"Queen ", "Worker"};
    if(simulation_data->debug_msgs >= 1){
        printf("\033[1;34m| DEBUG : new egg \"%s\"%sant_type \"%s\" initialized in nest \"%s\"\n\033[0m", new_egg->Name_ID, simulation_data->space_tab[(strlen(new_egg->Name_ID)-3)%4], ant_types[new_egg->Ant_type], nest->Clan);
    }

    return new_egg;
}

void free_egg(Simulation_data* simulation_data, Egg* egg){
    if(egg != NULL){
        if(simulation_data->debug_msgs >= 1){
            printf("\033[1;32m| DEBUG : egg \"%s\"%sfreed\n\033[0m", egg->Name_ID, simulation_data->space_tab[(strlen(egg->Name_ID)-3)%4]);
        }

        simulation_data->egg_NB--;

        for(int i = 0; i < egg->Nest->Egg_number; i++){
            if(egg->Nest->Egg_list[i] == egg){
                egg->Nest->Egg_list[i] = egg->Nest->Egg_list[--egg->Nest->Egg_number];
                egg->Nest->Egg_list = realloc(egg->Nest->Egg_list, egg->Nest->Egg_number * sizeof(Egg*));
            }
        }

        for(int i = 0; i < egg->Position->Egg_count; i++){
            if(egg->Position->Egg_list[i] == egg){
                egg->Position->Egg_list[i] = egg->Position->Egg_list[--egg->Position->Egg_count];
                egg->Position->Egg_list = realloc(egg->Position->Egg_list, egg->Position->Egg_count * sizeof(Egg*));
            }
        }

        free(egg->Name_ID);
        free(egg);
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
            if(simulation_data->debug_msgs >= 4){
                printf("| DEBUG : egg \"%s\"%sdied : ", egg->Name_ID, simulation_data->space_tab[(strlen(egg->Name_ID)-3)%4]);
                printf(death_message[condition-1], (condition == 1)? egg->PV : egg->Hunger);
            }
            free_egg(simulation_data, egg);
        }
    }

}

bool test_grow_egg(Simulation_data* simulation_data, Egg* egg){
    if(egg != NULL){
        if(egg->Grow <= 0){
            if(simulation_data->debug_msgs >= 7){
                printf("| DEBUG : egg \"%s\"%scan evolve\n", egg->Name_ID, simulation_data->space_tab[(strlen(egg->Name_ID)-3)%4]);
            }
            return true;
        }
        else{
            if(simulation_data->debug_msgs >= 7){
                printf("| DEBUG : egg \"%s\"%scannot evolve\n", egg->Name_ID, simulation_data->space_tab[(strlen(egg->Name_ID)-3)%4]);
            }
            return false;
        }
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
    new_larve->Name_ID = calloc(strlen(egg->Name_ID) + 2, sizeof(char));
    if (new_larve->Name_ID == NULL) {
        perror("Erreur d'allocation pour Name_ID de la larve");
        exit(EXIT_FAILURE);
    }
    strcpy(new_larve->Name_ID, egg->Name_ID);

    new_larve->Ant_type = egg->Ant_type;
    new_larve->PV = egg->PV;
    new_larve->Grow = (egg->Nest->Life_min + rand() % (egg->Nest->Life_max - egg->Nest->Life_min + 1))/3; // Grow entre (Life_min et Life_max)/3
    new_larve->Hunger = egg->Hunger;
    new_larve->Clan = egg->Clan;
    new_larve->Nest = egg->Nest;
    new_larve->Position = egg->Position;     // Position NULL au départ, assignation plus tard

    char* ant_types[] = {"Queen ", "Worker"};
    if(simulation_data->debug_msgs >= 1){
        printf("\033[1;34m| DEBUG : new larve \"%s\"%sant_type \"%s\" initialized in nest \"%s\"\n\033[0m", new_larve->Name_ID, simulation_data->space_tab[(strlen(new_larve->Name_ID)-3)%4], ant_types[new_larve->Ant_type], new_larve->Nest->Clan);
    }

    simulation_data->larve_NB++;
    new_larve->Nest->Larve_list = realloc(new_larve->Nest->Larve_list, (++new_larve->Nest->Larve_number)*sizeof(Larve*));
    new_larve->Nest->Larve_list[new_larve->Nest->Larve_number-1] = new_larve;

    free_egg(simulation_data, egg);
    
    return new_larve;
}

void free_larve(Simulation_data* simulation_data, Larve* larve){
    if(larve != NULL){
        if(simulation_data->debug_msgs >= 1){
            printf("\033[1;32m| DEBUG : larve \"%s\"%sfreed\n\033[0m", larve->Name_ID, simulation_data->space_tab[(strlen(larve->Name_ID)-3)%4]);
        }

        simulation_data->larve_NB--;

        for(int i = 0; i < larve->Nest->Larve_number; i++){
            if(larve->Nest->Larve_list[i] == larve){
                larve->Nest->Larve_list[i] = larve->Nest->Larve_list[--larve->Nest->Larve_number];
                larve->Nest->Larve_list = realloc(larve->Nest->Larve_list, larve->Nest->Larve_number * sizeof(Larve*));
            }
        }
        for(int i = 0; i < larve->Position->Larve_count; i++){
            if(larve->Position->Larve_list[i] == larve){
                larve->Position->Larve_list[i] = larve->Position->Larve_list[--larve->Position->Larve_count];
                larve->Position->Larve_list = realloc(larve->Position->Larve_list, larve->Position->Larve_count * sizeof(Larve*));
            }
        }

        free(larve->Name_ID);
        free(larve);
    }
}

void test_kill_larve(Simulation_data* simulation_data, Larve* larve) {
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
            if(simulation_data->debug_msgs >= 4) {
                printf("| DEBUG : larve \"%s\"%sdied : ", larve->Name_ID, simulation_data->space_tab[(strlen(larve->Name_ID)-3)%4]);
                printf(death_message[condition-1], (condition == 1)? larve->PV : larve->Hunger);
            }
            free_larve(simulation_data, larve);
        }
    }

}

bool test_grow_larve(Simulation_data* simulation_data, Larve* larve) {
    if(larve != NULL){
        if(larve->Grow <= 0){
            if(simulation_data->debug_msgs >= 7){

                printf("| DEBUG : larve \"%s\"%scan evolve\n", larve->Name_ID, simulation_data->space_tab[(strlen(larve->Name_ID)-3)%4]);
            }
            return true;
        }

        if(simulation_data->debug_msgs >= 7){
            printf("| DEBUG : larve \"%s\"%scannot evolve\n", larve->Name_ID, simulation_data->space_tab[(strlen(larve->Name_ID)-3)%4]);
        }
        return false;
    }
    return false;
}

// Ants

void attach_ant_to_nest(Ant* ant, Nest* nest) {
    // On retire les liens avec l'ancienne colonie
    if (ant->Nest != NULL && ant->Nest != nest && 0) {
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
    new_ant->Name_ID = malloc((strlen(larve->Name_ID) + 1) * sizeof(char));
    if(new_ant->Name_ID == NULL){
        perror("Échec de l'allocation mémoire pour ant->Name_ID");
        return NULL;
    }
    strcpy(new_ant->Name_ID, larve->Name_ID);

    simulation_data->ant_NB++;
    simulation_data->Exterior->Ant_number++;
    simulation_data->Exterior->Ant_list = realloc(simulation_data->Exterior->Ant_list, simulation_data->Exterior->Ant_number * sizeof(Ant*));
    simulation_data->Exterior->Ant_list[simulation_data->Exterior->Ant_number-1] = new_ant;

    new_ant->Ant_type = larve->Ant_type;
    new_ant->PV = larve->PV;
    new_ant->DMG = larve->Nest->DMG[larve->Ant_type];
    new_ant->Hunger = larve->Hunger;
    new_ant->Life = larve->Nest->Life_min + rand() % (larve->Nest->Life_max - larve->Nest->Life_min + 1); // Life entre Life_min et Life_max
    if (larve->Ant_type == 0) {
        new_ant->Life *= 127; // Une reine vie en moyenne 30 ans (127 = 360 * 30 / moy(Life_min, Life_max))
    }
    new_ant->Position = larve->Position;     // Position NULL au départ, assignation plus tard
    new_ant->Held_object = NULL;  // Pas d'objet au départ
    new_ant->Action = NULL;       // Pas de phéromone assignée au départ
    new_ant->Path = NULL;

    attach_ant_to_nest(new_ant, larve->Nest);

    new_ant->Nest->Ant_number++;

    char* ant_types[] = {"Queen ", "Worker"};
    if(simulation_data->debug_msgs >= 1){
        printf("\033[1;34m| DEBUG : new ant \"%s\"%sant_type \"%s\" initialized in nest \"%s\"\n\033[0m", new_ant->Name_ID, simulation_data->space_tab[(strlen(new_ant->Name_ID)-3)%4], ant_types[new_ant->Ant_type], new_ant->Nest->Clan);
    }

    free_larve(simulation_data, larve);
    return new_ant;
}

bool move_ant(Simulation_data* simulation_data, Ant* ant, Room* room) {
    if (ant == NULL) {
        perror("| ERROR : Cannot move NULL ant");
    }
    if (room == NULL) {
        printf("| ERROR : Cannot move ant to NULL room");
        return false;
    }
    if(ant->Position == room){
        printf("| WARNING : trying to move ant to current location : move cancelled\n");
        return true;
    }

    // Vérification de la possibilité de déplacement
    bool possible = false;
    for (int i = 0; i < ant->Position->Connexion_list_size; i++) {
        if (ant->Position->Connexion_list[i] == room) {
            possible = true;
            break;
        }
    }
    if (!possible) {
        printf("| WARNING : trying to move ant to unaccessible location : move cancelled\n");
        return false;
    }

    if (simulation_data->debug_msgs >= 7) {
        printf("| DEBUG : moving ant \"%s\"%sfrom room \"%s\" to room \"%s\"\n", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4], ant->Position->Name_ID, room->Name_ID);
    }

    // Déplacement :

    // on retire la fourmi de l'ancienne salle
    for (int i = 0; i < ant->Position->Ant_count; i++) {
        if (ant->Position->Ant_list[i] == ant) {
            ant->Position->Ant_list[i] = ant->Position->Ant_list[--ant->Position->Ant_count];
            ant->Position->Ant_list = realloc(ant->Position->Ant_list, (ant->Position->Ant_count) * sizeof(Ant*));
        }
    }

    // on déplace l'objet que porte la fourmi
    if (ant->Held_object != NULL) {
        move_object(ant->Held_object, ant->Position, room);
    }

    // on ajoute la fourmi à la nouvelle salle
    room->Ant_list = realloc(room->Ant_list, (++room->Ant_count) * sizeof(Ant*));
    room->Ant_list[room->Ant_count-1] = ant;
    ant->Position = room;

    return true;
}
void use_path(Simulation_data* simulation_data, Ant* ant) {
    if (ant->Path==NULL) {
        printf("| WARNING : Ant \"%s\"%shas no path to use\n", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4]);
        return;
    }

    Path* old = ant->Path;
    ant->Path = ant->Path->next;
    free(old);

}
void follow_path(Simulation_data* simulation_data, Ant* ant) {
    if (move_ant(simulation_data, ant, ant->Path->room)) {
        use_path(simulation_data, ant);
    }
}

void Action_queen(Simulation_data* simulation_data, Ant* ant){
    //si hunger < 10 --> aller manger
    //si stamina < 10 --> aller dormir ( si on fait le système du cycle de repos)
    if(!strcmp(ant->Position->Name_ID, "Queen chamber")){
        int egg_count = rand()%MAX_EGG_PER_ITERATION + 1;
        for(int i = 0; i < egg_count; i++){
            // printf("remaining space : %d\n", remaining_space(ant->Position));
            if(ant->Hunger > (EGG_COST + 50) && remaining_space(ant->Position) > 25){
                // printf("egg\n");
                ant->Hunger = ant->Hunger - EGG_COST;   // on lui retire la nouriture utilisée
                ant->Position->Egg_list = realloc(ant->Position->Egg_list, (ant->Position->Egg_count+1)*sizeof(Egg*));
                if(ant->Position->Egg_list == NULL){
                    perror("Échec de la réallocation mémoire pour Egg_list");
                    return;
                }
                //ant_type_choice
                int ant_type_choice;

                if(ant->Hunger > 100 && (rand()%10 == 0)) { // && simulation_data->current_season == 2
                    ant_type_choice = 0; // on veut une reine
                }
                else{
                    ant_type_choice = 1;
                }
                //egg creation
                if(ant_type_choice == 0){
                    char name[20]; // Taille adaptée à "Queen" + numéro + '\0'
                    sprintf(name, "Queen%d", simulation_data->queen_IDs++);
                    // printf("%s\n", name);
                    ant->Position->Egg_list[ant->Position->Egg_count] = init_new_egg(simulation_data, ant->Nest, name, ant_type_choice , ant->Position);
                }
                else{
                    ant->Position->Egg_list[ant->Position->Egg_count] = init_new_egg(simulation_data, ant->Nest, NULL, ant_type_choice , ant->Position);
                }
                ant->Position->Egg_count++;
            }
        }
    }
    if(ant->Hunger <= EGG_COST){
        insert_pheromone(&(ant->Position->Pheromone), init_pheromone("bring_me_food", 10, 0));
    }

    //manger
    // printf("Queen %s essaie de miam : Hunger %d\n", ant->Name_ID, ant->Hunger);
    Object* food;
    food = search_object(ant->Position, "food");
    if(food == NULL){
        // printf("want food\n");
        insert_pheromone(&(ant->Position->Pheromone), init_pheromone("bring_me_food", 5, 0));
    }
    else{
        food->Size--;
        ant->Hunger += 15;
        // printf("Queen %s miam : Hunger %d\n", ant->Name_ID, ant->Hunger);
        if(food->Size == 0){
            // printf("obj_count %d\n", ant->Position->Obj_count);
            for(int i = 0; i < ant->Position->Obj_count; i++){
                if(food == ant->Position->Obj_list[i]){
                    ant->Position->Obj_list[i] = ant->Position->Obj_list[--ant->Position->Obj_count];
                }
            }
            free_object(simulation_data, food);
            ant->Position->Obj_list = realloc(ant->Position->Obj_list, ant->Position->Obj_count * sizeof(Object*));
            if(ant->Position->Obj_list == NULL && ant->Position->Obj_count != 0){
                printf("Échec de la réallocation mémoire pour ant->Position->Obj_list in \"Action_queen\"\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void Action_worker(Simulation_data* simulation_data, Ant* ant) {
    // Décision de la mission
    if(ant->Action == NULL){
        if (ant->Position->Pheromone == NULL) { // Aucun pheromone, la fourmi va chercher à manger pour la colonie
            printf("ant %s%ssent to look for food for STORAGE and is in %s\n", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4], ant->Position->Name_ID);
            insert_pheromone(&(ant->Action), init_pheromone("find_food", 6, 1));
        } else {
            insert_pheromone(&(ant->Action), ant->Position->Pheromone);
            ant->Position->Pheromone = NULL;
        }
    }

    // Execution de la mission choisie
    if(ant->Action->ph_ID == 0){
        printf("ant %s%slooking for food for QUEEN and is in %s\n", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4], ant->Position->Name_ID);
        if(ant->Held_object == NULL) {
            if(ant->Path == NULL){
                ant->Path = find_path_to_food(ant->Position, true); // true = pas le droit d'aller chercher de la nourriture en dehors de la fourmilière
                reinitialiser_rooms(simulation_data, ant->Position);
                use_path(simulation_data, ant);    //pour décaller de 1 path : si départ = a et objectif = c, path = a,b,c; On a besoin de b,c seulement
            }
            if(ant->Path == NULL){
                ant->Path = find_path_to_food(ant->Position, false); // Si aucune nourriture a été trouvée dans la fourmilière, on cherche également à l'extérieur
                reinitialiser_rooms(simulation_data, ant->Position);
                use_path(simulation_data, ant);
            }
            if (ant->Path == NULL) {
                if (search_object(ant->Position, "food") != NULL && strcmp(ant->Position->Name_ID, "Queen chamber")) {
                    pick_up(ant, search_object(ant->Position, "food")); // nourriture trouvée dans la salle actuelle de la fourmi
                }
            } else {
                follow_path(simulation_data, ant);
            }
        }
        else{
            if(ant->Path == NULL){
                if (strcmp(ant->Position->Name_ID, "Queen chamber")) {
                    ant->Path = find_path_to_name(ant->Position, "Queen chamber", false);
                    reinitialiser_rooms(simulation_data, ant->Position);
                    use_path(simulation_data, ant);
                }
                else if (!strcmp(ant->Position->Name_ID, "Queen chamber")) {
                    drop_object(ant);
                    //free pheromone done
                    Pheromone* old_ph = ant->Action;
                    ant->Action = ant->Action->next;
                    free(old_ph);

                    free(ant->Path);    //1 seul elmt à free
                }
                else {
                    drop_object(ant);
                    //free pheromone done
                    Pheromone* old_ph = ant->Action;
                    ant->Action = ant->Action->next;
                    free(old_ph);

                    free(ant->Path);    //1 seul elmt à free
                }
            }
            if(ant->Path != NULL){  // if path succeed
                // forcement un chemin
                if(ant->Path->length == 0){
                    drop_object(ant); // if(held != NULL)     //ant->Held_object = search_object(ant->position, "food");
                    //remove_obj_from_room_list(ant->position, "food");
                    //chemin fini + obj held

                    //free pheromone done
                    Pheromone* old_ph = ant->Action;
                    ant->Action = ant->Action->next;
                    free(old_ph);

                    free_Path(ant->Path);    //1 seul elmt à free
                }
                else{       //move closer to destination
                    if (move_ant(simulation_data, ant, ant->Path->room)) {
                        use_path(simulation_data, ant);
                    }
                }
            }
        }
    }
    else if(ant->Action->ph_ID == 1){
        printf("ant %s%slooking for food for STORAGE and is in %s\n", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4], ant->Position->Name_ID);
        if(ant->Held_object == NULL){
            if (ant->Path == NULL && strcmp(ant->Position->Name_ID, "Exterior") == 0) {
                if(search_object(ant->Position, "food") == NULL){
                    ant->Path = find_path_to_food(ant->Position, false);
                    reinitialiser_rooms(simulation_data, ant->Position);
                    use_path(simulation_data, ant);
                } else {
                    pick_up(ant, search_object(ant->Position, "food"));
                }
            }
            else if (ant->Path == NULL){ // si dans nest et pas de chemin
                ant->Path = find_path_to_name(ant->Position, "Exterior", false);
                reinitialiser_rooms(simulation_data, ant->Position);
            }   //trouver un chemin vers exterior
            if(ant->Path != NULL){  // if path succeeded
                // forcement un chemin
                // move closer to destination
                follow_path(simulation_data, ant);
            }
        }
        else{
            if (ant->Path == NULL) {
                if (strcmp(ant->Position->Name_ID, "Storage Room")) {
                    ant->Path = find_path_to_name(ant->Position, "Storage Room", false);
                    reinitialiser_rooms(simulation_data, ant->Position);
                } else {
                    drop_object(ant);
                }
            } else {
                follow_path(simulation_data, ant);
            }
        }
    }

    //manger
    if(ant->Hunger < 10 && ant->Held_object != NULL){
        if(ant->Held_object->Size >= 2){
            ant->Held_object->Size--;
            ant->Hunger += 15;
        }
    }
}

void Action_ant(Simulation_data* simulation_data, Ant* ant){    //fonction qui défini l'action d'une fourmis ouvrière/reine lors du cycle
    if(ant->Ant_type == 0){  // actions possibles des reines
        Action_queen(simulation_data, ant);
    }

    else if(ant->Ant_type == 1){ // actions possibles des ouvrières
        Action_worker(simulation_data, ant);
    }

    
    ant->Hunger--;   // à chaque unité de temps on retire de la nourtiure et du temps de vie à la fourmis
    ant->Life--;
    test_kill_ant(simulation_data, ant); //vérifie si la fourmis est bien en vie
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
    for (int i = 0; i < Exterior->Ant_number; i++) {
        if (strcmp(Exterior->Ant_list[i]->Name_ID, AntID) == 0) {
            return Exterior->Ant_list[i];
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
        if(simulation_data->debug_msgs >= 1){
            printf("\033[1;32m| DEBUG : ant \"%s\"%sfreed\n\033[0m", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4]);
        }

        simulation_data->ant_NB--;

        for(int i = 0; i < simulation_data->Exterior->Ant_number; i++){
            if(simulation_data->Exterior->Ant_list[i] == ant){
                simulation_data->Exterior->Ant_list[i] = simulation_data->Exterior->Ant_list[--(simulation_data->Exterior->Ant_number)];
                simulation_data->Exterior->Ant_list = realloc(simulation_data->Exterior->Ant_list, simulation_data->Exterior->Ant_number * sizeof(Ant*));
            }
        }
        for(int i = 0; i < ant->Nest->Ant_number; i++){
            if(ant->Nest->Ant_list[i] == ant){
                ant->Nest->Ant_list[i] = ant->Nest->Ant_list[--(ant->Nest->Ant_number)];
                ant->Nest->Ant_list = realloc(ant->Nest->Ant_list, ant->Nest->Ant_number * sizeof(Ant*));
            }
        }
        for(int i = 0; i < ant->Position->Ant_count; i++){
            if(ant->Position->Ant_list[i] == ant){
                ant->Position->Ant_list[i] = ant->Position->Ant_list[--(ant->Position->Ant_count)];
                ant->Position->Ant_list = realloc(ant->Position->Ant_list, ant->Position->Ant_count * sizeof(Ant*));
            }
        }
        free(ant->Name_ID);
        free(ant);
    }
}

void test_kill_ant(Simulation_data* simulation_data, Ant* ant){
    if(ant != NULL){
        char* death_message[] = {"PV <= %d\n\033[0m", "Life = %d\n\033[0m", "Hunger <= %d\n\033[0m"};
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
            if(simulation_data->debug_msgs >= 4){
                printf("\033[1;35m| DEBUG : ant \"%s\"%sdied : ", ant->Name_ID, simulation_data->space_tab[(strlen(ant->Name_ID)-3)%4]);
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

    if(simulation_data->debug_msgs >= 4){
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
