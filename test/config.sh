#!/bin/bash

# this file is meant to be used by 'source config.sh'

UNAME=`uname`

# path to test input data
DATA=./data
# path to temporary data sets
TMPDATA=./tmp/data
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

# prepare some test data if not exists
# requires the quantize command to work properly
if [ ! -d ${TMPDATA} ]; then

	mkdir ${TMPDATA}
	touch ${TMPDATA}/log.txt
	
	for  q in 8 10 16
	do
		# quantize plane
		$CMD \
			quantize --qp $q --qt 0 --qc 0 --qn 0 --dequantize --inputModel ${DATA}/plane.obj --outputModel ${TMPDATA}/plane_qp${q}.obj END \
			quantize --qp 0 --qt $q --qc 0 --qn 0 --dequantize --inputModel ${DATA}/plane.obj --outputModel ${TMPDATA}/plane_qt${q}.obj \
			>> ${TMPDATA}/log.txt 2>&1
		
		# quantize basket
		$CMD \
			sequence --firstFrame 1 --lastFrame 3 END \
			quantize --qp $q --qt 0 --qc 0 --qn 0 --dequantize --inputModel ${DATA}/basketball_player_0000000%1d.obj \
				--outputModel ${TMPDATA}/basketball_player_0000000%1d_qp${q}.obj END \
			quantize --qp 0 --qt $q --qc 0 --qn 0 --dequantize --inputModel ${DATA}/basketball_player_0000000%1d.obj \
				--outputModel ${TMPDATA}/basketball_player_0000000%1d_qt${q}.obj \
			>> ${TMPDATA}/log.txt 2>&1
	done
	
	grep -iF "error" ${TMPDATA}/log.txt
fi

# EOF