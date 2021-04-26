#!/bin/bash

source config.sh

STATS=${TMP}/sample_sdiv.csv
# reset csv stats file
> ${STATS}

OUT=sample_sdiv_plane_1_bilinear_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode sdiv --hideProgress --bilinear --outputCsv ${STATS}  > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_sdiv_plane_1_nearest_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode sdiv --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_sdiv_plane_1_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode sdiv --mapThreshold --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_sdiv_plane_irregular_1_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode sdiv --mapThreshold \
	--hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# color per vertex
OUT=sample_sdiv_cpv_plane_1
echo $OUT
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode sdiv --hideProgress  --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_sdiv_sphere_1
echo $OUT
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT}.ply --mode sdiv --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_sdiv_sphere_qp8_1
echo $OUT
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT}.ply --mode sdiv --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
# no ref (takes some space in the git)

# test degenerate triangle
OUT=sample_sdiv_degenerate
echo $OUT
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode sdiv --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1

# extended tests
if [ "$1" == "ext" ]; 
then

	#
	OUT=sample_sdiv_basketball_player_00000001_2
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode sdiv --areaThreshold 2.0 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	#
	OUT=sample_sdiv_basketball_player_00000001_qp8_2
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode sdiv --areaThreshold 2.0 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	# compare the overall results
	echo "Compare overall result csv"
	diff -a ${TMP}/sample_sdiv.csv ${REFS}/sample_sdiv.csv

fi
