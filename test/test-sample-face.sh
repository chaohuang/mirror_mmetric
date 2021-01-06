#!/bin/bash

source config.sh

OUT=sample_face_plane_10_bilinear_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 --bilinear  > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_face_plane_10_nearest_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_face_plane_10_nearest_10_10
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# color per vertex
OUT=sample_face_cpv_plane_10
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10  > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_face_sphere_10
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_face_sphere_qp8_10
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT}.txt 2>&1
# no ref yet (takes some space in the git)
# cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# test degenerate triangle
OUT=sample_face_degenerate
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Skipped 1 triangles" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=sample_face_basketball_player_00000001_512
$CMD sample -i ${EXTDATA}/basketball_player_00000001.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 512  > ${TMP}/${OUT}.txt 2>&1

OUT=sample_face_basketball_player_00000001_qp8_orig_1024
$CMD sample -i ${EXTDATA}/basketball_player_00000001_qp8_orig.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 1024 > ${TMP}/${OUT}.txt 2>&1
fi
