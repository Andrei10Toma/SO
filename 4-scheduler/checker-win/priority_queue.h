#include "thread.h"
#include <stdlib.h>

typedef struct node
{
	TThread *thread;
	struct node *next;
} TNode;

TNode *alloc_node(TThread *thread);

void push(TNode **queue, TThread *thread);

void push_without_priority(TNode **queue, TThread *thread);

void pop(TNode **queue);

TThread *peek(TNode *queue);

int is_empty(TNode *queue);

void destroy_queue_and_threads(TNode **queue);

void destroy_queue(TNode **queue);
