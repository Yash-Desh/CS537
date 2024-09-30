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


int main(int argc, char *argv[])
{
    // entered process wsh
    // setting environment variable of process-wsh to 
    // PATH=/bin as mentioned in write-up
    setenv("PATH", "/bin", 1);

    // shell_vars stores shell variables as a string
    // currently shellvars is an array of static size 100
    char *shellvars[100];
    int shellvars_len = 0;


    // select between batch mode & interactive mode
    if (argc == 2)
    {
        // open argv[1] as a file
        printf("Opening %s as a file\n", argv[1]);
    }
    else if (argc == 1)
    {
        printf("Interactive Mode On\n");
    }

    // declarations for the getline() function
    char *str = NULL;
    size_t size = 0;
    ssize_t len = 0;

    do
    {
        // use fflush to avoid wsh being not print

        printf("wsh> ");
        if ((len = getline(&str, &size, stdin)) == -1)
            exit(0);

        // remove new-line character that getline() reads by default
        if (str[strlen(str) - 1] == '\n')
        {
            str[strlen(str) - 1] = '\0';
        }

        // parse the input based on desired delimiter
        // static array of strings to store user arguments
        char *arg_arr[MAXARGS] = {NULL};
        int arg_cnt = arg_parse(str, arg_arr, " ");

        // ###################### Built-in Commands #######################

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

            builtin_cd(arg_arr);
        }

        else if (strcmp(arg_arr[0], "export") == 0)
        {
            // will the user take care of $PATH
            // builtin_export(arg_arr[1], environ, &environ_len);
        }
        else if (strcmp(arg_arr[0], "local") == 0)
        {
            builtin_local(arg_arr[1], shellvars, &shellvars_len);
        }

        else if(strcmp(arg_arr[0], "vars") == 0)
        {
            builtin_vars(shellvars, shellvars_len);
        }

        // fork+exec
        else if (strcmp(arg_arr[0], "test") == 0)
        {
            int rc = fork();
            if (rc < 0)
            {
                // fork failed; exit
                fprintf(stderr, "fork failed\n");
                exit(1);
            }
            else if (rc == 0)
            {
                // child (new process)
                // printf("hello, I am child (pid:%d)\n", (int)getpid());
                char *myargs[arg_cnt + 1];
                for (int i = 0; i < arg_cnt; i++)
                {
                    myargs[i] = arg_arr[i];
                }
                myargs[arg_cnt] = NULL;   // marks end of array
                execv(myargs[0], myargs); // runs word count
                printf("this shouldn't print out\n");
            }
            else
            {
                // parent goes down this path (original process)
                int wc = wait(NULL);
                printf("hello, I am parent of %d (wc:%d) (pid:%d)\n", rc, wc, (int)getpid());
            }
        }

        // to call process
        else //if(strcmp(arg_arr[0], "ps") == 0)
        {
            int rc = fork();
            if (rc < 0)
            {
                // fork failed; exit
                fprintf(stderr, "fork failed\n");
                exit(1);
            }
            else if (rc == 0)
            {
                // child (new process)
                printf("hello, I am child (pid:%d)\n", (int)getpid());
                char *myargs[arg_cnt + 1];
                for (int i = 0; i < arg_cnt; i++)
                {
                    myargs[i] = arg_arr[i];
                }
                myargs[arg_cnt] = NULL;   // marks end of array

                // pointer to path string
                char *path;
                // path variable has the entire path string now.
                path = getenv("PATH");
                printf("%s\n", path);
                char*path_arg[100];

                // contains the number of added paths
                int path_cnt = arg_parse(path, path_arg, ":");
                printf("%d\n", path_cnt);
                // loop over each added path
                for(int i=0; i<path_cnt; i++)
                {
                    char temp[100];
                    strcpy(temp, path_arg[i]);
                    printf("%s\n", temp);
                    strcat(temp, "/");
                    printf("%s\n", temp);
                    strcat(temp, myargs[0]);
                    printf("%s\n", temp);
                    if(access(temp, X_OK) != -1)
                    {
                        int ret = execv(temp, myargs); // runs word count
                        printf("%d\n", ret);
                    }
                } 
                // strcat(path, myargs[0]);
                // printf("%s\n", path);
                
                // check errno & perror
                printf("%s: command not found\n", myargs[0]);
                // in case failed exec, the child needs to be killed
                exit(1);
            }
            else
            {
                // fflush : glibc buffer 
                // parent goes down this path (original process)
                int wc = wait(NULL);
                printf("hello, I am parent of %d (wc:%d) (pid:%d)\n", rc, wc, (int)getpid());
            }
        }
    } while ((len != -1));

    free(str);
    return 0;
}