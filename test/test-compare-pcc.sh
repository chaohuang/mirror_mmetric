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
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 66.4296" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=compare_pcc_basket_self
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : inf" 1

OUT=compare_pcc_basket_qp8_self
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001_qp8_orig.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp8_orig.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 61.8476" 1

OUT=compare_pcc_basket_qp8_self_presampled
$CMD compare --mode pcc \
	--inputModelA ${TMP}/compare_pcc_basket_qp8_self_A.ply \
	--inputModelB ${TMP}/compare_pcc_basket_qp8_self_B.ply > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 61.8476" 1

OUT=compare_pcc_basket_qp8
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp8_orig.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6033" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 31.9359" 1

OUT=compare_pcc_basket_qp16_self
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001_qp16.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : inf" 1	

OUT=compare_pcc_basket_qp16
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputMapA  ${EXTDATA}/basketball_player_00000001.png \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj --inputMapB  ${EXTDATA}/basketball_player_00000001.png  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 115.398" 1
fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 38.1793" 1	

OUT=compare_pcc_basket_qp16_nomap
$CMD compare --mode pcc \
	--inputModelA ${EXTDATA}/basketball_player_00000001.obj  \
	--inputModelB ${EXTDATA}/basketball_player_00000001_qp16.obj  > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 115.398" 1

fi

# sequence dataset
if [ "$1" == "seq" ]; 
then
	# test sequence mode with self
	OUT=compare_pcc_longdress_1K_seq
	$CMD sequence --firstFrame 1051 --lastFrame 1053 END \
		compare --mode pcc  \
		--inputModelA /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMapA  /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.png \
		--inputModelB /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.obj \
		--inputMapB  /c/Users/marviej/Datasets/Reconstruct/8i/longdress/poisson40k_uv_map/longdress_vox10_%4d_poisson40k_uv_map.png \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

fi