#ifndef BST_H
#define BST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define CACHE_LINE_SIZE 64

/* States defined in the paper */
#define STATE_NONE 0
#define STATE_MARK 1
#define STATE_CHILDCAS 2
#define STATE_RELOCATE 3

/* In the relocate_op struct */
#define STATE_OP_ONGOING 0
#define STATE_OP_SUCCESSFUL 1
#define STATE_OP_FAILED 2

/* States for the result of a search operation */
#define FOUND 0x0
#define NOT_FOUND_L 0x1
#define NOT_FOUND_R 0x2
#define ABORT 0x3

#define UNUSED __attribute__ ((unused))
#define CAS_PTR(_a, _b, _c) __sync_val_compare_and_swap((_a), (_b), (_c))
#define CAS_U32(_a, _b, _c) __sync_val_compare_and_swap((_a), (_b), (_c))

typedef struct relocate_op {
  int state; // initialize to ONGOING every time a relocate operation is created
  struct bst_node *dest;
  union operation *dest_op;
  int remove_key;
  void *remove_value;
  int replace_key;
  void *replace_value;
} relocate_op_t;

typedef struct child_cas_op {
  bool is_left;
  struct bst_node *expected;
  struct bst_node *update;
} child_cas_op_t;

typedef union operation {
  struct child_cas_op child_cas_op;
  struct relocate_op relocate_op;
  uint8_t padding[CACHE_LINE_SIZE];
} operation_t;

#define operation_s (sizeof(operation_t))

typedef struct bst_node {
  int key;
  void *value;
  operation_t *op;
  volatile struct bst_node *left;
  volatile struct bst_node *right;
  uint8_t padding[CACHE_LINE_SIZE - sizeof(void *) - sizeof(int) -
                  3 * sizeof(uintptr_t)];
} bst_node_t;

#define bst_node_s (sizeof(struct bst_node))

static inline uint64_t GETFLAG(operation_t *ptr) { return ((uint64_t)ptr) & 3; }

static inline uint64_t FLAG(operation_t *ptr, uint64_t flag) {
  return (((uint64_t)ptr) & 0xfffffffffffffffc) | flag;
}

static inline uint64_t UNFLAG(operation_t *ptr) {
  return (((uint64_t)ptr) & 0xfffffffffffffffc);
}

// Last bit of the node pointer will be set to 1 if the pointer is null
static inline uint64_t ISNULL(volatile bst_node_t *node) {
  return (node == NULL) || (((uint64_t)node) & 1);
}

static inline uint64_t SETNULL(volatile bst_node_t *node) {
  return (((uint64_t)node) & 0xfffffffffffffffe) | 1;
}

bst_node_t *bst_initialize(void);
void *bst_contains(bst_node_t *root, int key);
void *bst_find(bst_node_t *root, int key, bst_node_t **pred, operation_t **pred_op, bst_node_t **curr, operation_t **curr_op, bst_node_t *aux_root);
bool bst_add(bst_node_t *root, int key, void *val);
void *bst_remove(bst_node_t *root, int key);
void bst_print(volatile bst_node_t *node);

#endif
