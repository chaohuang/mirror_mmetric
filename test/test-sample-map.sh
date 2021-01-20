#!/bin/bash

source config.sh

OUT=sample_map_plane_2_2
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode map  --hideProgress        > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_map_plane_qp8_2_2
$CMD sample -i ${DATA}/plane_qp8.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode map --hideProgress > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_map_plane_qp8_10_10
$CMD sample -i ${DATA}/plane_qp8.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode map --hideProgress > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# test invalid map
OUT=sample_map_cvp_plane
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode map --hideProgress > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Error" 1

# test degenerate triangle
OUT=sample_map_degenerate
$CMD sample -i ${DATA}/degenerate.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode map --hideProgress > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Skipped 1 triangles" 1

# external dataset
if [ "$1" == "ext" ]; 
then
$CMD sample -i ${EXTDATA}/basketball_player_00000001.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/sample_map_basketball_player_00000001.ply --mode map --hideProgress  > ${TMP}/sample_map_basketball_player_00000001.txt 2>&1
	
	
$CMD sample -i ${EXTDATA}/basketball_player_00000001_qp8_orig.obj -m ${EXTDATA}/basketball_player_00000001.png \
	-o ${TMP}/sample_map_basketball_player_00000001_qp8_orig.ply --mode map --hideProgress  > ${TMP}/sample_map_basketball_player_00000001_qp8_orig.txt 2>&1
fi

