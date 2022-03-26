/**
 * SO
 * Lab #2, Simple I/O operations
 *
 * Task #5, Linux
 *
 * Locking a file
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h> /* flock */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> /* errno */

#include "../utils/utils.h"

#define LOCK_FILE "/tmp/my_lock_file"

static int fdlock = -1;

static void do_stuff(void)
{
	sleep(10);
}

static void check_lock(void)
{
	int rc;

	/* TODO - Open LOCK_FILE file */
	/* fdlock = open(...) */
	fdlock = open(LOCK_FILE, O_CREAT | O_RDONLY, 0644);
	DIE(fdlock < 0, "descriere fisier de lock");

	/**
	 * TODO - Lock the file using flock
	 * - flock must not block in any case !
	 *
	 * - in case of error - print a message showing
	 *   there is another instance running and exit
	 */
	rc = flock(fdlock, LOCK_EX | LOCK_NB);
	if (rc < 0) {
		DIE(errno == EWOULDBLOCK, "Lock already gotten");
	} else {
		DIE(rc > 0, "flock failed");
	}

	printf("\nGot Lock\n\n");
}

static void clean_up(void)
{
	int rc;

	/* TODO - Unlock file, close file and delete it */

	rc = flock(fdlock, LOCK_UN);
	DIE(rc < 0, "flock fail");
	rc = close(fdlock);
	DIE(rc < 0, "close fail");
}

int main(void)
{
	check_lock();

	do_stuff();

	clean_up();

	return 0;
}
