/**
 * SO
 * Lab #3
 *
 * Task #5, Linux
 *
 * Use your magic to make this program print 'Hello World'
 */
#include <stdio.h>
#include <unistd.h>	/* fork */
#include <sys/wait.h>	/* wait */

int main(void)
{
	int status;
	if (fork() == 0)
		printf(" Hello ");
	else
		printf(" World ");
	wait(&status);
	return 0;
}
