VERSION 	= 0
PATHLEVEL	= 1
SUBLEVEL	= 0
EXTRAVERSION 	= 
NAME		= Navier-Stokes fluido incomprimibile omogeneo
AUTHOR		= Stefano Mandelli
DATE		= 9 Aprile 2016
SRCDIR		= src
## Switch #######################################
DEVICE		:=true
DEBUGMODE	:= true
#################################################


#Makefile for the Device ########################

NVCC		:= nvcc -G
ESEGUIBILE	:= fluids.x
OBJ			:= fluidsGL.o fluidsGL_kernels.o
CC			:= g++
FLAGS 		:= -O2 -g 
ARCH 		:= -arch=sm_20
LIBS		:= -lGL -lglut -lGLEW  -lcuda -lcufft
INC			:= -I/usr/local/cuda/include/ -I/usr/local/cuda/samples/common/inc 

%.o : %.cpp
	$(CC) ${LIBS} ${FLAGS} ${INC}  -c $< -o $@

%.o : %.cu
	$(NVCC) ${FLAGS} ${ARCH} ${LIBS} ${INC} -c $< -o $@

${ESEGUIBILE}: ${OBJ}
	$(NVCC) ${FLAGS} ${ARCH} ${LIBS} ${INC} -o $@ $^	

.PHONY: clean

clean:
	rm -rf *.x *.o


