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
TARGET := $(BIN)/wifi-client.out

# Sources
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# Common headers
MAVMSG := -I ../common-headers/testing -I ../common-headers/common
DBINC := -I ../common-headers/database

# libraries
SQLITE := -lsqlite3

# =========== Build procedure ========
#
.PHONY: clean

all:
	@echo "Compiling wifi client.."
	$(CC) $(CFLAGS) $(MAVMSG) $(DBINC) $(SOURCES) -o $(TARGET) $(SQLITE)

clean:
	@echo "Cleaning.."
	rm -vf $(TARGET)

