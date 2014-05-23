CUDA_PATH?=/usr/local/cuda
AF_PATH?=../..

LIB := lib
ifeq ($(shell uname), Linux)
ifeq ($(shell uname -m), x86_64)
    LIB := lib64
endif
endif

SRC:=$(wildcard *.cu)
BIN:=$(patsubst %.cu, %, $(SRC))
OBJ:=$(patsubst %.cu, %.o, $(SRC))
NVCC:=$(CUDA_PATH)/bin/nvcc

CFLAGS+=-I$(CUDA_PATH)/include
CFLAGS+=-I$(AF_PATH)/include
CFLAGS+=-gencode=arch=compute_30,code=sm_30
CFLAGS+=-gencode=arch=compute_20,code=sm_20

LDFLAGS+=-L$(CUDA_PATH)/$(LIB) -lcuda -lcudart
LDFLAGS+=-L$(AF_PATH)/$(LIB) -lafcu
LDFLAGS += -Xlinker -rpath -Xlinker $(AF_PATH)/$(LIB)
LDFLAGS += -Xlinker -rpath -Xlinker $(abspath $(AF_PATH))/$(LIB)

all: $(BIN)

%: %.cu
	@$(NVCC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	rm -f $(BIN)
