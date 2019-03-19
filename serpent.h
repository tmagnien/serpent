#define N 32
#define M 64
#define TAILLE_CASE 10
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
