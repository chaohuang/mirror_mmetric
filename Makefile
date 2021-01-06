CC = g++
CFLAGS = -g -Wall
SRCS = source/main.cpp source/mmImage.cpp source/mmIO.cpp source/mmGeometry.cpp \
       source/mmQuantize.cpp source/mmSample.cpp source/mmCompare.cpp \
       3rdparty/pcc/pcc_distortion.cpp 3rdparty/pcc/pcc_processing.cpp 3rdparty/pcqm/pcqm.cpp
PROG = mm

$(PROG):$(SRCS)
	$(CC) -std=c++11 -fno-fast-math -fopenmp -I ./3rdparty -I ./3rdparty/eigen3 -I ./3rdparty/nanoflann -O3 $(CFLAGS) -o $(PROG) $(SRCS) 
