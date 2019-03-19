#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <MLV/MLV_all.h>

#include "serpent.h"
#include "affichage_serpent.h"

int main(int argc, char *argv[])
{
	MLV_Keyboard_button touche;
	struct timeval debut, fin;
	long delai;
	FILE *fp;
	int largeur, hauteur, nombre_pommes, taille_serpent, duree_tour;

	/* Initialisation random */
	srandom(time(NULL));

	/* Valeurs par défaut */
	largeur = M;
	hauteur = N;
	nombre_pommes = NB_POMMES;
	taille_serpent = TAILLE_SERPENT;
	duree_tour = DUREE_TOUR_MS;

	/* Fichier de configuration existant ? */
	fp = fopen(FICHIER_CONFIG, "r");
	if (fp != NULL) {
		lire_fichier_config(fp, &largeur, &hauteur, &nombre_pommes, &taille_serpent, &duree_tour);
	}

	/* Initialisation graphique */
	MLV_create_window("Serpent", "Serpent", TAILLE_CASE*largeur, TAILLE_CASE*(hauteur+3));

	Monde mon = init_monde(nombre_pommes, hauteur, largeur, taille_serpent);

	afficher_monde(&mon);

	MLV_wait_keyboard(NULL, NULL, NULL);

	while (1) {
		/* Mort du serpent, partie terminée */
		if (mort_serpent(&mon)) {
			break;
		}
		/* On essaie de se déplacer sur une case vide */
		if (!deplacer_serpent(&mon, 1)) {
			/* Sinon on essaie de manger une pomme */
			if (manger_pomme_serpent(&mon)) {
				/* SI on a mangé une pomme on en recréée une */
				ajouter_pomme_monde(&mon);
			}
		}
		/* On affiche le monde */
		afficher_monde(&mon);
		/* Détection touche pressée ou délai atteint */
		/* On récupère l'heure courante */
		gettimeofday(&debut, NULL);
		while (MLV_get_event(&touche, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == MLV_NONE ||
				touche != MLV_KEYBOARD_a ||
				touche != MLV_KEYBOARD_p ||
				touche != MLV_KEYBOARD_q ||
				touche != MLV_KEYBOARD_o ||
				touche != MLV_KEYBOARD_SPACE) {
			/* On récupère l'heure courante */
			gettimeofday(&fin, NULL);
			delai = (fin.tv_sec*1000+fin.tv_usec/1000) - (debut.tv_sec*1000+debut.tv_usec/1000);
			/* Délai atteint on sort de la boucle */
			if (delai > duree_tour) {
				break;
			}
		}

		/* On ne prend pas en compte la 'marche arrière' */
		switch (touche) {
			case MLV_KEYBOARD_a:
				if (mon.serpent.direction != SUD) {
					mon.serpent.direction = NORD;
				}
				break;
			case MLV_KEYBOARD_p:
				if (mon.serpent.direction != OUEST) {
					mon.serpent.direction = EST;
				}
				break;
			case MLV_KEYBOARD_q:
				if (mon.serpent.direction != NORD) {
					mon.serpent.direction = SUD;
				}
				break;
			case MLV_KEYBOARD_o:
				if (mon.serpent.direction != EST) {
					mon.serpent.direction = OUEST;
				}
				break;
			case MLV_KEYBOARD_SPACE:
				MLV_wait_keyboard(NULL, NULL, NULL);
				break;
		}
		/* Si le délai n'a pas été atteint, on attend la fin du délai */
		if (delai < duree_tour) {
			MLV_wait_milliseconds(duree_tour - delai);
		}
	}

	/* Effacer la fenêtre */
	MLV_clear_window(MLV_COLOR_WHITE);

	/* Affiche la fin de partie avec le score */
	MLV_draw_text(TAILLE_CASE, TAILLE_CASE*(hauteur/2), "Partie terminée, score : %d", MLV_COLOR_RED, mon.nb_pommes_mangees);

	/* Actualiser l'affichage */
	MLV_actualise_window();

	MLV_wait_keyboard(NULL, NULL, NULL);

	return 0;
}
