#
#	Copyright (C) 1987, 1988 Chuck Simmons
#
# $Id: Makefile,v 1.24 1998/09/11 20:23:06 jwise Exp $
#
# See the file COPYING, distributed with empire, for restriction
# and warranty information.

VERSION=1.2

#
# 1.) Pick your compiler
#	If you want to use your system's native compiler, use the following.
#	It MUST be at least relatively ANSI compliant for this to work.
#       	CC=cc
#
#	Pick the following to use GCC (or EGCS).  Needed if your system's
#	native compiler is not ANSI compliant (or not present)
#		CC=gcc
#
CC=gcc

#
# 2.) Pick your preprocessor defines
#	choose from the following options:
#		-DDEBUG		turn on consistency checking
#		-DUSE_NCURSES	you need to turn this on if you want
#				to use the ncurses library.  Use this if
#				cempire doesn't work with your system's
#				native curses library
#
DEFINES=-DDEBUG

#
# 3.) Pick one of the following LIBS lines
#	for System V curses, or other non-termcap curses implementations, use:
#		LIBS=-lcurses
#
#	for traditional BSD curses implementations, which rely on the
#	termcap libraries, use:
#		LIBS=-lcurses -ltermcap
#
#	if you want to use ncurses.  Change /usr/local/lib to wherever
#	you have the ncurses library installed.  You also need to turn on
#	-DUSE_NCURSES above, and check your includes below.
#		LIBS=-L/usr/local/lib -lncurses
#
#	use the following if you have ncurses installed using NetBSD's
#	wonderful package system.
#		LIBS=-L/usr/pkg/lib -lncurses
#
LIBS=-lcurses

#
# 4.) Pick your includes
#
#	If your system curses works, use this:
#		INCLUDES=
#
# 	if you are using ncurses, you need this.  Change /usr/local/include to
#	wherever you have the ncurses headers installed, and turn on
#	-DUSE_NCURSES and your curses library, above
#		INCLUDES=-I/usr/local/include
#
#	use this if you have ncurses installed under NetBSD's package system.
#		INCLUDES=-I/usr/pkg/include
#
INCLUDES=

#
# The following turns on _extremely_ pedantic error checking for gcc or egcs.
# Don't try to use this if you are using another compiler.  If cempire
# won't build this way on your system, please let me know...
#
WARNS=-ansi -Wall -W -Werror -pedantic -Wtraditional -Wshadow \
	-Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings \
	-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -Winline

#
# You shouldn't have to modify anything below this line.
#

TARGET = cempire

CPPFLAGS= $(DEFINES) $(INCLUDES)
CFLAGS= $(CPPFLAGS) $(COPTS) $(WARNS)
LINTFLAGS = -brxHu

FILES= attack.c compmove.c data.c display.c edit.c empire.c game.c main.c \
	map.c math.c object.c term.c usermove.c util.c

HEADERS= empire.h extern.h

OFILES= attack.o compmove.o data.o display.o edit.o empire.o game.o main.o \
	map.o math.o object.o term.o usermove.o util.o

MISCFILES= READ.ME COPYING BUGS Makefile cempire.6 .cvsignore

SOURCES= $(MISCFILES) $(FILES) $(HEADERS)

ARCHIVES= cempire-$(VERSION).tar cempire-$(VERSION).tar.gz \
	cempire-$(VERSION).shar

all: $(TARGET)

$(TARGET): $(OFILES)
	$(CC) -o $(TARGET) $(OFILES) $(LIBS)

lint: $(FILES)
	lint $(LINTFLAGS) $(CPPFLAGS) $(FILES) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)

clobber:	clean
	rm -f $(ARCHIVES)

tar:	cempire-$(VERSION).tar
tgz:	cempire-$(VERSION).tar.gz
shar:	cempire-$(VERSION).shar

cempire-$(VERSION).tar: $(SOURCES)
	(cd ..; \
	 tar -cvf cempire-$(VERSION)/cempire-$(VERSION).tar \
	 `for X in $(SOURCES); do echo cempire-$(VERSION)/$$X; done`)

cempire-$(VERSION).tar.gz: cempire-$(VERSION).tar
	gzip -9 -f cempire-$(VERSION).tar

cempire-$(VERSION).shar: $(SOURCES)
	shar $(SOURCES) >cempire.shar

$(OFILES): extern.h empire.h
