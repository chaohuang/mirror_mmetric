#!/bin/bash

source config.sh

OUT=degrade_plane
echo $OUT
$CMD degrade --mode delface --inputModel ${DATA}/plane.obj --outputModel ${TMP}/${OUT}.obj > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

OUT=degrade_sphere
echo $OUT
$CMD degrade --mode delface --nthFace 10 --inputModel ${DATA}/sphere.obj --outputModel ${TMP}/${OUT}.obj > ${TMP}/${OUT}.txt 2>&1
grep -iF "error" ${TMP}/${OUT}.txt

