CC=g++
CFLAGS=-c -Wall
LDFLAGS=
EXECUTABLE=chip8
SDL=-framework SDL2

all: $(EXECUTABLE)

$(EXECUTABLE): run.o chip8.o
	$(CC) $(LDFLAGS) $(SDL) $^ -o $@

run.o: run.cpp
	$(CC) $(CFLAGS) $^ -o run.o

chip8.o: chip8.cpp
	$(CC) $(CFLAGS) $^ -o chip8.o

clean:
	rm *.o && rm $(EXECUTABLE)
