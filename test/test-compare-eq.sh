#!/bin/bash

source config.sh

# compare meshes

OUT=compare_equ_mesh_plane_self
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1

# vertex 2 and 4 are swaped and some face indices are shifted
OUT=compare_equ_mesh_plane_shifted
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane_shifted.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1

# vertex 2 and 4 are swaped and some faces indices enumerated in revert order, disable early return
OUT=compare_equ_mesh_plane_reorder
$CMD compare --mode equ --earlyReturn=false --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane_reorder.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are not equal, 2 different triangles" 1

# same as before but do not consider faces as oriented in comparison
OUT=compare_equ_mesh_plane_reorder_unoriented
$CMD compare --mode equ --unoriented --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane_reorder.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1

# meshes with different number of tringles.
OUT=compare_equ_mesh_plane_sphere
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/sphere.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are not equal" 1

# compare point clouds
OUT=compare_equ_pc_plane_self_eps_0
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png --outputModel ID:sampled END \
	compare --mode equ --inputModelA ID:sampled --inputModelB ID:sampled \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are equals" 1

OUT=compare_equ_pc_plane_eps_0_01
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png --outputModel ID:sampled END \
	compare --mode equ --epsilon 0.01 --inputModelA ID:sampled --inputModelB ID:sampled \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are equals" 1

OUT=compare_equ_pc_plane_sphere_eps_0
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --outputModel ID:plane END \
	sample --mode sdiv --inputModel ${DATA}/sphere.obj --outputModel ID:sphere END \
	compare --mode equ --inputModelA ID:plane --inputModelB ID:sphere > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are not equals" 1

OUT=compare_equ_pc_plane_sphere_eps_0_01
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --outputModel ID:plane END \
	sample --mode sdiv --inputModel ${DATA}/sphere.obj --outputModel ID:sphere END \
	compare --mode equ --epsilon 0.01 --inputModelA ID:plane --inputModelB ID:sphere > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are not equals" 1

# external dataset
if [ "$1" == "ext" ]; 
then
	OUT=compare_equ_basket_self
	$CMD compare --mode equ \
		--inputModelA ${EXTDATA}/basketball_player_00000001.obj  \
		--inputModelB ${EXTDATA}/basketball_player_00000001.obj > ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1
	# TODO add some tests with permutations
		
	OUT=compare_eq_longdress_yuv420
	$CMD  \
		compare --mode equ  \
		--inputModelA ${EXTDATA}/longdress_fr1051.obj \
		--inputMapA  ${EXTDATA}/longdress_fr1051.png \
		--inputModelB ${EXTDATA}/longdress_fr1051.obj \
		--inputMapB  ${EXTDATA}/longdress_fr1051_2048x2048_yuv420p.yuv \
		 > ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "texture maps are not equal" 1
	grep -iF "error" ${TMP}/${OUT}.txt

	OUT=compare_eq_longdress_gbrp444
	$CMD  \
		compare --mode equ  \
		--inputModelA ${EXTDATA}/longdress_fr1051.obj \
		--inputMapA  ${EXTDATA}/longdress_fr1051.png \
		--inputModelB ${EXTDATA}/longdress_fr1051.obj \
		--inputMapB  ${EXTDATA}/longdress_fr1051_2048x2048_gbrp444.rgb \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "texture maps are equal" 1
fi
