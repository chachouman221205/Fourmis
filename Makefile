CFLAGS = -Wall -Wfatal-errors -Werror

# Compile (.c -> .o)
temp:
	mkdir temp

temp/simulation.o: simulation.c
	gcc $(CFLAGS) simulation.c -o temp/simulation.o

