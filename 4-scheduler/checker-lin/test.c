#include "priority_queue.h"
#include <stdio.h>

int main(void)
{
	TNode *queue = (TNode *)calloc(1, sizeof(TNode));
	TThread thread1, thread2, thread3, thread4;

	thread1.priority = 3;
	thread2.priority = 2;
	thread3.priority = 4;
	thread4.priority = 1;
	thread1.thread_id = 1;
	thread2.thread_id = 2;
	thread3.thread_id = 3;
	thread4.thread_id = 4;
	push(&queue, &thread1);
	push(&queue, &thread2);
	push(&queue, &thread3);
	push(&queue, &thread4);

	TThread *thread = peek(queue);

	printf("%d %d\n", thread->thread_id, thread->priority);
	pop(&queue);
	thread = peek(queue);
	printf("%d %d\n", thread->thread_id, thread->priority);
	pop(&queue);
	thread = peek(queue);
	printf("%d %d\n", thread->thread_id, thread->priority);
	pop(&queue);
	thread = peek(queue);
	printf("%d %d\n", thread->thread_id, thread->priority);
}
