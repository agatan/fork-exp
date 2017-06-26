#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static inline void pinger(int to_fd, int from_fd, int count) {
	int i;
	char buf[64];
	for (i = 0; i < count; i++) {
		printf("#%d\n", i);
		if (write(to_fd, "ping\n", 5) != 5) {
			perror("write ping");
			exit(1);
		}
		if (read(from_fd, buf, 5) != 5) {
			perror("read pong");
			exit(1);
		}
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

	return 0;
}
