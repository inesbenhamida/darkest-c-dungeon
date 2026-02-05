#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// ----------------------------------------------------------------------------------------- //
// ------------------------------------- STRUCTURES ---------------------------------------- //
// ----------------------------------------------------------------------------------------- //

// Structure pour une classe
typedef struct {
    char nom[50];
    int att, def, HPmax, rest;
} Classe;

// Structure pour accessoire
typedef struct Accessoire {
    char nom[50];
    int attbonus, defbonus, HPbonus, restbonus, strred, prix;
    struct Accessoire *suivant;
} Accessoire;

// Structure pour un personnage
typedef struct Personnage {
    char nom[50];
    Classe classe;
    int HP, stress, nbcomb, defense_temporaire;
    Accessoire *accessoire1;
    Accessoire *accessoire2;
    struct Personnage *suivant;
} Personnage;

// Structure pour un ennemi
typedef struct {
    const char *nom;
    int niveau, attenn, defenn, HPenn, attstrenn;
} Ennemi;

// Variables globales
Personnage *disponibles = NULL, *combattants = NULL;
Personnage *sanitarium = NULL, *taverne = NULL;
Accessoire *roulotte = NULL, *inventaire = NULL;
int argent = 5, niveau = 1;

// Classes globales
Classe furie = {"Furie", 13, 0, 20, 0};
Classe vestale = {"Vestale", 3, 0, 20, 10};
Classe chasseur_primes = {"Chasseur de primes", 7, 3, 25, 3};
Classe maitre_chien = {"Maître chien", 10, 6, 17, 5};

// Constantes globales 
const char *noms_ennemis[10] = {
    "Gobelin débutant",         // Niveau 1
    "Bandit agile",             // Niveau 2
    "Chevalier sombre",         // Niveau 3
    "Mage des ombres",          // Niveau 4
    "Assassin spectral",        // Niveau 5
    "Dragonnet furieux",        // Niveau 6
    "Golem de pierre",          // Niveau 7
    "Seigneur des ténèbres",    // Niveau 8
    "Archidémon",               // Niveau 9
    "Dieu de la destruction"    // Niveau 10
};

const char *accessoires_ennemis[5] = {
    "Dague des bannis",
    "Bouclier fendu",
    "Anneau maudit",
    "Talisman ancien",
    "Cape invisible"
};

// ----------------------------------------------------------------------------------------- //
// ------------------------------------ DIVERS --------------------------------------- //
// ----------------------------------------------------------------------------------------- //

// Prototypes des fonctions
void afficher_classes();
void initialiser_roulotte();
void initialiser_inventaire();
double random_roll();
void normaliser_nom(char *nom);
void supprimer_personnage(Personnage **liste, Personnage *personnage);
void deplacer_personnages(Personnage **source, Personnage **destination);
void ajouter_personnage(Personnage **liste, const char *nom, Classe classe);
void afficher_personnages(Personnage *liste);
void liberer_personnages(Personnage **liste);
void envoyer_au_sanitarium(Personnage **disponibles, Personnage **sanitarium);
void soigner_au_sanitarium(Personnage *sanitarium);
void retourner_du_sanitarium(Personnage **sanitarium, Personnage **disponibles);
void envoyer_a_la_taverne(Personnage **disponibles, Personnage **taverne);
void reduire_stress_a_la_taverne(Personnage *taverne);
void retourner_de_la_taverne(Personnage **taverne, Personnage **disponibles);
void ajouter_accessoire(Accessoire **liste, const char *nom, int att, int def, int HP, int rest, int strred, int prix);
Accessoire *retirer_accessoire(Accessoire **liste, const char *nom);
void afficher_accessoires(const Accessoire *liste);
void gerer_roulotte();
void equiper_accessoire(Personnage *personnage, Accessoire **inventaire);
void retourner_accessoires(Personnage *personnage, Accessoire **inventaire);
void recuperer_accessoire_sur_ennemi();
void liberer_accessoires(Accessoire **liste);
void selectionner_combattants(Personnage **disponibles, Personnage **combattants, int max_combattants);
void combat(Personnage **equipe, Ennemi *ennemi);
void gerer_personnages_morts(Personnage **liste);
void sauvegarder_partie(const char *nom_fichier);
int charger_partie(const char *nom_fichier);

void afficher_classes() {                       // Permet d'afficher les classes de personnages dans le jeu
    printf("\n** Classes disponibles **\n");
    printf("Furie : Att = 13 | Déf = 0 | HPmax = 20 | Rest = 0\n");
    printf("Vestale : Att = 3 | Déf = 0 | HPmax = 20 | Rest = 10\n");
    printf("Chasseur de primes : Att = 7 | Def = 3 | HPmax = 25 | Rest = 3\n");
    printf("Maître chien : Att = 10 | Déf = 6 | HPmax = 17 | Rest = 5\n\n");
}

void initialiser_roulotte() {               // Permet d'initialiser les accessoires disponibles à l'achat dans la roulotte
    ajouter_accessoire(&roulotte, "Dague perdue", 5, 0, 2, 3, 0, 7);
    ajouter_accessoire(&roulotte, "Pierre de protection", 0, 12, 5, 2, 0, 35);
    ajouter_accessoire(&roulotte, "Bottes furtives", 5, 0, 0, 2, 8, 16);
    ajouter_accessoire(&roulotte, "Bouclier miroir", 2, 0, 0, 0, 0, 7);
    ajouter_accessoire(&roulotte, "Bagues d’assurance", 0, 5, 10, 0, 0, 16);
}

void initialiser_inventaire() {             // Les deux accessoires de départ
    ajouter_accessoire(&inventaire, "Pendentif tranchant", 5, 1, 3, 0, 0, 0);
    ajouter_accessoire(&inventaire, "Calice de jeunesse", 0, 3, 5, 0, 5, 0);
}

double random_roll() {
    return 0.8 + ((double)rand() / RAND_MAX) * 0.4;
}

void normaliser_nom(char *nom) {
    int j = 0; // Index pour écrire dans le même buffer
    for (int i = 0; nom[i]; i++) {
        // On convertit les majuscules en minuscules
        if (nom[i] >= 'A' && nom[i] <= 'Z') {
            nom[j++] = nom[i] + 32;
        }
        // On traite les caractères accentués (UTF-8 simplifié)
        else if ((unsigned char)nom[i] == 0xC3) {
            i++; 
            if ((unsigned char)nom[i] == 0xA9 || (unsigned char)nom[i] == 0xA8) { // é ou è
                nom[j++] = 'e';
            } else if ((unsigned char)nom[i] == 0xA0) { // à
                nom[j++] = 'a';
            } else if ((unsigned char)nom[i] == 0xAA) { // ê
                nom[j++] = 'e';
            } else if ((unsigned char)nom[i] == 0xB4 || (unsigned char)nom[i] == 0xB2) { // ó ou ò
                nom[j++] = 'o';
            } else if ((unsigned char)nom[i] == 0x89) { // É
                nom[j++] = 'e';
            }
        }
        // Autres caractères valides
        else if (isalpha((unsigned char)nom[i]) || isdigit((unsigned char)nom[i]) || nom[i] == ' ') {
            nom[j++] = nom[i];
        }
    }
    nom[j] = '\0'; // Terminer la chaîne

    // Supprimer les espaces de début et fin
    char *debut = nom;
    while (*debut == ' ') debut++; // Retirer espaces au début
    memmove(nom, debut, strlen(debut) + 1);
}

// ----------------------------------------------------------------------------------------- //
// ------------------------------- GESTION DES PERSONNAGES --------------------------------- //
// ----------------------------------------------------------------------------------------- //

void supprimer_personnage(Personnage **liste, Personnage *personnage) {
    Personnage *courant = *liste, *precedent = NULL;
    while (courant != NULL && courant != personnage) {
        precedent = courant;
        courant = courant->suivant;
    }
    if (courant != NULL) {
        if (precedent == NULL) {
            *liste = courant->suivant;
        } else {
            precedent->suivant = courant->suivant;
        }
        free(courant);
    }
}

void deplacer_personnages(Personnage **source, Personnage **destination) {
    while (*source) {
        Personnage *temp = *source;     // Premier personnage de la source
        *source = (*source)->suivant;   // Avancer dans la source

        // Ajouter le personnage au début de la destination
        temp->suivant = *destination;
        *destination = temp;
    }
}

void ajouter_personnage(Personnage **liste, const char *nom, Classe classe) {
    Personnage *nouveau = malloc(sizeof(Personnage));
    if (!nouveau) {
        printf("Erreur d'allocation mémoire pour le personnage.\n");
        return;
    }

    strcpy(nouveau->nom, nom);
    nouveau->classe = classe;
    nouveau->HP = classe.HPmax;
    nouveau->stress = 0;
    nouveau->nbcomb = 0;
    nouveau->defense_temporaire = 0;
    nouveau->accessoire1 = NULL;
    nouveau->accessoire2 = NULL;
    nouveau->suivant = *liste;
    *liste = nouveau;

    // Affiche les informations de la classe et des statistiques
    printf("%s de classe %s a rejoint l'équipe !\n", nom, classe.nom);
    printf("Stats : Att = %d | Déf = %d | HPmax = %d | Rest = %d\n",
           classe.att, classe.def, classe.HPmax, classe.rest);
}


void afficher_personnages(Personnage *liste) {
    if (!liste) {
        printf("Aucun personnage dans cette liste.\n");
        return;
    }

    while (liste) {
        printf("- %s | Classe: %s | HP: %d/%d | Stress: %d | Combats: %d\n",
               liste->nom, liste->classe.nom, liste->HP, liste->classe.HPmax,
               liste->stress, liste->nbcomb);
        liste = liste->suivant;
    }
}

void liberer_personnages(Personnage **liste) {
    while (*liste) {
        Personnage *temp = *liste;
        *liste = (*liste)->suivant;
        free(temp);
    }
}

void envoyer_au_sanitarium(Personnage **disponibles, Personnage **sanitarium) {
    int max_sanitarium = 2;
    printf("\n** Liste des personnages disponibles pour le Sanitarium **\n");
    afficher_personnages(*disponibles);

    for (int i = 0; i < max_sanitarium; i++) {
        printf("\nEntrez le nom du personnage à envoyer au Sanitarium (ou 'stop' pour terminer) : ");
        char nom[50];
        fgets(nom, sizeof(nom), stdin);
        nom[strcspn(nom, "\n")] = '\0'; // Retirer le retour à la ligne

        if (strcasecmp(nom, "stop") == 0) {
            printf("Aucun personnage supplémentaire envoyé au Sanitarium.\n");
            break;
        }

        Personnage *courant = *disponibles, *precedent = NULL;

        // Recherche du personnage dans la liste des disponibles
        while (courant) {
            if (strcasecmp(courant->nom, nom) == 0) { // Comparaison insensible à la casse
                // Retirer le personnage de la liste des disponibles
                if (precedent == NULL) {
                    *disponibles = courant->suivant;
                } else {
                    precedent->suivant = courant->suivant;
                }

                // Ajouter le personnage au Sanitarium
                courant->suivant = *sanitarium;
                *sanitarium = courant;

                printf("%s a été envoyé au Sanitarium.\n", courant->nom);
                break;
            }
            precedent = courant;
            courant = courant->suivant;
        }

        if (!courant) {
            printf("Personnage non trouvé. Réessayez.\n");
            i--; // Redemander pour le même personnage
        }
    }
}

void soigner_au_sanitarium(Personnage *sanitarium) {
    printf("\n** Soins au Sanitarium **\n");
    while (sanitarium) {
        int hp_restores = 7;
        sanitarium->HP += hp_restores;
        if (sanitarium->HP > sanitarium->classe.HPmax) {
            sanitarium->HP = sanitarium->classe.HPmax; // Limite des HP restaurés
        }
        printf("%s récupère %d HP au Sanitarium. HP actuels : %d/%d\n",
               sanitarium->nom, hp_restores, sanitarium->HP, sanitarium->classe.HPmax);
        sanitarium = sanitarium->suivant;
    }
}

void retourner_du_sanitarium(Personnage **sanitarium, Personnage **disponibles) {
    if (!*sanitarium) {
        printf("\nAucun personnage dans le Sanitarium.\n");
        return;
    }

    printf("\n** Retour des personnages du Sanitarium **\n");
    Personnage *courant = *sanitarium;
    Personnage *precedent = NULL;

    while (courant) {
        printf("\nVoulez-vous faire sortir %s du Sanitarium ? (oui/non) : ", courant->nom);
        char choix[10];
        fgets(choix, sizeof(choix), stdin);
        choix[strcspn(choix, "\n")] = '\0'; // Supprimer le retour à la ligne

        if (strcasecmp(choix, "oui") == 0) {
            printf("%s est sorti du Sanitarium et retourne à l'équipe.\n", courant->nom);

            // Retirer de la liste du Sanitarium
            if (precedent == NULL) {
                *sanitarium = courant->suivant;
            } else {
                precedent->suivant = courant->suivant;
            }

            // Ajouter à la liste des disponibles
            Personnage *temp = courant;
            courant = courant->suivant;

            temp->suivant = *disponibles;
            *disponibles = temp;
        } else if (strcasecmp(choix, "non") == 0) {
            printf("%s reste au Sanitarium.\n", courant->nom);
            precedent = courant;
            courant = courant->suivant;
        } else {
            printf("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.\n");
        }
    }
}


void envoyer_a_la_taverne(Personnage **disponibles, Personnage **taverne) {
    int max_taverne = 2;
    printf("\n** Liste des personnages disponibles pour la Taverne **\n");
    afficher_personnages(*disponibles);

    for (int i = 0; i < max_taverne; i++) {
        printf("\nEntrez le nom du personnage à envoyer à la Taverne (ou 'stop' pour terminer) : ");
        char nom[50];
        fgets(nom, sizeof(nom), stdin);
        nom[strcspn(nom, "\n")] = '\0'; // Retirer le retour à la ligne

        if (strcasecmp(nom, "stop") == 0) {
            printf("Aucun personnage supplémentaire envoyé à la Taverne.\n");
            break;
        }

        Personnage *courant = *disponibles, *precedent = NULL;

        // Recherche du personnage dans la liste des disponibles
        while (courant) {
            if (strcasecmp(courant->nom, nom) == 0) { // Comparaison insensible à la casse
                // Retirer le personnage de la liste des disponibles
                if (precedent == NULL) {
                    *disponibles = courant->suivant;
                } else {
                    precedent->suivant = courant->suivant;
                }

                // Ajouter le personnage à la Taverne
                courant->suivant = *taverne;
                *taverne = courant;

                printf("%s a été envoyé à la Taverne.\n", courant->nom);
                break;
            }
            precedent = courant;
            courant = courant->suivant;
        }

        if (!courant) {
            printf("Personnage non trouvé. Réessayez.\n");
            i--; // Redemander pour le même personnage
        }
    }
}

void reduire_stress_a_la_taverne(Personnage *taverne) {
    printf("\n** Réduction de stress à la Taverne **\n");
    while (taverne) {
        int stress_reduction = 25;
        taverne->stress -= stress_reduction;
        if (taverne->stress < 0) {
            taverne->stress = 0; // Limite inférieure du stress
        }
        printf("%s récupère %d points de stress à la Taverne. Stress actuel : %d%%\n",
               taverne->nom, stress_reduction, taverne->stress);
        taverne = taverne->suivant;
    }
}

void retourner_de_la_taverne(Personnage **taverne, Personnage **disponibles) {
    if (!*taverne) {
        printf("\nAucun personnage dans la Taverne.\n");
        return;
    }

    printf("\n** Retour des personnages de la Taverne **\n");
    Personnage *courant = *taverne;
    Personnage *precedent = NULL;

    while (courant) {
        printf("\nVoulez-vous faire sortir %s de la Taverne ? (oui/non) : ", courant->nom);
        char choix[10];
        fgets(choix, sizeof(choix), stdin);
        choix[strcspn(choix, "\n")] = '\0'; // Supprimer le retour à la ligne

        if (strcasecmp(choix, "oui") == 0) {
            printf("%s est sorti de la Taverne et retourne à l'équipe.\n", courant->nom);

            // Retirer de la liste de la Taverne
            if (precedent == NULL) {
                *taverne = courant->suivant;
            } else {
                precedent->suivant = courant->suivant;
            }

            // Ajouter à la liste des disponibles
            Personnage *temp = courant;
            courant = courant->suivant;

            temp->suivant = *disponibles;
            *disponibles = temp;
        } else if (strcasecmp(choix, "non") == 0) {
            printf("%s reste à la Taverne.\n", courant->nom);
            precedent = courant;
            courant = courant->suivant;
        } else {
            printf("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.\n");
        }
    }
}

// ----------------------------------------------------------------------------------------- //
// ------------------------------- GESTION DES ACCESSOIRES --------------------------------- //
// ----------------------------------------------------------------------------------------- //

void ajouter_accessoire(Accessoire **liste, const char *nom, int att, int def, int HP, int rest, int strred, int prix) {
    Accessoire *nouvel_accessoire = malloc(sizeof(Accessoire));
    if (!nouvel_accessoire) {
        printf("Erreur d'allocation mémoire pour l'accessoire.\n");
        return;
    }

    strcpy(nouvel_accessoire->nom, nom);
    nouvel_accessoire->attbonus = att;
    nouvel_accessoire->defbonus = def;
    nouvel_accessoire->HPbonus = HP;
    nouvel_accessoire->restbonus = rest;
    nouvel_accessoire->strred = strred;
    nouvel_accessoire->prix = prix;
    nouvel_accessoire->suivant = *liste;
    *liste = nouvel_accessoire;
}

Accessoire *retirer_accessoire(Accessoire **liste, const char *nom) {
    Accessoire *courant = *liste, *precedent = NULL;
    while (courant) {
        if (strcmp(courant->nom, nom) == 0) { // Accessoire trouvé
            if (precedent == NULL) {
                *liste = courant->suivant;
            } else {
                precedent->suivant = courant->suivant;
            }
            return courant; // Retourne l'accessoire retiré
        }
        precedent = courant;
        courant = courant->suivant;
    }
    return NULL; // Accessoire non trouvé
}

void afficher_accessoires(const Accessoire *liste) {
    if (!liste) {
        printf("Aucun accessoire disponible.\n");
        return;
    }
    while (liste) {
        printf("- %s | Att: %d | Déf: %d | HP: %d | Rest: %d | Stress Red: %d | Prix: %d or\n",
               liste->nom, liste->attbonus, liste->defbonus, liste->HPbonus,
               liste->restbonus, liste->strred, liste->prix);
        liste = liste->suivant;
    }
}

void gerer_roulotte() { 
    while (1) {
        printf("\n--- Accessoires disponibles ---\n");
        afficher_accessoires(roulotte);
        printf("\nVous avez %d pièces d'or.\n", argent);

        printf("\nActions :\n");
        printf("1. Acheter un accessoire\n");
        printf("2. Quitter la roulotte\n");
        printf("Votre choix : ");

        int choix;
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide, réessayez.\n");
            while (getchar() != '\n'); // Nettoyer le buffer
            continue;
        }

        if (choix == 1) {
            printf("Entrez le nom de l'accessoire à acheter : ");
            char nom_accessoire[50];
            getchar(); // Consomme le '\n' résiduel
            fgets(nom_accessoire, sizeof(nom_accessoire), stdin);

            // Nettoyage de la chaîne
            nom_accessoire[strcspn(nom_accessoire, "\n")] = 0; // Retire le retour à la ligne
            char *debut = nom_accessoire;
            while (*debut == ' ') debut++; // Retire espaces au début
            memmove(nom_accessoire, debut, strlen(debut) + 1); // Déplace la chaîne

            if (strlen(nom_accessoire) == 0) {
                printf("Nom d'accessoire vide. Réessayez.\n");
                continue;
            }

            // Normaliser le nom
            char nom_normalise[50];
            strcpy(nom_normalise, nom_accessoire);
            normaliser_nom(nom_normalise);

            Accessoire *achete = NULL;
            Accessoire *courant = roulotte, *precedent = NULL;

            // Recherche dans la roulotte
            while (courant) {
                char nom_courant[50];
                strcpy(nom_courant, courant->nom);
                normaliser_nom(nom_courant);

                if (strcmp(nom_normalise, nom_courant) == 0) {
                    achete = courant;
                    if (precedent == NULL) {
                        roulotte = courant->suivant;
                    } else {
                        precedent->suivant = courant->suivant;
                    }
                    break;
                }
                precedent = courant;
                courant = courant->suivant;
            }

            if (achete) {
                if (argent >= achete->prix) {
                    argent -= achete->prix;
                    ajouter_accessoire(&inventaire, achete->nom, achete->attbonus, achete->defbonus,
                                       achete->HPbonus, achete->restbonus, achete->strred, achete->prix);
                    printf("Vous avez acheté %s pour %d or. Or restant : %d.\n",
                           achete->nom, achete->prix, argent);
                } else {
                    printf("Vous n'avez pas assez de pièces pour acheter %s (Prix : %d or).\n",
                           achete->nom, achete->prix);
                    ajouter_accessoire(&roulotte, achete->nom, achete->attbonus, achete->defbonus,
                                       achete->HPbonus, achete->restbonus, achete->strred, achete->prix);
                }
                free(achete);
            } else {
                printf("Accessoire '%s' introuvable dans la roulotte. Assurez-vous de bien écrire le nom.\n",
                       nom_accessoire);
            }
        } else if (choix == 2) {
            printf("Vous quittez la roulotte.\n");
            break;
        } else {
            printf("Choix invalide, réessayez.\n");
        }
    }
}

void equiper_accessoire(Personnage *personnage, Accessoire **inventaire) {
    if (!inventaire || !*inventaire) {
        printf("\nAucun accessoire dans l'inventaire. Passer à %s.\n", personnage->nom);
        return;
    }

    while (1) {
        printf("\nVoulez-vous équiper un accessoire pour %s ? (oui/non) : ", personnage->nom);
        char choix[10];
        if (!fgets(choix, sizeof(choix), stdin)) {
            printf("Erreur de saisie. Veuillez réessayer.\n");
            continue;
        }
        
        choix[strcspn(choix, "\n")] = '\0'; // Retirer le retour à la ligne

        if (strcasecmp(choix, "non") == 0) {
            printf("Passer à l'équipement pour le prochain personnage.\n");
            return;
        } else if (strcasecmp(choix, "oui") == 0) {
            for (int i = 0; i < 2; i++) { // Permet d'équiper jusqu'à 2 accessoires
                printf("\nAccessoires disponibles dans l'inventaire :\n");
                afficher_accessoires(*inventaire);

                printf("Choisissez un accessoire à équiper pour %s (ou 'aucun' pour passer) : ", personnage->nom);
                char nom_accessoire[50];
                if (!fgets(nom_accessoire, sizeof(nom_accessoire), stdin)) {
                    printf("Erreur de saisie. Réessayez.\n");
                    i--; // Redemander pour le même tour
                    continue;
                }
                nom_accessoire[strcspn(nom_accessoire, "\n")] = '\0'; // Retirer le retour à la ligne

                if (strcasecmp(nom_accessoire, "aucun") == 0) {
                    printf("%s a choisi de ne pas équiper d'accessoire supplémentaire.\n", personnage->nom);
                    return;
                }

                // Rechercher l'accessoire dans l'inventaire
                Accessoire *courant = *inventaire, *precedent = NULL;
                while (courant) {
                    if (strcasecmp(courant->nom, nom_accessoire) == 0) { // Comparaison insensible à la casse
                        // Équiper l'accessoire
                        if (!personnage->accessoire1) {
                            personnage->accessoire1 = courant;
                        } else if (!personnage->accessoire2) {
                            personnage->accessoire2 = courant;
                        } else {
                            printf("%s porte déjà deux accessoires.\n", personnage->nom);
                            return;
                        }

                        // Retirer l'accessoire de l'inventaire
                        if (precedent == NULL) {
                            *inventaire = courant->suivant;
                        } else {
                            precedent->suivant = courant->suivant;
                        }

                        courant->suivant = NULL; // Détache l'accessoire
                        printf("%s a équipé %s.\n", personnage->nom, courant->nom);

                        // Si aucun accessoire n'est disponible après, arrêter la boucle
                        if (!*inventaire) {
                            printf("Plus d'accessoires disponibles dans l'inventaire.\n");
                            return;
                        }
                        break;
                    }
                    precedent = courant;
                    courant = courant->suivant;
                }

                if (!courant) {
                    printf("Accessoire non trouvé dans l'inventaire. Réessayez.\n");
                    i--; // Répéter le même tour pour corriger l'erreur
                }
            }
            return;
        } else {
            printf("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.\n");
        }
    }
}

void retourner_accessoires(Personnage *personnage, Accessoire **inventaire) {
    if (personnage->accessoire1) {
        ajouter_accessoire(inventaire, personnage->accessoire1->nom, 
                           personnage->accessoire1->attbonus, 
                           personnage->accessoire1->defbonus, 
                           personnage->accessoire1->HPbonus, 
                           personnage->accessoire1->restbonus, 
                           personnage->accessoire1->strred, 
                           personnage->accessoire1->prix);
        free(personnage->accessoire1);
        personnage->accessoire1 = NULL;
    }

    if (personnage->accessoire2) {
        ajouter_accessoire(inventaire, personnage->accessoire2->nom, 
                           personnage->accessoire2->attbonus, 
                           personnage->accessoire2->defbonus, 
                           personnage->accessoire2->HPbonus, 
                           personnage->accessoire2->restbonus, 
                           personnage->accessoire2->strred, 
                           personnage->accessoire2->prix);
        free(personnage->accessoire2);
        personnage->accessoire2 = NULL;
    }
}

void recuperer_accessoire_sur_ennemi() {
    // Sélectionner un accessoire aléatoire
    int index = rand() % 5; // Taille du tableau accessoires_ennemis
    const char *nom_accessoire = accessoires_ennemis[index];

    // Générer des statistiques aléatoires pour l'accessoire
    int att_bonus = rand() % 5;    // Bonus d'attaque
    int def_bonus = rand() % 5;    // Bonus de défense
    int hp_bonus = rand() % 5;     // Bonus de HP
    int rest_bonus = rand() % 3;   // Bonus de repos
    int stress_red = rand() % 3;   // Réduction de stress

    // Ajouter l'accessoire avec les stats générées et un prix non défini
    ajouter_accessoire(&inventaire, nom_accessoire, att_bonus, def_bonus, hp_bonus, rest_bonus, stress_red, 0);

    // Afficher le message de récupération
    printf("L'ennemi a laissé tomber %s ! Vous le ramassez.\n", nom_accessoire);

    // Afficher les statistiques de l'accessoire
    printf("Stats de l'accessoire récupéré :\n");
    printf("- Att : %d | Déf : %d | HP : %d | Rest : %d | Stress Red : %d\n", att_bonus, def_bonus, hp_bonus, rest_bonus, stress_red);
}

void liberer_accessoires(Accessoire **liste) {
    while (*liste) {
        Accessoire *temp = *liste;
        *liste = (*liste)->suivant;
        free(temp);
    }
}


// ----------------------------------------------------------------------------------------- //
// ---------------------------------- GESTION DU COMBAT ------------------------------------ //
// ----------------------------------------------------------------------------------------- //

// Séléction des combattants

void selectionner_combattants(Personnage **disponibles, Personnage **combattants, int max_combattants) {
    printf("\nPersonnages aptes au combat:\n");
    afficher_personnages(*disponibles);
    printf("\nVous pouvez sélectionner jusqu'à %d combattants.\n", max_combattants);

    int selectionnes = 0;

    while (selectionnes < max_combattants) {
        printf("Entrez le nom du personnage à ajouter au combat (ou 'stop' pour terminer) : ");
        char nom[50];
        scanf("%s", nom);
        normaliser_nom(nom);

        if (strcmp(nom, "stop") == 0) {
            break;
        }

        Personnage *precedent = NULL, *courant = *disponibles;
        while (courant != NULL) {
            char nom_normalise[50];
            strcpy(nom_normalise, courant->nom);
            normaliser_nom(nom_normalise);

            if (strcmp(nom_normalise, nom) == 0) {
                break;
            }
            precedent = courant;
            courant = courant->suivant;
        }

        if (courant != NULL) {
            if (precedent == NULL) {
                *disponibles = courant->suivant;
            } else {
                precedent->suivant = courant->suivant;
            }

            courant->suivant = *combattants;
            *combattants = courant;
            printf("%s a été ajouté aux combattants.\n", courant->nom);
            selectionnes++;
        } else {
            printf("Personnage non trouvé. Veuillez réessayer.\n");
        }
    }

    printf("\nListe des combattants :\n");
    afficher_personnages(*combattants);
}

void combat(Personnage **equipe, Ennemi *ennemi) {
    printf("\n** Début du combat contre %s ** (HP : %d, Att : %d, Déf : %d)\n",
           ennemi->nom, ennemi->HPenn, ennemi->attenn, ennemi->defenn);

    while (ennemi->HPenn > 0 && *equipe != NULL) {
        Personnage *perso = *equipe;

        while (perso != NULL && ennemi->HPenn > 0) {
            if (perso->HP > 0 && perso->stress < 100) {
                char action;
                int action_valide = 0;

                // Demander l'action jusqu'à ce qu'une entrée valide soit fournie
                while (!action_valide) {
                    printf("Que doit faire %s ?\nA: Attaque\nD: Défense\nS: Soins\n", perso->nom);
                    scanf(" %c", &action);

                    action = tolower(action); // Convertir en minuscule pour uniformiser

                    if (action == 'a' || action == 'd' || action == 's') {
                        action_valide = 1;
                    } else {
                        printf("Entrée invalide. Veuillez choisir entre A, D ou S.\n");
                        while (getchar() != '\n'); // Nettoyer le buffer d'entrée
                    }
                }

                switch (action) {
                    case 'a': {
                        int degats = (perso->classe.att +
                                      (perso->accessoire1 ? perso->accessoire1->attbonus : 0) +
                                      (perso->accessoire2 ? perso->accessoire2->attbonus : 0)) -
                                     ennemi->defenn;
                        degats = (degats > 0) ? degats : 1;
                        ennemi->HPenn -= degats;
                        if (ennemi->HPenn < 0) ennemi->HPenn = 0;
                        printf("%s inflige %d dégâts à %s ! Il lui reste %d HP\n",
                               perso->nom, degats, ennemi->nom, ennemi->HPenn);
                        break;
                    }
                    case 'd': {
                        perso->defense_temporaire = perso->classe.def +
                                                    (perso->accessoire1 ? perso->accessoire1->defbonus : 0) +
                                                    (perso->accessoire2 ? perso->accessoire2->defbonus : 0);
                        printf("%s se défend, défense temporaire : %d.\n", perso->nom, perso->defense_temporaire);
                        break;
                    }
                    case 's': {
                        printf("Choisissez un personnage à soigner :\n");
                        Personnage *cible = *equipe;
                        int choix = 1, selection;
                        while (cible) {
                            printf("%d - %s (HP : %d/%d)\n", choix++, cible->nom, cible->HP, cible->classe.HPmax);
                            cible = cible->suivant;
                        }
                        scanf("%d", &selection);
                        cible = *equipe;
                        for (int i = 1; i < selection && cible; i++) {
                            cible = cible->suivant;
                        }
                        if (cible) {
                            int soin = perso->classe.rest +
                                       (perso->accessoire1 ? perso->accessoire1->restbonus : 0) +
                                       (perso->accessoire2 ? perso->accessoire2->restbonus : 0);
                            cible->HP += soin;
                            if (cible->HP > cible->classe.HPmax) cible->HP = cible->classe.HPmax;
                            printf("%s soigne %s pour %d HP. HP actuels : %d/%d\n",
                                   perso->nom, cible->nom, soin, cible->HP, cible->classe.HPmax);
                        }
                        break;
                    }
                }
            } else if (perso->stress >= 100) {
                printf("%s est trop stressé pour agir !\n", perso->nom);
            }
            perso = perso->suivant;
        }

        if (ennemi->HPenn > 0) {
            // Attaque ennemie
            Personnage *cible = *equipe;
            while (cible && cible->HP <= 0) cible = cible->suivant;

            if (cible) {
                int type_attaque = rand() % 2; // Choix aléatoire entre physique (0) et stress (1)
                if (type_attaque == 0) {
                    int degats = ennemi->attenn - cible->classe.def;
                    degats = (degats > 0) ? degats : 1;
                    cible->HP -= degats;
                    if (cible->HP < 0) cible->HP = 0;
                    printf("%s attaque %s avec une attaque physique pour %d dégâts ! (HP restants : %d/%d)\n",
                           ennemi->nom, cible->nom, degats, cible->HP, cible->classe.HPmax);
                } else {
                    int stress_points = ennemi->attstrenn;
                    cible->stress += stress_points;
                    if (cible->stress > 100) cible->stress = 100;
                    printf("%s attaque %s avec une attaque de stress pour %d points de stress ! (Stress actuel : %d%%)\n",
                           ennemi->nom, cible->nom, stress_points, cible->stress);
                }
            }
        }

        gerer_personnages_morts(equipe);

        if (*equipe == NULL) {
            printf("Tous les personnages combattants sont morts. Vous avez perdu la partie.\n");
            exit(0);
        }
    }

    if (ennemi->HPenn <= 0) {
        printf("Victoire contre %s !\n", ennemi->nom);
        argent += 10;
        printf("Vous gagnez 10 pièces d'or. Or actuel : %d.\n", argent);

        // Récupérer un accessoire après le combat
        recuperer_accessoire_sur_ennemi();

        // Retourner les accessoires équipés dans l'inventaire
        Personnage *perso = *equipe;
        while (perso) {
            retourner_accessoires(perso, &inventaire);
            perso = perso->suivant;
        }
    }
}

void gerer_personnages_morts(Personnage **liste) {
    Personnage *courant = *liste;
    while (courant) {
        if (courant->HP <= 0) {
            printf("%s sombre dans les ténèbres...\n", courant->nom);

            // On oublie pas de supprimer les accessoires portés
            if (courant->accessoire1) {
                free(courant->accessoire1);
                courant->accessoire1 = NULL;
            }
            if (courant->accessoire2) {
                free(courant->accessoire2);
                courant->accessoire2 = NULL;
            }

            supprimer_personnage(liste, courant);
            courant = *liste;
        } else {
            courant = courant->suivant;
        }
    }
}

// ----------------------------------------------------------------------------------------- //
// ---------------------------------- PROGRAMME PRINCIPAL ---------------------------------- //
// ----------------------------------------------------------------------------------------- //

int main() {
    srand(time(NULL));

    while (1) {
        printf("\n--- Bienvenue dans Darkest C Dungeon ---\n");
        printf("1. Nouvelle partie\n");
        printf("2. Charger une partie\n");
        printf("3. Quitter le jeu\n");
        printf("Votre choix : ");

        int choix_principal;
        if (scanf("%d", &choix_principal) != 1) {
            printf("Entrée invalide. Veuillez entrer un chiffre.\n");
            while (getchar() != '\n'); // Vider le buffer
            continue;
        }

        if (choix_principal == 1) {
            printf("\n--- Nouvelle Partie ---\n");

            // Initialiser une nouvelle partie
            liberer_personnages(&disponibles);
            liberer_personnages(&combattants);
            liberer_personnages(&sanitarium);
            liberer_personnages(&taverne);
            liberer_accessoires(&roulotte);
            liberer_accessoires(&inventaire);

            disponibles = NULL;
            combattants = NULL;
            sanitarium = NULL;
            taverne = NULL;
            roulotte = NULL;
            inventaire = NULL;

            argent = 5; 
            niveau = 1;  // Commencer au niveau 1

           

            afficher_classes();

            // Initialisation des personnages
            ajouter_personnage(&disponibles, "Boudicca", furie);
            ajouter_personnage(&disponibles, "Junia", vestale);

            // Initialisation des accessoires
            initialiser_inventaire();
            initialiser_roulotte();

            break; // Sortir du menu principal pour commencer la partie

        } else if (choix_principal == 2) {
            printf("\nEntrez le nom du fichier de sauvegarde : ");
            char nom_fichier[100];
            scanf("%s", nom_fichier);

            if (charger_partie(nom_fichier)) {
                printf("\nPartie chargée avec succès depuis '%s'.\n", nom_fichier);
                break; // Sortir du menu principal pour reprendre la partie
            } else {
                printf("\nÉchec du chargement de la sauvegarde. Retour au menu principal.\n");
            }

        } else if (choix_principal == 3) {
            printf("Au revoir !\n");
            return 0; // Quitter le jeu
        } else {
            printf("Choix invalide. Veuillez choisir une option entre 1 et 3.\n");
        }
    }

    // Boucle principale du jeu
    for (; niveau <= 10; niveau++) {
        printf("\n--- Niveau %d ---\n", niveau);

        // Création de l'ennemi pour ce niveau
        Ennemi ennemi = {
            noms_ennemis[niveau - 1], niveau, 5 + niveau, 3 + niveau,
            20 + niveau * 2, 10
        };

        int choix = 0;
        while (choix != 4) { // Modification ici pour gérer correctement la sauvegarde
            printf("\nVous avez %d pièces d'or dans votre bourse.\n", argent);
            printf("\n-> Inventaire :\n");
            afficher_accessoires(inventaire);

            printf("\nActions disponibles :\n");
            printf("1. -> Sanitarium\n");
            printf("2. -> Taverne\n");
            printf("3. -> Roulotte\n");
            printf("4. -> Prochain donjon\n");
            printf("5. -> Sauvegarder la partie\n");
            printf("Entrez votre choix : ");
            scanf("%d", &choix);
            getchar(); // Vider le buffer d'entrée

            switch (choix) {
                case 1: 
                    envoyer_au_sanitarium(&disponibles, &sanitarium);
                    break;
                case 2: 
                    envoyer_a_la_taverne(&disponibles, &taverne);
                    break;
                case 3: 
                    gerer_roulotte();
                    break;
                case 4: 
                    break; // Quitter le menu pour lancer le prochain donjon
                case 5: {
                    printf("\nEntrez le nom du fichier de sauvegarde : ");
                    char nom_fichier[100];
                    scanf("%s", nom_fichier);
                    sauvegarder_partie(nom_fichier);
                    printf("Partie sauvegardée avec succès.\n");
                    break;
                }
                default: 
                    printf("Choix invalide. Réessayez.\n");
                    break;
            }
        }

        // Gérer les soins et la réduction de stress
        soigner_au_sanitarium(sanitarium);
        reduire_stress_a_la_taverne(taverne);

        // Sélection des combattants
        int max_combattants = (niveau <= 5) ? 2 : 3;
        selectionner_combattants(&disponibles, &combattants, max_combattants);

        // Équipement des accessoires avant le combat
        Personnage *courant = combattants;
        while (courant) {
            equiper_accessoire(courant, &inventaire);
            courant = courant->suivant;
        }

        // Lancer le combat
        combat(&combattants, &ennemi);

        // Retour des combattants dans les disponibles
        deplacer_personnages(&combattants, &disponibles);

        // Retour des personnages du sanitarium et de la taverne
        retourner_du_sanitarium(&sanitarium, &disponibles);
        retourner_de_la_taverne(&taverne, &disponibles);

        // Ajout de nouveaux personnages tous les 2 niveaux
        if (niveau % 2 == 0) {
            char *noms_nouveaux[] = {"Dismas", "Tardif", "William", "Alice"};
            Classe classes_nouvelles[] = {chasseur_primes, maitre_chien, vestale, furie};
            ajouter_personnage(&disponibles, noms_nouveaux[(niveau / 2) - 1], classes_nouvelles[(niveau / 2) - 1]);
        }
    }

    printf("\nFélicitations, vous avez terminé les 10 niveaux !\n");

    // Libération de la mémoire
    liberer_personnages(&disponibles);
    liberer_personnages(&combattants);
    liberer_accessoires(&roulotte);
    liberer_accessoires(&inventaire);

    return 0;
}



// ----------------------------------------------------------------------------------------- //
// ------------------------------- FONCTIONS DE SAUVEGARDE --------------------------------- //
// ----------------------------------------------------------------------------------------- //

void sauvegarder_partie(const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "w");
    if (!fichier) {
        printf("Erreur : impossible d'ouvrir le fichier de sauvegarde.\n");
        return;
    }

    // Sauvegarder le niveau et l'argent
    fprintf(fichier, "Niveau %d\n", niveau);
    fprintf(fichier, "Argent %d\n", argent);

    // Sauvegarder les personnages disponibles
    fprintf(fichier, "Personnages disponibles\n");
    Personnage *perso = disponibles;
    while (perso) {
        fprintf(fichier, "%s %s %d %d %d %d\n",
                perso->nom, perso->classe.nom,
                perso->HP, perso->stress,
                perso->classe.HPmax, perso->nbcomb);
        perso = perso->suivant;
    }
    fprintf(fichier, "FIN_PERSONNAGES\n");

    // Sauvegarder les personnages au sanitarium
    fprintf(fichier, "Sanitarium\n");
    perso = sanitarium;
    while (perso) {
        fprintf(fichier, "%s %s %d %d %d %d\n",
                perso->nom, perso->classe.nom,
                perso->HP, perso->stress,
                perso->classe.HPmax, perso->nbcomb);
        perso = perso->suivant;
    }
    fprintf(fichier, "FIN_SANITARIUM\n");

    // Sauvegarder les personnages à la taverne
    fprintf(fichier, "Taverne\n");
    perso = taverne;
    while (perso) {
        fprintf(fichier, "%s %s %d %d %d %d\n",
                perso->nom, perso->classe.nom,
                perso->HP, perso->stress,
                perso->classe.HPmax, perso->nbcomb);
        perso = perso->suivant;
    }
    fprintf(fichier, "FIN_TAVERNE\n");

    // Sauvegarder les accessoires disponibles (inventaire)
    fprintf(fichier, "Inventaire\n");
    Accessoire *acc = inventaire;
    while (acc) {
        fprintf(fichier, "%s %d %d %d %d %d\n",
                acc->nom, acc->attbonus, acc->defbonus, acc->HPbonus,
                acc->restbonus, acc->strred);
        acc = acc->suivant;
    }
    fprintf(fichier, "FIN_INVENTAIRE\n");

    // Sauvegarder les accessoires en vente (roulotte)
    fprintf(fichier, "Roulotte\n");
    acc = roulotte;
    while (acc) {
        fprintf(fichier, "%s %d %d %d %d %d %d\n",
                acc->nom, acc->attbonus, acc->defbonus, acc->HPbonus,
                acc->restbonus, acc->strred, acc->prix);
        acc = acc->suivant;
    }
    fprintf(fichier, "FIN_ROULOTTE\n");

    fclose(fichier);
    printf("Partie sauvegardée dans le fichier '%s'.\n", nom_fichier);
}

int charger_partie(const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (!fichier) {
        printf("Erreur : impossible d'ouvrir le fichier de sauvegarde '%s'.\n", nom_fichier);
        return 0;
    }

    // Libérer les listes actuelles
    liberer_personnages(&disponibles);
    liberer_personnages(&sanitarium);
    liberer_personnages(&taverne);
    liberer_accessoires(&inventaire);
    liberer_accessoires(&roulotte);

    // Charger le niveau et l'argent
    if (fscanf(fichier, "Niveau %d\n", &niveau) != 1 ||
        fscanf(fichier, "Argent %d\n", &argent) != 1) {
        printf("Erreur : format invalide dans le fichier de sauvegarde.\n");
        fclose(fichier);
        return 0;
    }

    // Charger les personnages disponibles
    char buffer[50], classe_nom[50];
    int HP, stress, HPmax, nbcomb;
    if (fscanf(fichier, "Personnages disponibles\n") != 0) {
        printf("Erreur : section 'Personnages disponibles' manquante.\n");
        fclose(fichier);
        return 0;
    }

    while (fscanf(fichier, "%s %s %d %d %d %d\n", buffer, classe_nom, &HP, &stress, &HPmax, &nbcomb) == 6) {
        Classe classe = {0};
        strcpy(classe.nom, classe_nom);
        classe.HPmax = HPmax;

        ajouter_personnage(&disponibles, buffer, classe);
        disponibles->HP = HP;
        disponibles->stress = stress;
        disponibles->nbcomb = nbcomb;
    }

    // Charger les personnages au sanitarium
    fscanf(fichier, "Sanitarium\n");
    while (fscanf(fichier, "%s %s %d %d %d %d\n", buffer, classe_nom, &HP, &stress, &HPmax, &nbcomb) == 6) {
        Classe classe = {0};
        strcpy(classe.nom, classe_nom);
        classe.HPmax = HPmax;

        ajouter_personnage(&sanitarium, buffer, classe);
        sanitarium->HP = HP;
        sanitarium->stress = stress;
        sanitarium->nbcomb = nbcomb;
    }

    // Charger les personnages à la taverne
    fscanf(fichier, "Taverne\n");
    while (fscanf(fichier, "%s %s %d %d %d %d\n", buffer, classe_nom, &HP, &stress, &HPmax, &nbcomb) == 6) {
        Classe classe = {0};
        strcpy(classe.nom, classe_nom);
        classe.HPmax = HPmax;

        ajouter_personnage(&taverne, buffer, classe);
        taverne->HP = HP;
        taverne->stress = stress;
        taverne->nbcomb = nbcomb;
    }

    // Charger les accessoires disponibles (inventaire)
    fscanf(fichier, "Inventaire\n");
    char acc_nom[50];
    int attbonus, defbonus, HPbonus, restbonus, strred;
    while (fscanf(fichier, "%s %d %d %d %d %d\n", acc_nom, &attbonus, &defbonus, &HPbonus, &restbonus, &strred) == 6) {
        ajouter_accessoire(&inventaire, acc_nom, attbonus, defbonus, HPbonus, restbonus, strred, 0);
    }

    // Charger les accessoires en vente (roulotte)
    fscanf(fichier, "Roulotte\n");
    int prix;
    while (fscanf(fichier, "%s %d %d %d %d %d %d\n", acc_nom, &attbonus, &defbonus, &HPbonus, &restbonus, &strred, &prix) == 7) {
        ajouter_accessoire(&roulotte, acc_nom, attbonus, defbonus, HPbonus, restbonus, strred, prix);
    }

    fclose(fichier);
    printf("Partie chargée depuis le fichier '%s'.\n", nom_fichier);
    return 1;
}