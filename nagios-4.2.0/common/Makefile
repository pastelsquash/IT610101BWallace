############################
# Makefile for Nagios
#
############################

# Source code directories
SRC_BASE=../common
SRC_CGI=../cgi

CC=gcc
CFLAGS=-g -O2 -DHAVE_CONFIG_H
LDFLAGS= 

prefix=/usr/local/nagios
exec_prefix=${prefix}
LOGDIR=${prefix}/var
CFGDIR=${prefix}/etc
BINDIR=${exec_prefix}/bin
CGIDIR=${exec_prefix}/sbin
HTMLDIR=${prefix}/share
INSTALL=/usr/bin/install -c
INSTALL_OPTS=-o nagios -g nagios
COMMAND_OPTS=-o nagios -g nagcmd

CP=@CP@


clean:
	rm -f core *.o
	rm -f *~

distclean: clean
	rm -f Makefile

devclean: distclean

install:

