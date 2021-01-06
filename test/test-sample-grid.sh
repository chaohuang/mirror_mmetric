#!/bin/bash

source config.sh


OUT=sample_grid_plane_10_bilinear_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --bilinear > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_grid_plane_10_nearest_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_grid_plane_10_nearest_10_10
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_grid_plane_4_nearest_10_10
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 4 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# color per vertex
OUT=sample_grid_cpv_plane_10
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_grid_sphere_10
$CMD sample -i ${DATA}/sphere.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# test degenerate triangle
OUT=sample_grid_degenerate
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Skipped 1 triangles" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=sample_grid_basketball_player_00000001_256
$CMD sample -i ${EXTDATA}/basketball_player_00000001.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 256  > ${TMP}/${OUT}.txt 2>&1

OUT=sample_grid_basketball_player_00000001_qp8_orig_256	
$CMD sample -i ${EXTDATA}/basketball_player_00000001_qp8_orig.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 256  > ${TMP}/${OUT}.txt 2>&1
fi
