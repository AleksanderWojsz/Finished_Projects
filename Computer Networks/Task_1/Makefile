CC     = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu17
LFLAGS =

.PHONY: all clean

TARGET1 = ppcbc
TARGET2 = ppcbs

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(TARGET1).o utilities.o
$(TARGET2): $(TARGET2).o utilities.o

ppcbc.o: ppcbc.c utilities.h
ppcbs.o: ppcbs.c utilities.h
utilities.o: utilities.c utilities.h

clean:
	rm -f $(TARGET1) $(TARGET2) *.o *~
