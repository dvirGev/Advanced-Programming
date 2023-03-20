#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main()
{
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status;
    char *argv[10];
    char prompt[1024] = "hello";

    while (1)
    {
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

        if (!strcmp(argv[i - 3], "prompt") && !strcmp(argv[i - 2], "="))
        {
            strcpy(prompt, argv[i - 1]);
            continue;
        }
        else if (!strcmp(argv[0], "echo"))
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
                    printf("%s", argv[i]);
                }
                printf("\n");
            }
            continue;
        }
        else if (!strcmp(argv[0], "cd"))
        {
            if (chdir((argv[1])) != 0)
            {
                printf("Erro Somthing Bad Happened :(\n");
                return 1;
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

        if (fork() == 0)
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
    }
}
