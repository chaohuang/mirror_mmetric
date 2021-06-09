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
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 29.6178983" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 29.6169116" 1
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
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 27.2497506" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 27.5957658" 1
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
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 27.2320758" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 27.5849626" 1
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
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR = 51.8963922" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR = 51.8963921" 1
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
			fileHasString ${TMP}/${OUT}.txt "RGB PSNR Mean=51.3068012" 1
			fileHasString ${TMP}/${OUT}.txt "GEO PSNR Mean=51.4473063" 1
		fi
	fi
done

# EOF