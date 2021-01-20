#!/bin/bash

source config.sh

$CMD            > ${TMP}/helpMain.txt 	 2>&1
cmpOsLog helpMain

$CMD sample     > ${TMP}/helpSample.txt	 2>&1
cmpOsLog helpSample

$CMD render     > ${TMP}/helpRender.txt	 2>&1
cmpOsLog helpRender

$CMD compare   > ${TMP}/helpCompare.txt 2>&1
cmpOsLog helpCompare

$CMD quantize   > ${TMP}/helpQuantize.txt 2>&1
cmpOsLog helpQuantize

$CMD badcmd     > ${TMP}/helpBadCmd.txt 2>&1
cmpOsLog helpBadCmd
