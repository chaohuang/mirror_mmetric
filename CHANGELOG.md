
# Change log

Primary author Jean-Eudes Marvie (InterDigital).

Additional contributors mentioned per version or item hereafter.

## Version 0.1.6

Contributors: Danillo Graziosi (Sony), Ali Tabatabai (Sony)

- add fixed point option to position quantize/dequantize
- add post cleanup to quantize command to remove degenerate triangles (T-vertex removal not yet implemented) 
- add sum in statistics (be aware of potential overflow for many sample and large sample values)
- fix bounding box update in quantize method so --dequantize option works correctly when bbox are processed internally.
- fix set cout floating point precision identical to log file precision
- fix add #include <algorithm>, to mmGeometry.h to remove build issue on some platforms

## Version 0.1.5

- add quantize/dequantize for qp, qt, qn, qc
- add analyse command to get stats on frame and sequences
- fix command registration system

## Version 0.1.4

- add remove duplicates for all sample modes (for exact vertex equality only)
- add degrade command for metric experiments, remove one face every n faces
- fix compare pcc, set bAverageNormals to true by default
- fix compare pcqm, invert ref and deg as in original source code 
- fix compare pcqm, bbox computation in model convert
- fix reindex, issues with model normal API not complete
- fix 3rdparty pcc, change distance threshold from 1e-8 to 1e-20, leads to self compare = Inf

## Version 0.1.3

Update license to Apache 2.0

- Change compare pcqm and pcc default sampling to sdiv
- add reindex command, tests and doc
- add eq comparison method for meshes with topology reordering
- add plane reorder and shift test
- add documentation
- add model builder and generic compare
- add statistics std dev and variance computation for pcqm and pcc
- add support for RGBA input textures
- add filename templates, model cache cleanup
- add sequence command
- add multi frame system
- pcqm use similar maxEnergy as for pcc.
- fix sample sdiv remove duplicates
- fix normal not generated in subdiv sdiv mode
- fix subdiv default areaThreshold type

## Version 0.1.2

- add and use model store for all commands
- add multiple command system
- add doc folder and readme base file
- add root test script
- add change log
- add sample sdiv map threshold option
- fix pcqm bug: if no input color use white
- fix in compare pcqm color not properly set

## Version 0.1.1

- add cmake
- and build.sh script
- add clean.sh script

Compare pcc
- add default options,
- add duplicate points handling
- fix pcc point cloud init,
- fix printers to preserve highest precision
- fix several issues in compare pcc
- update references

Compare pcqm
- compare pcqm set default RadiusCurvature to 0.01 to reduce process time

Sampling
- add subdivision sampling (sdiv)
- sdiv use vertex texels adjacancy criterion
- add mapCoord function

Other fixes
- texture sampling fix texture shift
- ply loader fix loading uvcoordinates
- ply/obj writers use full float dynamic

## Version 0.1.0

- add sampling map, face, grid,
- add compare eq, pcc, pcqm
- add quantize (Contributor: Yannick Olivier - InterDigital)
