# ⚔️ Darkest C Dungeon


Darkest C Dungeon est une version simplifiée d'un jeu de rôle (RPG) développée entièrement en langage C. Le projet met l'accent sur la gestion dynamique des données via des listes chaînées.

## Auteure


Inès Benhamida 

## Gameplay


Système de combat : Combats au tour par tour contre des ennemis de plus en plus forts (10 niveaux au total).


Gestion du stress : Une jauge de stress qui, si elle atteint 100%, rend le personnage inutilisable.


Équipement : Possibilité d'acheter et d'équiper des accessoires pour améliorer les statistiques (Attaque, Défense, HP, Repos).


Lieux de repos : Un Sanitarium pour soigner les points de vie et une Taverne pour réduire le stress.

##  Aspects Techniques


Structures de données : Utilisation intensive de listes chaînées pour les personnages, les ennemis et l'inventaire.


Gestion mémoire : Allocation et libération dynamique de la mémoire pour éviter les fuites.


Sauvegarde : Possibilité de sauvegarder et charger une partie via des fichiers texte.

##  Compilation et Exécution Pour compiler le projet, utilisez clang (ou gcc) avec la commande suivante:

clang -std=c17 -Wall Projet_darkest_c_dungeon.c -o darkest_dungeon

Puis lancez le jeu :

./darkest_dungeon
