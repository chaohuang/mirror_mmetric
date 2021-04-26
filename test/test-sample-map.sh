#!/bin/bash

source config.sh

STATS=${TMP}/sample_map.csv
# reset csv stats file
> ${STATS}

OUT=sample_map_plane_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode map  --hideProgress --outputCsv ${STATS}  > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_map_plane_qp8_2_2
echo $OUT
$CMD sample -i ${DATA}/plane_qp8.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode map --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_map_plane_qp8_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_qp8.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode map --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# test irregular mesh
OUT=sample_map_plane_irregular_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode map --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# test invalid map
OUT=sample_map_cvp_plane
echo $OUT
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode map --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "Error" 1

# test degenerate triangle
OUT=sample_map_degenerate
echo $OUT
$CMD sample -i ${DATA}/degenerate.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode map --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1

# extended tests
if [ "$1" == "ext" ]; 
then
	
	#
	OUT=sample_map_basketball_player_00000001
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode map --hideProgress  \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	#
	OUT=sample_map_basketball_player_00000001_qp8
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode map --hideProgress \
		--outputCsv ${STATS}  > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	# compare the overall results
	diff -a ${TMP}/sample_map.csv ${REFS}/sample_map.csv

fi

