CFLAGS = -Wall -Wfatal-errors -Werror

MAIN_FILE ?= demo.c

DEBUG = -g

# Compile dependencies (.c/.h -> .o)
compile: temp/simulation.o temp/objects.o temp/ants.o temp/rooms.o temp/main.o
	gcc $(CFLAGS) temp/main.o temp/simulation.o temp/objects.o temp/ants.o temp/rooms.o -o Fourmi.exe
	rm -r temp


# dependencies
temp:
	mkdir temp
temp/simulation.o: simulation.c temp
	gcc $(CFLAGS) -c simulation.c -o temp/simulation.o $(DEBUG)
temp/objects.o: objects.c temp
	gcc $(CFLAGS) -c objects.c -o temp/objects.o $(DEBUG)
temp/ants.o: ants.c temp
	gcc $(CFLAGS) -c ants.c -o temp/ants.o $(DEBUG)
temp/rooms.o: rooms.c temp
	gcc $(CFLAGS) -c rooms.c -o temp/rooms.o $(DEBUG)


# main file
temp/main.o: $(MAIN_FILE) temp
	gcc $(CFLAGS) -c $(MAIN_FILE) -o temp/main.o $(DEBUG)


#cleanup
clean:
	rm -r temp

.PHONY: clean compile
