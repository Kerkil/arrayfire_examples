AF_PATH?=../..

LIB := lib
ifeq ($(shell uname), Linux)
ifeq ($(shell uname -m), x86_64)
    LIB := lib64
endif
endif

SRC:=$(wildcard *.cpp)
BIN:=$(patsubst %.cpp, %, $(SRC))
OBJ:=$(patsubst %.cpp, %.o, $(SRC))

CFLAGS+=-I$(AF_PATH)/include -DAFCL
LDFLAGS+=-L$(AF_PATH)/$(LIB) -lafcl
LDFLAGS+=-Wl,-rpath,$(AF_PATH)/$(LIB)
LDFLAGS+=-Wl,-rpath,$(abspath $(AF_PATH))/$(LIB)
LDFLAGS+=$(AF_PATH)/$(LIB)/libOpenCL.so.1
CPPFLAGS=$(CFLAGS)
CXXFLAGS=$(CFLAGS)

all: opencl

opencl: $(BIN)

clean:
	rm -f $(BIN)
