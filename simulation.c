#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "simulation.h"
#include "objects.h"
#include "ants.h"
#include "rooms.h"

/* -----< Récupération des variables de départ >----- */
void init_variables(Simulation_data* simulation){  // Récupère les scanf pour inititaliser des variables
    printf("Saison de départ : (1: Spring, 2: Summer, 3: Autumn, 4: Winter)   ");
    scanf("%d", &(simulation->start_season));
    if (simulation->start_season > 4 || simulation->start_season < 1) {
        printf("\033[1;31mValeur non acceptée\n\033[0m");
        while (getchar() != '\n');
        init_variables(simulation);
    }
}

// Seasons
void init_seasons(Simulation_data* simulation_data){     // Saison de départ (1 à 4) : 1 = spring, 2 = summer, 3 = autumn, 4 = winter
    Season *spring = malloc(sizeof(Season));
    if(spring == NULL){
        printf("Échec de l'allocation pour spring\n");
        exit(1);
    }
    spring->Name = "Spring";
    spring->Number = 1;
    spring->Chance = 7;  //en %

    Season *summer = malloc(sizeof(Season));
    if(summer == NULL){     // Si echec d'allocation, on free toutes les saisons déjà allouées
        printf("Échec de l'allocation pour summer\n");
        free(spring);
        exit(1);
    }
    summer->Name = "Summer";
    summer->Number = 2;
    summer->Chance = 12;

    Season *autumn = malloc(sizeof(Season));
    if(autumn == NULL){
        printf("Échec de l'allocation pour autumn\n");
        free(spring);
        free(summer);
        exit(1);
    }
    autumn->Name = "Autumn";
    autumn->Number = 3;
    autumn->Chance = 7;

    Season *winter = malloc(sizeof(Season));
    if(winter == NULL){
        printf("Échec de l'allocation pour winter\n");
        free(spring);
        free(summer);
        free(autumn);
        exit(1);
    }
    winter->Name = "Winter";
    winter->Number = 4;
    winter->Chance = 1;

    // Chaînage des saisons : Boucle cyclique
    spring->Next = summer;
    summer->Next = autumn;
    autumn->Next = winter;
    winter->Next = spring;

    Season* tab[] = {spring, summer, autumn, winter};

    if(simulation_data->debug_msgs >= 2){
        printf("| DEBUG : seasons initialized (start season \"%s\")\n", tab[simulation_data->start_season-1]->Name);
    }

    simulation_data->season_chain = tab[simulation_data->start_season-1];
}

void free_seasons(Simulation_data* simulation_data){
    Season *start_season = simulation_data->season_chain;
    Season *current = start_season;
    Season *next_season = current->Next;

    while(next_season != start_season){
        free(current);
        current = next_season;
        next_season = current->Next;
    }
    free(current); // Libérer la derniere saison

    if(simulation_data->debug_msgs >= 2){
        printf("\033[1;32m| DEBUG : seasons freed\n\033[0m");
    }
}

// Nest
Nest* init_nest(Simulation_data* simulation_data, char* specie, char* clan, int* pv, int* dmg, int life_min, int life_max, int hunger, Room* entry){
    Nest* new_nest = malloc(sizeof(Nest));
    if(new_nest == NULL){
        printf("Échec de l'allocation mémoire pour la nest\n");
        return NULL;
    }

    simulation_data->nest_NB++;

    new_nest->Specie = specie;
    new_nest->Clan = clan;
    new_nest->PV = pv;
    new_nest->DMG = dmg;
    new_nest->Life_min = life_min;
    new_nest->Life_max = life_max;
    new_nest->Hunger = hunger;
    new_nest->Ant_list = NULL;
    new_nest->Ant_number = 0;
    new_nest->Larve_list = NULL;
    new_nest->Larve_number = 0;
    new_nest->Egg_list = NULL;
    new_nest->Egg_number = 0;
    new_nest->Entry = entry;
    new_nest->Exterior = simulation_data->Exterior;
    new_nest->Queen_chamber = NULL;

    new_nest->Exterior->Nests = realloc(new_nest->Exterior->Nests, (++(new_nest->Exterior->Nest_number)) * sizeof(Nest*));
    new_nest->Exterior->Nests[new_nest->Exterior->Nest_number-1] = new_nest;

    if(simulation_data->debug_msgs >= 2){
        printf("| DEBUG : new nest \"%s\" initialized\n", new_nest->Clan);
    }

    return new_nest;
}

void free_nest(Simulation_data* simulation_data, Nest* nest){
    if(nest != NULL){

        free(nest->PV);
        free(nest->DMG);

        // on déconnecte la fourmilière de l'exterieur
        disconnect_rooms(nest->Entry, nest->Exterior->Entry);
        free_room_rec(simulation_data, nest->Entry);

        // on free les fourmis associées à la fourmilière
        for(int i = nest->Ant_number-1; i >= 0; i--){
            free_ant(simulation_data, nest->Ant_list[i]);
        }
        // on free les larve associées à la fourmilière
        for(int i = nest->Larve_number-1; i >= 0; i--){
            free_larve(simulation_data, nest->Larve_list[i]);
        }
        // on free les egg associées à la fourmilière
        for(int i = nest->Egg_number-1; i >= 0; i--){
            free_egg(simulation_data, nest->Egg_list[i]);
        }

        // On retire la fourmilière de l'Exterior
        Exterior* ext = nest->Exterior;
        for(int i = ext->Nest_number-1; i >= 0; i--){
            if (ext->Nests[i] == nest) {
                ext->Nests[i] = ext->Nests[--ext->Nest_number];
                ext->Nests = realloc(ext->Nests, (ext->Nest_number) * sizeof(Nest*));
            }
        }

        simulation_data->nest_NB--;

        if(simulation_data->debug_msgs >= 2){
            printf("\033[1;32m| DEBUG : Nest \"%s\" freed\n\033[0m", nest->Clan);
        }

        free(nest->Ant_list);
        free(nest->Larve_list);
        free(nest->Egg_list);

        free(nest);
    }
}

// Exterior
Exterior* init_exterior(Simulation_data* simulation_data, int size){
    Exterior* new_exterior = malloc(sizeof(Exterior));
    if(new_exterior == NULL){
        printf("Échec de l'allocation mémoire pour l'exterieur\n");
        return NULL;
    }

    new_exterior->Nests = NULL;
    new_exterior->Nest_number = 0;
    new_exterior->Entry = NULL;
    new_exterior->Ant_list = NULL;
    new_exterior->Ant_number = 0;


    Room** created_rooms = malloc(size * sizeof(Room*));
    if (created_rooms == NULL) {
        printf("Échec de l'allocation mémoire pour la liste des salles\n");
    }
    for (int i = 0; i < size; i++) {
        created_rooms[i] = NULL;
    }

    created_rooms[0] = init_room(simulation_data, "Exterior", (rand()%(600-500))+500);

    Room* R2;
    int j;
    for (int i = 1; i < size; i++) {
        created_rooms[i] = init_room(simulation_data, "Exterior", (rand()%(600-500))+500); // Chaque salle a une taille aléatoire entre 500 et 600

        // On connecte la salle crée à jusqu'à trois autres salles
        j = rand()%3+1;
        if(simulation_data->debug_msgs >= 8){
            printf("Connecting to %d other rooms\n", j);
        }
        for (; j>0; j--) {
            connect_rooms(simulation_data, created_rooms[rand()%(i)], created_rooms[i]);
        }

        R2 = created_rooms[i];

        if(simulation_data->debug_msgs >= 8){
            for (int k = 0; k < R2->Connexion_list_size; k++) {
                printf("%p\n", R2->Connexion_list[k]);
            }
        }
    }

    new_exterior->Entry = created_rooms[0];
    free(created_rooms);

    if(simulation_data->debug_msgs >= 2){
        printf("| DEBUG : new exterior initialized\n");
    }

    return new_exterior;
}

void free_exterior(Simulation_data* simulation_data, Exterior* exterior){

    if(exterior != NULL) {
        while (0 < exterior->Nest_number) {
            free_nest(simulation_data, exterior->Nests[0]);
        }

        for (int i = exterior->Ant_number-1; i >= 0; i--) {
            free_ant(simulation_data, exterior->Ant_list[i]);
        }
        free_room_rec(simulation_data, exterior->Entry);

        if(simulation_data->debug_msgs >= 2){
            printf("\033[1;32m| DEBUG : exterior freed\n\033[0m");
        }

        free(exterior);
    }
}

// Display
/*
void print_numbers(Simulation_data* sim){   // OLD
    printf("| DEBUG : Nests : %d | Rooms : %d | Eggs : %d | Larves : %d | Ants : %d | Creas : %d | Objs : %d | Ticks : %d | Season : %d | Season_counter %d\n\n",
           sim->nest_NB, sim->room_IDs, sim->egg_NB, sim->larve_NB, sim->ant_NB, sim->crea_NB, sim->obj_NB, sim->tick, sim->season_chain->Number, sim->counter);
}
*/
void print_numbers(Simulation_data* sim){
    printf("Eggs : %d | Larves : %d | Ants : %d | Objs : %d | Ticks : %d\n\n",
           sim->egg_NB, sim->larve_NB, sim->ant_NB, sim->obj_NB, sim->tick);
}

/* -----< Simulation >----- */
/*
 Ici sont gérées les étapes de la simulation, init et itérations
 */

void simuler_room(Simulation_data* simulation_data, Room* room) {
    if (room->Visited) {
        return;
    }
    room->Visited = true;

    // refill food
    int tries = 4;
    int chance = simulation_data->season_chain->Chance;
    int size_max = 20;
    if(!strcmp(room->Name_ID, "Exterior")){
        for(int i = 0; i < rand()% tries + 1; i++){
            if(rand()% 100 <= chance){
                Object* food = init_object(simulation_data, "food", rand()% size_max + 2, false);
                room->Obj_list = realloc(room->Obj_list, (room->Obj_count + 1)*sizeof(Object*));

                if(room->Obj_list == NULL){
                    printf("Échec de l'allocation mémoire pour la liste des salles\n");
                    exit(1);
                }
                room->Obj_list[room->Obj_count] = food;
                room->Obj_count++;
            }
        }
    }

        // test evolve
    // larves
    for(int i = room->Larve_count -1; i > -1; i--){
        if(simulation_data->debug_msgs >= 7){
            printf("\033[1;36m| DEBUG : larve \"%s\"%sin room \"%s\" has been tested\033[0m\n", room->Larve_list[i]->Name_ID, simulation_data->space_tab[(strlen(room->Larve_list[i]->Name_ID)-3)%4], room->Name_ID);
        }
        if(simulation_data->debug_msgs >= 6){
            printf("\033[1;36m| DEBUG : %s%s: time left before growth : %d\n\033[0m", room->Larve_list[i]->Name_ID, simulation_data->space_tab[(strlen(room->Larve_list[i]->Name_ID)-3)%4], room->Larve_list[i]->Grow);
        }
        if(test_grow_larve(simulation_data, room->Larve_list[i])){
            Ant* ant = init_new_ant(simulation_data, room->Larve_list[i]);
            room->Ant_list = realloc(room->Ant_list, (++room->Ant_count)*sizeof(Ant*));
            room->Ant_list[room->Ant_count-1] = ant;
        }
        else{
            room->Larve_list[i]->Grow--;
        }
    }
    // eggs
    for(int i = room->Egg_count -1; i > -1; i--){
        if(simulation_data->debug_msgs >= 7){
            printf("\033[1;36m| DEBUG : egg \"%s\"%sin room \"%s\" has been tested\n\033[0m", room->Egg_list[i]->Name_ID, simulation_data->space_tab[(strlen(room->Egg_list[i]->Name_ID)-3)%4], room->Name_ID);
        }
        if(simulation_data->debug_msgs >= 6){
            printf("\033[1;36m| DEBUG : %s%s: time left before growth : %d\n\033[0m", room->Egg_list[i]->Name_ID, simulation_data->space_tab[(strlen(room->Egg_list[i]->Name_ID)-3)%4], room->Egg_list[i]->Grow);
        }
        if(test_grow_egg(simulation_data, room->Egg_list[i])){
            Larve* larve = init_new_larve(simulation_data, room->Egg_list[i]);
            room->Larve_list = realloc(room->Larve_list, (++room->Larve_count)*sizeof(Larve*));
            room->Larve_list[room->Larve_count-1] = larve;
        }
        else{
            room->Egg_list[i]->Grow--;
        }
    }

    // Fin du code à éxecuter
    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(simulation_data, room->Connexion_list[i]);
    }
}

void simulation(Simulation_data* simulation_data, int iterations) {
    while (iterations > 0) {
        if(simulation_data->debug_msgs >= 1){
            printf("| DEBUG : iterations left : %d\n", iterations);
        }

        simulation_data->tick++;
        simulation_data->counter++;

        if(simulation_data->counter >= 90){     // Passage à la saison suivante
            simulation_data->counter = 0;

            simulation_data->season_chain = simulation_data->season_chain->Next;
            simulation_data->current_season = simulation_data->season_chain->Number;
        }

        simuler_room(simulation_data, simulation_data->Exterior->Entry);
        reinitialiser_rooms(simulation_data, simulation_data->Exterior->Entry);
        for (int i = 0; i < simulation_data->Exterior->Ant_number; i++) {
            Action_ant(simulation_data, simulation_data->Exterior->Ant_list[i]);
        }

        if(simulation_data->debug_msgs >= 1){
            printf("| DEBUG : ");
            print_numbers(simulation_data);
        }

        sleep(simulation_data->pause * simulation_data->pause_enable);
        iterations--;
    }
}

void simulation_choice(Simulation_data* simulation_data){
    simulation_data->pause_enable = 0;
    int choice;
    printf("Quel choix pour la simulation ? (-1 pour le message d'aide)   ");

    int valide = scanf("%d", &choice);
    if(!valide){
        printf("ERREUR : l'input n'est pas un \"int\"\n");
        while (getchar() != '\n');
        choice = -2;
    }
    if(choice == -1){
        printf("0 : fin de la simulation\n");
        printf("1 : avancer de 1 tick (1 itération)\n");
        printf("2 : avancer de X tick\n");
        printf("3 : avancer de X tick avec pause de 1s\n");
        printf("4 : changer le niveau de debug\n");
        printf("5 : print numbers\n\n");
        simulation_choice(simulation_data);
    }
    if(choice == 0){
        printf("| Simulation finie\n");
        fin(simulation_data);
        exit(0);
    }
    if(choice == 1){
        int X = 1;
        simulation(simulation_data, X);
    }
    if(choice == 2){
        int X;
        printf("Combien de ticks voulez-vous simuler ? :   ");
        scanf("%d", &X);
        simulation(simulation_data, X);
    }
    if(choice == 3){
        int X;
        printf("Combien de ticks voulez-vous simuler ? :   ");
        scanf("%d", &X);
        simulation_data->pause_enable = 1;
        simulation(simulation_data, X);
    }
    if(choice == 4){
        printf("Simulation debug messages:\n"
       "    0: No messages\n"
       "    1: Ants, larvae, eggs, creatures (init, free, evolution), print_numbers\n"
       "    2: 1+ Environment, seasons (init, free), iterations left\n"
       "    3: 2+ Rooms (init, free, connections)\n"
       "    4: 3+ Death and HP of ants, larvae, eggs, creatures\n"
       "    5: 4+ Food (creation, free), ant moves\n"
       "    6: 5+ Evolution (time left)\n"
       "    7: 6+ Evolution (test cases)\n"
       "    8: 7+ Room connections (init, free, reset)\n\n");
        scanf("%d", &simulation_data->debug_msgs);
    }
    if(choice == 5){
        print_numbers(simulation_data);
    }
    if (choice == 6) {
        char* ant_name = malloc(10 * sizeof(char));
        printf("Quelle fourmi voulez-vous consulter?   ");
        scanf("%s", ant_name);

        print_ant_details(search_AntID(ant_name, simulation_data->Exterior));
        free(ant_name);
    }
    if(choice < -1 || choice > 6){
        printf("0 : fin de la simulation\n");
        printf("1 : avancer de 1 tick (1 itération)\n");
        printf("2 : avancer de X tick\n");
        printf("3 : avancer de X tick avec pause de 1s\n");
        printf("4 : changer le niveau de debug\n");
        printf("5 : afficher les statisiques de la simulation\n");
        printf("6 : afficher l'état d'une fourmi\n  ");
        printf("\n");
    }
}

// Pheromones
Pheromone* init_pheromone(char *action, int density, int ID) {
    Pheromone *new_pheromone = malloc(sizeof(Pheromone));
    if(new_pheromone == NULL){
        printf("Erreur d'allocation de mémoire\n");
        return NULL;
    }
    new_pheromone->Action = action;
    new_pheromone->Density = density;
    new_pheromone->ph_ID = ID;
    new_pheromone->next = NULL;
    return new_pheromone;
}

void free_all_pheromones(Pheromone *stack) {
    while (stack != NULL) {
        Pheromone *temp = stack;
        stack = stack->next;
        free(temp);
    }
}

Pheromone* get_first_pheromone(Pheromone **stack) {
    if (*stack == NULL) {
        return NULL; // Pile vide
    }
    Pheromone *top_pheromone = *stack;
    *stack = (*stack)->next;
    return top_pheromone;
}

void insert_pheromone(Pheromone **stack, Pheromone *new_pheromone) {
    if (*stack == NULL || (*stack)->Density < new_pheromone->Density) {
        // Insérer en tête si la pile est vide ou si la nouvelle phéromone est la plus dense
        new_pheromone->next = *stack;
        *stack = new_pheromone;
        return;
    }

    // Trouver la position correcte pour insérer
    Pheromone *current = *stack;
    while (current->next != NULL && current->next->Density >= new_pheromone->Density) {
        current = current->next;
    }

    // Insérer la nouvelle phéromone
    new_pheromone->next = current->next;
    current->next = new_pheromone;
}

/* -----< Initialisation de la simulation >----- */
Simulation_data* init_simulation(){
    Simulation_data* sim = malloc(sizeof(Simulation_data));

    sim->tick = 0;
    sim->pause = 1;
    sim->pause_enable = 0;
    sim->space_tab[0] = "    ";
    sim->space_tab[1] = "   ";
    sim->space_tab[2] = "  ";
    sim->space_tab[3] = " ";

    sim->start_season = 0;
    sim->counter = 0;
    sim->current_season = 0;
    sim->season_chain = NULL;
    sim->Exterior = NULL;

    // IDs
    sim->room_IDs = 0;
    sim->egg_IDs = 0;
    sim->queen_IDs = 1;
    sim->obj_IDs = 0;
    sim->crea_IDs = 0;

    // Counts
    sim->nest_NB = 0;
    sim->room_NB = 0;
    sim->egg_NB = 0;
    sim->larve_NB = 0;
    sim->ant_NB = 0;
    sim->obj_NB = 0;
    sim->crea_NB = 0;

    return sim;
}

Nest* start(Simulation_data* simulation_data){   // Lancer la simulation
    // Season* season = init_seasons(simulation_data, 0);
    srand(time(NULL)); // Pour rendre la simulation aléatoire

    init_variables(simulation_data);
    init_seasons(simulation_data);

    // Création du monde
    // Génération de l'extérieur
    printf("Veuillez choisir une taille d'environnement pour la simulation. Nous recommandons entre 10 (très petit) et 300 (très grand) :   ");
    int room_number;
    scanf("%d", &room_number);
    simulation_data->Exterior = init_exterior(simulation_data, room_number);

    // Création de la fourmilière
    Room* entry = init_room(simulation_data, "Nest Entrance", 20);
    int* pv_param = malloc(2*sizeof(int));
    if(pv_param == NULL){
        printf("Erreur d'allocation de mémoire\n");
        return NULL;
    }
    pv_param[0] = 15;
    pv_param[1] = 5;
    int* dmg_param = malloc(2*sizeof(int));
    if(dmg_param == NULL){
        printf("Erreur d'allocation de mémoire\n");
        return NULL;
    }
    dmg_param[0] = 1;
    dmg_param[1] = 5;

    Nest* nest = init_nest(simulation_data, "fourmia trèspetitus", "léptites fourmis", pv_param, dmg_param, 70, 100, 30, entry);

    /* Structure de la fourmilière initiale voulue:
     *                 entrée
     *                /      \
     * Chambre de repos      Stock de nourriture 1
     *           |            /       |
     *           |           /        |
     *        Stock de nouriture 2    |
     *                         \      |
     *                          Chambre de la reine
     */

    // Création des salles
    Room* resting_room = init_room(simulation_data, "Resting Room", 100);
    Room* food_room1 = init_room(simulation_data, "Storage Room", 150);
    Room* food_room2 = init_room(simulation_data, "Storage Room", 120);
    Room* queen_chamber = init_room(simulation_data, "Queen chamber", 200);

    nest->Queen_chamber = queen_chamber;

    // Connection des salles
    connect_rooms(simulation_data, entry, resting_room);
    connect_rooms(simulation_data, entry, food_room1);
    connect_rooms(simulation_data, resting_room, food_room2);
    connect_rooms(simulation_data, food_room1, food_room2);
    connect_rooms(simulation_data, food_room1, queen_chamber);
    connect_rooms(simulation_data, food_room2, queen_chamber);

    // On s'assure qu'une des salles est connectée à la fourmilière
    connect_rooms(simulation_data, simulation_data->Exterior->Entry, entry);
    nest->Exterior = simulation_data->Exterior;

    return nest;
}

void fin(Simulation_data* simulation_data) {
    free_exterior(simulation_data, simulation_data->Exterior);
    free_seasons(simulation_data);
    free(simulation_data);
}
