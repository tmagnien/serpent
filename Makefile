default: serpent

serpent: serpent.o affichage_serpent.o main_serpent.o
	gcc -o serpent serpent.o affichage_serpent.o main_serpent.o -lMLV

serpent.o: serpent.c serpent.h
	gcc -c serpent.c

affichage_serpent.o: affichage_serpent.c affichage_serpent.h
	gcc -c affichage_serpent.c

main_serpent.o: main_serpent.c serpent.h
	gcc -c main_serpent.c
