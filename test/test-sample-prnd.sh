#!/bin/bash

source config.sh

STATS=${TMP}/sample_prnd.csv
# reset csv stats file
> ${STATS}

OUT=sample_prnd_plane_50_bilinear_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --bilinear --nbSamples=50 \
	--outputCsv ${STATS}  > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_prnd_plane_50_nearest_2_2
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --bilinear=false --nbSamples=50 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_prnd_plane_50_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --bilinear=false --nbSamples=50 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_prnd_plane_irregular_50_nearest_10_10
echo $OUT
$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode prnd --bilinear=false --nbSamples=50 \
	--hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

# color per vertex
OUT=sample_prnd_cpv_plane_50
echo $OUT
$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --nbSamples=50 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

# no color no map
OUT=sample_prnd_sphere_50
echo $OUT
$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --nbSamples=50 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply

OUT=sample_prnd_sphere_qp8_50
echo $OUT
$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --nbSamples=50 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
# no ref (takes some space in the git)

# test degenerate triangle
OUT=sample_prnd_degenerate_50
echo $OUT
$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode prnd --hideProgress --nbSamples=50 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt
fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1

# extended tests
if [ "$1" == "ext" ]; 
then

	#
	OUT=sample_prnd_basketball_player_00000001_2M
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png ${DUMP} --mode prnd \
		--hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	#
	OUT=sample_prnd_basketball_player_00000001_qp8_2M
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png ${DUMP} --mode prnd \
		--hideProgress --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	# compare the overall results
	echo "Compare overall result csv"
	diff -a ${TMP}/sample_prnd.csv ${REFS}/sample_prnd.csv

fi
