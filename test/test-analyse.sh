#!/bin/bash

source config.sh

# test single frame
OUT=analyse_basketball_player_1frame
echo $OUT
$CMD analyse --outputCsv ${TMP}/${OUT}.csv --outputVar ${TMP}/${OUT}_var.txt \
	--inputModel ${DATA}/basketball_player_00000001.obj \
	--inputMap ${DATA}/basketball_player_00000001.png END \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# test sequence mode
OUT=analyse_basketball_player_3frames
echo $OUT
$CMD sequence --firstFrame 1 --lastFrame 3 END \
	analyse --outputCsv ${TMP}/${OUT}.csv --outputVar ${TMP}/${OUT}_var.txt \
	--inputModel ${DATA}/basketball_player_0000000%1d.obj \
	--inputMap ${DATA}/basketball_player_0000000%1d.png END \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
