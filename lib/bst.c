#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bst.h"

const void *val_mask = (void *)~(0x3);

/* Static function definitions */
static bst_node_t *bst_create_node(int key, void *val);
static void bst_help_child_cas(bst_node_t *root, operation_t *op, bst_node_t *dest);
static operation_t *bst_alloc_op(void);
static void bst_help_marked(bst_node_t *root, bst_node_t *pred, operation_t *pred_op, bst_node_t *curr);
static bool bst_help_relocate(bst_node_t *root, operation_t *op, bst_node_t *pred,
                                   operation_t *pred_op, bst_node_t *curr);
static void bst_help(bst_node_t *root, bst_node_t *pred, operation_t *pred_op, bst_node_t *curr,
                     operation_t *curr_op);

static bst_node_t *bst_create_node(int key, void *val) {
  volatile bst_node_t *new_node;

  new_node = malloc(bst_node_s);
  if (new_node == NULL) {
    printf("No memory! Exiting.");
    exit(1);
  }
  new_node->key = key;
  new_node->value = val;
  new_node->op = NULL;
  new_node->left = NULL;
  new_node->right = NULL;

  asm volatile("" ::: "memory");
  return (bst_node_t *)new_node;
}

bst_node_t *bst_initialize(void) {
  /* Assign min key to the root and actual tree will be right subtree
   * of the root node
   */
  bst_node_t *root = bst_create_node(INT_MIN, NULL);

  return root;
}

void *bst_contains(bst_node_t *root, int key) {
  bst_node_t *pred;
  bst_node_t *curr;
  operation_t *pred_op;
  operation_t *curr_op;
  void *res = bst_find(root, key, &pred, &pred_op, &curr, &curr_op, root);
  if ((intptr_t)res & (intptr_t)val_mask) return res;
  return 0;
}

static void bst_help_child_cas(bst_node_t *root, operation_t *op, bst_node_t *dest) {
  bst_node_t **address = NULL;

  if (op->child_cas_op.is_left)
    address = (bst_node_t **) &(dest->left);
  else
    address = (bst_node_t **) &(dest->right);

  CAS_PTR(address, op->child_cas_op.expected, op->child_cas_op.update);
  CAS_PTR(&(dest->op), FLAG(op, STATE_CHILDCAS), FLAG(op, STATE_NONE));
}

static operation_t *bst_alloc_op(void) {
  volatile operation_t *new_op;

  new_op = malloc(operation_s);
  if (new_op == NULL) {
    printf("No memory! Exiting.");
    exit(1);
  }

  return (operation_t *)new_op;
}

static void bst_help_marked(bst_node_t *root, bst_node_t *pred, operation_t *pred_op, bst_node_t *curr) {
  bst_node_t *new_ref;
  operation_t *cas_op;

  if (ISNULL(curr->left)) {
    if (ISNULL(curr->right))
      new_ref = (bst_node_t *)SETNULL(curr);
    else
      new_ref = (bst_node_t *)curr->right;
  } else {
    new_ref = (bst_node_t *)curr->left;
  }

  cas_op = bst_alloc_op();
  cas_op->child_cas_op.is_left = (curr == pred->left);
  cas_op->child_cas_op.expected = curr;
  cas_op->child_cas_op.update = new_ref;

  if (CAS_PTR(&(pred->op), pred_op, FLAG(cas_op, STATE_CHILDCAS)) == pred_op)
    bst_help_child_cas(root, cas_op, pred);
}

static bool bst_help_relocate(bst_node_t *root, operation_t *op, bst_node_t *pred,
                                   operation_t *pred_op, bst_node_t *curr) {
  operation_t *seen_op;
  bool result;
  int seen_state = op->relocate_op.state;

  if (seen_state == STATE_OP_ONGOING) {
    seen_op = CAS_PTR(&(op->relocate_op.dest->op), op->relocate_op.dest_op, FLAG(op, STATE_RELOCATE));
    if ((seen_op = op->relocate_op.dest_op) || (seen_op == (operation_t *)FLAG(op, STATE_MARK))) {
      CAS_U32(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_SUCCESSFUL);
      seen_state = STATE_OP_SUCCESSFUL;
    } else {
      seen_state = CAS_U32(&(op->relocate_op.state), STATE_OP_ONGOING, STATE_OP_FAILED);
    }
  }

  if (seen_state == STATE_OP_SUCCESSFUL) {
    int UNUSED dummy0 = CAS_PTR(&(op->relocate_op.dest->key), op->relocate_op.remove_key, op->relocate_op.replace_key);
    void* UNUSED dummy1 = CAS_PTR(&(op->relocate_op.dest->value), op->relocate_op.remove_value, op->relocate_op.replace_value);
    void* UNUSED dummy2 = CAS_PTR(&(op->relocate_op.dest->op), FLAG(op, STATE_RELOCATE), FLAG(op, STATE_NONE));
  }

  result = (seen_state == STATE_OP_SUCCESSFUL);
  if (op->relocate_op.dest == curr)
    return result;

  void* UNUSED dummy = CAS_PTR(&(curr->op), FLAG(op, STATE_RELOCATE), FLAG(op, result ? STATE_MARK : STATE_NONE));
  if (result) {
    if (op->relocate_op.dest == pred)
      pred_op = (operation_t *)FLAG(op, STATE_NONE);
    bst_help_marked(root, pred, pred_op, curr);
  }
  return result;
}

static void bst_help(bst_node_t *root, bst_node_t *pred, operation_t *pred_op, bst_node_t *curr,
                     operation_t *curr_op) {
  if (GETFLAG(curr_op) == STATE_CHILDCAS)
    bst_help_child_cas(root, (operation_t *)UNFLAG(curr_op), curr);
  else if (GETFLAG(curr_op) == STATE_RELOCATE)
    bst_help_relocate(root, (operation_t *)UNFLAG(curr_op), pred, pred_op, curr);
  else if (GETFLAG(curr_op) == STATE_MARK)
    bst_help_marked(root, pred, pred_op, curr);
}

void *bst_find(bst_node_t *root, int key, bst_node_t **pred, operation_t **pred_op,
               bst_node_t **curr, operation_t **curr_op, bst_node_t * aux_root) {
  void *result;
  int curr_key;
  bst_node_t *next;
  bst_node_t *last_right;
  operation_t *last_right_op;

Retry:
  result = (void *)NOT_FOUND_R;
  *curr = aux_root;
  *curr_op = (*curr)->op;

  if (GETFLAG(*curr_op) != STATE_NONE) {
    if (aux_root == root) {
      bst_help_child_cas(root, (operation_t *)UNFLAG(*curr_op), *curr);
      goto Retry;
    } else
      return (void *)ABORT;
  }

  next = (bst_node_t *)(*curr)->right;
  last_right = *curr;
  last_right_op = *curr_op;
  while(!ISNULL(next)) {
    *pred = *curr;
    *pred_op = *curr_op;
    *curr = next;
    *curr_op = (*curr)->op;

    if (GETFLAG(*curr_op) != STATE_NONE) {
      bst_help(root, *pred, *pred_op, *curr, *curr_op);
      goto Retry;
    }
    curr_key = (*curr)->key;
    if (key < curr_key) {
      result = (void *)NOT_FOUND_L;
      next = (bst_node_t *)(*curr)->left;
    } else if (key > curr_key) {
      result = (void *)NOT_FOUND_R;
      next = (bst_node_t *)(*curr)->right;
      last_right = *curr;
      last_right_op = *curr_op;
    } else {
      result = (*curr)->value;
      break;
    }
  }

  if ((!((intptr_t)result & (intptr_t)val_mask)) && (last_right_op != last_right->op))
    goto Retry;

  if ((*curr)->op != *curr_op)
    goto Retry;

  return result;
}

bool bst_add(bst_node_t *root, int key, void *val) {
  bst_node_t *pred, *curr, *new_node = NULL, *old;
  operation_t *pred_op, *curr_op, *cas_op;
  void *result;
  bool is_left;

  while (true) {
    result = bst_find(root, key, &pred, &pred_op, &curr, &curr_op, root);
    if ((intptr_t)result & (intptr_t)val_mask)
      return false;

    if (new_node == NULL)
      new_node = bst_create_node(key, val);

    is_left = (result == NOT_FOUND_L);
    if (is_left)
      old = (bst_node_t *)curr->left;
    else
      old = (bst_node_t *)curr->right;

    cas_op = bst_alloc_op();
    cas_op->child_cas_op.is_left = is_left;
    cas_op->child_cas_op.expected = old;
    cas_op->child_cas_op.update = new_node;
    if (CAS_PTR(&curr->op, curr_op, FLAG(cas_op, STATE_CHILDCAS)) == curr_op) {
      bst_help_child_cas(root, cas_op, curr);
      return true;
    }
  }
}

void *bst_remove(bst_node_t *root, int key) {
  bst_node_t *pred, *curr, *replace;
  operation_t *pred_op, *curr_op, *replace_op, *reloc_op = NULL;
  void *val, *res;

  while (true) {
    res = bst_find(root, key, &pred, &pred_op, &curr, &curr_op, root);
    if (!((intptr_t)res & (intptr_t)val_mask))
      return NULL;

    if (ISNULL(curr->right) || ISNULL(curr->left)) {
      /* node has less than 2 children */
      if (CAS_PTR(&(curr->op), curr_op, FLAG(curr_op, STATE_MARK)) == curr_op) {
        bst_help_marked(root, pred, pred_op, curr);
        return res;
      }
    } else {
      /* node has 2 children */
      val = bst_find(root, key, &pred, &pred_op, &replace, &replace_op, curr);
      if ((val == ABORT) || (curr->op != curr_op))
        continue;
      reloc_op = bst_alloc_op();
      reloc_op->relocate_op.state = STATE_OP_ONGOING;
      reloc_op->relocate_op.dest = curr;
      reloc_op->relocate_op.dest_op = curr_op;
      reloc_op->relocate_op.remove_key = key;
      reloc_op->relocate_op.remove_value = res;
      reloc_op->relocate_op.replace_key = replace->key;
      reloc_op->relocate_op.replace_value = replace->value;
      if (CAS_PTR(&(replace->op), replace_op, FLAG(reloc_op, STATE_RELOCATE)) == replace_op)
        if (bst_help_relocate(root, reloc_op, pred, pred_op, replace))
          return res;
    }
  }
}

void bst_print(volatile bst_node_t *node) {
  if (ISNULL(node))
    return;

  bst_print(node->left);
  printf("key: %d ", node->key);
  printf("address: %p ", node);
  printf("left: %p; right: %p; op: %p \n", node->left, node->right, node->op);
  bst_print(node->right);
}

unsigned long bst_size_rec(volatile bst_node_t *node) {
  if (ISNULL(node))
    return 0;
  else if (GETFLAG(node->op) != STATE_MARK)
    return 1 + bst_size_rec(node->right) + bst_size_rec(node->left);
  else
    return bst_size_rec(node->right) + bst_size_rec(node->left);
}

unsigned long bst_size(bst_node_t *root) {
  return bst_size_rec(root) - 1;
}
