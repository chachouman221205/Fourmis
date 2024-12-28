#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

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

    // Représente une fourmi
typedef struct Ant {
    char *Name_ID;

    int PV;
    int DMG;

    int Life;       // Itérations restantes pour la fourmis, elle meurt quand Life = 0
    int Hunger;     // Nombre d'itérations avant que la fourmi ait besoin de manger, meurt si <= -5
        // La fourmi a faim quand hunger = 0, et doit manger en moins de 5 itérations pour survivre

    char *Clan;
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
    int Number;               // Numéro de la saison (0 à 3)
    struct Season *Next;
} Season;


/* -----< Initialisation des variables globales >----- */
/*
Initialisation de certaines varaiables plus pratiques quand globales
*/

bool debug_msgs = 0;    // Printf de messages si = 1, pour le debug

int tick = 0;           // Temps actuel dans la simulation

int nest_nb = 0;
int room_IDs = 0;
int ant_IDs = 0;        // IDs++ à chaque nouvel(le) room/ant/object/predator, permet de ne jamais avoir 2 fois le meme id
int obj_IDs = 0;
int crea_IDs = 0;


/* -----< Récupération des variables de départ >----- */
void init_variables(){  // Récupère les scanf pour inititaliser des variables
    scanf(" %d", &tick); // à modifier
    // ...
}


/* -----< Fonctions et procédures >----- */
/*
Main project
*/

    // Seasons
Season* init_seasons(int start_season){     // Saison de départ (0 à 3) : 0 = spring, 1 = summer, 2 = autumn, 3 = winter
    Season *spring = malloc(sizeof(Season));
    if(spring == NULL){
        perror("Échec de l'allocation pour spring");
        exit(1);
    }
    spring->Name = "Spring";
    spring->Number = 0;

    Season *summer = malloc(sizeof(Season));
    if(summer == NULL){     // Si echec d'allocation, on free toutes les saisons déjà allouées
        perror("Échec de l'allocation pour summer");
        free(spring);
        exit(1);
    }
    summer->Name = "Summer";
    summer->Number = 1;

    Season *autumn = malloc(sizeof(Season));
    if(autumn == NULL){
        perror("Échec de l'allocation pour autumn");
        free(spring);
        free(summer);
        exit(1);
    }
    autumn->Name = "Autumn";
    autumn->Number = 2;

    Season *winter = malloc(sizeof(Season));
    if(winter == NULL){
        perror("Échec de l'allocation pour winter");
        free(spring);
        free(summer);
        free(autumn);
        exit(1);
    }
    winter->Name = "Winter";
    winter->Number = 3;

    // Chaînage des saisons : Boucle cyclique
    spring->Next = summer;
    summer->Next = autumn;
    autumn->Next = winter;
    winter->Next = spring;

    Season* tab[] = {spring, summer, autumn, winter};

    if(debug_msgs){
        printf("| DEBUG : seasons initialized (start season \"%s\"\n", tab[start_season]->Name);
    }
    
    return tab[start_season];
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
Nest* init_nest(char* specie, char* clan, int* pv, int* dmg, int life_min, int life_max, int hunger, Room* entry){
    Nest* new_nest = malloc(sizeof(Nest));
    if(new_nest == NULL){
        perror("Échec de l'allocation mémoire pour la nest");
        return NULL;
    }

    nest_nb++;

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

void free_nest(Nest* nest){ // à modifier par nassim
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
Room* init_room(char* name_ID, int size){
    Room* new_room = malloc(sizeof(Room));
    if(new_room == NULL){
        perror("Échec de l'allocation mémoire pour la pièce");
        return NULL;
    }

    if (name_ID != NULL) {
        new_room->Name_ID = name_ID;
        room_IDs++;
    } else {
        sprintf(new_room->Name_ID, "Room%d", room_IDs++);
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
        perror("ERROR : attempting connection to a non existing room (\"%s\" - \"%s\")", room1->Name_ID, room2->Name_ID);
        exit(1);
    }

    if (room1 == room2) {
        perror("ERROR : Can't connect a room to itself: \"%s\"", room1->Name_ID);
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
        perror("ERROR : room \"%s\" and room \"%s\" have a bad connection", room1->Name_ID, room2->Name_ID);
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

void free_room(Room* room){
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
    return 1 + ant->Held_object->Size; // 1 = ant size
}

    // Ants
Ant* init_new_ant(Nest* nest, int ant_type, char *name) {
    Ant* new_ant = malloc(sizeof(Ant));
    if(new_ant == NULL){
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    // Initialisation des champs de la fourmi, on initialise en fonction de la nest
    if (name != NULL) {
        new_ant->Name_ID = name;
        ant_IDs++;
    } else {
        sprintf(new_ant->Name_ID, "Ant%d", ant_IDs++);
    }
    new_ant->PV = nest->PV[ant_type];
    new_ant->DMG = nest->DMG[ant_type];
    new_ant->Hunger = nest->Hunger;
    new_ant->Life = nest->Life_min + rand() % (nest->Life_max - nest->Life_min + 1); // Life entre Life_min et Life_max
    new_ant->Clan = nest->Clan;
    new_ant->Position = NULL;     // Position NULL au départ, assignation plus tard
    new_ant->Held_object = NULL;  // Pas d'objet au départ
    new_ant->Action = NULL;       // Pas de phéromone assignée au départ

    if(debug_msgs){
        printf("| DEBUG : new ant \"%s\" initialized in nest \"%s\"\n", new_ant->Name_ID, nest->Clan);
    }

    return new_ant;
}

/*
void Action_ant(Ant* ant){    //fonction qui défini l'action d'une fourmis ouvrière/reine lors du cycle 
    if(ant->Name_ID == "queen"){  // actions possibles des reines
        if(ant->Hunger > 10 && ant->position == "salle de ponte"){ // si reinne a bien la nourriture requise (ici 10 pr l'expml ) et que reine bien doans "salle de ponte " alors --> ponte
            ant->Hunger = ant->Hunger - 10;   // on lui retire la nouriture utilisée
            init_new_ant(ant->nest, ?, ? , ? , ? , 5 );  // ici  problème , comment on défini la classe et le nom de la fourmis lors de la création ? 
            // ici faut rajouter une phéromnone qui indique qu'il faut déplacer la larve 
        }

    }
    else if(ant->Name_ID == "worker"){ // actions possibles des ouvrières
        pass();
    }

}
*/

void free_ant(Ant* ant){
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
    }
}

void test_kill_ant(Ant* ant){
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
                printf(death_message[condition], (condition == 1)? ant->PV : (condition == 2)? ant->Life : ant->Hunger);
            }
            free_ant(ant);
        }
    }

}

void combat_ants(Ant* ant1, Ant* ant2){
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
    test_kill_ant(ant1);
    test_kill_ant(ant2);
}



    // Creatures
Creature* init_creature(char* name_ID, int pv, int dmg, int life, int hunger, Room* position){
    Creature* new_creature = malloc(sizeof(Creature));
    if(new_creature == NULL){
        perror("Échec de l'allocation mémoire pour la creature");
        return NULL;
    }
    
    if (name_ID != NULL) {
        new_creature->Name_ID = name_ID;
        crea_IDs++;
    } else {
        sprintf(new_creature->Name_ID, "Crea%d", crea_IDs++);
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

void free_creature(Creature* creature){
    if(creature != NULL){
        if(debug_msgs){
            printf("| DEBUG : creature \"%s\" freed\n", creature->Name_ID);
        }
        free(creature);
    }
}

void test_kill_creature(Creature* crea){
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
            free_creature(crea);
        }
    }

}

void combat_ant_creature(Ant* ant, Creature* crea){
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
    test_kill_ant(ant);
    test_kill_creature(crea);
}

    // Objects
Object* init_object(char* name_ID, int size, bool held){
    Object* new_obj = malloc(sizeof(Object));
    if(new_obj == NULL){
        perror("Échec de l'allocation mémoire pour l'objet");
        return NULL;
    }

    if (name_ID != NULL) {
        new_obj->Name_ID = name_ID;
        obj_IDs++;
    } else {
        sprintf(new_obj->Name_ID, "Obj%d", obj_IDs++);
    }
    new_obj->Size = size;
    new_obj->Held = held;

    if(debug_msgs){
        printf("| DEBUG : new onj \"%s\" initialized\n", new_obj->Name_ID);
    }
}

void free_object(Object* object){
    if(object != NULL){
        if(debug_msgs){
            printf("| DEBUG : object \"%s\" freed\n", object->Name_ID);
        }
        free(object);
    }
}

    // Display
void print_numbers(){
    printf("Nests : %d | Rooms : %d | Ants : %d | Creas : %d | Objs : %d\n", nest_nb,room_IDs,ant_IDs,crea_IDs,obj_IDs);
}


void simuler_room(Room* room) {
    if (room->Visited) {
        return;
    }
    room->Visited = true;

    // Code à éxecuter une fois par room


    // Fin du code à éxecuter

    for (int i = 0; i < room->Connexion_list_size; i++) {
        simuler_room(room->Connexion_list[i]);
    }
}
void simulation(Nest* nest, Exterior* exterior, int iterations) {
    if (iterations == 0) {
        return;
    }
    simuler_room(exterior->Entry);

    simulation(nest, exterior, iterations-1);
}

/* -----< Initialisation de la simulation >----- */
void start(){   // Lancer la simulation 
    Season* season = init_seasons(0);
    srand(time(NULL)); // Pour rendre la simulation aléatoire
    // ...

    //Création du monde
    Room* nest_entrance = init_room("Nest Entrance", 20);
    int* pv_param = malloc(3*sizeof(int));
    pv_param[0] = 1;
    pv_param[1] = 5;
    pv_param[2] = 10;
    int* dmg_param = malloc(3*sizeof(int));
    dmg_param[0] = 1;
    dmg_param[1] = 5;
    dmg_param[2] = 1;
    Nest* nest = init_nest("fourmia trèspetitus", "léptites fourmis", pv_param, dmg_param, 1, 10, 50, nest_entrance);

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
     *       Chambre de larves
     */

    // Création des salles
    resting_room = init_room("Resting Room", 50);
    food_room1 = init_room("Storage Room", 50);
    food_room2 = init_room("Storage Room", 60);
    queen_chamber = init_room("Queen chamber", 20);
    larva_room = init_room("Larva chamber", 30);

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
    Exterior* exterior = init_exterior(nest_entrance);
    printf("Veuillez choisir une taille d'environnement pour la simulation. Nous recommandons entre 10 (très petit) et 300 (très grand) :\n");
    int room_number;
    scanf("%d", &room_number);

    Room** created_rooms = malloc(room_number * sizeof(Room*));
    for (int i = 0; i < room_number; i++) {
        created_rooms[i] = NULL;
    }


    for (int i = 0; i < room_number; i++) {
        created_rooms[i] = init_room("Exterior", rand()%(600-500)+500); // Chaque salle a une taille aléatoire entre 500 et 600


        // On connecte la salle crée à jusqu'à trois autres salles
        for (int j = rand()%3+1; j>0; j++) {
            connect_rooms(created_rooms[rand()%i], created_rooms[i]);
        }
    }

    // On s'assure qu'une des salles est connectée à la fourmilière
    connect_rooms(created_rooms[0], nest_entrance);
}



/* -----< Main >----- */
int main(){
    printf(" \b"); // Pour éviter les problèmes de scanf, peut etre supprimé


    // ...

    return 0;
}
