/*
 * shell.c
 *
 *  Created on: Feb 22, 2017
 *      Author: john (Yehia Qtaish)
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

#define MAXLINE 80 /* Max length of command*/


int main()
{
	char *argv[MAXLINE/2 + 1]; /* Command line array with 40 args*/
	int should_run = 1; /* Flag to determine when to exit*/
	char *buffer = malloc(100*sizeof(MAXLINE)); // allocates memory for array
	pid_t pid, status; //pid number and status for the processes
	printf("CS149 Shell from Yehia Qtaish\n");

	int n;
	while (should_run == 1) {
		printf("YehiaQtaish-600>"); //Prompt
		fflush(stdout); // flush new lnve from printf

		fgets(buffer, MAXLINE, stdin);//Takes in user input

		char *buf;
		buf = strtok(buffer," \r\n"); //breaks the command line arguments
		// into strings by adding NULL terminating characters '\0'
		int argc = 0;
		while(buf != NULL) { //goes thru the string and stores it in argv array
			argv[argc++] = buf; //loops through argv array and add user input broken tokens by space
			buf = strtok(NULL, " \r\n");
		}

		if (strcmp(argv[0], "exit") == 0) { //if user enter exit, exits the program
			should_run = 0;//probably not necessary
			break;
		}

		// decide if the last argument was "&", requesting
		// to run the command as a background process
		int bg = (argc > 0 && strlen(argv[argc-1])==1 && *argv[argc-1]=='&')? 1: 0;
		
		if (bg) printf("To run as a background process...\n");

		if ((pid = fork()) < 0) { //fork child process
			fprintf(stderr, "Fork Failed\n");//if fork fails
			return 1;
		}
		else if (pid == 0) { // if the fork goes through
			printf("Child runs '%s'\n",argv[0]);
			int f = execvp(argv[0], argv); //executes child while parent is waiting
			perror(*argv);

		}
		else // parent waits unless run as a background process (bg=1/TRUE)
		{
			if (!bg) wait(&status);
		}

	} // (while) terminal/command line (command input loop)

	exit(0);
}

