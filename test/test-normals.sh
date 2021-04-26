#!/bin/bash

source config.sh

# external dataset
if [ "$1" == "ext" ]; 
then

	OUT=normals_basket
	echo $OUT
	$CMD normals \
		--inputModel ${DATA}/basketball_player_00000001.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

	OUT=normals_basket_noseams_off
	echo $OUT
	$CMD normals \
		--noSeams=false \
		--inputModel ${DATA}/basketball_player_00000001.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	
	OUT=normals_basket_noseams_off_normalized_off
	echo $OUT
	$CMD normals \
		--noSeams=false --normalized=false \
		--inputModel ${DATA}/basketball_player_00000001.obj \
		--outputModel ${TMP}/${OUT}.obj \
		> ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt

fi
