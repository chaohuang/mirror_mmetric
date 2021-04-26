#!/bin/bash

# attention: using only 1 digit after coma for PSNR results comparison due to imprecisions in linux/windows

source config.sh

# compare pcc
OUT=compare_pcc_plane
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
		--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
fi

# no map, no color
OUT=compare_pcc_sphere_qp8
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 66.4" 1
fi

####
# extended tests

OUT=compare_pcc_basket_self
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc \
		--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001.obj --inputMapB  ${DATA}/basketball_player_00000001.png > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
	fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : inf" 1
fi
	
OUT=compare_pcc_basket_qp8_self
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc \
		--inputModelA ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
		--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
	fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : inf" 1
fi

OUT=compare_pcc_basket_qp8_self_presampled
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc \
		--inputModelA ${TMP}/compare_pcc_basket_qp8_self_A.ply \
		--inputModelB ${TMP}/compare_pcc_basket_qp8_self_B.ply > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): inf" 1
	fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : inf" 1
fi

OUT=compare_pcc_basket_qp8
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc \
		--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6" 1
	fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 32.2" 1
fi

OUT=compare_pcc_basket_qp8_hole
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc \
		--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_00000001_qp8_hole.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6" 1
	fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF         : 31.9" 1
fi

OUT=compare_pcc_basket_qp16_nomap
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD compare --mode pcc \
		--inputModelA ${DATA}/basketball_player_00000001.obj  \
		--inputModelB ${TMPDATA}/basketball_player_00000001_qp16.obj  > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 115.3" 1
fi

OUT=compare_pcc_basket_qp16_seq
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sequence --firstFrame 1 --lastFrame 3 END \
		compare --mode pcc  \
		--inputModelA ${DATA}/basketball_player_0000000%1d.obj \
		--inputMapA  ${DATA}/basketball_player_0000000%1d.png \
		--inputModelB ${TMPDATA}/basketball_player_0000000%1d_qp16.obj \
		--inputMapB  ${DATA}/basketball_player_0000000%1d.png \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	# following p2plane line will work on Linux but not windows that leads to 115
	fileHasString ${TMP}/${OUT}.txt "mseF, PSNR(p2plane) Mean=114.97" 1
	fileHasString ${TMP}/${OUT}.txt "c\[0\],PSNRF          Mean=69.70" 1	
fi

# EOF