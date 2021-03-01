#!/bin/bash

source config.sh

# unit tests

for  q in 8 16
do
	# 1 - defaults
	OUT=quantize_plane_qp${q}_other_defaults
	$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp ${q} --outputVar ${TMP}/${OUT}_vars.txt \
		> ${TMP}/${OUT}.txt 2>&1
	cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

	# load result in memory
	source ${TMP}/${OUT}_vars.txt
	
	OUT2=dequantize_plane_qp${q}_other_defaults
	$CMD dequantize -i ${TMP}/${OUT}.obj -o ${TMP}/${OUT2}.obj --qp ${q} --qt 12 \
		--minPos="${minPos}" --maxPos="${maxPos}" --minUv="${minUv}" --maxUv="${maxUv}" \
		> ${TMP}/${OUT2}.txt 2>&1
	cmp ${TMP}/${OUT2}.obj ${REFS}/${OUT2}.obj

	# 2 - pass through
	OUT=quantize_plane_qp${q}_qt0_qn0_qc0
	$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp ${q} --qt 0 --qn 0 --qc 0 --outputVar ${TMP}/${OUT}_vars.txt > ${TMP}/${OUT}.txt 2>&1
	cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj

	# load result in memory
	source ${TMP}/${OUT}_vars.txt
	
	OUT2=dequantize_plane_qp${q}_qt0_qn0_qc0
	$CMD dequantize -i ${TMP}/${OUT}.obj -o ${TMP}/${OUT2}.obj --qp ${q} --qt 0 \
		--minPos="${minPos}" --maxPos="${maxPos}" \
		> ${TMP}/${OUT2}.txt 2>&1
	cmp ${TMP}/${OUT2}.obj ${REFS}/${OUT2}.obj

	# 3 - all set
	OUT=quantize_plane_qp${q}_qt${q}_qn${q}_qc${q}
	$CMD quantize -i ${DATA}/plane.obj -o ${TMP}/${OUT}.obj --qp ${q} --qt ${q} --qn ${q} --qc ${q} --outputVar ${TMP}/${OUT}_vars.txt > ${TMP}/${OUT}.txt 2>&1
	cmp ${TMP}/${OUT}.obj ${REFS}/${OUT}.obj
	
	# load result in memory
	source ${TMP}/${OUT}_vars.txt
	
	OUT2=dequantize_plane_qp${q}_qt${q}_qn${q}_qc${q}
	$CMD dequantize -i ${TMP}/${OUT}.obj -o ${TMP}/${OUT2}.obj \
		--qp ${q} --qt ${q} \
		--minPos="${minPos}" --maxPos="${maxPos}" --minUv="${minUv}" --maxUv="${maxUv}" \
		> ${TMP}/${OUT2}.txt 2>&1
	cmp ${TMP}/${OUT2}.obj ${REFS}/${OUT2}.obj
done

# external dataset with sequence
if [ "$1" == "ext" ]; 
then

	# first compute the extents over the entire sequence
	OUT=quantize_longdress_analyse
	$CMD \
		sequence --firstFrame 51 --lastFrame 54 END \
		analyse --inputModel ${EXTDATA}/longdress_vox10_1%03d_poisson40k_uv_map.obj \
			--outputVar ${TMP}/quantize_longdress_globals.txt END \
		> ${TMP}/${OUT}.txt 2>&1

	# load result in memory
	source ${TMP}/quantize_longdress_globals.txt

	# quantize/dequantize using the extents 
	for  qp in 8 12 16
	do
		OUT=quantize_longdress_qp${qp}
		$CMD \
			sequence --firstFrame 51 --lastFrame 54 END \
			quantize --qp ${qp} --qt ${qp} --qn ${qp} --inputModel=${EXTDATA}/longdress_vox10_1%03d_poisson40k_uv_map.obj \
				--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" --minUv="${globalMinUv}"  --maxUv="${globalMaxUv}" \
				--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}" --outputModel=${TMP}/${OUT}_1%03d_qp${qp}.obj END \
			dequantize --qp ${qp} --qt ${qp} --qn ${qp}  --inputModel=${TMP}/${OUT}_1%03d_qp${qp}.obj \
				--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" --minUv="${globalMinUv}"  --maxUv="${globalMaxUv}" \
				--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}" --outputModel=${TMP}/${OUT}_1%03d_dq_qp${qp}.obj END \
			quantize --dequantize --qp ${qp} --qt ${qp} --qn ${qp} --inputModel=${EXTDATA}/longdress_vox10_1%03d_poisson40k_uv_map.obj \
				--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" --minUv="${globalMinUv}"  --maxUv="${globalMaxUv}" \
				--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}" --outputModel=${TMP}/${OUT}_1%03d_qdq_qp${qp}.obj END \
			> ${TMP}/${OUT}.txt 2>&1
		
		# compare quantize inner dequantize wrt dequantize command results (must match)
		cmp ${TMP}/${OUT}_1051_dq_qp${qp}.obj ${TMP}/${OUT}_1051_qdq_qp${qp}.obj
		cmp ${TMP}/${OUT}_1052_dq_qp${qp}.obj ${TMP}/${OUT}_1052_qdq_qp${qp}.obj
		cmp ${TMP}/${OUT}_1053_dq_qp${qp}.obj ${TMP}/${OUT}_1053_qdq_qp${qp}.obj
		cmp ${TMP}/${OUT}_1054_dq_qp${qp}.obj ${TMP}/${OUT}_1054_qdq_qp${qp}.obj
	done

fi

