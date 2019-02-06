CC=gcc
CFLAGS=-std=c99 -Wextra -Wall -pedantic -Werror

all: tail tail2 wordcount wordcount-dynamic

tail: tail.c tail2.cc
	$(CC) $(CFLAGS) tail.c -o tail

tail2: tail2.cc
	g++ -std=c++11 -pedantic -Wall tail2.cc -o tail2

libhtab: htab_*.c
	$(CC) $(CFLAGS) -c htab_hash_function.c
	$(CC) $(CFLAGS) -c htab_init.c
	$(CC) $(CFLAGS) -c htab_bucket_count.c
	$(CC) $(CFLAGS) -c htab_clear.c
	$(CC) $(CFLAGS) -c htab_find.c
	$(CC) $(CFLAGS) -c htab_foreach.c
	$(CC) $(CFLAGS) -c htab_free.c
	$(CC) $(CFLAGS) -c htab_lookup_add.c
	$(CC) $(CFLAGS) -c htab_move.c
	$(CC) $(CFLAGS) -c htab_remove.c
	$(CC) $(CFLAGS) -c htab_size.c
	ar rcs libhtab.a *.o

	$(CC) $(CFLAGS) -fPIC -c htab_bucket_count.c -o htab_bucket_count-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_clear.c -o htab_clear-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_find.c -o htab_find-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_foreach.c -o htab_foreach-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_free.c -o htab_free-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_hash_function.c -o htab_hash-function-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_init.c -o htab_init-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_lookup_add.c -o htab_lookup_add-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_move.c -o htab_move-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_remove.c -o htab_remove-dynamic.o
	$(CC) $(CFLAGS) -fPIC -c htab_size.c -o htab_size-dynamic.o
	$(CC) -shared -fPIC -o libhtab.so *-dynamic.o

wordcount.o: wordcount.c
	$(CC) $(CFLAGS) -c wordcount.c -o wordcount.o

io.o: io.c
	$(CC) $(CFLAGS) -c io.c -o io.o

wordcount: wordcount.o libhtab io.o
	$(CC) $(CFLAGS) -o wordcount wordcount.o io.o libhtab.a


wordcount-dynamic: wordcount.o io.o libhtab.so
	$(CC) wordcount.o io.o -o wordcount-dynamic -L. -lhtab

pack: *.c *.h *.cc Makefile
	zip xvavro05.zip *.c *.h *.cc Makefile

clean:
	rm -f tail
	rm -f tail2
	rm -f wordcount
	rm -f wordcount-dynamic
	rm -f libhtab.a
	rm -f libhtab.so
	rm -f *.o
	rm -f xvavro05.zip