TOOLS = total

SRCS = total.c
OBJS = $(SRCS:.c=.o)

LIBS = -lm

LDFLAGS +=
# CFLAGS  += -g -Wall
CFLAGS += -O3 -Wall 

CFLAGS  += -D_GNU_SOURCE

all:	$(TOOLS)

total:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

clean:
	$(RM) $(OBJS) $(TOOLS)
