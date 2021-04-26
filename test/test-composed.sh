#!/bin/bash

# attention: using only 1 digit after coma for PCC PSNR results comparison due to imprecisions in linux/windows

source config.sh

OUT1=composed_sample_face_sphere
echo $OUT1
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT1}.ply --mode face --resolution 50 --hideProgress  > ${TMP}/${OUT1}.txt 2>&1
grep -iF "error" ${TMP}/${OUT1}.txt

OUT2=composed_sample_face_sphere_qp8
echo $OUT2
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT2}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT2}.txt 2>&1
grep -iF "error" ${TMP}/${OUT2}.txt

OUT=composed_sample_face_compare_sphere_pcc
echo $OUT
$CMD compare --mode pcc \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 59.7" 1

OUT=composed_sample_face_compare_sphere_pcqm
echo $OUT
$CMD compare --mode pcqm \
	--inputModelA ${TMP}/${OUT1}.ply \
	--inputModelB ${TMP}/${OUT2}.ply \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=inf" 1

# same without using intermediate files
OUT=composed_inmem_sample_face_sphere
echo $OUT
$CMD \
	sample -i ${DATA}/sphere.obj -o ID:pc1 --mode face --resolution 50 --hideProgress END \
	sample -i ${DATA}/sphere_qp8.obj -o ID:pc2 --mode face --resolution 10 --hideProgress END \
	compare --mode pcc --inputModelA ID:pc1 --inputModelB ID:pc2 \
	> ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 59.7" 1

# external dataset
if [ "$1" == "ext" ]; 
then

	OUT=composed_inmem_sample_map_compare_basketball
	echo $OUT
	$CMD \
	sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png -o ID:pc1 --mode map --hideProgress END \
	sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png -o ID:pc2 --mode map --hideProgress END \
	compare --mode pcc --inputModelA ID:pc1 --inputModelB ID:pc2 END \
	compare --mode pcqm --inputModelA ID:pc1 --inputModelB ID:pc2 END \
	> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "mseF,PSNR (p2plane): 67.6" 1	
	fileHasString ${TMP}/${OUT}.txt "PCQM-PSNR=36.6059236" 1

fi
