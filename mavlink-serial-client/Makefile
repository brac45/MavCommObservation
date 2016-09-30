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
BIN := bin
TEST := test
TARGET := $(BIN)/serial-client.out
TESTTRGT := $(BIN)/test.out

# File extensions and some more options
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
TESTSRCS := $(shell find $(TEST) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIB := -I ../mavlink-headers/common

# =========== Build procedure ========
$(TARGET): $(OBJECTS)
	@echo "Linking.."
	$(CC) $^ -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "Creating Objects.."
	$(CC) $(CFLAGS) $(LIB) -c -o $@ $<

clean:
	@echo "Cleaning.."
	rm -vf "$(TARGET)"
	find $(BUILDDIR) -name "*.o" -exec rm -rvf {} \;

testing:
	@echo "Creating test code.."
	$(CC) $(CFLAGS) $(LIB) $(TESTSRCS) -o $(TESTTRGT)

.PHONY: clean
