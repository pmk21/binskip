#include <assert.h>
#include <bits/pthreadtypes.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <popt.h>
#include <pthread.h>
#include <time.h>

#include "bst.h"
#include "skiplist.h"
#include "test.h"

int rand_seed = 0;

/* Util functions */
static int double_compare(const void *e1, const void *e2);

static void seed_rand(int seed) {
  if (!rand_seed) {
    srand((unsigned int)seed);
    srand48((long)seed);
  }
}

int main(int argc, const char *argv[]) {
  char c;
  int num_threads;
  test_options_t test_options = { 0, 2, 0, 0, 50, 50, 0 };
  poptContext opt_con;
  /* Options that we need to accept:
   * 1. Which data structure to test? sl or bst 
   * 2. Number of threads
   * 3. Number of operations 
   * 4. Range of randomly generated keys 
   * 5. Percent of get operations
   * 6. Percent of add operations
   * 7. Percent of remove operations
   */
  struct poptOption options_table[] = {
    { "sl", 'l', POPT_ARG_NONE, &test_options.test_skip_list, 0,
      "Run the test for the skip list data structure", NULL },
    { "num-threads", 't', POPT_ARG_INT, &test_options.num_threads, 0,
      "Number of threads to use", "NUM_THREADS" },
    { "num-ops", 'i', POPT_ARG_INT, &test_options.total_num_ops, 0,
      "Total number of operations to perform in each thread", "NUM_OPERATIONS" },
    { "key-range", 'k', POPT_ARG_INT, &test_options.key_range, 0,
      "Max value of keys. Maximum allowed=INT_MAX", "RANGE" },
    { "get-percent", 'g', POPT_ARG_INT, &test_options.pct_get_ops, 0,
      "Percentage of get operations", "PERCENT" },
    { "add-percent", 'a', POPT_ARG_INT, &test_options.pct_add_ops, 0,
      "Percentage of add operations", "PERCENT" },
    { "remove-percent", 'r', POPT_ARG_INT, &test_options.pct_remove_ops, 0,
      "Percentage of remove operations", "PERCENT" },
    { "csv", 'c', POPT_ARG_STRING, &test_options.out_csv, 0,
      "Generate comma separated output values to the given file in append mode"
      " Format will be: data_structure, num_threads, num_ops, key_max_value,"
      " pct_get, pct_add, pct_remove, throughput, memory_utilization.", "FILEPATH" },
    POPT_AUTOHELP
    POPT_TABLEEND
  };

  opt_con = poptGetContext(NULL, argc, argv, options_table, 0);
  if (argc < 2) {
    poptPrintUsage(opt_con, stderr, 0);
    exit(1);
  }
  /* Perform options parsing */
  while ((c = poptGetNextOpt(opt_con)) >= 0) {
    switch (c) {
      case 'l':
        break;
      case 't':
        break;
      case 'i':
        break;
      case 'k':
        break;
      case 'g':
        break;
      case 'a':
        break;
      case 'r':
        break;
      default:
        poptPrintUsage(opt_con, stderr, 0);
        break;
    }
  }
  if (c < -1) {
    /* Error occured during options processing */
    fprintf(stderr, "%s: %s\n", poptBadOption(opt_con, POPT_BADOPTION_NOALIAS), poptStrerror(c));
    return 1;
  }
  if ((test_options.pct_get_ops + test_options.pct_add_ops + test_options.pct_remove_ops) != 100) {
    printf("Summation of operations percentages is not 100! Setting default values.\n");
  }
  num_threads = test_options.num_threads;

  printf("Options chosen:\n");
  printf("Testing data structure: %s\n", test_options.test_skip_list ? "skiplist" : "binary search tree");
  printf("No. of threads: %d\n", test_options.num_threads);
  printf("No. of operations: %d\n", test_options.total_num_ops);
  printf("Highest key value: %d\n", test_options.key_range);
  printf("Percentage of get operations: %d%%\n", test_options.pct_get_ops);
  printf("Percentage of add operations: %d%%\n", test_options.pct_add_ops);
  printf("Percentage of remove operations: %d%%\n", test_options.pct_remove_ops);

  seed_rand(2024);
  pthread_t threads[num_threads];
  int rc;
  void *status;
  thread_data_t *tds = malloc(num_threads * thread_data_s);
  int t;
  node_t *head = NULL;
  bst_node_t *root = NULL;

  if (test_options.test_skip_list) {
    head = skiplist_init();
    pskiplist_insert(head, INT_MIN, "test");
    pskiplist_insert(head, INT_MAX, "test");
  } else {
    root = bst_initialize();
    bst_add(root, INT_MIN + 1, "test");
    bst_add(root, INT_MAX, "test");
  }

  op_weight_t cdf_arr[] = {
    { test_options.pct_add_ops/100.0, ADD },
    { test_options.pct_get_ops/100.0, GET },
    { test_options.pct_remove_ops/100.0, REMOVE }
  };
  cdf_arr[1].weight += cdf_arr[0].weight;
  cdf_arr[2].weight += cdf_arr[1].weight;
  // qsort(cdf_arr, 3, op_weight_s, double_compare);
  for (t = 0; t < num_threads; t++) {
    tds[t].id = t;
    tds[t].num_ops = test_options.total_num_ops;
    tds[t].key_range = test_options.key_range;
    tds[t].pct_get_ops = test_options.pct_get_ops;
    tds[t].pct_add_ops = test_options.pct_add_ops;
    tds[t].pct_remove_ops = test_options.pct_remove_ops;
    tds[t].cdf_arr = cdf_arr;
    if (test_options.test_skip_list)
      tds[t].head_or_root = head;
    else
      tds[t].head_or_root = root;

    if (test_options.test_skip_list)
      rc = pthread_create(&threads[t], NULL, test_sl, &tds[t]);
    else
      rc = pthread_create(&threads[t], NULL, test_bst, &tds[t]);
    if (rc) {
      fprintf(stderr, "ERROR: pthread_create rc=%d\n", rc);
      exit(-1);
    }
  }

  for (t = 0; t < num_threads; t++) {
    rc = pthread_join(threads[t], &status);
    if (rc) {
      fprintf(stderr, "ERROR: pthread_join rc=%d\n", rc);
      exit(-1);
    }
  }
  /* After all threads have completed, get the total time taken for all threads and divide by
   * total number of operation completed by all of them.
   */
  double tot_time_spent = 0.0;
  for (t = 0; t < num_threads; t++) {
    tot_time_spent += tds[t].time_spent;
    printf("Thread ID: %d add_ops: %d get_ops: %d remove_ops: %d\n",
           tds[t].id, tds[t].num_op_add, tds[t].num_op_get, tds[t].num_op_remove);
  }
  printf("Total time taken by all the threads: %0.4f\n", tot_time_spent);
  printf("Total average throughput: %0.4f\n", (tot_time_spent)/num_threads);
  int size;
  size_t mem_size;
  if (test_options.test_skip_list) {
    size = pskiplist_size(head);
    mem_size = size * node_s;
    printf("Skiplist memory utilization: %lu bytes\n", mem_size);
  } else {
    size = bst_size(root);
    mem_size = size * bst_node_s;
    printf("Binary search tree memory utilization: %lu bytes\n", mem_size);
  }

  if (test_options.out_csv != NULL) {
    char *filepath = test_options.out_csv;
    FILE *fp;

    fp = fopen(filepath, "a");
    fprintf(fp, "%s,%d,%d,%d,%d,%d,%d,%0.4f,%lu\n",
            test_options.test_skip_list ? "sl" : "bst",
            num_threads,
            test_options.total_num_ops,
            test_options.key_range,
            test_options.pct_get_ops,
            test_options.pct_add_ops,
            test_options.pct_remove_ops,
            (tot_time_spent)/num_threads,
            mem_size);
    fclose(fp);
  }

  // test_insert_get();
  // test_parallel_insert_get();
  // test_bst_parallel_insert_get();
  free(tds);
  poptFreeContext(opt_con);
  return 0;
}

static int double_compare(const void *e1, const void *e2) {
  double x = ((op_weight_t *)e1)->weight;
  double y = ((op_weight_t *)e2)->weight;
  if (x < y)
    return -1;
  return x > y;
}

/* Function design referred from https://stackoverflow.com/questions/4463561/weighted-random-selection-from-array */
static ds_op_type_e get_rand_op_with_dist(op_weight_t *cdf_arr) {
  int i;
  double randd = drand48();

  for (i = 0; i < 3; i++) {
    if (randd <= cdf_arr[i].weight)
      return cdf_arr[i].op_type;
  }
  return cdf_arr[2].op_type;
}

static int my_random(int low, int high) {
  double my_rand = rand() / (1.0 + RAND_MAX);
  int range = high - low + 1;

  return (my_rand * range) + low;
}

void *test_sl(void *thread) {
  int i, key, range, num_op_add = 0, num_op_get = 0, num_op_remove = 0;
  ds_op_type_e op_type;
  clock_t begin, end;
  thread_data_t *td = (thread_data_t *)thread;
  node_t *head = (node_t *)td->head_or_root;
  op_weight_t *cdf_arr = td->cdf_arr;
  
  range = td->key_range;
  begin = clock();
  for (i = 0; i < td->num_ops; i++) {
    key = my_random(0, range);
    op_type = get_rand_op_with_dist(cdf_arr);
    switch (op_type) {
      case ADD:
        pskiplist_insert(head, key, TEST_VALUE);
        num_op_add++;
        break;
      case GET:
        pskiplist_get(head, key);
        num_op_get++;
        break;
      case REMOVE:
        pskiplist_remove(head, key);
        num_op_remove++;
        break;
      default:
        pskiplist_get(head, key);
        break;
    }
  }
  end = clock();

  td->clk_begin = begin;
  td->clk_end = end;
  td->time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  td->num_op_add = num_op_add;
  td->num_op_get = num_op_get;
  td->num_op_remove = num_op_remove;
  pthread_exit(NULL);
}

void *test_bst(void *thread) {
  int i, key, range, num_op_add = 0, num_op_get = 0, num_op_remove = 0;
  ds_op_type_e op_type;
  clock_t begin, end;
  thread_data_t *td = (thread_data_t *)thread;
  bst_node_t *root = (bst_node_t *)td->head_or_root;
  op_weight_t *cdf_arr = td->cdf_arr;
  
  range = td->key_range;
  begin = clock();
  for (i = 0; i < td->num_ops; i++) {
    key = my_random(0, range);
    op_type = get_rand_op_with_dist(cdf_arr);
    switch (op_type) {
      case ADD:
        bst_add(root, key, TEST_VALUE);
        num_op_add++;
        break;
      case GET:
        bst_contains(root, key);
        num_op_get++;
        break;
      case REMOVE:
        bst_remove(root, key);
        num_op_remove++;
        break;
      default:
        bst_contains(root, key);
        break;
    }
  }
  end = clock();

  td->clk_begin = begin;
  td->clk_end = end;
  td->time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  td->num_op_add = num_op_add;
  td->num_op_get = num_op_get;
  td->num_op_remove = num_op_remove;
  pthread_exit(NULL);
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
