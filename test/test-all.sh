#!/bin/bash

# you can either run this script from the current directory to run all tests,
# all the parameters provided to test.sh will be forwarded to subtests,
# or run tests individually
# in any case you must execute the test script from the test directory 

tests=(
"test-help"
"test-analyse"
"test-compare-eq" 
"test-compare-topo"
"test-compare-pcc"
"test-compare-pcqm"
"test-composed"
"test-degrade"
"test-normals"
"test-quantize"
"test-reindex"
"test-sample-ediv"
"test-sample-face"
"test-sample-grid"
"test-sample-map"
"test-sample-sdiv"
)

for test in ${tests[@]}; do
	echo "** "$test $@
	./${test}.sh "$@"
done
