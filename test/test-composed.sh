#!/bin/bash

source config.sh

OUT1=composed_sample_face_sphere
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT1}.ply --mode face --resolution 50 --hideProgress  > ${TMP}/${OUT1}.txt 2>&1

OUT2=composed_sample_face_sphere_qp8
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT2}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT2}.txt 2>&1

OUT=composed_sample_face_compare_sphere_pcc
$CMD compare --mode pcc \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 59.786" 1

# inf because PCQM needs color, shall use another metric
OUT=composed_sample_face_compare_sphere_pcqm
$CMD compare --mode pcqm \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1

# external dataset
if [ "$1" == "ext" ]; 
then

OUT1=composed_sample_map_basketball_player_00000001
$CMD sample -i ${EXTDATA}/basketball_player_00000001.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT1}.ply --mode map --hideProgress  > ${TMP}/${OUT1}.txt 2>&1

OUT2=composed_sample_map_basketball_player_00000001_qp8_orig	
$CMD sample -i ${EXTDATA}/basketball_player_00000001_qp8_orig.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT2}.ply --mode map --hideProgress  > ${TMP}/${OUT2}.txt 2>&1

OUT=composed_sample_map_compare_basketball_pcc
$CMD compare --mode pcc \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6801" 1

OUT=composed_sample_map_compare_basketball_pcqm
$CMD compare --mode pcqm \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=46.9943" 1

fi
