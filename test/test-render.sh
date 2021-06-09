#!/bin/bash

source config.sh

for renderer in sw gl12
do
	OUT=render_plane_defsize_${renderer}
	echo $OUT
	$CMD render --renderer ${renderer}_raster --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png \
		--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

	OUT=render_plane_1K_${renderer}
	echo $OUT
	$CMD render --renderer ${renderer}_raster --width=1024 --height=1024 --inputModel ${DATA}/plane.obj --inputMap ${DATA}/plane.png \
		--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

	# no map, no color
	OUT=render_sphere_4K_${renderer}
	echo $OUT
	$CMD render --renderer ${renderer}_raster --width=4096 --height=4096 --inputModel ${DATA}/sphere.obj \
		--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
	grep -iF "error" ${TMP}/${OUT}.txt
	fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

	# extended tests
	if [ "$1" == "ext" ]; 
	then

		OUT=render_basket_map_light_4K_${renderer}
		echo $OUT
		$CMD render \
			--renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="0.0 0.0 -1.0" \
			--enableCulling \
			--enableLighting --autoLightPosition --lightAutoDir="-1.0 1.0 -1.0" \
			--inputModel ${DATA}/basketball_player_00000001.obj \
			--inputMap  ${DATA}/basketball_player_00000001.png \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

		OUT=render_basket_map_ccw_4K_${renderer}
		echo $OUT
		$CMD render \
			--renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="0.0 0.0 -1.0" \
			--enableCulling --cwCulling=false\
			--inputModel ${DATA}/basketball_player_00000001.obj \
			--inputMap  ${DATA}/basketball_player_00000001.png \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

		OUT=render_basket_map_4K_box_${renderer}
		echo $OUT
		$CMD render --renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="1.0 0.0 0.0" \
			--enableCulling \
			--bboxMin="-150 1220 0" --bboxMax="150 1300 40" \
			--inputModel ${DATA}/basketball_player_00000001.obj \
			--inputMap  ${DATA}/basketball_player_00000001.png \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

		OUT=render_basket_map_4K_back_${renderer}
		echo $OUT
		$CMD render --renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="0.0 0.0 1.0" \
			--enableCulling \
			--inputModel ${DATA}/basketball_player_00000001.obj \
			--inputMap  ${DATA}/basketball_player_00000001.png \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

		OUT=render_basket_map_4K_left_${renderer}
		echo $OUT
		$CMD render --renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="1.0 0.0 0.0" \
			--enableCulling \
			--inputModel ${DATA}/basketball_player_00000001.obj \
			--inputMap  ${DATA}/basketball_player_00000001.png \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1

		OUT=render_basket_cpv_4K_${renderer}
		echo $OUT
		$CMD render --renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="0.0 0.0 -1.0" \
			--enableCulling \
			--inputModel ${DATA}/cpv_basketball_player_00000001.ply \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1
		
		OUT=render_basket_red_4K_${renderer}
		echo $OUT
		$CMD render --renderer ${renderer}_raster --width=4096 --height=4096 --viewDir="0.0 0.0 -1.0" \
			--enableCulling \
			--inputModel ${DATA}/basketball_player_00000001.obj \
			--outputImage ${TMP}/${OUT}.png --outputDepth ${TMP}/${OUT}-depth.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 1
		
		# test sequence mode
		OUT=render_basket_${renderer}
		echo $OUT
		mkdir -p ${TMP}/render_basket_${renderer}
		$CMD sequence --firstFrame 1 --lastFrame 3 END \
			render --renderer ${renderer}_raster --width=1024 --height=1024 --viewDir="0.0 0.0 -1.0" \
			--inputModel ${DATA}/basketball_player_0000%04d.obj \
			--inputMap ${DATA}/basketball_player_0000%04d.png \
			--outputImage ${TMP}/render_basket_${renderer}/basketball_player_0000%4d.png > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "Render ${renderer}_raster" 3
	fi
done
