#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* -----< Structures Principales >----- */
/*
Pour plus d'explications sur chacun des paramètres, ce référer au document pdf "Rapport modélisation projet"
Seules les explications importantes sont en commentaire
*/

    // Représente une pièce dans l'environnement
typedef struct Room {
    char *Name_ID;
    struct Room *Entry;
    bool Visited;   // Pour l'exploration du graphe (graphe d'objets "Room")
    int Size;       // Taille max de la Room

    struct Ant **Ant_list;
    int Ant_count;

    struct Object **Obj_list;
    int Obj_count;
    int Max_obj_nb;

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

    struct Room *Entrance;     // Pointeur vers la pièce d'entrée de la fourmilière

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
int start_season;       // Saison de départ (0 à 3) : 0 = spring, 1 = summer, 2 = autumn, 3 = winter
int IDs = 0;            // IDs++ à chaque nouvel ant/object/predator, permet de ne jamais avoir 2 fois le meme id


/* -----< Fonctions et procédures >----- */
/*
Main project
*/

    // Seasons
Season* init_seasons(){
    Season *spring = malloc(sizeof(Season));
    if(spring == NULL){
        perror("Échec de l'allocation pour spring");
        return -1;
    }
    spring->Name = "Spring";
    spring->Number = 0;

    Season *summer = malloc(sizeof(Season));
    if(summer == NULL){     // Si echec d'allocation, on free toutes les saisons déjà allouées
        perror("Échec de l'allocation pour summer");
        free(spring);
        return -1;
    }
    summer->Name = "Summer";
    summer->Number = 1;

    Season *autumn = malloc(sizeof(Season));
    if(autumn == NULL){
        perror("Échec de l'allocation pour autumn");
        free(spring);
        free(summer);
        return -1;
    }
    autumn->Name = "Autumn";
    autumn->Number = 2;

    Season *winter = malloc(sizeof(Season));
    if(winter == NULL){
        perror("Échec de l'allocation pour winter");
        free(spring);
        free(summer);
        free(autumn);
        return -1;
    }
    winter->Name = "Winter";
    winter->Number = 3;

    // Chaînage des saisons : Boucle cyclique
    spring->Next = summer;
    summer->Next = autumn;
    autumn->Next = winter;
    winter->Next = spring;

    if(debug_msgs){
        printf("| DEBUG : seasons initialized\n");
    }
    
    Season* tab[] = {spring, summer, autumn, winter};
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
Nest* init_nest(char* specie, char* clan, int pv, int dmg, int life_min, int life_max, int hunger, Ant** ant_list, int ant_number){
    Nest* new_nest = malloc(sizeof(Nest));
    if(new_nest == NULL){
        perror("Échec de l'allocation mémoire pour la nest");
        return NULL;
    }

    new_nest->Specie = specie;
    new_nest->Clan = clan;
    new_nest->PV = pv;
    new_nest->DMG = dmg;
    new_nest->Life_min = life_min;
    new_nest->Life_max = life_max;
    new_nest->Hunger = hunger;
    new_nest->Ant_list = ant_list;
    new_nest->Ant_number = ant_number;

    if(debug_msgs){
        printf("| DEBUG : new nest \"%s\" initialized\n", new_nest->Clan);
    }

    return new_nest;
}

void free_nest(Nest* nest){
    if(nest != NULL){
        if(debug_msgs){
            printf("| DEBUG : ant \"%s\" freed\n", nest->Clan);
        }
        free(nest);
    }
}

    // Exterior
Exterior* init_exterior(){

}

void free_exterior(Exterior exterior){

}

    // Rooms
Room* init_room(char* name_ID, Room* entry, bool visited, int size, Ant** ant_list, int ant_count, Object** obj_list, int obj_count, int max_object, Room** connexion_list, int connexion_list_size, Pheromone** pheromone_stack){
    Room* new_room = malloc(sizeof(Room));
    if(new_room == NULL){
        perror("Échec de l'allocation mémoire pour la pièce");
        return NULL;
    }

    new_room->Name_ID = name_ID;
    new_room->Entry = entry;
    new_room->Visited = 0;  // pas visité à l'initialisation
    new_room->Size = size;
    new_room->Ant_list = ant_list;
    new_room->Ant_count = ant_count;
    new_room->Obj_list = obj_list;
    new_room->Obj_count = obj_count;
    new_room->Max_obj_nb = max_object;
    new_room->Connexion_list = connexion_list;
    new_room->Connexion_list_size = connexion_list_size;

    new_room->Pheromone_stack = pheromone_stack;

    if(debug_msgs){
        printf("| DEBUG : new room \"%s\" initialized\n", new_room->Name_ID);
    }

    return new_room;
}

void free_room(Room* room){
    if(room != NULL){
        if(debug_msgs){
            printf("| DEBUG : room \"%s\" freed\n", room->Name_ID);
        }

        // On retire les connections avec les autres Rooms
        Room* R;
        for (int i = 0; i < room.Connexion_list_size; i++) {
            R = room.Connexion_list[i]
            // chercher la connection
            for (int j = 0; j < R.Connexion_list_size; j++) {
                if (R.Connexion_list[j] == room) {
                    R.Connexion_list[j] = Null; // On retire la connection
                }
            }
        }

        free(room);
    }
}

    // Ants
Ant* init_new_ant(Nest* nest, int ant_type, char *name, int PV, int DMG, int Hunger) {
    Ant* new_ant = malloc(sizeof(Ant));
    if(new_ant == NULL){
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    srand(time(NULL));  // Random pour Life de la fourmis

    // Initialisation des champs de la fourmi, on initialise en fonction de la nest
    (name != NULL)? (new_ant->Name_ID = name) : (sprintf(new_ant->Name_ID, "Ant%d", IDs++));
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

void free_ant(Ant* ant){
    if(ant != NULL){
        if(ant->Held_object != NULL){
            ant->Position->Obj_count++;
            if(ant->Position->Max_obj_nb > ant->Position->Obj_count){   // Si il reste de la place on "drop" l'obj dans la piece
                ant->Position->Obj_list[ant->Position->Obj_count] = ant->Held_object;
            }
            else{   // Sinon on le free
                free_object(ant->Held_object);
            }
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
                printf("| DEBUG : ant \"%s\" died : %s\n", ant->Name_ID, death_message[condition], 
                    (condition == 0)? (ant->PV) : ( (condition == 1)? (ant->Life) : (ant->Hunger) ) );
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

void combat_ant_crea(Ant* ant, Creature* crea){
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
        ant->Hunger += 15;  // la creature se nourrit
    }
    if(ant->PV <= 0){
        crea->Hunger += 5;  // la creature se nourrit
    }
    test_kill_ant(ant);
    test_kill_crea(crea);
}

    // Creatures
Creature* init_creature(char* name_ID, int pv, int dmg, int life, int hunger, Room* position){
    Creature* new_creature = malloc(sizeof(Creature));
    if(new_creature == NULL){
        perror("Échec de l'allocation mémoire pour la creature");
        return NULL;
    }
    
    new_creature->Name_ID = name_ID;
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

void test_kill_crea(Creature* crea){
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
                printf("| DEBUG : crea \"%s\" died : %s\n", crea->Name_ID, death_message[condition], 
                    (condition == 0)? (crea->PV) : ( (condition == 1)? (crea->Life) : (crea->Hunger) ) );
            }
            free_crea(crea);
        }
    }

}

    // Objects
Object* init_object(char* name_ID, int size, bool held){
    Object* new_obj = malloc(sizeof(Object));
    if(new_obj == NULL){
        perror("Échec de l'allocation mémoire pour l'objet");
        return NULL;
    }

    new_obj->Name_ID = name_ID;
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


/* -----< Récupération des variables de départ >----- */
void init_variables(){  // Récupère les scanf pour inititaliser des variables
    scanf(" %d", &tick); // à modifier
    // ...
}


/* -----< Initialisation de la simulation >----- */
void start(){   // Lancer la simulation 
    Season* season = init_seasons();
    // ...
}


/* -----< Main >----- */
int main(){
    printf(" \b"); // Pour éviter les problèmes de scanf, peut etre supprimé

    // ...

    return 0;
}
