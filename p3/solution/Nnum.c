#include <stdio.h>

int main()
{
    int num;
    int sum=0;
    for (int i = 0; i < 5; i++)
    {
        printf("Enter number : ");
        scanf("%d", &num);
        printf("You entered : %d\n", num);
        sum += num;
    }
    fprintf(stderr, "THIS IS ERROR\n");
    printf("Sum = %d\n", sum);
    return 0;
}