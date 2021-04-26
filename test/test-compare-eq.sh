#!/bin/bash

source config.sh

# compare meshes

OUT=compare_equ_mesh_plane_self
echo $OUT
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1

# vertex 2 and 4 are swaped and some face indices are shifted
OUT=compare_equ_mesh_plane_shifted
echo $OUT
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane_shifted.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1

# vertex 2 and 4 are swaped and some faces indices enumerated in revert order, disable early return
OUT=compare_equ_mesh_plane_reorder
echo $OUT
$CMD compare --mode equ --earlyReturn=false --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane_reorder.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are not equal, 2 different triangles" 1

# same as before but do not consider faces as oriented in comparison
OUT=compare_equ_mesh_plane_reorder_unoriented
echo $OUT
$CMD compare --mode equ --unoriented --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane_reorder.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1

# meshes with different number of tringles.
OUT=compare_equ_mesh_plane_sphere
echo $OUT
$CMD compare --mode equ --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/sphere.obj > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "meshes are not equal" 1

# compare point clouds
OUT=compare_equ_pc_plane_self_eps_0
echo $OUT
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png --outputModel ID:sampled END \
	compare --mode equ --inputModelA ID:sampled --inputModelB ID:sampled \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are equals" 1

OUT=compare_equ_pc_plane_eps_0_01
echo $OUT
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png --outputModel ID:sampled END \
	compare --mode equ --epsilon 0.01 --inputModelA ID:sampled --inputModelB ID:sampled \
	--outputModelA ${TMP}/${OUT}_A.ply --outputModelB ${TMP}/${OUT}_B.ply > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are equals" 1

OUT=compare_equ_pc_plane_sphere_eps_0
echo $OUT
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --outputModel ID:plane END \
	sample --mode sdiv --inputModel ${DATA}/sphere.obj --outputModel ID:sphere END \
	compare --mode equ --inputModelA ID:plane --inputModelB ID:sphere > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are not equals" 1

OUT=compare_equ_pc_plane_sphere_eps_0_01
echo $OUT
$CMD sample --mode sdiv --inputModel ${DATA}/plane.obj --outputModel ID:plane END \
	sample --mode sdiv --inputModel ${DATA}/sphere.obj --outputModel ID:sphere END \
	compare --mode equ --epsilon 0.01 --inputModelA ID:plane --inputModelB ID:sphere > ${TMP}/${OUT}.txt 2>&1
fileHasString ${TMP}/${OUT}.txt "model vertices are not equals" 1

# extended tests
# TODO add some tests with permutations
if [ "$1" == "ext" ]; 
then

	OUT=compare_equ_basket_self
	echo $OUT
	$CMD compare --mode equ \
		--inputModelA ${DATA}/basketball_player_00000001.obj  \
		--inputModelB ${DATA}/basketball_player_00000001.obj > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1
	
	OUT=compare_equ_basket_yuv420
	echo $OUT
	$CMD  \
		compare --mode equ  \
		--inputModelA ${DATA}/basketball_player_fr0001_qp12_qt12_dequant.obj \
		--inputMapA ${DATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_fr0001_qp12_qt12.obj \
		--inputMapB  ${DATA}/basketball_player_2048x2048_yuv420p.yuv \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "texture maps are not equal" 1
	fileHasString ${TMP}/${OUT}.txt "meshes are not equal" 1

	OUT=compare_equ_basket_gbrp444
	echo $OUT
	$CMD  \
		compare --mode equ  \
		--inputModelA ${DATA}/basketball_player_fr0001_qp12_qt12_dequant.obj \
		--inputMapA ${DATA}/basketball_player_00000001.png \
		--inputModelB ${DATA}/basketball_player_fr0001_qp12_qt12_dequant.obj \
		--inputMapB  ${DATA}/basketball_player_2048x2048_gbr444p.rgb \
		 > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "texture maps are equal" 1
	fileHasString ${TMP}/${OUT}.txt "meshes are equal" 1
fi
