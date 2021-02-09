#!/bin/bash

source config.sh

OUT=reindex_plane
$CMD \
reindex --sort none       -i ${DATA}/plane.obj -o ${TMP}/reindex_plane_none.obj END \
reindex --sort vertex     -i ${DATA}/plane.obj -o ${TMP}/reindex_plane_vertex.obj END \
reindex --sort oriented   -i ${DATA}/plane.obj -o ${TMP}/reindex_plane_oriented.obj END \
reindex --sort unoriented -i ${DATA}/plane.obj -o ${TMP}/reindex_plane_unoriented.obj END \
compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${TMP}/reindex_plane_none.obj END \
compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${TMP}/reindex_plane_vertex.obj END \
compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${TMP}/reindex_plane_oriented.obj END \
compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${TMP}/reindex_plane_unoriented.obj END \
> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 3
fileHasString ${TMP}/${OUT}.txt "meshes are not equal" 1

# external dataset
if [ "$1" == "ext" ]; 
then
OUT=reindex_basketball
$CMD \
reindex --sort none       -i ${EXTDATA}/basketball_player_00000001.obj -o ${TMP}/reindex_basket_none.obj END \
reindex --sort vertex     -i ${EXTDATA}/basketball_player_00000001.obj -o ${TMP}/reindex_basket_vertex.obj END \
reindex --sort oriented   -i ${EXTDATA}/basketball_player_00000001.obj -o ${TMP}/reindex_basket_oriented.obj END \
reindex --sort unoriented -i ${EXTDATA}/basketball_player_00000001.obj -o ${TMP}/reindex_basket_unoriented.obj END \
compare --mode equ --inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputModelB ${TMP}/reindex_basket_none.obj END \
compare --mode equ --inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputModelB ${TMP}/reindex_basket_vertex.obj END \
compare --mode equ --inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputModelB ${TMP}/reindex_basket_oriented.obj END \
compare --mode equ --inputModelA ${EXTDATA}/basketball_player_00000001.obj --inputModelB ${TMP}/reindex_basket_unoriented.obj END \
> ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 3
fileHasString ${TMP}/${OUT}.txt "meshes are not equal" 1

fi
