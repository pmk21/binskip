#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>

#include "bst.h"
#include "skiplist.h"
#include "test.h"

int rand_seed = 0;

static void seed_rand(int seed) {
  if (!rand_seed)
    srand((unsigned int)seed);
}

int main() {

  // test_insert_get();
  // test_parallel_insert_get();
  test_bst_parallel_insert_get();

  return 0;
}

void test_insert_get() {
  printf("Starting %s\n", __FUNCTION__);

  printf("Allocating a skip list...\n");
  node_t *head = skiplist_init();

  printf("Inserting first element...\n");
  skiplist_insert(head, INT_MIN, "MIN");
  skiplist_insert(head, INT_MAX, "MAX");
  skiplist_insert(head, 5, "5");
  printf("Inserting element...\n");
  skiplist_insert(head, 3, "3");
  printf("Inserting element...\n");
  skiplist_insert(head, 8, "8");
  printf("Inserting element...\n");
  skiplist_insert(head, 13, "13");
  printf("Inserting element...\n");
  skiplist_insert(head, 11, "11");

  skiplist_display(head);

  printf("%s\n", (char *)skiplist_get(head, 11));
  printf("%s\n", (char *)skiplist_get(head, 13));
  printf("%s\n", (char *)skiplist_get(head, 8));
  printf("%s\n", (char *)skiplist_get(head, 3));
  printf("%s\n", (char *)skiplist_get(head, 5));
  if (NULL == skiplist_get(head, 1))
    printf("It's null.\n");

  node_t *preds[SKIPLIST_MAX_HEIGHT] = {NULL}, *succs[SKIPLIST_MAX_HEIGHT] = {NULL};
  printf("findNode: %d\n", skiplist_find_node(head, 3, preds, succs));
  printf("findNode: %d\n", skiplist_find_node(head, 5, preds, succs));
  printf("findNode: %d\n", skiplist_find_node(head, 11, preds, succs));
  printf("findNode: %d\n", skiplist_find_node(head, INT_MAX, preds, succs));
}

static int my_random(int low, int high) {
  double my_rand = rand() / (1.0 + RAND_MAX);
  int range = high - low + 1;

  return (my_rand * range) + low;
}

static void* test_thr(void *head) {
  int key = my_random(-100000, 100000);
  int rkey = my_random(-100000, 100000);
  printf("Key : %d\n", key);
  pskiplist_insert(head, key, "test");
  printf("RKey : %d\n", rkey);
  pskiplist_remove(head, rkey);
  pthread_exit(head);
}

void test_parallel_insert_get(void) {
  node_t *head = skiplist_init();
  int num_thread = 5;
  pthread_t ptids[5];
  void *status;

  printf("Allocating a skip list...\n");
  /* Set the seed for the random number generator */
  seed_rand(2024);
  
  /* Algorithm requires a LSentinel and RSetinel which are 
   * INT_MAX and INT_MIN respectively
   */
  pskiplist_insert(head, INT_MIN, "test");
  pskiplist_insert(head, INT_MAX, "test");
  for (int i = 0; i < num_thread; i++) {
    pthread_create(&ptids[i], NULL, test_thr, head);
  }

  for (int i = 0; i < num_thread; i++) {
    pthread_join(ptids[i], &status);
  }
  skiplist_display(head);
}

static void *test_bst_thread(void *root) {
  int key = my_random(-100000, 100000);
  int rkey = my_random(-100000, 100000);
  printf("Key : %d\n", key);
  bst_add(root, key, "test");
  printf("RKey : %d\n", rkey);
  bst_remove(root, rkey);
  pthread_exit(root);
}

void test_bst_parallel_insert_get(void) {
  bst_node_t *root = bst_initialize();
  int num_thread = 5;
  pthread_t ptids[num_thread];

  seed_rand(2024);
  bst_add(root, INT_MIN + 1, "test");
  bst_add(root, INT_MAX, "test");
  for (int i = 0; i < num_thread; i++) {
    pthread_create(&ptids[i], NULL, test_bst_thread, root);
  }
  for (int i = 0; i < num_thread; i++) {
    pthread_join(ptids[i], NULL);
  }
  bst_print(root);
}
