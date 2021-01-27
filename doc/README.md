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

Use specific grid sampling method, then compare using pcc_error and pcqm metrics in a single call.

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
	compare --mode pcc  --inputModelA pcA.ply --inputModelB pcB.ply ENd \
	compare --mode pcqm --inputModelA pcA.ply --inputModelB pcB.ply ENd \
```

Several commands can be cascaded using this mechanism, for instance doing quantization then sampling then compare. 
Note however that memory won't be released between sub command calls so cascading many commands may be very consuming in terms of memory.
