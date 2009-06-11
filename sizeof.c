#include <stdio.h>
//find sizeof's certain compile options on different architectures
int main()
{
        printf("char: %d\n", sizeof(char));
        printf("short: %d\n", sizeof(short));
        printf("int: %d\n", sizeof(int));
        printf("long: %d\n", sizeof(long));
        printf("long long: %d\n", sizeof(long long));
        printf("int *: %d\n", sizeof(int *));
	return 0;
}
