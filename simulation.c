#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "simulation.h"
#include "objects.h"
#include "ants.h"
#include "rooms.h"

bool debug_msgs = 0;    // Printf de messages si = 1, pour le debug


/* -----< Récupération des variables de départ >----- */
void init_variables(Simulation_data* simulation){  // Récupère les scanf pour inititaliser des variables
    printf("Saison de départ : (1: Spring, 2: Summer, 3: Autumn, 4: Winter)");
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

    Season *summer = malloc(sizeof(Season));
    if(summer == NULL){     // Si echec d'allocation, on free toutes les saisons déjà allouées
        perror("Échec de l'allocation pour summer");
        free(spring);
        exit(1);
    }
    summer->Name = "Summer";
    summer->Number = 2;

    Season *autumn = malloc(sizeof(Season));
    if(autumn == NULL){
        perror("Échec de l'allocation pour autumn");
        free(spring);
        free(summer);
        exit(1);
    }
    autumn->Name = "Autumn";
    autumn->Number = 3;

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

    // Chaînage des saisons : Boucle cyclique
    spring->Next = summer;
    summer->Next = autumn;
    autumn->Next = winter;
    winter->Next = spring;

    Season* tab[] = {spring, summer, autumn, winter};

    if(debug_msgs){
        printf("| DEBUG : seasons initialized (start season \"%s\"\n", tab[start_season]->Name);
    }

    simulation_data->season_chain = tab[start_season-1];
}

void free_seasons(Season* season){
    if(season != NULL){
        Season *current = season;
        Season *next_season = current->Next;

        while(current != season){
            free(current);
            current = next_season;
            next_season = current->Next;
        }
        free(current); // Libérer la derniere saison

        if(debug_msgs){
            printf("| DEBUG : seasons freed\n");
        }
    }
}

// Nest
Nest* init_nest(Simulation_data* simulation_data, char* specie, char* clan, int* pv, int* dmg, int life_min, int life_max, int hunger, Room* entry, Exterior* Exterior){
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
    new_nest->Entry = entry;
    new_nest->Exterior = Exterior;

    Exterior->Nests = realloc(Exterior->Nests, ++Exterior->Nest_number);
    Exterior->Nests[Exterior->Nest_number-1] = new_nest;

    if(debug_msgs){
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
        for(int i = 1; i < nest->Ant_number; i++){
            free_ant(nest->Ant_list[i]);
        }

        // On retire la fourmilière de l'Exterior
        Exterior* ext = nest->Exterior;
        for (int i = 0; i < ext->Nest_number; i++) {
            if (ext->Nests[i] == nest) {
                ext->Nests[i] = ext->Nest[--ext->Nest_number];
                ext->Nests = realloc(ext->Nests, ext->Nest_number);
            }
        }


        simulation_data->nest_NB--;

        if(debug_msgs){
            printf("| DEBUG : ant \"%s\" freed\n", nest->Clan);
        }

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

    new_exterior->All_Ant_list = malloc(0);
    new_exterior->Total_Ant_number = 0;


    Room** created_rooms = malloc(size * sizeof(Room*));
    if (created_rooms == NULL) {
        perror("Échec de l'allocation mémoire pour la liste des salles");
    }
    for (int i = 0; i < size; i++) {
        created_rooms[i] = NULL;
    }


    for (int i = 0; i < size; i++) {
        created_rooms[i] = init_room(simulation_data, "Exterior", rand()%(600-500)+500); // Chaque salle a une taille aléatoire entre 500 et 600


        // On connecte la salle crée à jusqu'à trois autres salles
        for (int j = rand()%3+1; j>0; j++) {
            connect_rooms(created_rooms[rand()%i], created_rooms[i]);
        }
    }

    new_exterior->Entry = created_rooms[0];
    free(created_rooms);

    if(debug_msgs){
        printf("| DEBUG : new exterior initialized\n");
    }

    return new_exterior;
}

void free_exterior(Exterior* exterior){

    if(exterior != NULL) {
        while (0 < exterior->Nest_number) {
            free_nest(exterior->Nests[0]);
        }

        if(debug_msgs){
            printf("| DEBUG : exterior freed\n");
        }

        free(exterior);
    }
}

// Display
void print_numbers(Simulation_data* sim){
    printf("Nests : %d | Rooms : %d | Eggs : %d | Larves : %d | Ants : %d | Creas : %d | Objs : %d\n",
           sim->nest_NB, sim->room_IDs, sim->egg_NB, sim->larve_NB, sim->ant_NB, sim->crea_NB, sim->obj_NB);
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

    // Code à éxecuter une fois par room
    if (room->Ant_count == 0 && room->Creature_count == 0){//peut etre vérifier larve et oeufs aussi
        return;
    }
    for (int i =  0; i < room->Ant_count; i++){ //effectuer l'action sur chaque fourmis
        Action_ant(simulation_data, room->Ant_list[i]);
    }

    // Fin du code à éxecuter

    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(simulation_data, room->Connexion_list[i]);
    }
}

<<<<<<< HEAD


void simulation(Simulation_data* simulation_data, Exterior* exterior, int iterations) {
=======
void simulation(Simulation_data* simulation_data, Nest* nest, Exterior* exterior, int iterations) {
>>>>>>> refs/remotes/origin/main
    if (iterations == 0) {
        return;
    }

    simulation_data->tick++;
    simulation_data->counter++;

    if(simulation_data->counter >= 90){     // Passage à la saison suivante
        simulation_data->counter = 0;

        simulation_data->season_chain = simulation_data->season_chain->Next;
        simulation_data->current_season = simulation_data->season_chain->Number;
    }


    simuler_room(simulation_data, exterior->Entry);
    reinitialiser_rooms(simulation_data, exterior->Entry);

    simulation(simulation_data, exterior, iterations-1);
}

/* -----< Initialisation de la simulation >----- */
void start(Simulation_data* simulation_data, Nest** nest, Exterior** exterior){   // Lancer la simulation
    // Season* season = init_seasons(simulation_data, 0);
    srand(time(NULL)); // Pour rendre la simulation aléatoire

    //Création du monde

    // Création de la fourmilière
    Room* entry = init_room(simulation_data, "Nest Entrance", 20);
    int* pv_param = malloc(2*sizeof(int));
    pv_param[0] = 15;
    pv_param[1] = 5;
    int* dmg_param = malloc(2*sizeof(int));
    dmg_param[0] = 1;
    dmg_param[1] = 5;

    *nest = init_nest(simulation_data, "fourmia trèspetitus", "léptites fourmis", pv_param, dmg_param, 10, 100, 20, entry, NULL);

    /* Structure de la fourmilière initiale voulue:
     *                 entrée
     *                /      \
     * Chambre de repos      Stock de nourriture 1
     *           |            /       |
     *           |           /        |
     *        Stock de nouriture 2    |
     *         |               \      |
     *         |                Chambre de la reine
     *         |               /
     *        Chambre de larves
     */

    // Création des salles
    Room* resting_room = init_room(simulation_data, "Resting Room", 50);
    Room* food_room1 = init_room(simulation_data, "Storage Room", 50);
    Room* food_room2 = init_room(simulation_data, "Storage Room", 60);
    Room* queen_chamber = init_room(simulation_data, "Queen chamber", 20);
    Room* larva_room = init_room(simulation_data, "Larva chamber", 60);

    // Connection des salles
    connect_rooms(entry, resting_room);
    connect_rooms(entry, food_room1);
    connect_rooms(resting_room, food_room2);
    connect_rooms(food_room1, food_room2);
    connect_rooms(food_room1, queen_chamber);
    connect_rooms(food_room2, queen_chamber);
    connect_rooms(food_room2, larva_room);
    connect_rooms(queen_chamber, larva_room);


    // Génération de l'extérieur
    printf("Veuillez choisir une taille d'environnement pour la simulation. Nous recommandons entre 10 (très petit) et 300 (très grand) :\n");
    int room_number;
    scanf("%d", &room_number);
    *exterior = init_exterior(simulation_data, room_number);



    // On s'assure qu'une des salles est connectée à la fourmilière
    connect_rooms((*exterior)->Entry, entry);
    (*nest)->Exterior = *exterior;
}
