#include <stdio.h>
#include <stdlib.h>

static int fib(int n) {
	if (n <= 1) {
		return n;
	}
	return fib(n-2) + fib(n-1);
}

int main(int argc, char** argv) {
	int n = atoi(argv[1]);
	printf("%d\n", fib(n));
	fprintf(stderr, "child: finish\n");
	return 0;
}
