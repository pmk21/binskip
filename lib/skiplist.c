#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "skiplist.h"

int rng_seed = 0;

static int geometric_level_generator(int max) {
  int result = 1;

  while ((result < max) && (random() > (RAND_MAX / 2)))
    ++result;

  return result;
}

static void skiplist_free_node(node_t *node) {
  free(node->value);
  node->value = NULL;
  free(node);
}

node_t *skiplist_init() {
  node_t *head = NULL;

  if (0 == rng_seed) {
    srand((unsigned int)2024);
    rng_seed = 1;
  }
  head = calloc(1, node_s);
  if (head == NULL) {
    printf("ERROR: Out of memory.");
    return NULL;
  }
  head->top_layer = SKIPLIST_MAX_HEIGHT;

  return head;
}

void skiplist_destroy(node_t *head) {
  node_t *curr = head;
  node_t *next = NULL;

  while (curr != NULL) {
    next = curr->next[0];
    skiplist_free_node(curr);
    curr = next;
  }
}

void *skiplist_get(node_t *head, int key) {
  int curr_key;
  node_t *curr = head;
  int level = head->top_layer - 1;

  while (curr != NULL && level >= 0) {
    if (curr->next[level] == NULL)
      level--;
    else {
      curr_key = curr->next[level]->key;
      if (key == curr_key)
        return curr->next[level]->value;
      else if (key > curr_key)
        curr = curr->next[level];
      else
        level--;
    }
  }

  return NULL;
}

int skiplist_insert(node_t *head, int key, void *value) {
  int curr_key, i;
  node_t *new_node = NULL;
  node_t *prev[SKIPLIST_MAX_HEIGHT];
  node_t *curr = head;
  int level = head->top_layer - 1;

  // Find the position where the key is expected
  while (curr != NULL && level >= 0) {
    prev[level] = curr;
    if (curr->next[level] == NULL) {
      level--;
    } else {
      curr_key = curr->next[level]->key;
      if (key == curr_key) { // Found a match, replace the old value
        free(curr->next[level]->value);
        curr->next[level]->value = value;
        return 0;
      } else if (key > curr_key) { // Drop down a level
        level--;
      } else { // Keep going at this level
        curr = curr->next[level];
      }
    }
  }

  // Didn't find it, we need to insert a new entry
  new_node = malloc(node_s);
  if (NULL == new_node) {
    printf("Out of memory.\n");
    return 1;
  }
  new_node->top_layer = geometric_level_generator(head->top_layer);
  new_node->key = key;
  new_node->value = value;
  // Null out pointers above height
  for (i = SKIPLIST_MAX_HEIGHT - 1; i > new_node->top_layer; i--) {
    new_node->next[i] = NULL;
  }
  // Tie in other pointers
  for (i = new_node->top_layer - 1; i >= 0; i--) {
    new_node->next[i] = prev[i]->next[i];
    prev[i]->next[i] = new_node;
  }

  return 0;
}

int skiplist_remove(node_t *head, int key) {
  int curr_key = INT_MAX, i;
  node_t *prev[SKIPLIST_MAX_HEIGHT];
  node_t *curr = head, *condemned = NULL;
  int level = head->top_layer - 1;

  // Find the list node just before the condemned node at every
  // level of the chain
  while (curr != NULL && level >= 0) {
    prev[level] = curr;
    if (curr->next[level] == NULL) {
      level--;
    } else {
      curr_key = curr->next[level]->key;
      if (curr_key >= key) { // Drop down a level
        level--;
      } else { // Keep going at this level
        curr = curr->next[level];
      }
    }
  }

  // We found the match we want, and it's in the next pointer
  if (curr && (curr_key == key)) {
    condemned = curr->next[0];
    // Remove the condemned node from the chain
    for (i = condemned->top_layer - 1; i >= 0; i--) {
      prev[i]->next[i] = condemned->next[i];
    }
    // Free it
    skiplist_free_node(condemned);
    condemned = NULL;
    return 0;
  }

  return 1;
}
