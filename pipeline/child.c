#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *pages;

static inline void spawn(int nchild) {
	int from_child_pipe[2];
	int to_child_pipe[2];
	if (pipe(from_child_pipe) < 0) {
		perror("pipe");
		exit(1);
	}
	if (pipe(to_child_pipe) < 0) {
		perror("pipe");
		exit(1);
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}

	if (pid == 0) {
		close(to_child_pipe[1]);
		if (dup2(to_child_pipe[0], 0) < 0) {
			perror("dup2: stdin");
			exit(1);
		}
		close(from_child_pipe[0]);
		if (dup2(from_child_pipe[1], 1) < 0) {
			perror("dup2: stdout");
			exit(1);
		}
		char* const cmd = "./child";
		char* number;
		asprintf(&number, "%d", nchild-1);
		char* const argv[] = {cmd, number, NULL};
		if (execvp(cmd, argv) < 0) {
			perror("execvp");
			exit(1);
		}
	} else {
		close(to_child_pipe[0]);
		close(from_child_pipe[1]);
		char buf[5];
		ssize_t n;
		for (;;) {
			n = read(0, buf, 4);
			if (n == 0) {
				exit(0);
			}
			if (n < 0) {
				perror("read from stdin in child");
				exit(1);
			}
			buf[n] = '\0';
			if (strncmp(buf, "ping", 4) != 0) {
				fprintf(stderr, "expected input is 'ping', but got '%s'\n", buf);
				exit(1);
			}
			if (write(to_child_pipe[1], "ping", 4) < 0) {
				perror("write");
				exit(1);
			}
			if ((n = read(from_child_pipe[0], buf, 4)) < 0) {
				perror("read");
			}
			buf[n] = '\0';
			if (strncmp(buf, "pong", 4) != 0) {
				fprintf(stderr, "expected output is 'pong', but got '%s'\n", buf);
				exit(1);
			}
			write(1, "pong", 4);
			memset(pages, n, getpagesize() * 4);
		}
	}
}

static inline void ponger() {
		char buf[5];
		ssize_t n;
		for (;;) {
			n = read(0, buf, 4);
			if (n == 0) {
				exit(0);
			}
			if (n != 4) {
				perror("read");
				exit(1);
			}
			buf[n] = '\0';
			if (strncmp(buf, "ping", 4) != 0) {
				fprintf(stderr, "expected input is 'ping', but got '%s'\n", buf);
				exit(1);
			}
			write(1, "pong", 4);
		}
}

int main(int argc, char** argv) {
	pages = malloc(getpagesize() * 4);
	memset(pages, 0, getpagesize() * 4);
	int nchild = atoi(argv[1]);
	if (nchild == 0) {
		ponger();
	} else {
		spawn(nchild);
	}
	return 0;
}
