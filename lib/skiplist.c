#include <immintrin.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

#include "skiplist.h"

int rng_seed = 0;
gmp_randstate_t gmp_state;

static inline void nop_rep(uint32_t num_reps) {
  uint32_t i;
  for (i = 0; i < num_reps; i++) {
	asm volatile ("");
  }
}

static inline int get_rand_level(int max) {
  int i, level = 1;
  mpz_t rop, rnd_max;
  unsigned int rval;

  mpz_init_set_ui(rnd_max, 101);

  mpz_init(rop);
  for (i = 0; i < max - 1; i++) {
    mpz_urandomm(rop, gmp_state, rnd_max);
    rval = mpz_get_ui(rop);
    if (rval < 50)
      level++;
    else
      break;
  }
  return level;
}

static int geometric_level_generator(int max) {
  int result = 1;

  while ((result < max) && (random() > (RAND_MAX / 2)))
    ++result;

  return result;
}

static node_t *skiplist_new_node(int key, void *value, int level) {
  node_t *new_node;

  new_node = malloc(node_s);
  if (NULL == new_node) {
    printf("Out of memory.\n");
    return NULL;
  }
  new_node->top_layer = level;
  new_node->key = key;
  new_node->value = value;
  new_node->marked = false;
  new_node->full_linked = false;
  pthread_mutex_init(&new_node->lock, NULL);

  return new_node;
}

node_t *skiplist_init() {
  node_t *head = NULL;

  if (0 == rng_seed) {
    srand((unsigned int)2024);
    gmp_randinit_default(gmp_state);
    gmp_randseed_ui(gmp_state, 2024l);
    rng_seed = 1;
  }
  head = calloc(1, node_s);
  if (head == NULL) {
    printf("ERROR: Out of memory.");
    return NULL;
  }
  head->top_layer = SKIPLIST_MAX_HEIGHT;
  pthread_mutex_init(&head->lock, NULL);

  skiplist_insert(head, INT_MIN, "");
  skiplist_insert(head, INT_MAX, "");

  return head;
}

static void skiplist_free_node(node_t *node) {
  free(node->value);
  node->value = NULL;
  free(node);
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
  new_node->top_layer = (key == INT_MAX || key == INT_MIN) ? SKIPLIST_MAX_HEIGHT : geometric_level_generator(head->top_layer);
  new_node->key = key;
  new_node->value = value;
  new_node->full_linked = true;
  new_node->marked = false;
  pthread_mutex_init(&new_node->lock, NULL);
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

void skiplist_print(node_t *head) {
  node_t *curr;
  int i, j;
  int arr[SKIPLIST_MAX_HEIGHT];
	
  for (i=0; i< sizeof arr/sizeof arr[0]; i++) arr[i] = 0;
	
  curr = head;
  do {
    printf("%d", (int) curr->key);
    for (i = 0; i < curr->top_layer; i++) {
      printf("-*");
    }
    arr[curr->top_layer]++;
    printf("\n");
    curr = curr->next[0];
  } while (curr); 
  for (j=0; j<SKIPLIST_MAX_HEIGHT; j++)
    printf("%d nodes of level %d\n", arr[j], j);
}

int skiplist_find_node(node_t *head, int key, node_t **preds, node_t **succs) {
  volatile node_t *curr, *pred;
  int lfound;
Restart:
  curr = NULL;
  pred = head;
  lfound = -1;

  for (int layer = SKIPLIST_MAX_HEIGHT - 1; layer >= 0; layer--) {
    curr = pred->next[layer];

    while (key > curr->key) {
      pred = curr;
      curr = pred->next[layer];
    }
    if (preds != NULL) {
      preds[layer] = pred;
      if (pred->marked)
        goto Restart;
    }
    
    succs[layer] = curr;
    if (lfound == -1 && key == curr->key)
      lfound = layer;
  }

  return lfound;
}

static inline void unlock_levels(node_t *head, node_t **nodes, int highest_level) {
  int i;
  node_t *old = NULL;

  for (i = 0; i <= highest_level; i++) {
    if (old != nodes[i]) {
      UNLOCK_NODE(nodes[i]);
    }
    old = nodes[i];
  }
}

int pskiplist_insert(node_t *head, int key, void *value) {
  int lfound, layer, highest_locked = -1;
  int top_layer = geometric_level_generator(head->top_layer);
  // int top_layer = get_rand_level(head->top_layer);
  volatile node_t *node_found;
  node_t *preds[SKIPLIST_MAX_HEIGHT] = {NULL}, *succs[SKIPLIST_MAX_HEIGHT] = {NULL};
  node_t *pred, *succ, *prev_pred, *new_node;
  bool valid;
  unsigned int backoff = 1;

  if (key == INT_MIN || key == INT_MAX)
    top_layer = SKIPLIST_MAX_HEIGHT - 1;
  while (true) {
    lfound = skiplist_find_node(head, key, preds, succs);
    if (lfound != -1) {
      node_found = succs[lfound];
      if (!node_found->marked) {
        while (!node_found->full_linked) {_mm_pause();}
        return 0;
      }
      continue;
    }
    highest_locked = -1;
    prev_pred = NULL;
    valid = true;
    for (layer = 0; valid && (layer <= top_layer); layer++) {
      pred = preds[layer];
      succ = succs[layer];
      if (pred != prev_pred) {
        pthread_mutex_lock(&pred->lock);
        highest_locked = layer;
        prev_pred = pred;
      }
      /* NOTE: valid should not be changed until and unless pred and succ are not NULL */
      valid = (!pred->marked && !succ->marked &&
        (volatile node_t *)pred->next[layer] == (volatile node_t *)succ);
    }
    if (!valid) {
      unlock_levels(head, preds, highest_locked);
      if (backoff > 5000) {
        nop_rep(backoff & MAX_BACKOFF);
      }
      backoff <<= 1;
      continue;
    }

    new_node = skiplist_new_node(key, value, top_layer);
    for (layer = 0; layer <= top_layer; layer++) {
      new_node->next[layer] = succs[layer];
      preds[layer]->next[layer] = new_node;
    }
    new_node->full_linked = true;
    unlock_levels(head, preds, highest_locked);
    return 1;
  }
}

static inline int ok_to_delete(node_t *node, int found) {
  return (node->full_linked && (node->top_layer == found) && !node->marked);
}

int pskiplist_remove(node_t *head, int key) {
  node_t *succs[SKIPLIST_MAX_HEIGHT], *preds[SKIPLIST_MAX_HEIGHT];
  node_t *node_todel, *prev_pred;
  node_t *pred, *succ;
  int is_marked, toplevel, highest_locked, i, valid, found;
  unsigned int backoff;

  node_todel = NULL;
  is_marked = 0;
  toplevel = -1;
  backoff = 1;

  while (true) {
    found = skiplist_find_node(head, key, preds, succs);

    if (is_marked || (found != -1 && ok_to_delete(succs[found], found))) {
      if (!is_marked) {
        node_todel = succs[found];
        pthread_mutex_lock(&node_todel->lock);
        toplevel = node_todel->top_layer;

        if (node_todel->marked) {
          pthread_mutex_unlock(&node_todel->lock);
          return 0;
        }

        node_todel->marked = 1;
        is_marked = 1;
      }

      highest_locked = -1;
      prev_pred = NULL;
      valid = 1;
      for (i = 0; valid && (i <= toplevel); i++) {
        pred = preds[i];
        succ = succs[i];
        if (pred != prev_pred) {
          LOCK_NODE(pred);
          highest_locked = i;
          prev_pred = pred;
        }
        valid = (!pred->marked && ((volatile node_t*) pred->next[i] == 
					 (volatile node_t*) succ));
      }

      if (!valid) {
        unlock_levels(head, preds, highest_locked);
        if (backoff > 5000) {
          nop_rep(backoff & MAX_BACKOFF);
        }
        backoff <<= 1;
        continue;
      }

      for (i = toplevel; i >= 0; i--) {
        preds[i]->next[i] = node_todel->next[i];
      }

      UNLOCK_NODE(node_todel);
      unlock_levels(head, preds, highest_locked);
      return 1;
    } else {
      return 0;
    }
  }
}

static node_t *skiplist_search_node(node_t *head, int key) {
  int i;
  node_t *pred, *curr, *nd = NULL;

  pred = head;
  for (i = pred->top_layer; i >= 0; i--) {
    curr = pred->next[i];
    while (curr != NULL && key > curr->key) {
      pred = curr;
      curr = pred->next[i];
    }

    if (curr != NULL && key == curr->key) {
      nd = curr;
      break;
    }
  }

  return nd;
}

void *pskiplist_get(node_t *head, int key) {
  void *result = NULL;
  node_t *rnode = skiplist_search_node(head, key);

  if (rnode != NULL && !rnode->marked && rnode->full_linked)
    result = rnode->value;

  return result;
}

int pskiplist_size(node_t *head) {
  int size = 0;
  node_t *curr;

  curr = head->next[0];
  while (curr->next[0] != NULL) {
    if (curr->full_linked && !curr->marked)
      size++;
    curr = curr->next[0];
  }

  return size;
}
