#
#	Copyright (C) 1987, 1988 Chuck Simmons
#	Portions of this file Copyright (C) 1998 Jim Wise
#
# $Id: Makefile,v 1.12 1998/08/08 17:21:27 jwise Exp $
#
# See the file COPYING, distributed with empire, for restriction
# and warranty information.

# Note: When the version changes, you also have to change
#  * the name of the containing directory
#  * the RPM spec file
V=1.1

# Use -g to compile the program for debugging.

CC=gcc

#DEBUG = -g -DDEBUG
DEBUG = -ansi -Wall -W -Werror -pedantic -Wtraditional -Wshadow -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
	-Wmissing-declarations -Wnested-externs -Winline
# -Wid-clash-6 -Wredundant-decls  -Wconversion -Waggregate-return

# Use -p to profile the program.
#PROFILE = -p -DPROFILE
PROFILE =

# Define all necessary libraries.  'curses' is necessary.  'termcap'
# is needed on BSD systems.
LIBS = -L/usr/pkg/lib -lncurses
#LIBS = -lcurses # -ltermcap

# You shouldn't have to modify anything below this line.

TARGET = empire

CFLAGS = $(DEBUG) $(PROFILE) -DUSE_NCURSES -I/usr/pkg/include

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

all: $(TARGET)

$(TARGET): $(OFILES)
	$(CC) $(PROFILE) -o empire $(OFILES) $(LIBS)

lint: $(FILES)
	lint -u $(FILES) -lcurses

clean:
	rm -f *.o $(TARGET)

clobber: clean
	rm -f empire.tar*

SOURCES = READ.ME empire.6 COPYING Makefile BUGS $(FILES) $(HEADERS)

empire.tar: $(SOURCES)
	(cd ..; tar -cvf empire-$(V)/empire.tar `echo $(SOURCES) | sed "/\(^\| \)/s// empire-$(V)\//g"`)
empire.tar.gz: empire.tar
	gzip -9 -f empire.tar

empire.shar: $(SOURCES)
	shar $(SOURCES) >empire.shar

$(OFILES): extern.h empire.h
