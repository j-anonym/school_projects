#Author: Ján Vavro
#Login: xvavro05

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c)) 
CC=gcc
NAME=EXECUTABLE_NAME
CFLAGS=-std=c99 -Wall -pedantic -Wextra -g

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

run:
	./$(NAME)

clean:
	rm -f $(NAME) *.o

