all: lab2

lab2: lab2.o funciones.o
	gcc -Wall -Wextra -std=c99 -g lab2.o funciones.o -o lab2

lab2.o: lab2.c funciones.h
	gcc -Wall -Wextra -std=c99 -g -c lab2.c

funciones.o: funciones.c funciones.h
	gcc -Wall -Wextra -std=c99 -g -c funciones.c

clean:
	rm -f *.o lab2 *.csv *.tsv
