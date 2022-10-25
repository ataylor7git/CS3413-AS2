CC = gcc

BASEFLAGS = -Wall -pthread -std=c99 -D_XOPEN_SOURCE=700
NODEBUG_FLAGS = -dNDEBUG
DEBUG_FLAGS = -g

LDLIBS = -lcurses -pthread

OBJS = main.o console.o threadManager.o

EXE = centipede

debug: CFLAGS = $(BASEFLAGS) $(DEBUG_FLAGS)
debug: $(EXE)

release: CFLAGS = $(BASEFLAGS) $(NODEBUG_FLAGS)
release: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

console.o: console.c console.h
	$(CC) $(CFLAGS) -c console.c

example.o: example.c
	$(CC) $(CFLAGS) -c example.c

threadManager.o: threadManager.c threadManager.h
	$(CC) $(CFLAGS) -c threadManager.c

clean:
	rm -f $(OBJS)
	rm -f *~
	rm -f $(EXE)
	rm -f $(EXE)_d

run:
	./$(EXE)

run_debug:
	./$(EXE)_d
