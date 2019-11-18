#Author: Ján Vavro
#Login: xvavro05

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.cpp))
CC=g++
NAME=d6r
CFLAGS=-std=c++11
LIBS=-lpcap -pthread

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

run:
	./$(NAME)

clear: clean
clean:
	rm -f $(NAME) *.o

