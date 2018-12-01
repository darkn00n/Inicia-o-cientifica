# The first rule of a Makefile is the default target. It will be built when make is called with no parameter
# Here, we want to build the binary 'mysimulator'
all:	main

# This second rule lists the dependencies of the mysimulator binary
# How this dependencies are linked is described in an implicit rule below
main:	main.o #util.o

# These third give the dependencies of the each source file
main.o:	main.cpp #util.h # list every .h that you use
#util.o: util.c util.h

# Some configuration

SIMGRID_INSTALL_PATH	=	/opt/simgrid# Where you installed simgrid

CC	=	g++# Your compiler

WARNING	=	-Wshadow	-Wcast-align	-Waggregate-return	-Wmissing-prototypes	\
          -Wmissing-declarations	-Wstrict-prototypes	-Wmissing-prototypes	\
          -Wmissing-declarations	-Wmissing-noreturn	-Wredundant-decls	\
          -Wnested-externs	-Wpointer-arith	-Wwrite-strings	-finline-functions

# CFLAGS = -g -O0 $(WARNINGS) # Use this line to make debugging easier

CFLAGS	=	-O2	$(WARNINGS)	# Use this line to get better performance

# No change should be mandated past that line
#############################################
# The following are implicit rules, used by default to actually build
# the targets for which you listed the dependencies above.
# The blanks before the $(CC) must be a Tab char, not spaces

%:	%.o
	$(CC)	-L$(SIMGRID_INSTALL_PATH)/lib/	$(CFLAGS)	$^	-lsimgrid	-o	$@
%.o:	%.c
	$(CC)	-I$(SIMGRID_INSTALL_PATH)/include	$(CFLAGS)	-c	-o	$@	$<
clean:
	rm	-f	*.o	*~

.PHONY:	clean
