CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -Werror

all:proj2

proj2:proj2.c
	$(CC) $(CFLAGS) proj2.c  -o proj2 -lpthread
pack:proj2.c Makefile
	zip proj2.zip proj2.c Makefile
clean:
	rm -f proj2 proj2.zip	
