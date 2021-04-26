#!/bin/bash

source config.sh

# unit tests
if [ "$1" == "" ] || [ "$1" == "ext" ]; then

for  q in 8 16
do
	# 1 - defaults
	OUT=quantize_plane_qp${q}_other_defaults
	echo $OUT
	$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp ${q} --outputVar ${TMP}/${OUT}_vars.txt \
		> ${TMP}/${OUT}.txt 2>&1
	cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

	# load result in memory
	source ${TMP}/${OUT}_vars.txt
	
	OUT2=dequantize_plane_qp${q}_other_defaults
	echo $OUT2
	$CMD dequantize -i ${TMP}/${OUT}.obj -o ${TMP}/${OUT2}.obj --qp ${q} --qt 12 \
		--minPos="${minPos}" --maxPos="${maxPos}" --minUv="${minUv}" --maxUv="${maxUv}" \
		> ${TMP}/${OUT2}.txt 2>&1
	cmp ${TMP}/${OUT2}.obj ${REFS}/${OUT2}.obj

	# 2 - pass through
	OUT=quantize_plane_qp${q}_qt0_qn0_qc0
	echo $OUT
	$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp ${q} --qt 0 --qn 0 --qc 0 --outputVar ${TMP}/${OUT}_vars.txt > ${TMP}/${OUT}.txt 2>&1
	cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

	# load result in memory
	source ${TMP}/${OUT}_vars.txt
	
	OUT2=dequantize_plane_qp${q}_qt0_qn0_qc0
	echo $OUT2
	$CMD dequantize -i ${TMP}/${OUT}.obj -o ${TMP}/${OUT2}.obj --qp ${q} --qt 0 \
		--minPos="${minPos}" --maxPos="${maxPos}" \
		> ${TMP}/${OUT2}.txt 2>&1
	cmp ${TMP}/${OUT2}.obj ${REFS}/${OUT2}.obj

	# 3 - all set
	OUT=quantize_plane_qp${q}_qt${q}_qn${q}_qc${q}
	echo $OUT
	$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp ${q} --qt ${q} --qn ${q} --qc ${q} --outputVar ${TMP}/${OUT}_vars.txt > ${TMP}/${OUT}.txt 2>&1
	cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj
	
	# load result in memory
	source ${TMP}/${OUT}_vars.txt
	
	OUT2=dequantize_plane_qp${q}_qt${q}_qn${q}_qc${q}
	echo $OUT2
	$CMD dequantize -i ${TMP}/${OUT}.obj -o ${TMP}/${OUT2}.obj \
		--qp ${q} --qt ${q} \
		--minPos="${minPos}" --maxPos="${maxPos}" --minUv="${minUv}" --maxUv="${maxUv}" \
		> ${TMP}/${OUT2}.txt 2>&1
	cmp ${TMP}/${OUT2}.obj ${REFS}/${OUT2}.obj
done

fi
	
# quantize basket auto per frame box, nofixed point
OUT=quantize_dequantize_basket_auto_extent
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT
	$CMD \
		sequence --firstFrame 1 --lastFrame 3 END \
		quantize --qp 16 --qt 0 --qc 0 --qn 0 --inputModel ${DATA}/basketball_player_0000000%1d.obj \
			--outputModel ${TMP}/${OUT}_0000000%1d_qp16.obj END \
		quantize --qp 0 --qt 16 --qc 0 --qn 0 --inputModel ${DATA}/basketball_player_0000000%1d.obj \
			--outputModel ${TMP}/${OUT}_0000000%1d_qt16.obj \
		> ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "Computing positions range" 3
fi

# quantize basket auto per frame box, nofixed point
OUT=quantize_dequantize_basket_forced_extent	
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then

	# load existing box extents in memory
	source ${DATA}/basketball_player_statistics_all.txt
	
	echo $OUT
	$CMD \
		sequence --firstFrame 1 --lastFrame 3 END \
		quantize --qp 16 --qt 0 --qc 0 --qn 0 --minPos="${globalMinPos}" --maxPos="${globalMaxPos}" \
			--dequantize --inputModel ${DATA}/basketball_player_00000003.obj \
			--outputModel ${TMP}/${OUT}_0000000%1d_qp16.obj END \
		quantize --qp 0 --qt 16 --qc 0 --qn 0 --dequantize --inputModel ${DATA}/basketball_player_00000003.obj \
			--outputModel ${TMP}/${OUT}_0000000%1d_qt16.obj \
		> ${TMP}/${OUT}.txt 2>&1
	fileHasString ${TMP}/${OUT}.txt "Using parameter positions range" 3
fi

OUT=quantize_dequantize_basket
if [ "$1" == "ext" ] || [ "$1" == "$OUT" ]; then
	echo $OUT

	# first compute the extents over the entire sequence
	$CMD \
		sequence --firstFrame 1 --lastFrame 3 END \
		analyse --inputModel ${DATA}/basketball_player_0000000%1d.obj \
			--outputVar ${TMP}/quantize_basket_globals.txt END \
		> ${TMP}/${OUT}.txt 2>&1

	# load result in memory
	source ${TMP}/quantize_basket_globals.txt

	# quantize/dequantize using the extents 
	for  qp in 8 12 16
	do
		$CMD \
			sequence --firstFrame 1 --lastFrame 3 END \
			quantize --qp ${qp} --qt ${qp} --qn ${qp} --inputModel=${DATA}/basketball_player_0000000%1d.obj \
				--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" --minUv="${globalMinUv}"  --maxUv="${globalMaxUv}" \
				--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}" --outputModel=${TMP}/${OUT}_%1d_qp${qp}.obj END \
			dequantize --qp ${qp} --qt ${qp} --qn ${qp}  --inputModel=${TMP}/${OUT}_%1d_qp${qp}.obj \
				--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" --minUv="${globalMinUv}"  --maxUv="${globalMaxUv}" \
				--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}" --outputModel=${TMP}/${OUT}_%1d_dq_qp${qp}.obj END \
			quantize --dequantize --qp ${qp} --qt ${qp} --qn ${qp} --inputModel=${DATA}/basketball_player_0000000%1d.obj \
				--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" --minUv="${globalMinUv}"  --maxUv="${globalMaxUv}" \
				--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}" --outputModel=${TMP}/${OUT}_%1d_qdq_qp${qp}.obj END \
			>> ${TMP}/${OUT}.txt 2>&1
		
		# compare quantize inner dequantize wrt dequantize command results (must match)
		cmp ${TMP}/${OUT}_1_dq_qp${qp}.obj ${TMP}/${OUT}_1_qdq_qp${qp}.obj
		cmp ${TMP}/${OUT}_2_dq_qp${qp}.obj ${TMP}/${OUT}_2_qdq_qp${qp}.obj
		cmp ${TMP}/${OUT}_3_dq_qp${qp}.obj ${TMP}/${OUT}_3_qdq_qp${qp}.obj
	done

fi

