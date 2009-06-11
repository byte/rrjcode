#include <stdio.h>
#include <math.h>
//inefficient implementation of calculating the fibonacci sequence

long int Fib(int N) {
	if (N<=1)
		return 1;
	else
		return Fib(N-1) + Fib(N-2);
}

int main() {
	int n;
	scanf("%d",&n);
	printf("fib %d is %d",n,Fib(n));
	return 0;
}
