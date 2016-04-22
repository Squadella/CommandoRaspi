CFLAGS=-Wall -Werror -std=gnu99 -g
INCDIR=-I./include

all: CommandoRaspi

main.o: src/main.c
	gcc $(CFLAGS) -c $^ $(INCDIR)

processingEvent.o: src/processingEvent.c
	gcc $(CFLAGS) -c $^ $(INCDIR)

CommandoRaspi: main.o processingEvent.o
	gcc $(CFLAGS) main.o processingEvent.o -o $@ $(INCDIR) -lasound -lpthread -lm

.PHONY: clean
clean:
	rm -f *.o *.out include/*.gch
