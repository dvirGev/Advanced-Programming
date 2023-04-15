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
char prompt[1024] = "hello:";
static int run = 0;
pid_t pid;
size_t sizeComnds = 0;

typedef struct var_
{
    char name[10];
    char val[50];
} var, *pvar;

void addComnd(char *com, char lastscoms[][50])
{
    if (sizeComnds < 20)
    {
        strcpy(lastscoms[sizeComnds], com);
        sizeComnds++;
        return;
    }
    for (size_t i = 0; i < sizeComnds - 1; i++)
    {
        strcpy(lastscoms[i], lastscoms[i + 1]);
    }
    strcpy(lastscoms[sizeComnds - 1], com);
}
void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        if (run)
        {
            if (kill(pid, SIGINT) == -1)
                perror("kill");
        }
        else
        {
            printf("You typed  Controle-C!\n");
        }
    }
    else
    {
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
    int num_commands = 0;
    char *argv[10][10];
    argv[0][0] = NULL;
    int argc[10];
    char last[1024] = "echo dvir the king";
    var variables[50];
    size_t lastVar = 0;

    char lastsComs[20][50];
    signal(SIGINT, sigint_handler);

    while (1)
    {
        if (argv[0][0] != NULL)
        {
            strcpy(last, "");
            for (int k = 0; k < num_commands; k++)
            {
                for (int j = 0; argv[k][j] != NULL; j++)
                {
                    strcat(last, argv[k][j]);
                    strcat(last, " ");
                }
                strcat(last, " | ");
            }
            last[strlen(last) - 3] = '\0';
        }
        addComnd(last, lastsComs);
        printf("%s ", prompt);
        fgets(command, 1024, stdin);
        char temp[1024];
        strcpy(temp, command);
        size_t index = sizeComnds;
        while (strlen(temp) >= 3 && temp[0] == '\x1b' && temp[1] == '[')
        {
            if (index > 0 && temp[2] == 'A')
            {
                index--;
            }
            if (index < 20 && temp[2] == 'B')
            {
                index++;
            }
            strcpy(command, lastsComs[index]);
            printf("\033[1A"); // line up
            printf("\x1b[2K"); // delete line
            printf("%s: %s", prompt, command);
            fgets(temp, 1024, stdin);
        }
        if (command[0] == '\n')
        {
            continue;
            ;
        }
        command[strlen(command) - 1] = '\0';
        /* parse command line */
        i = 0;
        num_commands = 0;
        token = strtok(command, " ");
        while (token != NULL)
        {
            argv[num_commands][i] = token;
            token = strtok(NULL, " ");
            i++;
            if (token && !strcmp(token, "|"))
            {
                token = strtok(NULL, " ");
                argv[num_commands][i] = NULL;
                argc[num_commands] = i;
                i = 0;
                num_commands++;
            }
        }
        argv[num_commands][i] = NULL;
        argc[num_commands] = i;
        num_commands++;
        *argv[num_commands] = NULL;

        for (i = 0; i < num_commands; i++)
        {
            /* Is command empty */
            if (argv[i][0] == NULL)
                continue;
            if (!strcmp(argv[i][0], "quit"))
                return 0;
            else if (!strcmp(argv[i][0], "!!"))
            {
                strcpy(command, last);
                printf("%s\n", command);
                //* parse command line */
                i = 0;
                num_commands = 0;
                token = strtok(command, " ");
                while (token != NULL)
                {
                    argv[num_commands][i] = token;
                    token = strtok(NULL, " ");
                    i++;
                    if (token && !strcmp(token, "|"))
                    {
                        token = strtok(NULL, " ");
                        argv[num_commands][i] = NULL;
                        i = 0;
                        num_commands++;
                    }
                }
                argv[num_commands][i] = NULL;
                num_commands++;
                *argv[num_commands] = NULL;

                i = -1;
                continue;
            }
            if (!strcmp(argv[i][0], "echo"))
            {
                if (!strcmp(argv[i][1], "$?"))
                {
                    printf("%d\n", status);
                }
                else if (*argv[i][1] == '$')
                {

                    for (size_t k = 0; k < lastVar; k++)
                    {
                        if (!strcmp(variables[k].name, argv[i][1]))
                        {
                            printf("%s\n", variables[k].val);
                            break;
                        }
                    }
                }
                else
                {
                    for (size_t j = 1; argv[i][j] != NULL; j++)
                    {
                        printf("%s ", argv[i][j]);
                    }
                    printf("\n");
                }
                continue;
            }
            else if (!strcmp(argv[i][0], "prompt"))
            {
                if (!strcmp(argv[i][1], "="))
                {
                    strcpy(prompt, argv[i][2]);
                }
                continue;
            }
            else if (!strcmp(argv[i][0], "cd"))
            {
                if (chdir((argv[i][1])) != 0)
                {
                    printf("Erro Somthing Bad Happened :(\n");
                }
                continue;
            }
            else if (!strcmp(argv[i][0], "read"))
            {
                if (lastVar < 50)
                {
                    strcpy(variables[lastVar].name, "$");
                    strcat(variables[lastVar].name, argv[i][1]);
                    gets(variables[lastVar].val, 50);
                    lastVar++;
                }
            }
            else if (*argv[i][0] == '$' && !strcmp(argv[i][1], "="))
            {
                if (lastVar < 50)
                {
                    strcpy(variables[lastVar].name, argv[i][0]);
                    strcpy(variables[lastVar].val, argv[i][2]);
                    lastVar++;
                }
            }
            else if (!strcmp(argv[i][0], "if"))
            {
                char externalCommand[1024] = "";
                int j = 0;
                while (argv[i][j] != NULL)
                {
                    strcat(externalCommand, argv[i][j]);
                    strcat(externalCommand, " ");
                    j++;
                }
                strcat(externalCommand, "\n");
                char buf[1024] = "";
                fgets(buf, 1024, stdin);
                while (strcmp(buf, "fi\n"))
                {
                    strcat(externalCommand, buf);
                    strcat(externalCommand, " ");
                    memset(buf, 0, sizeof(buf));
                    fgets(buf, 1024, stdin);
                }
                strcat(externalCommand, "fi\n");
                system(externalCommand);
                continue;
            }
            // else if (!strcmp(argv[i][0], "pwd"))
            // {
            //     char cwd[1024];
            //     if (getcwd(cwd, sizeof(cwd)) != NULL)
            //     {
            //         printf("%s", cwd);
            //     }
            //     else
            //     {
            //         perror("getcwd() error");
            //         return 1;
            //     }
            // }
            /* Does command line end with & */
            if (!strcmp(&(*argv[i][argc[i] - 1]), "&"))
            {
                amper = 1;
                argv[i][argc[i] - 1] = NULL;
            }
            else
                amper = 0;
            if (argc[i] >= 3 && !strcmp(&(*argv[i][argc[i] - 2]), ">"))
            {
                redirect = 1;
                argv[i][argc[i] - 2] = NULL;
                outfile = argv[i][argc[i] - 1];
            }
            else if (argc[i] >= 3 && !strcmp(&(*argv[i][argc[i] - 2]), "2>"))
            {
                redirect = 2;
                argv[i][argc[i] - 2] = NULL;
                outfile = argv[i][argc[i] - 1];
            }
            else if (argc[i] >= 3 && !strcmp(&(*argv[i][argc[i] - 2]), ">>"))
            {
                redirect = 3;
                argv[i][argc[i] - 2] = NULL;
                outfile = argv[i][argc[i] - 1];
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
                printf("\n");
                execvp(argv[i][0], argv[i]);
            }
            /* parent continues here */
            if (amper == 0)
                retid = wait(&status);
            run = 0;
        }
    }
}