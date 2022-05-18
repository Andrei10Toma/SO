#include "so_scheduler.h"
#include "priority_queue.h"

typedef struct scheduler {
	unsigned int time_quantum;
	unsigned int io;
	unsigned int thread_counter;
	TNode *ready_threads_queue;
	TNode **waiting_threads_queue;
	TNode *terminated_threads;
	TThread *running_thread;
	HANDLE stop_sem;
} TScheduler;

static TScheduler *scheduler;

int so_init(unsigned int time_quantum, unsigned int io)
{
	int i;

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
	scheduler->waiting_threads_queue =
		(TNode **)calloc(io, sizeof(TNode *));
	for (i = 0; i < io; i++)
		scheduler->waiting_threads_queue[i] = 
			(TNode *)calloc(1, sizeof(TNode));
	scheduler->terminated_threads = (TNode *)calloc(1, sizeof(TNode));
	scheduler->stop_sem = CreateSemaphore(
		NULL,
		0,
		1,
		NULL);

	return 0;
}

static void plan_thread(TThread *new_thread)
{
	scheduler->thread_counter++;

	if (scheduler->running_thread == NULL) {
		scheduler->running_thread = new_thread;
		ReleaseSemaphore(
			scheduler->running_thread->planned_semaphore,
			1,
			NULL);
		ReleaseSemaphore(scheduler->running_thread->running_semaphore,
			1,
			NULL);
	} else {
		push(&scheduler->ready_threads_queue, new_thread);
		ReleaseSemaphore(new_thread->planned_semaphore, 1, NULL);
	}
}

static DWORD WINAPI start_thread(LPVOID arg)
{
	TThread *new_thread = (TThread *)arg;

	plan_thread(new_thread);

	WaitForSingleObject(new_thread->running_semaphore, INFINITE);
	new_thread->handler(new_thread->priority);
	push(&scheduler->terminated_threads, new_thread);
	scheduler->thread_counter--;
	if (scheduler->thread_counter == 0)
		ReleaseSemaphore(scheduler->stop_sem, 1, NULL);
	else {
		TThread *next_thread = peek(scheduler->ready_threads_queue);

		pop(&scheduler->ready_threads_queue);
		scheduler->running_thread = next_thread;
		ReleaseSemaphore(next_thread->running_semaphore, 1, NULL);
	}
	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	TThread *new_thread = (TThread *)calloc(1, sizeof(TThread));

	if (func == NULL)
		return INVALID_TID;

	if (priority > SO_MAX_PRIO)
		return INVALID_TID;


	if (new_thread == NULL)
		return INVALID_TID;
	new_thread->handler = func;
	new_thread->priority = priority;
	new_thread->time_quantum_left = scheduler->time_quantum;
	new_thread->planned_semaphore =
		CreateSemaphore(
		NULL,
		0,
		1,
		NULL);
	new_thread->running_semaphore =
		CreateSemaphore(
		NULL,
		0,
		1,
		NULL);
	 new_thread->thread_handle =
		CreateThread(
			NULL,
			0,
			start_thread,
			new_thread,
			0,
			&new_thread->thread_id);
	WaitForSingleObject(new_thread->planned_semaphore, INFINITE);
	if (scheduler->running_thread != new_thread)
		so_exec();
	return new_thread->thread_id;
}

int so_wait(unsigned int io)
{
	TThread *new_thread;
	TThread *wait_thread;

	if (io >= scheduler->io)
		return -1;
	wait_thread = scheduler->running_thread;

	wait_thread->time_quantum_left = scheduler->time_quantum;
	push_without_priority(
		&scheduler->waiting_threads_queue[io],
		wait_thread);
	if (!is_empty(scheduler->ready_threads_queue)) {
		new_thread = peek(scheduler->ready_threads_queue);

		if (new_thread != NULL) {
			scheduler->running_thread = new_thread;
			pop(&scheduler->ready_threads_queue);
			ReleaseSemaphore(
				new_thread->running_semaphore,
				1,
				NULL);
			WaitForSingleObject(
				wait_thread->running_semaphore,
				INFINITE);
		}
	}
	return 0;
}

int so_signal(unsigned int io)
{
	TThread *signaled_thread;
	int threads_number;

	if (io >= scheduler->io)
		return -1;
	threads_number = 0;
	signaled_thread = peek(scheduler->waiting_threads_queue[io]);

	while (!is_empty(scheduler->waiting_threads_queue[io])
			&& signaled_thread != NULL) {
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
	TThread *preempted_thread;
	TThread *next_thread;

	scheduler->running_thread->time_quantum_left--;
	if (scheduler->running_thread->time_quantum_left == 0) {
		preempted_thread = scheduler->running_thread;

		if (is_empty(scheduler->ready_threads_queue)) {
			preempted_thread->time_quantum_left =
				scheduler->time_quantum;
			return;
		}

		preempted_thread->time_quantum_left = scheduler->time_quantum;
		push(&scheduler->ready_threads_queue, preempted_thread);
		next_thread = peek(scheduler->ready_threads_queue);

		pop(&scheduler->ready_threads_queue);
		scheduler->running_thread = next_thread;
		ReleaseSemaphore(
			next_thread->running_semaphore,
			1,
			NULL);
		WaitForSingleObject(
			preempted_thread->running_semaphore,
			INFINITE);
	}
}

void so_end(void)
{
	int i;
	TNode *p;

	if (scheduler != NULL) {
		if (scheduler->thread_counter != 0)
			WaitForSingleObject(scheduler->stop_sem, INFINITE);
		for (p = scheduler->terminated_threads;
			p != NULL; p = p->next) {
			if (p->thread != NULL)
				WaitForSingleObject(p->thread->thread_handle,
					INFINITE);
		}
		destroy_queue_and_threads(&scheduler->terminated_threads);
		destroy_queue(&scheduler->ready_threads_queue);
		for (i = 0; i < scheduler->io; i++)
			destroy_queue(&scheduler->waiting_threads_queue[i]);
		free(scheduler->waiting_threads_queue);
		CloseHandle(scheduler->stop_sem);
		free(scheduler);
	}
	scheduler = NULL;
}
