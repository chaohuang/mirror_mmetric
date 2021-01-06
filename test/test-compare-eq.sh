#!/bin/bash

source config.sh

OUT=compare_plane_eps_0
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
if [ $? -ne 0 ]; then echo "Error: $OUT expected return code = 0, got $?"; fi

OUT=compare_equ_plane_eps_0_01
$CMD compare --mode equ --epsilon 0.01 --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
if [ $? -ne 0 ]; then echo "Error: $OUT expected return code = 0, got $?"; fi

OUT=compare_equ_plane_sphere_eps_0
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/sphere.obj \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
if [ $? -ne 1 ]; then echo "Error: $OUT expected return code = 1, got $?"; fi

OUT=compare_equ_plane_sphere_eps_0_01
$CMD compare --mode equ --epsilon 0.01 --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/sphere.obj \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
if [ $? -ne 1 ]; then echo "Error: $OUT expected return code = 1, got $?"; fi
