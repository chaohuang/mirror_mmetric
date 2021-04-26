#!/bin/bash

source config.sh

STATS=${TMP}/sample_face.csv
# reset csv stats file
> ${STATS}

OUT=sample_face_plane
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD \
		sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/sample_face_plane_10_bilinear_2_2.ply \
			--mode face --hideProgress --resolution 10 --bilinear --outputCsv ${STATS} END \
		sample -i ${DATA}/plane.obj -m ${DATA}/plane.png -o ${TMP}/sample_face_plane_10_nearest_2_2.ply \
			--mode face --hideProgress --resolution 10 --outputCsv ${STATS} END \
		sample -i ${DATA}/plane.obj -m ${DATA}/plane_10_10.png -o ${TMP}/sample_face_plane_10_nearest_10_10.ply \
			--mode face --hideProgress --resolution 10 --outputCsv ${STATS} \
	> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	cmp ${TMP}/sample_face_plane_10_bilinear_2_2.ply ${REFS}/sample_face_plane_10_bilinear_2_2.ply
	cmp ${TMP}/sample_face_plane_10_nearest_2_2.ply ${REFS}/sample_face_plane_10_nearest_2_2.ply
	cmp ${TMP}/sample_face_plane_10_nearest_10_10.ply ${REFS}/sample_face_plane_10_nearest_10_10.ply
fi

# test irregular mesh
# generated files are refered to by the meshlab project located in meshlab subfolder
OUT=sample_face_plane_irregular_10_10_10
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi

OUT=sample_face_plane_irregular_20_10_10
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sample -i ${DATA}/plane_irregular.obj -m ${DATA}/plane_10_10.png -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 20 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi

# color per vertex
OUT=sample_face_cpv_plane_10
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sample -i ${DATA}/cpv_plane.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply
fi

# no color no map
OUT=sample_face_sphere_10
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sample -i ${DATA}/sphere_qp8.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	cmp ${TMP}/${OUT}.ply ${REFS}/${OUT}.ply
fi

OUT=sample_face_sphere_qp8_10
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sample -i ${DATA}/sphere.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	# no ref (takes some space in the git	
fi

# test degenerate triangle
OUT=sample_face_degenerate
if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD sample -i ${DATA}/degenerate.obj -o ${TMP}/${OUT}.ply --mode face --hideProgress --resolution 10 \
	--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Skipped 1 degenerate triangles" 1
fi

###
# extended tests

OUT=sample_face_basketball_player_00000001_auto_1M
if [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi	
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode face --hideProgress --nbSamplesMin 1000000 --nbSamplesMax 1001000 --maxIterations 5 \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi

OUT=sample_face_basketball_player_00000001_auto_2M
if [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${DATA}/basketball_player_00000001.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode face --hideProgress --nbSamplesMin 2000000 --nbSamplesMax 2001000 --maxIterations 5 \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi

OUT=sample_face_basketball_player_00000001_qp8_1024
if [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
	echo $OUT
	if [ "$2" == "dump" ]; then	
		DUMP="-o ${TMP}/${OUT}.ply"
	else
		DUMP="-o ID:NUL"
	fi
	$CMD sample -i ${TMPDATA}/basketball_player_00000001_qp8.obj -m ${DATA}/basketball_player_00000001.png \
		${DUMP} --mode face --hideProgress --resolution 1024 \
		--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi

# compare the overall results
if [ "$1" == "ext" ]; then
	echo "Compare overall result csv"
	diff -a ${TMP}/sample_face.csv ${REFS}/sample_face.csv
fi

