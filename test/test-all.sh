#!/bin/bash

# you can either run this script from the current directory to run all tests,
# all the parameters provided to test.sh will be forwarded to subtests,
# or run tests individually
# in any case you must execute the test script from the test directory 

tests=(
"test-help"
"test-quantize"
"test-reindex"
"test-sample-face"
"test-sample-sdiv"
"test-sample-grid"
"test-sample-map"
"test-compare-eq" 
"test-compare-pcc"
"test-compare-pcqm"
"test-composed"
)

for test in ${tests[@]}; do
	echo $test $@
	./${test}.sh "$@"
done
