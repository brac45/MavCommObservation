#
# Makefile for managing the build of the project
#

# ========== Variables ==============
# Main compiler
CC := gcc
#		-Wall : give verbose compiler warnings
#		-g : compile with debugging info
CFLAGS := -g -Wall

#	Directory and main executable variable
SRCDIR := src
BUILDDIR := build
TARGET := bin/main_exe

# File extensions and some more options
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
INCLUDE := -I include
LIBRARY := -I lib/common/

# =========== Build procedure ========
server:
	@echo "Compiling server.."
	$(CC) $(INCLUDE) $(LIBRARY) -o bin/mavlink_server src/mavlink_udp_echoserver.c

client:
	@echo "Compiling client.."
	$(CC) $(INCLUDE) $(LIBRARY) -o bin/mavlink_client src/mavlink_udp_client.c

clean:
	@echo "Cleaning.."
	rm -vf bin/*
	find $(BUILDDIR) -name "*.o" -exec rm -rvf {} \;

# TESTS
test:
	$(CC) $(CFLAGS) test/tester.cpp $(INCLUDE) $(LIBRARY) -o bin/tester

.PHONY: clean
