/**
 * SO
 * Lab #2, Simple I/O operations
 *
 * Task #3, Linux
 *
 * cat/cp applications
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "../utils/utils.h"
#include "xfile.h"

#define BUFSIZE 32

int main(int argc, char **argv)
{
	int fd_src;
	int fd_dst;
	int rc;
	int bytesRead;
	char buffer[BUFSIZE];

	if (argc < 2 || argc > 3) {
		printf("Usage:\n\t%s source_file [destination_file]\n",
			   argv[0]);
		return 0;
	}

	/* TODO 1 - Open source file for reading */
	fd_src = open(argv[1], O_RDONLY);
	DIE(fd_src < 0, "open source file");

	if (argc == 3) {
		fd_dst = open(argv[2], O_WRONLY | O_CREAT, 0644);
		DIE(fd_dst < 0, "open destination file");
		rc = dup2(fd_dst, STDOUT_FILENO);
		DIE(rc < 0, "dup on file");
		/* TODO 2 - Redirect stdout to destination file */
	}

	/**
	 * TODO 1 - Read from file and print to stdout
	 * use _only_ read and write functions
	 * for writing to output use write(STDOUT_FILENO, buffer, bytesRead);
	 */
	bytesRead = xread(fd_src, buffer, BUFSIZE);
	while (bytesRead != 0) {
		DIE(bytesRead < 0, "read from file");
		rc = xwrite(STDOUT_FILENO, buffer, bytesRead);
		DIE(rc < 0, "write stdout");
		bytesRead = xread(fd_src, buffer, BUFSIZE);
	}

	/**
	 * TODO 3 - Change the I/O strategy and implement xread/xwrite. These
	 * functions attempt to read _exactly_ the size provided as parameter.
	 */

	/* TODO 1 - Close file */
	close(fd_src);
	close(fd_dst);
	return 0;
}
