#include "so_scheduler.h"
#include <semaphore.h>

typedef struct thread {
	so_handler *handler; // handler of the thread
	unsigned int priority; // priority of the thread
	unsigned int time_quantum_left; // quantum left for execution
	tid_t thread_id; // id of the thread
	sem_t planned_semaphore; // sempahore used for the planning in 'start_thread' function
	sem_t running_semaphore; // sempahore used for running the handler
} TThread;
