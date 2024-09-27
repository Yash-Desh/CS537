#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
    printf("Hello OS World\n");
    char *str = NULL;
    size_t size = 0;
    ssize_t len = 0;
    char exit_str[] = "exit";
    

    do
    {
        printf("wsh> ");
        len = getline(&str, &size, stdin);
        if(str[strlen(str)-1] == '\n')
        {
            str[strlen(str)-1] = '\0';
        }
    }
    while((len != -1)  && (strcmp(str, exit_str) != 0));

    // if ((len = getline(&str, &size, stdin)) != -1)
    // {
    //     const char *trimmed = trim(str);

    //     printf("<%s>\n", trimmed); // Use trimmed
    // }
    free(str);
    return 0;
}