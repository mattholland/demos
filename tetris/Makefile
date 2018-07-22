#
# A simple makefile for managing build of project composed of C source files.
#


# Set the compiler, on OSX gcc invokes clang
CC = gcc

# The CFLAGS variable sets compile flags for gcc:
#  -g        compile with debug information
#  -c        compile but don't link
#  -Wall     give verbose compiler warnings
#  -O0       do not optimize generated code
#  -std=c99  use the C99 standard language definition
#  -M		 ask the compiler to tell you where it found include files
CFLAGS = -g -Wall -O0 -std=c99

# The LDFLAGS variable sets flags for linker
#  -lm   says to link in libm (the math library)
LDFLAGS = -lncurses

# In this section, you list the files that are part of the project.
# If you add/change names of source files, here is where you
# edit the Makefile.
SOURCES = main.c 
OBJECTS = $(SOURCES:.c=.o)
TARGET = tetris


# The first target defined in the makefile is the one
# used when make is invoked with no argument. Given the definitions
# above, this Makefile file will build the one named TARGET and
# assume that it depends on all the named OBJECTS files.
# $@ and $^ are automatic variables, $@ is the name of the target, $^ is the name of the dependencies

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Phony means not a "real" target, it doesn't build anything
# The phony target "clean" is used to remove all compiled object files.

.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJECTS) core



