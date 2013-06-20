# -*- coding: utf-8; -*-
#
# @file total/Makefile
#
# Copyright © 2008-2013 Totzeit, Inc.
#
# This work is licensed under
#
#     Creative Commons Attribution 3.0 Unported License (CC BY 3.0)
#
# the full text of which may be retrieved at
#
#     http://creativecommons.org/licenses/by/3.0/
#

TOOLS = total

SRCS = total.c
OBJS = $(SRCS:.c=.o)

LIBS = -lm

LDFLAGS +=
# CFLAGS  += -g -Wall
CFLAGS += -O3 -Wall -pedantic -std=c99

CFLAGS  += -D_GNU_SOURCE

all:	$(TOOLS)

total:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

clean:
	$(RM) $(OBJS) $(TOOLS)

# Local Variables:
# indent-tabs-mode: nil
# fill-column: 79
# comment-column: 37
# End:

