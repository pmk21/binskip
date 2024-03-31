#!/bin/bash

run_test_binary() {
    local is_sl=$1
    local num_threads="$2"
    local num_ops="$3"
    local key_range="$4"
    local pct_get_ops="$5"
    local pct_add_ops="$6"
    local pct_remove_ops="$7"
    local csv_out_file="$8"
    local num_repeats="$9"
    

    for (( i=1; i <= $num_repeats; ++i ))
    do
        echo "Starting run no. $i"
        echo ""
        if [ $is_sl = true ]; then
            timeout --foreground 5s ./tests/tests --sl --num-threads $num_threads --num-ops $num_ops -k $key_range -g $pct_get_ops -a $pct_add_ops -r $pct_remove_ops --csv $csv_out_file
            # Keep trying until the program successfully runs
            while [ $? -eq 124 ] 
            do
                timeout --foreground 5s ./tests/tests --sl --num-threads $num_threads --num-ops $num_ops -k $key_range -g $pct_get_ops -a $pct_add_ops -r $pct_remove_ops --csv $csv_out_file
            done
        else
            timeout --foreground 5s ./tests/tests --num-threads $num_threads --num-ops $num_ops -k $key_range -g $pct_get_ops -a $pct_add_ops -r $pct_remove_ops --csv $csv_out_file
        fi
        echo ""
    done
}

CSV_OUT_FILE="$1"

echo "Outputting data to $CSV_OUT_FILE"

# Run all the tests
KEY_RANGE="200000"
PCT_GET="0"
PCT_ADD="50"
PCT_REMOVE="50"

run_test_binary true 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5

PCT_GET="70"
PCT_ADD="20"
PCT_REMOVE="10"

run_test_binary true 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5

PCT_GET="90"
PCT_ADD="9"
PCT_REMOVE="1"

run_test_binary true 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5

KEY_RANGE="2000000"
PCT_GET="0"
PCT_ADD="50"
PCT_REMOVE="50"

run_test_binary true 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5

PCT_GET="70"
PCT_ADD="20"
PCT_REMOVE="10"

run_test_binary true 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5

PCT_GET="90"
PCT_ADD="9"
PCT_REMOVE="1"

run_test_binary true 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 2 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 4 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 6 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 8 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary true 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
run_test_binary false 10 1000000 $KEY_RANGE $PCT_GET $PCT_ADD $PCT_REMOVE $CSV_OUT_FILE 5
