/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "exec_parser.h"
#include "utils.h"

#define PAGE_SIZE getpagesize()

static so_exec_t *exec;
static struct sigaction old_action;
static int fd;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	if (signum != SIGSEGV) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	char *rc;
	char *addr = (char *)info->si_addr;
	so_seg_t *found_segment = NULL;

	// found the segment which caused the page fault
	for (int i = 0; i < exec->segments_no; i++)
		if (addr >= (char *)exec->segments[i].vaddr &&
			addr <= (char *)exec->segments[i].vaddr + exec->segments[i].mem_size) {
			found_segment = &exec->segments[i];
			break;
		}

	// segment not found
	if (found_segment == NULL) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	// compute the index of the page so that the pages will be alligned with
	// the segment
	int addr_offset = (int)(addr - found_segment->vaddr);
	int page_index = addr_offset / PAGE_SIZE;

	// check if the page of the segment is already mapped
	if (((char *)found_segment->data)[page_index] == 1) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	// map the address where the page fault occured and load the information
	// from the ELF executable
	rc = mmap((char *)(found_segment->vaddr + PAGE_SIZE * page_index),
		PAGE_SIZE,
		PROT_WRITE,
		MAP_PRIVATE | MAP_FIXED,
		fd, found_segment->offset + PAGE_SIZE * page_index);
	DIE(rc == MAP_FAILED, "mmap failed");

	// bss case: when the allocated page is bigger than the size in the file
	// set the rest of the memory to 0
	if (found_segment->file_size < found_segment->mem_size && (page_index + 1) * PAGE_SIZE > found_segment->file_size)
		memset((char *)(found_segment->vaddr + found_segment->file_size),
			0,
			(page_index + 1) * PAGE_SIZE - found_segment->file_size);

	// set the permissions of the mapping
	mprotect(rc, PAGE_SIZE, found_segment->perm);

	// set the page as mapped
	((char *)found_segment->data)[page_index] = 1;
}

static void set_signal(void)
{
	struct sigaction action;
	int rc;

	action.sa_sigaction = segv_handler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGSEGV);
	action.sa_flags = SA_SIGINFO;

	rc = sigaction(SIGSEGV, &action, &old_action);
	DIE(rc == -1, "sigaction");
}

int so_init_loader(void)
{
	// set the handler for SIGSEGV signal
	set_signal();

	return -1;
}

static void so_free_memory(void)
{
	// free the data vector
	int rc;

	for (int i = 0; i < exec->segments_no; i++) {
		so_seg_t *segment = &exec->segments[i];
		for (char *addr = (char *)segment->vaddr; addr <= (char *)(segment->vaddr + segment->mem_size); addr += PAGE_SIZE) {
			rc = munmap(addr, PAGE_SIZE);
			DIE(rc == -1, "umnap error");
		}
		free(segment->data);
		free(segment);
	}
	free(exec->segments);
	free(exec);
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	// open the file and save the descriptor in a static variable
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;

	// allocate memory for an array to save if the memory was mapped
	for (int i = 0; i < exec->segments_no; i++) {
		so_seg_t *segment = &exec->segments[i];
		int number_of_pages = segment->mem_size / PAGE_SIZE;

		segment->data = calloc((segment->mem_size % PAGE_SIZE == 0) ? number_of_pages : number_of_pages + 1, sizeof(char));
		if (segment->data == NULL)
			return -1;
	}

	so_start_exec(exec, argv);
	so_free_memory();

	return -1;
}
