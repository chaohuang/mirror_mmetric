#!/bin/bash

source config.sh

STATS=${TMP}/compare_pcqm.csv
# reset csv stats file
> ${STATS}

OUT=compare_pcqm_plane
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcqm --radiusFactor 1.0 --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
		--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1
fi

# no map, no color
OUT=compare_pcqm_sphere_qp8
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcqm --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1
fi

################
# extended tests

OUT=compare_pcqm_basket_self
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcqm --radiusFactor 1.0 \
		--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=92.0076712" 1
fi

OUT=compare_pcqm_basket_qp8_orig
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcqm --radiusFactor 1.0 \
		--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=37.5514087" 1
fi

OUT=compare_pcqm_basket_qp8_hole
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcqm --radiusFactor 1.0 \
		--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001_qp8_hole.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=37.2551472" 1
fi

OUT=compare_pcqm_basket_qp16_nomap
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcqm \
		--inputModelA ${DATA}/basketball_player_00000001.obj  \
		--inputModelB ${TMPDATA}/basketball_player_00000001_qp16.obj \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=68.2653741" 1
fi

# test sequence mode with self
OUT=compare_pcqm_basket_qp16_seq
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sequence --firstFrame 1 --lastFrame 3 END \
		compare --mode pcqm  \
		--inputModelA ${DATA}/basketball_player_0000000%1d.obj \
		--inputMapA  ${DATA}/basketball_player_0000000%1d.png \
		--inputModelB ${TMPDATA}/basketball_player_0000000%1d_qp16.obj \
		--inputMapB  ${DATA}/basketball_player_0000000%1d.png \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR Mean=67.9765308" 1
fi
