#define N 32
#define M 64
#define TAILLE_CASE 10

typedef struct {
	int x;
	int y;
} Case;

typedef enum {
	NORD,
	EST,
	SUD,
	OUEST
} Direction;

typedef struct {
	Case		corps[N*M];
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
