#define _GNU_SOURCE // for getline()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // for readdir()
#include <unistd.h>   // for system calls
#include <sys/wait.h> // for wait()
#include "wsh.h"

// // Piazza @226
// // constraints on the input like command length and number of arguments
// #define MAXLINE 1024
// #define MAXARGS 128

int main(int argc, char *argv[])
{
    // entered process wsh
    // setting environment variable of process-wsh to
    // PATH=/bin as mentioned in write-up
    setenv("PATH", "/bin", 1);

    // Declaring the Linked List to store all the shell variables
    // int shellvars_len = 0;
    // struct shell_var *shellvar_head=NULL;

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
    char *input = NULL;
    size_t size = 0;
    ssize_t len = 0;

    do
    {
        // ######################## Take user input ###########################
        printf("wsh> ");
        if ((len = getline(&input, &size, stdin)) == -1)
            exit(0);

        // remove new-line character that getline() reads by default
        if (input[strlen(input) - 1] == '\n')
        {
            input[strlen(input) - 1] = '\0';
        }

        // ################## parse the input for spaces #######################
        // make a deep copy of the user input
        char *str1 = strdup(input);
        char *arg_arr1[MAXARGS] = {NULL};
        int arg_cnt = arg_parse(str1, arg_arr1, " ");


        // ####################### Handle comments ###############################
        char * comment_token = arg_arr1[0];
        if(comment_token[0] == '#')
        {
            printf("Treated as comment\n");
            continue;
        }

        // ################### handle variable substitution ######################
        char *str5 = strdup(input);
        char *sub_input = NULL;
        for(int i=0; i<arg_cnt; i++)
        {   char * temp_token = arg_arr1[i];
            if(temp_token[0] == '$')
            {
                printf("$ encountered\n");
                sub_input = variable_sub(i, arg_arr1, arg_cnt, str5);
            }
        }
        if(sub_input == NULL)
        {
            sub_input = input;
        }


        // ################### handle history ###################
        // create a deep copy of input
        char *str2 = strdup(sub_input);
        int used_history = 0;
        char * actual_input = history_replace(str2, arg_arr1, arg_cnt, &used_history);

        // only if needed record history
        char *str3 = strdup(sub_input);
        char input_copy[MAXLINE];
        record_input(input_copy, str3);
        if (used_history == 0)
        {
            record_history(input_copy, arg_arr1[0]);
        }

        // execute the needed command
        // create a deep copy of input
        char *str4 = strdup(actual_input);
        char *arg_arr2[MAXARGS] = {NULL};
        arg_cnt = arg_parse(str4, arg_arr2, " ");
        solve(arg_arr2, arg_cnt);

    } while ((len != -1));

    free(input);
    return 0;
}