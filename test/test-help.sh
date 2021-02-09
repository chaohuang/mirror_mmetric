#!/bin/bash

source config.sh

$CMD            > ${TMP}/helpMain.txt 	 2>&1
cmpOsLog helpMain

$CMD sample     > ${TMP}/helpSample.txt	 2>&1
cmpOsLog helpSample

$CMD compare   > ${TMP}/helpCompare.txt 2>&1
cmpOsLog helpCompare

$CMD reindex   > ${TMP}/helpReindex.txt 2>&1
cmpOsLog helpReindex

$CMD quantize   > ${TMP}/helpQuantize.txt 2>&1
cmpOsLog helpQuantize

$CMD badcmd     > ${TMP}/helpBadCmd.txt 2>&1
cmpOsLog helpBadCmd
