#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define CUL_TITLU 1
#define CUL_LITERA_NEVERIF 2
#define CUL_LITERA_INVALID 3
#define CUL_LITERA_PARTIAL 4
#define CUL_LITERA_VALID 5
#define CUL_FUNDAL 6

#define N_CUVINTE_EXEMPLU 27
#define N_CUVINTE 6
#define N_LITERE 5

#define MENIU_JOC 0
#define MENIU_CONTROL 1
#define TASTA_MENIU_CONTROL ':'
#define TASTA_EXIT 'Q'
#define TASTA_NEW 'N'

#define S_DERULARE 0
#define S_CASTIGAT 1
#define S_PIERDUT 2

typedef struct {
	char caracter;
	int culoare;
} litera;

typedef struct {
	litera tabla[N_CUVINTE][N_LITERE];
	int status, pozitie_cuvant, pozitie_litera, eroare_numar_litere;
	char cuvant[N_LITERE + 1];
} joc;

void initializare_culori() {
	/* Aceasta functie initializeaza culorile ce vor fi folosite in restul 
	programului. */

	start_color();
	init_pair(CUL_TITLU, COLOR_WHITE, COLOR_BLACK);
	init_pair(CUL_LITERA_NEVERIF, COLOR_BLACK, COLOR_WHITE);
	init_pair(CUL_LITERA_INVALID, COLOR_WHITE, COLOR_BLACK);
	init_pair(CUL_LITERA_PARTIAL, COLOR_BLACK, COLOR_YELLOW);
	init_pair(CUL_LITERA_VALID, COLOR_BLACK, COLOR_GREEN);
	init_pair(CUL_FUNDAL, COLOR_BLACK, COLOR_MAGENTA);
}

void afisare_titlu(WINDOW *header_window) {
	/* Aceasta functie primeste pointer-ul catre subfereastra in care se afla 
	titlul si il afiseaza. */
	
	wclear(header_window);
	char titlu[7] = "WORDLE";
	
	// Colorare bara de titlu.
	wbkgd(header_window, COLOR_PAIR(CUL_TITLU));
	
	// Titlu.
	mvwaddstr(header_window, 0, (COLS - strlen(titlu)) / 2, titlu);
	refresh();
	wrefresh(header_window);
}

void afisare_meniu_control(WINDOW *body_window) {
	/* Aceasta functie afiseaza meniul de control. */
	
	char text1[] = "MENIU DE CONTROL";
	char text2[] = "  - Paraseste jocul";
	char text3[] = "  - Incepe un joc nou";
	char text4[] = "  - Inapoi la jocul curent";
	
	text2[0] = TASTA_EXIT;
	text3[0] = TASTA_NEW;
	text4[0] = TASTA_MENIU_CONTROL;
	
	// Stergem ecranul si afisam mesajele.
	wclear(body_window);
	wbkgd(body_window, COLOR_PAIR(CUL_FUNDAL));
	mvwaddstr(body_window, 2, (COLS - strlen(text1)) / 2, text1);
	mvwaddstr(body_window, 3, (COLS - strlen(text2)) / 2, text2);
	mvwaddstr(body_window, 4, (COLS - strlen(text3)) / 2, text3);
	mvwaddstr(body_window, 5, (COLS - strlen(text4)) / 2, text4);
	
	refresh();
	wrefresh(body_window);
}

void coordonate_pixeli(int ln, int col, int coord[2]) {
	/* Functia primeste numarul liniei (0 .. N_CUVINTE - 1) si numarul coloanei 
	(0 .. N_LITERE - 1) pentru o litera de pe tabla de joc si intoarce in 
	parametrul coord cele doua coordonate in pixeli. */
	
	int x, y, dif;
	
	/* Calculam coordonatele de plecare, incepand din mijlocul tablei, stiind 
	ca primul rand este ocupat de titlu. */
	
	y = (LINES - 1) / 2;
	x = COLS / 2;
	
	/* Coordonata y este reprezentativa randului N_CUVINTE / 2 - 1. In functie 
	de numarul liniei, ne deplasam fie in sus, fie in jos. */
	
	dif = ln - (N_CUVINTE / 2 - 1);
	y = y + dif * 2;
		
	/* Coordonata x este reprezentativa coloanei N_LITERE / 2 - 1. In functie 
	de numarul coloanei, ne deplasam la dreapta sau la stanga. */	
	
	dif = col - (N_CUVINTE / 2 - 1);
	x = x + dif * 4;
	
	// Depunem coordonatele gasite in parametru.
	
	coord[0] = x;
	coord[1] = y;
}

void initializare_joc(joc *meci, char cuvinte[][N_LITERE + 1]) {
	/* Functia (re)initializeaza jocul, punand spatii pe fiecare pozitie a 
	tablei si schimband status-ul jocului. */
	
	int i, j, pozitie_random;
	(*meci).status = S_DERULARE;
	(*meci).pozitie_cuvant = 0;
	(*meci).pozitie_litera = 0;
	
	/* Aceasta variabila este utilizata atunci cand utilizatorul introduce 
	un cuvant mai scurt de N_LITERE */
	(*meci).eroare_numar_litere = 0;
	
	// Alegere cuvant
	srand(time(NULL));
	pozitie_random = rand() % N_CUVINTE_EXEMPLU; 
	strcpy((*meci).cuvant, cuvinte[pozitie_random]);
	
	for (i = 0; i < N_CUVINTE; i++) {
		for (j = 0; j < N_LITERE; j++) {
			(*meci).tabla[i][j].caracter = ' ';
			(*meci).tabla[i][j].culoare = CUL_LITERA_NEVERIF;
		}
	}
}

void afisare_joc(WINDOW *body_window, joc *meci) {
	/* Aceasta functie afiseaza tabla jocului si mesajul de castig/infrangere 
	daca exista. */
	
	wclear(body_window);
	wbkgd(body_window, COLOR_PAIR(CUL_FUNDAL));
	int i, j, coord[2];
	char mesaj_status1[40] = "", mesaj_status2[40] = "";
	
	if ((*meci).status == S_CASTIGAT) {
		strcpy(mesaj_status1, "Felicitari! Ai castigat!");
		sprintf(mesaj_status2, "Apasa %c%c pentru a incepe un joc nou.", TASTA_MENIU_CONTROL, TASTA_NEW);
	} else if ((*meci).status == S_PIERDUT) {
		// Afisam si cuvantul
		strcpy(mesaj_status1, "Felicitari! Ai pierdut!");
		sprintf(mesaj_status2, "(%s)", (*meci).cuvant);
	} else if ((*meci).eroare_numar_litere) {
		/* Jucatorul a apasat ENTER fara sa introduca numarul corespunzator de 
		litere. Acest mesaj este temporar si va disparea la urmatoarea actiune. 
		*/
		(*meci).eroare_numar_litere = 0;		
		sprintf(mesaj_status1, "Te rog alege un cuvant din %d litere.", N_LITERE);
		
	}
	
	wattron(body_window, COLOR_PAIR(CUL_TITLU));
	mvwaddstr(body_window, 0, 
		(COLS - strlen(mesaj_status1)) / 2, mesaj_status1);
	mvwaddstr(body_window, 1, 
		(COLS - strlen(mesaj_status2)) / 2, mesaj_status2);
	wattroff(body_window, COLOR_PAIR(CUL_TITLU));
	
	for (i = 0; i < N_CUVINTE; i++) {
		for (j = 0; j < N_LITERE; j++) {
			coordonate_pixeli(i, j, coord);
			wattron(body_window, COLOR_PAIR((*meci).tabla[i][j].culoare));
			mvwaddch(body_window, coord[1], coord[0], 
				(*meci).tabla[i][j].caracter);
			wattroff(body_window, COLOR_PAIR((*meci).tabla[i][j].culoare));
		}
	}
	
	refresh();
	wrefresh(body_window);
}

void adaugare_litera(WINDOW *body_window, joc *meci, char c) {
	/* Functia adauga o litera citita de la tastatura daca aceasta incape 
	in cuvantul curent. */
	
	if ((*meci).status == S_DERULARE && (*meci).pozitie_litera < N_LITERE) {
		(*meci).tabla[(*meci).pozitie_cuvant]
			[(*meci).pozitie_litera].caracter = c;
		(*meci).pozitie_litera = (*meci).pozitie_litera + 1;
		
		afisare_joc(body_window, meci);
	}
}		

void stergere_litera(WINDOW *body_window, joc *meci) {
	// Functia sterge ultima litera de pe randul curent daca aceasta exista.
	
	if ((*meci).status == S_DERULARE && (*meci).pozitie_litera > 0) {
		(*meci).tabla[(*meci).pozitie_cuvant]
			[(*meci).pozitie_litera - 1].caracter = ' ';
		(*meci).pozitie_litera = (*meci).pozitie_litera - 1;
	
		afisare_joc(body_window, meci);
	}
}		

void verificare_cuvant(WINDOW *body_window, joc *meci) {
	/* Functia verifica ultimul cuvant de pe tabla daca acesta este complet 
	si coloreaza literele corespunzator. De asemenea, daca in urma verificarii 
	jucatorul pierde sau castiga, atunci variabila status va fi modificata 
	corespunzator. */
	
	int valid, i, j;
	char caracter_curent;
	
	if ((*meci).status != S_DERULARE) {
		// Jocul nu este in desfasurare...
		return;
	}
	
	if ((*meci).pozitie_litera != N_LITERE) {
		(*meci).eroare_numar_litere = 1;
		afisare_joc(body_window, meci);
	} else {
		// Jocul este in desfasurare, iar ultimul cuvant este complet.
		valid = 1;
		
		for (i = 0; i < N_LITERE; i++) {
			/* Prima data, vom verifica daca litera de pe pozitia curenta din 
			ultimul cuvant coincide cu cea din cuvantul ales la inceputul 
			jocului */
			caracter_curent = (*meci).tabla[(*meci).pozitie_cuvant][i].caracter;
			
			if (caracter_curent == (*meci).cuvant[i]) {
				(*meci).tabla[(*meci).pozitie_cuvant][i].culoare = 
					CUL_LITERA_VALID;
			} else {
				/* Daca nu coincide, in primul rand, marcam faptul ca jucatorul 
				inca nu a castigat si, pe urma, verificam daca litera este una 
				partiala, adica daca se regaseste pe alte pozitii din cuvantul 
				ales la inceputul jocului.*/
				valid = 0;
				
				for (j = 0; j < N_LITERE; j++) {
					if (caracter_curent == (*meci).cuvant[j]) {
						break;
					}
				}
				
				if (j == N_LITERE) {
					// Nu am gasit litera, o coloram ca invalida.
					(*meci).tabla[(*meci).pozitie_cuvant][i].culoare = 
						CUL_LITERA_INVALID;
				} else {
					// Am gasit litera, o coloram ca partiala.
					(*meci).tabla[(*meci).pozitie_cuvant][i].culoare = 
						CUL_LITERA_PARTIAL;
				}
			}
		}
		
		// Trecem la randul urmator.
		(*meci).pozitie_cuvant = (*meci).pozitie_cuvant + 1;
		(*meci).pozitie_litera = 0;
		
		if (valid) {
			// Jucatorul a castigat.
			(*meci).status = S_CASTIGAT;
		} else if ((*meci).pozitie_cuvant == N_CUVINTE) {
			/* Jucatorul nu a castigat si a completat cele N_CUVINTE, deci 
			a pierdut. */
			(*meci).status = S_PIERDUT;
		}
		
		afisare_joc(body_window, meci);
	}
}		

void redimensionare_ferestre(WINDOW *window, WINDOW *header_window, 
	WINDOW *body_window) {
	// Aceasta functie actualizeaza dimensiunile ferestrelor.
	
	resize_term(0, 0);
	wresize(window, LINES, COLS);
	wresize(header_window, 1, COLS);
	wresize(body_window, LINES - 1, COLS);
}

void citire_caractere(WINDOW *window, WINDOW *header_window, 
	WINDOW *body_window, joc *meci, char cuvinte[][N_LITERE + 1]) {
	/* Aceasta functie se ocupa de citirea caracterelor si apelarea functiilor 
	necesare, dupa fiecare caz. */
	
	int meniu = MENIU_JOC;
	int c;
	
	do {
		c = getch();
		
		if (meniu == MENIU_CONTROL) {
			if (c == TASTA_MENIU_CONTROL) {
				// Ne intoarcem la joc
				meniu = MENIU_JOC;
				afisare_joc(body_window, meci);
			} else if (toupper(c) == TASTA_NEW) {
				// Reincepem jocul
				initializare_joc(meci, cuvinte);
				
				// Inchidem meniul de control si ne intoarcem la joc
				meniu = MENIU_JOC;
				afisare_joc(body_window, meci);
				
			} else if (c == KEY_RESIZE) {
				// Redimensionare fereastra terminal
				redimensionare_ferestre(window, header_window, body_window);
				afisare_titlu(header_window);
				afisare_meniu_control(body_window);
			}	
		} else if (meniu == MENIU_JOC) {
			if (c == TASTA_MENIU_CONTROL) {
				// Deschidem meniul de control
				meniu = MENIU_CONTROL;
				afisare_meniu_control(body_window);
			} else if (toupper(c) >= 'A' && toupper(c) <= 'Z') {
				// Am primit o litera in joc
				adaugare_litera(body_window, meci, toupper(c));
			} else if (c == KEY_BACKSPACE) {
				// Backspace
				stergere_litera(body_window, meci);
			} else if (c == '\n') {
				// Enter
				verificare_cuvant(body_window, meci);
			} else if (c == KEY_RESIZE) {
				// Redimensionare fereastra terminal
				redimensionare_ferestre(window, header_window, body_window);
				afisare_titlu(header_window);
				afisare_joc(body_window, meci);
			}
		}
		
	} while (meniu != MENIU_CONTROL || toupper(c) != TASTA_EXIT);
}

int main() {
	WINDOW *window, *header_window, *body_window;
	joc meci;
	char cuvinte[][N_LITERE + 1] = {"ARICI", "ATENT", "BAIAT", "CEATA", "DEBUT", 
		"PESTE", "FIXAT", "HAMAC", "HARTA", "JALON", "JUCAM", "LACAT", "MAGIE", 
		"NUFAR", "OASTE", "PERUS", "RIGLE", "ROMAN", "SANIE", "SCRIS", "SONDA", 
		"TEXTE", "TIPAR", "TITAN", "ZEBRA", "VAPOR", "VATRA"};
	
	use_default_colors();
	initscr();
	
	/* Utilizam functia keypad pentru a separa caracterele speciale care nu 
	corespund unui caracter in ASCII, ca de exemplu sagetile, de restul 
	literelor. De exemplu, daca nu este utilizata aceasta functie, getch 
	primeste ca input de la sageti trei caractere succesiv, printre care si 
	litere ale alfabetului (A, B, C, D) care vor fi afisate in mod nedorit pe 
	tabla de joc. */
	keypad(stdscr, 1);
	
	// Oprim cursorul si echo-ul.
	curs_set(0);
	noecho();
	
	// Subferestre
	window = newwin(LINES, COLS, 0, 0);
	header_window = subwin(window, 1, COLS, 0, 0);
	body_window = subwin(window, LINES - 1, COLS, 1, 0);
	
	initializare_culori();
	afisare_titlu(header_window);
	initializare_joc(&meci, cuvinte);
	afisare_joc(body_window, &meci);
	
	citire_caractere(window, header_window, body_window, &meci, cuvinte);
	endwin();

	return 0;
}

