all: memgrind.c mymalloc.o
	gcc -o memgrind memgrind.c mymalloc.o
mymalloc.o: mymalloc.c
	gcc -c mymalloc.c
clean:
	rm mymalloc.o; rm memgrind;

	
	