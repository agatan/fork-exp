#include <sched.h>
#include <string.h>

int main(int argc, char** argv) {
	int count = atoi(argv[1]);
	int i;
	for (i = 0; i < count; i++) {
		sched_yield();
	}
	return 0;
}
