#!/bin/bash

source config.sh

OUT=quantize_plane_qp8
$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp 8      > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

OUT=quantize_plane_qp16
$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp 16    > ${TMP}/${OUT}.txt 2>&1
cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj
