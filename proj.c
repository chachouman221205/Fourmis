#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

/* -----< Structures Principales >----- */
/*
Pour plus d'explications sur chacun des paramètres, ce référer au document pdf "Rapport modélisation projet"
Seules les explications importantes sont en commentaire
*/

    // Représente une pièce dans l'environnement
typedef struct Room {
    char *Name_ID;
    bool Visited;   // Pour l'exploration du graphe (graphe d'objets "Room")
    int Size;       // Taille max de la Room, limite d'objet et de fourmis pouvant entrer

    struct Ant **Ant_list;
    int Ant_count;

    struct Object **Obj_list;
    int Obj_count;

    struct Object **Creature_list;
    int Creature_count;

    struct Room **Connexion_list;
    int Connexion_list_size;

    struct Pheromone **Pheromone_stack;
} Room;

    // Représente l'extérieur de l'environnement (graphe de "Room")
typedef struct Exterior {
    struct Room *Entry;     // Pointeur vers l'entrée de l'environnement

    struct Ant **Ant_list;
    int Ant_number;
} Exterior;

    // Représente une fourmilière (graphe de "Room")
typedef struct Nest {
    char *Specie;
    char *Clan;     // Identifiant du clan auquel appartient la fourmilière

        // Liste des paramètres des fourmis de cette nest
    int* PV;        // tableau des PVs en fonction du type de fourmi : {1, 5, 10}; 1 ouvrière, 5 garde, 10 reine
    int* DMG;       // comme pour les PVs
    int Life_min;   // random pour plus de diversité
    int Life_max;
    int Hunger;

    struct Room *Entry;     // Pointeur vers la pièce d'entrée de la fourmilière

    struct Ant **Ant_list;
    int Ant_number;
} Nest;

    // Représente un oeuf de fourmi
typedef struct Egg {
    char *Name_ID;
    int Ant_type;

    int PV;

    int Grow;       // nombre de ticks avant que la fourmi passe au stade supérieur
    int Hunger;     // Nombre d'itérations avant que la fourmi ait besoin de manger, meurt si <= -5
        // La fourmi a faim quand hunger = 0, et doit manger en moins de 5 itérations pour survivre

    char *Clan;
    Nest* Nest;
    struct Room *Position;      // Pointeur vers la pièce où se trouve la fourmi
} Egg;

    // Représente une larve de fourmi
typedef struct Larve {
    char *Name_ID;
    int Ant_type;

    int PV;
    
    int Grow;       // nombre de ticks avant que la fourmi passe au stade supérieur
    int Hunger;     // Nombre d'itérations avant que la fourmi ait besoin de manger, meurt si <= -5
        // La fourmi a faim quand hunger = 0, et doit manger en moins de 5 itérations pour survivre

    char *Clan;
    Nest* Nest;
    struct Room *Position;      // Pointeur vers la pièce où se trouve la fourmi
} Larve;

    // Représente une fourmi
typedef struct Ant {
    char *Name_ID;
    int Ant_type;

    int PV;
    int DMG;

    int Life;       // Itérations restantes pour la fourmis, elle meurt quand Life = 0
    int Hunger;     // Nombre d'itérations avant que la fourmi ait besoin de manger, meurt si <= -5
        // La fourmi a faim quand hunger = 0, et doit manger en moins de 5 itérations pour survivre

    char *Clan;
    Nest* Nest;
    struct Room *Position;      // Pointeur vers la pièce où se trouve la fourmi
    struct Object *Held_object; // Pointeur vers l'objet que la fourmi transporte (NULL si aucun)
    struct Pheromone *Action;   // Pointeur vers la phéromone qui détermine son objectif
} Ant;

    // Représente un objet
typedef struct Object {
    char *Name_ID;

    int Size;       // Taille de l'objet
    bool Held;      // Indique si l'objet est transporté par une fourmi
} Object;

    // Représente une phéromone (stack)
typedef struct Pheromone {
    char *Action;   // Type ou action associée à la phéromone (pour la compréhension)
    int Density;    // Densité de la phéromone (indique sa force)
} Pheromone;

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

    // Représente les saisons (liste chainée bouclée)
typedef struct Season {
    char *Name;
    int Number;               // Numéro de la saison (1 à 4)
    struct Season *Next;
} Season;

typedef struct Simulation_data {
    int tick;           // Temps actuel dans la simulation
    int start_season;
    int counter;        // On change de saison et counter = 0 quand counter = 30;
    int current_season;
    struct Season* season_chain;

        // IDs
    int room_IDs;
    int egg_IDs;        // IDs++ à chaque nouvel(le) room/ant/object/predator, permet de ne jamais avoir 2 fois le meme id
    int obj_IDs;
    int crea_IDs;

        // Counts
    int nest_NB;
    int room_NB;
    int egg_NB;
    int larve_NB;
    int ant_NB;
    int obj_NB;
    int crea_NB;
} Simulation_data;


/* -----< Initialisation des variables globales >----- */
/*
Initialisation de certaines varaiables plus pratiques quand globales
*/

bool debug_msgs = 0;    // Printf de messages si = 1, pour le debug


/* -----< Récupération des variables de départ >----- */


void init_variables(Simulation_data* simulation){  // Récupère les scanf pour inititaliser des variables
    printf("Saison de départ : (1: Spring, 2: Summer, 3: Autumn, 4: Winter)");
    scanf(" %d", &simulation->start_season);
    
    // ...
}


/* -----< Fonctions et procédures >----- */
/*
Main project
*/
    // Déclaration
/*
Season* init_seasons();
void free_seasons();
Nest* init_nest();
void free_nest();
Exterior* init_exterior();
void free_exterior();
Room* init_room();
void connect_rooms();
void free_room();
int remaining_space();
int total_size();
Ant* init_new_ant();
//void Action_ant();
void free_ant();
void test_kill_ant();
void combat_ants();
Creature* init_creature();
void free_creature();
void test_kill_creature();
void combat_ant_creature();
Object* init_object();
void free_object();
*/

    // Seasons
void init_seasons(Simulation_data* simulation_data, int start_season){     // Saison de départ (0 à 3) : 0 = spring, 1 = summer, 2 = autumn, 3 = winter
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
    new_nest->Entry = entry;

    if(debug_msgs){
        printf("| DEBUG : new nest \"%s\" initialized\n", new_nest->Clan);
    }

    return new_nest;
}

void free_nest(Simulation_data* simulation_data, Nest* nest){ // à modifier par nassim
    if(nest != NULL){
        if(debug_msgs){
            printf("| DEBUG : ant \"%s\" freed\n", nest->Clan);
        }
        /*
        for(int i = 1; i < nest->Ant_number; i++){
            free_ant(nest->Ant_list[i]);
        }
        */
        free(nest);
        simulation_data->nest_NB--;
    }
}

    // Exterior
Exterior* init_exterior(Room* entry){
    Exterior* new_exterior = malloc(sizeof(Exterior));
    if(new_exterior == NULL){
        perror("Échec de l'allocation mémoire pour l'exterieur");
        return NULL;
    }

    new_exterior->Entry = entry;
    new_exterior->Ant_list = malloc(0);
    new_exterior->Ant_number = 0;

    if(debug_msgs){
        printf("| DEBUG : new exterior initialized\n");
    }

    return new_exterior;
}

void free_exterior(Exterior* exterior){ // à modifier par nassim
    if(exterior != NULL){
        if(debug_msgs){
            printf("| DEBUG : exterior freed\n");
        }
        /*
        for(int i = 1; i < exterior->Ant_number; i++){
            free_ant(exterior->Ant_list[i]);
        }
        */
        free(exterior);
    }
}

    // Rooms
Room* init_room(Simulation_data* simulation_data, char* name_ID, int size){
    Room* new_room = malloc(sizeof(Room));
    if(new_room == NULL){
        perror("Échec de l'allocation mémoire pour la pièce");
        return NULL;
    }

    simulation_data->room_NB++;

    if (name_ID != NULL) {
        new_room->Name_ID = name_ID;
        simulation_data->room_IDs++;
    } else {
        sprintf(new_room->Name_ID, "Room%d", simulation_data->room_IDs++);
    }
    new_room->Visited = false;  // pas visité à l'initialisation
    new_room->Size = size;
    new_room->Ant_list = malloc(0);
    new_room->Ant_count = 0;
    new_room->Obj_list = malloc(0);
    new_room->Obj_count = 0;
    new_room->Creature_list = malloc(0);
    new_room->Creature_count = 0;
    new_room->Connexion_list = malloc(0);
    new_room->Connexion_list_size = 0;

    new_room->Pheromone_stack = NULL;

    if(debug_msgs){
        printf("| DEBUG : new room \"%s\" initialized\n", new_room->Name_ID);
    }

    return new_room;
}

void connect_rooms(Room* room1, Room* room2) {

    if (room1 == NULL || room2 == NULL) {
        printf("ERROR : attempting connection to a non existing room (\"%s\" - \"%s\")", room1->Name_ID, room2->Name_ID);
        exit(1);
    }

    if (room1 == room2) {
        printf("ERROR : Can't connect a room to itself: \"%s\"", room1->Name_ID);
        exit(1);
    }

    // Verifier si les salles sont déjà connectées
    int existing_connection_status = 0;
    for (int i = 0; i < room1->Connexion_list_size; i++) {
        if (room1->Connexion_list[i] == room2) {
            existing_connection_status++;
            break;
        }
    }
    for (int i = 0; i < room2->Connexion_list_size; i++) {
        if (room2->Connexion_list[i] == room1) {
            existing_connection_status++;
            break;

        }
    }
    

    if (existing_connection_status == 2) {
        return; // Les deux rooms sont déjà connectés
    }
    if (existing_connection_status == 1) {
        printf("ERROR : room \"%s\" and room \"%s\" have a bad connection", room1->Name_ID, room2->Name_ID);
        exit(1);
    }

    // Ajustement de la taille de chaque tableau
    room1->Connexion_list_size++;
    room1->Connexion_list = realloc(room1->Connexion_list, room1->Connexion_list_size * sizeof(Room*));
    room2->Connexion_list_size++;
    room2->Connexion_list = realloc(room2->Connexion_list, room2->Connexion_list_size * sizeof(Room*));

    if(room1->Connexion_list == NULL || room2->Connexion_list){
        perror("Échec de l'allocation mémoire pour la connexion entre pièces");
        exit(1);
    }

    // Ajouter une connection
    room1->Connexion_list[room1->Connexion_list_size-1] = room2;
    room2->Connexion_list[room2->Connexion_list_size-1] = room1;
}

void free_room(Simulation_data* simulation_data, Room* room){
    if(room != NULL){
        if(debug_msgs){
            printf("| DEBUG : room \"%s\" freed\n", room->Name_ID);
        }

        // On retire les connections avec les autres Rooms
        Room* R;
        for (int i = 0; i < room->Connexion_list_size; i++) {
            R = room->Connexion_list[i];
            // chercher la connection
            for (int j = 0; j < R->Connexion_list_size; j++) {
                if (R->Connexion_list[j] == room) {
                    R->Connexion_list[j] = NULL; // On retire la connection
                }
            }
        }

        free(room->Ant_list);
        free(room->Obj_list);
        free(room->Creature_list);
        free(room->Connexion_list);
        free(room);

        simulation_data->room_NB--;
    }
}

int remaining_space(Room* room) {
    int space_used = 0;
    // Objects
    for (int i = 0; i < room->Obj_count; i++) {
        if (room->Obj_list[i] != NULL) {
            space_used += room->Obj_list[i]->Size;
        }
    }

    // Ants
    space_used += room->Ant_count;

    // Creatures
    for (int i = 0; i < room->Creature_count; i++) {
        if (room->Creature_list[i] != NULL) {
            space_used += room->Creature_list[i]->Size;
        }
    }

    return room->Size - space_used;
}

int total_size(Ant* ant){
    if(debug_msgs){
        printf("| DEBUG : total size of ant \"%s\" = %d\n", ant->Name_ID, 1 + ant->Held_object->Size);
    }
    if (ant->Held_object == NULL) {
        return 1;
    }
    return 1 + ant->Held_object->Size; // 1 = ant size
}

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

    if(debug_msgs){
        printf("| DEBUG : new egg \"%s\" initialized in nest \"%s\"\n", new_egg->Name_ID, nest->Clan);
    }

    return new_egg;
}

void free_egg(Simulation_data* simulation_data, Egg* egg){
    if(egg != NULL){
        if(debug_msgs){
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
            if(debug_msgs){
                printf("| DEBUG : egg \"%s\" died : ", egg->Name_ID);
                printf(death_message[condition-1], (condition == 1)? egg->PV : egg->Hunger);
            }
            free_egg(simulation_data, egg);
        }
    }

}

bool test_grow_egg(Egg* egg){
    if(egg != NULL){
        if(egg->Grow <= 0){
            if(debug_msgs){
                printf("| DEBUG : egg \"%s\" can evolve", egg->Name_ID);
            }
            return true;
        }

        if(debug_msgs){
            printf("| DEBUG : egg \"%s\" cannot evolve", egg->Name_ID);
        }
        return false;
    }
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

    if(debug_msgs){
        printf("| DEBUG : new larve \"%s\" initialized in nest \"%s\"\n", new_larve->Name_ID, egg->Nest->Clan);
    }
    free_egg(simulation_data, egg);
    return new_larve;
}

void free_larve(Simulation_data* simulation_data, Larve* larve){
    if(larve != NULL){
        if(debug_msgs){
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
            if(debug_msgs){
                printf("| DEBUG : larve \"%s\" died : ", larve->Name_ID);
                printf(death_message[condition-1], (condition == 1)? larve->PV : larve->Hunger);
            }
            free_larve(simulation_data, larve);
        }
    }

}

bool test_grow_larve(Larve* larve){
    if(larve != NULL){
        if(larve->Grow <= 0){
            if(debug_msgs){
                printf("| DEBUG : larve \"%s\" can evolve", larve->Name_ID);
            }
            return true;
        }
        
        if(debug_msgs){
            printf("| DEBUG : larve \"%s\" cannot evolve", larve->Name_ID);
        }
        return false;
    }
}

    // Ants
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



    if(debug_msgs){
        printf("| DEBUG : new ant \"%s\" initialized in nest \"%s\"\n", new_ant->Name_ID, larve->Nest->Clan);
    }

    free_larve(simulation_data, larve);
    return new_ant;
}

void attach_ant_to_nest(Ant* ant, Nest* nest) {
    ant->Nest = nest;
    ant->Clan = nest->Clan;
    nest->Ant_list = realloc(nest->Ant_list, (nest->Ant_number++) * sizeof(Ant*));
}

void Action_ant(Simulation_data* simulation_data, Ant* ant){    //fonction qui défini l'action d'une fourmis ouvrière/reine lors du cycle 
    char salle_de_ponte[] = "salle de ponte";
    int condition = strcmp(ant->Position, salle_de_ponte);
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
        pass();
    }
    ant->Hunger--;
    ant->Life--;
    //faire vérif si il meurt ou pas
}

void free_ant(Simulation_data* simulation_data, Ant* ant){
    if(ant != NULL){
        if(ant->Held_object != NULL){ // Si la fourmi porte un objet, on le pose dans la salle avant de free la fourmi
            ant->Held_object->Held = false;
            ant->Position->Obj_count++;
            ant->Position->Obj_list = realloc(ant->Position->Obj_list, ant->Position->Obj_count * sizeof(Object*));
            ant->Position->Obj_list[ant->Position->Obj_count-1] = ant->Held_object;
        }
        if(debug_msgs){
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
            if(debug_msgs){
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

    if(debug_msgs){
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

    // Objects
Object* init_object(Simulation_data* simulation_data, char* name_ID, int size, bool held){
    Object* new_obj = malloc(sizeof(Object));
    if(new_obj == NULL){
        perror("Échec de l'allocation mémoire pour l'objet");
        return NULL;
    }

    simulation_data->obj_NB++;

    if (name_ID != NULL) {
        new_obj->Name_ID = name_ID;
        simulation_data->obj_IDs++;
    } else {
        sprintf(new_obj->Name_ID, "Obj%d", simulation_data->obj_IDs++);
    }
    new_obj->Size = size;
    new_obj->Held = held;

    if(debug_msgs){
        printf("| DEBUG : new obj \"%s\" initialized\n", new_obj->Name_ID);
    }
}

void free_object(Simulation_data* simulation_data, Object* object){
    if(object != NULL){
        if(debug_msgs){
            printf("| DEBUG : object \"%s\" freed\n", object->Name_ID);
        }
        free(object);
        simulation_data->obj_NB--;
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
        return 0;
    }
    for (int i = room->Ant_count ; i > 0 ; i--){ //effectuer l'action sur chaque fourmis
        Action_ant(simulation_data, room->Ant_list[i]);
    }

    // Fin du code à éxecuter

    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(simulation_data, room->Connexion_list[i]);
    }
}

void reinitialiser_rooms(Simulation_data* simulation_data, Room* room) {
    if (!room->Visited) {
        return;
    }
    room->Visited = false;
    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(simulation_data, room->Connexion_list[i]);
    }
}

void simulation(Simulation_data* simulation_data, Nest* nest, Exterior* exterior, int iterations) {
    if (iterations == 0) {
        return;
    }
    
    simulation_data->tick++;
    simulation_data->counter++;

    if(simulation_data->counter >= 50){     // Passage à la saison suivante
        simulation_data->counter = 0;

        simulation_data->season_chain = simulation_data->season_chain->Next;
        simulation_data->current_season = simulation_data->season_chain->Number;
    }

    simuler_room(simulation_data, nest->Entry);
    simuler_room(simulation_data, exterior->Entry);
    reinitialiser_rooms(simulation_data, exterior->Entry);
    reinitialiser_rooms(simulation_data, nest->Entry);

    simulation(simulation_data, nest, exterior, iterations-1);
}

/* -----< Initialisation de la simulation >----- */
void start(Simulation_data* simulation_data, Nest** nest, Exterior** exterior){   // Lancer la simulation
    srand(time(NULL)); // Pour rendre la simulation aléatoire

    //Création du monde

    // Création de la fourmilière
    Room* nest_entrance = init_room(simulation_data, "Nest Entrance", 20);
    int* pv_param = malloc(2*sizeof(int));
    pv_param[0] = 15;
    pv_param[1] = 5;
    int* dmg_param = malloc(2*sizeof(int));
    dmg_param[0] = 1;
    dmg_param[1] = 5;

    *nest = init_nest(simulation_data, "fourmia trèspetitus", "léptites fourmis", pv_param, dmg_param, 10, 100, 20, nest_entrance);

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
    connect_rooms(nest_entrance, resting_room);
    connect_rooms(nest_entrance, food_room1);
    connect_rooms(resting_room, food_room2);
    connect_rooms(food_room1, food_room2);
    connect_rooms(food_room1, queen_chamber);
    connect_rooms(food_room2, queen_chamber);
    connect_rooms(food_room2, larva_room);
    connect_rooms(queen_chamber, larva_room);


    // Génération de l'extérieur
    *exterior = init_exterior(nest_entrance);
    printf("Veuillez choisir une taille d'environnement pour la simulation. Nous recommandons entre 10 (très petit) et 300 (très grand) :\n");
    int room_number;
    scanf("%d", &room_number);

    Room** created_rooms = malloc(room_number * sizeof(Room*));
    if (created_rooms == NULL) {
        perror("Échec de l'allocation mémoire pour la liste des salles");
    }
    for (int i = 0; i < room_number; i++) {
        created_rooms[i] = NULL;
    }


    for (int i = 0; i < room_number; i++) {
        created_rooms[i] = init_room(simulation_data, "Exterior", rand()%(600-500)+500); // Chaque salle a une taille aléatoire entre 500 et 600


        // On connecte la salle crée à jusqu'à trois autres salles
        for (int j = rand()%3+1; j>0; j++) {
            connect_rooms(created_rooms[rand()%i], created_rooms[i]);
        }
    }

    // On s'assure qu'une des salles est connectée à la fourmilière
    connect_rooms(created_rooms[0], nest_entrance);
    free(created_rooms);
}


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