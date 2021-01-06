#!/bin/bash

source config.sh

# compare pcc

OUT=compare_pcc_plane
$CMD compare --mode pcc --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
	--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1

# no map, no color
OUT=compare_pcc_sphere_qp8
$CMD compare --mode pcc --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 68.7086" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=compare_pcc_basket_qp8
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp8_orig.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6879" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 32.0255" 1

OUT=compare_pcc_basket_qp16
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 107.072" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 39.6673" 1	
	
OUT=compare_pcc_basket_qp16_nomap
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj  \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 103.823" 1

fi
