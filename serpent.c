#include <stdlib.h>
#include <time.h>
#include <MLV/MLV_all.h>

#include "serpent.h"

Pomme pomme_gen_alea(int n, int m)
{
	Pomme	pomme;

	pomme.c.x = random() % n;
	pomme.c.y = random() % m;

	return pomme;
}

int case_est_hors_plateau(Case destination)
{
	/* La case destination est-elle en-dehors du plateau ? */
	if (destination.x < 0 || destination.x > N || destination.y < 0 || destination.y > M) {
		return 1;
	}

	return 0;
}

int case_appartient_serpent(Case destination, Serpent ser)
{
	int i;

	/* La case destination est-elle dans le plateau */
	if (case_est_hors_plateau(destination)) {
		return 0;
	}

	/* La case destination contient-elle un morceau du serpent ? */
	for (i = 0; i < ser.taille; i++) {
		if (destination.x == ser.corps[i].x && destination.y == ser.corps[i].y) {
			return 1;
		}
	}

	return 0;
}

int case_est_pomme(Case destination, Monde *mon)
{
	int i;

	/* La case destination est-elle dans le plateau */
	if (case_est_hors_plateau(destination)) {
		return 0;
	}

	/* La case destination contient-elle une pomme ? */
	for (i = 0; i < mon->nb_pommes_monde; i++) {
		if (destination.x == mon->pommes[i].c.x && destination.y == mon->pommes[i].c.y) {
			return 1;
		}
	}

	return 0;
}

int case_est_occupee(Case destination, Monde *mon)
{
	/* Une case est hors plateau ou contient un lmorceau du corps du serpent ou une pomme */
	return (case_est_hors_plateau(destination) ||
		case_appartient_serpent(destination, mon->serpent) ||
		case_est_pomme(destination, mon));
}

void ajouter_pomme_monde(Monde *mon)
{
	Pomme	pomme;

	/* On génère une pomme tant qu'on ne peut pas la placer */
	do {
		pomme = pomme_gen_alea(N, M);
	} while (case_est_occupee(pomme.c, mon));

	/* On peut placer la pomme */
	mon->pommes[mon->nb_pommes_monde] = pomme;

	/* On incrémente le nombre de pommes du monde */
	mon->nb_pommes_monde++;
}

void retirer_pomme_monde(Case c, Monde *mon)
{
	int i, j;

	/* On cherche l'index de la pomme */
	for (i = 0; i < mon->nb_pommes_monde; i++) {
		if (c.x == mon->pommes[i].c.x && c.y == mon->pommes[i].c.y) {
			/* Trouve */
			break;
		}
	}

	/* Cas en principe impossible, on n'a pas trouvé la pomme */
	if (i == mon->nb_pommes_monde) {
		return;
	}

	/* On décale les pommes après l'index */
	for (j = i; j < mon->nb_pommes_monde - 1; j++) {
		mon->pommes[j] = mon->pommes[j+1];
	}

	/* On décrémente le nombre de pommes du monde */
	mon->nb_pommes_monde--;
}

Serpent init_serpent()
{
	Serpent ser;

	/* Nouveau serpent de deux cases, au milieu, direction est */
	ser.direction = EST;
	ser.taille = 2;
	ser.corps[0].x = N / 2;
	ser.corps[0].y = M / 2;
	ser.corps[1].x = ser.corps[0].x;
	ser.corps[1].y = ser.corps[0].y - 1;

	/* On renvoie le serpent */
	return ser;
}

Monde init_monde(int nb_pommes)
{
	int i;

	Monde m;

	/* Un monde vide */
	m.nb_pommes_mangees = 0;
	m.nb_pommes_monde = 0;

	/* Un serpent */
	m.serpent = init_serpent();

	/* On ajoute les pommes */
	for (i = 0; i < nb_pommes; i++) {
		ajouter_pomme_monde(&m);
	}

	/* On renvoie le monde */
	return m;
}

Case destination_serpent(Serpent ser)
{
	Case destination;

	/* On vérifie la destination en fonction de la direction du serpent */
	switch(ser.direction) {
		case NORD:
			destination.x = ser.corps[0].x - 1;
			destination.y = ser.corps[0].y;
			break;

		case EST:
			destination.x = ser.corps[0].x;
			destination.y = ser.corps[0].y + 1;
			break;

		case SUD:
			destination.x = ser.corps[0].x + 1;
			destination.y = ser.corps[0].y;
			break;

		case OUEST:
			destination.x = ser.corps[0].x;
			destination.y = ser.corps[0].y - 1;
			break;

		default:
			/* Erreur, direction inconnue */
			exit (1);
	}

	return destination;
}

int deplacer_serpent(Monde *mon)
{
	Case destination;
	int i;

	/* On récupère la case de destination du serpent */
	destination = destination_serpent(mon->serpent);

	/* Si la case n'est pas vide, on renvoie 0 et on ne modifie pas le serpent */
	if (case_est_occupee(destination, mon)) {
		return 0;
	}

	/* On déplace le serpent */
	for (i = mon->serpent.taille - 1; i > 0; --i) {
		mon->serpent.corps[i] = mon->serpent.corps[i-1];
	}
	/* La tête est à la case destination */
	mon->serpent.corps[0] = destination;

	return 1;
}

int mort_serpent(Monde *mon)
{
	Case destination;
	int i;

	/* On récupère la case de destination du serpent */
	destination = destination_serpent(mon->serpent);

	/* Deux cas de mort possibles */
	return (case_est_hors_plateau(destination) || case_appartient_serpent(destination, mon->serpent));
}

int manger_pomme_serpent(Monde *mon)
{
	Case destination;
	int i;

	/* On récupère la case de destination du serpent */
	destination = destination_serpent(mon->serpent);

	/* La destination est-elle bien une pomme ? */
	if(!case_est_pomme(destination, mon)) {
		return 0;
	}

	/* La destination est bien une pomme */

	/* On retire la pomme du monde */
	retirer_pomme_monde(destination, mon);

	/* On incrémente le score */
	mon->nb_pommes_mangees++;

	/* On déplace le serpent */
	deplacer_serpent(mon);

	/* On augmente la taille du serpent */
	mon->serpent.taille++;

	return 1;
}

void afficher_quadrillage(Monde *mon)
{
	int i;

	/* On trace les lignes horizontales */
	for (i = 0; i <= N; i++) {
		MLV_draw_line(0, i*TAILLE_CASE, M*TAILLE_CASE, i*TAILLE_CASE, MLV_COLOR_BLACK);
	}

	/* On trace les lignes verticales */
	for (i = 0; i <= M; i++) {
		MLV_draw_line(i*TAILLE_CASE, 0, i*TAILLE_CASE, N*TAILLE_CASE, MLV_COLOR_BLACK);
	}
}

void afficher_pomme(Pomme *pom)
{
	/* Disque rouge */
	MLV_draw_filled_circle(pom->c.y*TAILLE_CASE + TAILLE_CASE/2, pom->c.x*TAILLE_CASE + TAILLE_CASE/2, TAILLE_CASE*0.4, MLV_COLOR_RED);
}

void afficher_serpent(Serpent *ser)
{
	int i;

	/* Disque noir pour le tête du serpent */
	MLV_draw_filled_circle(ser->corps[0].y*TAILLE_CASE + TAILLE_CASE/2, ser->corps[0].x*TAILLE_CASE + TAILLE_CASE/2, TAILLE_CASE*0.4, MLV_COLOR_BLACK);

	/* Carrés verts pour le corps */
	for (i = 1; i < ser->taille; i++) {
		MLV_draw_filled_rectangle(ser->corps[i].y*TAILLE_CASE + TAILLE_CASE*0.1, ser->corps[i].x*TAILLE_CASE + TAILLE_CASE*0.1, TAILLE_CASE*0.8, TAILLE_CASE*0.8, MLV_COLOR_GREEN);
	}
}

void afficher_monde(Monde *mon)
{
	int i;

	/* Effacer la fenêtre */
	MLV_clear_window(MLV_COLOR_WHITE);

	/* Afficvhage quadrillage */
	afficher_quadrillage(mon);

	/* Affichage serpent */
	afficher_serpent(&mon->serpent);

	/* Affichage pommes */
	for (i = 0; i < mon->nb_pommes_monde; i++) {
		afficher_pomme(&mon->pommes[i]);
	}

	/* Affichage score */
	/* FIXME */

	/* Actualiser l'affichage */
	MLV_actualise_window();
}

int main(int argc, char *argv[])
{
	/* Initialisation random */
	srandom(time(NULL));

	/* Initialisation graphique */
	MLV_create_window("Serpent", "Serpent", TAILLE_CASE*M, TAILLE_CASE*N);

	Monde mon = init_monde(12);

	afficher_monde(&mon);

	MLV_wait_keyboard(NULL, NULL, NULL);

	while (1) {
		/* Mort du serpent, partie terminée */
		if (mort_serpent(&mon)) {
			break;
		}
		/* On essaie de se déplacer sur une case vide */
		if (!deplacer_serpent(&mon)) {
			/* Sinon on essaie de manger une pomme */
			if (manger_pomme_serpent(&mon)) {
				/* SI on a mangé une pomme on en recréée une */
				ajouter_pomme_monde(&mon);
			}
		}
		/* On affiche le monde */
		afficher_monde(&mon);
		/* Détection touche pressée */
		if (MLV_get_keyboard_state(MLV_KEYBOARD_a) == MLV_PRESSED) {
			mon.serpent.direction = NORD;
		}
		else if (MLV_get_keyboard_state(MLV_KEYBOARD_p) == MLV_PRESSED) {
			mon.serpent.direction = EST;
		}
		else if (MLV_get_keyboard_state(MLV_KEYBOARD_q) == MLV_PRESSED) {
			mon.serpent.direction = SUD;
		}
		else if (MLV_get_keyboard_state(MLV_KEYBOARD_o) == MLV_PRESSED) {
			mon.serpent.direction = OUEST;
		}
		MLV_wait_milliseconds(250);
	}

	return 0;
}
