#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

#define RDTSC(X) asm volatile ("rdtsc" : "=A"(X))

static void ping(int to, int from, int niter) {
	for (int i = 0; i < niter; i++) {
		if (write(to, "ping", 4) < 0) {
			perror("write in main");
			exit(1);
		}
		char buf[5];
		ssize_t n;
		if ((n = read(from, buf, 4)) < 0) {
			perror("read in main");
			exit(1);
		}
		buf[n] = '\0';
		if (strncmp(buf, "pong", 4) != 0) {
			fprintf(stderr, "expected output is 'pong', but got '%s'\n", buf);
			exit(1);
		}
	}
}

int main(int argc, char** argv) {
	if (argc < 3) {
		fprintf(stderr, "no number given\n");
		return 1;
	}

	char* nchild = argv[1];
	int niter = atoi(argv[2]);

	int to_child_pipe[2];
	if (pipe(to_child_pipe) == -1) {
		perror("pipe");
		exit(1);
	}
	int from_child_pipe[2];
	if (pipe(from_child_pipe) == -1) {
		perror("pipe");
		exit(1);
	}

	struct timeval s, e;
	gettimeofday(&s, NULL);
	uint64_t x;
	RDTSC(x);

	pid_t cpid;
	cpid = fork();
	if (cpid < 0) {
		perror("fork");
		exit(1);
	}
	if (cpid == 0) {
		// child
		close(to_child_pipe[1]);
		close(from_child_pipe[0]);
		if (dup2(to_child_pipe[0], 0) == -1) {
			perror("dup2");
			exit(1);
		}
		if (dup2(from_child_pipe[1], 1) == -1) {
			perror("dup2");
			exit(1);
		}
		char *const cmd = "./child";
		char *const argv[] = {
			cmd,
			nchild,
			NULL,
		};
		if (execvp(cmd, argv) < 0) {
			perror("execvp");
			exit(1);
		}
	}
	// parent
	close(to_child_pipe[0]);
	close(from_child_pipe[1]);

	ping(to_child_pipe[1], from_child_pipe[0], niter);

	uint64_t y;
	RDTSC(y);
	printf("elapsed tsc = %lld : %lld / %lld\n", y-x, x, y);

	gettimeofday(&e, NULL);
	printf("time = %ldusec\n", (e.tv_sec - s.tv_sec) * 1000000 + (e.tv_usec - s.tv_usec));

	return 0;
}
