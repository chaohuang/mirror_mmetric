#!/bin/bash

source config.sh

OUT=render_plane_defsize
$CMD render --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png --outputImage ${TMP}/${OUT}.png > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

OUT=render_plane_1K
$CMD render --width=1024 --height=1024 --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png --outputImage ${TMP}/${OUT}.png > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# no map, no color
OUT=render_sphere_4K
$CMD render --width=4096 --height=4096 --inputModel ${DATA}/sphere.obj --outputImage ${TMP}/${OUT}.png > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=compare_pcc_basket_qp8_4K
$CMD render --width=4096 --height=4096 \
	--inputModel ${EXTDATA}/basketball_player_00000001.obj \
	--inputMap  ${EXTDATA}/basketball_player_00000001.png \
	--outputImage ${TMP}/${OUT}.png > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

OUT=compare_pcc_basket_qp16_4K
$CMD render --width=4096 --height=4096 \
	--inputModel ${EXTDATA}/basketball_player_00000001.obj \
	--inputMap  ${EXTDATA}/basketball_player_00000001.png \
	--outputImage ${TMP}/${OUT}.png > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
	
OUT=compare_pcc_basket_qp16_nomap_4K
$CMD render --width=4096 --height=4096 \
	--inputModel ${EXTDATA}/basketball_player_00000001.obj \
	--outputImage ${TMP}/${OUT}.png > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

fi
