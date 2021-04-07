#!/bin/bash

source config.sh

# external dataset
if [ "$1" == "ext" ]; 
then
	# test single frame
	OUT=analyse_longdress_1K_1frame
	$CMD analyse --outputCsv ${TMP}/${OUT}.csv \
		--inputModel ${EXTDATA}/longdress_vox10_1051_poisson40k_uv_map.obj \
		--inputMap ${EXTDATA}/longdress_vox10_1051_poisson40k_uv_map.png END \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	# test sequence mode
	OUT=analyse_longdress_1K_3frames
	$CMD sequence --firstFrame 1051 --lastFrame 1054 END \
		analyse --outputCsv ${TMP}/${OUT}.csv \
		--inputModel ${EXTDATA}/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMap ${EXTDATA}/longdress_vox10_%4d_poisson40k_uv_map.png END \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	# test sequence mode
	OUT=basketball_player_3frames
	$CMD sequence --firstFrame 1 --lastFrame 3 END \
		analyse --outputCsv ${TMP}/${OUT}.csv --outputVar ${TMP}/${OUT}_var.txt \
		--inputModel ${EXTDATA}/basketball_player_0000000%1d.obj \
		--inputMap ${EXTDATA}/basketball_player_0000000%1d.png END \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi
