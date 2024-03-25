#ifndef TEST_H
#define TEST_H

#include <time.h>
typedef struct test_options {
  /* If set, run tests for skip list */
  int test_skip_list; 
  /* Number of threads to use for this test run */
  int num_threads; 
  int total_num_ops;
  int key_range;
  int pct_get_ops;
  int pct_add_ops;
  int pct_remove_ops;
} test_options_t;

typedef struct thread_data {
  /* Data provided to the thread */
  int id;
  int num_ops;
  int key_range;
  int pct_get_ops;
  int pct_add_ops;
  int pct_remove_ops;

  /* Data returned from the thread */
  clock_t clk_begin;
  clock_t clk_end;
  double time_spent;
} thread_data_t;

#define thread_data_s (sizeof(struct thread_data))

void test_insert_get(void);
void test_parallel_insert_get(void);
void test_bst_parallel_insert_get(void);

void *test_sl(void *);
void *test_bst(void *);

#endif
