# examples/Makefile - Build examples against an installed libcreate
#
# Usage:
#   make                              (uses pkg-config to find libcreate)
#   PKG_CONFIG_PATH=/path/to make     (override search path)

CC       ?= cc
CFLAGS   ?= -Wall -Wextra -O2
PKG_CFLAGS  := $(shell pkg-config --cflags libcreate 2>/dev/null)
PKG_LIBS    := $(shell pkg-config --libs libcreate 2>/dev/null)

ifeq ($(PKG_LIBS),)
$(error pkg-config could not find libcreate. Set PKG_CONFIG_PATH.)
endif

.PHONY: all clean

all: hello

hello: hello.c
	$(CC) $(CFLAGS) $(PKG_CFLAGS) -o $@ $< $(PKG_LIBS)

clean:
	rm -f hello
