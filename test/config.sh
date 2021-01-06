#!/bin/bash

# this file is meant to be used by source config.sh

UNAME=`uname`

# path to test input data
DATA=./data
# path to external data sets
EXTDATA=../../../metrics/in
# path to test output references
REFS=./refs
# make a tmp folder to store processing ouotputs
TMP=./tmp
mkdir -p $TMP

# the command to test
CMD=""
if [ "$(uname)" == "Linux" ]; 
then
	CMD=../mm
else
	CMD=../x64/Release/mm.exe
fi

# function to compare two Os specific logs
function cmpOsLog {
	if [ "$(uname)" == "Linux" ]; 
	then
		cmp ${REFS}/${1}_linux.txt ${TMP}/${1}.txt
	else
		cmp ${REFS}/${1}.txt ${TMP}/${1}.txt
	fi
}

# test if $1 file contains $3 times the $2 string
function fileHasString {
	if [ $(grep -c "$2" $1) -ne $3 ]; then 
		echo "Error: $1 expected grep return code = $3, got $?"
	fi
}
