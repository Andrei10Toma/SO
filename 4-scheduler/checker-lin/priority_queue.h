#include "thread.h"
#include <stdlib.h>

typedef struct node
{
	TThread *thread;
	struct node *next;
} TNode;

// alloc memory for a node
TNode *alloc_node(TThread *thread);

// add element to the priority queue
void push(TNode **queue, TThread *thread);

// add element to the queue without taking priority in account
void push_without_priority(TNode **queue, TThread *thread);

// remove element from queue
void pop(TNode **queue);

// get element from queue
TThread *peek(TNode *queue);

// check if the queue is empty
int is_empty(TNode *queue);

// free the memory
void destroy_queue_and_threads(TNode **queue);

// destroy only the nodes of the queue and not the threads from the nodes
void destroy_queue(TNode **queue);
