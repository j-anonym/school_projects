#Author: Ján Vavro
#Login: xvavro05

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.cpp))
CC=g++
NAME=d6r
CFLAGS=-Wall -pedantic -Wextra -g
LIBS=-lpcap

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

run:
	./$(NAME)

clean:
	rm -f $(NAME) *.o

