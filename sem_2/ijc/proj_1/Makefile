CFLAGS = -Wall -pedantic -lm -std=c99 -O2
CC = gcc

all: steg-decode primes

steg-decode: steg-decode.c eratosthenes.c bit_array.h error.c ppm.c
	$(CC) $(CFLAGS) steg-decode.c eratosthenes.c error.c ppm.c  -o steg-decode

primes: primes.c eratosthenes.c bit_array.h error.c
	$(CC) $(CFLAGS) primes.c eratosthenes.c error.c -o primes
	$(CC) $(CFLAGS) primes.c eratosthenes.c error.c -DUSE_INLINE -o primes-i

run: primes
	time ./primes
	time ./primes-i

clean:
	rm primes primes-i steg-decode

