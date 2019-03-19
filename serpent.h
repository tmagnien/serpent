#define N 32
#define M 64
#define TAILLE_CASE 10
#define DUREE_TOUR_MS 250

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

typedef struct {
	Case	c;
} Pomme;

typedef struct {
	Serpent		serpent;
	Pomme		pommes[N*M];
	int		nb_pommes_monde;
	int		nb_pommes_mangees;
} Monde;
