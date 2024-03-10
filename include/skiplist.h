#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <pthread.h>
#include <stdbool.h>

#define SKIPLIST_MAX_HEIGHT 32

typedef struct node {
	int key;
	void *value;
	int top_layer;
	struct node *next[SKIPLIST_MAX_HEIGHT];
	bool marked;
	bool full_linked;
	pthread_mutex_t lock;
} node_t;

#define node_s (sizeof(struct node))

node_t *skiplist_init(void);
void skiplist_destroy(node_t *head);
int skiplist_insert(node_t *head, int key, void *value);
void *skiplist_get(node_t *head, int key);
int skiplist_remove(node_t *head, int key);

#endif
