#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <MLV/MLV_all.h>

#include "serpent.h"

/* Amélioration #1 (*)   : barre d'espace pour mettre en pause */
/* Amélioration #9 (***) : liste chaînée de cases pour le serpent */

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
	Case *c;

	/* La case destination est-elle dans le plateau */
	if (case_est_hors_plateau(destination)) {
		return 0;
	}

	/* La case destination contient-elle un morceau du serpent ? */
	c = ser.corps;
	while (c != NULL) {
		if (destination.x == c->x && destination.y == c->y) {
			return 1;
		}
		c = c->suiv;
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

void ajouter_tete_serpent(int x, int y, Serpent *ser)
{
	Case *c;

	c = malloc(sizeof(Case));
	c->x = x;
	c->y = y;
	c->suiv = ser->corps;
	ser->corps = c;
}

void ajouter_queue_serpent(int x, int y, Serpent *ser)
{
	Case *corps, *c;

	c = malloc(sizeof(Case));
	c->x = x;
	c->y = y;
	c->suiv = NULL;

	corps = ser->corps;
	while (corps->suiv != NULL) {
		corps = corps->suiv;
	}
	corps->suiv = c;
}

void retirer_queue_serpent(Serpent *ser)
{
	Case *corps, *c;

	corps = ser->corps;
	if (corps == NULL) {
		return;
	}
	if (corps->suiv == NULL) {
		/* Le serpent n'a qu'une tête on ne fait rien */
		return;
	}
	while (corps->suiv->suiv != NULL) {
		corps = corps->suiv;
	}
	/* On est sur la queue du serpent */
	free(corps->suiv);
	corps->suiv = NULL;
}

Serpent init_serpent()
{
	Serpent ser;
	Case *c;

	/* Nouveau serpent de deux cases, au milieu, direction est */
	ser.direction = EST;
	ser.taille = 2;
	ser.corps = NULL;

	/* Ajout de la tête */
	ajouter_tete_serpent(N / 2, M / 2, &ser);

	/* Corps */
	ajouter_queue_serpent(N / 2, (M / 2) - 1, &ser);

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
			destination.x = ser.corps->x - 1;
			destination.y = ser.corps->y;
			break;

		case EST:
			destination.x = ser.corps->x;
			destination.y = ser.corps->y + 1;
			break;

		case SUD:
			destination.x = ser.corps->x + 1;
			destination.y = ser.corps->y;
			break;

		case OUEST:
			destination.x = ser.corps->x;
			destination.y = ser.corps->y - 1;
			break;

		default:
			/* Erreur, direction inconnue */
			exit (1);
	}

	return destination;
}

int deplacer_serpent(Monde *mon, int retirer_queue)
{
	Case destination, c;
	int i;

	/* On récupère la case de destination du serpent */
	destination = destination_serpent(mon->serpent);

	/* Si la case n'est pas vide, on renvoie 0 et on ne modifie pas le serpent */
	if (case_est_occupee(destination, mon)) {
		return 0;
	}

	/* On déplace le serpent : la destination devient la nouvelle tête et on retire la queue (si besoin) */
	ajouter_tete_serpent(destination.x, destination.y, &mon->serpent);
	if (retirer_queue) {
		retirer_queue_serpent(&mon->serpent);
	}

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
	deplacer_serpent(mon, 0);

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
	Case *c;

	/* Disque noir pour le tête du serpent */
	MLV_draw_filled_circle(ser->corps->y*TAILLE_CASE + TAILLE_CASE/2, ser->corps->x*TAILLE_CASE + TAILLE_CASE/2, TAILLE_CASE*0.4, MLV_COLOR_BLACK);

	/* Carrés verts pour le corps */
	c = ser->corps->suiv;
	while (c != NULL) {
		MLV_draw_filled_rectangle(c->y*TAILLE_CASE + TAILLE_CASE*0.1, c->x*TAILLE_CASE + TAILLE_CASE*0.1, TAILLE_CASE*0.8, TAILLE_CASE*0.8, MLV_COLOR_GREEN);
		c = c->suiv;
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
	MLV_draw_text(TAILLE_CASE, TAILLE_CASE*(N+1), "Score: %d", MLV_COLOR_BLACK, mon->nb_pommes_mangees);

	/* Actualiser l'affichage */
	MLV_actualise_window();
}

int main(int argc, char *argv[])
{
	MLV_Keyboard_button touche;
	struct timeval debut, fin;
	long delai;

	/* Initialisation random */
	srandom(time(NULL));

	/* Initialisation graphique */
	MLV_create_window("Serpent", "Serpent", TAILLE_CASE*M, TAILLE_CASE*(N+3));

	Monde mon = init_monde(12);

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
			if (delai > DUREE_TOUR_MS) {
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
		if (delai < DUREE_TOUR_MS) {
			MLV_wait_milliseconds(DUREE_TOUR_MS - delai);
		}
	}

	/* Effacer la fenêtre */
	MLV_clear_window(MLV_COLOR_WHITE);

	/* Affiche la fin de partie avec le score */
	MLV_draw_text(TAILLE_CASE, TAILLE_CASE*(N/2), "Partie terminée, score : %d", MLV_COLOR_RED, mon.nb_pommes_mangees);

	/* Actualiser l'affichage */
	MLV_actualise_window();

	MLV_wait_keyboard(NULL, NULL, NULL);

	return 0;
}
