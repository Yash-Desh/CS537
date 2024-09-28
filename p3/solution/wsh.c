#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // for readdir()
#include <unistd.h>   // for system calls
#include <sys/wait.h> // for wait()
#include "wsh.h"

// Piazza @226
// constraints on the input like command length and number of arguments
#define MAXLINE 1024
#define MAXARGS 128

void builtin_ls()
{
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            // Prevent "." & ".." from getting printed
            if (dir->d_name[0] == '.')
                continue;
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}

int arg_parse(char *str, char **arg_arr)
{
    // using strtok()
    // Returns first token
    char *token = strtok(str, " ");

    // count the number of arguments
    int arg_cnt = 0;

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL)
    {
        // printf(" % s\n", token);
        arg_cnt++;
        arg_arr[arg_cnt - 1] = token;
        token = strtok(NULL, " ");
    }

    // debug statement
    printf("tokenization done %d\n", arg_cnt);
    return arg_cnt;
}

void handle_command(char **arg_arr, int arg_cnt)
{
    // exit built-in command
    if ((strcmp(arg_arr[0], "exit") == 0) && (arg_cnt == 1))
    {
        exit(0);
    }

    // ls built-in command
    if ((strcmp(arg_arr[0], "ls") == 0) && (arg_cnt == 1))
    {
        builtin_ls();
    }
}

int main(int argc, char *argv[])
{
    // Add feature to perform batch mode
    if (argc == 2)
    {
        // open argv[1] as a file
        printf("Opening %s as a file\n", argv[1]);
    }
    else if (argc == 1)
    {
        printf("Interactive Mode On\n");
    }

    char *str = NULL;
    size_t size = 0;
    ssize_t len = 0;
    

    do
    {
        printf("wsh> ");
        if((len = getline(&str, &size, stdin)) == -1)
            exit(0);

        // remove new-line character that getline() reads by default
        if (str[strlen(str) - 1] == '\n')
        {
            str[strlen(str) - 1] = '\0';
        }

        // parse the input
        // static array of strings to store user arguments
        char *arg_arr[MAXARGS] = {NULL};
        int arg_cnt = arg_parse(str, arg_arr);

        // exit built-in command
        if ((strcmp(arg_arr[0], "exit") == 0) && (arg_cnt == 1))
        {
            exit(0);
        }

        // ls built-in command
        else if ((strcmp(arg_arr[0], "ls") == 0) && (arg_cnt == 1))
        {
            builtin_ls();
        }

        // cd built-in
        else if (strcmp(arg_arr[0], "cd") == 0)
        {
            // check if it takes only one argument
            if (arg_cnt != 2)
                continue;

            char s[100];

            // using the command
            int rc = chdir(arg_arr[1]);

            if (rc == 0)
            {
                // printing current working directory
                printf("%s\n", getcwd(s, 100));
            }
            else
            {
                printf("chdir error\n");
            }
        }

        // else
        // {
        //     int rc = fork();
        //     if (rc < 0)
        //     {
        //         // fork failed; exit
        //         fprintf(stderr, "fork failed\n");
        //         exit(1);
        //     }
        //     else if (rc == 0)
        //     {
        //         // child (new process)
        //         printf("hello, I am child (pid:%d)\n", (int)getpid());
        //         char *myargs[arg_cnt+1];
        //         for(int i=0; i<arg_cnt; i++)
        //         {
        //             myargs[i] = arg_arr[i];
        //         }
        //         myargs[arg_cnt] = NULL;          // marks end of array
        //         execv(myargs[0], myargs); // runs word count
        //         printf("this shouldn't print out\n");
        //     }
        //     else
        //     {
        //         // parent goes down this path (original process)
        //         int wc = wait(NULL);
        //         printf("hello, I am parent of %d (wc:%d) (pid:%d)\n",
        //                rc, wc, (int)getpid());
        //     }
        // }

    } while ((len != -1));

    // if ((len = getline(&str, &size, stdin)) != -1)
    // {
    //     const char *trimmed = trim(str);

    //     printf("<%s>\n", trimmed); // Use trimmed
    // }
    free(str);
    return 0;
}