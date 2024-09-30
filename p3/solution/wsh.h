#include <stdio.h>
#include <string.h>
#include <dirent.h> // for readdir()
#include <unistd.h>   // for system calls

// ######################################## Parsers ##############################################

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




// ######################################### Built-in #############################################

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

void builtin_cd(char **arg_arr)
{
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


// void handle_command(char **arg_arr, int arg_cnt)
// {
//     
// }
