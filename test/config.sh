#!/bin/bash

# this file is meant to be used by source config.sh

UNAME=`uname`

# path to test input data
DATA=./data
# path to external data sets
EXTDATA=./extdata
# path to test output references
REFS=./refs
# make a tmp folder to store processing outputs
TMP=./tmp
mkdir -p $TMP

# the command to test
CMD=""
if [ "$(uname)" == "Linux" ]; 
then
	CMD=../bin/mm
else
	CMD=../bin/mm.exe
fi

# function to compare two Os specific logs
function cmpOsLog {
	if [ "$(uname)" == "Linux" ]; 
	then
		# first apply sed to change command name
		# '-' in diff part means use std input (from sed)
		sed 's/mm.exe/mm/' ${REFS}/${1}.txt | diff -a - ${TMP}/${1}.txt
	else
		diff -a ${REFS}/${1}.txt ${TMP}/${1}.txt
	fi
}

# test if $1 file contains $3 times the $2 string
function fileHasString {
	if [ $(grep -c "$2" $1) -ne $3 ]; then 
		echo "Error: expected $3 time(s) the string \"$2\" in file $1"
	fi
}
