CFLAGS = -Wall -Wfatal-errors -Werror

MAIN_FILE = main.c

DEBUG = -g

# Compile dependencies (.c/.h -> .o)
compile:
	make temp
	make temp/simulation.o
	make temp/objects.o
	make temp/ants.o
	make temp/rooms.o

	make temp/main.o
	make Fourmi.exe

	make clean


# dependencies
temp:
	mkdir temp
temp/simulation.o: simulation.c
	gcc $(CFLAGS) -c simulation.c -o temp/simulation.o $(DEBUG)
temp/objects.o: objects.c
	gcc $(CFLAGS) -c objects.c -o temp/objects.o $(DEBUG)
temp/ants.o: ants.c
	gcc $(CFLAGS) -c ants.c -o temp/ants.o $(DEBUG)
temp/rooms.o: rooms.c
	gcc $(CFLAGS) -c rooms.c -o temp/rooms.o $(DEBUG)


# main file
temp/main.o: $(MAIN_FILE)
	make temp
	gcc $(CFLAGS) -c $(MAIN_FILE) -o temp/main.o $(DEBUG)

Fourmi.exe: temp/simulation.o temp/objects.o temp/ants.o temp/rooms.o
	gcc $(CFLAGS) temp/main.o temp/simulation.o temp/objects.o temp/ants.o temp/rooms.o -o Fourmi.exe

#cleanup
clean:
	rm -r temp

uncompile:
	rm Fourmi.exe
