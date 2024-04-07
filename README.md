# Binskip

A thorough comparison of a concurrent binary search tree and skip list.

## Instructions to Run

- **Prerequisites**:
    - GCC >= 13.2.1 (There are some `x86_64` machine specific instructions which might not work on Windows or Mac).
    - Python >= 3.11.8.
    - Jupyter Lab/Notebook.
    - Pandas.
    - Matplotlib.
    - scipy.
- To compile and build the binaries to run the experiments, execute these commands in the base directory of the repository:
```output
$ make clean
$ make
$ ./run_tests.sh <output_file_path>
```
- After running the `run_tests.sh` script, open the `data_analysis.ipynb` and set the `data_csv_name` in the second code block of the notebook to the name of the `<output_file_path>` provided to the `run_tests.sh` script.
- Furthermore, the `tests` binary generated in the `tests` directory can be used to run experiments individually. Usage instructions:
```output
$ ./tests --help
Usage: tests [OPTION...]
  -l, --sl                          Run the test for the skip list data structure
  -t, --num-threads=NUM_THREADS     Number of threads to use
  -i, --num-ops=NUM_OPERATIONS      Total number of operations to perform in each thread
  -k, --key-range=RANGE             Max value of keys. Maximum allowed=INT_MAX
  -g, --get-percent=PERCENT         Percentage of get operations
  -a, --add-percent=PERCENT         Percentage of add operations
  -r, --remove-percent=PERCENT      Percentage of remove operations
  -c, --csv=FILEPATH                Generate comma separated output values to the given file in append mode Format will be: data_structure, num_threads, num_ops, key_max_value, pct_get, pct_add, pct_remove,
                                    throughput(ops/millisecond), memory_utilization.

Help options:
  -?, --help                        Show this help message
      --usage                       Display brief usage message
```
- An example of use the `tests` binary:
```output
$ /tests --num-threads 6 --num-ops 1000000 -k 2000 -g 90 -a 9 -r 1
Options chosen:
Testing data structure: binary search tree
No. of threads: 6
No. of operations: 1000000
Highest key value: 2000
Percentage of get operations: 90%
Percentage of add operations: 9%
Percentage of remove operations: 1%
Thread ID: 0 time_spend:4.2417 add_ops: 90373 get_ops: 899701 remove_ops: 9926 ops/sec: 235754.9214
Thread ID: 1 time_spend:4.2685 add_ops: 89795 get_ops: 900171 remove_ops: 10034 ops/sec: 234274.8841
Thread ID: 2 time_spend:4.2788 add_ops: 90131 get_ops: 899875 remove_ops: 9994 ops/sec: 233711.4785
Thread ID: 3 time_spend:4.2741 add_ops: 89952 get_ops: 900015 remove_ops: 10033 ops/sec: 233969.1367
Thread ID: 4 time_spend:4.2823 add_ops: 90194 get_ops: 899822 remove_ops: 9984 ops/sec: 233521.9880
Thread ID: 5 time_spend:4.2818 add_ops: 90152 get_ops: 899787 remove_ops: 10061 ops/sec: 233547.9483
Total time taken by all the threads: 25.6271
Total average time spent per thread: 4.2712
Total average throughput per thread: 234127.5290
Total throughput(operations/second): 7128983.6315
Binary search tree memory utilization: 128592 bytes
```

## Data Source

- The only data source required for this project is the generation of keys to perform operations on the binary search tree or skip list.
- The keys in this project are randomly generated using the C standard library's `rand()` function.
- Keys are integers generated from 0 to the maximum range provided by the user.
- Random key generating function as present in the test code:
```c
static int my_random(int low, int high) {
  double my_rand = rand() / (1.0 + RAND_MAX);
  int range = high - low + 1;

  return (my_rand * range) + low;
}
``` 
- The seed of the random function is set as `2024`.

## Project Structure

```bash
├── compile_commands.json
├── data_analysis.ipynb
├── final_test.csv
├── include
│   ├── bst.h
│   └── skiplist.h
├── lib
│   ├── bst.c
│   ├── Makefile
│   └── skiplist.c
├── Makefile
├── README.md
├── run_tests.sh
└── tests
    ├── Makefile
    ├── test.c
    └── test.h
```
- `include` contains the API for the binary search tree and skip list algorithms.
- `lib` contains the main implementation of the binary search tree and skip list algorithms.
- `tests` contains the program to run experiments to collect data.
- `data_analysis.ipynb` contains the data analysis and graph plotting code for this project.
- `run_tests.sh` is the test script to reproduce the results.
- `final_test.csv` contains the data presented in the project presentation.
