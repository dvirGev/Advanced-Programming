#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
// hi
char prompt[1024] = "hello";
static int run = 0;
pid_t pid;

void sigint_handler(int signum)
{
    if(signum == SIGINT) 
    {
        if (run) 
        {
		    if (kill(pid, SIGINT) == -1)
                perror("kill");
        }
    }
    else{
        printf("\nYou typed Control-C!\n");
        write(1, prompt, strlen(prompt));
        write(1, ": ", 2);
    }
}

int main()
{
    char command[1024] = "echo ariel the king";
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10][10];
    char last[1024] = "echo dvir the king";

    signal(SIGINT, sigint_handler);
    while (1)
    {
        if(argv[0] != NULL)
        {
            strcpy(last, "");
            for (size_t i = 0; argv[i] != NULL; i++)
            {
                strcat(last, argv[i]);
                strcat(last, " ");
            }
        }
        printf("%s: ", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';

        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;
        if(!strcmp(argv[0], "quit"))
            exit(0);

        else if (!strcmp(argv[0], "!!"))
        {
            strcpy(command, last);
            // *parse command line *
            i = 0;
            token = strtok(command, " ");
            while (token != NULL)
            {
                argv[i] = token;
                token = strtok(NULL, " ");
                i++;
            }
            argv[i] = NULL;
        }

        if (!strcmp(argv[0], "echo"))
        {
            if (!strcmp(argv[1], "$?"))
            {
                printf("%d\n", status);
                // strcpy(buffer,status);
            }
            else
            {
                for (size_t i = 1; argv[i] != NULL; i++)
                {
                    printf("%s ", argv[i]);
                }
                printf("\n");
            }
            continue;
        }
        else if (!strcmp(argv[0], "prompt"))
        {
            if (!strcmp(argv[1], "="))
            {
                strcpy(prompt, argv[2]);
            }
            continue;
        }
        else if (!strcmp(argv[0], "cd"))
        {
            if (chdir((argv[1])) != 0)
            {
                printf("Erro Somthing Bad Happened :(\n");
            }
            continue;
        }

        /* Does command line end with & */
        if (!strcmp(argv[i - 1], "&"))
        {
            amper = 1;
            argv[i - 1] = NULL;
        }
        else
            amper = 0;

        if (!strcmp(argv[i - 2], ">"))
        {
            redirect = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else if (!strcmp(argv[i - 2], "2>"))
        {
            redirect = 2;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else if (!strcmp(argv[i - 2], ">>"))
        {
            redirect = 3;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else
            redirect = 0;

        /* for commands not part of the shell command language */
        run = 1;
        pid = fork();
        if (pid == 0)
        {
            /* redirection of IO ? */
            if (redirect == 3)
            {
                if ((fd = open(outfile, O_APPEND | O_CREAT | O_WRONLY, 0660)) < 0)
                {
                    perror("Problem in opening the file in append mode");
                    exit(1);
                }
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
            }
            else if (redirect)
            {
                if ((fd = creat(outfile, 0660)) < 0)
                {
                    perror("Problem in opening the file in create mode");
                    exit(1);
                }
                close(redirect);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            }
            execvp(argv[0], argv);
        }
        /* parent continues here */
        if (amper == 0)
            retid = wait(&status);
        run = 0;
    }
}
