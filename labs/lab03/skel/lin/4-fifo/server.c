/**
 * SO
 * Lab #3
 *
 * Task #4, Linux
 *
 * FIFO server
 */
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "utils.h"
#include "common.h"	/* PIPE_NAME, BUFSIZE */

ssize_t xread(int fd, void *buf, size_t count)
{
        size_t bytes_read = 0;

        /**
         * While read returns less than count bytes check for:
         *  - 0 => EOF
         *  - <0 => I/O error
         *  - else continue reading until count is reached
         */
        while (bytes_read < count) {
                ssize_t bytes_read_now = read(fd, buf + bytes_read,
                                                                          count - bytes_read);

                /* TODO 3 */
                if (bytes_read_now == 0)
                        break;
                bytes_read += bytes_read_now;
        }

        return bytes_read;
}

int main(void)
{
	int fd;
	int bytesRead;
	int rc;
	int offset = 0;
	char buff[BUFSIZE];

	/* TODO - create named pipe */
	mkfifo(PIPE_NAME, 0644);

	/* TODO - open named pipe */
	fd = open(PIPE_NAME, O_CREAT | O_RDONLY);

	/* TODO - read in buff from pipe while not EOF */
	memset(buff, 0, sizeof(buff));
	xread(fd, buff, BUFSIZE);

	printf("Message received:%s\n", buff);

	/* Close and delete pipe */
	rc = close(fd);
	DIE(rc < 0, "close");

	rc = unlink(PIPE_NAME);
	DIE(rc < 0, "unlink");

	return 0;
}
