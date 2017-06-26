#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "no number given\n");
		return 1;
	}

	char* number = argv[1];

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

	pid_t cpid;
	cpid = fork();
	if (cpid < 0) {
		perror("fork");
		exit(1);
	}
	if (cpid == 0) {
		// child
		fprintf(stderr, "spawn child\n");
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
	} else {
		// parent
		fprintf(stderr, "parent\n");
		close(to_child_pipe[0]);
		close(to_child_pipe[1]);
		close(from_child_pipe[1]);
		ssize_t n;
		char buf[64];
		while ((n = read(from_child_pipe[0], buf, sizeof(buf))) != 0) {
			write(STDOUT_FILENO, "read: ", 6);
			write(STDOUT_FILENO, buf, n);
		}
		close(from_child_pipe[0]);

		fprintf(stderr, "parent: close all pipes\n");

		gettimeofday(&e, NULL);
    printf("time = %ldusec\n", (e.tv_sec - s.tv_sec) * 1000000 + (e.tv_usec - s.tv_usec));
	}
}
