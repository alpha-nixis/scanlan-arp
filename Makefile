NAME    = scanlan-arp
SRC     = $(wildcard *.c)
OBJ     = $(SRC:%.c=%.o)
HEADERS = $(wildcard *.h)
CC      = gcc
LFLAGS  = -lpthread
CFLAGS  = -Wall

.PHONY: all, clean, install

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf *.o $(NAME)

install:
	cp $(NAME) /usr/bin
