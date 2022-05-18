#include "so_scheduler.h"

typedef struct thread {
	so_handler *handler;
	unsigned int priority;
	unsigned int time_quantum_left;
	tid_t thread_id;
	HANDLE thread_handle;
	HANDLE planned_semaphore;
	HANDLE running_semaphore;
} TThread;
