#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* -----< Structures Principales >----- */
    // Représente une pièce dans l'environnement
typedef struct Room {
    char *Name_ID;             // Nom ou identifiant de la pièce
    struct Room *Entry;        // Pointeur vers une autre pièce (entrée de cette pièce)
    bool Visited;              // Indique si la pièce a été visitée
    int size;                  // Taille maximale de la pièce
    struct Ant **Ant_list;     // Liste des pointeurs vers les fourmis présentes dans la pièce
    int Ant_count;             // Nombre de fourmis dans la pièce
    struct Object **Obj_list;  // Liste des pointeurs vers les objets présents dans la pièce
    int Obj_count;             // Nombre d'objets dans la pièce
    struct Room **Connexion_list; // Liste des pointeurs vers les pièces voisines
    int Connexion_list_size;   // Nombre de connexions vers d'autres pièces
    struct Pheromone **Pheromone_stack; // Liste de phéromones présentes dans la pièce (pile)
} Room;

    // Représente l'extérieur de l'environnement
typedef struct Exterior {
    struct Room *Entry;        // Pointeur vers une pièce de l'environnement
    struct Ant **Ant_list;     // Liste des pointeurs vers toutes les fourmis à l'extérieur
    int Ant_number;            // Nombre total de fourmis présentes à l'extérieur
} Exterior;

    // Représente une fourmilière
typedef struct Nest {
    char *Species;             // Espèce de la fourmilière
    char *Clan;                // Identifiant du clan auquel appartient la fourmilière
    int Life_min;              // Vie min d'une fourmi (cf Ant)
    int Life_max;              // Vie max d'une fourmi (cf Ant)

    struct Room *Entrance;     // Pointeur vers la pièce d'entrée de la fourmilière
    struct Ant **Ant_list;     // Liste des pointeurs vers les fourmis de la fourmilière
    int Ant_number;            // Nombre total de fourmis dans la fourmilière
} Nest;

    // Représente une fourmi
typedef struct Ant {
    char *Name_ID;             // Nom ou identifiant de la fourmi
    int PV;                    // Points de vie de la fourmi
    int DMG;                   // Dégâts que la fourmi peut infliger
    int Hunger;                // Nombre d'itérations avant que la fourmi ait besoin de manger
    int Life;                  // Itérations restantes pour la fourmis, elle meurt quand Life = 0 
    char *Clan;                // Clan ou fourmilière d'origine
    struct Room *Position;     // Pointeur vers la pièce où se trouve la fourmi
    struct Object *Held_object; // Pointeur vers l'objet que la fourmi transporte (NULL si aucun)
    struct Pheromone *Action;  // Pointeur vers la phéromone qui détermine son objectif
} Ant;

    // Représente un objet
typedef struct Object {
    char *Name_ID;             // Nom ou identifiant de l'objet
    int size;                  // Taille de l'objet
    bool held;                 // Indique si l'objet est transporté par une fourmi
} Object;

    // Représente une phéromone
typedef struct Pheromone {
    char *Action;              // Type ou action associée à la phéromone
    int Density;               // Densité de la phéromone (indique sa force)
} Pheromone;

    // Représente une créature quelconque (prédateur ou autre)
typedef struct Creature {
    char *Name_ID;             // Nom ou type de la créature
    int PV;                    // Points de vie de la créature
    int DMG;                   // Dégâts que la créature peut infliger
    int Age;                   // Âge de la créature en itérations
    struct Room *Position;     // Pointeur vers la pièce où se trouve la créature
} Creature;

    // Représente les saisons (boucle)
typedef struct Season {
    char *name;               // Nom de la saison
    int number;               // Numéro de la saison (0 à 3)
    struct Season *next;      // Pointeur vers la saison suivante
} Season;


/* -----< Initialisation des variables globales >----- */
int tick = 0;   // temps actuel dans la simulation
int start_season;   // Saison de départ (0 à 3) : 0 = spring, 1 = summer, 2 = autumn, 3 = winter
int IDs = 0;    // IDs++ à chaque nouvel ant/object/predator, permet de ne jamais avoir 2 fois le meme id


/* -----< Fonctions et procédures >----- */
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
    if(summer == NULL){
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
        free(current); // Libérer le dernier nœud
    }
}

    // Ants
Ant* init_new_ant(char *name, int PV, int DMG, int Hunger, Nest nest) {
    Ant *new_ant = malloc(sizeof(Ant));
    if(new_ant == NULL){
        perror("Échec de l'allocation mémoire pour la fourmi");
        return NULL;
    }

    srand(time(NULL));  // Random pour Life de la fourmis

    // Initialisation des champs de la fourmi
    sprintf(new_ant->Name_ID, "Ant%d", IDs++);
    new_ant->PV = PV;
    new_ant->DMG = DMG;
    new_ant->Hunger = Hunger;
    new_ant->Life = nest.Life_min + rand() % (nest.Life_max - nest.Life_min + 1); 
        // Life entre Life_min et Life_max, dépends de la nest
    new_ant->Clan = nest.Clan;
    new_ant->Position = NULL;     // Position NULL au départ, assignation plus tard
    new_ant->Held_object = NULL;  // Pas d'objet au départ
    new_ant->Action = NULL;       // Pas de phéromone assignée au départ

    return new_ant; // Retourne un pointeur vers la fourmi nouvellement allouée
}

void test_kill_ant(Ant* ant){
    if(ant != NULL){

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