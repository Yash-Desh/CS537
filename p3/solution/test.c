#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE // for getline()
#include <sys/types.h>
#include <unistd.h>
int main()
{
    printf("Hello world\n");
    // // declarations for the getline() function
    // char *input = NULL;
    // size_t size = 0;
    // ssize_t len = 0;
    // while (1)
    // {
    //     if ((len = getline(&input, &size, stdin)) == -1)
    //     {
    //         break;
    //     }
    // }

    return 0;
}