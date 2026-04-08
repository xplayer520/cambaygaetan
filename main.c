#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PARKINGS 100
#define MAX_STRING 200
#define MAX_VEHICULES 500

// ---------------- STRUCTURES ----------------
typedef struct {
    char plaque[20];
    time_t heure_entree;
} Vehicule;

typedef struct {
    char code[MAX_STRING];
    char nom[MAX_STRING];
    char ville[MAX_STRING];
    char statut[MAX_STRING];
    int capacite;
    int places_occupees;
    float tarif_horaire;
    Vehicule vehicules[MAX_VEHICULES];
} Parking;

// ---------------- INPUT SECURISE ----------------
void lireChaine(char *buffer, int taille) {
    fgets(buffer, taille, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
}

// ---------------- LECTURE CSV ----------------
int lireParkings(const char *nomFichier, Parking parkings[]) {
    FILE *f = fopen(nomFichier, "r");
    if (!f) {
        printf("Erreur ouverture fichier\n");
        return 0;
    }

    char ligne[1024];
    int i = 0;

    fgets(ligne, sizeof(ligne), f); // ignorer header

    while (fgets(ligne, sizeof(ligne), f) && i < MAX_PARKINGS) {

        char adresse[MAX_STRING], date[MAX_STRING], affichage[MAX_STRING];
        int places_dispo, capacite;

        int n = sscanf(ligne,
            "%[^;];%[^;];%[^;];%[^;];%[^;];%d;%d;%[^;];%[^;\n]",
            parkings[i].code,
            parkings[i].nom,
            adresse,
            parkings[i].ville,
            parkings[i].statut,
            &places_dispo,
            &capacite,
            date,
            affichage
        );

        if (n < 7) continue;

        parkings[i].capacite = capacite;
        parkings[i].places_occupees = capacite - places_dispo;

        if (parkings[i].places_occupees < 0)
            parkings[i].places_occupees = 0;

        if (parkings[i].places_occupees > capacite)
            parkings[i].places_occupees = capacite;

        parkings[i].tarif_horaire = 2.5;

        for (int v = 0; v < MAX_VEHICULES; v++)
            parkings[i].vehicules[v].plaque[0] = '\0';

        i++;
    }

    fclose(f);
    return i;
}

// ---------------- AFFICHAGE ----------------
void afficherTousParkings(Parking parkings[], int nb) {
    printf("\n--------------------------------------------------\n");
    printf("CODE       NOM                VILLE       OCC/CAP   DISPO\n");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < nb; i++) {
        int dispo = parkings[i].capacite - parkings[i].places_occupees;

        printf("%-10s %-18s %-10s %3d/%-5d %-5d\n",
               parkings[i].code,
               parkings[i].nom,
               parkings[i].ville,
               parkings[i].places_occupees,
               parkings[i].capacite,
               dispo);
    }
}

void afficherParkingParCode(Parking parkings[], int nb) {
    char code[MAX_STRING];

    printf("Code : ");
    lireChaine(code, MAX_STRING);

    for (int i = 0; i < nb; i++) {
        if (strcmp(parkings[i].code, code) == 0) {

            printf("\n--- DETAILS PARKING ---\n");
            printf("Code : %s\n", parkings[i].code);
            printf("Nom  : %s\n", parkings[i].nom);
            printf("Ville: %s\n", parkings[i].ville);
            printf("Occupation : %d/%d\n",
                   parkings[i].places_occupees,
                   parkings[i].capacite);

            return;
        }
    }
    printf("Parking introuvable\n");
}

// ---------------- LOGIQUE ----------------
int verifierPlacesDisponibles(Parking p) {
    return (p.places_occupees >= p.capacite);
}

void mettreAJourOccupation(Parking *p, int entree) {
    if (entree && p->places_occupees < p->capacite)
        p->places_occupees++;
    else if (!entree && p->places_occupees > 0)
        p->places_occupees--;
}

// ---------------- CLIENT ----------------
void suiviClient(char plaque[], double duree, float montant) {
    FILE *f = fopen("clients.txt", "a");
    if (!f) return;

    fprintf(f, "%s | %.2f h | %.2f €\n", plaque, duree, montant);
    fclose(f);
}

// ---------------- ENTREE ----------------
void entreeParking(Parking parkings[], int nb) {
    char code[MAX_STRING], plaque[20];

    printf("Code parking : ");
    lireChaine(code, MAX_STRING);

    printf("Plaque : ");
    lireChaine(plaque, 20);

    for (int i = 0; i < nb; i++) {
        if (strcmp(parkings[i].code, code) == 0) {

            if (verifierPlacesDisponibles(parkings[i])) {
                printf("Parking complet\n");
                return;
            }

            int index = parkings[i].places_occupees;

            if (index >= MAX_VEHICULES) {
                printf("Erreur capacite vehicules\n");
                return;
            }

            strcpy(parkings[i].vehicules[index].plaque, plaque);
            parkings[i].vehicules[index].heure_entree = time(NULL);

            mettreAJourOccupation(&parkings[i], 1);

            printf("Entree enregistree\n");
            return;
        }
    }
    printf("Parking introuvable\n");
}

// ---------------- SORTIE ----------------
void sortieParking(Parking parkings[], int nb) {
    char code[MAX_STRING], plaque[20];

    printf("Code parking : ");
    lireChaine(code, MAX_STRING);

    printf("Plaque : ");
    lireChaine(plaque, 20);

    for (int i = 0; i < nb; i++) {
        if (strcmp(parkings[i].code, code) == 0) {

            for (int j = 0; j < parkings[i].places_occupees; j++) {

                if (strcmp(parkings[i].vehicules[j].plaque, plaque) == 0) {

                    double duree = difftime(time(NULL),
                        parkings[i].vehicules[j].heure_entree) / 3600.0;

                    float montant = duree * parkings[i].tarif_horaire;

                    printf("Duree : %.2f h\n", duree);
                    printf("Montant : %.2f €\n", montant);

                    parkings[i].vehicules[j] =
                        parkings[i].vehicules[parkings[i].places_occupees - 1];

                    mettreAJourOccupation(&parkings[i], 0);

                    suiviClient(plaque, duree, montant);

                    printf("Sortie enregistree\n");
                    return;
                }
            }
            printf("Vehicule non trouve\n");
            return;
        }
    }
    printf("Parking introuvable\n");
}

// ---------------- SAUVEGARDE ----------------
void sauvegarderEtatParking(const char *nomFichier, Parking parkings[], int nb) {
    FILE *f = fopen(nomFichier, "w");
    if (!f) {
        printf("Erreur sauvegarde\n");
        return;
    }

    fprintf(f, "code;nom;ville;statut;capacite;occupees\n");

    for (int i = 0; i < nb; i++) {
        fprintf(f, "%s;%s;%s;%s;%d;%d\n",
                parkings[i].code,
                parkings[i].nom,
                parkings[i].ville,
                parkings[i].statut,
                parkings[i].capacite,
                parkings[i].places_occupees);
    }

    fclose(f);
    printf("Sauvegarde reussie\n");
}

// ---------------- ADMIN ----------------
int modeAdmin() {
    char code[50];
    printf("Code admin : ");
    lireChaine(code, 50);
    return strcmp(code, "gaetancambay") == 0;
}

void menuAdmin(Parking parkings[], int nb) {
    int choix;
    char code[MAX_STRING];

    do {
        printf("\n----- MODE ADMIN -----\n");
        printf("1 Modifier capacite\n");
        printf("2 Modifier tarif\n");
        printf("3 Voir occupation\n");
        printf("0 Retour\n");
        printf("Choix : ");
        scanf("%d", &choix);
        getchar();

        if (choix == 0) break;

        printf("Code parking : ");
        lireChaine(code, MAX_STRING);

        for (int i = 0; i < nb; i++) {
            if (strcmp(parkings[i].code, code) == 0) {

                if (choix == 1) {
                    int cap;
                    printf("Nouvelle capacite : ");
                    scanf("%d", &cap);
                    getchar();
                    parkings[i].capacite = cap;
                }

                if (choix == 2) {
                    float tarif;
                    printf("Nouveau tarif : ");
                    scanf("%f", &tarif);
                    getchar();
                    parkings[i].tarif_horaire = tarif;
                }

                if (choix == 3) {
                    printf("Occupation : %d/%d\n",
                           parkings[i].places_occupees,
                           parkings[i].capacite);
                }
            }
        }

    } while (choix != 0);
}




void verifierParking(Parking parkings[], int nb) {
    char code[MAX_STRING];

    printf("Code parking : ");
    lireChaine(code, MAX_STRING);

    for (int i = 0; i < nb; i++) {
        if (strcmp(parkings[i].code, code) == 0) {

            if (verifierPlacesDisponibles(parkings[i])) {
                printf("Parking COMPLET\n");
            } else {
                printf("Places disponibles : %d\n",
                    parkings[i].capacite - parkings[i].places_occupees);
            }
            return;
        }
    }
    printf("Parking introuvable\n");
}
// ---------------- MENU ----------------
void afficherMenu() {
    printf("\n====================================\n");
    printf("      GESTION PARKING\n");
    printf("====================================\n");
    printf("1. Afficher tous les parkings\n");
    printf("2. Detail d'un parking\n");
    printf("3. Entree vehicule\n");
    printf("4. Sortie vehicule\n");
    printf("5. Mode administrateur\n");
    printf("6. Sauvegarder\n");
    printf("7. Verifier disponibilite parking\n");
    printf("0. Quitter\n");
    printf("====================================\n");
    printf("Choix : ");
}

// ---------------- MAIN ----------------
int main() {
    Parking parkings[MAX_PARKINGS];
    int nb = lireParkings("parking-metropole.csv", parkings);

    if (nb == 0) {
        printf("Erreur chargement\n");
        return 1;
    }

    int choix;

    do {
        afficherMenu();
        scanf("%d", &choix);
        getchar();

        switch (choix) {
            case 1: afficherTousParkings(parkings, nb); break;
            case 2: afficherParkingParCode(parkings, nb); break;
            case 3: entreeParking(parkings, nb); break;
            case 4: sortieParking(parkings, nb); break;
            case 5:
                if (modeAdmin())
                    menuAdmin(parkings, nb);
                else
                    printf("Acces refuse\n");
                break;
            case 6:
                sauvegarderEtatParking("save.csv", parkings, nb);
                break;
                case 7:
    verifierParking(parkings, nb);
    break;
        }

    } while (choix != 0);

    return 0;
}
