###################################
# Makefile for NEB examples
#
###################################


# Source code directories
SRC_INCLUDE=../include

CC=gcc
MOD_CFLAGS=-fPIC
CFLAGS=-g -O2 -DHAVE_CONFIG_H
MOD_LDFLAGS=-shared
LDFLAGS=
LIBS=

prefix=/usr/local/nagios
exec_prefix=${prefix}
BINDIR=${exec_prefix}/bin
INSTALL=/usr/bin/install -c
INSTALL_OPTS=-o nagios -g nagios
COMMAND_OPTS=-o nagios -g nagcmd
STRIP=/usr/bin/strip

CP=@CP@

all:	helloworld.o

helloworld.o: helloworld.c
	$(CC) -I.. $(MOD_CFLAGS) $(CFLAGS) -o helloworld.o helloworld.c $(MOD_LDFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f helloworld.o
	rm -f core *.o
	rm -f *~ *.*~

distclean: clean
	rm -f Makefile

devclean: distclean

install:

