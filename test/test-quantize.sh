#!/bin/bash

source config.sh

OUT=quantize_plane_qp8
$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp 8      > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

OUT=quantize_plane_qp16
$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp 16    > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

# external dataset
if [ "$1" == "ext" ]; 
then

OUT=quantize_pcc_basket
$CMD \
	quantize --qp 8  --inputModel ${EXTDATA}/basketball_player_00000001.obj --outputModel  ${TMP}/${OUT}_qp8.obj  END\
	quantize --qp 12 --inputModel ${EXTDATA}/basketball_player_00000001.obj --outputModel  ${TMP}/${OUT}_qp12.obj END\
	quantize --qp 16 --inputModel ${EXTDATA}/basketball_player_00000001.obj --outputModel  ${TMP}/${OUT}_qp16.obj END\
	render --width=4096 --height=4096 \
	--inputModel ${TMP}/${OUT}_qp8.obj \
	--inputMap  ${EXTDATA}/basketball_player_00000001.png \
	--outputImage ${TMP}/${OUT}_qp8.png END \
	render --width=4096 --height=4096 \
	--inputModel ${TMP}/${OUT}_qp12.obj \
	--inputMap  ${EXTDATA}/basketball_player_00000001.png \
	--outputImage ${TMP}/${OUT}_qp12.png END \
	render --width=4096 --height=4096 \
	--inputModel ${TMP}/${OUT}_qp16.obj \
	--inputMap  ${EXTDATA}/basketball_player_00000001.png \
	--outputImage ${TMP}/${OUT}_qp16.png END \
	> ${TMP}/${OUT}.txt 2>&1

fi
