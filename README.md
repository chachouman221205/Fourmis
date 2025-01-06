# Projet C 2A : Simulation d'une fourmilière
Notre projet a été codé en tant que module. C'est à dire que pour l'utiliser, il faut créer son propre fichier .c et utiliser les fonctions fournies dans chacun des modules pour simuler une fourmilière. Un fichier demo.c contient un example de simulation.

## Compilation

Pour compiler le fichier demo.c, utilisez `make`

Pour compiler un autre fichier, utilisez `make MAIN_FILE=<fichier>`

## Outils de Debug :

Pour pouvoir voir tout ce qui se passe dans la simulation, il est possible de modifier la variable `<Simulation>->debug_msgs`
Plus la valeur de cette variable est haute, plus les messages seront détaillés.

Voici les différentes valeurs possibles pour `<Simulation>->debug_msgs`:
    0 : Aucun Messages
    1 : Affichage de la création d'oeufs, larves, et fourmis. Affichage des compteurs de la simulation
    2 : environment & saisons, nombre d'itération restantes
    3 : Créations et suppressions de salles
    4 : Valeurs spécifiques des fourmis
    5 : Affichage de la création des objets
    6 : Affichage du nombre de cycles restants avant l'évolution des oeufs et des larves
    7 : Affichage des vérifications pour la croissance des oeufs et larves
    8 : Affichage des étapes de libération des salles

## Codé par
BOUDEKHANI Nassim
COUPRIE Joseph
FAUVEAU Enzo
