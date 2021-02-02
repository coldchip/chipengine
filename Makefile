CC      := gcc
LD      := ld
BIN     := bin
SRCS    := $(wildcard *.c)
EXE     := $(BIN)/chipengine
CFLAGS  := -Wall -std=c99 -g
LIBS    := 
ifeq ($(OS),Windows_NT)
	LIBS := $(LIBS) -lws2_32
endif

.PHONY: clean install

all:
	./build.sh
run:
	$(EXE)
clean:
	rm -rf bin/*