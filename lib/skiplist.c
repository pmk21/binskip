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
      } else if (curr_key > key) { // Drop down a level
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

void skiplist_display(node_t *head) {
  int level = head->top_layer - 1;
  node_t *curr = head;

  while (curr != NULL && level >= 0) {
    if (curr->next[level] == NULL) {
      printf("\n");
      level--;
      curr = head;
    } else {
      printf("%d ", curr->next[level]->key);
      curr = curr->next[level];
    }
  }
}

int skiplist_find_node(node_t *head, int key, node_t **preds, node_t **succs) {
  node_t *curr = NULL;
  int lfound = -1;
  node_t *pred = head;

  for (int layer = SKIPLIST_MAX_HEIGHT - 1; layer >= 0; layer--) {
    if (pred)
      curr = pred->next[layer];

    while (curr != NULL && key > curr->key) {
      pred = curr;
      curr = pred->next[layer];
    }
    
    if (lfound == -1 && curr != NULL && key == curr->key)
      lfound = layer;

    preds[layer] = pred;
    succs[layer] = curr;
  }

  return lfound;
}

int pskiplist_insert(node_t *head, int key, void *value) {
  int ret = -1;
  int top_layer = geometric_level_generator(head->top_layer);
  node_t *preds[SKIPLIST_MAX_HEIGHT] = {NULL}, *succs[SKIPLIST_MAX_HEIGHT] = {NULL};
  int highest_locked = -1;

  while (true) {
    int lfound = skiplist_find_node(head, key, preds, succs);
    if (lfound != -1) {
      node_t *node_found = succs[lfound];
      if (!node_found->marked) {
        while (!node_found->full_linked) {;}
        ret = 1;
      }
      goto Finally;
    }
    highest_locked = -1;
    node_t *pred, *succ, *prev_pred = NULL;
    bool valid = true;
    for (int layer = 0; valid && (layer <= top_layer); layer++) {
      pred = preds[layer];
      succ = succs[layer];
      if (pred != NULL && pred != prev_pred) {
        pthread_mutex_lock(&pred->lock);
        highest_locked = layer;
        prev_pred = pred;
      }
      if (pred != NULL && succ != NULL)
        valid = pred != NULL && succ != NULL && !pred->marked && !succ->marked &&
          pred->next[layer] == succ;
    }
    if (!valid) goto Finally;

    node_t *new_node = malloc(node_s);
    new_node->top_layer = top_layer;
    new_node->key = key;
    new_node->value = value;
    new_node->marked = false;
    for (int layer = 0; layer <= top_layer; layer++) {
      new_node->next[layer] = succs[layer];
      if (preds[layer])
        preds[layer]->next[layer] = new_node;
    }
    new_node->full_linked = true;
    ret = 0;
Finally:
    if (highest_locked >= 0) {
      for (int j = highest_locked; j >= 0; j--) {
        pthread_mutex_unlock(&preds[j]->lock);
      }
      highest_locked = -1;
    }
    if (ret >= 0)
      return ret;
  }
}

