#
#	Copyright (C) 1987, 1988 Chuck Simmons
#	Portions of this file Copyright (C) 1998 Jim Wise
#
# $Id: Makefile,v 1.4 1998/02/24 23:47:56 jim Exp $
#
# See the file COPYING, distributed with empire, for restriction
# and warranty information.

# Note: When the version changes, you also have to change
#  * the name of the containing directory
#  * the RPM spec file
V=1.1

# Change the line below for your system.  If you are on a Sun or Vax,
# you may want BSD.

#SYS = BSD
SYS = SYSV

# Use -g to compile the program for debugging.

CC=gcc

#DEBUG = -g -DDEBUG
DEBUG = -Wall -W -Werror

# Use -p to profile the program.
#PROFILE = -p -DPROFILE
PROFILE =

# Define all necessary libraries.  'curses' is necessary.  'termcap'
# is needed on BSD systems.
#LIBS = -lncurses
LIBS = -lcurses # -ltermcap

# You shouldn't have to modify anything below this line.

CFLAGS = $(DEBUG) $(PROFILE) -D$(SYS)

FILES = \
	attack.c \
	compmove.c \
	data.c \
	display.c \
	edit.c \
	empire.c \
	game.c \
	main.c \
	map.c \
	math.c \
	object.c \
	term.c \
	usermove.c \
	util.c

HEADERS = empire.h extern.h

OFILES = \
	attack.o \
	compmove.o \
	data.o \
	display.o \
	edit.o \
	empire.o \
	game.o \
	main.o \
	map.o \
	math.o \
	object.o \
	term.o \
	usermove.o \
	util.o

all: empire

empire: $(OFILES)
	$(CC) $(PROFILE) -o empire $(OFILES) $(LIBS)

TAGS: $(HEADERS) $(FILES)
	etags $(HEADERS) $(FILES)

lint: $(FILES)
	lint -u -D$(SYS) $(FILES) -lcurses

clean:
	rm -f *.o TAGS

clobber: clean
	rm -f empire empire.tar*

SOURCES = READ.ME empire.6 COPYING Makefile BUGS $(FILES) $(HEADERS) MANIFEST empire.lsm empire.spec

empire.tar: $(SOURCES)
	(cd ..; tar -cvf empire-$(V)/empire.tar `echo $(SOURCES) | sed "/\(^\| \)/s// empire-$(V)\//g"`)
empire.tar.gz: empire.tar
	gzip -f empire.tar

empire.shar: $(SOURCES)
	shar $(SOURCES) >empire.shar
