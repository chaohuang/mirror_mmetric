#!/bin/bash

source config.sh

# external dataset
if [ "$1" == "ext" ]; 
then

	OUT=normals_basket
	$CMD normals \
		--inputModel ${EXTDATA}/basketball_player_00000001.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	OUT=normals_longdress
	$CMD normals \
		--inputModel ${EXTDATA}/longdress_vox10_1051_poisson40k_uv_map.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	OUT=normals_basket_noseams_off_normalized_off
	$CMD normals \
		--noSeams=false --normalized=false \
		--inputModel ${EXTDATA}/basketball_player_00000001.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	OUT=normals_longdress_noseams_off
	$CMD normals \
		--noSeams=false \
		--inputModel ${EXTDATA}/longdress_vox10_1051_poisson40k_uv_map.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
fi
