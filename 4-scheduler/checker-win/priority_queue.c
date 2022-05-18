#include "priority_queue.h"
#include <stdio.h>

TNode *alloc_node(TThread *thread)
{
	TNode *node = (TNode *)calloc(1, sizeof(TNode));

	node->thread = thread;
	return node;
}

TThread *peek(TNode *queue)
{
	return queue->thread;
}

void pop(TNode **queue)
{
	if ((*queue)->next != NULL) {
		TNode *old_head = *queue;

		*queue = (*queue)->next;
		free(old_head);
	} else {
		(*queue)->thread = NULL;
	}
}

int is_empty(TNode *head)
{
	return head == NULL;
}

void push(TNode **queue, TThread *thread)
{
	if ((*queue)->thread == NULL) {
		(*queue)->thread = thread;
	} else {
		TNode *new_node = alloc_node(thread);

		if ((*queue)->thread->priority < thread->priority) {
			new_node->next = *queue;
			*queue = new_node;
		} else {
			TNode *p = *queue;

			while (p->next != NULL && p->next->thread->priority >= thread->priority)
				p = p->next;
			new_node->next = p->next;
			p->next = new_node;
		}
	}
}

void push_without_priority(TNode **queue, TThread *thread)
{
	if ((*queue)->thread == NULL)
		(*queue)->thread = thread;
	else {
		TNode *new_node = alloc_node(thread);

		new_node->next = *queue;
		*queue = new_node;
	}
}

void destroy_queue_and_threads(TNode **queue)
{
	TNode *p = *queue;

	while (p != NULL) {
		TNode *tmp = p;

		p = p->next;
		if (tmp != NULL) {
			if (tmp->thread != NULL) {
				CloseHandle(tmp->thread->planned_semaphore);
				CloseHandle(tmp->thread->running_semaphore);
				CloseHandle(tmp->thread->thread_handle);
				free(tmp->thread);
				tmp->thread = NULL;
			}
			free(tmp);
			tmp = NULL;
		}
	}
}

void destroy_queue(TNode **queue)
{
	TNode *p = *queue;

	while (p != NULL) {
		TNode *tmp = p;

		p = p->next;
		if (tmp != NULL) {
			free(tmp);
			tmp = NULL;
		}
	}
}
