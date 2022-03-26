/**
 * SO
 * Lab #3
 *
 * Task #4, Linux
 *
 * FIFO client
 */
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "utils.h"
#include "common.h"	/* PIPE_NAME, BUFSIZE */

ssize_t xwrite(int fd, const void *buf, size_t count)
{
        size_t bytes_written = 0;

        /**
         * While write returns less than count bytes check for:
         *  - <=0 => I/O error
         *  - else continue writing until count is reached
         */
        while (bytes_written < count) {
                ssize_t bytes_written_now = write(fd, buf + bytes_written, count - bytes_written);
                bytes_written += bytes_written_now;
                /* TODO 3 */
        }

        return bytes_written;
}

int main(void)
{
	int fd;
	int rc;
	int bytesWritten;
	int len;
	int total = 0;
	char message[BUFSIZE];

	/* TODO - open named pipe for writing */
	fd = open(PIPE_NAME, O_CREAT | O_WRONLY);

	/* Read message from user */
	memset(message, 0, sizeof(message));
	printf("Message to send:");
	scanf("%s", message);
	len = strlen(message);

	/* TODO - write message to pipe */
	xwrite(fd, message, strlen(message));
	/* close pipe */
	rc = close(fd);
	DIE(rc < 0, "close");

	return 0;
}
