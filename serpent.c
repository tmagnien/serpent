#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <MLV/MLV_all.h>

#include "serpent.h"

/* Amélioration #1 (*)    : barre d'espace pour mettre en pause */
/* Amélioration #9 (***)  : liste chaînée de cases pour le serpent */
/* Amélioration #10 (***) : fichier de coniguration */

Pomme pomme_gen_alea(int n, int m)
{
	Pomme	pomme;

	pomme.c.x = random() % n;
	pomme.c.y = random() % m;

	return pomme;
}

int case_est_hors_plateau(Case destination, Monde *mon)
{
	/* La case destination est-elle en-dehors du plateau ? */
	if (destination.x < 0 || destination.x > mon->n || destination.y < 0 || destination.y > mon->m) {
		return 1;
	}

	return 0;
}

int case_appartient_serpent(Case destination, Serpent ser, Monde *mon)
{
	Case *c;

	/* La case destination est-elle dans le plateau */
	if (case_est_hors_plateau(destination, mon)) {
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
	Pomme *p;

	/* La case destination est-elle dans le plateau */
	if (case_est_hors_plateau(destination, mon)) {
		return 0;
	}

	/* La case destination contient-elle une pomme ? */
	p = mon->pommes;
	while (p != NULL) {
		if (destination.x == p->c.x && destination.y == p->c.y) {
			return 1;
		}
		p = p->suiv;
	}

	return 0;
}

int case_est_occupee(Case destination, Monde *mon)
{
	/* Une case est hors plateau ou contient un lmorceau du corps du serpent ou une pomme */
	return (case_est_hors_plateau(destination, mon) ||
		case_appartient_serpent(destination, mon->serpent, mon) ||
		case_est_pomme(destination, mon));
}

void ajouter_pomme_monde(Monde *mon)
{
	Pomme	pomme, *p;

	/* On génère une pomme tant qu'on ne peut pas la placer */
	do {
		pomme = pomme_gen_alea(mon->n, mon->m);
	} while (case_est_occupee(pomme.c, mon));

	/* On peut placer la pomme en tête, l'ordre n'importe pas */
	p = malloc(sizeof(Pomme));
	p->c.x = pomme.c.x;
	p->c.y = pomme.c.y;
	p->suiv = mon->pommes;
	mon->pommes = p;

	/* On incrémente le nombre de pommes du monde */
	mon->nb_pommes_monde++;
}

void retirer_pomme_monde(Case c, Monde *mon)
{
	Pomme *p, *tmp;

	/* On cherche la pomme */
	p = mon->pommes;

	/* Pas de pommes, on ne fait rien */
	if (p == NULL) {
		return;
	}

	/* On parcourt les pommes */
	while (p->suiv != NULL) {
		if (c.x == p->suiv->c.x && c.y == p->suiv->c.y) {
			/* Trouve */
			break;
		}
		p = p->suiv;
	}

	/* Pas trouvé la pomme, on ne fait rien */
	if (p->suiv == NULL) {
		return;
	}

	/* On retire la pomme p->suiv */
	tmp = p->suiv;
	p->suiv = p->suiv->suiv;
	free(tmp);

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

Serpent init_serpent(int taille_serpent, Monde mon)
{
	Serpent ser;
	Case *c;
	int i;

	/* Nouveau serpent de deux cases, au milieu, direction est */
	ser.direction = EST;
	ser.taille = taille_serpent;
	ser.corps = NULL;

	/* Ajout de la tête */
	ajouter_tete_serpent(mon.n / 2, mon.m / 2, &ser);

	/* Corps */
	for (i = 1; i < ser.taille; i++) {
		ajouter_queue_serpent(mon.n / 2, (mon.m / 2) - i, &ser);
	}

	/* On renvoie le serpent */
	return ser;
}

Monde init_monde(int nb_pommes, int n, int m, int taille_serpent)
{
	int i;

	Monde mon;

	/* Un monde vide */
	mon.nb_pommes_mangees = 0;
	mon.nb_pommes_monde = 0;
	mon.pommes = NULL;

	/* De taille NxM */
	mon.n = n;
	mon.m = m;

	/* Un serpent */
	mon.serpent = init_serpent(taille_serpent, mon);

	/* On ajoute les pommes */
	for (i = 0; i < nb_pommes; i++) {
		ajouter_pomme_monde(&mon);
	}

	/* On renvoie le monde */
	return mon;
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
	return (case_est_hors_plateau(destination, mon) || case_appartient_serpent(destination, mon->serpent, mon));
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
	for (i = 0; i <= mon->n; i++) {
		MLV_draw_line(0, i*TAILLE_CASE, mon->m*TAILLE_CASE, i*TAILLE_CASE, MLV_COLOR_BLACK);
	}

	/* On trace les lignes verticales */
	for (i = 0; i <= mon->m; i++) {
		MLV_draw_line(i*TAILLE_CASE, 0, i*TAILLE_CASE, mon->n*TAILLE_CASE, MLV_COLOR_BLACK);
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
	Pomme *p;

	/* Effacer la fenêtre */
	MLV_clear_window(MLV_COLOR_WHITE);

	/* Afficvhage quadrillage */
	afficher_quadrillage(mon);

	/* Affichage serpent */
	afficher_serpent(&mon->serpent);

	/* Affichage pommes */
	p = mon->pommes;
	while (p != NULL) {
		afficher_pomme(p);
		p = p->suiv;
	}

	/* Affichage score */
	MLV_draw_text(TAILLE_CASE, TAILLE_CASE*(N+1), "Score: %d", MLV_COLOR_BLACK, mon->nb_pommes_mangees);

	/* Actualiser l'affichage */
	MLV_actualise_window();
}

void lire_valeur(char *ligne, int *valeur)
{
	char *equal = strchr(ligne, '=');
	if (equal == NULL) {
		return;
	}

	while (equal != NULL && *equal != '\0' && !isdigit(*equal)) {
		equal++;
	}

	if (equal != NULL && *equal != '\0') {
		sscanf(equal, "%d", valeur);
	}
}

void lire_fichier_config(FILE *fp, int *largeur, int *hauteur, int *nombre_pommes, int *taille_serpent, int *duree_tour)
{
	char ligne[80];
	char cle[40];
	char *equal;
	int valeur;

	/* On lit toutes les lignes du fichier */
	while (fgets(ligne, 80, fp)) {
		/* Paramètre à lire */
		if (strncmp(ligne, "largeur", strlen("largeur")) == 0) {
			lire_valeur(ligne, largeur);
		}
		else if (strncmp(ligne, "hauteur", strlen("hauteur")) == 0) {
			lire_valeur(ligne, hauteur);
		}
		else if (strncmp(ligne, "nombre_pommes", strlen("nombre_pommes")) == 0) {
			lire_valeur(ligne, nombre_pommes);
		}
		else if (strncmp(ligne, "taille_serpent", strlen("taille_serpent")) == 0) {
			lire_valeur(ligne, taille_serpent);
		}
		else if (strncmp(ligne, "duree_tour", strlen("duree_tour")) == 0) {
			lire_valeur(ligne, duree_tour);
		}
	}
}

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
