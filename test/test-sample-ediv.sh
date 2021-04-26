#!/bin/bash

source config.sh

STATS=${TMP}/sample_ediv.csv
# reset csv stats file
> ${STATS}

OUT=sample_ediv_plane_res_10_bilinear_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode ediv --resolution 10 --hideProgress --bilinear --outputCsv ${STATS}  > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_plane_res_10_nearest_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode ediv --resolution 10 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_plane_res_10_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode ediv  --resolution 10 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_plane_irregular_10_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode ediv  --resolution 10 \
	--hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

OUT=sample_ediv_plane_irregular_20_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode ediv  --resolution 20 \
	--hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# color per vertex
OUT=sample_ediv_cpv_plane_res_10
echo $OUT
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode ediv --hideProgress  --resolution 10  --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_ediv_sphere_qp8_len_1
echo $OUT
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT}.ply --mode ediv --lengthThreshold 1 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_ediv_sphere_res_30
echo $OUT
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT}.ply --mode ediv --resolution 30 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt # no reference, file too large to store in git
	
# test degenerate triangle
OUT=sample_ediv_degenerate_res_10
echo $OUT
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode ediv --resolution 10 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1

# extended tests
if [ "$1" == "ext" ]; 
then
	#
	OUT=sample_ediv_basketball_player_00000001_len_2
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode ediv --lengthThreshold 2 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	#
	OUT=sample_ediv_basketball_player_00000001_auto_1M
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode ediv --hideProgress --nbSamplesMin 1000000 --nbSamplesMax 1001000 --maxIterations 5 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	#
	OUT=sample_ediv_basketball_player_00000001_auto_2M
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode ediv --hideProgress --nbSamplesMin 2000000 --nbSamplesMax 2001000 --maxIterations 5 --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	#
	OUT=sample_ediv_basketball_player_00000001_qp8_res_1024
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode ediv --resolution 1024 --hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	# compare the overall results
	echo "Compare overall result csv"
	diff -a ${TMP}/sample_ediv.csv ${REFS}/sample_ediv.csv

fi

# EOF