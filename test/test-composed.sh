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

OUT=composed_sample_face_compare_sphere_pcqm
$CMD compare --mode pcqm \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1

# same without using intermediate files
OUT=composed_inmem_sample_face_sphere
$CMD \
sample -i ${DATA}/sphere.obj -o ID:pc1 --mode face --resolution 50 --hideProgress END \
sample -i ${DATA}/sphere_qp8.obj -o ID:pc2 --mode face --resolution 10 --hideProgress END \
compare --mode pcc --inputModelA ID:pc1 --inputModelB ID:pc2 \
> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 59.786" 1

# external dataset
if [ "$1" == "ext" ]; 
then

	OUT=composed_inmem_sample_map_compare_basketball
	$CMD \
	sample -i ${EXTDATA}/basketball_player_00000001.obj -m ${EXTDATA}/basketball_player_00000001.png -o ID:pc1 --mode map --hideProgress END \
	sample -i ${DATA}/basketball_player_00000001_qp8_orig.obj -m ${EXTDATA}/basketball_player_00000001.png -o ID:pc2 --mode map --hideProgress END \
	compare --mode pcc --inputModelA ID:pc1 --inputModelB ID:pc2 END \
	compare --mode pcqm --inputModelA ID:pc1 --inputModelB ID:pc2 END \
	> ${TMP}/${OUT}.txt 2>&1

	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6801" 1	
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=36.4337" 1

fi
