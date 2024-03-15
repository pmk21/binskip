#include "test.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#include "skiplist.h"

int main() {

  // test_insert_get();
  test_parallel_insert_get();

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

void test_parallel_insert_get(void) {

  printf("Allocating a skip list...\n");
  node_t *head = skiplist_init();

  pskiplist_insert(head, INT_MIN, "MIN");
  pskiplist_insert(head, INT_MAX, "MAX");
  pskiplist_insert(head, 35, "three");
  pskiplist_insert(head, 5, "three");
  pskiplist_insert(head, 21, "three");
  skiplist_display(head);
  pskiplist_remove(head, 21);
  skiplist_display(head);
}
