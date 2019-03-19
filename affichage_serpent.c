#include <MLV/MLV_all.h>
#include "serpent.h"
#include "affichage_serpent.h"

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

