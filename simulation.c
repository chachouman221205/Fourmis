#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "simulation.h"
#include "objects.h"
#include "ants.h"
#include "rooms.h"


/* -----< Récupération des variables de départ >----- */
void init_variables(Simulation_data* simulation){  // Récupère les scanf pour inititaliser des variables
    printf("Saison de départ : (1: Spring, 2: Summer, 3: Autumn, 4: Winter)   ");
    scanf(" %d", &(simulation->start_season));
}

// Seasons
void init_seasons(Simulation_data* simulation_data, int start_season){     // Saison de départ (1 à 4) : 1 = spring, 2 = summer, 3 = autumn, 4 = winter
    Season *spring = malloc(sizeof(Season));
    if(spring == NULL){
        perror("Échec de l'allocation pour spring");
        exit(1);
    }
    spring->Name = "Spring";
    spring->Number = 1;
    spring->Chance = 7;  //en %

    Season *summer = malloc(sizeof(Season));
    if(summer == NULL){     // Si echec d'allocation, on free toutes les saisons déjà allouées
        perror("Échec de l'allocation pour summer");
        free(spring);
        exit(1);
    }
    summer->Name = "Summer";
    summer->Number = 2;
    summer->Chance = 12;

    Season *autumn = malloc(sizeof(Season));
    if(autumn == NULL){
        perror("Échec de l'allocation pour autumn");
        free(spring);
        free(summer);
        exit(1);
    }
    autumn->Name = "Autumn";
    autumn->Number = 3;
    autumn->Chance = 7;

    Season *winter = malloc(sizeof(Season));
    if(winter == NULL){
        perror("Échec de l'allocation pour winter");
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
        printf("| DEBUG : seasons initialized (start season \"%s\")\n", tab[start_season-1]->Name);
    }

    simulation_data->season_chain = tab[start_season-1];
}

void free_seasons(Simulation_data* simulation_data, Season* season){
    if(season != NULL){
        Season *current = season;
        Season *next_season = current->Next;

        while(current != season){
            free(current);
            current = next_season;
            next_season = current->Next;
        }
        free(current); // Libérer la derniere saison

        if(simulation_data->debug_msgs >= 2){
            printf("\033[1;32m| DEBUG : seasons freed\n\033[0m");
        }
    }
}

// Nest
Nest* init_nest(Simulation_data* simulation_data, char* specie, char* clan, int* pv, int* dmg, int life_min, int life_max, int hunger, Room* entry){
    Nest* new_nest = malloc(sizeof(Nest));
    if(new_nest == NULL){
        perror("Échec de l'allocation mémoire pour la nest");
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
    new_nest->Ant_list = malloc(0);
    new_nest->Ant_number = 0;
    new_nest->Larve_list = malloc(0);
    new_nest->Larve_number = 0;
    new_nest->Egg_list = malloc(0);
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
        for(int i = 0; i < nest->Ant_number; i++){
            free_ant(simulation_data, nest->Ant_list[i]);
        }
        // on free les larve associées à la fourmilière
        for(int i = 0; i < nest->Larve_number; i++){
            free_larve(simulation_data, nest->Larve_list[i]);
        }
        // on free les egg associées à la fourmilière
        for(int i = 0; i < nest->Egg_number; i++){
            free_egg(simulation_data, nest->Egg_list[i]);
        }

        // On retire la fourmilière de l'Exterior
        Exterior* ext = nest->Exterior;
        for(int i = 0; i < ext->Nest_number; i++){
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
        perror("Échec de l'allocation mémoire pour l'exterieur");
        return NULL;
    }

    new_exterior->Nests = malloc(0);
    new_exterior->Nest_number = 0;
    new_exterior->Entry = NULL;
    new_exterior->Ant_list = malloc(0);
    new_exterior->Ant_number = 0;


    Room** created_rooms = malloc(size * sizeof(Room*));
    if (created_rooms == NULL) {
        perror("Échec de l'allocation mémoire pour la liste des salles");
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

        for (int i = 0; i < exterior->Ant_number; i++) {
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
    int tries = 7;
    int chance = simulation_data->season_chain->Chance;
    int size_max = 20;
    if(!strcmp(room->Name_ID, "Exterior")){
        for(int i = 0; i < rand()% tries + 1; i++){
            if(rand()% 100 <= chance){
                Object* food = init_object(simulation_data, "food", rand()% size_max + 2, false);
                room->Obj_list = realloc(room->Obj_list, (room->Obj_count + 1)*sizeof(Object*));

                if(room->Obj_list == NULL){
                    perror("Échec de l'allocation mémoire pour la liste des salles");
                    exit(1);
                }
                room->Obj_list[room->Obj_count] = food;
                room->Obj_count++;
            }
        }
    }

        // test evolve
    // larves
    for(int i = 0; i < room->Larve_count; i++){
        if(simulation_data->debug_msgs >= 7){
            printf("\033[1;36m| DEBUG : larve \"%s\" in room \"%s\" has been tested\033[0m\n", room->Larve_list[i]->Name_ID, room->Name_ID);
        }
        if(simulation_data->debug_msgs >= 6){
            printf("\033[1;36m| DEBUG : %s : time left before growth : %d\n\033[0m", room->Larve_list[i]->Name_ID, room->Larve_list[i]->Grow);
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
    for(int i = 0; i < room->Egg_count; i++){
        if(simulation_data->debug_msgs >= 7){
            printf("\033[1;36m| DEBUG : egg \"%s\" in room \"%s\" has been tested\n\033[0m", room->Egg_list[i]->Name_ID, room->Name_ID);
        }
        if(simulation_data->debug_msgs >= 6){
            printf("\033[1;36m| DEBUG : %s : time left before growth : %d\n\033[0m", room->Egg_list[i]->Name_ID, room->Egg_list[i]->Grow);
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

    // action
    for(int i = 0; i < room->Ant_count; i++){
        Action_ant(simulation_data, room->Ant_list[i]);
    }

    // Fin du code à éxecuter
    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(simulation_data, room->Connexion_list[i]);
    }
}

void simulation(Simulation_data* simulation_data, int iterations) {
    if (iterations == 0) {
        return;
    }

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

    if(simulation_data->debug_msgs >= 1){
        printf("| DEBUG : ");
        print_numbers(simulation_data);
    }
    simulation(simulation_data, iterations-1);
}

void simulation_choice(Simulation_data* simulation_data){
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
        printf("2 : avancer de X tick\n\n");
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
        print_numbers(simulation_data);
    }
    if(choice < -1 || choice > 3){
        printf("0 : fin de la simulation\n");
        printf("1 : avancer de 1 tick (1 itération)\n");
        printf("2 : avancer de X tick\n\n");
        simulation_choice(simulation_data);
    }
}

// Pheromones
Pheromone* init_pheromone(char *action, int density, int ID) {
    Pheromone *new_pheromone = malloc(sizeof(Pheromone));
    if(new_pheromone == NULL){
        perror("Erreur d'allocation de mémoire");
        return NULL;
    }
    new_pheromone->Action = action;
    new_pheromone->Density = density;
    new_pheromone->ph_ID = ID;
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
        perror("Erreur d'allocation de mémoire");
        return NULL;
    }
    pv_param[0] = 15;
    pv_param[1] = 5;
    int* dmg_param = malloc(2*sizeof(int));
    if(dmg_param == NULL){
        perror("Erreur d'allocation de mémoire");
        return NULL;
    }
    dmg_param[0] = 1;
    dmg_param[1] = 5;

    Nest* nest = init_nest(simulation_data, "fourmia trèspetitus", "léptites fourmis", pv_param, dmg_param, 10, 100, 50, entry);

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
    Room* resting_room = init_room(simulation_data, "Resting Room", 50);
    Room* food_room1 = init_room(simulation_data, "Storage Room", 50);
    Room* food_room2 = init_room(simulation_data, "Storage Room", 60);
    Room* queen_chamber = init_room(simulation_data, "Queen chamber", 70);

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
    free(simulation_data);
}
