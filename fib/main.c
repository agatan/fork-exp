#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
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
		char const* cmd = "./child";
		if (argc > 1) {
			cmd = "./echo";
		}
		char *const argv[] = {
			cmd,
			NULL,
		};
		if (execvp(cmd, argv) == -1) {
			perror("execvp");
			exit(1);
		}
	} else {
		// parent
		close(to_child_pipe[0]);
		close(from_child_pipe[1]);
		size_t i;
		char buf[4096];
		for (i = 0; i < 100; i++) {
			ssize_t n;
			if ((n = sprintf(buf, "%ld\n", i)) < 0) {
				perror("sprintf");
				exit(1);
			}
			write(to_child_pipe[1], buf, n);
		}
		close(to_child_pipe[1]);
		ssize_t n;
		while ((n = read(from_child_pipe[0], buf, sizeof(buf))) != 0) {
			write(STDOUT_FILENO, buf, n);
		}
		sleep(1);
	}
}
