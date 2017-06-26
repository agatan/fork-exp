#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline void ponger(int to_fd, int from_fd, int count) {
	int i;
	char buf[64];
	for (i = 0; i < count; i++) {
		if (read(from_fd, buf, 5) != 5) {
			perror("read ping");
			exit(1);
		}
		buf[5] = '\0';
		if (strncmp("ping\n", buf, 6) != 0) {
			fprintf(stderr, "recieved message is not ping, but %s\n", buf);
			exit(1);
		}
		if (write(to_fd, "pong\n", 5) != 5) {
			perror("write pong");
			exit(1);
		}
	}
}

int main(int argc, char** argv) {
	int count = atoi(argv[1]);
	ponger(1, 0, count);
	return 0;
}
