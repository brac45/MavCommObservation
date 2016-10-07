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
TARGET := $(BIN)/bluetooth-server.out

# Source files
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# Common headers
MAVMSG := -I ../common-headers/testing
DBINC := -I ../common-headers/database

# Libraries
SQLITE := -lsqlite3
BLUETOOTH := -lbluetooth

# =========== Build procedure ========
all:
	@echo "Compiling bluetooth server.."
	$(CC) $(CFLAGS) $(MAVMSG) $(DBINC) $(SOURCES) -o $(TARGET) $(BLUETOOTH) $(SQLITE)

clean:
	@echo "Cleaning.."
	rm -vf $(TARGET)

.PHONY: clean
