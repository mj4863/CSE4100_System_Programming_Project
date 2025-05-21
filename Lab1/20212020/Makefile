CC = gcc
CFLAGS = -c
OBJECTS = bitmap.o debug.o hash.o hex_dump.o list.o main.o
TARGET = testlib

$(TARGET) : $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

main.o : limits.h round.h main.c
	$(CC) $(CFLAGS) -o main.o main.c

bitmap.o : limits.h round.h bitmap.h bitmap.c
	$(CC) $(CFLAGS) -o bitmap.o bitmap.c

debug.o : limits.h round.h debug.h debug.c
	$(CC) $(CFLAGS) -o debug.o debug.c

hash.o : limits.h round.h hash.h hash.c
	$(CC) $(CFLAGS) -o hash.o hash.c

hex_dump.o : limits.h round.h hex_dump.h hex_dump.c
	$(CC) $(CFLAGS) -o hex_dump.o hex_dump.c

list.o : limits.h round.h list.h list.c
	$(CC) $(CFLAGS) -o list.o list.c

.PHONY : clean
clean:
	rm $(TARGET) $(OBJECTS)
