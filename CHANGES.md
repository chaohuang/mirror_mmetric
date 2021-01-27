
# Change log

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
- add quantize
