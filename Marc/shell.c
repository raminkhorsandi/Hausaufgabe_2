#include "./shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(){
	int bg;
	char ** args;

	while(1){
		/* next shell prompt */
		print_prompt();

		/* wait for input */
		args = parse_cmd(&bg);

		/* if valid input, execute */
		if(args){
			execute_cmd(args,bg);
		}
		else {
			printf("Invalid input\n");
		}

		/* free args */
		free(args);
	}
}

/* prints a beautiful prompt */
void print_prompt(){
	printf("\x1B[31mss2018\x1B[0m@\x1B[34msysprog\x1B[0m: ");
}

/* parse commands and allocates memory for it */
char ** parse_cmd(int * bg){

	/* check for EOF */
	if(feof(stdin))exit(1);

	/* check input */
	if(!fgets(input,input_size,stdin))return NULL;
	if(strlen(input) <= 1)return NULL;

	/* Setting up args, argc and bg*/
	int argc = 1;
	char ** args = NULL;
	*bg = 0;

	/* parsing arguments */
	char * next = strtok(input," \n");

	/* check for unparsable input */
	if(!next)return NULL;

	/* check if exiting */
	if(!strcmp(next,"exit")){
		shell_exit();
	}

	/* parse */
	args = (char**) malloc(sizeof(char**));
	args[0] = next;
	while( (next = strtok(NULL," \n")) ){
		args = (char **)realloc(args,sizeof(char**) * (argc + 1));
		args[argc] = next;
		argc++;
	}

	/* check if meant to be executed in the background */
	if(!strcmp(args[argc - 1],"&") && argc > 1){
		*bg = 1;
		args[argc-1] = NULL;
	}
	/* more space to terminate with null pointer */
	else {
		args = (char **)realloc(args,sizeof(char**) * (argc + 1));
		args[argc] = NULL;
	}

	return args;
}

/* --- Platz für eigene Hilfsfunktionen --- */




/* ---------------------------------------- */

/* execute command
 * char * args[]: (arguments)
 *	Enthält den Dateinamen des Auszuführenden Programmes und die
 *	 dazu übergebenen Argumente
 *	args[0]	      --> Dateiname des Programmes
 *	args[1 bis n] --> Argumente
 *	args[n+1]     --> NULL (Nicht wichtig für die Aufgabe)
 *	Gleicher Aufgbau wie char * args[] in der main Methode üblich ist
 *
 * int bg: (background)
 *	flag ob der Prozess im Hintergrund ausgeführt werden soll oder die Shell
 *	 auf den Prozess warten soll
 *	0       --> Shell soll auf den Prozess warten
 *	nicht 0 --> Prozess soll im Hintergrund ausgeführt werden
 */
void execute_cmd(char * args[], int bg){

	/* Relevant für Aufgabe 2 */
	/* clean up children and check if space available */
	if(update_children() >= children_amount){
		printf("Too many processes already running\n");
		return;
	}

	pid_t pid = fork();
	if (0 == pid) { // Kindprozess
		execvp(args[0], &args[0]); // fuehre Programm aus
		printf("Das auszufuehrende Programm konnte nicht gestartet werden.\n");
		return; // bei fehlschlag terminiere kind
	} else { // Elternprozess
		if (0 == bg) { // Vordergrund
			waitpid(pid, NULL, 0); // auf Kindprozess warten
		} else { // Hintergrund
			// im Elternprozess werden die Kindprozesse dokumentiert (nur für Hintergrundprozesse)
			int i;
			for(i = 0; i < PIDS_SIZE && children[i] != 0; ++i) ; // finde erste freie Stelle
			children[i] = pid; // add pid to children;
		}
	}
	return;
}



/* Diese Funktion soll alle Child Prozesse die in dem Array "pid_t children[]"
 *  gespeichert sind darauf überprüfen, ob diese noch laufen oder bereits
 *  beendet sind. Wenn ein Prozess beendet wurde soll der Eintrag in dem Array
 *  gelöscht werden, so dass dieser wieder zur Verfügung steht.
 *
 *  return value:
 *   Diese Funktion soll die Anzahl der momentan im Hintergrund laufenden
 *   Prozesse zurückgeben. Prozesse die beendet wurden zählen nicht dazu
 */
int update_children(){
	int count = 0;
	for(int i = 0; i < PIDS_SIZE; ++i) { // durchlaufe children
		// check ob children[i] einen Prozess beinhaltet & ist terminiert?
		if (children[i] != 0 && waitpid(children[i], NULL, WNOHANG) != 0) {
			children[i] = 0;
		}
		if (children[i] != 0) // beruecksichtige alle eintraege != 0 um aktive prozesse zu zaehlen
			++count;
	}

	return count;
}

/* Diese Funktion wird aufgerufen, falls das Stichwort "exit" in der Shell
 *  eingetippt wird. Diese Funktion beendet die Shell, jedoch soll sie zuerst darauf
 *  warten, dass alle Hintergrundprozesse beendet wurden.
 */
void shell_exit(){
	// prozesse, die aktuell noch laufenden
	int count = update_children();
	int count_alt = -1;
	while (count > 0) {
		count = update_children(); // update count
		if (count_alt != count) { // wenn sich der count veraendert hat, neue ausgabe
			if (1 < count) {
				printf("Es laufen noch %d Programme.\n", count);
			} else if (1 == count) {
				printf("Es laeuft noch %d Programm.\n", count);
			}
		}
		count_alt = count; // um den alten zustand von count zu speichern
	}

	printf("Shell exiting\n");
	exit(0);
}
