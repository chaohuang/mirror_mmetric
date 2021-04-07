#!/bin/bash

source config.sh

OUT=sample_ediv_plane_res_10_bilinear_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode ediv --resolution 10 --hideProgress --bilinear  > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_plane_res_10_nearest_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode ediv --resolution 10 --hideProgress > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_plane_res_10_nearest_10_10
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode ediv  --resolution 10 --hideProgress > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# color per vertex
OUT=sample_ediv_cpv_plane_res_10
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode ediv --hideProgress  --resolution 10  > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_ediv_sphere_qp8_len_1
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT}.ply --mode ediv --lengthThreshold 1 --hideProgress > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_sphere_res_30
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT}.ply --mode ediv --resolution 30 --hideProgress > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt # no reference, file too large to store in git
	
# test degenerate triangle
OUT=sample_ediv_degenerate_res_10
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode ediv --resolution 10 --hideProgress > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=sample_ediv_basketball_player_00000001_len_2
$CMD sample -i ${EXTDATA}/basketball_player_00000001.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT}.ply --mode ediv --lengthThreshold 2 --hideProgress > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

OUT=sample_ediv_basketball_player_00000001_qp8_orig_res_1024
$CMD sample -i ${DATA}/basketball_player_00000001_qp8_orig.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT}.ply --mode ediv --resolution 1024 --hideProgress > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

fi

# EOF