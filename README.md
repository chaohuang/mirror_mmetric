File automatically generated by build-doc.sh and ./bin/mm.exe, do not edit manually.

# Compilation

Build software with following command:
- ./build.sh [release|debug] [noomp] [nojobs]
- release|debug: sets build target (release is default)
- noomp: disables openmp build
- nojobs: disables multi-processor build on unix (i.e. use -j 1) 
- output binary produced in bin folder.

Update readme with following command:
- ./build.sh doc
- output readme.md.

# Usage

```
3D model processing commands v0.1.1
Usage:
  mm.exe command [OPTION...]

Command help:
  mm.exe command --help

Command:
  sample     Convert mesh to point cloud
  compare    Compare model A vs model B
  quantize   Quantize model (mesh or point cloud) positions

```

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

```

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
      --epsilon arg  floating point value, error threshold in equality
                     comparison. if not set use memcmp. (default: 0.0)

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

```
Quantize model (mesh or point cloud) positions
Usage:
  mm.exe quantize [OPTION...]

  -i, --inputModel arg   path to input model (obj or ply file)
  -o, --outputModel arg  path to output model (obj or ply file)
      --float            if set the processings and outputs will be float32,
                         int32 otherwise (default: true)
  -h, --help             Print usage
      --qp arg           Geometry quantization bitdepth (default: 16)

```
