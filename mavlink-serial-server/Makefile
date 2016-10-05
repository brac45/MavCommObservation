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
TARGET := $(BIN)/serial-server.out

# File extensions and some more options
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIB := -I ../message-headers/common
SQLITE := -l sqlite3

# =========== Build procedure ========
$(TARGET): $(OBJECTS)
	@echo "Linking.."
	$(CC) $^ -o $(TARGET) $(SQLITE)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "Creating Objects.."
	$(CC) $(CFLAGS) $(LIB) -c -o $@ $<

clean:
	@echo "Cleaning.."
	rm -vf "$(TARGET)"
	find $(BUILDDIR) -name "*.o" -exec rm -rvf {} \;

.PHONY: clean
