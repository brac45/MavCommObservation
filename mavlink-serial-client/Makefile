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
TEST := test
TARGET := $(BIN)/serial-client.out
TESTTRGT := $(BIN)/test.out

# Source files
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# Source files for testing program
TESTSRCS := $(shell find $(TEST) -type f -name *.$(SRCEXT))

# Common headers
MAVMSG := -I ../common-headers/testing
DBINC := -I ../common-headers/database

# libraries
SQLITE := -lsqlite3

# =========== Build procedure ========
all: 
	@echo "Compiling serial client.."
	$(CC) $(CFLAGS) $(MAVMSG) $(DBINC) $(SOURCES) -o $(TARGET) $(SQLITE)

clean:
	@echo "Cleaning.."
	rm -vf $(TARGET)
	rm -vf $(TESTTRGT)

testing:
	@echo "Creating test code.."
	$(CC) $(CFLAGS) $(LIB) $(TESTSRCS) -o $(TESTTRGT)

.PHONY: clean
