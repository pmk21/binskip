#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <pthread.h>
#include <stdbool.h>

#define SKIPLIST_MAX_HEIGHT 32

typedef struct node {
  int key;
  void *value;
  int top_layer;
  volatile struct node *next[SKIPLIST_MAX_HEIGHT];
  volatile bool marked;
  volatile bool full_linked;
  pthread_mutex_t lock;
} node_t;

#define node_s (sizeof(struct node))

#define LOCK_NODE(_node) pthread_mutex_lock(&((_node)->lock))
#define UNLOCK_NODE(_node) pthread_mutex_unlock(&((_node)->lock))

/* Common helper functions */
int skiplist_remove(node_t *head, int key);
void skiplist_display(node_t *head);
void skiplist_print(node_t *head);
int skiplist_find_node(node_t *head, int key, node_t **preds, node_t **succs);

/* Basic skip list */
node_t *skiplist_init(void);
void skiplist_destroy(node_t *head);
int skiplist_insert(node_t *head, int key, void *value);
void *skiplist_get(node_t *head, int key);

/* Concurrent skip list */
node_t *pskiplist_init(void);
void pskiplist_destroy(node_t *head);
int pskiplist_insert(node_t *head, int key, void *value);
void *pskiplist_get(node_t *head, int key);
int pskiplist_remove(node_t *head, int key);

#endif
