#include "so_scheduler.h"
#include "priority_queue.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct scheduler {
	unsigned int time_quantum; // quantum of time for a thread
	unsigned int io; // maximum number of io opperation
	unsigned int thread_counter; // number of threads in READY or RUNNING
	TNode *ready_threads_queue; // threads in the READY state
	TNode **waiting_threads_queue; // list of lists for the threads waiting IO events
	TNode *terminated_threads; // queue for the terminated threads
	TThread *running_thread; // the running thread
	sem_t stop_sem; // semaphore used so that the program knows when to stop
} TScheduler;

static TScheduler *scheduler;

int so_init(unsigned int time_quantum, unsigned int io)
{
	if (time_quantum <= 0)
		return -1;

	if (io > SO_MAX_NUM_EVENTS)
		return -1;

	if (scheduler != NULL)
		return -1;

	scheduler = (TScheduler *)calloc(1, sizeof(TScheduler));
	if (scheduler == NULL)
		return -1;
	scheduler->time_quantum = time_quantum;
	scheduler->io = io;
	scheduler->ready_threads_queue = (TNode *)calloc(1, sizeof(TNode));
	if (scheduler->ready_threads_queue == NULL)
		return -1;
	scheduler->waiting_threads_queue = (TNode **)calloc(io, sizeof(TNode *));
	for (int i = 0; i < io; i++)
		scheduler->waiting_threads_queue[i] = (TNode *)calloc(1, sizeof(TNode));
	scheduler->terminated_threads = (TNode *)calloc(1, sizeof(TNode));
	sem_init(&scheduler->stop_sem, 0, 0);
	return 0;
}

static void plan_thread(TThread *new_thread)
{
	scheduler->thread_counter++;

	// nothing was planned until now
	if (scheduler->running_thread == NULL) {
		scheduler->running_thread = new_thread;
		// the value from the 'so_fork' function can be returned
		sem_post(&scheduler->running_thread->planned_semaphore);
		// run the handler of the thread
		sem_post(&scheduler->running_thread->running_semaphore);
	} else {
		// add the thread to the ready queue
		push(&scheduler->ready_threads_queue, new_thread);
		// the value from the 'so_fork' function can be returned
		sem_post(&new_thread->planned_semaphore);
	}
}

static void *start_thread(void *arg)
{
	TThread *new_thread = (TThread *)arg;

	plan_thread(new_thread);

	// wait for the running
	sem_wait(&new_thread->running_semaphore);
	new_thread->handler(new_thread->priority);
	// thread finished the execution
	push(&scheduler->terminated_threads, new_thread);
	scheduler->thread_counter--;
	// all threads finished so_end can be called
	if (scheduler->thread_counter == 0)
		sem_post(&scheduler->stop_sem);
	else {
		// extract the next_thread to be executed
		TThread *next_thread = peek(scheduler->ready_threads_queue);

		pop(&scheduler->ready_threads_queue);
		scheduler->running_thread = next_thread;
		// run the extracted thread from the queue
		sem_post(&next_thread->running_semaphore);
	}
	pthread_exit(NULL);
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	int rc;

	if (func == NULL)
		return INVALID_TID;

	if (priority > SO_MAX_PRIO)
		return INVALID_TID;

	TThread *new_thread = (TThread *)calloc(1, sizeof(TThread));

	if (new_thread == NULL)
		return INVALID_TID;
	new_thread->handler = func;
	new_thread->priority = priority;
	new_thread->time_quantum_left = scheduler->time_quantum;
	rc = sem_init(&new_thread->planned_semaphore, 0, 0);
	if (rc != 0)
		return INVALID_TID;
	rc = sem_init(&new_thread->running_semaphore, 0, 0);
	if (rc != 0)
		return INVALID_TID;
	rc = pthread_create(&new_thread->thread_id, NULL, start_thread, new_thread);
	if (rc != 0)
		return INVALID_TID;
	// wait for the thread to be scheduled in READY/RUNNING state
	sem_wait(&new_thread->planned_semaphore);
	if (scheduler->running_thread != new_thread)
		so_exec();
	return new_thread->thread_id;
}

int so_wait(unsigned int io)
{
	if (io >= scheduler->io)
		return -1;
	TThread *wait_thread = scheduler->running_thread;

	wait_thread->time_quantum_left = scheduler->time_quantum;
	// push the thread in the waiting_threads_queue
	push_without_priority(&scheduler->waiting_threads_queue[io], wait_thread);
	if (!is_empty(scheduler->ready_threads_queue)) {
		// the thread that will be executed
		TThread *new_thread = peek(scheduler->ready_threads_queue);

		if (new_thread != NULL) {
			scheduler->running_thread = new_thread;
			// remove the thread from the queue
			pop(&scheduler->ready_threads_queue);
			sem_post(&new_thread->running_semaphore);
			sem_wait(&wait_thread->running_semaphore);
		}
	}
	return 0;
}

int so_signal(unsigned int io)
{
	if (io >= scheduler->io)
		return -1;
	int threads_number = 0;
	TThread *signaled_thread = peek(scheduler->waiting_threads_queue[io]);

	// extract all the threads from the IO event queue and add them to the ready queue
	while (!is_empty(scheduler->waiting_threads_queue[io]) && signaled_thread != NULL) {
		signaled_thread = peek(scheduler->waiting_threads_queue[io]);
		if (signaled_thread != NULL) {
			push(&scheduler->ready_threads_queue, signaled_thread);
			pop(&scheduler->waiting_threads_queue[io]);
			threads_number++;
		} else {
			break;
		}
	}
	return threads_number;
}

void so_exec(void)
{
	scheduler->running_thread->time_quantum_left--;
	if (scheduler->running_thread->time_quantum_left == 0) {
		// preempt the thread
		TThread *preempted_thread = scheduler->running_thread;

		// put it back in execution if it is nothing left in queue
		if (is_empty(scheduler->ready_threads_queue)) {
			preempted_thread->time_quantum_left = scheduler->time_quantum;
			return;
		}

		preempted_thread->time_quantum_left = scheduler->time_quantum;
		// put the preempted threa with the queue
		push(&scheduler->ready_threads_queue, preempted_thread);
		TThread *next_thread = peek(scheduler->ready_threads_queue);

		// extract the thread with the highest priority 
		pop(&scheduler->ready_threads_queue);
		scheduler->running_thread = next_thread;
		sem_post(&next_thread->running_semaphore);
		sem_wait(&preempted_thread->running_semaphore);
	}
}

void so_end(void)
{
	if (scheduler != NULL) {
		// wait all threads to call their handler
		if (scheduler->thread_counter != 0)
			sem_wait(&scheduler->stop_sem);
		for (TNode *p = scheduler->terminated_threads; p != NULL; p = p->next) {
			if (p->thread != NULL)
				pthread_join(p->thread->thread_id, NULL);
		}
		destroy_queue_and_threads(&scheduler->terminated_threads);
		destroy_queue(&scheduler->ready_threads_queue);
		for (int i = 0; i < scheduler->io; i++)
			destroy_queue(&scheduler->waiting_threads_queue[i]);
		free(scheduler->waiting_threads_queue);
		free(scheduler);
	}
	scheduler = NULL;
}
