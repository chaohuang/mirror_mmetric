#!/bin/bash

# hardware renderer will not provide metric results matching exactly software results
# hence we only test software results stability as reference

source config.sh

STATS=${TMP}/compare_ibsm.csv
# reset csv stats file
> ${STATS}

for renderer in sw_raster gl12_raster
do

	# compare ib
	OUT=compare_ibsm_${renderer}_plane_self
	if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} --inputModelA ${DATA}/plane.obj --inputModelB ${DATA}/plane.obj \
			--inputMapA ${DATA}/plane.png --inputMapB ${DATA}/plane.png --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = inf" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = inf" 1
		fi
	fi

	# no map, no color
	OUT=compare_ibsm_${renderer}_sphere_qp8
	if [ "$1" == "" ] || [ "$1" == "ext" ] ||  [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} --inputModelA ${DATA}/sphere.obj --inputModelB ${DATA}/sphere_qp8.obj --outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 23.6637415" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 23.6637415" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 23.6624282" 1
		fi
	fi

	####
	# extended tests

	OUT=compare_ibsm_${renderer}_basket_self
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${DATA}/basketball_player_00000001.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = inf" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = inf" 1
		fi
	fi

	# internal reordering of model A shall lead to identical meshes
	OUT=compare_ibsm_${renderer}_basket_reordered
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD \
			reindex --sort oriented --inputModel ${DATA}/basketball_player_00000001.obj --outputModel ID:reordered END \
			compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ID:reordered --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = inf" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = inf" 1
		fi
	fi
	
	# internal reordering disabled shall lead to numerical differences
	OUT=compare_ibsm_${renderer}_basket_reordering_disabled
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD \
			reindex --sort oriented --inputModel ${DATA}/basketball_player_00000001.obj --outputModel ID:reordered END \
			compare --mode ibsm --ibsmDisableReordering --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ID:reordered --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 68.5539723" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 68.5669411" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 68.610331" 1
		fi
	fi
	
	OUT=compare_ibsm_${renderer}_basket_qp8_self
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = inf" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = inf" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp8
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${TMPDATA}/basketball_player_00000001_qp8.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 17.6131681" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 17.6163361" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 17.8969953" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp8_hole
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj --inputMapA  ${DATA}/basketball_player_00000001.png \
			--inputModelB ${DATA}/basketball_player_00000001_qp8_hole.obj --inputMapB  ${DATA}/basketball_player_00000001.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 17.5950707" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 17.5981576" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 17.8859371" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp16_nomap
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		echo $OUT
		$CMD compare --mode ibsm --ibsmRenderer ${renderer} \
			--inputModelA ${DATA}/basketball_player_00000001.obj  \
			--inputModelB ${TMPDATA}/basketball_player_00000001_qp16.obj \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 1
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 42.0976932" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR = 42.0976932" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 42.0790485" 1
		fi
	fi

	OUT=compare_ibsm_${renderer}_basket_qp16_seq
	if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
		# to dump the color buffers:
		#   uncomment next line to prepare output fodler
		#   mkdir -p ${TMP}/cmpIbsm
		#   And add the following line to the compare command
		#   --ibsmOutputPrefix ${TMP}/cmpIbsm/$OUT \
		echo $OUT
		$CMD sequence --firstFrame 1 --lastFrame 3 END \
			compare --mode ibsm --ibsmRenderer ${renderer}  \
			--inputModelA ${DATA}/basketball_player_0000000%1d.obj \
			--inputMapA  ${DATA}/basketball_player_0000000%1d.png \
			--inputModelB ${TMPDATA}/basketball_player_0000000%1d_qp16.obj \
			--inputMapB  ${DATA}/basketball_player_0000000%1d.png \
			--outputCsv ${STATS} > ${TMP}/${OUT}.txt 2>&1
		grep -iF "error" ${TMP}/${OUT}.txt
		fileHasString ${TMP}/${OUT}.txt "  ibsmRenderer = ${renderer}" 3
		if [ $renderer == "sw_raster" ]; then
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR Mean=41.5748371" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR Mean=41.6754508" 1
			fileHasString ${TMP}/${OUT}.txt "Y   PSNR Mean=41.5802332" 1
			fileHasString ${TMP}/${OUT}.txt "U   PSNR Mean=41.6908137" 1
			fileHasString ${TMP}/${OUT}.txt "V   PSNR Mean=41.6825611" 1
			fileHasString ${TMP}/${OUT}.txt "YUV PSNR Mean=41.5786822" 1
		fi
	fi
done

# EOF