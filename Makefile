#
#	Copyright (C) 1987, 1988 Chuck Simmons
#	Portions of this file Copyright (C) 1998 Jim Wise
#
# $Id: Makefile,v 1.17 1998/08/08 21:37:40 jwise Exp $
#
# See the file COPYING, distributed with empire, for restriction
# and warranty information.

VERSION=1.5

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

TARGET = cempire

CPPFLAGS = -DUSE_NCURSES -I/usr/pkg/include
CFLAGS = $(DEBUG) $(PROFILE)

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
	$(CC) $(PROFILE) -o $(TARGET) $(OFILES) $(LIBS)

lint: $(FILES)
	lint $(CPPFLAGS) -u $(FILES) $(LIBS)

ARCHIVES = cempire-$(VERSION).tar cempire-$(VERSION).tar.gz cempire-$(VERSION).shar

clean:
	rm -f *.o $(TARGET)

clobber:	clean
	rm -f $(ARCHIVES)

SOURCES = READ.ME cempire.6 COPYING Makefile BUGS .cvsignore $(FILES) $(HEADERS)

tar:	cempire-$(VERSION).tar
tgz:	cempire-$(VERSION).tar.gz
shar:	cempire-$(VERSION).shar

cempire-$(VERSION).tar: $(SOURCES)
	(cd ..;tar -cvf cempire-$(VERSION)/cempire-$(VERSION).tar `for X in $(SOURCES); do echo cempire-$(VERSION)/$$X; done`)

cempire-$(VERSION).tar.gz: cempire-$(VERSION).tar
	gzip -9 -f cempire-$(VERSION).tar

cempire-$(VERSION).shar: $(SOURCES)
	shar $(SOURCES) >cempire.shar

$(OFILES): extern.h empire.h
