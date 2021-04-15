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
	OUT=compare_pcqm_basket_qp8_orig
	$CMD compare --mode pcqm --radiusFactor 1.0 \
		--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001_qp8_orig.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=36.899058" 1

	OUT=compare_pcqm_basket_qp8_hole
	$CMD compare --mode pcqm --radiusFactor 1.0 \
		--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001_qp8_hole.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=36.8903" 1

	OUT=compare_pcqm_basket_qp16
	$CMD compare --mode pcqm --radiusFactor 1.0 \
		--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001_qp16.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=46.7802411" 1

	OUT=compare_pcqm_basket_qp16_nomap
	$CMD compare --mode pcqm \
		--inputModelA ${EXTDATA}/basketball_player_00000001.obj  \
		--inputModelB ${DATA}/basketball_player_00000001_qp16.obj  > ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=68.2679284" 1

	# test sequence mode with self 
	OUT=compare_pcqm_longdress_1K_seq
	$CMD sequence --firstFrame 1051 --lastFrame 1053 END \
		compare --mode pcqm  \
		--inputModelA ${EXTDATA}/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMapA  ${EXTDATA}/longdress_vox10_%4d_poisson40k_uv_map.png \
		--inputModelB ${EXTDATA}/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMapB  ${EXTDATA}/longdress_vox10_%4d_poisson40k_uv_map.png \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	OUT=compare_pcqm_longdress_near_lossless
	$CMD  \
		compare --mode pcqm  \
		--inputModelA ${EXTDATA}/longdress_fr1051.obj \
		--inputMapA  ${EXTDATA}/longdress_fr1051.png \
		--inputModelB ${EXTDATA}/longdress_fr1051_tfan.obj \
		--inputMapB  ${EXTDATA}/longdress_fr1051.png \
		--topologyFile ${EXTDATA}/longdress_fr1051_tfan_topo.txt \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi
