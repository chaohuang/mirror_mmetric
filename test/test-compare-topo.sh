#!/bin/bash

source config.sh

OUT=compare_topo_plane_reorder_near_lossless
if [ "$1" == "" ] || [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD  \
		compare --mode topo  \
		--inputModelA ${DATA}/plane.obj \
		--inputModelB ${DATA}/plane_reorder.obj \
		--faceMapFile ${DATA}/plane_reorder_topo_face.txt \
		--vertexMapFile ${DATA}/plane_reorder_topo_vert.txt \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Topologies are different: orientations are not preserved" 1
fi

OUT=compare_topo_plane_shifted_near_lossless
if [ "$1" == "" ] || [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD  \
		compare --mode topo  \
		--inputModelA ${DATA}/plane.obj \
		--inputModelB ${DATA}/plane_shifted.obj \
		--faceMapFile ${DATA}/plane_shifted_topo_face.txt \
		--vertexMapFile ${DATA}/plane_shifted_topo_vert.txt \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Topologies are matching." 1
fi

####
# external datasets

OUT=compare_topo_basket_near_lossless_tfan_qp8
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD  \
		compare --mode topo  \
		--inputModelA ${DATA}/basketball_player_fr0001_qp12_qt12_dequant.obj \
		--inputModelB ${DATA}/basketball_player_fr0001_tfan_qp8.obj \
		--faceMapFile ${DATA}/basketball_player_fr0001_tfan_topo_face_order.txt \
		--vertexMapFile ${DATA}/basketball_player_fr0001_tfan_topo_vert_order.txt \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Topologies are matching." 1
fi

OUT=compare_topo_basket_near_lossless_tfan_qp10
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD  \
		compare --mode topo  \
		--inputModelA ${DATA}/basketball_player_fr0001_qp12_qt12_dequant.obj \
		--inputModelB ${DATA}/basketball_player_fr0001_tfan_qp10.obj \
		--faceMapFile ${DATA}/basketball_player_fr0001_tfan_topo_face_order.txt \
		--vertexMapFile ${DATA}/basketball_player_fr0001_tfan_topo_vert_order.txt \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Topologies are matching." 1
fi

OUT=compare_topo_basket_near_lossless_tfan_qp12
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD  \
		compare --mode topo  \
		--inputModelA ${DATA}/basketball_player_fr0001_qp12_qt12_dequant.obj \
		--inputModelB ${DATA}/basketball_player_fr0001_tfan_qp12.obj \
		--faceMapFile ${DATA}/basketball_player_fr0001_tfan_topo_face_order.txt \
		--vertexMapFile ${DATA}/basketball_player_fr0001_tfan_topo_vert_order.txt \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Topologies are matching." 1
fi

# EOF