#!/bin/bash

source config.sh

STATS=${TMP}/sample_grid.csv
# reset csv stats file
> ${STATS}

OUT=sample_grid_plane_10_bilinear_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --bilinear --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_grid_plane_10_nearest_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_grid_plane_10_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_grid_plane_4_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 4 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# test irregular mesh
OUT=sample_grid_plane_irregular_10_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

OUT=sample_grid_plane_irregular_20_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 20 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# color per vertex
OUT=sample_grid_cpv_plane_10
echo $OUT
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_grid_sphere_10
echo $OUT
$CMD sample -i ${DATA}/sphere.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# test degenerate triangle
OUT=sample_grid_degenerate
echo $OUT
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode grid --hideProgress --gridSize 10 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1

# extended tests
if [ "$1" == "ext" ]; 
then
	OUT=sample_grid_basketball_player_00000001_auto_1M
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode grid --hideProgress --nbSamplesMin 1000000 --nbSamplesMax 1001000 --maxIterations 5 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	#
	OUT=sample_grid_basketball_player_00000001_auto_2M
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode grid --hideProgress --nbSamplesMin 2000000 --nbSamplesMax 2001000 --maxIterations 5 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	#
	OUT=sample_grid_basketball_player_00000001_qp8_1024
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode grid --hideProgress --gridSize 1024  --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	# compare the overall results
	echo "Compare overall result csv"
	diff -a ${TMP}/sample_grid.csv ${REFS}/sample_grid.csv

fi
