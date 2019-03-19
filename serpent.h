#define N 32
#define M 64
#define DUREE_TOUR_MS 250
#define NB_POMMES 12
#define TAILLE_SERPENT 2
#define FICHIER_CONFIG "Serpent.ini"

typedef struct Case {
	int x;
	int y;
	struct Case *suiv;
} Case;

typedef enum {
	NORD,
	EST,
	SUD,
	OUEST
} Direction;

typedef struct {
	Case		*corps;
	Direction	direction;
	int		taille;
} Serpent;

typedef struct Pomme {
	Case	c;
	struct Pomme *suiv;
} Pomme;

typedef struct {
	int		n, m;
	Serpent		serpent;
	int		nb_pommes_monde;
	int		nb_pommes_mangees;
	Pomme		*pommes;
} Monde;

void lire_fichier_config(FILE *fp, int *largeur, int *hauteur, int *nombre_pommes, int *taille_serpent, int *duree_tour);

Monde init_monde(int nb_pommes, int n, int m, int taille_serpent);
int mort_serpent(Monde *mon);
void ajouter_pomme_monde(Monde *mon);
int deplacer_serpent(Monde *mon, int retirer_queue);
int manger_pomme_serpent(Monde *mon);

