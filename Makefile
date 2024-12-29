CFLAGS = -Wall -Wfatal-errors -Werror

# Compile (.c -> .o)
compile_all:
	make temp
	make temp/simulation.o
	make temp/objects.o
	make temp/ants.o
	make temp/rooms.o

temp:
	mkdir temp
temp/simulation.o: simulation.c
	make temp
	gcc $(CFLAGS) -c simulation.c -o temp/simulation.o
temp/objects.o: objects.c
	make temp
	gcc $(CFLAGS) -c objects.c -o temp/objects.o
temp/ants.o: ants.c
	make temp
	gcc $(CFLAGS) -c ants.c -o temp/ants.o
temp/rooms.o: rooms.c
	make temp
	gcc $(CFLAGS) -c rooms.c -o temp/rooms.o

