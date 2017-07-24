#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline void ponger(int to_fd, int from_fd) {
	int i;
	char buf[64];
	for (i = 0; ; i++) {
		fprintf(stderr, "child: #%d\n", i);
		if (read(from_fd, buf, 5) != 5) {
			perror("read ping");
			exit(1);
		}
		fprintf(stderr, "child: read\n");
		buf[5] = '\0';
		if (strncmp("ping\n", buf, 6) != 0) {
			fprintf(stderr, "recieved message is not ping, but %s\n", buf);
			exit(1);
		}
		if (write(to_fd, "pong\n", 5) != 5) {
			perror("write pong");
			exit(1);
		}
		fprintf(stderr, "child: write\n");
	}
}

int main(int argc, char** argv) {
	int count = atoi(argv[1]);
	ponger(1, 0);
	return 0;
}
