#-----------------------------------------------------------------------
# Copyright (c) AccelerEyes LLC. All rights reserved.
# See http://www.accelereyes.com/eula for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the above copyright notices for more information.
#-----------------------------------------------------------------------

## Set the default ArrayFire installation directory (if not already set)
AF_PATH ?= ../..

## Set the default ArrayFire and CUDA installation directories (if not already set)
CUDA_PATH ?= /usr/local/cuda
# We ship all the headers and libraries needed for OpenCL

# determine operating system
OS_WIN = 0
OS_LNX = 0
OS_MAC = 0

ifeq ($(shell uname), Darwin)
  OS_MAC = 1
  CFLAGS += -DOS_MAC
endif
ifeq ($(shell uname), Linux)
  OS_LNX = 1
  CFLAGS += -DOS_LNX
endif
ifeq ($(OS_MAC), 0)
  ifeq ($(OS_LNX), 0)
    OS_WIN = 1
    CFLAGS += -DOS_WIN
  endif
endif

# determine if 64bit OS
ifeq ($(OS_WIN), 1)
  ifeq ($(findstring 64, $(shell uname -s)), 64)
    OS := 64
  endif
else
  ifeq ($(shell uname -m), x86_64)
    OS := 64
  endif
endif

ifeq ($(OS_MAC),1)
  VERSION=$(shell sw_vers -productVersion)
  ifeq ($(shell [[ $(VERSION) =~ 10\.6.* ]] && echo 1 || echo 0),1) # SnowLeopard
    OS := 64
  endif
  LIB := lib
else
  LIB := lib$(OS)
endif

ifeq ($(OS),64)
  CFLAGS += -m64
endif

ifeq ($(OS_WIN),0)
  CFLAGS += -Wall -Werror -lstdc++
endif

CFLAGS   += -I$(AF_PATH)/include -pthread
CFLAGS   += $(shell test -f .DEBUG && echo -g -O0 -DDEBUG || echo -O2 -DNDEBUG)
ifeq ($(OS_LNX),1)
  LDFLAGS  += -Wl,--no-as-needed
endif
LDFLAGS  += -L$(AF_PATH)/$(LIB) -lpthread -lm
LDFLAGS  += -Wl,-rpath,$(AF_PATH)/$(LIB),-rpath,$(abspath $(AF_PATH))/$(LIB)

ifeq ($(findstring cuda, $(MAKECMDGOALS)), cuda)
        LDFLAGS += -Wl,-rpath,$(CUDA_PATH)/$(LIB)
	LDFLAGS += -lafcu -L$(CUDA_PATH)/$(LIB) -lcudart -lcurand -lcusparse -lstdc++
	CFLAGS += -I$(CUDA_PATH)/include
	EXT:=cuda
	WIN_BIN_DIR := CUDA
	TARGET := 'Release|CUDA_x64'
    ifeq ($(OS_MAC),1)
        LDFLAGS += -F/Library/Frameworks -framework CUDA
    else
        ifeq ($(OS_LNX),1)
	    LDFLAGS += -lcuda -lrt
        endif
    endif
else
	CFLAGS += -DAFCL
	LDFLAGS += -lafcl -lstdc++
	EXT:=ocl
	WIN_BIN_DIR := OpenCL
	TARGET := 'Release|OpenCL_x64'
    ifeq ($(OS_MAC),1)
        CFLAGS += -framework OpenCL
    endif
endif

SRC:=$(wildcard *.cpp)
BIN:=$(patsubst %.cpp, %_$(EXT), $(SRC))
NOEXT:=$(patsubst %.cpp, %, $(SRC))
COMPUTE_DEVICE?=0

ifeq ($(OS_WIN),1)
    BIN := $(NOEXT:%=$(WIN_BIN_DIR)/%.exe)
endif

CPPFLAGS += $(CFLAGS)

cuda: $(BIN)

opencl: $(BIN)

run : $(BIN)
	for F in $(BIN); do echo $$F; ./$${F} $(COMPUTE_DEVICE) -; done

$(NOEXT): $(BIN) rmsym
	ln -s $(BIN) $(NOEXT)

rmsym:
	rm -f $(NOEXT)

%_ocl: %.cpp
	$(CC) $(CPPFLAGS) $(LDFLAGS) $< -o $@

%_cuda: %.cpp
	$(CC) $(CPPFLAGS) $(LDFLAGS) $< -o $@

$(WIN_BIN_DIR)/%.exe: %_vs2008.sln %.cpp
	devenv.com $< /Rebuild $(TARGET)
	rm -rf $(WIN_BIN_DIR)/*.{pdb,ilk,ncb,suo,user} %USERPROFILE%

debug:
	@test -f .DEBUG && (rm .DEBUG; echo off) || (touch .DEBUG; echo on)

clean:
ifeq ($(OS_WIN), 1)
	rm -rf OpenCL CUDA
else
	rm -f *.o $(NOEXT) *_cuda *_ocl
endif

.PHONY: all run debug clean

.NOTPARALLEL: $(WIN_BIN_DIR)/%.exe $(WIN_BIN_DIR)/%.exe
