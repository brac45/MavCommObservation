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
BINDIR := bin/
TARGET_SERVER := bin/server
TARGET_CLIENT := bin/client

# File extensions and some more options
SRCEXT := c
SERVER_SRC := l2cap-echo-server.c
CLIENT_SRC := l2cap-echo-client.c
LIBRARY := -lbluetooth
INCLUDE := -I lib/common/

# =========== Build procedure ========
server: 
	@echo "Building bluetooth server.."
	$(CC) $(INCLUDE) -o $(TARGET_SERVER) $(SRCDIR)/$(SERVER_SRC) $(LIBRARY)

client:
	@echo "Building bluetooth client.."
	$(CC) $(INCLUDE) -o $(TARGET_CLIENT) $(SRCDIR)/$(CLIENT_SRC) $(LIBRARY)

clean:
	@echo "Cleaning.."
	rm -vf bin/* 
	find $(BUILDDIR) -name "*.o" -exec rm -rvf {} \;

# TESTS
test:
	$(CC) $(CFLAGS) test/tester.cpp $(INCLUDE) $(LIBRARY) -o bin/tester

.PHONY: clean
