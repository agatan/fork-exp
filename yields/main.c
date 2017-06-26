#include <sched.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "no number given\n");
		return 1;
	}

	char* number = argv[1];
	int count = atoi(number);

	struct timeval s, e;

	gettimeofday(&s, NULL);

	pid_t cpid;
	cpid = fork();
	if (cpid < 0) {
		perror("fork");
		exit(1);
	}
	if (cpid == 0) {
		// child
		fprintf(stderr, "spawn child\n");
		char *const cmd = "./child";
		char *const argv[] = {
			cmd,
			number,
			NULL,
		};
		if (execvp(cmd, argv) == -1) {
			perror("execvp");
			exit(1);
		}
	} else {
		// parent
		fprintf(stderr, "parent\n");

		int i;
		for (i = 0; i < count; i++) {
			sched_yield();
		}

		int x;
		waitpid(cpid, &x, 0);

		gettimeofday(&e, NULL);
    printf("time = %ldusec\n", (e.tv_sec - s.tv_sec) * 1000000 + (e.tv_usec - s.tv_usec));
	}
}
