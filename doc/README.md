# Compilation

Download and build dependencies with following command:
- ./build.sh deps

Build software with following command:
- ./build.sh [release|debug] [noomp] [nojobs]
- release|debug: sets build target (release is default)
- noomp: disables openmp build
- nojobs: disables multi-processor build on unix (i.e. use -j 1)
- output binary produced in bin folder.

Update readme with following command:
- ./build.sh doc
- output readme.md

# Usage examples

Note: 
- the system can read/write point clouds as obj or ply format.
- the system can read/write meshes as obj or ply format.
- once a file loaded the system detects a mesh by checking if any topology is avilable.

## Simple commands

Sample input.obj into output.ply using subdivision mode (sdiv).

```
mm.exe sample --mode sdiv -i input.obj --areaThreshold=1.0 --mapThreshold -o output.ply 
```

Compare textured meshB.obj mesh against meshA reference mesh using pcc_error. Will use default sampling method.

```
mm.exe compare --mode pcc --inputModelA inputA.obj --inputMapA mapA.png  --inputModelB inputB.obj --inputMapB mapB.png
```

## Commands combination

Following example uses specific grid sampling method, then compare using pcc_error and pcqm metrics in a single call.

```
mm.exe \
	sample --mode grid -i inputA.obj -m mapA.png -o ID:pcA END \
	sample --mode grid -i inputB.obj -m mapB.png -o ID:pcB END \
	compare --mode pcc  --inputModelA ID:pcA --inputModelB ID:pcB END \
	compare --mode pcqm --inputModelA ID:pcA --inputModelB ID:pcB
```

Same as previous but dumping intermediate sampling results into files. Compare still use the versions in memory.

```
mm.exe \
	sample --mode grid -i inputA.obj -m mapA.png -o pcA.ply END \
	sample --mode grid -i inputB.obj -m mapB.png -o pcB.ply END \
	compare --mode pcc  --inputModelA pcA.ply --inputModelB pcB.ply END \
	compare --mode pcqm --inputModelA pcA.ply --inputModelB pcB.ply END \
```

Several commands can be cascaded using this mechanism, for instance doing quantization then sampling then compare. 
Note however that memory won't be released between sub command calls so cascading many commands may be very consuming in terms of memory.

## Sequence processing

Following sample demonstrates how to execute commands on a numerated sequence of objects ranging from 00150 to 00165 included. 
The "%3d" part of the file names will be replaced by the frame number ranging from firstFrame to lastFrame, coded on 3 digits.

```
mm.exe \
	sequence --firstFrame 150 --lastFrame 165
	sample --mode grid -i inputA_00%3d.obj -m mapA_00%3d.png -o ID:pcA END \
	sample --mode grid -i inputB_00%3d.obj -m mapB_00%3d.png -o ID:pcB END \
	compare --mode pcc  --inputModelA ID:pcA --inputModelB ID:pcB END \
	compare --mode pcqm --inputModelA ID:pcA --inputModelB ID:pcB
```

The replacement mechanism can also be used on final or intermediate output file names as shown in the two following examples.

```
mm.exe \
	sequence --firstFrame 150 --lastFrame 165
	sample --mode grid -i input_00%3d.obj -m map_00%3d.png -o output_00%3d_pcloud.obj
```

```
mm.exe \
	sequence --firstFrame 150 --lastFrame 165
	quantize --qp 12  -i input_00%3d.obj -o quantized_%3d.obj
	sample --mode grid -i quantized_%3d.obj -m map_00%3d.png -o pcloud_00%3d.obj
```

The following statement will perform an analysis of the frames of a sequence and ouput a summary into file globals.txt. This
text file can then be directly sourced by bash to access the variables and reinject into quantization command for instance. 
In the following example, the extremums (Position bounding box, normal bounding box and uv bounding box) computed for the entire 
sequence by analyse will be used as the quantization range for each frame by the quantize sequence.

```
mm.exe \
	sequence --firstFrame 150 --lastFrame 165 ENd \
	analyse --inputModel input_00%3d.obj --outputVar globals.txt

# load result in memory
source globals.txt

mm.exe \
	sequence --firstFrame 150 --lastFrame 165 END \
	quantize --inputModel input_00%3d.obj  --outputModel=output_00%3d.obj \
		--qp 12 --qt 12 --qn 10
		--minPos="${globalMinPos}" --maxPos="${globalMaxPos}" \
		--minUv="${globalMinUv}"   --maxUv="${globalMaxUv}" \
		--minNrm="${globalMinNrm}" --maxNrm="${globalMaxNrm}"
```