#!/bin/bash

source config.sh

OUT=compare_pcqm_plane
$CMD compare --mode pcqm --radiusFactor 1.0 --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
	--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1

# no map, no color
OUT=compare_pcqm_sphere_qp8
$CMD compare --mode pcqm --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=17.2273" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=compare_pcqm_basket_qp8
$CMD compare --mode pcqm --radiusFactor 1.0 \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp8_orig.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=0.0447924" 1

OUT=compare_pcqm_basket_qp16
$CMD compare --mode pcqm --radiusFactor 1.0 \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=45.7881" 1

OUT=compare_pcqm_basket_qp16_nomap
$CMD compare --mode pcqm \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj  \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=34.6799" 1
fi
