#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "serpent.h"

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

