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

	for model in basketball_player_00000001 longdress_vox10_1051_poisson40k_uv_map longdress_vox10_1051_uv
	do
		OUT=reindex_${model}
		$CMD \
		reindex --sort none       -i ${EXTDATA}/${model}.obj -o ${TMP}/reindex_${model}_none.obj END \
		reindex --sort vertex     -i ${EXTDATA}/${model}.obj -o ${TMP}/reindex_${model}_vertex.obj END \
		reindex --sort oriented   -i ${EXTDATA}/${model}.obj -o ${TMP}/reindex_${model}_oriented.obj END \
		reindex --sort unoriented -i ${EXTDATA}/${model}.obj -o ${TMP}/reindex_${model}_unoriented.obj END \
		compare --mode equ --inputModelA ${EXTDATA}/${model}.obj --inputModelB ${TMP}/reindex_${model}_none.obj END \
		compare --mode equ --inputModelA ${EXTDATA}/${model}.obj --inputModelB ${TMP}/reindex_${model}_vertex.obj END \
		compare --mode equ --inputModelA ${EXTDATA}/${model}.obj --inputModelB ${TMP}/reindex_${model}_oriented.obj END \
		compare --mode equ --inputModelA ${EXTDATA}/${model}.obj --inputModelB ${TMP}/reindex_${model}_unoriented.obj END \
		> ${TMP}/${OUT}.txt 2>&1
		fileHasString ${TMP}/${OUT}.txt "meshes are equal" 3
		fileHasString ${TMP}/${OUT}.txt "meshes are not equal" 1
	done
fi
