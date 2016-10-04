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
BIN := bin
TARGET := $(BIN)/bluetooth-client.out

# File extensions and some more options
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
LIB := -lbluetooth
INCLUDE := -I ../mavlink-headers/common

# =========== Build procedure ========
$(TARGET): 
	@echo "Building bluetooth client.."
	$(CC) $(INCLUDE) -o $(TARGET) $(SOURCES) $(LIB)

clean:
	@echo "Cleaning.."
	rm -vf bin/* 

# TESTS
test:
	$(CC) $(CFLAGS) test/tester.cpp $(INCLUDE) $(LIBRARY) -o bin/tester

.PHONY: clean
