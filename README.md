File automatically generated by build-doc.sh and ./bin/mm.exe, do not edit manually.

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
# Command references

```
3D model processing commands v0.1.6
Usage:
  mm.exe command [OPTION...]

Command help:
  mm.exe command --help

Command:
  analyse	Analyse model and/or texture map
  compare	Compare model A vs model B
  degrade	Degrade a mesh (todo points)
  dequantize	Dequantize model (mesh or point cloud) 
  quantize	Quantize model (mesh or point cloud)
  reindex	Reindex mesh and optionaly sort vertices and face indices
  sample	Convert mesh to point cloud
  sequence	Sequence global parameters

```

## Sample

```
Convert mesh to point cloud
Usage:
  mm.exe sample [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -m, --inputMap arg     path to input texture map (png, jpeg)
  -o, --outputModel arg  path to output model (obj or ply file)
      --mode arg         the sampling mode in [face,grid,map,sdiv]
      --hideProgress     hide progress display in console for use by robot
  -h, --help             Print usage

 Face mode options:
      --float           if set the processings and outputs will be float32,
                        int32 otherwise (default: true)
      --resolution arg  integer value in [1,maxuint], nb samples per edge of
                        maximal size (default: 1024)
      --thickness arg   floating point value, distance to border of the face
                        (default: 0.0)

 Grid mode options:
      --gridSize arg  integer value in [1,maxint], side size of the grid
                      (default: 1024)

 Grid, Face and sdiv modes options:
      --bilinear  if set, texture filtering will be bilinear, nearest
                  otherwise (default: true)

 Sdiv mode options:
      --areaThreshold arg  area limit to stop subdivision (default: 1.0)
      --mapThreshold       if set will refine until face vertices texels are
                           distanced of 1 and areaThreshold reached

```

## Compare

```
Compare model A vs model B
Usage:
  mm.exe compare [OPTION...]

      --inputModelA arg   path to input model A (obj or ply file)
      --inputModelB arg   path to input model B (obj or ply file)
      --inputMapA arg     path to input texture map A (png, jpeg)
      --inputMapB arg     path to input texture map B (png, jpeg)
      --outputModelA arg  path to output model A (obj or ply file)
      --outputModelB arg  path to output model B (obj or ply file)
      --mode arg          the comparison mode in [equ,pcc,pcqm] (default:
                          equ)
  -h, --help              Print usage

 equ mode options:
      --epsilon arg  Used for point cloud comparison only. Distance threshold
                     in world units for "equality" comparison. If 0.0 use
                     strict equality (no distace computation). (default: 0.0)
      --earlyReturn  Return as soon as a difference is found (faster).
                     Otherwise provide more complete report (slower). (default:
                     true)
      --unoriented   If set, comparison will not consider faces orientation
                     for comparisons.

 pcc mode options:
      --singlePass          Force running a single pass, where the loop is
                            over the original point cloud
      --hausdorff           Send the Haursdorff metric as well
      --color               Check color distortion as well (default: true)
      --resolution arg      Amplitude of the geometric signal. Will be
                            automatically set to diagonal of the models bounding box
                            if value = 0 (default: 0.0)
      --neighborsProc arg   0(undefined), 1(average), 2(weighted average)
                            3(min), 4(max) neighbors with same geometric distance
                            (default: 1)
      --dropDuplicates arg  0(detect), 1(drop), 2(average) subsequent points
                            with same coordinates (default: 2)
      --bAverageNormals     false(use provided normals), true(average normal
                            based on neighbors with same geometric distance)
                            (default: true)

 pcqm mode options:
      --radiusCurvature arg     Set a radius for the construction of the
                                neighborhood. As the bounding box is already
                                computed with this program, use proposed value.
                                (default: 0.001)
      --thresholdKnnSearch arg  Set the number of points used for the quadric
                                surface construction (default: 20)
      --radiusFactor arg        Set a radius factor for the statistic
                                computation. (default: 2.0)

```

## Reindex

```
Reindex mesh and optionaly sort vertices and face indices
Usage:
  mm.exe reindex [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
  -h, --help             Print usage
      --sort arg         Sort method in none, vertices, oriented, unoriented.
                         (default: none)

```
## Quantize

```
Quantize model (mesh or point cloud)
Usage:
  mm.exe quantize [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
  -h, --help             Print usage
      --dequantize       set to process dequantification at the ouput
      --qp arg           Geometry quantization bitdepth (default: 12)
      --qt arg           UV coordinates quantization bitdepth (default: 12)
      --qn arg           Normals quantization bitdepth (default: 12)
      --qc arg           Colors quantization bitdepth (default: 8)
      --minPos arg       min corner of vertex position bbox, a string of
                         three floats. Computed of not set.
      --maxPos arg       max corner of vertex position bbox, a string of
                         three floats. Computed of not set.
      --minUv arg        min corner of vertex texture coordinates bbox, a
                         string of three floats. Computed of not set.
      --maxUv arg        max corner of vertex texture coordinates bbox, a
                         string of three floats. Computed of not set.
      --minNrm arg       min corner of vertex normal bbox, a string of three
                         floats. Computed of not set.
      --maxNrm arg       max corner of vertex normal bbox, a string of three
                         floats. Computed of not set.
      --minCol arg       min corner of vertex color bbox, a string of three
                         floats. Computed of not set.
      --maxCol arg       max corner of vertex color bbox, a string of three
                         floats. Computed of not set.
      --outputVar arg    path to the output variables file.
      --useFixedPoint    internally convert the minPos and maxPos to fixed
                         point 16.

```
## Degrade

```
Degrade a mesh (todo points)
Usage:
  mm.exe degrade [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
      --mode arg         the sampling mode in [delface]
      --nthFace arg      in delface mode, remove one face every nthFace.
                         (default: 50)
  -h, --help             Print usage

```
## Sequence

```
Sequence global parameters
Usage:
  mm.exe sequence [OPTION...]

      --firstFrame arg  Sets the first frame of the sequence, included.
                        (default: 0)
      --lastFrame arg   Sets the last frame of the sequence, included. Must
                        be >= to firstFrame. (default: 0)
  -h, --help            Print usage

```
