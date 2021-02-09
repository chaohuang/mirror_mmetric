#!/bin/bash

source config.sh

OUT=compare_pcqm_plane
$CMD compare --mode pcqm --radiusFactor 1.0 --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
	--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1

# no map, no color
OUT=compare_pcqm_sphere_qp8
$CMD compare --mode pcqm --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=compare_pcqm_basket_qp8
$CMD compare --mode pcqm --radiusFactor 1.0 \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp8_orig.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=36.7492" 1

OUT=compare_pcqm_basket_qp16
$CMD compare --mode pcqm --radiusFactor 1.0 \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=47.621" 1

OUT=compare_pcqm_basket_qp16_nomap
$CMD compare --mode pcqm \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj  \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=72.7664" 1
fi

# sequence dataset
if [ "$1" == "seq" ]; 
then
	# test sequence mode with self
	OUT=compare_pcqm_longdress_1K_seq
	$CMD sequence --firstFrame 1051 --lastFrame 1053 END \
		compare --mode pcqm  \
		--inputModelA /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMapA  /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.png \
		--inputModelB /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMapB  /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.png \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

fi