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
    int size;       // Taille max de la Room

    struct Ant **Ant_list;
    int Ant_count;

    struct Object **Obj_list;
    int Obj_count;

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
    char *Species;
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

    int size;       // Taille de l'objet
    bool held;      // Indique si l'objet est transporté par une fourmi
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

    struct Room *Position;
} Creature;

    // Représente les saisons (liste chainée bouclée)
typedef struct Season {
    char *name;
    int number;               // Numéro de la saison (0 à 3)
    struct Season *next;
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
    spring->name = "Spring";
    spring->number = 0;

    Season *summer = malloc(sizeof(Season));
    if(summer == NULL){     // Si echec d'allocation, on free toutes les saisons déjà allouées
        perror("Échec de l'allocation pour summer");
        free(spring);
        return -1;
    }
    summer->name = "Summer";
    summer->number = 1;

    Season *autumn = malloc(sizeof(Season));
    if(autumn == NULL){
        perror("Échec de l'allocation pour autumn");
        free(spring);
        free(summer);
        return -1;
    }
    autumn->name = "Autumn";
    autumn->number = 2;

    Season *winter = malloc(sizeof(Season));
    if(winter == NULL){
        perror("Échec de l'allocation pour winter");
        free(spring);
        free(summer);
        free(autumn);
        return -1;
    }
    winter->name = "Winter";
    winter->number = 3;

    // Chaînage des saisons : Boucle cyclique
    spring->next = summer;
    summer->next = autumn;
    autumn->next = winter;
    winter->next = spring;

    if(debug_msgs){
        printf("| DEBUG : seasons initialized\n");
    }
    
    Season* tab[] = {spring, summer, autumn, winter};
    return tab[start_season];
}

void free_seasons(Season *season){
    if(season != NULL){
        Season *current = season;
        Season *next_season = current->next;

        while(current != season){
            free(current);
            current = next_season;
            next_season = current->next;
        }
        free(current); // Libérer la derniere saison

        if(debug_msgs){
            printf("| DEBUG : seasons freed\n");
        }
    }
}

    // Ants
Ant* init_new_ant(Nest* nest, int ant_type, char *name, int PV, int DMG, int Hunger) {
    Ant *new_ant = malloc(sizeof(Ant));
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

void test_kill_ant(Ant* ant){
    if(ant != NULL){
        char* death_message[] = {"PV <= %d", "Life = %d", "Hunger <= %d"};
        int condition = 0;
        
        if(debug_msgs){
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
                printf("| DEBUG : ant \"%s\" died : %s\n", ant->Name_ID, death_message[condition], 
                    (condition == 0)? (ant->PV) : ( (condition == 1)? (ant->Life) : (ant->Hunger) ) );
                free_ant(ant);
            }
        }
        else{
            if(ant->PV <= 0 || ant->Life <= 0 || ant->Hunger <= -5){
                free_ant(ant);
            }
        }

    }
}

void free_ant(Ant* ant){
    if(ant != NULL){
        if(debug_msgs){
            printf("| DEBUG : ant \"%s\" freed\n", ant->Name_ID);
        }
        free(ant);
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