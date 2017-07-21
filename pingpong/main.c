#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define RDTSC(X) asm volatile ("rdtsc" : "=A"(X))

static inline void pinger(int to_fd, int from_fd, int count) {
	int i;
	char buf[64];
	for (i = 0; i < count; i++) {
		fprintf(stderr, "parent: #%d\n", i);
		if (write(to_fd, "ping\n", 5) != 5) {
			perror("write ping");
			exit(1);
		}
		fprintf(stderr, "parent: write\n");
		if (read(from_fd, buf, 5) != 5) {
			perror("read pong");
			exit(1);
		}
		fprintf(stderr, "parent: read\n");
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "no number given\n");
		return 1;
	}

	char* number = argv[1];
	int count = atoi(number);

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
			number,
			NULL,
		};
		if (execvp(cmd, argv) == -1) {
			perror("execvp");
			exit(1);
		}
		return 1;
	}
	// parent
	close(to_child_pipe[0]);
	close(from_child_pipe[1]);

	pinger(to_child_pipe[1], from_child_pipe[0], count);

	uint64_t y;
	RDTSC(y);
	printf("elapsed tsc = %lld : %lld / %lld\n", y-x, x, y);

	gettimeofday(&e, NULL);
	printf("time = %ldusec\n", (e.tv_sec - s.tv_sec) * 1000000 + (e.tv_usec - s.tv_usec));

	return 0;
}
